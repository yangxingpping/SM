/**
 * @file HttpCmdTag.h
 * @author alqaz (you@domain.com)
 * @brief macros define for gateway routers
 * @version 0.1
 * @date 2022-11-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once
#include "templatefuncs.h"
#include "magic_enum.hpp"
#include "database.h"
#include "MainCmdTag.h"
#include "enums.h"
#include "Utils.h"
#include "reqreps.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "IOContextManager.h"
#include "DBConnectManager.h"
#include "boost/callable_traits/function_type.hpp"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include "iguana/json.hpp"
#include <string>
#include <string_view>
#include <stdint.h>
#include <map>
#include <functional>
#include <memory>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::string_view;
using asio::awaitable;
using std::map;



#define BEGIN_ROUTER_MAP(mainc) static void initPathRouting(){ \
   _mainc = mainc; {\
    auto vv = std::make_shared<DependIOCTXInitFuncType>([](){\
            MyInit();\
        });\
    SMHotupdate::AddDependIOCTXInitFunc(vv);\
   }




/**
 * only need send query to database instance, request parameter and response parameter not need modify
 */
#define ROUTER_NEED_DB_JSON(ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            ParamterType req;\
            ReturnType ret;\
            RouterFuncReturnType strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
                SPDLOG_WARN("invalid json request {}", msg);\
            }\
            else\
            {\
                if (!token.empty())\
                {\
                    req.token = token; \
                }\
				co_await DBCMGRREF.executeQuery(req, ret, magic_enum::enum_integer(AssDBTypeVar));\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

 /**
  * router send req to local cache, if cache hit, return rep; else, send req to db proxy, then return rep
  */
#define ROUTER_NEED_CACHE_DB_JSON(localQuery, ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            ParamterType req;\
            ReturnType ret;\
            RouterFuncReturnType strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                if(!token.empty())\
                {\
                    req.token = token;\
                }\
                bool bret = std::invoke(localQuery, &_inst2, req, ret);\
                if(bret)\
                {\
					co_await DBCMGRREF.executeQuery(req, ret, magic_enum::enum_integer(AssDBTypeVar));\
                }\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

  /**
	 * router send req to local cache and direct return rep
	 */
#define ROUTER_NEED_CACHE_JSON(localQuery, ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            ParamterType req;\
            ReturnType ret;\
            RouterFuncReturnType strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                if(!token.empty())\
                {\
                    req.token = token;\
                }\
                ret = std::invoke(localQuery, &_inst2, req);\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, *strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

	 /**
	  * @brief router uniform db request, send db request to db proxy, return rep
	  *
	  */
#define ROUTER_NEED_DBREQ_DB_JSON(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            ParamterType req;\
            ReturnType ret;\
            RouterFuncReturnType strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                if(!token.empty())\
                {\
                    req.token = token;\
                }\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                auto dbreq = vv(req);\
                {\
				    co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(AssDBTypeVar));\
                }\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }


//#define ROUTER_NEED_DBREQ_DB_JSON2(func, ParamterType, DBParamterType, AssTypeVar, AssDBTypeVar)\
//    ROUTER_QUERY()


template<typename F, typename AssType, typename Inst>
void ROUTER_QUERY(F func, AssType asstype, Inst* inst)
{
	using ReturnType = boost::callable_traits::return_type_t<F>;
	using Args = boost::callable_traits::args_t<F>;

	auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
		RouterFuncReturnType strret;
	using CommFuncTypes = decltype(cdr(std::declval<Args>()));
	CommFuncTypes args;
	bool bArgsParse{ true };
	size_t offset{ 0 };
	ReturnType ret;

	SMUtils::for_each(args, [&](auto& arg) {
		auto strparam = SMUtils::unpackstring(string_view(msg.data() + offset, msg.length() - offset));
	bArgsParse = my_json_parse_from_string(arg, strparam);
	if (!bArgsParse)
	{
		return;
	}
	offset += (strparam.length() + sizeof(uint32_t));

		});
	if (!bArgsParse)
	{
		invalidJSONRep rep;
		SPDLOG_WARN("parse req json {} failed", msg.data());
		co_return my_to_string(rep);
	}
	else
	{
		if (!token.empty())
		{
			std::get<0>(args).token = token;
		}
        std::tuple<Inst*> ff = new std::tuple<Inst*>(inst);
		auto dbreq = std::apply(func, std::tuple_cat(ff, args));
		ReturnType ret;
		{
			co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(asstype));
		}
	}
	strret = my_to_string(ret);
	co_return strret;
		});
	SMNetwork::addRouterJson(SMNetwork::combinePath(inst->_mainc, asstype), lambdax);
}

#define ROUTER_NEED_DBREQ_DB_JSON2(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\

/**
 * @brief router unifrom db request , send db request to database proxy, convert db rep, return rep
 *
 */
#define ROUTER_NEED_DBREQ_DB_DBREP_JSON(func, funcpost, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            ParamterType req;\
            ReturnType ret;\
            RouterFuncReturnType strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                if(!token.empty())\
                {\
                    req.token = token;\
                }\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                auto dbreq = vv(req);\
                {\
				    co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(AssDBTypeVar));\
                }\
                auto vvpost = std::bind(funcpost, &_inst2, std::placeholders::_1);\
                vvpost(ret);\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }


#define ROUTER_RETURN_STR_HTML(func, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
            RouterFuncReturnType strret = co_await vv(msg);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define END_ROUTER_MAP }

template<class T, class MainType>
class HttpCmdTag : public MainCmdTag
{
public:
	static T _inst2;
	static MainType _mainc;
	HttpCmdTag() {}


	static T& Inst() { return _inst2; }
	static void MyInit() { _inst2.init(ServeMode::SBind); }
	static MainType GetMainCmd() { return _mainc; }

private:
	class methodRegistrator
	{
	public:
		methodRegistrator()
		{
			T::initPathRouting();
		}
	};
	methodRegistrator _inst;

protected:
	bool _inited = false;
};

template<class T, class MainType>
T HttpCmdTag<T, MainType>::_inst2;

template<class T, class MainType>
MainType HttpCmdTag<T, MainType>::_mainc;

