
#include "LocalNetManager.h"
#include <memory>
#include <atomic>
using std::unique_ptr;
using std::make_unique;
using std::atomic_int;
using std::make_shared;
namespace SMNetwork
{
	static unique_ptr<LocalNetManager> _sLocalNetManager{nullptr};
	static atomic_int _Index;
	LocalNetManager::LocalNetManager()
	{

	}

	LocalNetManager* LocalNetManager::GetInst2()
	{
		if (!_sLocalNetManager)
		{
			_sLocalNetManager = make_unique<LocalNetManager>();
		}
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
		auto itpair = _channels.find(sock);
		auto ittcp = _channelsTcp.find(sock);
		if (ittcp != _channelsTcp.end())
		{
			bret = co_await ittcp->second->sendMessage(msg);
		}
		else if (itpair != _channels.end())
		{
			bret = co_await itpair->second->sendMessage(msg);
		}
		else
		{
			SPDLOG_WARN("can not find sock {}", sock);
		}
		MR->notifySendNetComplete(sock, msg->No(), bret);
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

	tuple<shared_ptr<ChannelCombine<FullDuplexChannel, MainCmd>>, shared_ptr<ChannelCombine<FullDuplexChannel, MainCmd>>> LocalNetManager::createPair(MainCmd mainc)
	{
		int f = ++_Index;
		int s = ++_Index;
		_senddata.insert({ f, make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX,10) });
		_senddata.insert({ s, make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX,10) });
		_channelPairs.insert({ f,s });
		_channelPairs.insert({ s,f });
		auto s1tos2 = make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX, 16);
		auto s2tos1 = make_shared<concurrent_channel<void(asio::error_code, shared_ptr<string>)>>(*IOCTX, 16);

		auto c1 = make_shared <ChannelCombine<FullDuplexChannel, MainCmd>>(make_shared<FullDuplexChannel>(f, s1tos2, s2tos1), ChannelModeC::Initiative, mainc);
		auto c2 = make_shared <ChannelCombine<FullDuplexChannel, MainCmd>>(make_shared<FullDuplexChannel>(s, s2tos1, s1tos2), ChannelModeC::Passive, mainc);
		_channels.insert({ f, c1 });
		_channels.insert({ s, c2 });
		c1->start();
		c2->start();
		return make_tuple(c1,c2);
	}

	ChannelCombine<TcpChannelImpl, MainCmd>* LocalNetManager::createTcpChannel(unique_ptr<asio::ip::tcp::socket> sock, ChannelModeC channal, MainCmd mainc)
	{		
		auto sockno = newSockNo();
		auto tcpiml = make_shared< TcpChannelImpl>(move(sock), sockno);
		shared_ptr<ChannelCombine<TcpChannelImpl, MainCmd>> ret = make_shared<ChannelCombine<TcpChannelImpl, MainCmd>>(tcpiml, channal, mainc);
		_channelsTcp.insert({ sockno, ret });
		return ret.get();
	}

}
