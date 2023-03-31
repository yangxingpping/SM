#pragma once
/**
 * @file Https.h
 * @author alqaz (alqaz@outlook.com)
 * @brief class for manage http service node. function init(...) and startServer(...) shoud call at the same thread as
	uWesocket implement.
 * @version 0.1
 * @date 2022-01-27
 *
 * @copyright Copyright (c) 2022
 *
 */

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


