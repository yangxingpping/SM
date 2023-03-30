#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "CoEvent.h"
#include <memory>
#include <string>
#include <string>
using std::string;
using std::shared_ptr;
using std::list;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT LocalNetChannel
	{
	public:
		LocalNetChannel(int index);
		void start();
		void stop();
		void sendMessage(shared_ptr<string> msg);
		asio::awaitable<shared_ptr<string>> recvPack();
	private:
		shared_ptr<SMHotupdate::CoEvent> _notifyRecv{ nullptr };
		list<shared_ptr<string>> _recvFrames;
		int _index{ -1 };
		bool _brun{ true };
	};
}
