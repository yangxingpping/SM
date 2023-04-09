
#include "DBNNg.h"
#include "enums.h"

namespace SMDB
{

	DBNNg::DBNNg(string ip, uint16_t port):_addr(ip), _port(port)//,_sock(nullptr)
	{
	}

	void DBNNg::init()
	{
		_sock = make_shared<SMNetwork::RepManager<ChannelModeC::Initiative, MainCmd>>(_addr, _port, MainCmd::DBQuery);
		bool bret{ false };
		bret = _sock->init();
		assert(bret);
		if (!bret)
		{
			SPDLOG_WARN("init database nng rep failed on ip {}, port {}", _addr, _port);
		}
	}
}
