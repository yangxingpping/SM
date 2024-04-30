
#include "networkinterface.h"
#include "IOContextManager.h"
#include "coros.h"
#include "whereami.h"
#include "asio/co_spawn.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "magic_enum.hpp"
#include "DBManager.h"
#include "socket/TcpClient.h"
#include "socket/TcpServer.h"
#include "DBTcp.h"
#include "subprocess.h"

#include "date/date.h"
#include "date/tz.h"
#include "Routers.h"
#include "database.h"
#include "nngs/AsynRep.h"
#include "nngs/AsynReq.h"
#include "nngs/SyncReq.h"
#include "PackDealerNoHead.h"
#include "FileOp.h"

#include "sha256.h"

#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/supplemental/util/platform.h"

#ifdef OS_WIN
#include <windows.h>
#endif

#include <memory>

#define PARALLEL 128

using std::shared_ptr;
using std::make_shared;
using std::thread;

enum class T
{
	first,
	second,
};

void test_sha256()
{
	string str{ "hello.world" };
	SHA256 sha256;
	string result = sha256(str);
	int i = 0;
	(void)(i);
}

void test_encode_decode()
{
	MainCmd mainc = MainCmd::DBQuery;
	short assc = 1;
	auto v = SMUtils::packcmdrep(mainc, assc, 8);
}

void test_nng_xreqrep()
{
	const char reqaddr[] = "tcp://127.0.0.1:20086";
	const char repaddr[] = "tcp://127.0.0.1:20087";
	int nn_op = 0;
	nng_socket rreq = NNG_SOCKET_INITIALIZER;
	nng_socket rrep = NNG_SOCKET_INITIALIZER;
	nng_socket req = NNG_SOCKET_INITIALIZER;
	nng_socket rep = NNG_SOCKET_INITIALIZER;
	nn_op = nng_req0_open_raw(&rreq);
	nn_op = nng_rep0_open_raw(&rrep);
	nn_op = nng_req0_open(&req);
	nn_op = nng_rep0_open(&rep);
	nn_op = nng_listen(rreq, repaddr, NULL, 0);
	nn_op = nng_listen(rrep, reqaddr, NULL, 0);
	thread thdevice([&]() {
 		nng_device(rreq, rrep);
		SPDLOG_INFO("nng_device finish");
		nng_close(rrep);
		nng_close(rreq);
		return 0;
		});
	
	char strreq[] = "hello.world";
	char strrep[] = "fuck.world";

	thread threq([&]() {
		char bufrecv[1024];
		size_t lenrecv = sizeof(bufrecv);
		int nn = 0;
		nn = nng_dial(req, reqaddr, NULL, 0);
		nn = nng_send(req, strreq, strlen(strreq), 0);
		nn = nng_recv(req, bufrecv, &lenrecv, 0);
		nng_close(req);
		return 0;
		});
	thread threp([&]() {
		char bufrecv[1024];
		size_t lenrecv = sizeof(bufrecv);
		int op = 0;
		op = nng_dial(rep, repaddr, NULL, 0);
		op = nng_recv(rep, bufrecv, &lenrecv, 0);
		nng_send(rep, strrep, strlen(strrep), 0);
		nng_close(rep);
		return 0;
		});

	threq.join();
	threp.join();

	thdevice.detach();
}

void test_asio()
{
	using namespace SMNetwork;
}


void fatal(const char* func, int rv)
{
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

/*  The client runs just once, and then returns. */
int client(const char* url, const char* msecstr)
{
	nng_socket sock;
	int        rv;
	nng_msg* msg;
	nng_time   start;
	nng_time   end;
	unsigned   msec;

	msec = atoi(msecstr);

	if ((rv = nng_req0_open(&sock)) != 0) {
	}

	if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
	}

	start = nng_clock();

	if ((rv = nng_msg_alloc(&msg, 0)) != 0) {
	}
	if ((rv = nng_msg_append_u32(msg, msec)) != 0) {
	}

	if ((rv = nng_sendmsg(sock, msg, 0)) != 0) {
	}

	if ((rv = nng_recvmsg(sock, &msg, 0)) != 0) {
	}
	end = nng_clock();
	nng_msg_free(msg);
	nng_close(sock);

	printf("Request took %u milliseconds.\n", (uint32_t)(end - start));
	return (0);
}
enum CC { INIT, RECV, WAIT, SEND };

struct work
{
	CC state;
	nng_aio* aio;
	nng_msg* msg;
	nng_ctx  ctx;
};

asio::awaitable<void> taskf(struct work* work)
{
	nng_aio_set_msg(work->aio, work->msg);
	work->msg = NULL;
	work->state = SEND;
	nng_ctx_send(work->ctx, work->aio);
	co_return;
}

void server_cb(void* arg)
{
	struct work* work = (struct work*)arg;
	nng_msg* msg;
	int          rv;
	uint32_t     when;

	switch (work->state)
	{
	case INIT:
	{
		work->state = RECV;
		nng_ctx_recv(work->ctx, work->aio);
	}break;
	case RECV:
	{
		if ((rv = nng_aio_result(work->aio)) != 0)
		{
		}
		msg = nng_aio_get_msg(work->aio);
		if ((rv = nng_msg_trim_u32(msg, &when)) != 0)
		{
			// bad message, just ignore it.
			nng_msg_free(msg);
			nng_ctx_recv(work->ctx, work->aio);
			return;
		}
		work->msg = msg;
		asio::co_spawn(*IOCTX, taskf(work), asio::detached);
		int j = 1;
	}break;
	case SEND:
	{
		if ((rv = nng_aio_result(work->aio)) != 0) {
			nng_msg_free(work->msg);
		}
		work->state = RECV;
		nng_ctx_recv(work->ctx, work->aio);
	}break;
	default:
		break;
	}
}

struct work* alloc_work(nng_socket sock)
{
	struct work* w;
	int          rv;

	if ((w = (struct work*)nng_alloc(sizeof(*w))) == NULL) {
	}
	if ((rv = nng_aio_alloc(&w->aio, server_cb, w)) != 0) {
	}
	if ((rv = nng_ctx_open(&w->ctx, sock)) != 0) {
	}
	w->state = INIT;
	return (w);
}

struct work* works[PARALLEL];
nng_socket   sock;
int server(const char* url)
{
	
	
	int          rv;
	int          i;

	/*  Create the socket. */
	rv = nng_rep0_open(&sock);
	if (rv != 0) {
	}

	for (i = 0; i < PARALLEL; i++) {
		works[i] = alloc_work(sock);
	}

	if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
	}

	for (i = 0; i < PARALLEL; i++) {
		server_cb(works[i]); // this starts them going (INIT state)
	}
	return 0;
	//for (;;) {
	//	nng_msleep(3600000); // neither pause() nor sleep() portable
	//}
}

void test_pack()
{
	using namespace SMUtils;
	uint16_t src{ 0x0102 };
	string dst;
	SMUtils::packuint16(dst, src);
	uint16_t cdst{ 0 };
	SMUtils::unpackuint16(dst, cdst);
	assert(src == cdst);
}

void test_nng_asyn_asio()
{
	char addr[] = "tcp://127.0.0.1:10086";
	server(addr);
	thread clientx([&]() {
		char seconds[] = "3000";
		client(addr, seconds);
		});
	clientx.join();

}

void test_nng_asyn_asio2()
{
	/*auto serverfunc = std::make_shared<RouterFuncType>([](std::string req, string token)->asio::awaitable<RouterFuncReturnType>
	{
		RouterFuncReturnType  ret = std::make_shared<string>(string{ "hello" });

		co_return ret;
	});

	uint16_t portx = 888;

	SMCONF::addRouterTrans(MainCmd::DefaultMain, 0, serverfunc);

	shared_ptr<SMNetwork::PackDealerBase> ps = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoServer));
	shared_ptr<SMNetwork::PackDealerBase> pc = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoClient));
	
	SMNetwork::AsynRep rep("127.0.0.1", portx, ChannelType::EchoServer);
	rep.init(ServeMode::SBind, ps);

	SMNetwork::SyncReq req("127.0.0.1", portx, ChannelType::EchoClient);
	req.init(ServeMode::SConnect, pc);

	std::string strreq{ "hello.world" };
	auto strrep = req.reqrep(strreq, 0);
	int i = 1;*/
}

//void test_nng_asyn_reqrep_asio()
//{
//	auto serverfunc = std::make_shared<RouterFuncType>([](std::string req, string token)->asio::awaitable<RouterFuncReturnType>
//		{
//			RouterFuncReturnType  ret = std::make_shared<string>(string("fuck"));
//
//			co_return ret;
//		});
//
//	uint16_t portx = 888;
//
//	SMCONF::addRouterTrans(MainCmd::DefaultMain, 0, serverfunc);
//
//	shared_ptr<SMNetwork::PackDealerBase> ps = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoServer));
//	shared_ptr<SMNetwork::PackDealerBase> pc = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoClient));
//
//	SMNetwork::AsynRep rep("127.0.0.1", portx, ChannelType::EchoServer);
//	rep.init(ServeMode::SBind, ps);
//
//	SMNetwork::AsyncReq req("127.0.0.1", portx, ChannelType::EchoClient);
//	req.init(ServeMode::SConnect, pc);
//
//	asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
//		std::string strreq{ "hello.world" };
//		auto strrep = co_await req.reqrep(strreq);
//		SPDLOG_INFO("req {} recv rep {}", strreq, *strrep);
//		co_return;
//		}(), asio::detached);
//
//	std::this_thread::sleep_for(std::chrono::seconds(10));
//}

void test_asio_tcp_client_timeout()
{
	
	asio::co_spawn(*IOCTX, []()->asio::awaitable<void> {
		SMNetwork::TcpClient<ChannelType::DBClient, NetHeadType::FixPackLenPlaceHolder, ChannelIsInitiative::Passive> c("127.0.0.1", 9998);
		auto channel = co_await c.getChannel();
		(void)(channel);
		co_return;
		}, asio::detached);

}

void test_asio_timer()
{
}

void test_db_tcp_server_try_stop()
{
	using namespace SMDB;
	shared_ptr<DBTcp> v = make_shared<DBTcp>("127.0.0.1", 2089);
	v->init();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	v->stop();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	assert(v->stoped());

}


void test_async_stream_file()
{
	const char* command_line[] = { "echo", "hello.world", NULL};
	struct subprocess_s subprocess;
	int result = subprocess_create(command_line, 0, &subprocess);
	assert(result == 0);
}

void test_zoned_time()
{
	date::zoned_time t2{ SMDB::getDefaultTimeZone(), std::chrono::system_clock::now() };
	auto vv2 = sqlpp::chrono::day_point(sqlpp::chrono::floor<sqlpp::chrono::days>(t2.get_local_time().time_since_epoch()));
}

int main(int argc, char* argv[])
{
	string exepath;
	exepath.resize(1024);
	int outlen{ 0 };
	auto mypath = wai_getExecutablePath(exepath.data(), exepath.size(), &outlen);

	asio::io_context ioc;
	bool biorun = true;
	test_pack();
	//SMDB::init(true);
	test_encode_decode();
	test_sha256();
	test_nng_xreqrep();

	SMHotupdate::sInit(&ioc);

	//test_zoned_time();

	std::istringstream ss{ "2022-06-22" };
	date::sys_days tp;
#ifdef _WIN32
	ss >> parse("%F", tp);
#else
	ss >> date::parse("%F", tp);
#endif
	if (!ss.bad())
	{
		int i = 1;
		(void)(i);
	}
	auto func = [&]()
	{
		asio::signal_set signals(*IOCTX, SIGINT, SIGTERM);
		signals.async_wait([&](auto, auto) {});
		while (biorun)
		{
			T t = T::first;
			SPDLOG_INFO("asio run start {}", magic_enum::enum_name(t));
			IOCTX->run();
			SPDLOG_INFO("asio run finish");
		}
	};

	std::thread th(func);
	//th.detach();

	test_async_stream_file();
	test_db_tcp_server_try_stop();
	test_asio();
	test_asio_timer();
	test_nng_asyn_asio();
	//test_nng_asyn_reqrep_asio();
	//test_asio_tcp_client_timeout();
	//test_nng_asyn_asio2();
	asio::co_spawn(*IOCTX, [=, &biorun]() -> asio::awaitable<void> {
		SPDLOG_INFO("last spawn for ioctx");
		biorun = false;
		IOCTX->stop();
		co_return;
		}(),
			asio::detached);
	th.join();
	
	/*int i = 1;
	std::cin >> i;*/
	return 0;
}

