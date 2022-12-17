
#pragma once

#include "PackDealerCommon.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerNoHead : public PackDealerCommon
    {
    public:
        PackDealerNoHead(ChannelType channel);
        PackDealerNoHead(const PackDealerNoHead& c);

		virtual std::string* unpack(seqNumType* seqnum, std::string& pack)  override;
        virtual std::string* unpack(seqNumType* seqnum, std::string_view pack) override;
		virtual std::string* pack(seqNumType* seqnum, std::string_view msg)  override;
        virtual asio::awaitable<std::string*> dealmsg(string& msg) override;
        virtual PackDealerBase* clone() override;
    };

	

}
