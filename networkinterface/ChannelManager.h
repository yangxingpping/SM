
#pragma once

namespace SMNetwork
{
	class ChannelManager
	{
	public:
		ChannelManager() {}
		virtual ~ChannelManager() {}
		virtual bool IsInitiativeChannel() = 0;
	};
}
