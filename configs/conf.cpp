
#include "conf.h"
#include "Configs.h"
#include "whereami.h"
namespace SMCONF
{
	bool sInit(const char* path)
	{
		bool bret{ false };

		string exepath;
		exepath.resize(1024);
		int outlen{ 0 };
		auto mypath = wai_getModulePath(exepath.data(), exepath.size(), &outlen);

		if (!path)
		{
			return bret;
		}
		bret = Configs::sInit();
		if (!bret)
		{
			return bret;
		}
		string strpath = path;
		bret = SMCONF::Configs::getInst2().Parse(strpath);
		return bret;
	}

	DatabaseConfig* getDBConfig()
	{
		return &SMCONF::Configs::getInst2().getDBConfig();
	}

	void setDBConfig(const DatabaseConfig* conf)
	{
		assert(conf != nullptr);
		SMCONF::Configs::getInst2().setDBConfig(*conf);
	}

	HttpConfig* getHttpConfig()
	{
		return &SMCONF::Configs::getInst2().getHttpConfig();
	}

	LogConfig* getLogConfig()
	{
		return &SMCONF::Configs::getInst2().getLogConfig();
	}

	TransportConfig* getTransportConfig()
	{
		return &SMCONF::Configs::getInst2().getTransportConfig();
	}

	NanoRepConf* getNngRepConfig()
	{
		return &SMCONF::Configs::getInst2().getNngRepConfig();
	}

	JWTConf* getJWTConf()
	{
		return &SMCONF::Configs::getInst2().getJWTConf();
	}

	TimeZoneConfig* getTimeZoneConfig()
	{
		return &SMCONF::Configs::getInst2().getTimeZoneConfig();
	}

	bool isRouterNeedJwt(string_view url)
	{
		return SMCONF::Configs::getInst2().isRouterNeedJwt(url);
	}

}
