#pragma once

#include "database.h"
#include "Utils.h"
#include "reqreps.h"
#include "HttpCmdTag.h"
#include <memory>

namespace SMDB
{

	class DBManager;

	class DealEcho : public HttpCmdTag<DealEcho, MainCmd>
	{
	public:
		BEGIN_ROUTER_MAP(MainCmd::Echo);
		ROUTER_NEED_CACHE_JSON(&DealEcho::lEcho, EchoReq, EchoRep, AssEcho::Echo, AssDB::DBEcho);
		END_ROUTER_MAP;
		virtual ~DealEcho();

		EchoRep lEcho(EchoReq& req);

	
		void init(ServeMode mode) override;
	};
}


