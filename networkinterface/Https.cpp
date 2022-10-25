#include "Https.h"
#include "Utils.h"
#include "IOContextManager.h"
#include "FilePathMonitor.h"
#include "Configs.h"
#include "wss.h"
#include "spdlog/spdlog.h"
#include "asio/signal_set.hpp"
#include "networkinterface.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/awaitable.hpp"
#include "jwt-cpp/jwt.h"

#include <locale>
#include <memory>
#include <thread>

using std::make_shared;
using std::string;

namespace SMNetwork
{
    static map<string, string> _resHeaders;
    static std::unique_ptr<Https> _defaulthttp = nullptr;
    static string _noAuthRep{ "need auth" };
    static string PostProcess404(uWS::HttpResponse<useSSL>* rep)
    {
        rep->writeStatus("404 Not Found");
        SPDLOG_WARN("response is empty, set as 404");
        return "404 not found";
    }

	void Https::addResHeader(string& k, string& v)
	{
        _resHeaders.insert({ k,v });
	}

	Https& Https::getDefaultInst()
	{
        if (!_defaulthttp)
        {
            _defaulthttp = std::make_unique<Https>();
			auto hconf = CONFINST.getHttpConfig();
			_defaulthttp->init(hconf._rootHtmlPath, hconf._rootJsonPath, hconf._wsPath);
            std::thread th([=]() {
                _defaulthttp->startServer();
            });
            th.detach();
        }
        assert(_defaulthttp);
        return *_defaulthttp;
	}

	Https::Https()

    {
    }

    bool Https::init(string httproot, string jsonrooot, string wsroot)
    {
        bool bret{ true };
        if (!_httpRoot.empty() || !_jsonRoot.empty() || !_wsRoot.empty() || _app)
        {
            SPDLOG_ERROR("httproot {}, jsonroot {}, wsroot {}, _app {}", _httpRoot, _jsonRoot, _wsRoot, fmt::ptr(_app));
            assert(0);
            bret = false;
        }
        else
        {
            _httpRoot = httproot;
            _jsonRoot = jsonrooot;
            _wsRoot = wsroot;
            _app = new uWS::TemplatedApp<useSSL>();
        }
        return bret;
    }

    bool Https::startServer()
    {
        if (!_initRes() || !_initJson() || !_initHtml() || !_initWs())
        {
            return false;
        }
        _app->listen(CONFINST.getHttpConfig()._port,
            [=](auto* listen_socket)
            {
                if (listen_socket) {
                    SPDLOG_INFO(
                        "Listening on port {}", CONFINST.getHttpConfig()._port);
                }
            });
        _app->run();
        return true;
    }

    bool Https::_isAuthed(string_view url, const string& token)
    {
        bool bret = false;
        auto jwtc = CONFINST.getJWTConf();
        if (!token.empty()) //check if token valid
        {
            auto decoded = jwt::decode(token);
            auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ jwtc._key }).with_issuer(jwtc._issuer);
            try
            {
                verifier.verify(decoded);
                auto decoded = jwt::decode(token);
                auto& palyload = decoded.get_payload();
                auto dexpire = decoded.get_expires_at();
                auto dnow = std::chrono::system_clock::now();
                if (dexpire.time_since_epoch() < dnow.time_since_epoch())
                {
                    SPDLOG_WARN("token {} time expired", token);
                    bret = false;
                }
                bret = true;
            }
            catch (std::runtime_error& e)
            {
                SPDLOG_WARN("verify token {} failed with runtime error {}", token, e.what());
            }
            catch (const std::exception& e)
            {
                SPDLOG_WARN("verfiy token {} failed with {}", token, e.what());
            }
        }
        else //check if url not need auth
        {
            bret = jwtc._noAuthRouter.find(url) != jwtc._noAuthRouter.end();
        }
        return bret;
    }

    bool Https::_initRes()
    {
        bool bret = true;
        static tsl::htrie_map<char, string> _contentTypes = {
            {".js", "text/javascript"},
        {".html", "text/html;charset=UTF-8"},
        {".htm", "text/html;charset=UTF-8"},
        {".json", "application/json"},
            {".woff", "font/woff"}, {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".css", "text/css"},
            {".ico", "image/vnd.microsoft.icon"}, {".json", "application/json"} };

        auto repdealer = [=](string url, uWS::HttpResponse<useSSL>* rep)->asio::awaitable<void> {
            size_t pos = url.find_last_of('.');
            if (pos != string::npos)
            {
                auto it = _contentTypes.find(string_view{ url.substr(pos, url.size()) });
                if (it != _contentTypes.end())
                {
                    rep->writeHeader("content-type", it.value());
                }
                else
                {
                    SPDLOG_WARN("resource {} have no resource type", url);
                }
            }
            for (auto& h : _resHeaders)
            {
                rep->writeHeader(h.first, h.second);
            }
            string_view resp;
            BEGIN_ASIO;
            resp = co_await RESLOAD->asyncGetResource(url);
            END_ASIO;
            if (!resp.empty())
            {
                SPDLOG_INFO("http response data len [{}] fo url [{}]", resp.length(), url);
                rep->end(resp);
            }
            else
            {
                rep->end(PostProcess404(rep));
            }
            
        };

        auto dealer = [=](auto* rep, auto* req) {
            if (!RESLOAD->isReady())
            {
                rep->end("not ready");
                return;
            }
            rep->onAborted(
                []() {});
            string url = string{ req->getUrl() };
            if (url.length() == 1 && url[0] == '/')
            {
                url = "/index.html";
            }
            
            string params = string{ req->getQuery() };
            asio::co_spawn(IOCTXHOTUPDATE, repdealer(url, rep), asio::detached);
        };

        _app->get("/*", dealer);
        return bret;
    }

    bool Https::_initHttp(std::map<string, string>* headers, HtmlBodyType bodytype)
    {
        bool bret = true;
        auto r1 = getRouters();
        auto repdealer = [=](auto func, auto* rep, string msg, string url, map<string, string>* hs, string token)->asio::awaitable<void> {
            string strreq = msg;
            string resp;
            BEGIN_ASIO;
            resp = co_await(*func)(strreq, token);
            END_ASIO;
            for (const auto& head : *hs)
            {
                rep->writeHeader(head.first, head.second);
            }
            if (resp.empty())
            {
                resp = PostProcess404(rep);
            }

            rep->end(resp);
            co_return;
        };
        auto& routers = (*r1)[bodytype];
        for (auto it = routers.begin(); it != routers.end(); ++it)
        {
            string root1 = fmt::format("/{}{}", _httpRoot, it->first);
            SPDLOG_INFO("add router for path[{}] with http type {}", root1, magic_enum::enum_name(bodytype));

            auto dealer = [=, this](uWS::HttpResponse<useSSL>* rep, uWS::HttpRequest* req)
            {
                string url = string{ req->getUrl() };
                string querystr = string{ req->getQuery() };
                rep->onAborted([]() {});
                auto func = it->second;
                auto datadealer = [=, this](std::string_view chunk, bool isEnd)
                {
                    string postdata(chunk.data(), chunk.length());
                    if (isEnd)
                    {
                        SPDLOG_INFO("start deal post url {} with data {}", url, postdata);
                        string token{ req->getHeader("authorization") }; 
                        asio::co_spawn(IOCTX, repdealer(func, rep, (postdata.empty() ? querystr : postdata), url, headers, token), asio::detached);              
                    }
                };
                rep->onData(datadealer);
            };
            _app->get(root1, dealer);
            _app->post(root1, dealer);
        }
        return bret;
    }

    bool Https::_initHtml()
    {
        static map<string, string> headers{ {"content-type", "text/html;charset=UTF-8"} };
        for (auto& h : _resHeaders)
        {
            headers.insert({ h.first, h.second });
        }
        return _initHttp(&headers, HtmlBodyType::Html);
    }

    bool Https::_initJson()
    {
        static map<string, string> headers{ {"content-type", "application/json"} };
        return _initHttp(&headers, HtmlBodyType::Json);
    }

    bool Https::_initWs()
    {
        bool bret = true;
        uWS::TemplatedApp<useSSL>::WebSocketBehavior<WebSocketData> ud;
        ud.message = [=](auto* ws, std::string_view message, uWS::OpCode opCode)
        {
            SPDLOG_INFO("recv data:{}", message);
            asio::co_spawn(
                IOCTX,
                [=]() -> asio::awaitable<void>
                {
                    string resp = "not found";
                    auto handlers = getCodeRouters();
                    auto it = handlers.find(string_view{ message.data(), 8 });
                    if (it != handlers.end())
                    {
                        string req = string(message.data() + 12, message.length() - 12);
                        string token;
                        string strrep;
                        BEGIN_ASIO;
                            strrep = co_await(*(*it))(req, token);
                        END_ASIO;
                            resp.assign(message.data(), message.data() + 8);
                        std::ostringstream convert;
                        auto v = SMUtils::packlen(strrep.length());
                        for (size_t a = 0; a < v.size(); a++) {
                            convert << v[a];
                        }
                        std::string key_string = convert.str();
                        resp += key_string;
                        resp += strrep;
                    }
                    ws->send(resp, opCode);
                },
                asio::detached);
        };

        ud.open = [&](auto* ws)
        {
            SPDLOG_INFO("websocket connected");
            ws->getUserData()->valid = true;
        };
        ud.close = [](auto* ws, int size, std::string_view msg)
        {
            SPDLOG_INFO("websocket closed");
        };

        ud.maxPayloadLength = 16 * 1024 * 1024;
        ud.idleTimeout = 16;
        ud.maxBackpressure = 1024 * 1024;
        ud.compression = uWS::SHARED_COMPRESSOR;
        ud.maxBackpressure = 1 * 1024 * 1024;
        ud.resetIdleTimeoutOnSend = false;
        ud.sendPingsAutomatically = true;
        ud.upgrade = [](auto* res, auto* req, auto* context)
        {
            res->template upgrade<WebSocketData>({},
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"), context);
        };
        string wsroot = fmt::format("/{}", _wsRoot);
        _app->ws<WebSocketData>(wsroot, std::move(ud));
        return bret;
    }
}
