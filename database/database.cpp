#include "database.h"
#include "spdlog/spdlog.h"
#include <string>

#include "DealAuth.h"
#include "DealEcho.h"

#include "PackDealerMainSub.h"
#include "PackDealerNoHead.h"
#include "PackDealerNoMain.h"

#include "AsynRep.h"
#include "SyncReq.h"

#include "DBs.h"
#include "conf.h"

#include <map>

using std::make_shared;
using std::shared_ptr;
using std::map;


namespace SMDB
{
    static map<BeforeQueryFuncType*, shared_ptr<BeforeQueryFuncType>> _before;
    static map<AfterQueryFuncType*, shared_ptr<AfterQueryFuncType>> _after;
    static const date::time_zone* _defaultTZ{ nullptr };
    static string _strDefaultTZ;

     bool  beforeQuery(string_view token)
    {
        SPDLOG_INFO("before query call");
        bool bret = true;
        for(auto it : _before)
        {
            bret = (*(it.second))(token);
            if (!bret)
            {
                SPDLOG_WARN("the before query check failed");
                break;
            }
        }
        return bret;
    }
     void  afterQuery()
    {
        SPDLOG_INFO("after query call");
        for(auto it: _after)
        {
            (*(it.second))();
        }
    }

    bool  setDefaultTimeZone(string tz)
	{
        bool bret{ false };
        BEGIN_STD;
        _defaultTZ = date::locate_zone(tz);
        bret = true;
        _strDefaultTZ = tz;
        END_STD;
        if (!bret)
        {
           SPDLOG_ERROR("date get locate_zone {} failed", tz);
        }
        return bret;
	}

     const string& getDefaultStrTimeZone()
	{
        return _strDefaultTZ;
	}

	const date::time_zone* getDefaultTimeZone()
	{
        if (!_defaultTZ)
        {
            SPDLOG_ERROR("no default time zone");
        }
        assert(_defaultTZ);
        return _defaultTZ;
	}

	void  addBeforeQuery(shared_ptr<BeforeQueryFuncType> func)
    {
        _before.insert({ func.get(), func });
    }
    void  addAfterQuery(shared_ptr<AfterQueryFuncType> func)
    {
        _after.insert({ func.get(), func });
    }

	void init(bool dbnode/* = false*/)
	{
        auto conftz = SMCONF::getTimeZoneConfig();
        assert(setDefaultTimeZone(conftz->_defaultTimeZone));
        
        if (dbnode)
        {
            DBs::getInst().init();
        }
	}

	void asyn_nng_demo()
	{
        auto serverfunc = std::make_shared<RouterFuncType>([](std::string& req, string token)->asio::awaitable<RouterFuncReturnType>
            {
                RouterFuncReturnType ret = std::make_shared<string>(string("fuck"));

				co_return ret;
			});

		uint16_t portx = 998;

		SMCONF::addRouterTrans(MainCmd::MainCmdBegin, 0, serverfunc);

		shared_ptr<SMNetwork::PackDealerBase> ps = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoServer));
		shared_ptr<SMNetwork::PackDealerBase> pc = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerNoHead(ChannelType::EchoClient));

		shared_ptr<SMNetwork::AsynRep> rep =  make_shared<SMNetwork::AsynRep>("127.0.0.1", portx, ChannelType::EchoServer);
		rep->init(ServeMode::SBind, ps);

        shared_ptr < SMNetwork::SyncReq> req = make_shared<SMNetwork::SyncReq>("127.0.0.1", portx, ChannelType::EchoClient);
		req->init(ServeMode::SConnect, pc);

		std::string strreq{ "fuck.world" };
		auto strrep = req->reqrep(strreq, 0);
		int i = 1;
	}

}