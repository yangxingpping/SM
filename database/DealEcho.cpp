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

	EchoRep DealEcho::lEcho(EchoReq req)
	{
		EchoRep rep;

		return rep;
	}

	

	void DealEcho::init(ServeMode mode) { }
}

