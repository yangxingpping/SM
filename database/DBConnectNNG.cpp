
#include "DBConnectNNG.h"
#include "DBConnectManager.h"
#include "fmt/format.h"
#include "PackDealerMainSub.h"
#include <exception>
#include <stdexcept>
#include <assert.h>
#include <cassert>

using std::exception;
using std::runtime_error;
using std::to_string;
namespace SMDB
{
DBConnectNNG::DBConnectNNG( string ip, uint16_t port):_ip(ip), _port(port), _sock(nullptr)
{
    
}


DBConnectNNG::~DBConnectNNG() {

}

asio::awaitable<bool> DBConnectNNG::_execQuery(string& req, string& rep, int op)
{
    bool bret = true;
    if (_sock == nullptr)
    {
        _sock = std::make_shared<SMNetwork::AsyncReq>(_ip, _port, ChannelType::DBClient);
        _packer = std::shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerMainSub(MainCmd::DBQuery, ChannelType::DBClient));
        _packer->setAssc(op);
        _sock->init(ServeMode::SConnect, _packer);
    }
    auto w = _sock->getWorker();
    assert(w != nullptr);
    if (w == nullptr)
    {
        co_return false;
    }
    BEGIN_ASIO;
	rep = co_await w->reqrep(req, (uint16_t)(op));
    END_ASIO;
    co_return bret;
}
}

