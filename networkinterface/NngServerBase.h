
#pragma once
#include "networkinterfaceExport.h"
#include "nng/nng.h"
#include "MainCmdTag.h"
#include "enums.h"
#include "asio/awaitable.hpp"
#include <string>
#include <string_view>

using std::string;
using std::string_view;

enum class NngSockType
{
    SyncReq,
    SyncPush,
    SyncPull,
    AsynRep,
};

namespace SMNetwork
{

class NNgServerBase : public MainCmdTag
{
  public:
    NNgServerBase(){}
    NNgServerBase(const NNgServerBase& cons) = delete;
    NNgServerBase(const NNgServerBase&& cons) = delete;
    virtual ~NNgServerBase() {};
    virtual ChannelType getChannelType() = 0;
    virtual string getAddr() = 0;
    virtual NNgTransType getNNgTransType() = 0;
};
}
