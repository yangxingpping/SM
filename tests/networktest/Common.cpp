

#include <memory>
#include <iostream>
#include <thread>

#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/compat/nanomsg/nn.h"
#include "hotupdate.h"
#include "PackUnpackManager.h"
#include "networkinterface.h"
#include "PackDealerMainSub.h"
#include "Configs.h"
#include "Routers.h"
#include "enums.h"
#include "hotupdate.h"
#include "asio/signal_set.hpp"
#include "fmt/format.h"
#include "FullDuplexChannel.h"
#include "AsynRep.h"
#include "reqreps.h"
#include "magic_enum.hpp"
#include "LocalNetManager.h"
#include "ChannelCombine.h"
#include "TransCmdTag.h"
#include "catch.hpp"

using Catch::Matchers::Equals;
using SMNetwork::PackDealerMainSub;

using std::thread;
using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;
using std::string;
using std::string_view;

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
	asio::io_context ioc;
	SMHotupdate::sInit(&ioc);
	SMNetwork::initNetwork();
	auto plat = shared_ptr<SMNetwork::PlatformPackInterface>(new SMNetwork::MainAssPlatPack(magic_enum::enum_integer(MainCmd::Echo)));
	assert(SMNetwork::addPlatformPack<MainCmd>(plat));
	int result = Catch::Session().run(argc, argv);

	// your clean-up...

	return result;
}

//TEST_CASE("req rep test", "nng")
//{
//	const char strreq[] = "hello.world";
//	const char addr[] = "tcp://127.0.0.1:20086";
//	thread threp([=]() {
//		int op = 0;
//		char buf[1024];
//		size_t len = sizeof(buf);
//		nng_socket rep = NNG_SOCKET_INITIALIZER;
//		op = nng_rep0_open(&rep);
//		REQUIRE(op == 0);
//		op = nng_listen(rep, addr, NULL, 0);
//		REQUIRE(op == 0);
//		op = nng_recv(rep, buf, &len, 0);
//		REQUIRE(len > 0);
//		REQUIRE(op == 0);
//		op = nng_send(rep, buf, len, 0);
//		REQUIRE(op == 0);
//		nng_close(rep);
//		rep = NNG_SOCKET_INITIALIZER;
//		});
//	thread threq([=]() {
//		int op = 0;
//		char buf[1024];
//		size_t len = sizeof(buf);
//		std::this_thread::sleep_for(std::chrono::seconds(2));
//		nng_socket req = NNG_SOCKET_INITIALIZER;
//		op = nng_req0_open(&req);
//		REQUIRE(op == 0);
//		op = nng_dial(req, addr, NULL, 0);
//		REQUIRE(op == 0);
//		op = nng_send(req, (void*)strreq, strlen(strreq), 0);
//		REQUIRE(op == 0);
//		op = nng_recv(req, buf, &len, 0);
//		REQUIRE(op == 0);
//		REQUIRE(len == strlen(strreq));
//		buf[len] = '\0';
//		REQUIRE_THAT(strreq, Equals(buf));
//		nng_close(req);
//		req = NNG_SOCKET_INITIALIZER;
//		});
//	threq.join();
//	threp.join();
//}

class echo : public TransCmdTag<echo, MainCmd>
{
public:
	BEGIN_ROUTER_MAP_DB(MainCmd::Echo);
	ROUTER_DB3(&echo::Echo, AssEcho::Echo);
	END_ROUTER_MAP_DB;
	EchoRep Echo(EchoReq req)
	{
		EchoRep rep;
		rep.code = 10086;
		return rep;
	}
};

//TEST_CASE("fullduplexchannel tcp impl test", "localNetManager")
//{
//	using namespace SMNetwork;
//
//	auto [creq, crep] = LNM->createPair(MainCmd::Echo);
//	uint32_t reqno{ 1 };
//	shared_ptr<string> s1tos2 = make_shared<string>("hello");
//
//	asio::co_spawn(IOCTX, [&]()->asio::awaitable<void> {
//		//s1 send to s2
//		auto bflag = co_await crep->reqrep(s1tos2);
//		assert(*bflag == *s1tos2);
//		co_return;
//		}, asio::detached);
//	auto icount = IOCTX.run();
//}

TEST_CASE("tcp req rep", "localNetManager")
{
	using namespace SMNetwork;
	
	auto [s1, s2] = LNM->createPair(MainCmd::Echo);
	
	asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
		
		EchoReq ereq;
		auto strreq = my_to_string(ereq);
		auto packer = PUM->clone(magic_enum::enum_integer(MainCmd::Echo));
		assert(packer);
		shared_ptr<string> packsend = make_shared<string>();
		packsend->resize(packer->len());
		packer->setAss(magic_enum::enum_integer(AssEcho::Echo));
		assert(packer->pack(span<char>(packsend->begin(), packsend->begin() + packer->len())));
		packsend->append(*strreq);
		shared_ptr<string> rep{nullptr};
		BEGIN_ASIO;
		rep = co_await s1->reqrep(packsend);
		END_ASIO;
		packer->setAss(magic_enum::enum_integer(AssEcho::EchoRep));
		assert(packer->unpack(string_view(rep->begin(), rep->begin() + packer->len())));
		string strbody = rep->substr(packer->len());
		EchoRep myrep;
		REQUIRE(my_json_parse_from_string(myrep, strbody));
		REQUIRE(myrep.code == 10086);
		co_return;
		}, asio::detached);
	auto icount = IOCTX->run();
}

TEST_CASE("pack and unpack", "PackDealerMainSub")
{
	auto serverfunc = std::make_shared<RouterFuncType>([](std::string& req, string token)->asio::awaitable<RouterFuncReturnType>
		{
			RouterFuncReturnType ret = std::make_shared<string>(string("fuck"));

			co_return ret;
		});

	uint16_t portx = 998;

	SMCONF::addRouterTrans(MainCmd::Echo, 0, serverfunc);
	PackDealerMainSub p(MainCmd::Echo, ChannelType::EchoServer);
	EchoReq req{ "name", "pass", "token" };
	p.setAssc(magic_enum::enum_integer(AssEcho::EchoReq));
	auto strreq = my_to_string(req);
	auto frame = p.pack(string_view(strreq->data()));
	req.username = "alqaz";
	auto strreq2 = p.unpack(string_view(frame->data(), frame->length()));
	REQUIRE(my_json_parse_from_string(req, string_view(strreq2->data())));
}

TEST_CASE("xreq xrep test async", "nng")
{
	const char strreq[] = "hello.world";
	const char addrreq[] = "tcp://127.0.0.1:20086";
	const char addrrep[] = "tcp://127.0.0.1:20087";
	thread thdevice([=]() {
		int op = 0; nng_socket xrep = NNG_SOCKET_INITIALIZER;
		op = nng_rep0_open_raw(&xrep);
		REQUIRE(op == 0);
		nng_socket xreq = NNG_SOCKET_INITIALIZER;
		op = nng_req0_open_raw(&xreq);
		REQUIRE(op == 0);
		op = nng_listen(xreq, addrreq, NULL, 0);
		REQUIRE(op == 0);
		op = nng_listen(xrep, addrrep, NULL, 0);
		REQUIRE(op == 0);
		op = nng_device(xreq, xrep);
		REQUIRE(op == 0); //not reach this state
		});

	thread threp([=]() {
		int op = 0;
		char buf[1024];
		size_t len = sizeof(buf);
		nng_socket rep = NNG_SOCKET_INITIALIZER;
		op = nng_rep0_open(&rep);
		REQUIRE(op == 0);
		op = nng_dial(rep, addrreq, NULL, 0);
		REQUIRE(op == 0);
		op = nng_recv(rep, buf, &len, 0);
		REQUIRE(len > 0);
		REQUIRE(op == 0);
		op = nng_send(rep, buf, len, 0);
		REQUIRE(op == 0);
		nng_close(rep);
		});
	thread threq([=]() {
		int op = 0;
		char buf[1024];
		size_t len = sizeof(buf);
		nng_socket req = NNG_SOCKET_INITIALIZER;
		op = nng_req0_open(&req);
		REQUIRE(op == 0);
		op = nng_dial(req, addrrep, NULL, 0);
		REQUIRE(op == 0);
		op = nng_send(req, (void*)strreq, strlen(strreq), 0);
		REQUIRE(op == 0);
		op = nng_recv(req, buf, &len, 0);
		REQUIRE(op == 0);
		REQUIRE(len == strlen(strreq));
		buf[len] = '\0';
		REQUIRE_THAT(strreq, Equals(buf));
		nng_close(req);
		});
	threq.join();
	threp.join();
	thdevice.detach();
}

