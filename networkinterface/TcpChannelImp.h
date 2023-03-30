#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "asio/buffer.hpp"
#include "asio/ip/tcp.hpp"
#include <string>
#include <memory>
#include <array>
#include <string_view>
#include <tuple>
using std::string;
using std::unique_ptr;
using std::array;
using std::tuple;
using std::string_view;
using std::shared_ptr;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT TcpChannelImpl
	{
	public:
		TcpChannelImpl(unique_ptr<asio::ip::tcp::socket> sock, uint32_t sockNo);
		TcpChannelImpl(TcpChannelImpl&& right);
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
	private:
		unique_ptr<asio::ip::tcp::socket> _sock{ nullptr };
		unique_ptr<string> _bRecv;
		unique_ptr<string> _bRecvMsg;
		array<uint8_t, 512> _recvasio;
		int _retryCount{ 2 };
		int _sockNo{ -1 };
	};
}

