
#include "PackDealerNoMain.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"

#include "IOContextManager.h"

#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	std::string* PackDealerNoMain::unpack(std::string& pack)
	{
		thread_local string ret;
		ret.clear();
		uint16_t assc{ 0xffff };
		size_t len;
		if (!SMUtils::uppackmaincmd3(len, assc, pack, ret))
		{
			SPDLOG_WARN("unpack recv data failed");
		}
		return &ret;
	}

	std::string* PackDealerNoMain::pack(std::string_view msg, uint16_t assc)
	{
		thread_local string ret;
		ret.clear();
		SMUtils::packmaincmd3(assc, ret, msg);
		return &ret;
	}

	asio::awaitable<std::string*> PackDealerNoMain::dealmsg(std::string& msg)
	{
		thread_local std::string drep;
		drep.clear();
		PtTransRouterElement funcs = getRouterTransByMainC(_mainc);
		SPDLOG_INFO("channel type {}", magic_enum::enum_name(_channel));

		string strreq;
		auto pstrreq = unpack(msg);
		auto func2 = funcs->find(_assc);
		assert(func2 != funcs->end());
		if (func2 == funcs->end())
		{
			SPDLOG_ERROR("  templ mainc = {}, assc = {}, deal func = {}", magic_enum::enum_name(_mainc), _assc, fmt::ptr(funcs));
			co_return nullptr;
		}
		string ret;
		string token;
		BEGIN_ASIO;
		ret = co_await(*(func2->second))(*pstrreq, token);
		END_ASIO;
		SPDLOG_INFO("send to client data: [{}], size:[{}]", ret, ret.length());
		if (ret.empty())
		{
			SPDLOG_ERROR("  mainc = {}, assc = {} response 0", magic_enum::enum_name(_mainc), _assc);
			co_return nullptr;
		}
		auto drep2 = pack(string_view(ret), _assc);

		co_return drep2;

	}


	SMNetwork::PackDealerBase* PackDealerNoMain::clone()
	{
		PackDealerNoMain* ret = new PackDealerNoMain(*this);
		return ret;
	}

	PackDealerNoMain::PackDealerNoMain(MainCmd mainc, ChannelType channel) : _channel(channel), _mainc(mainc)
	{
		PtTransRouterElement funcs = getRouterTransByMainC(_mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(_mainc));
		}
	}


	PackDealerNoMain::PackDealerNoMain(const PackDealerNoMain& c)
	{
		this->_mainc = c._mainc;
		this->_channel = c._channel;
		PtTransRouterElement funcs = getRouterTransByMainC(this->_mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(this->_mainc));
		}
	}

	MainCmd PackDealerNoMain::getMainc()
	{
		return _mainc;
	}

	uint16_t PackDealerNoMain::getAssc()
	{
		return _assc;
	}

	void PackDealerNoMain::setAssc(uint16_t assc)
	{
		_assc = assc;
	}

	std::string_view PackDealerNoMain::getMsg()
	{
		return string_view{ _msg};
	}

	
}
