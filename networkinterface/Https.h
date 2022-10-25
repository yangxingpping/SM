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

using std::shared_ptr;
using std::vector;
using std::string;


class DBManager;
namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT Https
	{
	public:
		static void addResHeader(string& k, string& v);
		static Https& getDefaultInst();
	public:
		Https();
		Https(const Https&) = delete;
		Https(const Https&&) = delete;
		/**
		 * @brief init http server instance
		 *
		 * @param httproot local http resource path
		 * @param wsroot websocket root path
		 */
		bool init(string htmlroot, string jsonrooot, string wsroot);
		/**
		 * @brief start http server instance, this call will block at httpserver.run
		 *
		 * @return true
		 * @return false
		 */
		bool startServer();

	protected:
		bool _isAuthed(string_view url, const string& token);
		bool _initRes();
		bool _initHttp(std::map<string, string>* headers, HtmlBodyType bodytype);
		bool _initHtml();
		bool _initJson();
		bool _initWs();

	private:
		uWS::TemplatedApp<useSSL>* _app = nullptr;


		string _httpRoot;
		string _jsonRoot;
		string _wsRoot;
	};
}

#define DEFHTTP (SMNetwork::Https::getDefaultInst())

