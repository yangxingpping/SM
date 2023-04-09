#pragma once
#include "networkinterfaceExport.h"
#include "TcpStreamHandler.h"
#include "MesageHandlerBase.h"
#include "../inc/myconcept.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "taskflow/taskflow.hpp"
#include "magic_enum.hpp"
#include "enums.h"
#include "PackDealerBase.h"
#include "spdlog/spdlog.h"

#include "ChannelManager.h"

enum class ChannelMode
{
    Req,
    Rep,
    Pub,
    Sub,
};

#include <memory>
#include <vector>
#include <string_view>
using std::shared_ptr;
using std::vector;
using std::string;

namespace SMNetwork
{

template <ChannelType t, NetHeadType nethead>
class TcpChannel : public TcpStreamHandler
{
  public:
      TcpChannel(shared_ptr<asio::ip::tcp::socket> sock)
          : TcpStreamHandler(sock, nethead)
    {
    }

    ~TcpChannel(){
        
    }

    asio::awaitable<std::string*> clientEntry(shared_ptr<PackDealerBase> packdealer, std::string& req)
    {
        _resp.clear();
        *(packdealer->reqSeq()) = SMUtils::getSeqNum();
        std::string* pack = packdealer->pack(packdealer->reqSeq(), string_view(req), packdealer->getAssc());
        bool succ{ false };
        SPDLOG_INFO("start the [{}] req {}", *(packdealer->reqSeq()), req);
		BEGIN_ASIO;
        succ = co_await sendPack(*pack);
        END_ASIO;
		if (succ)
		{
			auto [bsucc, recvd] = co_await recvPack(3);
            SPDLOG_INFO("success get tcp response from server, resp len {}", recvd->length());
			succ = bsucc;
			if (bsucc)
			{
				string msgrep;
                auto rep = packdealer->unpack(packdealer->repSeq(), *recvd);
                if (*packdealer->reqSeq() != *packdealer->repSeq())
                {
                    SPDLOG_WARN("req seq {} != rep seq {}", *packdealer->reqSeq(), *packdealer->repSeq());
                }
                _resp.insert(_resp.end(), rep->begin(), rep->end());
			}
		}
        if (_resp.empty())
        {
            SPDLOG_WARN("send req [{}] get rep empty", req);
        }
        co_return &_resp;
    }

    asio::awaitable<void> _serverEntry(shared_ptr<TcpChannel> cc, shared_ptr<PackDealerBase> packdealer)
    {
        SPDLOG_INFO("channel type {}, address info {}", magic_enum::enum_name(t), getFullAddr());
        bool bsucc = true;
        while(bsucc)
        {
            BEGIN_ASIO;
            auto [bs, recvreq] = co_await recvPack(60);
            if(!bs)
            {
                SPDLOG_WARN("recv data from channel {} packtype {} failed", magic_enum::enum_name(t), magic_enum::enum_name(nethead));
                break;
            }
            //process recv request, generate response, send back
            auto rep = co_await packdealer->dealmsg(*recvreq);
            if (!rep || rep->empty())
            {
                SPDLOG_WARN("deal data from channel {} packtype {} response 0", magic_enum::enum_name(t), magic_enum::enum_name(nethead));
                break;
            }
            bsucc = co_await sendPack(*rep);
            END_ASIO;
            if (!bsucc)
            {
                SPDLOG_WARN("send data for channel {} packtype {} failed", magic_enum::enum_name(t), magic_enum::enum_name(nethead));
                break;
            }
            _sendbuf.clear(); 
        }
        co_return;
    } 

    virtual NetHeadType getNetHeadType()
    {
        return nethead;
    }

private:
    string _resp;
};

}
