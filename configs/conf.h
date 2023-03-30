#pragma once
#include "configsExport.h"
#include "Structs.h"
#include <stdint.h>
#include <stdbool.h>
namespace SMCONF
{
	CONFIGS_EXPORT bool sInit(const char* path);
	CONFIGS_EXPORT DatabaseConfig* getDBConfig();
	CONFIGS_EXPORT void setDBConfig(const DatabaseConfig* conf);
	CONFIGS_EXPORT HttpConfig* getHttpConfig();
	CONFIGS_EXPORT LogConfig* getLogConfig();
	CONFIGS_EXPORT TransportConfig* getTransportConfig();
	CONFIGS_EXPORT NanoRepConf* getNngRepConfig();
	CONFIGS_EXPORT JWTConf* getJWTConf();
	CONFIGS_EXPORT TimeZoneConfig* getTimeZoneConfig();
	CONFIGS_EXPORT bool isRouterNeedJwt(string_view url);
}
