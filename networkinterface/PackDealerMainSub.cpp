
#include "PackDealerMainSub.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "Utils.h"
#include "fmt/format.h"
#include "Routers.h"
#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	std::string* PackDealerMainSub::unpack(seqNumType* seqnum, string& pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		MainCmd mainc{MainCmd::MainCmdBegin};
		uint16_t assc{ 0xffff };
		bool bsucc = SMUtils::uppackmaincmd3(seqnum, mainc, assc, len, pack, ret);
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

	std::string* PackDealerMainSub::unpack(seqNumType* seqnum, std::string_view pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		MainCmd mainc{ MainCmd::MainCmdBegin };
		uint16_t assc{ 0xffff };
		auto bsucc = SMUtils::uppackmaincmd3(seqnum, mainc, assc, len, pack, ret);
		assert(bsucc);
		assert(mainc == _mainc);
		if (!bsucc || mainc != _mainc)
		{
			return nullptr;
		}
		_assc = assc;
		return &ret;
	}

	std::string* PackDealerMainSub::unpack(std::string_view pack)
	{
		thread_local string ret;
		ret.clear();
		size_t len;
		MainCmd mainc{ MainCmd::MainCmdBegin };
		uint16_t assc{ 0xffff };
		auto bsucc = SMUtils::uppackmaincmd3(&_seqnumreq, mainc, assc, len, pack, ret);
		assert(bsucc);
		assert(mainc == _mainc);
		if (!bsucc || mainc != _mainc)
		{
			return nullptr;
		}
		_assc = assc;
		return &ret;
	}

	std::string* PackDealerMainSub::pack(seqNumType* seqnum, std::string_view msg)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(*seqnum, _mainc, _assc, ret, msg);
		return &ret;
	}

	std::string* PackDealerMainSub::pack(seqNumType* seqnum, std::string_view msg, uint16_t assc)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(*seqnum, _mainc, assc, ret, msg);
		return &ret;
	}

	std::string* PackDealerMainSub::pack(std::string_view msg)
	{
		thread_local std::string ret;
		ret.clear();
		SMUtils::packmaincmd3(++_seqnumreq, _mainc, _assc, ret, msg);
		return &ret;
	}

	asio::awaitable<std::string*> PackDealerMainSub::dealmsg(std::string& msg)
	{
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(_mainc);
		string strreq;
		auto pstrreq = unpack(reqSeq(), msg);
		SPDLOG_INFO("channel type {}, ass {}, dealer pt {}", magic_enum::enum_name(_channel), _assc, fmt::ptr(this));
		auto func2 = funcs->find(_assc);
		assert(func2 != funcs->end());
		if (func2 == funcs->end())
		{
			SPDLOG_ERROR("  templ mainc = {}, assc = {}, deal func = {}", magic_enum::enum_name(_mainc), _assc, fmt::ptr(funcs));
			co_return nullptr;
		}
		RouterFuncReturnType ret;
		string token;
		BEGIN_ASIO;
		ret = co_await(*(func2->second))(*pstrreq, token);
		END_ASIO;
		SPDLOG_INFO("send to client data: [{}], size:[{}]", *ret, ret->length());
		if (ret->empty())
		{
			SPDLOG_ERROR("  mainc = {}, assc = {} response 0", magic_enum::enum_name(_mainc), _assc);
			co_return nullptr;
		}
		auto drep2 = pack(reqSeq(), string_view(*ret), _assc);

		co_return drep2;

	}

	SMNetwork::PackDealerBase* PackDealerMainSub::clone()
	{
		PackDealerMainSub* ret = new PackDealerMainSub(*this);
		return ret;
	}

	PackDealerMainSub::PackDealerMainSub(MainCmd mainc, ChannelType channel) :  _mainc(mainc), PackDealerCommon(channel)
	{
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(_mainc);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(_mainc));
		}
		assert(funcs != nullptr);
	}


	PackDealerMainSub::PackDealerMainSub(const PackDealerMainSub& c): PackDealerCommon(c)
	{
		this->_mainc = c._mainc;
		_channel = c._channel;
		PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(this->_mainc);
		assert(funcs != nullptr);
		if (funcs == nullptr)
		{
			SPDLOG_ERROR("mainc cmd {} have no routers ", magic_enum::enum_name(this->_mainc));
		}
	}

	void PackDealerMainSub::setAssc(int assc)
	{
		_assc = assc;
	}

	int PackDealerMainSub::getAssc()
	{
		return _assc;
	}

	MainCmd PackDealerMainSub::getMainc()
	{
		return _mainc;
	}

	

}
