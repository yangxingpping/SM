#pragma once
#include "networkinterfaceExport.h"
//#include "TcpChannel.h"
#include "hotupdate.h"

#include "concurrentqueue/concurrentqueue.h"

#include "PackDealerBase.h"

#include "IOContextManager.h"
#include "TcpChannel.h"
#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;
using std::string;

namespace SMNetwork
{

template <ChannelType t, NetHeadType nethead, ChannelIsInitiative isInitiative>
class TcpServer
{
  public:
    TcpServer(std::string addr, uint16_t port)
        : _ip(addr)
        , _port(port)
        ,_brun(false)
        ,_wantstop(false)
    {
    }

    ChannelIsInitiative IsInitiativeChannel()
    {
        return isInitiative;
    }

    asio::awaitable<void> _listen(shared_ptr<PackDealerBase> packdealer)
    {
        string addr = _ip + ":" + std::to_string(_port);
        SPDLOG_INFO("channel {} in listen func, port {}", magic_enum::enum_name(t),  addr);
        auto executor = co_await asio::this_coro::executor; 
        _acc = std::make_shared<tcp::acceptor>(executor, asio::ip::tcp::endpoint{ tcp::v4(), _port });
        while(!_wantstop)
        {
            _brun = true;
            BEGIN_ASIO;
			tcp::socket sock = co_await _acc->async_accept(asio::use_awaitable);
			shared_ptr<tcp::socket> pt = std::make_shared<tcp::socket>(std::move(sock));
			shared_ptr<TcpChannel<t, nethead> > channel = std::make_shared<TcpChannel<t, nethead> >(pt);
			SPDLOG_INFO("one client connect to server, full address {}", channel->getFullAddr());
			auto pd = shared_ptr<PackDealerBase>(packdealer->clone());
			asio::co_spawn(*IOCTX, channel->_serverEntry(channel, pd), asio::detached);
            END_ASIO;
        }
        _brun = false;
    }
    void init(shared_ptr<PackDealerBase> packdealer)
    {
        asio::co_spawn(*IOCTX, _listen(packdealer), asio::detached);
    }

    void stop()
    {
        if (_acc)
        {
            _wantstop = true;
			_acctimer = std::make_unique<asio::steady_timer>(*IOCTX);
			_acctimer->expires_after(std::chrono::microseconds(10));
			_acctimer->async_wait([this](const asio::system_error& e) {
				SPDLOG_INFO("asyn accept from accepter failed");
				_acc->cancel();
				});
        }
    }

    
    shared_ptr<TcpChannel<t, nethead>> getChannel() {
        shared_ptr<TcpChannel< t, nethead>> ret;
        _channels.try_dequeue(ret);
        return ret;
    }

    bool stoped()
    {
        return !_brun;
    }

    private:
    moodycamel::ConcurrentQueue<shared_ptr<TcpChannel<t, nethead>>> _channels;
    moodycamel::ConcurrentQueue<shared_ptr<TcpChannel<t, nethead>>> _closingchannels;
    string _ip{"invalid"};
    uint16_t _port{0};
    std::shared_ptr<tcp::acceptor> _acc{nullptr};
    std::unique_ptr<asio::steady_timer> _acctimer{ nullptr };
    bool _brun{ false };
    bool _wantstop{ false };
};
}
