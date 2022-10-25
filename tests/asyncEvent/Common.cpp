#include <memory>
#include <iostream>
#include <thread>
#include <fstream>
#include <filesystem>
#include <string>

#include "HttpCmdTag.h"
#include "enums.h"
#include "asio/signal_set.hpp"
#include "asio/steady_timer.hpp"
#include "asio/use_awaitable.hpp"
#include "asio/high_resolution_timer.hpp"
#include "asio/redirect_error.hpp"
#include "asio/co_spawn.hpp"
#include "IOContextManager.h"
#include "CoEvent.h"
#include "fmt/format.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/strand.hpp"
#include "FileOp.h"
#include <future>

#include "catch.hpp"
#include <atomic>

using Catch::Matchers::Equals;

using namespace std;

class demo : public HttpCmdTag<demo>
{
public:
	demo()
	{
		SPDLOG_INFO("call demo");
	}
	BEGIN_ROUTER_MAP(MainCmd::Echo)
	END_ROUTER_MAP
};

int repeatcount = 20000;

int main(int argc, char* argv[]) 
{
	spdlog::set_level(spdlog::level::level_enum::err);
	SMHotupdate::IOContextManager::Init();
	SPDLOG_INFO("repeat count {}", repeatcount);
	int result = Catch::Session().run(argc, argv);
	
	return result;
}

TEST_CASE("asio exception ", "coroutine throw exception at co_spawn")
{
	IOCTX.restart();
	SMHotupdate::IOContextManager::run();
}

TEST_CASE("asio exception co_spawn", "coroutine throw exception at co_spawn")
{
	atomic_int _intv{ 0 };
	asio::co_spawn(IOCTX, []()->asio::awaitable<void> {
		//throw std::exception(std::string("thank you"));
		co_return;
		}, asio::detached);
	IOCTX.restart();
	SMHotupdate::IOContextManager::run();
}

TEST_CASE("strand", "simple test")
{
	int _intv{ 0 };
	asio::io_context::strand st(IOCTX);
	for (int i = 0; i < repeatcount; ++i)
	{
		st.post([&]() {
			++_intv;
			});
	}
	IOCTX.restart();
	IOCTX.run();
	REQUIRE(_intv == repeatcount);
}

TEST_CASE("strand multithread serilize", "simple test")
{
	int _intv{ 0 };
	asio::io_context::strand st(IOCTX);
	for (int i = 0; i < repeatcount; ++i)
	{
		st.post([&]() {
			++_intv;
			});
	}
	IOCTX.restart();
	SMHotupdate::IOContextManager::run();
	REQUIRE(_intv == repeatcount);
}

TEST_CASE("strand multithread concurrence int", "simple test")
{
	int _intv{ 0 };
	for (int i = 0; i < repeatcount; ++i)
	{
		asio::dispatch([&]() {
			++_intv;
			});
	}
	IOCTX.restart();
	SMHotupdate::IOContextManager::run();
	//REQUIRE(_intv != repeatcount);
}

TEST_CASE("strand multithread concurrence atom", "simple test")
{
	atomic_int _intv{ 0 };
	for (int i = 0; i < repeatcount; ++i)
	{
		asio::dispatch([&]() {
			++_intv;
			});
	}
	IOCTX.restart();
	SMHotupdate::IOContextManager::run();
	REQUIRE(_intv == repeatcount);
}


TEST_CASE("CoEvent", "triger at execute")
{
	/*atomic_int _intv{ 0 };
	for (int i = 0; i < repeatcount; ++i)
	{
		shared_ptr<SMHotupdate::CoEvent> ev = make_shared<SMHotupdate::CoEvent>(IOCTX);

		asio::co_spawn(IOCTX, [=]()->asio::awaitable<void> {
			co_await ev->async_wait();
			co_return;
			}, asio::detached);

		TFEXEC.async([=, &_intv]() {
			++_intv;
			asio::co_spawn(IOCTX, [=]()->asio::awaitable<void> {
				ev->trigger_all();
				co_return;
				}, asio::detached);
			});
	}
	IOCTX.run();
	REQUIRE(_intv == repeatcount);*/
}

TEST_CASE("asio timer cancel by taskflow thread", "create asio trigger from taskflow thread")
{
	atomic_int _intv{ 0 };
	atomic_int _awaitv{ 0 };

	for (int i = 0; i < repeatcount; ++i)
	{
		shared_ptr<asio::steady_timer> ev = make_shared<asio::steady_timer>(IOCTX);
		ev->expires_after(std::chrono::seconds(10));
		asio::co_spawn(IOCTX, [=, &_awaitv]()->asio::awaitable<void> {
			asio::error_code ec;
			co_await ev->async_wait(asio::redirect_error(asio::use_awaitable, ec));
			++_awaitv;
			co_return;
			}, asio::detached);
		TFEXEC.async([=, &_intv]() {
			asio::co_spawn(IOCTX, [=, &_intv]()->asio::awaitable<void> {
				++_intv;
				try
				{
					ev->cancel();
				}
				catch (asio::system_error& e)
				{
					SPDLOG_WARN("error {}", e.what());
				}
				co_return;
				}, asio::detached);
		});
	}
	IOCTX.restart();
	auto vvx = IOCTX.run();
	(void)(vvx);
	REQUIRE(_awaitv == repeatcount);
	REQUIRE(_intv == repeatcount);
}

TEST_CASE("asio timer", "timer trigger at taskflow thread, just for test, as asio timer cancel use throw exception(not effective)")
{
	atomic_int _intv{ 0 };
	atomic_int _awaitv{ 0 };

	for (int i = 0; i < repeatcount; ++i)
	{
		shared_ptr<asio::steady_timer> ev = make_shared<asio::steady_timer>(IOCTX);
		ev->expires_after(std::chrono::seconds(10));
		asio::co_spawn(IOCTX, [=, &_awaitv]()->asio::awaitable<void> {
			asio::error_code ec;
			co_await ev->async_wait(asio::redirect_error(asio::use_awaitable, ec));
			++_awaitv;
			co_return;
			}, asio::detached);
		asio::co_spawn(IOCTX, [=, &_intv]()->asio::awaitable<void> {
			++_intv;
			try
			{
				ev->cancel_one();
			}
			catch (asio::system_error& e)
			{
				SPDLOG_WARN("error {}", e.what());
			}
			co_return;
			}, asio::detached);
	}
	IOCTX.restart();
	auto vvx = IOCTX.run();
	(void)(vvx);
	REQUIRE(_awaitv == repeatcount);
	REQUIRE(_intv == repeatcount);
}



