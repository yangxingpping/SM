#pragma once
#include "networkinterfaceExport.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "asio/io_context.hpp"
#include "asio/co_spawn.hpp"
#include "hotupdate.h"
#include "asio/deferred.hpp"
#include "asio/detached.hpp"
#include "asio/experimental/concurrent_channel.hpp"
#include "taskflow/taskflow.hpp"
#include "magic_enum.hpp"
#include "enums.h"
#include "Routers.h"
#include "spdlog/spdlog.h"
#include "MessageRouter.h"
#include "myconcept.h"
#include "ChannelManager.h"
#include "myconcept.h"
#include "magic_enum.hpp"
#include <memory>
#include <set>
#include <vector>
#include <stdbool.h>
#include <string_view>
#include <array>
#include <atomic>
#include <numeric>
#include <stdbool.h>
#include <map>
using std::shared_ptr;
using std::vector;
using std::string;
using std::array;
using std::unique_ptr;
using std::enable_shared_from_this;
using std::atomic_uint32_t;
using std::set;
using std::numeric_limits;
using std::make_unique;
using std::move;
using std::array;
using std::map;
using std::make_shared;
using std::enable_shared_from_this;
using asio::experimental::concurrent_channel;

namespace SMNetwork
{
/**
 * @brief
 * 
 * @tparam NetImpl 
 */
template<NetInterface NetImpl, EnumConcept MainType>
class ChannelCombine
{
  public:
      ChannelCombine(shared_ptr<NetImpl> sock, ChannelModeC cmode, MainType maincmd)
          :_sock(sock), _cmode(cmode), _mainc(maincmd)
    {
          switch (_cmode)
          {
          case ChannelModeC::Initiative:
          {
              _reqNo = 0;
          }break;
          case ChannelModeC::Passive:
          {
              _reqNo = std::numeric_limits<uint32_t>::max() / 2;
          }break;
          default:
              break;
          }
    }
    ChannelCombine(const ChannelCombine& copy) = delete;
    ~ChannelCombine()
    {
        
    }
    void start()
    {
        startIO();
        startApp();
    }

    //start io
    void startIO()
    {
		//start recv data from network
		asio::co_spawn(*IOCTX, [this](decltype(this) hh)->asio::awaitable<void> {
            SPDLOG_INFO("start coro recv pack from network for sock {}", hh->_sock->sockNo());
			while (hh->_brun)
			{
				auto [no, msg] = co_await hh->_sock->recvPack();
                shared_ptr<NMessage> packed = make_shared<NMessage>(no, msg);
                //first whether this is an app module waiting for this message recv from network
                if(!notifyNetRepComplete(hh->_sock->sockNo(), packed))
                { //spawn coro to call App dealer and send back rep
                    asio::co_spawn(*IOCTX, [this](shared_ptr<NMessage> msg, decltype(hh) h2)->asio::awaitable<void> {
                        auto recvAppRep = prepareAppRep(h2->_sock->sockNo(), msg->No());
                        assert(recvAppRep != nullptr);
                        BEGIN_ASIO;
                        co_await addPackFromIo(h2->_sock->sockNo(), magic_enum::enum_integer(h2->_mainc), msg);
                        co_await recvAppRep->async_wait(asio::deferred);
                        auto rep = recvAppRep->get();
                        if (rep)
                        {
							auto bflag = co_await h2->_sock->sendPack(string_view(rep->body()->begin(), rep->body()->end()), rep->No());
							if (!bflag)
							{
								SPDLOG_WARN("send rep for msg No.{} failed", rep->No());
							}
                        }
                        END_ASIO;
                        co_return;
                        }(packed, hh), asio::detached);
                }
			}
            SPDLOG_INFO("channel for sock {} stop recv data from network", hh->_sock->sockNo());
			co_return;
			}(this), asio::detached);
    }

    //start app
    void startApp()
    {
       
    }

    asio::awaitable<bool> sendMessage(shared_ptr<NMessage> msg)
    {
		auto body = msg->body();
		auto bsucc = co_await _sock->sendPack(string_view(body->begin(), body->end()), msg->No());
        co_return bsucc;
    }

    //called by app
    asio::awaitable<shared_ptr<string>> reqrep(shared_ptr<string> req)
    {
        shared_ptr<string> ret{nullptr};
        auto no = newReq();
        auto recvers = SMNetwork::prepareReqRep(_sock->sockNo(), no);
        assert(recvers._valid);
        auto msg = make_shared<SMNetwork::NMessage>(no, req);
        co_await addPackFromApp(_sock->sockNo(), msg);
        co_await recvers._r1.async_wait(asio::deferred);
        auto bflag = recvers._r1.get();
        if (!bflag)
        {
            co_return ret;
        }
        //start wait rep
        co_await recvers._r2.async_wait(asio::deferred);
        auto vv = recvers._r2.get();
        if (vv == nullptr)
        {
            SPDLOG_WARN("req msg No.{} rep failed", no);
        }
        ret = vv->body();
        co_return ret;
    }

    asio::awaitable<bool> stop()
    {
        auto strrep = co_await reqrep(make_shared<string>("A"));
        assert(strrep != nullptr);
        if (strrep == nullptr)
        {
            SPDLOG_WARN("request stop return with error");
            co_return false;
        }
        co_return !strrep->empty();
    }

    uint32_t newReq()
    {
        uint32_t ret{ 0 };
        ++_reqNo;
        switch (_cmode)
        {
        case ChannelModeC::Initiative:
        {
            if (_reqNo >= std::numeric_limits<uint32_t>::max())
            {
                _reqNo = 0;
            }
        }break;
        case ChannelModeC::Passive:
        {
            if (_reqNo <= std::numeric_limits<uint32_t>::max()/2)
            {
                _reqNo = std::numeric_limits<uint32_t>::max() / 2;
            }
        }break;
        default:
        {
            assert(0);
        }break;
        }
        ret = _reqNo;
        return ret;
    }

    int MainCmd()
    {
        return magic_enum::enum_integer(_mainc);
    }

    shared_ptr<NetImpl> _sock{ nullptr };
    ChannelModeC _cmode;
    atomic_uint32_t _reqNo{ 0 };
    bool _brun{ true };
    MainType _mainc;
};

};
