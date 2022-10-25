#pragma once
#include "../configs/Configs.h"
#include "Utils.h"
#include "IOContextManager.h"
#include "MesageHandlerBase.h"
#include "StreamHandler.h"
#include "coros.h"
#include "asio/local/stream_protocol.hpp"


#include "MessageSplitFuncs.h"
#include "fmt/core.h"

#include <memory>
#include <vector>
#include <tuple>
#include <stdbool.h>

using asio::ip::tcp;
using asio::steady_timer;
using std::shared_ptr;
using std::function;
using std::string;
using std::string_view;
using std::vector;
using std::tuple;
using std::make_tuple;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT TcpStreamHandler : public StreamHandler<asio::ip::tcp::socket>
	{
	public:
		TcpStreamHandler()=delete;
		TcpStreamHandler(shared_ptr<asio::ip::tcp::socket> sock, NetHeadType nettype);
		TcpStreamHandler(const TcpStreamHandler&) = delete;
		TcpStreamHandler(TcpStreamHandler&& c);
		TcpStreamHandler& operator = (TcpStreamHandler&& c);
		virtual ~TcpStreamHandler();

		virtual shared_ptr<asio::ip::tcp::socket> getHandler() override;

		virtual string getFullAddr() override;

		virtual asio::awaitable<tuple<bool, std::string*>> recvPack(uint16_t retrycount = 60) override;
		virtual asio::awaitable<bool> sendPack(string_view msg) override;

		virtual NetHeadType getNetHeadType() override;

	protected:

		void _packnethead(uint32_t packlen);

		shared_ptr<asio::ip::tcp::socket> _socket;
		NetHeadType _nethead;
		string _fullAddr;
		uint32_t _curpacklen = 0;
		std::string _recvbuf;
		std::array<uint8_t, 1024> _recvasio;
		std::string _recvmsg;
		std::string _recvmsgbin; //for binary protocol
		std::string _sendbuf;
	};
}
