/*****************************************************************//**
 * \file   PackDealerCommon.h
 * \brief  common implement for all pack dealers
 * 
 * \author alqaz
 * \date   February 2023
 *********************************************************************/

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
        virtual ChannelType getChannelType() override;
        virtual ~PackDealerCommon();
    protected:
        seqNumType _seqnumreq{ 0 };
        seqNumType _seqnumrep{ 0 };
        ChannelType _channel;
        std::string _msg;
    };
}

