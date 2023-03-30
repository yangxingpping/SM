
#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "StreamNetDealer.h"
#include "asio/experimental/concurrent_channel.hpp"
#include <map>
#include <array>
#include <list>
#include <memory>
#include <string>
#include "PackDealerMainSub.h"
#include "CoEvent.h"
using std::string;
using std::map;
using std::array;
using std::list;
using std::array;
using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using asio::experimental::concurrent_channel;

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT FullDuplexChannel
	{
	public:
		explicit FullDuplexChannel(int sock, shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>> sender, shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>> recv);
		FullDuplexChannel(FullDuplexChannel& copy);
		FullDuplexChannel(FullDuplexChannel&& right);
		~FullDuplexChannel();

		asio::awaitable<bool> sendPack(string_view src, uint32_t no);
		asio::awaitable<tuple<uint32_t, shared_ptr<string>>> recvPack();

		uint32_t sockNo();
		void setSockNo(uint32_t sockno);
	private:
		int _socket;
		SMNetwork::StreamNetDealer _netpack;

		shared_ptr<concurrent_channel<void(asio::error_code,shared_ptr<string>)>> _sender;
		shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>> _recv;
	};
} // namespace SMNetwork

