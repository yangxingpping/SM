
#include "MessageRouter.h"
#include "LocalNetManager.h"
#include "spdlog/spdlog.h"
#include "asio/co_spawn.hpp"
#include "asio/deferred.hpp"
#include "asio/detached.hpp"
#include "hotupdate.h"
#include "PackUnpackManager.h"
#include <assert.h>

using std::move;
using asio::deferred;
using asio::co_spawn;
using asio::detached;
using std::make_shared;
using std::make_tuple;

namespace SMNetwork
{
	static shared_ptr<MessageRouter> _sInst = nullptr;
	static shared_ptr<oneshot::sender<void>> _nofityFinish = nullptr;
	static shared_ptr<oneshot::receiver<void>> _waitFinish = nullptr;
	static bool _brun = false;

	bool MessageRouter::sInit()
	{
		bool bret{ false };
		if (_sInst != nullptr)
		{
			SPDLOG_WARN("message router init more than 1 time");
			return bret;
		}
		assert(_sInst == nullptr);
		auto [s, r] = oneshot::create<void>();
		_nofityFinish = make_shared<oneshot::sender<void>>(move(s));
		_waitFinish = make_shared<oneshot::receiver<void>>(move(r));
		_sInst = make_shared<MessageRouter>();
		bret = true;
		_brun = true;
		return bret;
	}

	void MessageRouter::sUninit()
	{
		asio::co_spawn(*IOCTX, []()->asio::awaitable<void> {
			_brun = false;
			co_await _waitFinish->async_wait(asio::deferred);
			_waitFinish = nullptr;
			_nofityFinish = nullptr;
			_sInst = nullptr;
			co_return;
			}, asio::detached);
	}

	MessageRouter* MessageRouter::sInst()
	{
		assert(_sInst);
		return _sInst.get();
	}

	MessageRouter::MessageRouter()
	{
		SPDLOG_INFO("create MessageRouter instance");
		_dataio = make_shared<concurrent_channel<void(asio::error_code, uint32_t, uint32_t, shared_ptr<NMessage>)>>(*IOCTX, 10);
		_dataapp = make_shared<concurrent_channel<void(asio::error_code, uint32_t, shared_ptr<NMessage>)>>(*IOCTX, 10);
		_notifyio = make_shared<concurrent_channel<void(asio::error_code, uint32_t, uint32_t)>>(*IOCTX, 10);
	}

	PrepareReqRepRecver MessageRouter::prepareReqRep(uint32_t sock, uint32_t msgno)
	{
		auto [s1, r1] = oneshot::create<bool>();
		auto [s2, r2] = oneshot::create<shared_ptr<NMessage>>();
		auto& itSendCompleteSignals = _sendCompleteSignals[sock];
		if (itSendCompleteSignals.find(msgno) != itSendCompleteSignals.end())
		{

		}
		auto& itNetRepSignals = _netRepSignals[sock];
		return PrepareReqRepRecver(move(r1), move(r2));
	}

	bool MessageRouter::finishReqRep(uint32_t sock, uint32_t msgno)
	{
		bool bret{ false };

		return bret;
	}

	asio::awaitable<bool> MessageRouter::addPackFromIo(uint32_t sock, int mainc, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		BEGIN_ASIO;
		co_await _dataio->async_send(asio::error_code(), sock, mainc, msg, asio::use_awaitable);
		bret = true;
		END_ASIO;
		co_return bret;
	}

	asio::awaitable<bool> MessageRouter::addPackFromApp(uint32_t sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		BEGIN_ASIO;
		co_await _dataapp->async_send(asio::error_code(), sock, msg, asio::use_awaitable);
		bret = true;
		END_ASIO;
		co_return bret;
	}

	oneshot::receiver<bool> MessageRouter::prepareSignalSendComplete(uint32_t sock, uint32_t msgno)
	{
		auto [s, r] = oneshot::create<bool>();
		_sendCompleteSignals[sock][msgno] = move(s);
		return move(r);
	}

	oneshot::receiver<shared_ptr<SMNetwork::NMessage>> MessageRouter::prepareSignalRecvNetRep(uint32_t sock, uint32_t msgno)
	{
		auto [s, r] = oneshot::create<shared_ptr<NMessage>>();
		_netRepSignals[sock][msgno] = move(s);
		return move(r);
	}

	bool MessageRouter::notifyRecvNetRep(uint32_t sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		auto it = _netRepSignals[sock].find(msg->No());
		if (it != _netRepSignals[sock].end())
		{
			it->second.send(msg);
			bret = true;
		}
		return bret;
	}

	bool MessageRouter::notifyRecvAppRep(uint32_t sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		auto it = _appRepSignals[sock].find(msg->No());
		if (it != _appRepSignals[sock].end())
		{
			it->second.send(msg);
			bret = true;
		}
		return bret;
	}

	bool MessageRouter::notifySendNetComplete(uint32_t sock, uint32_t msgno, bool flag)
	{
		bool bret{ false };
		auto it = _sendCompleteSignals[sock].find(msgno);
		if (it != _sendCompleteSignals[sock].end())
		{
			it->second.send(flag);
			bret = true;
		}
		return bret;
	}

	oneshot::receiver<shared_ptr<SMNetwork::NMessage>> MessageRouter::prepareSignalRecvAppRep(uint32_t sock, uint32_t msgno)
	{
		assert(_appRepSignals[sock].find(msgno) == _appRepSignals[sock].end());
		auto [s, r] = oneshot::create<shared_ptr<NMessage>>();
		_appRepSignals[sock][msgno] = move(s);
		return move(r);
	}

	bool MessageRouter::canReqRep(uint32_t sockno, uint32_t msgno)
	{
		if (_sendCompleteSignals[sockno].find(msgno) != _sendCompleteSignals[sockno].end())
		{
			return false;
		}
		if (_netRepSignals[sockno].find(msgno) != _netRepSignals[sockno].end())
		{
			return false;
		}
		return true;
	}

	bool MessageRouter::canWaitAppRep(uint32_t sockno, uint32_t msgno)
	{
		if (_appRepSignals[sockno].find(msgno) != _appRepSignals[sockno].end())
		{
			return false;
		}
		return true;
	}

	bool MessageRouter::start()
	{
		bool bret{ false };
		//start coro to get pack need send to network
		asio::co_spawn(*IOCTX, [this]()->asio::awaitable<void> {
			while (_brun)
			{
				auto [sockno, msg] = co_await _dataapp->async_receive(asio::use_awaitable);
				//SPDLOG_INFO("recv data need send to network from app in sock {} message No.{}", sockno, msg->No());
				auto bsend = co_await LNM->sendPack(sockno, msg);
			}
			_nofityFinish->send();
			co_return;
			}, asio::detached);
		//start coro to deal pack send from network with no need wait rep message
		asio::co_spawn(*IOCTX, [this]()->asio::awaitable<void> {
			while (_brun)
			{
				BEGIN_ASIO;
				auto [sockno, mainc, msg] = co_await _dataio->async_receive(asio::use_awaitable);
				auto plat = clonePlatformPack(mainc);
				shared_ptr<string> strrep = make_shared<string>();
				if (plat->len() > 0)
				{
					assert(plat->unpack(string_view(msg->body()->begin(), msg->body()->begin() + plat->len())));
					string strreq = string(msg->body()->begin() + plat->len(), msg->body()->end());
					//SPDLOG_INFO("request No. {} recv request json {} ", msg->No(), strreq);
					PtTransRouterElement funcs = SMCONF::getRouterTransByMainC(plat->getMain());
					auto func2 = funcs->find(plat->getAss());
					assert(func2 != funcs->end());
					if (func2 == funcs->end())
					{
						SPDLOG_ERROR("  templ mainc = {}, assc = {}, deal func = {}", plat->getMain(), plat->getAss(), fmt::ptr(funcs));
						co_return;
					}
					RouterFuncReturnType ret;
					string token;
					
					ret = co_await(*(func2->second))(strreq, token);
					strrep->resize(plat->len());
					plat->pack(span<char>(strrep->begin(), strrep->begin() + plat->len()));
					strrep->append(*ret);
				}
				else
				{
					strrep = msg->body();
				}
				assert(notifyRecvAppRep(sockno, make_shared<NMessage>(msg->No(), strrep)));
				END_ASIO;
			}
			co_return;
			}, asio::detached);
		bret = true;  
		return bret;
	}

}
