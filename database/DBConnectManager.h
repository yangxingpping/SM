
#pragma once

#include "databaseExport.h"
#include "templatefuncs.h"
#include "myconcept.h"
#include "DBConnectNNG.h"
#include "DBConnectTCP.h"
#include "concurrentqueue/concurrentqueue.h"
#include "spdlog/spdlog.h"
#include "iguana/json.hpp"
#include "magic_enum.hpp"
#include <string>
#include <memory>
#include <any>
#include <functional>

using std::string;
using std::make_shared;
using std::shared_ptr;
using std::any;
using std::function;
using std::any_cast;

enum class DBMethod
{
	TCP,
	NNG,
};

#define DBCMGRPT (SMDB::DBConnectManager::getInst())
#define DBCMGRREF (SMDB::DBConnectManager::getInst2())
namespace SMDB
{
	class DATABASE_EXPORT DBConnectManager
	{
	public:
		static shared_ptr< DBConnectManager> getInst();
		static DBConnectManager& getInst2();

		template <class Rep>
		asio::awaitable<bool> executeQueryTCP(std::shared_ptr<std::string> req, Rep& rep, int op)
		{
			bool bret = false;
			string strrep;

			shared_ptr<DBConnectTCP> ret;
			if (!_connsTcp.try_dequeue(ret))
			{
				ret = make_shared<DBConnectTCP>(_ip, _port);
			}
			BEGIN_ASIO;
			bret = co_await ret->_execQuery(*req, strrep, op);
			END_ASIO;
			if (bret)
			{
				_connsTcp.enqueue(ret);
				bret = my_json_parse_from_string(rep, strrep);
				if(bret)
				{
					co_return bret;
				}
				else
				{
					SPDLOG_WARN("tcp database query req {}, rep {} parse rep failed", *req, strrep);
				}
			}
			else
			{
				SPDLOG_WARN("execute query req {} failed", *req);
			}
			co_return bret;
		}

		template <class Rep>
		asio::awaitable<bool> executeQueryNNG(std::shared_ptr<std::string> req, Rep& rep, int op)
		{
			bool bret = false;
			string strrep;
			SPDLOG_INFO("send db query req {} query type {}", *req, SMUtils::getOpDBNameByValue(op));
			auto addrinfo = SMCONF::Configs::getInst2().getDBConfig()._nngClient;

			shared_ptr<DBConnectNNG> ret;
			if (!_connsNNG.try_dequeue(ret))
			{
				BEGIN_ASIO;
				ret = make_shared<DBConnectNNG>(addrinfo._ip, addrinfo._port);
				END_ASIO;
			}
			bret = co_await ret->_execQuery(*req, strrep, op);
			if (bret)
			{
				_connsNNG.enqueue(ret);
			}


			bret = my_json_parse_from_string(rep, strrep);
			if (!bret)
			{
				SPDLOG_WARN("nng database operate {} str {} return {} parse failed ", SMUtils::getOpDBNameByValue(op), *req, strrep);
			}
			co_return bret;
		}

		template <class Req, class Rep>
		asio::awaitable<bool> executeQuery(Req req, Rep& rep, int op, DBMethod method = DBMethod::TCP)
		{
			bool bret = false;
			auto strreq = my_to_string(req);
			BEGIN_ASIO;
			switch (method)
			{
			case DBMethod::TCP:
			{
				bret = co_await executeQueryTCP(strreq, rep, op);
			}break;
			case DBMethod::NNG:
			{
				bret = co_await executeQueryNNG(strreq, rep, op);
			}break;
			default:
			{
				assert(0);
			}break;
			}
			if (!bret)
			{
				rep.code = magic_enum::enum_integer(statusCode::sendDBQueryExecFailed);
				SPDLOG_WARN("database operate {} return false ", SMUtils::getOpDBNameByValue(op));
			}
			END_ASIO;
			co_return bret;
		}


	private:
		string _ip;
		uint16_t _port;
		string _nngip;
		uint16_t _nngport;
		int16_t _timeoutMiseconds = 5000;

		moodycamel::ConcurrentQueue<shared_ptr<DBConnectTCP>> _connsTcp;
		moodycamel::ConcurrentQueue<shared_ptr<DBConnectNNG>> _connsNNG;
		DBConnectManager(string ip, uint16_t port);

	};

}

