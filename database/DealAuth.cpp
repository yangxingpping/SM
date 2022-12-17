#include "DealAuth.h"
#include "reqreps.h"
#include "enums.h"
#include "Utils.h"
#include "DBManager.h"
#include "spdlog/spdlog.h"
#include "networkinterface.h"
#include "DBConnectManager.h"
#include "dbCommon.h"

namespace SMDB
{
	DealAuth::DealAuth()
	{

	}

	DealAuth::~DealAuth()
	{

	}

	bool DealAuth::lregist(RegistReq& req, RegistRep& rep)
	{
		bool bret = true;
		return bret;
	}

	bool DealAuth::llogin(LoginReq& req, LoginRep& rep)
	{
		bool bret = true;
		return bret;
	}

	bool DealAuth::laddSysAdmin(addSysAdminReq& req, addSysAdminRep& rep)
	{
		bool bret = true;
		bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
			return bret;
		}
		return bret;
	}

	bool DealAuth::lgetSysAdmin(getSysAdminReq& req, getSysAdminRep& rep)
	{
		bool bret = true;
		bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
			return bret;
		}
		return bret;
	}

	bool DealAuth::ldelSysAdmin(delSysAdminReq& req, delSysAdminRep& rep)
	{
		bool bret = true;
		bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
			return bret;
		}
		return bret;
	}

	bool DealAuth::lgetUserGenInfo(getUserGenInfoReq& req, getUserGenInfoRep& rep)
	{
		bool bret = true;
		bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
			return bret;
		}
		return bret;
	}

	

	void DealAuth::init(ServeMode mode)
	{

	}
}


