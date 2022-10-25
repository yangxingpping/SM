

#include <memory>
#include <iostream>
#include <thread>

#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/compat/nanomsg/nn.h"

#include "asio/signal_set.hpp"

#include "fmt/format.h"

#include "AsynRep.h"

#include "catch.hpp"

using Catch::Matchers::Equals;

using std::thread;
using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;




TEST_CASE("req rep test", "nng")
{
	const char strreq[] = "hello.world";
	const char addr[] = "tcp://127.0.0.1:20086";
	thread threp([=]() {
		int op = 0;
		char buf[1024];
		size_t len = sizeof(buf);
		nng_socket rep = NNG_SOCKET_INITIALIZER;
		op = nng_rep0_open(&rep);
		REQUIRE(op == 0);
		op = nng_listen(rep, addr, NULL, 0);
		REQUIRE(op == 0);
		op = nng_recv(rep, buf, &len, 0);
		REQUIRE(len > 0);
		REQUIRE(op == 0);
		op = nng_send(rep, buf, len, 0);
		REQUIRE(op == 0);
		nng_close(rep);
		rep = NNG_SOCKET_INITIALIZER;
		});
	thread threq([=]() {
		int op = 0;
		char buf[1024];
		size_t len = sizeof(buf);
		nng_socket req = NNG_SOCKET_INITIALIZER;
		op = nng_req0_open(&req);
		REQUIRE(op == 0);
		op = nng_dial(req, addr, NULL, 0);
		REQUIRE(op == 0);
		op = nng_send(req, (void*)strreq, strlen(strreq), 0);
		REQUIRE(op == 0);
		op = nng_recv(req, buf, &len, 0);
		REQUIRE(op == 0);
		REQUIRE(len == strlen(strreq));
		buf[len] = '\0';
		REQUIRE_THAT(strreq, Equals(buf));
		nng_close(req);
		req = NNG_SOCKET_INITIALIZER;
		});
	threq.join();
	threp.join();
	int i = 0;
}

TEST_CASE("xreq xrep test sync", "nng")
{
	// asio::io_context ios;
	// uWS::TemplatedApp<useSSL> app;
	// tf::Executor ex;

	// SMHotupdate::IOContextManager::Init(&ios, &app, &ex);

	// auto func = [&]()
	// {
	// 	asio::signal_set signals(IOCTX, SIGINT, SIGTERM);
	// 	signals.async_wait([&](auto, auto) {});
	// 	while (true)
	// 	{
	// 		IOCTX.run();
	// 	}
	// };
	// std::thread thio(func);

	// string ip = "127.0.0.1";
	// uint16_t reqport = 20086;
	// uint16_t repport = 20087;
	// string straddrreq = fmt::format("tcp://{}:{}", ip, reqport).c_str();
	// string straddrrep = fmt::format("tcp://{}:{}", ip, repport).c_str();
	// const char strreq[] = "hello.world";
	// const char* addrreq = straddrreq.c_str();
	// const char* addrrep = straddrrep.c_str();
	// thread thdevice([=]() {
	// 	int op = 0; nng_socket xrep = NNG_SOCKET_INITIALIZER;
	// 	op = nng_rep0_open_raw(&xrep);
	// 	REQUIRE(op == 0);
	// 	nng_socket xreq = NNG_SOCKET_INITIALIZER;
	// 	op = nng_req0_open_raw(&xreq);
	// 	REQUIRE(op == 0);
	// 	op = nng_listen(xreq, addrreq, NULL, 0);
	// 	REQUIRE(op == 0);
	// 	op = nng_listen(xrep, addrrep, NULL, 0);
	// 	REQUIRE(op == 0);
	// 	op = nng_device(xreq, xrep);
	// 	REQUIRE(op == 0); //not reach this state
	// 	});
	
	// thread threp([=]() {
	// 	int op = 0;
	// 	char buf[1024];
	// 	size_t len = sizeof(buf);
	// 	auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {
	// 		string strret{ "world.hello" };
	// 		co_return strret; 
	// 		}); 
	// 	SMNetwork::addRouterTrans(MainCmd::Echo, (int)(AssEcho::EchoReq), lambdax);
	// 	SMNetwork::AsynRep rep(ip, reqport, ChannelType::EchoServer, MainCmd::Echo, PackType::NoHead, NNgTransType::TCP);
	// 	rep.Init(ServeMode::SConnect);
	// 	});
	// thread threq([=]() {
	// 	int op = 0;
	// 	char buf[1024];
	// 	size_t len = sizeof(buf);
	// 	nng_socket req = NNG_SOCKET_INITIALIZER;
	// 	op = nng_req0_open(&req);
	// 	REQUIRE(op == 0);
	// 	op = nng_dial(req, addrrep, NULL, 0);
	// 	REQUIRE(op == 0);
	// 	op = nng_send(req, (void*)strreq, strlen(strreq), 0);
	// 	REQUIRE(op == 0);
	// 	op = nng_recv(req, buf, &len, 0);
	// 	REQUIRE(op == 0);
	// 	REQUIRE(len == strlen(strreq));
	// 	buf[len] = '\0';
	// 	REQUIRE_THAT(strreq, Equals(buf));
	// 	nng_close(req);
	// 	});
	// threq.join();
	// threp.join();
	// thdevice.detach();
	// thio.detach();
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

