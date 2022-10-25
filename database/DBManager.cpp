#include "DBManager.h"

#include "Utils.h"

using std::lock_guard;

#include "CommonDBProcess.h"

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
    auto conf = CONFINST.getDBConfig();
#ifdef USE_SQLITE
	const auto& dbconf = conf._sqlite;
	_dbmgr = make_shared<DBManager>(dbconf._filepath + "/" + dbconf._file, conf._comm._debug);
#else
	const auto& dbconf = conf._postgres;
	_dbmgr = make_shared<DBManager>(8, dbconf._dbname, dbconf._addr,
		dbconf.username, dbconf.password, conf._comm._debug);
#endif
	_dbmgr->setConfig(conf);
    CommonDBProcess::_inst2.init(ServeMode::SBind);
    return _dbmgr;
}

#ifdef USE_SQLITE
DBManager::DBManager(string file, bool debug)
{
    _confsqlite = make_shared<sqlite::connection_config>();
    _confsqlite->path_to_database = file;
    _confsqlite->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    _confsqlite->debug = debug;
}
#else
DBManager::DBManager(size_t concurrent, string db, string host, string user,
    string pass, bool debug)
{
    _confpg = make_shared<pg::connection_config>();
    _confpg->host = host;
    _confpg->user = user;
    _confpg->password = pass;
    _confpg->dbname = db;
    _confsqlite->debug = debug;
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

#ifdef USE_SQLITE
sqlite::connection* DBManager::_addConnect()
{
    sqlite::connection* db = nullptr;
    BEGIN_STD;
	db = new sqlite::connection(*_confsqlite);
	SPDLOG_INFO("sqlite db connect success, database file {}", _confsqlite->path_to_database);
	END_STD;
    return db;
}
#else
pg::connection* DBManager::_addsConnect()
{
    auto db = new pg::connection();
    db->connectUsing(_confpg);
    SPDLOG_INFO("postgresql db connect success");

    return db;
}
#endif


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
