#pragma once
#include <list>
#include "ChannelCombine.h"
#include "TcpChannelImp.h"
#include "conf.h"
#include "asio/redirect_error.hpp"
#include "asio/awaitable.hpp"
#include "asio/error.hpp"
#include "LocalNetManager.h"
#include "coros.h"
#include "enums.h"
#include "myconcept.h"
#include <memory>
#include <vector>
#include "ChannelManager.h"

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::list;
using asio::use_awaitable;
using asio::redirect_error;

namespace SMNetwork
{

	template <ChannelModeC modec, EnumConcept MainType>
	class TcpClientCombine
	{
	public:
		TcpClientCombine(std::string addr, uint16_t port, MainType mainc)
			: _ip(addr)
			, _port(port)
			, _mainc(mainc)
		{
			_strmainc = magic_enum::enum_name(_mainc);
		}

		ChannelModeC channelMode()
		{
			return modec;
		}

		asio::awaitable<ChannelCombine<TcpChannelImpl>*> getChannel()
		{
			ChannelCombine<TcpChannelImpl>* retchannel = nullptr;
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
					retchannel = LNM->createTcpChannel(move(sock), ChannelModeC::Initiative, magic_enum::enum_integer(MainCmd::DBQuery));
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

		void _returnChannel(ChannelCombine<SMNetwork::TcpChannelImpl>* ch)
		{
			if (ch)
			{
				_channelpool.push_back(ch);
			}
		}
	private:
		string _ip;
		uint16_t _port;
		list<ChannelCombine<TcpChannelImpl>*> _channelpool;
		asio::ip::tcp::resolver::results_type _endpoints;
		shared_ptr<asio::steady_timer> _reconnTimer{nullptr};
		bool _bstop{ false };
		MainType _mainc;
		string _strmainc;
	};

}
