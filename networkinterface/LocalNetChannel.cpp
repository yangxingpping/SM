
#include "LocalNetChannel.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"

namespace SMNetwork
{
	LocalNetChannel::LocalNetChannel(int index)
	{
		_notifyRecv = make_shared<SMHotupdate::CoEvent>(*IOCTX);
	}
	void LocalNetChannel::start()
	{
		asio::co_spawn(*IOCTX, [this]()->asio::awaitable<void> {
			while (_brun)
			{
				co_await _notifyRecv->async_time_wait();
			}
			}, asio::detached);
	}
	void LocalNetChannel::stop()
	{
		asio::co_spawn(*IOCTX, [this]()->asio::awaitable<void> {
			this->_brun = false;
			_notifyRecv->trigger();
			co_return;
			}, asio::detached);
	}
	void LocalNetChannel::sendMessage(shared_ptr<string> msg)
	{
		asio::co_spawn(*IOCTX, [this, msg]()->asio::awaitable<void> {
			auto neednotify = _recvFrames.empty();
			_recvFrames.push_back(msg);
			if (neednotify)
			{
				_notifyRecv->trigger();
			}
			co_return;
			}, asio::detached);
	}
	asio::awaitable<shared_ptr<string>> LocalNetChannel::recvPack()
	{
		shared_ptr<string> ret{nullptr};
		if (_recvFrames.empty())
		{
			co_await _notifyRecv->async_time_wait();
		}
		if (!_recvFrames.empty())
		{
			ret = *(_recvFrames.begin());
			_recvFrames.pop_front();
		}
		co_return ret;
	}
}

