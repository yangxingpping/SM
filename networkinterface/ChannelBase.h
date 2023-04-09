
#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"

#include <memory>
#include <tuple>
#include <string_view>
#include <stdint.h>

using std::tuple;
using std::string_view;
using std::shared_ptr;
using std::string;
using std::make_tuple;
using std::make_shared;

namespace SMNetwork
{
	enum class ChannelNet
	{
		
	};

	class NETWORKINTERFACE_EXPORT ChannelCommon
	{
	public:
		virtual uint32_t sockNo()
		{
			assert(0);
			return 0;
		}
		virtual void setSockNo(uint32_t sockno)
		{
			(void)(sockno);
			assert(0);
		}
	};

	class NETWORKINTERFACE_EXPORT ChannelSend
	{
		virtual asio::awaitable<bool> sendPack(string_view src, uint32_t no)
		{
			assert(0);
			co_return false;
		}
	};

	class NETWORKINTERFACE_EXPORT ChannelRecv
	{
		virtual asio::awaitable<tuple<uint32_t, shared_ptr<string>>> recvPack()
		{
			assert(0);
			co_return make_tuple(0, make_shared<string>(""));
		}
	};
}
