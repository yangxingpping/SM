#pragma once
#include "hotupdateExport.h"
#include "consts.h"
#include "uwebsockets/App.h"
#include "taskflow/taskflow.hpp"
#include "asio/io_context.hpp"
#include <string>
#include <memory>

using asio::io_context;
using std::shared_ptr;
using std::unique_ptr;

typedef std::function<void()> DependIOCTXInitFuncType;

#define BEGIN_ASIO  try {
#define END_ASIO }\
	catch (const asio::system_error& e)\
	{\
		SPDLOG_WARN("asio return system error {}", e.what());\
	}\
	catch (const std::runtime_error& e)\
	{\
		SPDLOG_WARN("asio return runtime error {}", e.what());\
	}\
	catch (const std::exception& e)\
	{\
		SPDLOG_WARN("asio return std exception {}", e.what());\
	}\
	catch (...)\
	{\
		SPDLOG_WARN("asio return unexpected error ");\
	}

namespace SMHotupdate
{
	bool HOTUPDATE_EXPORT AddDependIOCTXInitFunc(shared_ptr<DependIOCTXInitFuncType> func);

HOTUPDATE_EXPORT io_context& getContext();
class HOTUPDATE_EXPORT IOContextManager
{
public:
	static void Init();
	static void Uninit();
	static tf::Executor& getExec();
	static void run(int concurrentcount=1);
};
}

#define IOCTX (SMHotupdate::getContext())

#define IOCTXHOTUPDATE (SMHotupdate::getContext())

#define TFEXEC (SMHotupdate::IOContextManager::getExec())
