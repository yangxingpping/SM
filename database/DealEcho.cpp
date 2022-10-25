#include "DealEcho.h"
#include "reqreps.h"
#include "enums.h"
#include "Utils.h"
#include "DBManager.h"
#include "spdlog/spdlog.h"
#include "networkinterface.h"

namespace SMDB
{
	DealEcho::~DealEcho()
	{

	}

	bool DealEcho::Echo(EchoReq& req)
	{
		bool bret = false;
		bret = SMUtils::isjwttokenright(string_view{ req.token });
		return bret;
	}

	EchoRep DealEcho::lEcho(EchoReq& req)
	{
		EchoRep rep;

		return rep;
	}

	PackType DealEcho::getPackType() {
		return _packtype;
	}

	void DealEcho::init(ServeMode mode) { }
}

