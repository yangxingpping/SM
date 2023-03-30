#pragma once
#include "configsExport.h"
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
#include <list>

using std::map;
using std::string;
using std::shared_ptr;
using std::tuple;
using std::make_tuple;
using tsl::htrie_map;
using std::list;

typedef std::function<asio::awaitable<RouterFuncReturnType>(string&, string)> RouterFuncType;
typedef shared_ptr<RouterFuncType> ConRouterType;
typedef map<string, list<ConRouterType>> RoutersType;
typedef RoutersType* PtRoutersType;
typedef RoutersType& RefRoutersType;
typedef htrie_map<char, ConRouterType> CodeRoutersType;

typedef map<HtmlBodyType, RoutersType>* PtHtmlRoutersType;

typedef map<int, ConRouterType> TransRouterElement;
typedef TransRouterElement& RefTransRouterElement;
typedef TransRouterElement* PtTransRouterElement;

typedef map<int, TransRouterElement> TransRoutersType;

namespace SMCONF
{
    

    CONFIGS_EXPORT bool  addRouter(HtmlBodyType bodyType, tuple<string, string> path, ConRouterType func);

    CONFIGS_EXPORT bool  addRouterJson(tuple<string, string> path, ConRouterType func);

    CONFIGS_EXPORT bool  addRouterHtml(tuple<string, string> path, ConRouterType func);

    CONFIGS_EXPORT bool  addRouterTrans(MainCmd mainc, int assc, ConRouterType func);


    CONFIGS_EXPORT ConRouterType  getRouterTrans(MainCmd mainc, int assc);

    CONFIGS_EXPORT PtTransRouterElement  getRouterTransByMainC(MainCmd mainc);

    CONFIGS_EXPORT PtTransRouterElement  getRouterTransByMainC(int mainc);

    CONFIGS_EXPORT PtHtmlRoutersType  getRouters();

    CONFIGS_EXPORT PtRoutersType getRouters(HtmlBodyType body);

    CONFIGS_EXPORT CodeRoutersType  getCodeRouters();

	template <class MainC, class AssC>
	tuple<string, string> combinePath(MainC mainc, AssC assc)
	{
		auto v = SMUtils::packheads(magic_enum::enum_integer(mainc), magic_enum::enum_integer(assc));

		return make_tuple<string, string>(std::move(v), fmt::format("/{}/{}", magic_enum::enum_name(mainc), magic_enum::enum_name(assc)));
	}

}
