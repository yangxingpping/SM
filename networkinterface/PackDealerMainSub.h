
#pragma once

#include "PackDealerBase.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerMainSub : public PackDealerBase
    {
    public:
        PackDealerMainSub(MainCmd mainc, ChannelType channel);
        PackDealerMainSub(const PackDealerMainSub& c);
        virtual void setAssc(uint16_t) override;
        virtual uint16_t getAssc() override;
        virtual MainCmd getMainc() override;
		virtual std::string_view getMsg() override;
		virtual std::string* unpack(string& pack)  override;
        virtual std::string* unpack(std::string_view pack) override;
		virtual std::string* pack(std::string_view msg)  override;
        virtual std::string* pack(std::string_view msg, uint16_t assc) override;
        virtual asio::awaitable<std::string*> dealmsg(string& msg) override;
        virtual PackDealerBase* clone() override;
        
    private:
        ChannelType _channel;
        std::string _msg;
        uint16_t _assc;
        MainCmd _mainc;
    };

	

}
