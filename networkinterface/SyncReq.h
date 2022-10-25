
#pragma once

#include "networkinterfaceExport.h"
#include "NNGCommBase.h"
#include "PackDealerBase.h"
#include "enums.h"
#include <string>
#include <memory>
#include <string_view>

using std::string;
using std::string_view;
using std::shared_ptr;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT SyncReq : public NNGCommBase
	{
	public:
		SyncReq(string ip, uint16_t port, ChannelType ctype, NNgTransType trans = NNgTransType::TCP, NngSockImplType socktype = NngSockImplType::NormalSock);
        virtual void init(ServeMode mode) override;
        void init(ServeMode mode, shared_ptr<PackDealerBase> dealer);
		std::string reqrep(string_view req, uint16_t assc);
	private:
		std::shared_ptr<PackDealerBase> _dealer;
		ServeMode _mode;
    };
}
