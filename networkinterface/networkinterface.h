#pragma once
#include "networkinterfaceExport.h"
#include "magic_enum.hpp"
#include "templatefuncs.h"
#include "asio/awaitable.hpp"
#include "fmt/format.h"
#include "Utils.h"
#include "tsl/htrie_map.h"
#include <map>
#include <string>
#include <stdbool.h>
#include <functional>
#include <tuple>
#include <sstream>

using std::map;
using std::string;
using std::shared_ptr;
using std::tuple;
using std::make_tuple;
using tsl::htrie_map;

typedef std::function<asio::awaitable<RouterFuncReturnType>(string&, string)> RouterFuncType;
typedef shared_ptr<RouterFuncType> ConRouterType;
typedef map<string, ConRouterType> RoutersType;
typedef RoutersType* PtRoutersType;
typedef RoutersType& RefRoutersType;
typedef htrie_map<char, ConRouterType> CodeRoutersType;

typedef map<HtmlBodyType, RoutersType>* PtHtmlRoutersType;

typedef map<int, ConRouterType> TransRouterElement;
typedef TransRouterElement& RefTransRouterElement;
typedef TransRouterElement* PtTransRouterElement;

typedef map<MainCmd, TransRouterElement> TransRoutersType;

namespace SMNetwork
{
    NETWORKINTERFACE_EXPORT void initjwtconfig(string_view issuer = "alqaz", string_view type = "JWS", string_view key = "key", uint32_t expiresecond = 3600);
    NETWORKINTERFACE_EXPORT void initjwtconfig(JWTConf& jwtconf);
    NETWORKINTERFACE_EXPORT bool isjwttokenright(string_view token);
    NETWORKINTERFACE_EXPORT string getjwttoken();

    NETWORKINTERFACE_EXPORT bool initNetwork();

    NETWORKINTERFACE_EXPORT bool  addRouter(HtmlBodyType bodyType, tuple<string, string> path, ConRouterType func);

    NETWORKINTERFACE_EXPORT bool  addRouterJson(tuple<string, string> path, ConRouterType func);

    NETWORKINTERFACE_EXPORT bool  addRouterHtml(tuple<string, string> path, ConRouterType func);

    NETWORKINTERFACE_EXPORT bool  addRouterTrans(MainCmd mainc, int assc, ConRouterType func);

    NETWORKINTERFACE_EXPORT void testSpawn();

    NETWORKINTERFACE_EXPORT ConRouterType  getRouterTrans(MainCmd mainc, int assc);

    NETWORKINTERFACE_EXPORT PtTransRouterElement  getRouterTransByMainC(MainCmd mainc);

    NETWORKINTERFACE_EXPORT PtHtmlRoutersType  getRouters();

    NETWORKINTERFACE_EXPORT CodeRoutersType  getCodeRouters();

    NETWORKINTERFACE_EXPORT void  asyn_nng_demo();

template <class MainC, class AssC>
tuple<string, string> combinePath(MainC mainc, AssC assc)
{
    auto v = SMUtils::packheads(magic_enum::enum_integer(mainc), magic_enum::enum_integer(assc));

    return make_tuple<string, string>(std::move(v), fmt::format("/{}/{}", magic_enum::enum_name(mainc), magic_enum::enum_name(assc)));
}

}
