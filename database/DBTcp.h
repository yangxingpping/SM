#pragma once

#include "databaseExport.h"
#include "enums.h"
#include "../networkinterface/socket/TcpServerCombine.h"
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
		shared_ptr<SMNetwork::TcpServerCombine<ChannelModeC::Initiative, MainCmd>> _tcp;
	};
}
