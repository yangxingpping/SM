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
#include "../networkinterface/socket/TcpChannelImp.h"
#include "../networkinterface/nngs/AsynRepImpl.h"
#include "../networkinterface/nngs/ReqImpl.h"

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
		static bool sInit();
		static void sUninit();
	public:
		LocalNetManager();
		static LocalNetManager* GetInst2();
		asio::awaitable<bool> sendPack(int sock, shared_ptr<string> msg);
		asio::awaitable<bool> sendPack(int sock, shared_ptr<NMessage> msg);
		asio::awaitable<shared_ptr<string>> recvPack(int sock);

	
		tuple<shared_ptr<ChannelCombine<FullDuplexChannel>>, shared_ptr<ChannelCombine<FullDuplexChannel>>> createPair(int mainc);
		ChannelCombine<TcpChannelImpl>* createTcpChannel(unique_ptr<asio::ip::tcp::socket> sock, ChannelModeC channal, int mainc);
		bool createRep(string_view addr, ChannelModeC channel, int mainc, int cur);
		ChannelCombine<ReqImpl>* createReq(string_view addr, ChannelModeC channel, int mainc);

	protected:
		asio::awaitable<bool> _sendTcp(int sock, shared_ptr<NMessage> msg);
		asio::awaitable<bool> _sendRep(int sock, shared_ptr<NMessage> msg);
		asio::awaitable<bool> _sendReq(int sock, shared_ptr<NMessage> msg);
	private:
		map<int, int> _channelPairs;
		map<int, shared_ptr<ChannelCombine<FullDuplexChannel>>> _channels;
		map<int, shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>> _senddata;

		map<int, shared_ptr<ChannelCombine<TcpChannelImpl>>> _channelsTcp;
		map<int, shared_ptr<ChannelCombine<AsynRepImpl>>> _channelsRep;
		map<int, shared_ptr<ChannelCombine<ReqImpl>>> _channelsReq;
	};
}

#define LNM (SMNetwork::LocalNetManager::GetInst2())
