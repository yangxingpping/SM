
#include "PackDealerMainSub.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "Utils.h"
#include "fmt/format.h"

#include "IOContextManager.h"

#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	std::string* PackDealerMainSub::unpack(string& pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		MainCmd mainc{MainCmd::Invalid};
		uint16_t assc{ 0xffff };
		bool bsucc = SMUtils::uppackmaincmd3(mainc, assc, len, pack, ret);
		assert(assc != 0xffff);
		assert(_mainc == mainc);
		assert(bsucc);
		if (!bsucc || _mainc != mainc)
		{
			return nullptr;
		}
		_assc = assc;
		ret.insert(ret.end(), pack.begin(), pack.end());
		return &ret;
	}

	std::string* PackDealerMainSub::unpack(std::string_view pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		MainCmd mainc{ MainCmd::Invalid };
		uint16_t assc{ 0xffff };
		auto bsucc = SMUtils::uppackmaincmd3(mainc, assc, len, pack, ret);
		assert(bsucc);
		assert(mainc == _mainc);
		if (!bsucc || mainc != _mainc)
		{
			return nullptr;
		}
		_assc = assc;
		return &ret;
	}

	std::string* PackDealerMainSub::pack(std::string_view msg)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(_mainc, _assc, ret, msg);
		return &ret;
	}

	std::string* PackDealerMainSub::pack(std::string_view msg, uint16_t assc)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(_mainc, assc, ret, msg);
		return &ret;
	}

	asio::awaitable<std::string*> PackDealerMainSub::dealmsg(std::string& msg)
	{
		PtTransRouterElement funcs = getRouterTransByMainC(_mainc);
		string strreq;
		auto pstrreq = unpack(msg);
		SPDLOG_INFO("channel type {}, ass {}, dealer pt {}", magic_enum::enum_name(_channel), _assc, fmt::ptr(this));
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

	SMNetwork::PackDealerBase* PackDealerMainSub::clone()
	{
		PackDealerMainSub* ret = new PackDealerMainSub(*this);
		return ret;
	}

	PackDealerMainSub::PackDealerMainSub(MainCmd mainc, ChannelType channel) : _channel(channel), _mainc(mainc)
	{
		PtTransRouterElement funcs = getRouterTransByMainC(_mainc);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(_mainc));
		}
		assert(funcs != nullptr);
	}


	PackDealerMainSub::PackDealerMainSub(const PackDealerMainSub& c)
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

	void PackDealerMainSub::setAssc(uint16_t assc)
	{
		_assc = assc;
	}

	uint16_t PackDealerMainSub::getAssc()
	{
		return 0;
	}

	MainCmd PackDealerMainSub::getMainc()
	{
		return _mainc;
	}

	std::string_view PackDealerMainSub::getMsg()
	{
		return string_view{ _msg};
	}

	
}
