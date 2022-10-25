#pragma once

#include "databaseExport.h"
#include "enums.h"
#include "TcpServer.h"
#include <string>
#include <memory>


using std::string;
using std::shared_ptr;

namespace SMDB
{
	class DATABASE_EXPORT DBTcp
	{
	public:
		DBTcp(string ip, uint16_t port);
		void init();
		void stop();
		bool stoped();
	private:
		string _addr;
		uint16_t _port;
		shared_ptr<SMNetwork::TcpServer<ChannelType::DBServer, NetHeadType::FixPackLenPlaceHolder, ChannelIsInitiative::Passive>> _tcp;
	};
}
