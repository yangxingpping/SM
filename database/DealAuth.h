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
		ROUTER_NEED_CACHE_DB_JSON3(&DealAuth::lregist, AssAuth::Regist, AssDB::Regist);
		ROUTER_NEED_CACHE_DB_JSON3(&DealAuth::llogin, AssAuth::Login, AssDB::Login);
		ROUTER_NEED_CACHE_DB_JSON3(&DealAuth::laddSysAdmin, AssAuth::AddSysAdmin, AssDB::AddSysAdmin);
		ROUTER_NEED_CACHE_DB_JSON3(&DealAuth::lgetSysAdmin, AssAuth::GetSysAdmin, AssDB::GetSysAdmin);
		ROUTER_NEED_CACHE_DB_JSON3(&DealAuth::ldelSysAdmin, AssAuth::DelSysAdmin, AssDB::DelSysAdmin);
		ROUTER_NEED_CACHE_DB_JSON3(&DealAuth::lgetUserGenInfo, AssAuth::GetUserGenInfo, AssDB::GetUserGenInfo);
		END_ROUTER_MAP;
		

		RegistRep lregist(RegistReq req);
		LoginRep llogin(LoginReq req);
		addSysAdminRep laddSysAdmin(addSysAdminReq req);
		getSysAdminRep lgetSysAdmin(getSysAdminReq req);
		delSysAdminRep ldelSysAdmin(delSysAdminReq req);
		getUserGenInfoRep lgetUserGenInfo(getUserGenInfoReq req);

		

		void init(ServeMode mode) override;

	};
}

