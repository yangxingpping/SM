
#include "PackDealerNoHead.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "Utils.h"
#include "hotupdate.h"
#include "Routers.h"
#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	std::string* PackDealerNoHead::unpack(seqNumType* seqnum, string& pack)
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

	std::string* PackDealerNoHead::unpack(seqNumType* seqnum, std::string_view pack)
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

	std::string* PackDealerNoHead::pack(seqNumType* seqnum, std::string_view msg)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(ret, msg);
		ret.insert(ret.end(), msg.begin(), msg.end());
		return &ret;
	}

	asio::awaitable<std::string*> PackDealerNoHead::dealmsg(string& msg)
	{
		auto mainc = MainCmd::DefaultMain;
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(mainc);
		SPDLOG_INFO("channel type {}", magic_enum::enum_name(_channel));

		string strreq;
		seqNumType seqnum;
		auto pstrreq = unpack(reqSeq(), msg);
		auto func2 = funcs->begin();
		assert(func2 != funcs->end());
		if (func2 == funcs->end())
		{
			SPDLOG_ERROR("  templ mainc = {},  deal func = {}", magic_enum::enum_name(mainc), fmt::ptr(funcs));
			co_return nullptr;
		}
		RouterFuncReturnType ret;
		string token;
		BEGIN_ASIO;
		ret = co_await(*(func2->second))(*pstrreq, token);
		END_ASIO;
		if (!ret || ret->empty())
		{
			SPDLOG_ERROR("  mainc = {} response 0", magic_enum::enum_name(mainc));
			co_return nullptr;
		}
		SPDLOG_INFO("send to client data: [{}], size:[{}]", *ret, ret->length());
		auto drep2 = pack(reqSeq(), string_view(*ret));

		co_return drep2;

	}

	SMNetwork::PackDealerBase* PackDealerNoHead::clone()
	{
		PackDealerNoHead* ret = new PackDealerNoHead(*this);
		return ret;
	}

	PackDealerNoHead::PackDealerNoHead( ChannelType channel) : PackDealerCommon(channel)
	{
		auto mainc = MainCmd::MainCmdBegin;
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(mainc));
		}
	}


	PackDealerNoHead::PackDealerNoHead(const PackDealerNoHead& c):PackDealerCommon(c)
	{
		auto mainc = MainCmd::MainCmdBegin;
		this->_channel = c._channel;
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(mainc));
		}
	}

	
}
