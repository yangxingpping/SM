#pragma once
#include "concurrentqueue/concurrentqueue.h"
#include "hotupdate.h"
#include "TcpChannel.h"
#include "MessageSplitFuncs.h"
#include "asio/redirect_error.hpp"
#include "asio/awaitable.hpp"
#include "asio/error.hpp"
#include "conf.h"

using asio::use_awaitable;
using asio::redirect_error;


#include "coros.h"
#include "enums.h"

#include <memory>
#include <vector>

#include "ChannelManager.h"

#include "ChannelManager.h"

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::vector;

/*
template <MessageDealer T, ChannelType t>
class TcpClient;
*/

namespace SMNetwork
{

template <ChannelType t, NetHeadType nethead, ChannelIsInitiative isInitiative>
class TcpClient
{
  public:
    TcpClient(std::string addr, uint16_t port)
        : _ip(addr)
        , _port(port)
    {
    }

    ChannelIsInitiative IsInitiativeChannel()
    {
        return isInitiative;
    }


    asio::awaitable<void> start(shared_ptr<PackDealerBase> packdealer) 
    {
        while(!_bstop)
        {
            BEGIN_ASIO;
            auto channel = co_await getChannel();
            if (channel != nullptr)
            {
                co_await channel->_serverEntry(channel, packdealer);
            }
            END_ASIO;
        }
        co_return;
    }

    asio::awaitable<shared_ptr<TcpChannel<t, nethead>>> getChannel()
    {
        shared_ptr<TcpChannel<t, nethead>> retchannel;
        string addr = _ip + ":" + std::to_string(_port);
        if (!_channelpool.try_dequeue(retchannel))
        {
            auto timeoutmillsecond = SMCONF::getTransportConfig()->_timeout;
            if (timeoutmillsecond == 0)
            {
                timeoutmillsecond = 3000;
                SPDLOG_WARN("tcp connect time out time not set, use default value {}", timeoutmillsecond);
            }
            int trycount = 0;
            while(++trycount<=3)
            {
				SPDLOG_INFO("start get tcp connect channel {} from addr {}", magic_enum::enum_name(t), addr);
				asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(_ip), _port);
				shared_ptr<asio::ip::tcp::socket> sock = make_shared<asio::ip::tcp::socket>(*IOCTX);
                BEGIN_ASIO;
				co_await sock->async_connect(endpoint, asio::use_awaitable);
				retchannel = make_shared<TcpChannel<t, nethead>>(sock);
				break;
                END_ASIO;
            }
        }
        if (retchannel == nullptr)
        {
            SPDLOG_WARN("connect addr [{}] failed", addr);
        }
        co_return retchannel;
    }

    void _returnChannel(shared_ptr<TcpChannel<t, nethead>> ch)
    {
        if(ch)
        _channelpool.enqueue(ch);
    }

    private:
    string _ip;
    uint16_t _port;
    moodycamel::ConcurrentQueue<shared_ptr<TcpChannel<t, nethead>>> _channelpool;

    asio::ip::tcp::resolver::results_type _endpoints;

    shared_ptr<asio::steady_timer> _reconnTimer = nullptr;

    bool _bstop = false;
};

}
