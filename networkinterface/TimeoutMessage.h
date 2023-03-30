#pragma  once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "CoEvent.h"
#include "oneshot.hpp"
#include "NMessage.h"
#include <memory>
#include <string>
#include <stdbool.h>

using std::string;
using std::shared_ptr;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT TimeoutMessage
	{
	public:
		TimeoutMessage(shared_ptr<NMessage> msg);
		asio::awaitable<bool> waitOpFinish();
		asio::awaitable<void> notifyOpFinish();
		uint32_t No();
		shared_ptr<string> body();
		void body(shared_ptr<string> body);
	private:
		shared_ptr<NMessage> _msg;

		oneshot::sender<bool> _sender;
		oneshot::receiver<bool> _recv;
	};
}
