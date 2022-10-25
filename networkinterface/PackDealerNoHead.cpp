
#include "PackDealerNoHead.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "Utils.h"

#include "IOContextManager.h"

#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	std::string* PackDealerNoHead::unpack(string& pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		if (!SMUtils::uppackmaincmd3(len, pack, ret))
		{
			SPDLOG_WARN("unpack recv data failed");
		}
		return &ret;
	}

	std::string* PackDealerNoHead::unpack(std::string_view pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		if (!SMUtils::uppackmaincmd3(len, pack, ret))
		{
			SPDLOG_WARN("unpack recv data failed");
		}
		return &ret;
	}

	std::string* PackDealerNoHead::pack(std::string_view msg)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(ret, msg);
		ret.insert(ret.end(), msg.begin(), msg.end());
		return &ret;
	}

	asio::awaitable<std::string*> PackDealerNoHead::dealmsg(string& msg)
	{
		auto mainc = MainCmd::Invalid;
		PtTransRouterElement funcs = getRouterTransByMainC(mainc);
		SPDLOG_INFO("channel type {}", magic_enum::enum_name(_channel));

		string strreq;
		auto pstrreq = unpack(msg);
		auto func2 = funcs->begin();
		assert(func2 != funcs->end());
		if (func2 == funcs->end())
		{
			SPDLOG_ERROR("  templ mainc = {},  deal func = {}", magic_enum::enum_name(mainc), fmt::ptr(funcs));
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
			SPDLOG_ERROR("  mainc = {} response 0", magic_enum::enum_name(mainc));
			co_return nullptr;
		}
		auto drep2 = pack(string_view(ret));

		co_return drep2;

	}

	SMNetwork::PackDealerBase* PackDealerNoHead::clone()
	{
		PackDealerNoHead* ret = new PackDealerNoHead(*this);
		return ret;
	}

	PackDealerNoHead::PackDealerNoHead( ChannelType channel) : _channel(channel)
	{
		auto mainc = MainCmd::Invalid;
		PtTransRouterElement funcs = getRouterTransByMainC(mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(mainc));
		}
	}


	PackDealerNoHead::PackDealerNoHead(const PackDealerNoHead& c)
	{
		auto mainc = MainCmd::Invalid;
		this->_channel = c._channel;
		PtTransRouterElement funcs = getRouterTransByMainC(mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(mainc));
		}
	}

	MainCmd PackDealerNoHead::getMainc()
	{
		return MainCmd::Invalid;
	}

	std::string_view PackDealerNoHead::getMsg()
	{
		return string_view{ _msg};
	}

	
}
