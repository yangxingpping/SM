#pragma once

#include "configsExport.h"

#include "Structs.h"

#define CONFINST (Configs::getInst2())

class CONFIGS_EXPORT Configs
{
  public:
    static Configs& getInst2();
    Configs();
    ~Configs();
    bool Parse(string path);
     DatabaseConfig& getDBConfig();
    void setDBConfig(const DatabaseConfig& conf);
     HttpConfig& getHttpConfig();
     LogConfig& getLogConfig();
     TransportConfig& getTransportConfig();
     NanoRepConf& getNngRepConfig();
     JWTConf& getJWTConf();
     TimeZoneConfig& getTimeZoneConfig();
     bool isRouterNeedJwt(string_view url);

    //protected:
    
  //private:
    NanoRepConf _nanotcp;
    DatabaseConfig _dbconf;
    HttpConfig _http; 
    LogConfig _log;
    TransportConfig _transport;
    JWTConf _jwt;
    string _path;
    TimeZoneConfig _tzconf;
};

