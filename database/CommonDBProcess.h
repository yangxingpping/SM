
#pragma once

#include "TransCmdTag.h"
#include "templatefuncs.h"
#include "database.h"
#include "enums.h"
#include "coros.h"
#include "reqreps.h"
#include <string_view>

using std::string;
using std::string_view;

namespace SMDB
{
    class DATABASE_EXPORT CommonDBProcess : public TransCmdTag<CommonDBProcess, MainCmd>
    {
    public:
        BEGIN_ROUTER_MAP_DB(MainCmd::DBQuery);
        ROUTER_DB3(&CommonDBProcess::sregist, AssDB::Regist);
        ROUTER_DB3(&CommonDBProcess::slogin,  AssDB::Login);
        ROUTER_DB3(&CommonDBProcess::sgetAllUsers, AssDB::getAllUsers);
        ROUTER_DB3(&CommonDBProcess::saddSysAdmin, AssDB::AddSysAdmin);
        ROUTER_DB3(&CommonDBProcess::sgetSysAdmin, AssDB::GetSysAdmin);
        ROUTER_DB3(&CommonDBProcess::sdelSysAdmin, AssDB::DelSysAdmin);
        ROUTER_DB3(&CommonDBProcess::sgetUserGenInfo, AssDB::GetUserGenInfo);
        END_ROUTER_MAP_DB;
    public:

		RegistRep sregist(RegistReq req);
		LoginRep slogin(LoginReq req);
		getAllUserRep sgetAllUsers(getAllUserReq req);
		addSysAdminRep saddSysAdmin(addSysAdminReq req);
		getSysAdminRep sgetSysAdmin(getSysAdminReq req);
		delSysAdminRep sdelSysAdmin(delSysAdminReq req);
        getUserGenInfoRep sgetUserGenInfo(getUserGenInfoReq req);
        virtual void init(ServeMode mode) override;

    private:
        
    };
}
