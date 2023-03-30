#pragma once

#include "configsExport.h"

#include "Structs.h"



namespace SMCONF
{

	CONFIGS_EXPORT bool SInitLog(string logfile, string loggername);
	CONFIGS_EXPORT bool SInitLog();
	class CONFIGS_EXPORT Configs
	{
	public:
		static bool sInit();
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
}

#define CONFINST (SMCONF::Configs::getInst2())
