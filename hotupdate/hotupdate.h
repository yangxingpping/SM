#pragma once
#include "hotupdateExport.h"
#include "asio/io_context.hpp"
#include "taskflow/taskflow.hpp"
#include <memory>


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

using std::shared_ptr;
typedef std::function<void()> DependIOCTXInitFuncType;

namespace SMHotupdate
{
	HOTUPDATE_EXPORT bool sInit(asio::io_context* ioc);
	HOTUPDATE_EXPORT asio::io_context* getIoContext();
	HOTUPDATE_EXPORT bool  AddDependIOCTXInitFunc(shared_ptr<DependIOCTXInitFuncType> func);
	HOTUPDATE_EXPORT asio::io_context* getContext();
	HOTUPDATE_EXPORT tf::Executor* getExec();
}

#define IOCTX (SMHotupdate::getContext())

#define APPCTX (SMHotupdate::getContext())

#define IOCTXHOTUPDATE (SMHotupdate::getContext())

#define TFEXEC (SMHotupdate::getExec())
