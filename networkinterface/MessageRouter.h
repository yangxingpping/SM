
#pragma once
#include "networkinterfaceExport.h"
#include "networkinterface.h"
#include "NMessage.h"
#include "asio/awaitable.hpp"
#include "asio/experimental/concurrent_channel.hpp"
#include "oneshot.hpp"

#include <memory>
#include <map>
#include <stdbool.h>
#include <optional>
#include <stdint.h>
#include <tuple>

using std::shared_ptr;
using asio::experimental::concurrent_channel;
using std::map;
using std::tuple;
using std::optional;
using std::move;

namespace SMNetwork
{
	

	class MessageRouter
	{
	public:
		static bool sInit();
		static void sUninit();
		static MessageRouter* sInst();
	public:
		MessageRouter();

		PrepareReqRepRecver prepareReqRep(uint32_t sock, uint32_t msgno);
		bool finishReqRep(uint32_t sock, uint32_t msgno);

		asio::awaitable<bool> addPackFromIo(uint32_t sock, int mainc, shared_ptr<NMessage> msg);
		asio::awaitable<bool> addPackFromApp(uint32_t sock, shared_ptr<NMessage> msg);

		oneshot::receiver<bool> prepareSignalSendComplete(uint32_t sock, uint32_t msgno);
		oneshot::receiver<shared_ptr<NMessage>> prepareSignalRecvNetRep(uint32_t sock, uint32_t msgno);
		bool notifyRecvNetRep(uint32_t sock, shared_ptr<NMessage> msg);
		bool notifyRecvAppRep(uint32_t sock, shared_ptr<NMessage> msg);
		bool notifySendNetComplete(uint32_t sock, uint32_t msgno, bool flag);
		oneshot::receiver<shared_ptr<NMessage>> prepareSignalRecvAppRep(uint32_t sock, uint32_t msgno);

		bool canReqRep(uint32_t sockno, uint32_t msgno);
		bool canWaitAppRep(uint32_t sockno, uint32_t msgno);
		bool start();   
	private:
		map<uint32_t, map<uint32_t, oneshot::sender<bool>>> _sendCompleteSignals;
		map<uint32_t, map<uint32_t, oneshot::sender<shared_ptr<NMessage>>>> _netRepSignals;
		map<uint32_t, map<uint32_t, oneshot::sender<shared_ptr<NMessage>>>> _appRepSignals;

		//notify channel for recv data from network io
		shared_ptr<concurrent_channel<void(asio::error_code, uint32_t, uint32_t, shared_ptr<NMessage>)>> _dataio;

		//notify channel for send data to network io success
		shared_ptr<concurrent_channel<void(asio::error_code, uint32_t, uint32_t)>> _notifyio;

		//notify channel for recv data from app
		shared_ptr<concurrent_channel<void(asio::error_code, uint32_t, shared_ptr<NMessage>)>> _dataapp;
	};
}

#define MR (MessageRouter::sInst())
