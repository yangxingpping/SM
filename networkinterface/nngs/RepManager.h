#pragma once
#include "networkinterfaceExport.h"
#include "nng/nng.h"
#include "myconcept.h"
#include "hotupdate.h"
#include "nng/protocol/reqrep0/rep.h"
#include "ChannelCombine.h"
#include "LocalNetManager.h"
#include <memory>
#include <vector>
#include <set>
#include <exception>

using std::shared_ptr;
using std::vector;
using std::string;
using std::set;

namespace SMNetwork
{

	template <ChannelModeC modelc, EnumConcept MainType>
	class RepManager
	{
	public:
		RepManager(string ip, uint16_t port, MainType mainc, uint16_t cur=1)
			:_ip(ip)
			,_port(port)
			,_cur(cur)
			,_mainc(mainc)
		{
			assert(_port != 0);
		}
		bool init()
		{
			_addr = fmt::format("tcp://{}:{}", _ip, _port);
			auto bret = LNM->createRep(string_view(_addr), ChannelModeC::Passive, magic_enum::enum_integer(_mainc), _cur);
			return bret;
		}
	private:
		string _ip;
		uint16_t _port{ 0 };
		string _addr{};
		uint16_t _cur{ 1024 };
		nng_socket _nng;
		MainType _mainc;
	};
}
