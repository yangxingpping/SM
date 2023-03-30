
#include "DBConnectTCP.h"
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
DBConnectTCP::DBConnectTCP( string ip, uint16_t port)
    : _ip(ip)
    , _port(port)
{
    _client = make_shared<SMNetwork::TcpClient<ChannelType::DBClient, NetHeadType::FixPackLenPlaceHolder, ChannelIsInitiative::Initiative>>(_ip, _port);
    //_client->start();
}

DBConnectTCP::~DBConnectTCP() {

}


asio::awaitable<bool> DBConnectTCP::_execQuery(string& req, string& rep, uint16_t op)
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
        shared_ptr<SMNetwork::PackDealerBase> dealer = std::shared_ptr<SMNetwork::PackDealerBase>(new SMNetwork::PackDealerMainSub(MainCmd::DBQuery, ChannelType::DBClient));
        dealer->setAssc(op);
        auto ret2 = co_await ret->clientEntry(dealer, req);
        succ = !ret2->empty();
        rep.insert(rep.end(), ret2->begin(), ret2->end());
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