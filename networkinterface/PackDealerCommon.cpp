


#include "PackDealerCommon.h"

namespace SMNetwork
{
    
	PackDealerCommon::PackDealerCommon(ChannelType channel):_channel(channel)
	{

	}

	std::string_view PackDealerCommon::getMsg()
	{
		return string_view{ _msg };
	}

	seqNumType* PackDealerCommon::reqSeq()
	{
		return &_seqnumreq;
	}

	seqNumType* PackDealerCommon::repSeq()
	{
		return &_seqnumrep;
	}

	ChannelType PackDealerCommon::getChannelType()
	{
		return _channel;
	}

	PackDealerCommon::~PackDealerCommon()
	{

	}

}

