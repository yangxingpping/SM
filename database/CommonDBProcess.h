
#pragma once

#include "TransCmdTag.h"
#include "templatefuncs.h"
#include "database.h"
#include "enums.h"
#include "coros.h"
#include "reqreps.h"
#include "sha256.h"
#include <string_view>

using std::string;
using std::string_view;

namespace SMDB
{
    class DATABASE_EXPORT CommonDBProcess : public TransCmdTag<CommonDBProcess>
    {
    public:
        BEGIN_ROUTER_MAP_TRANS(MainCmd::DBQuery);
        ADD_ROUTER_DB_TRANS_NO_BEFORE_QUERY(&CommonDBProcess::sregist, RegistReq, RegistRep, AssDB::Regist);
        ADD_ROUTER_DB_TRANS_NO_BEFORE_QUERY(&CommonDBProcess::slogin, LoginReq, LoginRep, AssDB::Login);
        ADD_ROUTER_DB_TRANS(&CommonDBProcess::sgetAllUsers, getAllUserReq, getAllUserRep, AssDB::getAllUsers);
        ADD_ROUTER_DB_TRANS(&CommonDBProcess::saddSysAdmin, addSysAdminReq, addSysAdminRep, AssDB::AddSysAdmin);
        ADD_ROUTER_DB_TRANS(&CommonDBProcess::sgetSysAdmin, getSysAdminReq, getSysAdminRep, AssDB::GetSysAdmin);
        ADD_ROUTER_DB_TRANS(&CommonDBProcess::sdelSysAdmin, delSysAdminReq, delSysAdminRep, AssDB::DelSysAdmin);
        ADD_ROUTER_DB_TRANS(&CommonDBProcess::sgetUserGenInfo, getUserGenInfoReq, getUserGenInfoRep, AssDB::GetUserGenInfo);
        END_ROUTER_MAP_TRANS;
    public:

		RegistRep sregist(RegistReq req);
		LoginRep slogin(LoginReq req);
		getAllUserRep sgetAllUsers(getAllUserReq req);
		addSysAdminRep saddSysAdmin(addSysAdminReq req);
		getSysAdminRep sgetSysAdmin(getSysAdminReq req);
		delSysAdminRep sdelSysAdmin(delSysAdminReq req);
        getUserGenInfoRep sgetUserGenInfo(getUserGenInfoReq& req);
        virtual void init(ServeMode mode) override;

    private:
        SHA256 _sha256;
    };
}
