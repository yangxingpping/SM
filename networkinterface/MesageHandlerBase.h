#pragma once
#include "networkinterfaceExport.h"
#include "Utils.h"
#include "asio/ip/tcp.hpp"
#include "coros.h"
#include "taskflow/taskflow.hpp"
#include <memory>
#include <string_view>

using asio::ip::tcp;
using std::shared_ptr;
using std::function;
using std::string;
using std::string_view;

namespace SMNetwork
{
class NETWORKINTERFACE_EXPORT MesageHandlerBase
{
  public:
    MesageHandlerBase();
    virtual string syncDealMsg(string_view msg)=0;
    asio::awaitable<string> messageTask(string_view msg);
    void stop() {
        _setStopFlag(true);
    }
    bool isStop() {
        return _bstop;
    }

    protected:
        void _setStopFlag(bool bflag) {
            _bstop = bflag;
    }
  private:
    bool _bstop = false;
};
}
