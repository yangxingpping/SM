#pragma once

#include "databaseExport.h"
#include "../networkinterface/nngs/RepManager.h"
#include <string>
#include <memory>
#include <stdint.h>

using std::string;
using std::shared_ptr;

namespace SMDB
{
	class  DBNNg
	{
	public:
		DBNNg(string ip, uint16_t port);
		void init();
	private:
		string _addr;
		uint16_t _port;
		shared_ptr<SMNetwork::RepManager<ChannelModeC::Initiative, MainCmd>> _sock;
	};
}
