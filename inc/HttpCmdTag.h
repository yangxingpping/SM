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
#include "reqreps.h"
#include "enums.h"
#include "Utils.h"
#include "reqreps.h"
#include "Routers.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
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

template<typename ParamterType, typename ReturnType, typename AssType, typename AssDBType, typename Inst>
void T_ROUTER_NEED_DB_JSON(AssType AssTypeVar, AssDBType AssDBTypeVar, Inst* inst)
{
    auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
            ParamterType req;
            ReturnType ret;
            RouterFuncReturnType strret;
            if (!my_json_parse_from_string(req, msg)) 
            {
                ret.code = to_underlying(statusCode::invalidJson);
                SPDLOG_WARN("invalid json request {}", msg);
            }
            else
            {
                if (!token.empty())
                {
                    req.token = token;
                }
				co_await DBCMGRREF.executeQuery(req, ret, magic_enum::enum_integer(AssDBTypeVar));
            }
            strret = (ret.code == magic_enum::enum_integer(statusCode::needNextRouter)) ? std::make_shared<std::string>() : my_to_string(ret);
            co_return strret;
        });
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, AssTypeVar), lambdax);
}

#define ROUTER_NEED_DB_JSON3(ParamterType, ReturnType, AssTypeVar, AssDBTypeVar)\
    T_ROUTER_NEED_DB_JSON<ParamterType, ReturnType, decltype(AssTypeVar), decltype(AssDBTypeVar)>(AssTypeVar, AssDBTypeVar, &_inst2);

template<typename QueryCacheFunc, typename AssType, typename AssDBType, typename Inst>
void T_ROUTER_NEED_CACHE_DB_JSON(QueryCacheFunc localQuery, AssType asstype, AssDBType assdb, Inst* inst)
{
	auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType>
	{
			using Args = boost::callable_traits::args_t<QueryCacheFunc>;
	using CommFuncTypes = decltype(cdr(std::declval<Args>()));
    using ReturnType = boost::callable_traits::return_type_t<QueryCacheFunc>;
	CommFuncTypes args;
	auto [req] = args;
    ReturnType ret;
	RouterFuncReturnType strret;
	if (!my_json_parse_from_string(req, msg))
	{
		ret.code = to_underlying(statusCode::invalidJson);
	}
	else
	{
		if (!token.empty())
		{
			req.token = token;
		}
        ret = std::invoke(localQuery, inst, req);
		if (ret.code==magic_enum::enum_integer(statusCode::needNextRouter))
		{
			co_await DBCMGRREF.executeQuery(req, ret, magic_enum::enum_integer(assdb));
		}
	}
	strret = my_to_string(ret);
	co_return strret;
		});
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, asstype), lambdax);
}

#define ROUTER_NEED_CACHE_DB_JSON3(func, asstype, assdb)\
    T_ROUTER_NEED_CACHE_DB_JSON<decltype(func), decltype(asstype), decltype(assdb)>(func, asstype, assdb, &_inst2);

template<typename LocalQueryFunc, typename AssType, typename AssDBType, typename Inst>
void T_ROUTER_NEED_CACHE_JSON(LocalQueryFunc localQuery, AssType asstype, AssDBType assdb, Inst* inst)
{
	auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
		using Args = boost::callable_traits::args_t<LocalQueryFunc>;
	    using CommFuncTypes = decltype(cdr(std::declval<Args>()));
	    using ReturnType = boost::callable_traits::return_type_t<LocalQueryFunc>;
	    CommFuncTypes args;
	    auto [req] = args;
		ReturnType ret; 
		RouterFuncReturnType strret; 
		if (!my_json_parse_from_string(req, msg)) 
		{
			ret.code = to_underlying(statusCode::invalidJson); 
		}
		else
		{
			if (!token.empty())
			{
				req.token = token; 
			}
				ret = std::invoke(localQuery, inst, req); 
		}
		strret = my_to_string(ret);
		SPDLOG_INFO("execute query {} get response {}", msg, *strret);
		co_return strret;
		}); 
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, assdb), lambdax);
}

#define ROUTER_NEED_CACHE_JSON3(func, asstype, assdb)\
    T_ROUTER_NEED_CACHE_JSON<decltype(func), decltype(asstype), decltype(assdb)>(func, asstype, assdb, &_inst2);

template<typename Func, typename ReturnType, typename AssType, typename AssDBType, typename Inst, typename std::enable_if<std::is_member_function_pointer_v<Func>, Func>::type* = nullptr>
void T_ROUTER_NEED_DBREQ_DB_JSON(Func func, AssType AssTypeVar, AssDBType AssDBTypeVar, Inst* inst) {
    using DBArgType = boost::callable_traits::return_type_t<Func>;
    auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
            ReturnType ret;
            RouterFuncReturnType strret;
            auto req = unpackArgs<Func>(string_view(msg.data(), msg.length()));
            if (!std::get<0>(req)) 
            {
                ret.code = to_underlying(statusCode::invalidJson);
            }
            else
            {
                if(!token.empty())
                {
                    std::get<1>(req).token = token;
                }
                ReturnType ret;
				auto funcBindInst = std::bind_front(func, inst);
                DBArgType dbreq = std::apply(funcBindInst, tuple_pop_front(req));
                {
				    co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(AssDBTypeVar));
                }
            }
            strret = my_to_string(ret);
            co_return strret;
        });
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, AssTypeVar), lambdax);
}

template<typename Func, typename ReturnType, typename AssType, typename AssDBType, typename Inst, typename std::enable_if<std::is_function_v<Func>, Func>::type* = nullptr>
void T_ROUTER_NEED_DBREQ_DB_JSON(Func func, AssType AssTypeVar, AssDBType AssDBTypeVar, Inst* inst) {
    using DBArgType = boost::callable_traits::return_type_t<Func>;
	auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
	ReturnType ret;
	RouterFuncReturnType strret;
	auto req = unpackArgs<Func>(string_view(msg.data(), msg.length()));
	if (!std::get<0>(req))
	{
		ret.code = to_underlying(statusCode::invalidJson);
	}
	else
	{
		if (!token.empty())
		{
			std::get<1>(req).token = token;
		}
		ReturnType ret;
        DBArgType dbreq = std::apply(func, tuple_pop_front(req));
		{
			co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(AssDBTypeVar));
		}
	}
	strret = my_to_string(ret);
	co_return strret;
		});
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, AssTypeVar), lambdax);
}


#define ROUTER_NEED_DBREQ_DB_JSON2(func, ReturnType, AssTypeVar, AssDBTypeVar)\
    T_ROUTER_NEED_DBREQ_DB_JSON<decltype(func), ReturnType, decltype(AssTypeVar), decltype(AssDBTypeVar), decltype(_inst2)>(func, AssTypeVar, AssDBTypeVar, &_inst2); 


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
		auto funcBindInst = std::bind_front(func, inst);
		auto dbreq = std::apply(func, args);
		ReturnType ret;
		{
			co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(asstype));
		}
	}
	strret = my_to_string(ret);
	co_return strret;
		});
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, asstype), lambdax);
}

 /**
  * @brief template router uniform db request , send db request to database proxy, convert db rep, return rep
  *
  */
template<typename ConvertReqFunc, typename ConvertRepFunc, typename AssType, typename AssDBType, typename Inst>
void T_ROUTER_NEED_DBREQ_DB_DBREP_JSON(ConvertReqFunc reqfunc, ConvertRepFunc repfunc, AssType ass, AssDBType assdb, Inst* inst) 
{
	auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
		using ReqArgs = boost::callable_traits::args_t<ConvertReqFunc>;
		using CommReqArgs = decltype(cdr(std::declval<ReqArgs>()));
		using ReqReturn = boost::callable_traits::return_type_t<ConvertReqFunc>;
		using RepArgs = boost::callable_traits::args_t<ConvertRepFunc>;
		using CommRepArgs = decltype(cdr(std::declval<RepArgs>()));
		using RepReturn = boost::callable_traits::return_type_t<ConvertRepFunc>;
		RepReturn ret;
		RouterFuncReturnType strret;
		auto unpacked = unpackArgs<CommReqArgs>(string_view(msg.data(), msg.length()));
		if (!std::get<0>(unpacked))
		{
			ret.code = magic_enum::enum_integer(statusCode::invalidJson);
			co_return my_to_string(ret);
		}
		auto reqargs = tuple_pop_front(unpacked);
		if (!token.empty())
		{
			std::get<0>(reqargs).token = token;
		}
		auto funcBindInst = std::bind_front(reqfunc, inst);
		auto dbreq = std::apply(funcBindInst, reqargs);
		{
			co_await DBCMGRREF.executeQuery(dbreq, ret, magic_enum::enum_integer(assdb));
		}
		ret = std::apply(repfunc, std::tuple_cat(std::make_tuple(inst), std::make_tuple(ret)));
		strret = my_to_string(ret);
		co_return strret;
    });
	SMCONF::addRouterJson(SMCONF::combinePath(inst->_mainc, assdb), lambdax); }


#define  ROUTER_NEED_DBREQ_DB_DBREP_JSON2(reqfunc, repfunc, ass, dbass)\
	T_ROUTER_NEED_DBREQ_DB_DBREP_JSON<decltype(reqfunc), decltype(repfunc), decltype(ass), decltype(dbass), decltype(_inst2)>(reqfunc, repfunc, ass, dbass, &_inst2);

template<typename Func, typename AssType, typename Inst>
void t_add_router(Func func, AssType ass, Inst* inst)
{
	auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
		auto vv = std::bind(func, inst, std::placeholders::_1); 
		RouterFuncReturnType strret = co_await vv(msg); 
		co_return strret; 
		}); 
	SMCONF::addRouterHtml(SMCONF::combinePath(inst->_mainc, ass), lambdax);
}

#define ROUTER_RETURN_STR_HTML2(func, AssTypeVar)\
    t_add_router<decltype(func), decltype(AssTypeVar), decltype(_inst2)>(func, AssTypeVar, &_inst2);   

#define ROUTER_RETURN_STR_HTML(func, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
            RouterFuncReturnType strret = co_await vv(msg);\
            co_return strret;\
        });\
        SMCONF::addRouterHtml(SMCONF::combinePath(_mainc, AssTypeVar), lambdax); }

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

