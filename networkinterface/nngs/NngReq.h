#pragma once
#include "enums.h"
#include "myconcept.h"
#include "ChannelCombine.h"
#include "ReqImpl.h"

#include "concurrentqueue/concurrentqueue.h"

#include <string>
#include <memory>

using std::shared_ptr;
using std::string;

/**
 * class for manage nng request.
 */

namespace SMNetwork
{
	template <ChannelModeC modec, EnumConcept MainType>
	class NngReq
	{
	public:
		NngReq(string addr)
			:_addr(addr)
		{

		}

		asio::awaitable<ChannelCombine<ReqImpl, MainType>*> getChannel()
		{
			ChannelCombine<ReqImpl, MainType>* retchannel = nullptr;
			string addr = _ip + ":" + std::to_string(_port);
			if (_channelpool.empty())
			{
				auto timeoutmillsecond = SMCONF::getTransportConfig()->_timeout;
				if (timeoutmillsecond == 0)
				{
					timeoutmillsecond = 3000;
					SPDLOG_WARN("tcp connect time out time not set, use default value {} channel type {}", timeoutmillsecond, _strmainc);
				}
				int trycount = 0;
				while (++trycount <= 3)
				{
					SPDLOG_INFO("start get tcp connect channel from addr {} channel type {}", addr, _strmainc);
					asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(_ip), _port);
					unique_ptr<asio::ip::tcp::socket> sock = make_unique<asio::ip::tcp::socket>(*IOCTX);
					BEGIN_ASIO;
					co_await sock->async_connect(endpoint, asio::use_awaitable);
					uint32_t sockno = SMNetwork::newSockNo();
					retchannel = LNM->createTcpChannel(move(sock), ChannelModeC::Initiative, MainCmd::DBQuery);
					retchannel->start();
					break;
					END_ASIO;
				}
			}
			else
			{
				retchannel = _channelpool.front();
				_channelpool.pop_front();
			}
			if (retchannel == nullptr)
			{
				SPDLOG_WARN("connect addr [{}] failed channel type {}", addr, _strmainc);
			}
			co_return retchannel;
		}

		ChannelModeC channelType()
		{
			return modec;
		}
	private:
		string _addr;
		moodycamel::ConcurrentQueue<shared_ptr<ChannelCombine<ReqImpl, MainType>>> _channels;
	};
}

