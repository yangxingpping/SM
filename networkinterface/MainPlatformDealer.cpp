
#include "MainPlatformDealer.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "hotupdate.h"
#include "Routers.h"
#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	MainPlatformDealer::MainPlatformDealer() :_main(-1)
	{

	}

	MainPlatformDealer::MainPlatformDealer(uint32_t mainc) :_main(mainc)
	{

	}

	uint32_t MainPlatformDealer::getMain()
	{
		return _main;
	}

	void MainPlatformDealer::setMain(uint32_t mainc)
	{
		_main = mainc;
	}

	int MainPlatformDealer::getAss()
	{
		assert(0);
		SPDLOG_WARN("this platform dealer only need main cmd, not need get ass cmd");
		return 0;
	}

	void MainPlatformDealer::setAss(int ass)
	{
		assert(0);
		SPDLOG_WARN("this platform dealer only need main cmd, not need set ass cmd");
	}

	

	void MainPlatformDealer::pack(uint32_t& no, size_t len, span<char> dst)
	{

	}

	uint32_t MainPlatformDealer::getNo()
	{
		return _no;
	}


	bool MainPlatformDealer::unpack(uint32_t& no, string_view src)
	{
		bool bret{ true };

		return bret;
	}

	int MainPlatformDealer::HeadLen()
	{
		return sizeof(int);
	}
	
}
