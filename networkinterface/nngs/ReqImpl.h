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
	class NETWORKINTERFACE_EXPORT ReqImpl
	{
	public:
		ReqImpl(nng_socket sock, uint32_t sockNo);
		ReqImpl(const ReqImpl& ref) = delete;
		ReqImpl(ReqImpl&& right);
		/**
		 * send a pack.
		 *
		 * \param src message body
		 * \param no message No.
		 * \return if send success, return true; else return false
		 */
		asio::awaitable<bool> sendPack(string_view src, uint32_t no);
		/**
		 * recv a pack.
		 *
		 * \return message No. and message body tuple
		 */
		asio::awaitable<tuple<uint32_t, shared_ptr<string>>> recvPack();

		uint32_t sockNo();

		void setSockNo(uint32_t sockno);

	public:
		nng_msg* _req{ nullptr };
		nng_msg* _rep{ nullptr };
		nng_socket _sock;
		uint32_t _sockno;
		uint32_t _msgno;
	};
}
