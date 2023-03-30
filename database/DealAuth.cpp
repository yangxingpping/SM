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

	RegistRep DealAuth::lregist(RegistReq req)
	{
		RegistRep rep;
		rep.code = magic_enum::enum_integer(statusCode::needNextRouter);
		return rep;
	}

	LoginRep DealAuth::llogin(LoginReq req)
	{
		LoginRep rep;
		rep.code = magic_enum::enum_integer(statusCode::needNextRouter);
		return rep;
	}

	addSysAdminRep DealAuth::laddSysAdmin(addSysAdminReq req)
	{
		addSysAdminRep rep;
		auto bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
		}
		return rep;
	}

	getSysAdminRep DealAuth::lgetSysAdmin(getSysAdminReq req)
	{
		getSysAdminRep rep;
		auto bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
		}
		return rep;
	}

	delSysAdminRep DealAuth::ldelSysAdmin(delSysAdminReq req)
	{
		delSysAdminRep rep;
		auto bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
		}
		return rep;
	}

	getUserGenInfoRep DealAuth::lgetUserGenInfo(getUserGenInfoReq req)
	{
		getUserGenInfoRep rep;
		auto bret = SMNetwork::isjwttokenright(string_view{ req.token });
		if (!bret)
		{
			rep.code = magic_enum::enum_integer(statusCode::jwtCheckFailed);
		}
		return rep;
	}

	

	void DealAuth::init(ServeMode mode)
	{

	}
}


