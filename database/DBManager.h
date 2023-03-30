#pragma once

/**
 * @file DBManager.h class declare for connections to database instance(postgresql, sqlite, mysql...)
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "databaseExport.h"
#include "Configs.h"

#include "concurrentqueue/concurrentqueue.h"
#ifdef USE_SQLITE
#include "sqlpp11/sqlite3/connection.h"
#else
#include "sqlpp11/postgresql/connection.h"
#include "sqlpp11/postgresql/exception.h"
#endif

#include "cpp_redis/cpp_redis"

#include "sqlpp11/sqlpp11.h"
#include <memory>
#include <string>
#include <stdint.h>
#include <list>
#include <mutex>

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::list;
using std::mutex;

#ifdef USE_SQLITE
namespace sqlconn = sqlpp::sqlite3;
#else
namespace sqlconn = sqlpp::postgresql;
#endif

#define DBConnectType sqlconn::connection


#define RediscConnectType cpp_redis::client

#define DBMGRPT (SMDB::DBManager::_dbmgr.get())
#define DBMGREF (*SMDB::DBManager::_dbmgr)

#define DBMGR2 (SMDB::DBManager::SDBMgrInit2())

namespace SMDB
{

	class DBInst;

	class DATABASE_EXPORT DBManager
	{
	public:
		static shared_ptr<DBManager> _dbmgr;
		static void SDBMgrInit(const DatabaseConfig& conf);
		static shared_ptr<DBManager>  SDBMgrInit2();
		friend DBInst;
	public:
#ifdef USE_SQLITE
		DBManager(string file, bool debug); //for local sqlite3
#else
		DBManager(size_t concurrent, string db, string host, string user, string pass,
			bool debug); // for local postgresql
#endif
		void setConfig(const DatabaseConfig& conf);
		string serializeConf();
		DBConnectType* GetConn();
		RediscConnectType* GetRedisConn();
		void addConnect(DBConnectType* conn);
		void addRedisConnect(RediscConnectType* conn);
	protected:
		sqlconn::connection* _addConnect();
		RediscConnectType* _addRedisConnect();

	private:
		std::shared_ptr<sqlconn::connection_config> _connconf = nullptr;
		DatabaseConfig _conf;
		moodycamel::ConcurrentQueue<DBConnectType*> _conns;

		moodycamel::ConcurrentQueue<cpp_redis::client*> _redis;
	};

	class DATABASE_EXPORT DBInst
	{
	public:
		DBInst();
		DBConnectType& operator*();
		DBConnectType* get();
		DBConnectType& ref();
		~DBInst();
	private:
		DBConnectType* _conn = nullptr;
	};

	class DATABASE_EXPORT RDBInst
	{
	public:
		RDBInst();
		RediscConnectType& operator*();

		~RDBInst();
	private:
		RediscConnectType* _conn = nullptr;
	};

}