
#include "IOContextManager.h"

#include "asio/signal_set.hpp"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include <thread>

using std::thread;
using std::make_shared;


namespace SMHotupdate
{
static asio::io_context _ios;
static shared_ptr<tf::Executor> _ex = nullptr;
static shared_ptr<std::map<DependIOCTXInitFuncType*, shared_ptr<DependIOCTXInitFuncType>>> _dendioctxinitfuncs = nullptr;


void IOContextManager::Init()
{
	if (!_ex)
	{
		_ex = make_shared<tf::Executor>();
		if (_dendioctxinitfuncs)
		{
			for (auto it : *_dendioctxinitfuncs)
			{
				(*it.first)();
			}
		}
	}
    assert(_ex);
    thread thio([&]()
		{
			bool bstop = false;
	asio::signal_set signals(IOCTX, SIGINT, SIGTERM);
	signals.async_wait([&](auto, auto) {
		bstop = true;
		});
	while (!bstop)
	{
		SPDLOG_INFO("asio run start ");
		IOCTX.run();
		SPDLOG_INFO("asio run finish");
	}
		});
    thio.detach();
}

void IOContextManager::Uninit() { }


tf::Executor& IOContextManager::getExec()
{
    assert(_ex);
    return *_ex;
}

void IOContextManager::run(int concurrentcount/*=4*/)
{
	using std::thread;
	using std::vector;
	vector<thread> ths;
    for (int i = 0; i < concurrentcount; ++i)
    {
        ths.push_back(thread([] {
            IOCTX.run();
            }));
    }
    for (auto& it : ths)
    {
        it.join();
    }

}

bool AddDependIOCTXInitFunc(shared_ptr<DependIOCTXInitFuncType> func)
{
    bool bret = false;
    if (_dendioctxinitfuncs == nullptr)
    {
        _dendioctxinitfuncs = std::make_shared<std::map<DependIOCTXInitFuncType*, shared_ptr<DependIOCTXInitFuncType>>>();
    }
    if (_dendioctxinitfuncs->find(func.get()) != _dendioctxinitfuncs->end())
    {
        SPDLOG_WARN("dpend ioctx  function {} already at set", fmt::ptr(func.get()));
        return bret;
    }
    _dendioctxinitfuncs->insert({ func.get(), func });
    bret = true;
    return bret;
}

io_context& getContext() 
{
    return _ios;
}

}
