#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "asio/experimental/concurrent_channel.hpp"
#include "nng/nng.h"
#include <stdint.h>
#include <stdbool.h>
#include <memory>
#include <tuple>
#include <string>
#include <string_view>

using std::string_view;
using std::unique_ptr;
using std::tuple;
using std::shared_ptr;
using std::string;

namespace SMNetwork
{
	class AsynReqImpl
	{
	public:
		AsynReqImpl(nng_socket addr, uint32_t sockNo);
		AsynReqImpl(const AsynReqImpl& ref) = delete;
		AsynReqImpl(AsynReqImpl&& right);
		asio::awaitable<bool> sendPack(string_view src, uint32_t no);
		asio::awaitable<tuple<uint32_t, shared_ptr<string>>> recvPack();
		uint32_t sockNo();
		void setSockNo(uint32_t sockno);
	public:
		nng_socket _sock;
		uint32_t _sockno;
		nng_msg* _req{ nullptr };
		nng_msg* _rep{ nullptr };
		shared_ptr<asio::experimental::concurrent_channel<void(asio::error_code, string_view)>> _msgs;
	};
}
