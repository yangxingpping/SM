#include "database.h"
#include "spdlog/spdlog.h"
#include <string>

#include "DealAuth.h"
#include "DealEcho.h"
#include "../networkinterface/nngs/AsynRep.h"
#include "../networkinterface/nngs/SyncReq.h"
#include "dealers/MainAssPlatPack.h"
#include "networkinterface.h"
#include "PackUnpackManager.h"

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
        auto dealer = shared_ptr<SMNetwork::PlatformPackInterface>(new SMNetwork::MainAssPlatPack(magic_enum::enum_integer(MainCmd::DBQuery)));
        assert(SMNetwork::addPlatformDealer(dealer));
        if (dbnode)
        {
            DBs::getInst().init();
        }
	}

}