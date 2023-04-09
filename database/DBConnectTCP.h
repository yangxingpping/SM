#pragma once
#include "databaseExport.h"
#include "enums.h"
#include "../networkinterface/socket/TcpClientCombine.h"
#include "Utils.h"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

#include "myconcept.h"
#include "enums.h"

#include <string>
#include <string_view>
#include <algorithm>
#include <ranges>

using std::string;
using std::string_view;

namespace SMDB
{
class dealDBMsgPlaceHolder
{
  public:
    asio::awaitable<string> dealMsg(string_view str, uint16_t assc)
    {
      co_return "placeholder";
    }
    MainCmd getMainCmd() {
        return MainCmd::MainCmdBegin;
    }
};

class DATABASE_EXPORT DBConnectTCP
{
  public:
    DBConnectTCP( string ip, uint16_t port);
    ~DBConnectTCP();
    asio::awaitable<bool> _execQuery(string& req, string& succ);
  private:
    string _ip;
    uint16_t _port;
    int16_t _timeoutMiseconds = 5000;
    shared_ptr<SMNetwork::TcpClientCombine<ChannelModeC::Initiative, MainCmd>> _client;
    MainCmd _mainc = MainCmd::DBQuery;
};
}
