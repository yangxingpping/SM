/*****************************************************************//**
 * \file   DBConnectManager.h
 * \brief  this class is used to access database
 *
 * \author alqaz
 * \date   March 2022
 *********************************************************************/
#pragma once

#include "databaseExport.h"
#include "templatefuncs.h"
#include "myconcept.h"
#include "DBConnectNNG.h"
#include "DBConnectTCP.h"
#include "networkinterface.h"
#include "PlatformPackInterface.h"
#include "concurrentqueue/concurrentqueue.h"
#include "spdlog/spdlog.h"
#include "iguana/json.hpp"
#include "magic_enum.hpp"
#include <string>
#include <memory>
#include <span>
#include <any>
#include <functional>

using std::string;
using std::make_shared;
using std::shared_ptr;
using std::any;
using std::function;
using std::any_cast;
using std::span;

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

		asio::awaitable<bool> executeQueryTCP(std::shared_ptr<std::string> req, string& rep)
		{
			bool bret = false;
			string strrep;

			shared_ptr<DBConnectTCP> ret;
			if (!_connsTcp.try_dequeue(ret))
			{
				ret = make_shared<DBConnectTCP>(_ip, _port);
			}
			BEGIN_ASIO;
			bret = co_await ret->_execQuery(*req, rep);
			END_ASIO;
			if (bret)
			{
				_connsTcp.enqueue(ret);
			}
			co_return bret;
		}

		asio::awaitable<bool> executeQueryNNG(std::shared_ptr<std::string> req, string& rep)
		{
			bool bret = false;
			string strrep;
			SPDLOG_INFO("send db query req {} ", *req);
			auto addrinfo = SMCONF::getDBConfig()->_nngClient;

			shared_ptr<DBConnectNNG> ret;
			if (!_connsNNG.try_dequeue(ret))
			{
				BEGIN_ASIO;
				ret = make_shared<DBConnectNNG>(addrinfo._ip, addrinfo._port);
				END_ASIO;
			}
			bret = co_await ret->_execQuery(req, rep);
			if (bret)
			{
				_connsNNG.enqueue(ret);
			}
			co_return bret;
		}

		template <class Req, class Rep>
		asio::awaitable<bool> executeQuery(Req req, Rep& rep, int op, DBMethod method = DBMethod::TCP)
		{
			bool bret = false;
			auto strreq = my_to_string(req);
			auto pstrreq = make_shared<string>();
			auto _packer = SMNetwork::clonePlatformPack(magic_enum::enum_integer(_mainc));
			pstrreq->resize(_packer->len());
			_packer->setAss(op);
			bret = _packer->pack(span<char>(pstrreq->begin(), pstrreq->end()));
			assert(bret);
			if (!bret)
			{
				SPDLOG_WARN("pack database query req failed");
				co_return bret;
			}
			pstrreq->append(strreq->begin(), strreq->end());
			string platformRep;
			bret = false;
			BEGIN_ASIO;
			switch (method)
			{
			case DBMethod::TCP:
			{
				bret = co_await executeQueryTCP(pstrreq, platformRep);
			}break;
			case DBMethod::NNG:
			{
				bret = co_await executeQueryNNG(pstrreq, platformRep);
			}break;
			default:
			{
				assert(0);
			}break;
			}
			END_ASIO;
			if (!bret)
			{
				rep.code = magic_enum::enum_integer(statusCode::sendDBQueryExecFailed);
				SPDLOG_WARN("database operate {} return false ", SMUtils::getOpDBNameByValue(op));
			}
			else
			{
				bret = my_json_parse_from_string(rep, string_view(platformRep.data() + _packer->HeadLen(), platformRep.length() - _packer->HeadLen()));
				if (!bret)
				{
					rep.code = magic_enum::enum_integer(statusCode::invalidJson);
					SPDLOG_WARN("parse json string failed");
				}
			}
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

		MainCmd _mainc = MainCmd::DBQuery;
	};

}

