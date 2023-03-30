
#include "PackDealerNoMain.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "hotupdate.h"
#include "Routers.h"
#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	std::string* PackDealerNoMain::unpack(seqNumType* seqnum, std::string& pack)
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

	std::string* PackDealerNoMain::pack(seqNumType* seqnum, std::string_view msg, uint16_t assc)
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
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(MainCmd::DefaultMain);
		SPDLOG_INFO("channel type {}", magic_enum::enum_name(_channel));

		string strreq;
		auto pstrreq = unpack(reqSeq(), msg);
		auto func2 = funcs->find(_assc);
		assert(func2 != funcs->end());
		if (func2 == funcs->end())
		{
			SPDLOG_ERROR("  templ mainc = {}, assc = {}, deal func = {}", magic_enum::enum_name(MainCmd::DefaultMain), _assc, fmt::ptr(funcs));
			co_return nullptr;
		}
		RouterFuncReturnType ret;
		string token;
		BEGIN_ASIO;
		ret = co_await(*(func2->second))(*pstrreq, token);
		END_ASIO;
		if (!ret || ret->empty())
		{
			SPDLOG_ERROR("  mainc = {}, assc = {} response 0", magic_enum::enum_name(MainCmd::DefaultMain), _assc);
			co_return nullptr;
		}
		SPDLOG_INFO("send to client data: [{}], size:[{}]", *ret, ret->length());
		auto drep2 = pack(reqSeq(), string_view(*ret), _assc);

		co_return drep2;

	}


	SMNetwork::PackDealerBase* PackDealerNoMain::clone()
	{
		PackDealerNoMain* ret = new PackDealerNoMain(*this);
		return ret;
	}

	PackDealerNoMain::PackDealerNoMain(ChannelType channel) : PackDealerCommon(channel)
	{
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(MainCmd::DefaultMain);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(MainCmd::DefaultMain));
		}
	}


	PackDealerNoMain::PackDealerNoMain(const PackDealerNoMain& c):PackDealerCommon(c)
	{
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(MainCmd::DefaultMain);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(MainCmd::DefaultMain));
		}
	}



	int PackDealerNoMain::getAssc()
	{
		return _assc;
	}

	void PackDealerNoMain::setAssc(int assc)
	{
		_assc = assc;
	}



	
}
