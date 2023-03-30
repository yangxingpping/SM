#pragma once

#include "consts.h"
#include "networkinterfaceExport.h"
#include "coros.h"
#include "uwebsockets/App.h"
#include "Utils.h"
#include "asio/io_context.hpp"
#include "taskflow/taskflow.hpp"
#include "tsl/htrie_map.h"
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <set>

using std::shared_ptr;
using std::vector;
using std::string;


class DBManager;
namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT Https
	{
	public:
		static map<string, string>& getRespHeaders(HtmlBodyType type);
		static HtmlBodyType getRespBodyType(const string& context);
		static HtmlBodyType getRespBodyType(string_view context);
		static void sInit();
	
	};
}


