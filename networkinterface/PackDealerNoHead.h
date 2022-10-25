
#pragma once

#include "PackDealerBase.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerNoHead : public PackDealerBase
    {
    public:
        PackDealerNoHead(ChannelType channel);
        PackDealerNoHead(const PackDealerNoHead& c);

        virtual MainCmd getMainc() override;
		virtual std::string_view getMsg() override;
		virtual std::string* unpack(std::string& pack)  override;
        virtual std::string* unpack(std::string_view pack) override;
		virtual std::string* pack(std::string_view msg)  override;
        virtual asio::awaitable<std::string*> dealmsg(string& msg) override;
        virtual PackDealerBase* clone() override;
        
    private:
        ChannelType _channel;
        std::string _msg;
    };

	

}
