
#include "MainAssPlatformDealer.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "hotupdate.h"
#include "Routers.h"
#include <string>
#include <cassert>

using std::string;
using std::span;

namespace SMNetwork
{
	MainAssPlatformDealer::MainAssPlatformDealer(MainCmd mainc) :_main(mainc)
	{

	}

	MainAssPlatformDealer::MainAssPlatformDealer()
	{

	}

	MainCmd MainAssPlatformDealer::getMain()
	{
		return _main;
	}

	void MainAssPlatformDealer::setMain(MainCmd mainc)
	{
		_main = mainc;
	}

	int MainAssPlatformDealer::getAss()
	{
		return _ass;
	}

	void MainAssPlatformDealer::setAss(int ass)
	{
		_ass = ass;
	}

	bool MainAssPlatformDealer::pack(std::span<char> dst)
	{
		bool bret{ true };
		assert(dst.size() == HeadLen());
		if (dst.size() != HeadLen())
		{
			bret = false;
			SPDLOG_WARN("at pack platform pack head size {} != holder container size {}", HeadLen(), dst.size());
		}
		else
		{
			SMUtils::packuint32(span<char>(dst.data(), sizeof(uint32_t)), magic_enum::enum_integer(_main));
			SMUtils::packuint32(span<char>(dst.data() + sizeof(uint32_t), sizeof(uint32_t)), _ass);
		}
		return bret;
	}

	bool MainAssPlatformDealer::unpack(string_view src)
	{
		bool bret{ true };
		uint32_t temp;
		if (src.size() != HeadLen())
		{
			bret = false;
			SPDLOG_WARN("at unpack platform pack head size {} != holder container size {}", HeadLen(), src.size());
		}
		else
		{
			auto [bret, tail] = SMUtils::unpackuint32(src, temp);
			if (!bret)
			{
				SPDLOG_WARN("unpack main cmd failed");
			}
			else
			{
				auto etemp = magic_enum::enum_cast<MainCmd>(temp);
				if (!etemp.has_value())
				{
					SPDLOG_WARN("unpack main cmd value {} invalid enum value", temp);
					bret = false;
				}
				else
				{
					_main = etemp.value();
					auto [bret, _] = SMUtils::unpackuint32(tail, temp);
					if (!bret)
					{
						SPDLOG_WARN("unpack ass cmd failed");
					}
					else
					{
						_ass = temp;
					}
				}
			}
		}
		return bret;
	}

	int MainAssPlatformDealer::HeadLen()
	{
		return sizeof(int) + sizeof(_ass);
	}
	
}
