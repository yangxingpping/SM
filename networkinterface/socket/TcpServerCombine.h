#pragma once
#include "networkinterfaceExport.h"
#include "asio/ip/tcp.hpp"
#include "myconcept.h"
#include "hotupdate.h"
#include "ChannelCombine.h"
#include "TcpChannelImp.h"
#include "LocalNetManager.h"
#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;
using std::string;

namespace SMNetwork
{

template <ChannelModeC modelc, EnumConcept MainType>
class TcpServerCombine
{
  public:
      TcpServerCombine(std::string addr, uint16_t port, MainType mainc)
        : _ip(addr)
        , _port(port)
        ,_brun(false)
        ,_wantstop(false)
          ,_mainc(mainc)
    {
          _strmainc = magic_enum::enum_name(_mainc);
    }

    ChannelModeC getMode()
    {
        return modelc;
    }

    asio::awaitable<void> _listen()
    {
        string addr = _ip + ":" + std::to_string(_port);
        SPDLOG_INFO("listen func, port {} for channel {}", addr, _strmainc);
        auto executor = co_await asio::this_coro::executor; 
        _acc = std::make_shared<asio::ip::tcp::acceptor>(executor, asio::ip::tcp::endpoint{ asio::ip::tcp::v4(), _port });
        while(!_wantstop)
        {
            _brun = true;
            BEGIN_ASIO;
			asio::ip::tcp::socket sock = co_await _acc->async_accept(asio::use_awaitable);
            SPDLOG_INFO("listen port {} success get a new connect for channel {}", _port, _strmainc);
            auto channel = LNM->createTcpChannel(make_unique<asio::ip::tcp::socket>(move(sock)), modelc, magic_enum::enum_integer(_mainc));
            assert(channel != nullptr);
            if (channel == nullptr)
            {
                SPDLOG_ERROR("create channel for mainc {} failed", _strmainc);
            }
            else
            {
                channel->start();
            }
            END_ASIO;
        }
        _brun = false;
    }
    void init()
    {
        asio::co_spawn(*IOCTX, _listen(), asio::detached);
    }

    void stop()
    {
        if (_acc)
        {
            _wantstop = true;
            asio::co_spawn(*IOCTX, [this]()->asio::awaitable<void> {

                this->_acc->cancel();
                co_return;
                }, asio::detached);
        }
    }

    bool stoped()
    {
        return !_brun;
    }

private:
    MainType _mainc;
    string _ip{"invalid"};
    uint16_t _port{0};
    std::shared_ptr<asio::ip::tcp::acceptor> _acc{nullptr};
    std::unique_ptr<asio::steady_timer> _acctimer{ nullptr };
    bool _brun{ false };
    bool _wantstop{ false };
    string _strmainc;
};
}
