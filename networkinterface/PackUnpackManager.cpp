
#include "PackUnpackManager.h"

#include "Utils.h"

#include "spdlog/spdlog.h"
#include "magic_enum.hpp"

using std::make_shared;

namespace SMNetwork
{
	MainAssPlatPack::MainAssPlatPack(int mainc)
		: _main(mainc)
		, _ass(0)
	{
		assert(sizeof(int) == sizeof(uint32_t));
	}

	MainAssPlatPack::MainAssPlatPack(MainAssPlatPack& ref)
		:_main(ref._main)
		,_ass(ref._ass)
	{

	}

	MainAssPlatPack::~MainAssPlatPack()
	{

	}

	int MainAssPlatPack::getMain()
	{
		return static_cast<int>(_main);
	}

	void MainAssPlatPack::setAss(int ass)
	{
		_ass = static_cast<uint32_t>(ass);
	}

	int MainAssPlatPack::getAss()
	{
		return _ass;
	}

	bool MainAssPlatPack::pack(span<char> dst)
	{
		bool bret{ true };
		assert(dst.size() == len());
		if (dst.size() != len())
		{
			bret = false;
			SPDLOG_WARN("at pack platform pack head size {} != holder container size {}", len(), dst.size());
		}
		else
		{
			SMUtils::packuint32(span<char>(dst.data(), sizeof(uint32_t)), _main);
			SMUtils::packuint32(span<char>(dst.data() + sizeof(uint32_t), sizeof(uint32_t)), _ass);
		}
		return bret;
	}

	bool MainAssPlatPack::unpack(string_view src)
	{
		bool bret{ true };
		uint32_t temp;
		if (src.size() != len())
		{
			bret = false;
			SPDLOG_WARN("at unpack platform pack head size {} != holder container size {}", len(), src.size());
		}
		else
		{
			auto [bret, tail] = SMUtils::unpackuint32(src, temp);
			{
				_main = temp;
				auto [bret, _] = SMUtils::unpackuint32(tail, temp);
				_ass = temp;
			}
		}
		return bret;
	}

	size_t MainAssPlatPack::len()
	{
		return sizeof(_main) + sizeof(_ass);
	}

	SMNetwork::PlatformPackInterface* MainAssPlatPack::clone()
	{
		return new MainAssPlatPack(*this);
	}

	NoMainAssPlatPack::NoMainAssPlatPack(int mainc)
		: _main(mainc)
	{
		assert(sizeof(int) == sizeof(uint32_t));
	}

	NoMainAssPlatPack::NoMainAssPlatPack(NoMainAssPlatPack& ref)
		:_main(ref._main)
	{

	}

	NoMainAssPlatPack::~NoMainAssPlatPack()
	{

	}

	int NoMainAssPlatPack::getMain()
	{
		return static_cast<int>(_main);
	}

	

	bool NoMainAssPlatPack::pack(span<char> dst)
	{
		return true;
	}

	bool NoMainAssPlatPack::unpack(string_view src)
	{
		return true;
	}

	size_t NoMainAssPlatPack::len()
	{
		return 0;
	}

	SMNetwork::PlatformPackInterface* NoMainAssPlatPack::clone()
	{
		return new NoMainAssPlatPack(*this);
	}

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
		if (it != _platPacks.end())
		{
			ret = shared_ptr<PlatformPackInterface>(it->second->clone());
		}
		return ret;
	}

	shared_ptr<PlatformPackInterface> clonePlatformPack(int mainc)
	{
		return shared_ptr<PlatformPackInterface>(PackUnpackManager::sInst()->clone(mainc));
	}

}
