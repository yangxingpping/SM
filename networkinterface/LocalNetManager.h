#pragma once

#include "networkinterfaceExport.h"
#include "CoEvent.h"
#include "asio/detached.hpp"
#include "asio/co_spawn.hpp"
#include "asio/experimental/concurrent_channel.hpp"
#include "FullDuplexChannel.h"
#include "ChannelCombine.h"
#include "NMessage.h"
#include "oneshot.hpp"
#include "TcpChannelImp.h"
#include <map>
#include <string>
#include <list>
#include <memory>
#include <tuple>
using std::map;
using std::shared_ptr;
using std::string;
using std::list;
using std::tuple;
using std::make_tuple;
using std::pair;
using asio::experimental::concurrent_channel;

namespace SMNetwork
{
	/**
	 * @brief class for manage full duplex channel
	 * 
	 */
	class NETWORKINTERFACE_EXPORT LocalNetManager
	{
	public:
		LocalNetManager();
		static LocalNetManager* GetInst2();
		asio::awaitable<bool> sendPack(int sock, shared_ptr<string> msg);
		asio::awaitable<bool> sendPack(int sock, shared_ptr<NMessage> msg);
		asio::awaitable<shared_ptr<string>> recvPack(int sock);
		tuple<shared_ptr<ChannelCombine<FullDuplexChannel, MainCmd>>, shared_ptr<ChannelCombine<FullDuplexChannel, MainCmd>>> createPair(MainCmd mainc);

		ChannelCombine<TcpChannelImpl, MainCmd>* createTcpChannel(unique_ptr<asio::ip::tcp::socket> sock, ChannelModeC channal, MainCmd mainc);

	private:
		map<int, int> _channelPairs;
		map<int, shared_ptr<ChannelCombine<FullDuplexChannel, MainCmd>>> _channels;
		map<int, shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>> _senddata;

		map<int, shared_ptr<ChannelCombine<TcpChannelImpl, MainCmd>>> _channelsTcp;
	};
}

#define LNM (SMNetwork::LocalNetManager::GetInst2())
