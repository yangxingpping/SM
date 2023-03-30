#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"

#include <memory>
#include <vector>
#include <string>
#include <thread>
using std::shared_ptr;
using std::vector;
using std::string;

namespace SMNetwork
{

template <MessageDealer T, ChannelType t>
class SyncRep2
{
  public:
    SyncRep2(std::string addr, uint16_t port)
    :_ip(addr)
    ,_port(port)
    , _concurrent(concurrent)
    {

    }
    void init()
    {
      std::thread th[&,this](){
        while(_brun){
          nng_socket sock;
          nng_listener listener;
          int        rv;
          int        count = 0;
          string addr = "tcp://"
          addr += _ip;
          addr += ":";
          addr += std::to_string(_port);
          if ((rv = nng_rep0_open(&sock)) != 0) {
            //log
          }

          if ((rv = nng_listener_create(&listener, sock, addr.c_str())) != 0) {
            //log
          }
          nng_listener_start(listener, 0);
          for (;;) {
            char *   buf = NULL;
            size_t   sz;
            if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            }
            this->_dealer.dealMsg(string_view(buf, sz));
            rv = nng_send(sock, buf, sz, NNG_FLAG_ALLOC);
            nng_free(buf, sz);
          }
        }
      };
      th.detach();
    }


  private:
    string _ip;
    uint16_t _port;
    
    T _dealer;
    bool _brun = true;
};

}
