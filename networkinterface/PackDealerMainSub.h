
#pragma once

#include "PackDealerCommon.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerMainSub : public PackDealerCommon
    {
    public:
        PackDealerMainSub(MainCmd mainc, ChannelType channel);
        PackDealerMainSub(const PackDealerMainSub& c);
        virtual void setAssc(uint16_t) override;
        virtual uint16_t getAssc() override;
        virtual MainCmd getMainc() override;
		
		virtual std::string* unpack(seqNumType* seqnum, string& pack)  override;
        virtual std::string* unpack(seqNumType* seqnum, std::string_view pack) override;
		virtual std::string* pack(seqNumType* seqnum, std::string_view msg)  override;
        virtual std::string* pack(seqNumType* seqnum, std::string_view msg, uint16_t assc) override;
        virtual asio::awaitable<std::string*> dealmsg(string& msg) override;
        virtual PackDealerBase* clone() override;
        
    private:
        uint16_t _assc;
        MainCmd _mainc;
        
    };

	

}
