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
      DatabaseRemoteConfig(uint16_t port, string ip="127.0.0.1", bool usessl = false) :_ip(ip), _port(port), _useSSL(usessl)
      {
	      
      }
    string _ip;
    uint16_t _port;
    bool _useSSL;
};

class SqliteLocalConfig
{
public:
    string _file{"db.db"};
    string _filepath{"./"};
};

class PostgresqlLocalConfig
{
public:
    string _dbname{"demo"};
    bool _useSSL{false};
    string _addr{"127.0.0.1"};
    uint16_t _port{5432};
    string username{"postgres"};
    string password{"123456"};
};

class RedisLocalConfig
{
public:
    bool _useSSL{false};
    string _addr{"127.0.0.1"};
    uint16_t _port{6379};
    string username{""};
    string password{""};
} ;

class DatabaseConfigCommon
{
  public:
    DatabaseType type{ DatabaseType::Sqlite};
    DatabaseMode _model{ DatabaseMode::Embed};
    bool _debug{false};
};


class DatabaseConfig
{
  public:
    DatabaseConfig():_tcpServer(5400), _nngServer(5401, "*"), _tcpClient(5400), _nngClient(5401)
    {
	    
    }
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
    string _addr = "127.0.0.1";
    uint16_t _port{80};
    string _rootHtmlPath{"http"};
    string _rootJsonPath{"http"};
    string _resRootPath{"dist"};
    string _wsPath{"ws"};
    string _perPath{ "" };
    string _keyPath{ "" };
};

class TimeZoneConfig
{
public:
    string _defaultTimeZone{ "Asia/Shanghai"};
};

class NanoRepConf
{
  public:
    string _addr{"127.0.0.1"};
    uint16_t _port{9001};
};

class TransportConfig
{
  public:
    string _addr{"127.0.0.1"};
    uint16_t _port{10087};
    uint16_t _timeout{3600};
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
    string _format{"[%H:%M:%S %z] [%n] [%@] [%^%L%$] [%!] [thread %t] %v"};
    int _level{1};
    string _file{"log.log"};
    string _filepath{"./"};
    string _logger{ "logername" };
    uint64_t _fileRollSize{1000000000};
    int _rollfilecount{10};
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
    string _issuer{"alqaz"};
    string _type{"JWS"};
    string _key{"alqaz"};
    string _superuser{"alqaz"};
    string _superpass{"123456"};
    int _timeout{ 60 };
    tsl::htrie_set<char> _noAuthRouter;
};

