
#pragma once

#include "PackDealerBase.h"

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT PackDealerCommon : public PackDealerBase
    {
    public:
        PackDealerCommon(ChannelType channel);
        virtual std::string_view getMsg() override;
        virtual seqNumType* reqSeq() override;
        virtual seqNumType* repSeq() override;
        virtual ~PackDealerCommon();
    protected:
        seqNumType _seqnumreq;
        seqNumType _seqnumrep;
        ChannelType _channel;
        std::string _msg;
    };
}

