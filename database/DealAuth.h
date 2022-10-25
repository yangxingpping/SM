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

	class DealAuth : public HttpCmdTag<DealAuth>
	{
	public:
		DealAuth();
		virtual ~DealAuth();
	public:
		BEGIN_ROUTER_MAP(MainCmd::User);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealAuth::regist, &DealAuth::lregist, RegistReq, RegistRep, AssAuth::Regist, AssDB::Regist);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealAuth::login, &DealAuth::llogin, LoginReq, LoginRep, AssAuth::Login, AssDB::Login);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealAuth::addSysAdmin, &DealAuth::laddSysAdmin, addSysAdminReq, addSysAdminRep, AssAuth::AddSysAdmin, AssDB::AddSysAdmin);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealAuth::getSysAdmin, &DealAuth::lgetSysAdmin, getSysAdminReq, getSysAdminRep, AssAuth::GetSysAdmin, AssDB::GetSysAdmin);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealAuth::delSysAdmin, &DealAuth::ldelSysAdmin, delSysAdminReq, delSysAdminRep, AssAuth::DelSysAdmin, AssDB::DelSysAdmin);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealAuth::getUserGenInfo, &DealAuth::lgetUserGenInfo, getUserGenInfoReq, getUserGenInfoRep, AssAuth::GetUserGenInfo, AssDB::GetUserGenInfo);
		END_ROUTER_MAP;
		

		bool regist(RegistReq& req);
		bool login(LoginReq& req);
		bool addSysAdmin(addSysAdminReq& req);
		bool getSysAdmin(getSysAdminReq& req);
		bool delSysAdmin(delSysAdminReq& req);
		bool getUserGenInfo(getUserGenInfoReq& req);

		RegistRep lregist(RegistReq& req);
		LoginRep llogin(LoginReq& req);
		addSysAdminRep laddSysAdmin(addSysAdminReq& req);
		getSysAdminRep lgetSysAdmin(getSysAdminReq& req);
		delSysAdminRep ldelSysAdmin(delSysAdminReq& req);
		getUserGenInfoRep lgetUserGenInfo(getUserGenInfoReq& req);

		PackType getPackType() override;
		void init(ServeMode mode) override;

	private:
		PackType _packtype;
	};
}

