#include "Utils.h"
#include "templatefuncs.h"
#include "Configs.h"
#include "toml.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

using std::make_unique;
using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;

namespace SMCONF
{

    static  unique_ptr<Configs> _inst = nullptr;


    void _ParseHttp(const toml::value& root)
    {
        auto& _http = _inst->getHttpConfig();
        _http._addr = toml::find<string>(root, "addr");
        _http._port = toml::find<uint16_t>(root, "port");
        _http._resRootPath = toml::find<string>(root, "resRootPath");
        _http._rootJsonPath = toml::find<string>(root, "rootJsonPath");
        _http._rootHtmlPath = toml::find<string>(root, "rootHtmlPath");
        _http._wsPath = toml::find<string>(root, "wsPath");
        _http._keyPath = toml::find<string>(root, "keyName");
        _http._perPath = toml::find<string>(root, "pemName");
    }

    void _ParseTimeZoneConf(const toml::value& root)
    {
        auto& _tzconf = _inst->getTimeZoneConfig();
        _tzconf._defaultTimeZone = toml::find<string>(root, "defaultTimeZone");
    }

    void _ParseDatabase(const toml::value& dbmgr)
    {
        auto& _dbconf = _inst->getDBConfig();
        _dbconf._comm.type =
            (DatabaseType)(toml::find<int>(dbmgr, "databasetype"));
        _dbconf._comm._model =
            (DatabaseMode)(toml::find<int>(dbmgr, "databasemodel"));
        _dbconf._comm._debug = toml::find<bool>(dbmgr, "databasedebug");

        switch (_dbconf._comm.type)
        {
        case DatabaseType::Postgresql:
        {
            const auto& pgnode = toml::find(dbmgr, "postgresql");
            auto& localpg = _inst->_dbconf._postgres;
            localpg._addr = toml::find<string>(pgnode, "addr");
            localpg._port = toml::find<uint16_t>(pgnode, "port");
            localpg._dbname = toml::find<string>(pgnode, "dbname");
            localpg.username = toml::find<string>(pgnode, "user");
            localpg.password = toml::find<string>(pgnode, "pass");
        }
        break;
        case DatabaseType::Sqlite:
        {
            const auto& sqlitenode = toml::find(dbmgr, "sqlite");
            auto& localsqlite = _inst->_dbconf._sqlite;
            localsqlite._file = toml::find<string>(sqlitenode, "sqlitename");
            localsqlite._filepath =
                toml::find<string>(sqlitenode, "sqlitepath");
        }
        break;
        default:
            break;
        }
        {
            const auto& Nremote = toml::find(dbmgr, "tcpserver");
            if (!Nremote.is_empty())
            {
                auto& reconf = _inst->_dbconf._tcpServer;
                reconf._ip = toml::find<string>(Nremote, "addr");
                reconf._port = toml::find<uint16_t>(Nremote, "port");
                reconf._useSSL = (bool)(toml::find<int>(Nremote, "useSSL"));
            }
        }
        {
            const auto& Nremote = toml::find(dbmgr, "nngserver");
            if (!Nremote.is_empty())
            {
                auto& reconf = _inst->_dbconf._nngServer;
                reconf._ip = toml::find<string>(Nremote, "addr");
                reconf._port = toml::find<uint16_t>(Nremote, "port");
                reconf._useSSL = (bool)(toml::find<int>(Nremote, "useSSL"));
            }
        }
        {
            const auto& Nremote = toml::find(dbmgr, "tcpclient");
            if (!Nremote.is_empty())
            {
                auto& reconf = _inst->_dbconf._tcpClient;
                reconf._ip = toml::find<string>(Nremote, "addr");
                reconf._port = toml::find<uint16_t>(Nremote, "port");
                reconf._useSSL = (bool)(toml::find<int>(Nremote, "useSSL"));
            }
        }
        {
            const auto& Nremote = toml::find(dbmgr, "nngclient");
            if (!Nremote.is_empty())
            {
                auto& reconf = _inst->_dbconf._nngClient;
                reconf._ip = toml::find<string>(Nremote, "addr");
                reconf._port = toml::find<uint16_t>(Nremote, "port");
                reconf._useSSL = (bool)(toml::find<int>(Nremote, "useSSL"));
            }
        }
        {
            const auto& redisnode = toml::find(dbmgr, "redis");
            auto& localredis = _inst->_dbconf._redis;
            localredis._addr = toml::find<string>(redisnode, "addr");
            localredis._port = toml::find<uint16_t>(redisnode, "port");
            localredis.username = toml::find<string>(redisnode, "user");
            localredis.password = toml::find<string>(redisnode, "pass");
        }
    }

    void _ParseLog(const toml::value& root)
    {
        {
            const auto& node = toml::find(root, "node");
            auto& ref = _inst->_log._remote;
            ref._ip = toml::find<string>(node, "addr");
            ref._port = toml::find<uint16_t>(node, "port");
        }
        {
            const auto& node = toml::find(root, "context");
            auto& ref = _inst->_log._context;
            ref._format = toml::find<string>(node, "format");
            ref._level = toml::find<int>(node, "level");
            ref._file = toml::find<string>(node, "log_file");
            ref._filepath = toml::find<string>(node, "log_file_path");
            ref._fileRollSize = toml::find<uint64_t>(node, "log_roll_size");
            ref._rollfilecount = toml::find<int>(node, "log_roll_count");
            ref._logger = toml::find<string>(node, "logger_name");
        }
    }

    void _ParseTransport(const toml::value& root)
    {
        auto& _transport = _inst->_transport;
        _transport._addr = toml::find<string>(root, "addr");
        _transport._port = toml::find<uint16_t>(root, "port");
        _transport._timeout = toml::find<uint16_t>(root, "timeout");
        if (_transport._timeout > 60000) //60 seconds
        {
            _transport._timeout = 60000;
        }
    }

    void _ParseNanoTcp(const toml::value& root)
    {
        const auto& node = toml::find(root, "rep");
        auto& ref = _inst->_nanotcp;
        ref._addr = toml::find<string>(node, "addr");
        ref._port = toml::find<uint16_t>(node, "port");

    }

    void _ParseJWTConf(const toml::value& root)
    {
        JWTConf& ref = _inst->_jwt;
        ref._issuer = toml::find<string>(root, "issuer");
        ref._type = toml::find<string>(root, "type");
        ref._key = toml::find<string>(root, "key");
        ref._superuser = toml::find<string>(root, "superuser");
        ref._superpass = toml::find<string>(root, "superpass");
        ref._timeout = toml::find<int>(root, "timeout");
        const auto norouterarr = toml::find<vector<string>>(root, "noAuthUrl");
        for (const auto& i : norouterarr)
        {
            ref._noAuthRouter.insert(i);
        }
    }

	bool Configs::sInit()
	{
        bool bret{ false };
        if (_inst != nullptr)
        {
            return bret;
        }
        _inst = make_unique<Configs>();
        bret = true;
        return bret;
	}

	Configs* Configs::getInst2()
    {
        if (_inst == nullptr)
        {
            _inst = make_unique<Configs>();
        }
        return _inst.get();
    }

    Configs::Configs()
    {

    }

    Configs::~Configs()
    {
    }

    bool Configs::Parse(string path)
    {
        _path = path;
        bool bret = false;
        BEGIN_STD;
        const auto data = toml::parse(_path);
        const auto& dbmgr = toml::find(data, "dbmgr");
        const auto& http = toml::find(data, "http");
        const auto& log = toml::find(data, "log");
        const auto& transport = toml::find(data, "transport");
        const auto& nanotcp = toml::find(data, "nanomsg");
        const auto& jwt = toml::find(data, "jwt");
        const auto& tz = toml::find(data, "timezone");
        _ParseHttp(http);
        _ParseDatabase(dbmgr);
        _ParseLog(log);
        _ParseTransport(transport);
        _ParseNanoTcp(nanotcp);
        _ParseJWTConf(jwt);
        _ParseTimeZoneConf(tz);
        bret = SMUtils::cacheDBValuesNames<AssDB>();
        assert(bret);
        bret = SMUtils::cacheMainValuesNames<MainCmd>();
        assert(bret);
        END_STD;
        return bret;
    }

    DatabaseConfig& Configs::getDBConfig()
    {
        return _dbconf;
    }

    void Configs::setDBConfig(const DatabaseConfig& conf)
    {
        _dbconf = conf;
    }

    HttpConfig& Configs::getHttpConfig()
    {
        return _http;
    }

    LogConfig& Configs::getLogConfig()
    {
        return _log;
    }

    TransportConfig& Configs::getTransportConfig()
    {
        return _transport;
    }

    NanoRepConf& Configs::getNngRepConfig() {
        return _nanotcp;
    }

    JWTConf& Configs::getJWTConf()
    {
        return _jwt;
    }

    TimeZoneConfig& Configs::getTimeZoneConfig()
    {
        return _tzconf;
    }

    bool Configs::isRouterNeedJwt(string_view url)
    {
        bool bret = true;
        bret = _jwt._noAuthRouter.find(url) == _jwt._noAuthRouter.end();
        return bret;
    }

	static LogRemoteConfig _remote;
	static LogContextConfig _context;
	static vector<shared_ptr<spdlog::sinks::sink>> _sinks;
	static shared_ptr<spdlog::logger> _loger = { nullptr };

    bool SInitLog()
    {
        bool bret = true;
        auto conf = SMCONF::Configs::getInst2()->getLogConfig();
        _remote = conf._remote;
        _context = conf._context;

        auto outsink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
        outsink->set_level((spdlog::level::level_enum)_context._level);
        _sinks.push_back(outsink);

        auto rotatesink = make_shared<spdlog::sinks::rotating_file_sink_mt>(_context._filepath + _context._file, _context._fileRollSize, _context._rollfilecount);
        rotatesink->set_level((spdlog::level::level_enum)_context._level);
        _sinks.push_back(rotatesink);

        _loger = shared_ptr<spdlog::logger>(new spdlog::logger(_context._logger, _sinks.begin(), _sinks.end()));
        _loger->set_pattern(_context._format);
        _loger->set_level((spdlog::level::level_enum)_context._level);
        spdlog::set_default_logger(_loger);
        spdlog::default_logger()->flush_on(spdlog::level::info);
        return bret;
    }

	bool SInitLog(string logfile, string loggername)
	{
		SMCONF::Configs::getInst2()->getLogConfig()._context._file = logfile;
		SMCONF::Configs::getInst2()->getLogConfig()._context._logger = loggername;
        return SInitLog();
	}

}
