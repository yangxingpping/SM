#include "Https.h"
#include "Utils.h"
#include "FilePathMonitor.h"
#include "hotupdate.h"
#include "Configs.h"
#include "wss.h"
#include "spdlog/spdlog.h"
#include "asio/signal_set.hpp"
#include "networkinterface.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/awaitable.hpp"
#include "jwt-cpp/jwt.h"
#include "Routers.h"
#include "conf.h"
#include <locale>
#include <memory>
#include <thread>

using std::make_shared;
using std::string;

namespace SMTEMPLATE
{
	class Apps
	{
	public:
		static std::unique_ptr<Apps> _defaulthttp;
		static string PostProcess404(uWS::HttpResponse<useSSL>* rep)
		{
			rep->writeStatus("404 Not Found");
			SPDLOG_WARN("response is empty, set as 404");
			return "404 not found";
		}
		static Apps& getDefaultInst()
		{
			if (!_defaulthttp)
			{
				_defaulthttp = std::make_unique<Apps>();
				co_spawn(*IOCTX, RESLOAD->waitHotUpdate(), asio::detached);
				auto hconf = SMCONF::getHttpConfig();
				_defaulthttp->init(hconf->_rootHtmlPath, hconf->_rootJsonPath, hconf->_wsPath, hconf->_keyPath, hconf->_perPath);
			}
			assert(_defaulthttp);
			return *_defaulthttp;
		}

		Apps()
		{
		}

		bool init(string httproot, string jsonrooot, string wsroot, string key /*= ""*/, string pem /*= ""*/)
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

				SPDLOG_INFO("start https flag {}", useSSL);
				uWS::SocketContextOptions context;
				context.cert_file_name = pem.c_str();
				context.key_file_name = key.c_str();
				_app = new uWS::TemplatedApp<useSSL>(context);
				_loopinit = uWS::Loop::get();
			}
			return bret;
		}

		bool startServer()
		{
			_loop = uWS::Loop::get();
			if (_loop != _loopinit)
			{
				SPDLOG_ERROR("Https::init and http::startServer must call at the same thread");
			}
			assert(_loopinit == _loop);
			if (!_initRes() || !_initJson() || !_initHtml() || !_initWs())
			{
				return false;
			}
			_app->listen(SMCONF::getHttpConfig()->_port,
				[=](auto* listen_socket)
				{
					if (listen_socket) {
						SPDLOG_INFO("Listening on port {}", SMCONF::Configs::getInst2().getHttpConfig()._port);
					}
				});
			_app->run();
			return true;
		}

		bool _isAuthed(string_view url, const string& token)
		{
			bool bret = false;
			auto jwtc = SMCONF::getJWTConf();
			if (!token.empty()) //check if token valid
			{
				auto decoded = jwt::decode(token);
				auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ jwtc->_key }).with_issuer(jwtc->_issuer);
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
				bret = jwtc->_noAuthRouter.find(url) != jwtc->_noAuthRouter.end();
			}
			return bret;
		}

		bool _initRes()
		{
			bool bret = true;

			auto repdealer = [=, this](string url, uWS::HttpResponse<useSSL>* rep)->asio::awaitable<void> {

				string_view resp;
				BEGIN_ASIO;
				resp = co_await RESLOAD->asyncGetResource(url);
				END_ASIO;
				_loop->defer([this, rep, url, resp]() {
					auto it = _abortReps.find(rep);
				if (it != _abortReps.end())
				{
					_abortReps.erase(it);
					return;
				}
				size_t pos = url.find_last_of('.');
				if (pos != string::npos)
				{
					auto it = SMNetwork::Https::getRespBodyType(string_view{ url.substr(pos, url.size()) });
					
					auto& headers = SMNetwork::Https::getRespHeaders(it);
					for (auto& kv : headers)
					{
						rep->writeHeader(kv.first, kv.second);
					}
					
				}
				
				if (!resp.empty())
				{
					SPDLOG_INFO("http response data len [{}] fo url [{}]", resp.length(), url);
					rep->end(resp);
				}
				else
				{
					rep->end(PostProcess404(rep));
				}
					});
				co_return;
			};

			_app->get("/*", [=, this](auto* rep, auto* req) {
				if (!RESLOAD->isReady())
				{
					rep->end("not ready");
					return;
				}
			rep->onAborted(
				[rep, this]() {
					_abortReps.insert(rep);
				});
			string url = string{ req->getUrl() };
			if (url.length() == 1 && url[0] == '/')
			{
				url = "/index.html";
			}
			SPDLOG_INFO("get url [{}] resource", url);
			asio::co_spawn(*IOCTX, repdealer(url, rep), asio::detached);
				});
			return bret;
		}

		bool _initHttp(HtmlBodyType bodytype)
		{
			bool bret = true;
			
			auto repdealer = [=, this](auto func, auto* rep, shared_ptr<string> msg, string token, Apps* inst, HtmlBodyType body)->asio::awaitable<void> {
				RouterFuncReturnType resp;
				for (auto it2 = func->begin(); it2 != func->end(); ++it2)
				{
					BEGIN_ASIO;
					resp = co_await(*(*it2))(*msg, token);
					if (!resp->empty())
					{
						break;
					}
					END_ASIO;
				}
				inst->_loop->defer([=]() {
					auto it = inst->_abortReps.find(rep);
				if (it != inst->_abortReps.end())
				{
					inst->_abortReps.erase(it);
					return;
				}
				auto& hs = SMNetwork::Https::getRespHeaders(body);
				for (const auto& head : hs)
				{
					rep->writeHeader(head.first, head.second);
				}
				if (!resp || resp->empty())
				{
					rep->end(PostProcess404(rep));
				}
				else
				{
					rep->end(*resp);
				}
					});
				co_return;
			};
			auto routers = SMCONF::getRouters(bodytype);
			for (auto it = routers->begin(); it != routers->end(); ++it)
			{
				string root1 = fmt::format("/{}{}", _httpRoot, it->first);
				SPDLOG_INFO("add router for path[{}] with http type {}", root1, magic_enum::enum_name(bodytype));

				auto dealer = [=, this](uWS::HttpResponse<useSSL>* rep, uWS::HttpRequest* req)
				{
					string url = string{ req->getUrl() };
					shared_ptr<string> querystr = std::make_shared<string>(req->getQuery());
					shared_ptr<string> postdata = std::make_shared<string>();
					rep->onAborted([rep, this]() {
						SPDLOG_WARN("rep {} aborted", fmt::ptr(rep));
					_abortReps.insert(rep);
						});
					auto funcs = &(it->second);
					auto datadealer = [=, this](std::string_view chunk, bool isEnd)
					{
						auto it = _abortReps.find(rep);
						if (it != _abortReps.end())
						{
							SPDLOG_WARN("request {} aborted", fmt::ptr(rep));
							_abortReps.erase(it);
							return;
						}
						postdata->append(chunk);
						if (isEnd)
						{
							SPDLOG_INFO("start deal post url {} with post data {}, query str {}", url, *postdata, *querystr);
							string token{ req->getHeader("authorization") };
							asio::co_spawn(*IOCTX, repdealer(funcs, rep, (postdata->empty() ? querystr : postdata), token, this, bodytype), asio::detached);
						}
					};
					rep->onData(datadealer);
				};
				_app->get(root1, dealer);
				_app->post(root1, dealer);
			}
			return bret;
		}

		bool _initHtml()
		{
			return _initHttp(HtmlBodyType::Html);
		}

		bool _initJson()
		{
			return _initHttp(HtmlBodyType::Json);
		}

		bool _initWs()
		{
			bool bret = true;
			uWS::TemplatedApp<useSSL>::WebSocketBehavior<WebSocketData> ud;
			ud.message = [=](auto* ws, std::string_view message, uWS::OpCode opCode)
			{
				SPDLOG_INFO("recv data:{}", message);
				asio::co_spawn(
					*IOCTX,
					[=]() -> asio::awaitable<void>
					{
						string resp;
				auto handlers = SMCONF::getCodeRouters();
				auto it = handlers.find(string_view{ message.data(), 8 });
				if (it != handlers.end())
				{
					string req = string(message.data() + 12, message.length() - 12);
					string token;
					RouterFuncReturnType strrep;
					BEGIN_ASIO;
					strrep = co_await(*(*it))(req, token);
					END_ASIO;
					resp.assign(message.data(), message.data() + 8);
					std::ostringstream convert;
					auto v = SMUtils::packlen(strrep->length());
					for (size_t a = 0; a < v.size(); a++) {
						convert << v[a];
					}
					std::string key_string = convert.str();
					resp += key_string;
					resp += *strrep;
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

	private:
		uWS::TemplatedApp<useSSL>* _app = nullptr;
			uWS::Loop* _loop = nullptr;
			uWS::Loop* _loopinit = nullptr;
			std::set<uWS::HttpResponse<useSSL>*> _abortReps;

			string _httpRoot;
			string _jsonRoot;
			string _wsRoot;
	};

	std::unique_ptr<Apps> Apps::_defaulthttp = nullptr;

}

#define DEFHTTP (SMTEMPLATE::Apps::getDefaultInst())
