#include <memory>
#include <iostream>
#include <thread>
#include <fstream>
#include <filesystem>
#include "asio/signal_set.hpp"
#include "hotupdate.h"
#include "TransCmdTag.h"
#include "HttpCmdTag.h"
#include "fmt/format.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/redirect_error.hpp"
#include "asio/use_awaitable.hpp"
#include "FileOp.h"
#include "catch.hpp"

using Catch::Matchers::Equals;

using namespace std;

class httpcmd1 : public HttpCmdTag<httpcmd1, MainCmd>
{
public:
	httpcmd1()
	{

	}
	BEGIN_ROUTER_MAP(MainCmd::DefaultMain);
	END_ROUTER_MAP;
};

class transcmd1 : public TransCmdTag<transcmd1, MainCmd>
{
public:
	transcmd1()
	{
		std::cout << "transcmd1 create..." << std::endl;
	}
	BEGIN_ROUTER_MAP_DB(MainCmd::DefaultMain);
	END_ROUTER_MAP_DB;
};

int main(int argc, char* argv[]) 
{
	asio::io_context ioc;
	//setup ...
	spdlog::set_level(spdlog::level::level_enum::err);
	SMHotupdate::sInit(&ioc);
	int result = Catch::Session().run(argc, argv);
	//clean-up...
	
	return result;
}

TEST_CASE("test ip addr", "www.baidu.com")
{
	asio::ip::address addr;
	BEGIN_ASIO;
	addr = asio::ip::make_address("www.baidu.com");
	REQUIRE_FALSE(true);
	END_ASIO;
	REQUIRE(addr.is_unspecified());

	BEGIN_ASIO;
	addr = asio::ip::make_address("127.0.0.1");
	REQUIRE(addr.is_v4());
	END_ASIO;
	

}

TEST_CASE("dns parse intenet2", "www.baidu.com")
{
	asio::ip::tcp::resolver resv(*IOCTX);
	asio::error_code ec;
	asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
		auto v = co_await resv.async_resolve("www.alqaz.com", "", asio::redirect_error(asio::use_awaitable, ec));
		REQUIRE_FALSE(v.empty());
		
		}, asio::detached);
	IOCTX->run();
}

TEST_CASE("dns parse intenet3", "www.baidu.com")
{
	asio::ip::tcp::resolver resv(*IOCTX);
	asio::error_code ec;
	asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
		auto v = co_await resv.async_resolve("localhost", "", asio::redirect_error(asio::use_awaitable, ec));
		REQUIRE_FALSE(v.empty());
		}, asio::detached);
	IOCTX->restart();
	IOCTX->run();
}

TEST_CASE("dns parse intenet", "www.baidu.com")
{
	REQUIRE(IOCTX->stopped());
	
	asio::ip::tcp::resolver resv(*IOCTX);
	asio::error_code ec;
	asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
		auto v = co_await resv.async_resolve("baidu.com", "http", asio::redirect_error(asio::use_awaitable, ec));
		REQUIRE_FALSE(v.empty());
		}, asio::detached);
	IOCTX->restart();
	IOCTX->run();
}

TEST_CASE("io access", "async io read")
{
	string f{ "asiostream.txt" };
	string fc{ "hello.world" };
	SECTION("prepare file for read")
	{

		filesystem::path p(f);
		ofstream ofs(p, ios_base::trunc);
		ofs << fc;
	}
	SECTION("do asyn read from file")
	{
		REQUIRE(IOCTX->stopped());
		SMHotupdate::fileOp fio(f);
		asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
			auto reps = co_await fio.asyncGetData();
			REQUIRE(reps.length() == fc.length());
			}, asio::detached);
		IOCTX->restart();
		IOCTX->run();
	}
	SECTION("clean env")
	{
		REQUIRE(remove(f.c_str()) == 0);
	}
}
