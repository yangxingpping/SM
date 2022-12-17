
#pragma once

#include "NngServerBase.h"
#include <string>
#include <stdint.h>
#include "enums.h"

using std::string;

namespace SMNetwork
{

class NNGCommBase : public NNgServerBase
{
    public:
    NNGCommBase(string ip, uint16_t port, ChannelType type, NNgTransType trans, NngSockImplType socktype=NngSockImplType::NormalSock)
    : _channelType(type)
    ,_ip(ip)
    ,_port(port)
    , _trans(trans)
    ,_socktype(socktype)
    {

    }
    virtual ChannelType getChannelType() override
    {
        return _channelType;
    }

    virtual string getAddr() override
    {
        string ret;
        switch(_trans)
        {
            case NNgTransType::TCP:
            {
                ret = "tcp://";
            }break;
            case NNgTransType::WS:
            {
                ret = "ws://";
            }break;
            default:
            break;
        }
        ret += _ip;
        ret += ":";
        ret += std::to_string(_port);
        return ret;
    }

    

    

    virtual NNgTransType getNNgTransType() override{
        return _trans;
    }

    
    NngSockImplType Socktype() const { return _socktype; }
    void Socktype(NngSockImplType val) { _socktype = val; }
protected:
    nng_socket _socket = {};
    ChannelType _channelType;
    NngSockImplType _socktype;
    string      _ip;
    uint16_t    _port;
    PackType _packType;
    NNgTransType _trans;
};

}

