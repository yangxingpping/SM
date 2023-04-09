#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "magic_enum.hpp"
#include "myconcept.h"
#include "hotupdate.h"
#include "ChannelCombine.h"
#include "ReqImpl.h"
#include "LocalNetManager.h"
#include <memory>
#include <vector>
#include <string_view>
#include <list>

using std::shared_ptr;
using std::vector;
using std::string;
using std::string_view;
using std::list;

namespace SMNetwork
{

	template <ChannelModeC modelc, EnumConcept MainType>
	class ReqManager
	{
	public:
		ReqManager(string_view addr, MainType mainc)
			:_addr(addr)
			,_mainc(mainc)
		{
			
		}
		asio::awaitable<ChannelCombine<ReqImpl>*> getChannel()
		{
			ChannelCombine<ReqImpl>* ret{nullptr};
			if (_channelPools.empty())
			{
				ret = LNM->createReq(string_view(_addr), modelc, magic_enum::enum_integer(_mainc));
			}
			else
			{
				ret = _channelPools.front();
				_channelPools.pop_front();
			}
			co_return ret;
		}
	private:
		string _addr;
		list<ChannelCombine<ReqImpl>*> _channelPools;

		MainType _mainc;
	};
}
