#pragma once

#include "coros.h"
#include "database.h"
#include "wss.h"
#include "Utils.h"
#include "reqreps.h"
#include "HttpCmdTag.h"
#include "taskflow/taskflow.hpp"
#include <string>
#include <string_view>
#include <memory>

namespace SMDB
{

	class DBManager;

	class DealAuth : public HttpCmdTag<DealAuth, MainCmd>
	{
	public:
		DealAuth();
		virtual ~DealAuth();
	public:
		BEGIN_ROUTER_MAP(MainCmd::User);
		ROUTER_NEED_CACHE_DB_JSON(&DealAuth::lregist, RegistReq, RegistRep, AssAuth::Regist, AssDB::Regist);
		ROUTER_NEED_CACHE_DB_JSON(&DealAuth::llogin, LoginReq, LoginRep, AssAuth::Login, AssDB::Login);
		ROUTER_NEED_CACHE_DB_JSON(&DealAuth::laddSysAdmin, addSysAdminReq, addSysAdminRep, AssAuth::AddSysAdmin, AssDB::AddSysAdmin);
		ROUTER_NEED_CACHE_DB_JSON(&DealAuth::lgetSysAdmin, getSysAdminReq, getSysAdminRep, AssAuth::GetSysAdmin, AssDB::GetSysAdmin);
		ROUTER_NEED_CACHE_DB_JSON(&DealAuth::ldelSysAdmin, delSysAdminReq, delSysAdminRep, AssAuth::DelSysAdmin, AssDB::DelSysAdmin);
		ROUTER_NEED_CACHE_DB_JSON(&DealAuth::lgetUserGenInfo, getUserGenInfoReq, getUserGenInfoRep, AssAuth::GetUserGenInfo, AssDB::GetUserGenInfo);
		END_ROUTER_MAP;
		

		bool lregist(RegistReq& req, RegistRep& rep);
		bool llogin(LoginReq& req, LoginRep& rep);
		bool laddSysAdmin(addSysAdminReq& req, addSysAdminRep& rep);
		bool lgetSysAdmin(getSysAdminReq& req, getSysAdminRep& rep);
		bool ldelSysAdmin(delSysAdminReq& req, delSysAdminRep& rep);
		bool lgetUserGenInfo(getUserGenInfoReq& req, getUserGenInfoRep& rep);

		

		void init(ServeMode mode) override;

	};
}

