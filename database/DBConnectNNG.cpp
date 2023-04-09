
#include "DBConnectNNG.h"
#include "DBConnectManager.h"
#include "fmt/format.h"
#include <exception>
#include <stdexcept>
#include <assert.h>
#include <cassert>

using std::exception;
using std::runtime_error;
using std::to_string;
namespace SMDB
{
DBConnectNNG::DBConnectNNG( string ip, uint16_t port)
    :_ip(ip)
    , _port(port)
{
    auto addr = fmt::format("tcp://{}:{}", ip, port);
    _channels = make_shared<SMNetwork::ReqManager<ChannelModeC::Initiative, MainCmd>>(string_view(addr), MainCmd::DBQuery);
}


DBConnectNNG::~DBConnectNNG() {

}

asio::awaitable<bool> DBConnectNNG::_execQuery(shared_ptr<string> req, string& rep)
{
    bool bret{ false };
    assert(_channels);
    auto w = co_await _channels->getChannel();
    if (w == nullptr)
    {
        co_return false;
    }
    BEGIN_ASIO;
    auto ptrep = co_await w->reqrep(req);
    if (ptrep != nullptr)
    {
        rep = *ptrep;
    }
    END_ASIO;
    co_return bret;
}
}

