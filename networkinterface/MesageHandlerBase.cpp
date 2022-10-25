
#include "networkinterfaceExport.h"
#include "MesageHandlerBase.h"
#include "Utils.h"
#include "asio/ip/tcp.hpp"
#include "coros.h"

#include <memory>

using asio::ip::tcp;
using std::shared_ptr;
using std::function;
using std::string;
namespace SMNetwork
{
MesageHandlerBase::MesageHandlerBase()
{
}

asio::awaitable<string> MesageHandlerBase::messageTask(string_view msg)
{
    string ret = syncDealMsg(msg);
         
    co_return ret;
}
}
