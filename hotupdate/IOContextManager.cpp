 
#include "IOContextManager.h"
#include "hotupdate.h"
#include "asio/signal_set.hpp"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include <thread>
#include <mutex>

using std::thread;
using std::make_shared;
using std::mutex;
using std::lock_guard;

static mutex _mForInitIoContext;


namespace SMHotupdate
{
	static shared_ptr<IOContextManager> _inst = { nullptr };

void IOContextManager::sInit(asio::io_context* ioc)
{
	lock_guard<mutex> lock(_mForInitIoContext);
	assert(_inst == nullptr);
	if (_inst != nullptr)
	{
		return;
	}
	_inst = make_shared<IOContextManager>(ioc);
    assert(ioc != nullptr);
	_inst->run(1);
}

IOContextManager* IOContextManager::getInst()
{
	assert(_inst != nullptr);
	return _inst.get();
}

void IOContextManager::sUninit() 
{
}


IOContextManager::IOContextManager(asio::io_context* ioc)
	:_ioc(ioc)
{
	assert(_ioc != nullptr);
	assert(_ex == nullptr);
	_ex = make_shared<tf::Executor>();
}

tf::Executor* IOContextManager::getExec()
{
	assert(_ex != nullptr);
	return _ex.get();
}


asio::io_context* IOContextManager::getIoc()
{
	return _ioc;
}


bool IOContextManager::run(int threads)
{
	bool bret{ false };
	

	thread thio([&]()
		{
			bool bstop = false;
			asio::signal_set signals(*_ioc, SIGINT, SIGTERM);
			signals.async_wait([&](auto, auto) {
				bstop = true;
				});
			while (!bstop)
			{
				SPDLOG_INFO("asio run start ");
				_ioc->run();
				SPDLOG_INFO("asio run finish");
			}
		});
	thio.detach();
	bret = true;
	return bret;
}

}
