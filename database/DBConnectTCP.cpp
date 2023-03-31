
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
    _client = make_shared<SMNetwork::TcpClientCombine<ChannelType::DBClient, NetHeadType::FixPackLenPlaceHolder, ChannelModeC::Initiative>>(_ip, _port);
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
        auto pack = PUM->clone(magic_enum::enum_integer(_mainc));
        assert(pack != nullptr);
        if (pack == nullptr)
        {
            SPDLOG_ERROR("get plat pack for {} failed", magic_enum::enum_name(_mainc));
            co_return succ;
        }
        pack->setAss(op);
        auto strreq = make_shared<string>();
        strreq->resize(pack->len());
        pack->pack(span<char>(strreq->begin(), strreq->begin() + pack->len()));
        strreq->append(req);
        auto ret2 = co_await ret->reqrep(strreq);
        if (ret2 && ret2->length() > pack->len())
        {
            assert(pack->unpack(string_view(ret2->begin(), ret2->begin() + pack->len())));
            assert(pack->getMain() == magic_enum::enum_integer(_mainc));
            rep = ret2->substr(pack->len());
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