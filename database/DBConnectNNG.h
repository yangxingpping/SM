#pragma once
#include "databaseExport.h"
#include "enums.h"
#include "Utils.h"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

#include "AsynReq.h"
#include "PackDealerMainSub.h"
#include "myconcept.h"

#include <string>
#include <string_view>
#include <algorithm>
#include <ranges>
#include <memory>

using std::string;
using std::string_view;

namespace SMDB
{
class DATABASE_EXPORT DBConnectNNG
{
  public:
    DBConnectNNG( string ip, uint16_t port);
    ~DBConnectNNG();
    asio::awaitable<bool> _execQuery(string& req, string& rep, int op);

private:
    string _ip;
    uint16_t _port;
    std::shared_ptr<SMNetwork::AsyncReq> _sock;
    std::shared_ptr<SMNetwork::PackDealerBase> _packer;
};
}
