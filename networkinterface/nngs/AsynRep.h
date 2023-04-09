#pragma once
#include "networkinterfaceExport.h"
#include "NNGCommBase.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "AsynBase.h"

#include <memory>
#include <vector>
using std::shared_ptr;
using std::vector;
using std::string;

namespace SMNetwork
{

class NETWORKINTERFACE_EXPORT AsynRep : public NNGCommBase, public AsyncBase
{
  
  public:
    AsynRep(string ip, uint16_t port, ChannelType ctype, NNgTransType trans = NNgTransType::TCP, NngSockImplType socktype= NngSockImplType::NormalSock, int concurrent=1024);
    void init(ServeMode mode, shared_ptr<PackDealerBase> dealer);
    virtual void init(ServeMode mode) override;
    asio::awaitable<void> messageTask(work* sock, nng_msg* msg);
    virtual ServeMode getServeMode();
    virtual int concurrentcount() override;
  protected:
    friend work;
    
    private:
        ServeMode _serveMode;
    int _concurrent {1024};
    vector<shared_ptr<work>> _cons;
};



}
