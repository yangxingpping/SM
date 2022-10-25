
#pragma once
#include "templatefuncs.h"
#include "magic_enum.hpp"
#include "database.h"
#include "MainCmdTag.h"
#include "enums.h"
#include "Utils.h"
#include "networkinterface.h"
#include "asio/awaitable.hpp"
#include "IOContextManager.h"
#include "DBConnectManager.h"
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

//template<class ParamterType, class ReturnType, enum AssDB assDB>
//shared_ptr<RouterFuncType> 

/**
 * only need send query to database instance, request parameter and response parameter need not modify
 */
#define ADD_ROUTER_ONLY_NEED_DB_JSON(ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
				if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
				{\
					SPDLOG_WARN("execute db query failed"); \
				}\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

 /**
  * parameter send to database instance need call func to convert  DBParamterType
  */
#define ADD_ROUTER_MODIFY_QUERY_REQ_NEED_DB_JSON(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
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
                DBParamterType dbreq = vv(req);\
			    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
			    {\
				    SPDLOG_WARN("execute db query failed");\
			    }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

  /**
   * need use func to check whether this request need send database query to database instance
   */
#define ADD_ROUTER_DYNAMIC_NEED_DB_JSON(needQuery, localQuery, ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
                auto vv = std::bind(needQuery, &_inst2, std::placeholders::_1);\
                if (vv(req))\
                {\
				    if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
                else\
                {\
                    ret = std::invoke(localQuery, &_inst2, req);\
                }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_DBREQ_JSON(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
				    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_POST_JSON(func, funcpost, ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
                if (vv(req))\
                {\
				    if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
                auto vvpost = std::bind(funcpost, &_inst2, std::placeholders::_1);\
                vvpost(ret);\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_DBREQ_POST_JSON(func, funcpost, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
				    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
                auto vvpost = std::bind(funcpost, &_inst2, std::placeholders::_1);\
                vvpost(ret);\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER2_JSON(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
				    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_RETURN_STR_JSON(func, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
            string strret = co_await vv(msg);\
            co_return strret;\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_DB_JSON(func, ParamterType, ReturnType, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
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
                bool bret = SMDB::beforeQuery();\
                if(!bret) \
                {\
                    jwtCheckFailedJSONRep invalidrep;\
                    co_return my_to_string(invalidrep);\
                }\
			    auto vv = std::bind(func, &_inst2, std::placeholders::_1); \
			    ret = co_await vv(req); \
			    SMDB::afterQuery(); \
            }\
            co_return my_to_string(ret);\
        });\
        SMNetwork::addRouterJson(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }


   /**
	* only need send query to database instance, request parameter and response paramter need not modify
	*/
#define ADD_ROUTER_ONLY_NEED_DB_HTML(ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
			    if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
			    {\
				    SPDLOG_WARN("execute db query failed");\
			    }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }


	/**
	 * only need jwt check and then send query to database instance, request parameter and response paramter need not modify
	 */
#define ADD_ROUTER_ONLY_NEED_JWT_AND_DB_HTML(ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                if(!SMUtils::isjwttokenright(string_view{req.token}))\
                {\
                    jwtCheckFailedJSONRep defaultrep;\
                    SPDLOG_WARN("jwt check failed {}", msg);\
                    co_return my_to_string(defaultrep);\
                }\
                ReturnType ret;\
			    if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
			    {\
				    SPDLOG_WARN("execute db query failed");\
			    }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

	 /**
	  * parameter send to database instance need call func to convert  DBParamterType
	  */
#define ADD_ROUTER_MODIFY_QUERY_REQ_NEED_DB_HTML(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                DBParamterType dbreq = vv(req);\
			    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
			    {\
				    SPDLOG_WARN("execute db query failed");\
			    }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

	  /**
	   * need database req and database rep and need convert from req to database req and from database rep to rep
	   */
#define ADD_ROUTER_MODIFY_QUERY_REQ_REP_NEED_DB_HTML(reqconv, repconv, ParamterType, ReturnType, DBReqType, DBRepType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            DBRepType dbrep;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(reqconv, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                DBReqType dbreq = vv(req);\
			    if (!co_await DBCMGRREF.executeQuery(dbreq, dbrep, AssDBTypeVar))\
			    {\
				    SPDLOG_WARN("execute db query failed");\
			    }\
                auto vv2 = std::bind(repconv, &_inst2, std::placeholders::_1);\
                ret = vv2(dbrep);\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }


	   /**
		* no need send database query to database instance, only need call func and send return value back
		*/
#define ADD_ROUTER_NO_NEED_DB_HTML(func, ParamterType, ReturnType) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret = vv(req);\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("deal req {} from local get rep {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

		/**
		 * need use func to check whether this request need send database query to database instance
		 */
#define ADD_ROUTER_DYNAMIC_NEED_DB_HTML(needQuery, localQuery, ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(needQuery, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                if (vv(req))\
                {\
				    if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
                else\
                {\
                    ret = std::invoke(localQuery, &_inst2, req);\
                }\
            }\
            strret = my_to_string(ret);\
            SPDLOG_INFO("execute query {} get response {}", msg, strret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_DBREQ_HTML(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                auto dbreq = vv(req);\
                {\
				    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_POST_HTML(func, funcpost, ParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                if (vv(req))\
                {\
				    if (!co_await DBCMGRREF.executeQuery(req, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
                auto vvpost = std::bind(funcpost, &_inst2, std::placeholders::_1);\
                vvpost(ret);\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_DBREQ_POST_HTML(func, funcpost, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                auto dbreq = vv(req);\
                {\
				    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
                auto vvpost = std::bind(funcpost, &_inst2, std::placeholders::_1);\
                vvpost(ret);\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER2_HTML(func, ParamterType, DBParamterType, ReturnType, AssTypeVar, AssDBTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                auto vv = std::bind(func, &_inst2, std::placeholders::_1);\
                ReturnType ret;\
                auto dbreq = vv(req);\
                {\
				    if (!co_await DBCMGRREF.executeQuery(dbreq, ret, AssDBTypeVar))\
				    {\
					    SPDLOG_WARN("execute db query failed");\
				    }\
                }\
            }\
            strret = my_to_string(ret);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_RETURN_STR_HTML(func, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
            string strret = co_await vv(msg);\
            co_return strret;\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }

#define ADD_ROUTER_DB_HTML(func, ParamterType, ReturnType, AssTypeVar) {\
    auto lambdax = make_shared<RouterFuncType>([](string msg, string token) -> asio::awaitable<string> {\
            ParamterType req;\
            ReturnType ret;\
            string strret;\
            if (!my_json_parse_from_string(req, msg)) \
            {\
                ret.code = to_underlying(statusCode::invalidJson);\
            }\
            else\
            {\
                bool bret = SMDB::beforeQuery();\
                if(!bret) \
                {\
                  co_return my_to_string(jwtCheckFailedJSONRep());\
                }\
                auto vv = std::bind(func, &_inst2,  std::placeholders::_1);\
                SMDB::afterQuery();\
                ReturnType ret = co_await vv(req);\
            }\
            co_return my_to_string(ret);\
        });\
        SMNetwork::addRouterHtml(SMNetwork::combinePath(_mainc, AssTypeVar), lambdax); }


#define END_ROUTER_MAP }

template<class T>
class HttpCmdTag : public MainCmdTag
{
public:
	static T _inst2;
	static MainCmd _mainc;
	HttpCmdTag() {}


	static T& Inst() { return _inst2; }
	static void MyInit() { _inst2.init(ServeMode::SBind); }
	static MainCmd GetMainCmd() { return _mainc; }

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

template<class T>
T HttpCmdTag<T>::_inst2;

template<class T>
MainCmd HttpCmdTag<T>::_mainc;

