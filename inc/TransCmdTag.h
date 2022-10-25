
#pragma once

#include "magic_enum.hpp"
#include "MainCmdTag.h"
#include "enums.h"
#include "database.h"
#include "reqreps.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "IOContextManager.h"
#include "iguana/json.hpp"
#include "CoEvent.h"
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

#define BEGIN_ROUTER_MAP_TRANS(mainc) static void initPathRouting(){ \
   _mainc = mainc;\
    auto vv = std::make_shared<DependIOCTXInitFuncType>([](){\
            MyInit();\
        });\
    SMHotupdate::AddDependIOCTXInitFunc(vv);
#define ADD_ROUTER_TRANS(func, ParamterType, ReturnType, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
              ret.code=to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
                ret = co_await vv(req);\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterTrans(_mainc, (int)(AssTypeVar), lambdax); }

#define ADD_ROUTER_DB_TRANS(func, ParamterType, ReturnType, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            SPDLOG_INFO("req type {}", typeid(req).name());\
            if (!my_json_parse_from_string(req, msg)) \
            {\
              invalidJSONRep rep;\
              SPDLOG_WARN("parse req json {} failed", msg.data());\
              co_return my_to_string(rep);\
            }\
            else\
            {\
                bool bret = SMDB::beforeQuery(string_view{ req.token }); \
                if (!bret)\
                {\
                    jwtCheckFailedJSONRep rep;\
                    co_return my_to_string(rep);\
                }\
                else\
                {\
                    auto vv = std::bind(func, &_inst2, std::placeholders::_1); \
                    SMHotupdate::CoEvent ev; \
                    TFEXEC.async([&]() {\
                        SPDLOG_INFO("start send query to database"); \
                        ret = vv(req); \
                        asio::co_spawn(IOCTX, [&ev]()->asio::awaitable<void> {\
                            ev.trigger(); \
                            co_return; \
                            }, asio::detached); \
                        }); \
                    SPDLOG_INFO("start wait query finish in asio"); \
                    co_await ev.async_wait(); \
                    SMDB::afterQuery(); \
                }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("database query return {}", strret); \
            co_return strret;\
        });\
        SMNetwork::addRouterTrans(_mainc, (int)(AssTypeVar), lambdax); }


#define ADD_ROUTER_DB_TRANS_NO_BEFORE_QUERY(func, ParamterType, ReturnType, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            SPDLOG_INFO("req type {}", typeid(req).name());\
            if (!my_json_parse_from_string(req, msg)) \
            {\
              SPDLOG_WARN("parse req json {} failed", msg.data());\
              ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            { \
                auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
                SMHotupdate::CoEvent ev;\
                TFEXEC.async([&]() {\
                    SPDLOG_INFO("start send query to database");\
	                ret = vv(req);\
                    asio::co_spawn(IOCTX, [&ev]()->asio::awaitable<void>{\
                        ev.trigger();\
                        co_return;\
                        }, asio::detached);\
	                });\
                SPDLOG_INFO("start wait query finish in asio");\
                co_await ev.async_wait();\
                SMDB::afterQuery();\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("database query return {}", strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterTrans(_mainc, (int)(AssTypeVar), lambdax); }

#define END_ROUTER_MAP_TRANS }

template<class T>
class TransCmdTag : public MainCmdTag 
{
    public:
      static T _inst2;
      static MainCmd _mainc;
    TransCmdTag(){}

    static void MyInit() { _inst2.init(ServeMode::SBind); }

    virtual PackType getPackType() override{
      return PackType::FixMainSubHead;
    }

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

template<class T>
T TransCmdTag<T>::_inst2;

template<class T>
MainCmd TransCmdTag<T>::_mainc;

