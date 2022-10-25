#pragma once

#include "database.h"
#include "Utils.h"
#include "reqreps.h"
#include "HttpCmdTag.h"
#include <memory>

namespace SMDB
{

	class DBManager;

	class DealEcho : public HttpCmdTag<DealEcho>
	{
	public:
		BEGIN_ROUTER_MAP(MainCmd::Echo);
		ADD_ROUTER_DYNAMIC_NEED_DB_JSON(&DealEcho::Echo, &DealEcho::lEcho, EchoReq, EchoRep, AssEcho::Echo, AssDB::DBEcho);
		END_ROUTER_MAP;
		virtual ~DealEcho();

		bool Echo(EchoReq& req);

		EchoRep lEcho(EchoReq& req);

		PackType getPackType() override;
		void init(ServeMode mode) override;

	private:
		PackType _packtype;
	};
}


