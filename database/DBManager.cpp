#include "DBManager.h"

#include "Utils.h"
#include "conf.h"
#include "CommonDBProcess.h"

using std::lock_guard;

namespace SMDB
{

shared_ptr<DBManager> DBManager::_dbmgr = nullptr;

void DBManager::SDBMgrInit(const DatabaseConfig& conf)
{
    #ifdef USE_SQLITE
    const auto& dbconf = conf._sqlite;
        _dbmgr = make_shared<DBManager>(dbconf._filepath + "/" + dbconf._file, conf._comm._debug);
    #else
        const auto& dbconf = conf._postgres;
        _dbmgr = make_shared<DBManager>(8, dbconf._dbname, dbconf._addr,
            dbconf.username, dbconf.password, conf._comm._debug);
    #endif
    _dbmgr->setConfig(conf);

    
}

shared_ptr<DBManager>  DBManager::SDBMgrInit2()
{
    if (_dbmgr)
    {
        return _dbmgr;
    }
    auto conf = SMCONF::getDBConfig();
#ifdef USE_SQLITE
	const auto& dbconf = conf->_sqlite;
	_dbmgr = make_shared<DBManager>(dbconf._filepath + "/" + dbconf._file, conf->_comm._debug);
#else
	const auto& dbconf = conf->_postgres;
	_dbmgr = make_shared<DBManager>(8, dbconf._dbname, dbconf._addr,
		dbconf.username, dbconf.password, conf._comm._debug);
#endif
	_dbmgr->setConfig(*conf);
    CommonDBProcess::_inst2.init(ServeMode::SBind);
    return _dbmgr;
}

#ifdef USE_SQLITE
DBManager::DBManager(string file, bool debug)
{
    _connconf = make_shared<sqlconn::connection_config>();
    _connconf->path_to_database = file;
    _connconf->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    _connconf->debug = debug;
}
#else
DBManager::DBManager(size_t concurrent, string db, string host, string user,
    string pass, bool debug)
{
    _connconf = make_shared<sqlconn::connection_config>();
    _connconf->host = host;
    _connconf->user = user;
    _connconf->password = pass;
    _connconf->dbname = db;
    _connconf->debug = debug;
    for (size_t i = 0; i < concurrent; ++i)
    {
        auto pconn = _addPostgresConnect();
        _conns.enqueue(pconn);
    }
}
#endif

void DBManager::setConfig(const DatabaseConfig& conf)
{
    _conf = conf;
}


string DBManager::serializeConf()
{
    string ret;
#ifdef USE_SQLITE
    ret = fmt::format("sqlite db file {}", _connconf->path_to_database);
#else
    ret = fmt::format("db ip[], port[]");
#endif
    return ret;
}

sqlconn::connection* DBManager::_addConnect()
{
    sqlconn::connection* db = nullptr;
    BEGIN_STD;
#ifdef USE_SQLITE
	db = new sqlconn::connection(*_connconf);
	SPDLOG_INFO("sqlite db connect success, database file {}", _connconf->path_to_database);
#else
	auto db = new sqlconn::connection();
	db->connectUsing(_connconf);
#endif
	END_STD;
    if (!db)
    {
        SPDLOG_ERROR("connect to database failed, connect config info {}", serializeConf());
    }
    assert(db);
    return db;
}


RediscConnectType* DBManager::_addRedisConnect()
{
    auto rconf = _conf._redis;
    
    cpp_redis::client* _client = nullptr;/* = new cpp_redis::client();
    _client->connect(rconf._addr, rconf._port);
    */
    return _client;
}

void DBManager::addRedisConnect(RediscConnectType* conn)
{
    if(conn)
    {
        _redis.enqueue(conn);
    }
}

DBConnectType* DBManager::GetConn()
{
    DBConnectType* pret {nullptr};
    if (!_conns.try_dequeue(pret))
    {
        pret = _addConnect();
    }
    SPDLOG_INFO("get connect pointer {}", fmt::ptr(pret));
    if (!pret)
    {
        SPDLOG_WARN("get connect for db failed");
    }
    assert(pret);
    return pret;
}

RediscConnectType* DBManager::GetRedisConn()
{
    RediscConnectType* pret {nullptr};
    if(!_redis.try_dequeue(pret))
    {
        pret = _addRedisConnect();
    }
    return pret;
}

void DBManager::addConnect(DBConnectType* conn)
{
    if(conn)
    {
        _conns.enqueue(conn);
    }
}

DBInst::DBInst()
{
    _conn = DBMGR2->GetConn();
}

DBConnectType& DBInst::operator*()
{
    return *_conn;
}
DBConnectType* DBInst::get()
{
    return _conn;
}
DBConnectType& DBInst::ref()
{
    return *_conn;
}
DBInst::~DBInst()
{
    if (_conn)
    {
        DBMGR2->addConnect(_conn);
    }
    _conn = nullptr;
}

RDBInst::RDBInst()
{
    _conn = DBMGR2->GetRedisConn();
}

RediscConnectType& RDBInst::operator*()
{
    return *_conn;
}

RDBInst::~RDBInst()
{
    if (_conn)
    {
        DBMGR2->addRedisConnect(_conn);
    }
    _conn = nullptr;
}


}
