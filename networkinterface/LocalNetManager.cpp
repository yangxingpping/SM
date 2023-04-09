
#include "LocalNetManager.h"
#include "nng/protocol/reqrep0/req.h"
#include "nng/protocol/reqrep0/rep.h"
#include <memory>
#include <atomic>
using std::unique_ptr;
using std::make_unique;
using std::atomic_int;
using std::make_shared;
namespace SMNetwork
{
	static unique_ptr<LocalNetManager> _sLocalNetManager{nullptr};
	static atomic_int _Index{0};

	bool LocalNetManager::sInit()
	{
		bool bret{ false };
		if (_sLocalNetManager != nullptr)
		{
			SPDLOG_WARN("module had initialized");
			return bret;
		}
		_sLocalNetManager = make_unique<LocalNetManager>();
		bret = true;
		return bret;
	}

	void LocalNetManager::sUninit()
	{
		_sLocalNetManager = nullptr;
	}

	LocalNetManager::LocalNetManager()
	{

	}

	LocalNetManager* LocalNetManager::GetInst2()
	{
		if (_sLocalNetManager == nullptr)
		{
			SPDLOG_WARN("net manager not initialize");
		}
		assert(_sLocalNetManager != nullptr);
		return _sLocalNetManager.get();
	}

	asio::awaitable<bool> LocalNetManager::sendPack(int sock, shared_ptr<string> msg)
	{
		bool bret{ false };
		auto it = _senddata.find(sock);
		if (it == _senddata.end())
		{
			SPDLOG_ERROR("find sock {} failed", sock);
			co_return bret;
		}
		BEGIN_ASIO;
		co_await it->second->async_send(asio::error_code(), msg, asio::use_awaitable);
		bret = true;
		END_ASIO;
		co_return bret;
	}

	

	asio::awaitable<bool> LocalNetManager::sendPack(int sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		bret = co_await _sendTcp(sock, msg);
		if (!bret)
		{
			bret = co_await _sendRep(sock, msg);
			if (!bret)
			{
				bret = co_await _sendReq(sock, msg);
			}
		}
		if (!bret)
		{
			SPDLOG_WARN("send datat to sock {} with msg no.{} failed", sock, msg->No());
		}
		co_return bret;
	}

	asio::awaitable <shared_ptr<string>> LocalNetManager::recvPack(int sock)
	{
		shared_ptr<string> ret{nullptr};
		auto it = _channelPairs.find(sock);
		if (it == _channelPairs.end())
		{
			SPDLOG_INFO("sock {} has no pair", sock);
			co_return ret;
		}
		auto itsend = _senddata.find(it->second);
		if (itsend == _senddata.end())
		{
			SPDLOG_ERROR("sock {} has no send channel ", it->second);
			co_return ret;
		}
		BEGIN_ASIO;
		ret = co_await itsend->second->async_receive(asio::use_awaitable);
		END_ASIO;
		co_return ret;
	}

	tuple<shared_ptr<ChannelCombine<FullDuplexChannel>>, shared_ptr<ChannelCombine<FullDuplexChannel>>> LocalNetManager::createPair(int mainc)
	{
		int f = ++_Index;
		int s = ++_Index;
		_senddata.insert({ f, make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX,10) });
		_senddata.insert({ s, make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX,10) });
		_channelPairs.insert({ f,s });
		_channelPairs.insert({ s,f });
		auto s1tos2 = make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX, 16);
		auto s2tos1 = make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX, 16);

		auto c1 = make_shared <ChannelCombine<FullDuplexChannel>>(make_shared<FullDuplexChannel>(f, s1tos2, s2tos1), ChannelModeC::Initiative, mainc);
		auto c2 = make_shared <ChannelCombine<FullDuplexChannel>>(make_shared<FullDuplexChannel>(s, s2tos1, s1tos2), ChannelModeC::Passive, mainc);
		_channels.insert({ f, c1 });
		_channels.insert({ s, c2 });
		c1->start();
		c2->start();
		return make_tuple(c1,c2);
	}

	ChannelCombine<TcpChannelImpl>* LocalNetManager::createTcpChannel(unique_ptr<asio::ip::tcp::socket> sock, ChannelModeC channal, int mainc)
	{		
		auto sockno = newSockNo();
		auto tcpiml = make_shared< TcpChannelImpl>(move(sock), sockno);
		shared_ptr<ChannelCombine<TcpChannelImpl>> ret = make_shared<ChannelCombine<TcpChannelImpl>>(tcpiml, channal, mainc);
		_channelsTcp.insert({ sockno, ret });
		return ret.get();
	}

	bool LocalNetManager::createRep(string_view addr, ChannelModeC channel, int mainc, int cur)
	{
		bool bret{ false };
		nng_socket _nng;
		int nnop = nng_rep0_open(&_nng);
		if (nnop != 0)
		{
			SPDLOG_WARN("rep open failed on addr {}, errno {}, error desc {}", addr, nnop, nng_strerror(nnop));
			return bret;
		}
		set<ChannelCombine<AsynRepImpl>*> ccs;
		for (uint16_t i = 0; i < cur; ++i)
		{
			auto sockno = newSockNo();
			auto repimpl = make_shared<AsynRepImpl>(sockno);
			int nnop = nng_aio_alloc(&repimpl->_aio, &AsynRepImpl::rep_callback, repimpl.get());
			if (nnop != 0)
			{
				SPDLOG_WARN("nng_aio_alloc failed with errno {}, strerr {}", nnop, nng_strerror(nnop));
				return bret;
			}
			nnop = nng_ctx_open(&repimpl->_ctx, _nng);
			if (nnop != 0)
			{
				SPDLOG_WARN("nng_ctx_open failed with error {}, strerr {}", nnop, nng_strerror(nnop));
				return bret;
			}
			auto rep = make_shared<ChannelCombine<AsynRepImpl>>(repimpl, channel, mainc);
			_channelsRep.insert({ sockno, rep });
			ccs.insert(rep.get());
		}
		nnop = nng_listen(_nng, addr.data(), NULL, 0);
		if(nnop != 0)
		{
			SPDLOG_WARN("nng_listen on addr {} failed with errno {} error desc {}", addr, nnop, nng_strerror(nnop));
			return bret;
		}
		for (auto rep : ccs)
		{
			rep->start();
			AsynRepImpl::rep_callback(rep);
		}
		bret = true;
		return bret;
	}

	ChannelCombine<ReqImpl>* LocalNetManager::createReq(string_view addr, ChannelModeC channel, int mainc)
	{
		auto sockno = newSockNo();
		nng_socket sock;
		int nnop = nng_req0_open(&sock);
		auto repimpl = make_shared<ReqImpl>(sock, sockno);
		auto req = make_shared<ChannelCombine<ReqImpl>>(repimpl, channel, mainc);
		_channelsReq.insert({ sockno, req });
		return req.get();
	}

	asio::awaitable<bool> LocalNetManager::_sendTcp(int sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		auto ittcp = _channelsTcp.find(sock);
		if (ittcp != _channelsTcp.end())
		{
			bret = co_await ittcp->second->sendMessage(msg);
			MR->notifySendNetComplete(sock, msg->No(), bret);
			bret = true;
		}
		co_return bret;
	}

	asio::awaitable<bool> LocalNetManager::_sendRep(int sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		auto ittcp = _channelsRep.find(sock);
		if (ittcp != _channelsRep.end())
		{
			bret = co_await ittcp->second->sendMessage(msg);
			MR->notifySendNetComplete(sock, msg->No(), bret);
			bret = true;
		}
		co_return bret;
	}

	asio::awaitable<bool> LocalNetManager::_sendReq(int sock, shared_ptr<NMessage> msg)
	{
		bool bret{ false };
		auto ittcp = _channelsReq.find(sock);
		if (ittcp != _channelsReq.end())
		{
			bret = co_await ittcp->second->sendMessage(msg);
			MR->notifySendNetComplete(sock, msg->No(), bret);
			bret = true;
		}
		co_return bret;
	}

}
