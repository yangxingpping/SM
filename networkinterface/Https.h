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
		static Https& getDefaultInst();
		static map<string, string>& getRespHeaders(HtmlBodyType type);
		static HtmlBodyType getRespBodyType(const string& context);
		static HtmlBodyType getRespBodyType(string_view context);
		static void sInit();
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
		bool _initHttp(HtmlBodyType bodytype);
		bool _initHtml();
		bool _initJson();
		bool _initWs();

	private:
		uWS::TemplatedApp<useSSL>* _app = nullptr;
		uWS::Loop* _loop = nullptr;
		uWS::Loop* _loopinit = nullptr;
		std::set<uWS::HttpResponse<useSSL>*> _abortReps;

		string _httpRoot;
		string _jsonRoot;
		string _wsRoot;
	};
}

#define DEFHTTP (SMNetwork::Https::getDefaultInst())

