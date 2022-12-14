
#include "networkinterface.h"
#include "IOContextManager.h"
#include <memory>
#include <thread>
#include <map>
#include "Https.h"
#include <string>
#include "jwt-cpp/jwt.h"
#include "Configs.h"
#include "coros.h"
#include "templatefuncs.h"

#include "PackDealerMainSub.h"
#include "PackDealerNoHead.h"
#include "AsynRep.h"
#include "SyncReq.h"


using std::shared_ptr;
using std::make_shared;
using asio::io_context;
using std::thread;
using std::map;
using std::string;



static shared_ptr<map<HtmlBodyType, RoutersType>> _routers = nullptr;
static shared_ptr< TransRoutersType> _routersTrans = nullptr;
static shared_ptr<tsl::htrie_map<char, ConRouterType>> _handlers = nullptr;
static std::map<string, string> _r2;
std::map<string, string> _r3;
namespace SMNetwork {

	static std::string _issuer{ "alqaz" };
	static std::string _type{ "JWS" };
	static std::string _key{ "key" };
	static uint32_t _expiresecond{ 3600 };
	static bool _inited{ false };

	void initjwtconfig(string_view issuer/*="alqaz"*/, string_view type/*="JWS"*/, string_view key/*="key"*/, uint32_t expiresecond/*=3600*/)
	{
		_issuer = issuer;
		_type = type;
		_key = key;
		_expiresecond = expiresecond;
	}

	void initjwtconfig(JWTConf& jwtconf)
	{
		_issuer = jwtconf._issuer;
		_type = jwtconf._type;
		_key = jwtconf._key;
		_expiresecond = jwtconf._timeout;
	}

	bool isjwttokenright(string_view token)
	{
		bool bret{ false };
		if (token.empty())
		{
			return bret;
		}
		auto ver = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ _key }).with_issuer(_issuer);
		BEGIN_STD;
		auto decoded = jwt::decode(token.data());
		std::error_code ec;
		ver.verify(decoded, ec);
		if (ec.value() != 0)
		{
			SPDLOG_WARN("jwt token {} failed with {} {}", token, ec.value(), ec.message());
		}
		bret = (ec.value() == 0) ? true : false;
		END_STD;

		return bret;
	}

	string getjwttoken()
	{
		string ret;
		auto token = jwt::create();
		ret = token.set_issuer(_issuer).set_type(_type).set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(_expiresecond)).set_payload_claim("name", jwt::claim(std::string("alqaz"))).sign(jwt::algorithm::hs256{ _key });
		return ret;
	}

	bool initNetwork()
	{
		bool bret = true;
		if (_inited)
		{
			return bret;
		}
		initjwtconfig(CONFINST.getJWTConf());
		Https::sInit();
		return bret;
	}

	void _initRouters()
	{
		if (_routers)
		{
			return;
		}
		_routers = make_shared<map<HtmlBodyType, RoutersType>>();
	}

	bool  addRouter(HtmlBodyType bodyType, tuple<string, string> path, ConRouterType func)
	{
		bool bret = false;
		auto [codepath, strpath] = path;
		_initRouters();
		auto& routers = (*_routers)[bodyType];
		if (routers.find(strpath) == routers.end())
		{
			routers.insert({ strpath, func });
			bret = true;
		}
		else
		{
			assert((0) && "router can not be add more one time for path ");
			return bret;
		}
		if (_handlers == nullptr)
		{
			_handlers = std::make_shared<tsl::htrie_map<char, ConRouterType>>();
		}
		BEGIN_STD;
		_handlers->insert(codepath, func);
		bret = true;
		END_STD;


		return bret;
	}

	bool  addRouterJson(tuple<string, string> path, ConRouterType func)
	{
		return addRouter(HtmlBodyType::Json, path, func);
	}

	bool  addRouterHtml(tuple<string, string> path, ConRouterType func)
	{
		return addRouter(HtmlBodyType::Html, path, func);
	}

	bool  addRouterTrans(MainCmd mainc, int assc, ConRouterType func)
	{
		bool bret{ false };
		SPDLOG_INFO("add router for main cmd {}", magic_enum::enum_name(mainc));
		if (_routersTrans == nullptr)
		{
			_routersTrans = make_shared<TransRoutersType>();
		}
		BEGIN_STD;
		auto it = _routersTrans->find(mainc);
		if (it != _routersTrans->end())
		{
			if (it->second.find(assc) != it->second.end())
			{
				SPDLOG_ERROR("router for main cmd {} ass cmd {} already exist", magic_enum::enum_name(mainc), assc);
				return bret;
			}
			it->second.insert({ assc, func });
		}
		else
		{
			TransRouterElement vv;
			vv.insert({ assc, func });
			_routersTrans->insert({ mainc, vv });
			bret = true;
		}
		END_STD;
		return bret;
	}

	 void testSpawn()
	{
		asio::co_spawn(IOCTX, []()->asio::awaitable<void> {

			co_return;
			}(), asio::detached);
	}

	ConRouterType  getRouterTrans(MainCmd mainc, int assc)
	{
		ConRouterType ret = nullptr;
		if (_routersTrans == nullptr)
		{
			return ret;
		}
		auto it1 = _routersTrans->find(mainc);
		if (it1 != _routersTrans->end())
		{
			auto it2 = it1->second.find(assc);
			if (it2 != it1->second.end())
			{
				ret = it2->second;
			}
		}
		return ret;
	}

	PtTransRouterElement  getRouterTransByMainC(MainCmd mainc)
	{
		PtTransRouterElement ret = nullptr;
		if (_routersTrans == nullptr)
		{
			return ret;
		}
		auto it = _routersTrans->find(mainc);
		if (it != _routersTrans->end())
		{
			ret = &(it->second);
		}
		return ret;
	}

	PtHtmlRoutersType getRouters()
	{
		if (!_routers)
		{
			SPDLOG_WARN("get router return null");
		}
		assert(_routers);
		return _routers.get();
	}


	void asyn_nng_demo()
	{
		auto serverfunc = std::make_shared<RouterFuncType>([](std::string& req, string token)->asio::awaitable<RouterFuncReturnType>
			{
				RouterFuncReturnType ret = std::make_shared<string>(string("fuck"));

				co_return ret;
			});

		uint16_t portx = 999;

		SMNetwork::addRouterTrans(MainCmd::MainCmdBegin, 0, serverfunc);

		shared_ptr<SMNetwork::PackDealerBase> ps = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoServer));

		SMNetwork::AsynRep rep("127.0.0.1", portx, ChannelType::EchoServer);
		rep.init(ServeMode::SBind, ps);

		shared_ptr<SMNetwork::PackDealerBase> pc = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoClient));
		SMNetwork::SyncReq req("127.0.0.1", portx, ChannelType::EchoClient);
		req.init(ServeMode::SConnect, pc);

		std::string strreq{ "fuck.world" };
		auto strrep = req.reqrep(strreq, 0);
		(void)(strrep);
	}

	CodeRoutersType getCodeRouters()
	{
		return *_handlers;
	}
}

