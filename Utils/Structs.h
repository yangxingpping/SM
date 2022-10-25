#pragma once
#include "UtilsExport.h"
#include <string>

#include "tsl/htrie_set.h"

using std::string;
using std::string_view;

enum class DatabaseType
{
    Postgresql,
    Sqlite,
};

enum class DatabaseMode
{
    Embed,
    Alone,
};


class DatabaseRemoteConfig
{
  public:
    string _ip;
    uint16_t _port;
    bool _useSSL;
};

typedef struct
{
    string _file;
    string _filepath;
} SqliteLocalConfig;

typedef struct
{
    string _dbname;
    bool _useSSL;
    string _addr;
    uint16_t _port;
    string username;
    string password;
} PostgresqlLocalConfig;

typedef struct
{
    bool _useSSL;
    string _addr;
    uint16_t _port;
    string username;
    string password;
} RedisLocalConfig;

class DatabaseConfigCommon
{
  public:
    DatabaseType type;
    DatabaseMode _model;
    bool _debug;
};


class DatabaseConfig
{
  public:
    DatabaseConfigCommon _comm;
    DatabaseRemoteConfig _tcpServer;
    DatabaseRemoteConfig _nngServer;
    DatabaseRemoteConfig _tcpClient;
    DatabaseRemoteConfig _nngClient;
    SqliteLocalConfig _sqlite;
    PostgresqlLocalConfig _postgres;
    RedisLocalConfig _redis;
};

class HttpConfig
{ 
  public:
    string _addr;
    uint16_t _port;
    string _rootHtmlPath;
    string _rootJsonPath;
    string _resRootPath;
    string _wsPath;
};

class TimeZoneConfig
{
public:
    string _defaultTimeZone{ "Asia/Shanghai"};
};

class NanoRepConf
{
  public:
    string _addr;
    uint16_t _port;
};

class TransportConfig
{
  public:
    string _addr;
    uint16_t _port;
    uint16_t _timeout=5000;
};

class LogRemoteConfig
{
  public:
    string _ip;
    uint16_t _port;
};

class LogContextConfig
{
  public:
    string _format;
    int _level;
    string _file;
    string _filepath;
    uint64_t _fileRollSize;
    int rollfilecount;
};

class LogConfig
{
  public:
    LogRemoteConfig _remote;
    LogContextConfig _context;
};

class JWTConf
{
public:
    string _issuer;
    string _type;
    string _key;
    string _superuser;
    string _superpass;
    tsl::htrie_set<char> _noAuthRouter;
};

