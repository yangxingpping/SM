
#include "Utils.h"
#include "DBs.h"
#include "enums.h"
#include "DBManager.h"
#include "spdlog/spdlog.h"

#include "PackDealerMainSub.h"
#include "TcpServer.h"
#include "AsynRep.h"
#include "DBNNg.h"
#include "DBTcp.h"

#include <string>

using std::make_shared;
using std::shared_ptr;

using std::string;
using std::string_view;



namespace SMDB
{
    static shared_ptr<DBs> _inst = { nullptr };
    static shared_ptr<SMNetwork::TcpServer<ChannelType::RDBServer, NetHeadType::FixPackLenPlaceHolder, ChannelIsInitiative::Passive>> _rtcp = { nullptr };
    static shared_ptr<SMNetwork::AsynRep> _rnng = { nullptr };

    static shared_ptr<SMNetwork::PackDealerBase> _dealer = { nullptr };

    static shared_ptr<DBNNg> _nng = { nullptr };
    static shared_ptr<DBTcp> _tcp = { nullptr };
	DBs& DBs::getInst()
	{
        if (!_inst)
        {
            _inst = make_shared<DBs>();
        }
        return *_inst;
	}

DBs::DBs()
{
    // may be need split another function.
    _dealer = std::shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerMainSub(MainCmd::DBQuery, ChannelType::DBServer));
}

void DBs::init()
{
	auto conftcp = CONFINST.getDBConfig()._tcpServer;
    _tcp = make_shared<DBTcp>(conftcp._ip, conftcp._port);
    _tcp->init();

    auto confnng = CONFINST.getDBConfig()._nngServer;
    _nng = make_shared<DBNNg>(confnng._ip, confnng._port);
    _nng->init();
}

}
