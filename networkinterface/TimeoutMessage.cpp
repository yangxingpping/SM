
#include "TimeoutMessage.h"
#include "asio/deferred.hpp"

using std::move;

namespace SMNetwork
{
	TimeoutMessage::TimeoutMessage(shared_ptr<NMessage> msg)
		:_msg(msg)
	{
		if (!_msg)
		{
			SPDLOG_ERROR("msg pointer {}", fmt::ptr(_msg));
		}
		assert(_msg);
		auto [s, r] = oneshot::create<bool>();
		_sender = move(s);
		_recv = move(r);
	}
	asio::awaitable<bool> TimeoutMessage::waitOpFinish()
	{
		co_await _recv.async_wait(asio::deferred);
		co_return _recv.get();
	}
	asio::awaitable<void> TimeoutMessage::notifyOpFinish()
	{
		_sender.send(true);
		co_return;
	}
	uint32_t TimeoutMessage::No()
	{
		assert(_msg);
		return _msg->No();
	}
	shared_ptr<string> TimeoutMessage::body()
	{
		return _msg->body();
	}
	void TimeoutMessage::body(shared_ptr<string> body)
	{
		_msg->body(body);
	}
}
