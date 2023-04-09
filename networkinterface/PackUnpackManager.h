#pragma once
#include "networkinterfaceExport.h"
#include "myconcept.h"
#include "PlatformPackInterface.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"

#include <span>
#include <string_view>
#include <assert.h>
#include <stdint.h>
#include <map>
#include <memory>

using std::span;
using std::string_view;
using std::map;
using std::shared_ptr;

namespace SMNetwork
{
	
	class PackUnpackManager
	{
	public:
		static bool sInit();
		static PackUnpackManager* sInst();
		static bool sUninit();
	public:
		bool addPlatformPack(int mainc, shared_ptr<PlatformPackInterface> pack);
		shared_ptr<PlatformPackInterface> clone(int mainc);
	private:
		map<int, shared_ptr<PlatformPackInterface>> _platPacks;
	};

}

#define PUM (SMNetwork::PackUnpackManager::sInst())

