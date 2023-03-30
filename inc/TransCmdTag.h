
#pragma once

#include "magic_enum.hpp"
#include "MainCmdTag.h"
#include "hotupdate.h"
#include "enums.h"
#include "database.h"
#include "reqreps.h"
#include "Routers.h"
#include "boost/callable_traits/function_type.hpp"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "hotupdate.h"
#include "iguana/json.hpp"
#include "CoEvent.h"
#include "templatefuncs.h"
#include <string>
#include <string_view>
#include <stdint.h>
#include <map>
#include <functional>
#include <memory>
#include <typeinfo>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::string_view;
using asio::awaitable;
using std::map;

#define BEGIN_DB try{
#define END_DB }\
catch(const std::runtime_error& e)\
{\
    SPDLOG_WARN("db query error with runtime exception {}", e.what());\
}\
catch (const std::exception& e)\
{\
    SPDLOG_WARN("db query error with std exception {}", e.what());\
}\
catch (...)\
{\
    SPDLOG_WARN("db query error with no expected exception");\
}

#define BEGIN_ROUTER_MAP_DB(mainc) static void initPathRouting(){ \
   _mainc = mainc;\
    auto vv = std::make_shared<DependIOCTXInitFuncType>([](){\
            MyInit();\
        });\
    SMHotupdate::AddDependIOCTXInitFunc(vv);

template<typename F, typename AssType, typename Inst>
void ROUTER_DBT2(F func, AssType asstype, Inst* inst)
{
	using ReturnType = boost::callable_traits::return_type_t<F>;
    using Args = boost::callable_traits::args_t<F>;
    auto lambdax = make_shared<RouterFuncType>([=](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {
        RouterFuncReturnType strret;
    using CommFuncTypes = decltype(cdr(std::declval<Args>()));
    bool bArgsParse{ true };
    size_t offset{ 0 };
	ReturnType ret;
    auto unpacked = unpackArgs<CommFuncTypes>(string_view(msg.data(), msg.length()));
    bArgsParse = std::get<0>(unpacked);
    auto args = tuple_pop_front(unpacked);
	if (!bArgsParse)
	{
		invalidJSONRep rep;
		SPDLOG_WARN("parse req json {} failed", msg.data());
		co_return my_to_string(rep);
	}
    else
    {
        std::tuple<Inst*> ff = std::make_tuple(inst);
        auto funcargs = std::tuple_cat(ff, args);
		SMHotupdate::CoEvent ev(*IOCTX); 
		TFEXEC->async([&]() {
			SPDLOG_INFO("start send query to database"); 
            BEGIN_DB;
            ret.code = magic_enum::enum_integer(statusCode::sqlExecuteFailed);
            ret = std::apply(func, std::tuple_cat(ff, args));
            END_DB;
			asio::co_spawn(*IOCTX, [&ev]()->asio::awaitable<void> {
				ev.trigger(); 
				co_return; 
				}, asio::detached); 
			}); 
		SPDLOG_INFO("start wait query finish in asio"); 
		co_await ev.async_wait(); 
    }
	strret = my_to_string(ret);
	//SPDLOG_INFO("database query return {}", *strret);
	co_return strret;
		});
    SMCONF::addRouterTrans(inst->_mainc, (int)(asstype), lambdax);
}

#define ROUTER_DB3(func, asstype)\
    ROUTER_DBT2<decltype(func), decltype(asstype), decltype(_inst2)>(func, asstype, &_inst2);


#define ROUTER_DB_TRANS_NO_PRE_POST(func, ParamterType, ReturnType, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string& msg, string token) -> asio::awaitable<RouterFuncReturnType> {\
            ParamterType req;\
            ReturnType ret;\
            RouterFuncReturnType strret;\
            SPDLOG_INFO("req type {}", typeid(req).name());\
            if (!my_json_parse_from_string(req, msg)) \
            {\
              SPDLOG_WARN("parse req json {} failed", msg.data());\
              ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            { \
                auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
                SMHotupdate::CoEvent ev(*IOCTX);\
                TFEXEC->async([&]() {\
                    SPDLOG_INFO("start send query to database");\
	                ret = vv(req);\
                    asio::co_spawn(*IOCTX, [&ev]()->asio::awaitable<void>{\
                        ev.trigger();\
                        co_return;\
                        }, asio::detached);\
	                });\
                SPDLOG_INFO("start wait query finish in asio");\
                co_await ev.async_wait();\
                SMDB::afterQuery();\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("database query return {}", *strret);\
            co_return strret;\
        });\
        SMCONF::addRouterTrans(_mainc, (int)(AssTypeVar), lambdax); }

#define END_ROUTER_MAP_DB }

template<class T, class MainType>
class TransCmdTag : public MainCmdTag 
{
    public:
      static T _inst2;
      static MainType _mainc;
    TransCmdTag(){}

    static void MyInit() { _inst2.init(ServeMode::SBind); }

   

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
};

template<class T, class MainType>
T TransCmdTag<T, MainType>::_inst2;

template<class T, class MainType>
MainType TransCmdTag<T, MainType>::_mainc;

