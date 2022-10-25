
#include "DBNNg.h"

#include "enums.h"

#include "PackDealerMainSub.h"

namespace SMDB
{

	DBNNg::DBNNg(string ip, uint16_t port):_addr(ip), _port(port), _sock(nullptr)
	{

	}

	void DBNNg::init()
	{
		shared_ptr<SMNetwork::PackDealerBase> ps = shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerMainSub(MainCmd::DBQuery, ChannelType::DBServer));
		_sock = make_shared<SMNetwork::AsynRep>(_addr, _port, ChannelType::DBServer);
		_sock->init(ServeMode::SBind, ps);
	}
}
