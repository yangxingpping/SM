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

	bool DealAuth::regist(RegistReq& req)
	{
		bool bret = true;
		return bret;
	}

	bool DealAuth::login(LoginReq& req)
	{
		bool bret = true;
		return bret;
	}

	bool DealAuth::addSysAdmin(addSysAdminReq& req)
	{
		bool bret = true;
		bret = SMUtils::isjwttokenright(string_view{ req.token });
		return bret;
	}

	bool DealAuth::getSysAdmin(getSysAdminReq& req)
	{
		bool bret = true;
		bret = SMUtils::isjwttokenright(string_view{ req.token });
		return bret;
	}

	bool DealAuth::delSysAdmin(delSysAdminReq& req)
	{
		bool bret = true;
		bret = SMUtils::isjwttokenright(string_view{ req.token });
		return bret;
	}

	bool DealAuth::getUserGenInfo(getUserGenInfoReq& req)
	{
		bool bret = true;
		bret = SMUtils::isjwttokenright(string_view{ req.token });
		return bret;
	}

	RegistRep DealAuth::lregist(RegistReq& req)
	{
		RegistRep rep;

		return rep;
	}

	LoginRep DealAuth::llogin(LoginReq& req)
	{
		LoginRep rep;

		return rep;
	}

	addSysAdminRep DealAuth::laddSysAdmin(addSysAdminReq& req)
	{
		addSysAdminRep rep;

		return rep;
	}

	getSysAdminRep DealAuth::lgetSysAdmin(getSysAdminReq& req)
	{
		getSysAdminRep rep;

		return rep;
	}

	delSysAdminRep DealAuth::ldelSysAdmin(delSysAdminReq& req)
	{
		delSysAdminRep rep;

		return rep;
	}

	getUserGenInfoRep DealAuth::lgetUserGenInfo(getUserGenInfoReq& req)
	{
		getUserGenInfoRep rep;

		return rep;
	}

	PackType DealAuth::getPackType() {
		return _packtype;
	}

	void DealAuth::init(ServeMode mode)
	{

	}
}


