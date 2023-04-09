
#include "PackUnpackManager.h"

#include "Utils.h"

#include "spdlog/spdlog.h"
#include "magic_enum.hpp"

using std::make_shared;

namespace SMNetwork
{
	shared_ptr<PackUnpackManager> _sInst{nullptr};

	bool PackUnpackManager::sInit()
	{
		if (_sInst != nullptr)
		{
			return false;
		}
		_sInst = make_shared<PackUnpackManager>();
		return true;
	}

	PackUnpackManager* PackUnpackManager::sInst()
	{
		assert(_sInst != nullptr);
		if (_sInst == nullptr)
		{
			SPDLOG_WARN("PackUnpackManager instance is null");
			return nullptr;
		}
		return _sInst.get();
	}

	bool PackUnpackManager::sUninit()
	{
		if (_sInst == nullptr)
		{
			return false;
		}
		_sInst = nullptr;
		return true;
	}

	bool PackUnpackManager::addPlatformPack(int mainc, shared_ptr<PlatformPackInterface> pack)
	{
		bool bret{ false };
		if (_platPacks.find(mainc) != _platPacks.end())
		{
			SPDLOG_WARN("main cmd {} have other platform pack", mainc);
			return bret;
		}
		_platPacks[mainc] = pack;
		bret = true;
		return bret;
	}

	shared_ptr<PlatformPackInterface> PackUnpackManager::clone(int mainc)
	{
		shared_ptr<PlatformPackInterface> ret{nullptr};
		auto it = _platPacks.find(mainc);
		assert(it != _platPacks.end());
		if (it != _platPacks.end())
		{
			ret = shared_ptr<PlatformPackInterface>(it->second->clone());
		}
		return ret;
	}

}
