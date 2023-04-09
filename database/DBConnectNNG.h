#pragma once
#include "databaseExport.h"
#include "enums.h"
#include "Utils.h"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "enums.h"
#include "../networkinterface/nngs/AsynReq.h"
#include "PackDealerMainSub.h"
#include "myconcept.h"
#include "../networkinterface/nngs/ReqManager.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <ranges>
#include <memory>

using std::string;
using std::string_view;
using std::shared_ptr;
using std::make_shared;

namespace SMDB
{
class DATABASE_EXPORT DBConnectNNG
{
  public:
    DBConnectNNG( string ip, uint16_t port);
    ~DBConnectNNG();
    asio::awaitable<bool> _execQuery(shared_ptr<string> req, string& rep);

private:
    string _ip;
    uint16_t _port;
    shared_ptr < SMNetwork::ReqManager<ChannelModeC::Initiative, MainCmd>> _channels;
    MainCmd _mainc = MainCmd::DBQuery;
};
}
