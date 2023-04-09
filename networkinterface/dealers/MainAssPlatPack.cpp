#include "MainAssPlatPack.h"
#include "Utils.h"
#include "spdlog/spdlog.h"
#include <assert.h>


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
		, _ass(ref._ass)
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

	size_t MainAssPlatPack::HeadLen()
	{
		return sizeof(_main) + sizeof(_ass);
	}

	PlatformPackInterface* MainAssPlatPack::clone()
	{
		return new MainAssPlatPack(*this);
	}
}
