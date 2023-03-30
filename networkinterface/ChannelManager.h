#pragma once
#include "networkinterfaceExport.h"
#include <memory>
#include "myconcept.h"
#include "enums.h"
#include "magic_enum.hpp"
#include "concurrentqueue/concurrentqueue.h"
#include "spdlog/spdlog.h"
#include <map>
#include <any>

using std::shared_ptr;
using std::map;
using std::any;

namespace SMNetwork
{
	template<NetType net, EnvType env>
	class ChannelManager
	{
	public:
		void sInit();
		void sUninit();
		ChannelManager* getInst();
	public:
		ChannelManager();
		~ChannelManager();
		
	};
}
