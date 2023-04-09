
#include "hotupdate.h"
#include "IOContextManager.h"
#include "spdlog/spdlog.h"
#include <map>

using std::make_shared;
using std::map;

static bool _inited{ false };

static shared_ptr<map<DependIOCTXInitFuncType*, shared_ptr<DependIOCTXInitFuncType>>> _dendioctxinitfuncs = nullptr;

namespace SMHotupdate
{
	bool sInit(asio::io_context* ioc)
	{
		bool bret{ false };
		if (!_inited)
		{
			IOContextManager::sInit(ioc);
			if (_dendioctxinitfuncs)
			{
				for (auto& func : *_dendioctxinitfuncs)
				{
					(*func.first)();
				}
			}
			bret = true;
		}
		return bret;
	}

	HOTUPDATE_EXPORT void sUninit()
	{

	}

	asio::io_context* getIoContext()
	{
		return IOContextManager::getInst()->getIoc();
	}

	bool AddDependIOCTXInitFunc(shared_ptr<DependIOCTXInitFuncType> func)
	{
		bool bret{ true };
		if (_dendioctxinitfuncs == nullptr)
		{
			_dendioctxinitfuncs = make_shared<map<DependIOCTXInitFuncType*, shared_ptr<DependIOCTXInitFuncType>>>();
		}
		if (_dendioctxinitfuncs->find(func.get()) != _dendioctxinitfuncs->end())
		{
			SPDLOG_WARN("dpend ioctx  function {} already at set", fmt::ptr(func.get()));
			return bret;
		}
		_dendioctxinitfuncs->insert({ func.get(), func });
		return bret;
	}

	 asio::io_context* getContext()
	{
		return IOContextManager::getInst()->getIoc();
	}

	tf::Executor* getExec()
	{
		return IOContextManager::getInst()->getExec();
	}

	tf::Executor* getNetExec()
	{
		return IOContextManager::getInst()->getNetExec();
	}

}
