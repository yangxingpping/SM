
#include "DBConnectTCP.h"
#include "fmt/format.h"

#include "PackUnpackManager.h"

#include <exception>
#include <stdexcept>
#include <assert.h>
#include <cassert>

using std::exception;
using std::runtime_error;
using std::to_string;
namespace SMDB
{
DBConnectTCP::DBConnectTCP( string ip, uint16_t port)
    : _ip(ip)
    , _port(port)
{
    _client = make_shared<SMNetwork::TcpClientCombine<ChannelModeC::Initiative, MainCmd>>(_ip, _port, MainCmd::DBQuery);
}

DBConnectTCP::~DBConnectTCP() {

}


asio::awaitable<bool> DBConnectTCP::_execQuery(string& req, string& rep)
{
    bool succ = false;
    uint8_t trycount = 0;
    BEGIN_ASIO;
    while (++trycount<3)
    {
        auto ret = co_await _client->getChannel();
        if (ret == nullptr)
        {
            co_return succ;
        }
        auto strreq = make_shared<string>(req);
        auto ret2 = co_await ret->reqrep(strreq);
        if (ret2)
        {
            rep = *ret2;
            succ = true;
        }
        if (succ)
        {
            _client->_returnChannel(ret);
            break;
        }
        ++trycount;
    }
    END_ASIO;
    co_return succ;
}
}