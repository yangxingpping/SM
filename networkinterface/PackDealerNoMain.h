
#pragma once

#include "PackDealerCommon.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerNoMain : public PackDealerCommon
    {
    public:
        PackDealerNoMain(ChannelType channel);
        PackDealerNoMain(const PackDealerNoMain& c);

        virtual int getAssc() override;
        virtual void setAssc(int assc) override;
		virtual std::string* unpack(seqNumType* seqnum, std::string& pack)  override;
		virtual std::string* pack(seqNumType* seqnum, std::string_view msg, uint16_t assc)  override;

        virtual asio::awaitable<std::string*> dealmsg(std::string& msg) override;
        virtual PackDealerBase* clone() override;
        
    private:
        uint16_t _assc;
    };

	

}
