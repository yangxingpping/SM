
#include "networkinterface.h"
#include <memory>
#include <thread>
#include <map>
#include "Https.h"
#include <string>
#include "jwt-cpp/jwt.h"
#include "Configs.h"
#include "MessageRouter.h"
#include "coros.h"
#include "templatefuncs.h"
#include "ChannelsMgr.h"
#include "PackDealerMainSub.h"
#include "PackDealerNoHead.h"
#include "AsynRep.h"
#include "PackUnpackManager.h"
#include "SyncReq.h"
#include "Routers.h"
#include "MessageRouter.h"
#include <atomic>

using std::shared_ptr;
using std::make_shared;
using asio::io_context;
using std::thread;
using std::map;
using std::string;
using std::move;


namespace SMNetwork {

	static std::string _issuer{ "alqaz" };
	static std::string _type{ "JWS" };
	static std::string _key{ "key" };
	static uint32_t _expiresecond{ 3600 };
	static bool _inited{ false };
	static std::atomic_uint32_t _sockNo{0};

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
		initjwtconfig(SMCONF::Configs::getInst2().getJWTConf());
		Https::sInit();
		MessageRouter::sInit();
		assert(MR->start());
		assert(PackUnpackManager::sInit());
		assert(ChannelsMgr::sInit());
		return bret;
	}
	void asyn_nng_demo()
	{
		auto serverfunc = std::make_shared<RouterFuncType>([](std::string& req, string token)->asio::awaitable<RouterFuncReturnType>
			{
				RouterFuncReturnType ret = std::make_shared<string>(string("fuck"));

				co_return ret;
			});

		uint16_t portx = 999;

		SMCONF::addRouterTrans(MainCmd::MainCmdBegin, 0, serverfunc);

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

	PrepareReqRepRecver prepareReqRep(uint32_t sock, uint32_t msgno)
	{
		PrepareReqRepRecver ret;
		if (!MR->canReqRep(sock, msgno))
		{
			return ret;
		}
		auto rsendnet = MR->prepareSignalSendComplete(sock, msgno);
		auto rrecvnet = MR->prepareSignalRecvNetRep(sock, msgno);
		ret._r1 = move(rsendnet);
		ret._r2 = move(rrecvnet);
		ret._valid = true;
		return ret;
	}

	bool notifySendNetComplete(uint32_t sock, uint32_t msgno, bool flag)
	{
		return MR->notifySendNetComplete(sock, msgno, flag);
	}

	bool notifyNetRepComplete(uint32_t sock, shared_ptr<NMessage> msg)
	{
		return MR->notifyRecvNetRep(sock, msg);
	}

	shared_ptr<oneshot::receiver<shared_ptr<NMessage>>> prepareAppRep(uint32_t sock, uint32_t msgno)
	{
		if (!MR->canWaitAppRep(sock, msgno))
		{
			return nullptr;
		}
		auto appSend = MR->prepareSignalRecvAppRep(sock, msgno);
		return make_shared<oneshot::receiver<shared_ptr<NMessage>>>(move(appSend));
	}

	asio::awaitable<bool> addPackFromIo(uint32_t sock, int mainc, shared_ptr<NMessage> msg)
	{
		auto bret = co_await  MR->addPackFromIo(sock, mainc, msg);
		co_return bret;
	}

	NETWORKINTERFACE_EXPORT asio::awaitable<bool> addPackFromApp(uint32_t sock, shared_ptr<NMessage> msg)
	{
		auto bret = co_await MR->addPackFromApp(sock, msg);
		co_return bret;
	}

	uint32_t newSockNo()
	{
		return ++_sockNo;
	}

}

