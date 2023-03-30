
#include "SendRecvPacks.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/deferred.hpp"

using asio::co_spawn;
using asio::detached;
using std::move;
using std::make_pair;

namespace SMNetwork
{

	SendRecvPacks::SendRecvPacks() :_needSendNotify(std::make_shared<SMHotupdate::CoEvent>(*IOCTX))
		, _pendingNotify(make_shared<SMHotupdate::CoEvent>(*IOCTX))
		, _needRecvNotify(std::make_shared<SMHotupdate::CoEvent>(*IOCTX)) {
		assert(_needSendNotify);
		auto [s1, r1] = oneshot::create<void>();
		_notifyNeedSend = move(s1);
		_waitNeedSend = move(r1);
		auto [s2, r2] = oneshot::create<void>();
		_notifyRecv = move(s2);
		_waitRecv = move(r2);
	};

	asio::awaitable<shared_ptr<TimeoutMessage>> SendRecvPacks::getFirstSend()
	{
		shared_ptr<TimeoutMessage> ret{nullptr};
		bool needWait = _needSendBufs.empty();
		if (_needSendBufs.empty())
		{
			co_await _waitNeedSend.async_wait(asio::deferred);
		}
		if (!_needSendBufs.empty())
		{
			ret = _needSendBufs.begin()->second;
		}
		co_return ret;
	}

	asio::awaitable<shared_ptr<SMNetwork::TimeoutMessage>> SendRecvPacks::popNeeSend()
	{
		shared_ptr<TimeoutMessage> ret{nullptr};
		if (_needSendBufs.empty())
		{
			co_await _waitNeedSend.async_wait(asio::deferred);
		}
		assert(!_needSendBufs.empty());
		if (!_needSendBufs.empty())
		{
			ret = _needSendBufs.begin()->second;
			_needSendBufs.erase(_needSendBufs.begin());
		}
		co_return ret;
	}

	
	void SendRecvPacks::pushRecv(uint32_t no, shared_ptr<string> msg)
	{
		auto it = _waittingRepBufs.find(no);
		if (it != _waittingRepBufs.end())
		{
			it->second->body(msg);
			asio::co_spawn(*IOCTX, [=]()->asio::awaitable<void> {
				co_await it->second->notifyOpFinish();
				co_return;
				}, asio::detached);
		}
		else //this is a direct notify, direct co_spawn coro thread to deal this
		{

		}
	}

	void SendRecvPacks::addPending(shared_ptr<TimeoutMessage> msg)
	{
		_pendingbufs.insert({ msg->No(), msg });
	}

	void SendRecvPacks::addSendingMessage(shared_ptr<TimeoutMessage> msg)
	{
		_sendingBufs.insert({ msg->No(), msg });
	}

	asio::awaitable<shared_ptr<TimeoutMessage>> SendRecvPacks::popPending(uint32_t no/* = 0*/)
	{
		shared_ptr<TimeoutMessage> ret{nullptr};
		auto it = _pendingbufs.find(no);
		if (it == _pendingbufs.end())
		{
			auto vv = co_await _pendingNotify->async_time_wait();
		}
		it = _pendingbufs.find(no);
		if (it != _pendingbufs.end())
		{
			ret = it->second;
			_pendingbufs.erase(it);
		}
		co_return ret;
	}

	//called by coro from app
	void SendRecvPacks::pushSend(shared_ptr<TimeoutMessage> msg)
	{
		auto needNotify = _needSendBufs.empty();
		_needSendBufs.insert({ msg->No(), msg });
		if (needNotify)
		{
			_notifyNeedSend.send();
		}
	}
	asio::awaitable <shared_ptr<TimeoutMessage>> SendRecvPacks::popRecv()
	{
		shared_ptr<TimeoutMessage> ret{nullptr};
		bool needWait = _recvbufs.empty();
		if (_recvbufs.empty())
		{
			co_await _needSendNotify->async_wait();
		}
		if (!_recvbufs.empty())
		{
			ret = _recvbufs.begin()->second;
			_recvbufs.erase(_recvbufs.begin());
		}
		co_return ret;
	}

	void SendRecvPacks::notifyMesssageSent(uint32_t no)
	{
		auto it = _pendingbufs.find(no);
		if (it != _pendingbufs.end())
		{
			asio::co_spawn(*IOCTX, [=, this]()->asio::awaitable<void> {
				_pendingNotify->trigger();
				co_await it->second->notifyOpFinish();
				co_return;
				}, asio::detached);
		}
		else
		{
			SPDLOG_WARN("message No.{} not in send buffer", no);
		}
	}

	asio::awaitable<bool> SendRecvPacks::waitPendingFinish(uint32_t no)
	{
		bool bret{ true };
		auto it = _pendingbufs.find(no);
		if (it == _pendingbufs.end())
		{
			SPDLOG_WARN("can not find req no. {} message, start wait", no);
			auto vv = co_await _pendingNotify->async_time_wait();
		}
		it = _pendingbufs.find(no);
		assert(it != _pendingbufs.end());
		if (it == _pendingbufs.end())
		{
			bret = false;
		}
		else
		{
			_pendingbufs.erase(it);
		}
		co_return bret;
	}

	asio::awaitable<shared_ptr<string>> SendRecvPacks::waitRep(uint32_t no)
	{
		shared_ptr<string> ret{nullptr};
		auto it = _waittingRepBufs.find(no);
		if (it == _waittingRepBufs.end())
		{
			co_return ret;
		}
		auto bflag = co_await it->second->waitOpFinish();
		if (bflag)
		{
			ret = it->second->body();
		}
		co_return ret;
	}

	shared_ptr<string> SendRecvPacks::getPendingMessage(uint32_t no)
	{
		shared_ptr<string> ret{nullptr};
		auto it = _pendingbufs.find(no);
		if (it != _pendingbufs.end())
		{
			ret = it->second->body();
			_pendingbufs.erase(it);
		}
		return ret;
	}

	bool SendRecvPacks::isWait(uint32_t no)
	{
		return _waittingRepBufs.find(no) != _waittingRepBufs.end() ? true : false;
	}

	bool SendRecvPacks::initForReqRep(uint32_t no)
	{
		bool bret = initWaitSent(no);
		if (!bret)
		{
			return bret;
		}
		bret = initWaitRecv(no);
		if (!bret)
		{
			_swaitSent.erase(no);
			_rwaitSent.erase(no);
		}
		return bret;
	}

	bool SendRecvPacks::initWaitSent(uint32_t no)
	{
		bool bret{ false };
		if (_swaitSent.find(no) == _swaitSent.end())
		{
			auto [s, r] = oneshot::create<bool>();
			_swaitSent.insert({ no, move(s) });
			_rwaitSent.insert({ no, move(r) });
			bret = true;
		}
		return bret;
	}

	asio::awaitable<bool> SendRecvPacks::waitMessageSent(uint32_t no)
	{
		bool bret{ false };
		auto it = _rwaitSent.find(no);
		if (it != _rwaitSent.end())
		{
			co_await it->second.async_wait(asio::deferred);
			bret = it->second.get();
			_swaitSent.erase(no);
			_rwaitSent.erase(no);
		}
		co_return bret;
	}

	bool SendRecvPacks::initWaitRecv(uint32_t no)
	{
		bool bret{ false };
		if (_swaitRecv.find(no) == _swaitRecv.end())
		{
			auto [s, r] = oneshot::create<shared_ptr<NMessage>>();
			_swaitRecv.insert({ no, move(s) });
			_rwaitRecv.insert({ no, move(r) });
			bret = true;
		}
		return bret;
	}

	asio::awaitable<shared_ptr<NMessage>> SendRecvPacks::waitMessageRecv(uint32_t no)
	{
		shared_ptr<NMessage> ret{nullptr};
		auto it = _rwaitRecv.find(no);
		if (it != _rwaitRecv.end())
		{
			co_await it->second.async_wait(asio::deferred);
			ret = it->second.get();
			_swaitRecv.erase(no);
			_rwaitRecv.erase(no);
		}
		co_return ret;
	}

}
