
#include "DBTcp.h"

#include "PackDealerMainSub.h"

namespace SMDB
{

	DBTcp::DBTcp(string ip, uint16_t port):_addr(ip), _port(port), _tcp(nullptr)
	{

	}

	void DBTcp::init()
	{
		shared_ptr<SMNetwork::PackDealerBase> ps = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerMainSub(MainCmd::DBQuery, ChannelType::DBServer));
		_tcp = make_shared<SMNetwork::TcpServer<ChannelType::DBServer, NetHeadType::FixPackLenPlaceHolder, ChannelIsInitiative::Passive>>(_addr, _port);
		_tcp->init(shared_ptr<SMNetwork::PackDealerBase>(ps->clone()));
	}

	void DBTcp::stop()
	{
		SPDLOG_INFO("try stop db tcp server instance");
		if (_tcp)
		{
			_tcp->stop();
		}
		else
		{
			SPDLOG_WARN("db tcp server instance is null");
		}
	}

	bool DBTcp::stoped()
	{
		if (_tcp)
		{
			return _tcp->stoped();
		}
		return true;
	}

}
