
#include "DBTcp.h"

using std::make_shared;

namespace SMDB
{

	DBTcp::DBTcp(string ip, uint16_t port):_addr(ip), _port(port), _tcp(nullptr)
	{

	}

	void DBTcp::init()
	{
		_tcp = make_shared<SMNetwork::TcpServerCombine<ChannelModeC::Initiative, MainCmd>>(_addr, _port, MainCmd::DBQuery);
		_tcp->init();
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
