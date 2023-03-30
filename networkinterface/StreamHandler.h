#pragma once
#include "networkinterfaceExport.h"
#include "Utils.h"
#include "asio/awaitable.hpp"
#include "MessageSplitFuncs.h"
#include "enums.h"

#include <memory>
#include <deque>
#include <tuple>

using asio::ip::tcp;
using std::shared_ptr;
using std::function;
using std::string;
using std::string_view;
using std::deque;
using std::tuple;
using std::make_tuple;

namespace SMNetwork
{
/**
 * @brief base class for stream protocol implement, need split message for recv and pack message header for send message
 * 
 * @tparam ConnectType connect stream protocol implement(like tcp, tcp is a traditional stream protocol)
 */
template <class ConnectType>
class StreamHandler
{
  public:
    StreamHandler() { }
    virtual ~StreamHandler() { }
    virtual asio::awaitable<tuple<bool, string*>> recvPack(uint16_t retrycount=50) = 0;
    virtual asio::awaitable<bool> sendPack(string_view msg) = 0;
    virtual shared_ptr<ConnectType> getHandler() = 0;
    virtual string getFullAddr() = 0;
    virtual NetHeadType getNetHeadType() = 0;
};
}
