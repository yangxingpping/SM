
#pragma once

#include "networkinterfaceExport.h"
#include "networkinterface.h"
#include "enums.h"
#include "asio/awaitable.hpp"
#include "asio/detached.hpp"

#include <stdint.h>
#include <deque>
#include <string_view>
#include <cassert>

namespace SMNetwork
{
    /**
     * @brief base class for all packdealer, packdealer is used to pack logic classify headers
     * and transport headers to message send from app and unpack transport and logic headers 
     * message recv from transport and send to app.
     * 
     */
	class PackDealerBase
	{
	public:
		virtual ~PackDealerBase() {}
		virtual MainCmd getMainc() { assert(0);  return MainCmd::Invalid; }
		virtual uint16_t getAssc() { assert(0); return 0xffff; }
		virtual void setAssc(uint16_t ) { assert(0); }
		virtual std::string_view getMsg() { assert(0); return string_view(); };
		virtual std::string* unpack(string& pack) {assert(0);  return nullptr;};
		virtual std::string* unpack(std::string_view pack) { assert(0);  return nullptr; };
		virtual std::string* pack(std::string_view msg, uint16_t assc) {assert(0); return nullptr;};
		virtual std::string* pack(std::string_view msg) { assert(0); return nullptr; };
		virtual asio::awaitable<std::string*> dealmsg(string& msg) { assert(0); co_return nullptr; };
		virtual asio::awaitable<std::string*> dealmsgc(std::string& msg)
		{
			auto pstrreq = unpack(msg);
			co_return pstrreq;
		};
		virtual PackDealerBase* clone() { assert(0); return nullptr; };
	};

}
