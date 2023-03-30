
#pragma once

#include <stdbool.h>
#include <string>
#include <stdint.h>
#include <tuple>
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"

using std::string;
using std::tuple;
namespace SMDB
{
class NNGDatabaseReq
{
    public:
    NNGDatabaseReq(string ip, uint16_t port)
    : _connected(false)
    , _ip(ip)
    , _port(port)
    , _sock(NNG_SOCKET_INITIALIZER)
    , _dialer(NNG_DIALER_INITIALIZER)
    , _retry(3)
    {

    }

    template<class Req, class Rep>
    bool query(Req req, Rep& rep)
    {
        bool bret = true;
        if(!_connected)
        {
            _connected = _connect();
        }
        bret = _connected;
        if(!bret)
        {
            return bret;
        }
        string strreq = req.to_string();

        auto recvblock  = _send_recv(strreq);
        auto ptrecv = std::get<0>(recvblock);
        auto recvlen = std::get<1>(recvblock);
        bret = rep.parse(ptrecv);
        if(ptrecv)
        {
            nng_free(ptrecv, recvlen);
            ptrecv = NULL;
        }
        return bret;
    }

    protected:
    
    tuple<char*, size_t> _send_recv(string& strreq)
    {
        size_t len = 0;
        char* pt = NULL;
        int i=0;
        int nn_op = 0;
        for(i=0 ; i< _retry; ++i)
        {
            _connect();
            nn_op = nng_send(_sock, &strreq[0], strreq.length(), 0);
            if(nn_op!=0)
            {
                _disconnect();
                continue;
            }
            nn_op = nng_recv(_sock, &pt, &len, NNG_FLAG_ALLOC);
            if(nn_op!=0)
            {
                _disconnect();
                continue;
            }
        }
        return std::tuple<char*, size_t>(pt, len);
    }

    bool _connect()
    {
        bool bret = _connected;
        if(bret){
            return bret;
        }
        string addr = "tcp://";
        addr += _ip;
        addr += ":";
        addr += std::to_string(_port);
        int nn_op = nng_req0_open(&_sock);
        if(nn_op!=0)
        {
            return bret;
        }
        nn_op = nng_dialer_create(&_dialer, _sock, addr.c_str());
        if(nn_op!=0)
        {
            nng_close(_sock);
            _sock = NNG_SOCKET_INITIALIZER;
            return bret;
        }
        nn_op = nng_dialer_start(_dialer, NNG_FLAG_NONBLOCK);
        if(nn_op!=0)
        {
            nng_close(_sock);
            _sock = NNG_SOCKET_INITIALIZER;
            return bret;
        }
        bret = true;
        return bret;
    }

    void _disconnect()
    {
        if(_connected){
            nng_close(_sock);
            _sock = NNG_SOCKET_INITIALIZER;
        }
    }

    private:
    bool _connected;
    string _ip;
    uint16_t _port;
    nng_socket _sock;
    nng_dialer _dialer;
    int _retry;

};

}
