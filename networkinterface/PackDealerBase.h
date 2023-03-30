
#pragma once

#include "networkinterfaceExport.h"
#include "networkinterface.h"
#include "enums.h"
#include "asio/awaitable.hpp"
#include "asio/detached.hpp"

#include "Utils.h"

#include <stdint.h>
#include <deque>
#include <string_view>
#include <cassert>

namespace SMNetwork
{
    /**
     * @brief base class(interface) for all packdealer, packdealer is used to pack logic classify headers
     * and transport headers to message send from app and unpack transport and logic headers 
     * message recv from transport and send to app.
     * 
     */
	class PackDealerBase
	{
	public:
		virtual ~PackDealerBase() {}
		virtual MainCmd getMainc() { assert(0);  return MainCmd::MainCmdBegin; }
		virtual int getAssc() { assert(0); return 0xffff; }
		virtual ChannelType getChannelType() { assert(0); return ChannelType::EchoServer; }
		virtual void setAssc(int ) { assert(0); }
		virtual std::string_view getMsg() { assert(0); return string_view(); };
		virtual std::string* unpack(seqNumType* seqnum, string& pack) {assert(0);  return nullptr;};
		virtual std::string* unpack(seqNumType* seqnum, std::string_view pack) { assert(0);  return nullptr; };
		virtual std::string* unpack(std::string_view pack) { assert(0); return nullptr; };
		virtual std::string* pack(seqNumType* seqnum, std::string_view msg, uint16_t assc) {assert(0); return nullptr;};
		virtual std::string* pack(seqNumType* seqnum, std::string_view msg) { assert(0); return nullptr; };
		virtual std::string* pack(std::string_view msg) { assert(0); return nullptr; };
		virtual asio::awaitable<std::string*> dealmsg(string& msg) { assert(0); co_return nullptr; };
		virtual asio::awaitable<std::string*> dealmsgc(std::string& msg)
		{
			auto pstrreq = unpack(reqSeq(), msg);
			co_return pstrreq;
		};
		virtual seqNumType* reqSeq() { assert(0); return nullptr; }
		virtual seqNumType* repSeq() { assert(0); return nullptr; }
		virtual PackDealerBase* clone() { assert(0); return nullptr; };

		
	};

}
