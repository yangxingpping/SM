
#pragma once

#include "PackDealerBase.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerNoMain : public PackDealerBase
    {
    public:
        PackDealerNoMain(MainCmd mainc, ChannelType channel);
        PackDealerNoMain(const PackDealerNoMain& c);

        virtual MainCmd getMainc() override;
        virtual uint16_t getAssc() override;
        virtual void setAssc(uint16_t assc) override;
		virtual std::string_view getMsg() override;
		virtual std::string* unpack(std::string& pack)  override;
		virtual std::string* pack(std::string_view msg, uint16_t assc)  override;

        virtual asio::awaitable<std::string*> dealmsg(std::string& msg) override;
        virtual PackDealerBase* clone() override;
        
    private:
        ChannelType _channel;
        std::string _msg;
        uint16_t _assc;
        MainCmd _mainc;
    };

	

}
