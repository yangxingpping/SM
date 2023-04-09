#pragma once
#include "hotupdateExport.h"
#include "consts.h"
#include "hotupdate.h"
#include "taskflow/taskflow.hpp"
#include "asio/io_context.hpp"
#include <string>
#include <memory>
#include <map>

using asio::io_context;
using std::shared_ptr;
using std::unique_ptr;
using std::map;


namespace SMHotupdate
{
	
class  IOContextManager
{
public:
	static void sInit(asio::io_context* ioc);
	static IOContextManager* getInst();
	static void sUninit();
	
public:
	IOContextManager(asio::io_context* ioc);
	tf::Executor* getExec();
	tf::Executor* getNetExec();
	asio::io_context* getIoc();
	bool run(int threads);
private:
	
	asio::io_context* _ioc{nullptr};
	shared_ptr<tf::Executor> _ex{nullptr};
	shared_ptr<tf::Executor> _exNet{nullptr};
};
}


