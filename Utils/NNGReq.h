
#pragma once

#include "reqs.h"
#include "Utils.h"
#include "spdlog/spdlog.h"
#include "magic_enum.hpp"
#include <string>
#include <string_view>
#include <stdint.h>
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"

using std::string;
using std::string_view;

namespace SMUtils
{
class  NNGReq
{
public:
    NNGReq(string ip, uint16_t port, MainCmd mainx, ChannelType ct)
    : _ip(ip)
    , _port(port)
    , _mainc(mainx)
    , _channelType(ct)
    {
        string addr = "tcp://";
        addr += _ip;
        addr += ":";
        addr += std::to_string(_port);
        int opnng = nng_req0_open(&_sock);
        opnng = nng_dialer_create(&_dailer, _sock, addr.c_str());
        nng_dialer_start(_dailer, NNG_FLAG_NONBLOCK);
        SPDLOG_INFO("start nng request channel type {} main type {} on addr {}",
         magic_enum::enum_name(_channelType), magic_enum::enum_name(mainx), addr);
    }
    ~NNGReq()
    {
        nng_close(_sock);
    }
    template<class Req, class Rep, class AssType>
    Rep sendReq(Req& req, AssType op)
    {
        int nnop = 0;
        Rep rep;
        string strreq = req.to_string();
        auto reqhead = SMUtils::packcmdrep(_mainc, (short)(op) , strreq.length());
        string packreq(reqhead.begin(), reqhead.end());
        packreq += strreq;
        nnop = nng_send(_sock, &packreq[0], packreq.length(), 0);
        size_t recvlen = 0;
        char* recvbuf = NULL;
        nnop = nng_recv(_sock, &recvbuf, &recvlen, NNG_FLAG_ALLOC);
        auto header = parsecmd(string_view(recvbuf, 12));
        string strbody{recvbuf+12, recvlen-12};
        rep.parse(strbody);
        return rep;
    }
    nng_socket _sock;
    nng_dialer _dailer;
    string _ip;
    uint16_t _port;

    MainCmd _mainc;
    ChannelType _channelType;
};
}


//NNGDBReq
