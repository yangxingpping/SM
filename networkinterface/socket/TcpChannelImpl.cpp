#include "TcpChannelImp.h"
#include "asio/use_awaitable.hpp"
#include "IOContextManager.h"
#include "PackDealerMainSub.h"
#include "StreamNetDealer.h"

using std::make_shared;
using std::make_unique;

namespace SMNetwork
{
	TcpChannelImpl::TcpChannelImpl(unique_ptr<asio::ip::tcp::socket> sock, uint32_t sockNo) 
		:_sock(move(sock))
		, _sockNo(sockNo)
	{
		_bRecvMsg = make_unique<string>();
		_bRecv = make_unique<string>();
	}

	TcpChannelImpl::TcpChannelImpl(TcpChannelImpl&& right)
	{
		_sock = move(right._sock);
		_sockNo = right._sockNo;
		right._sock = nullptr;
	}

	asio::awaitable<bool> TcpChannelImpl::sendPack(string_view src, uint32_t no)
	{
		bool bret{ false };
		SPDLOG_INFO("send platform message data length {} data {}", src.length(), string_view(src.begin() + 8, src.end()));
		SMNetwork::StreamNetDealer _netpack;
		string strhead(_netpack.HeadLen(), '\0');
		_netpack.pack(no, src.length(), span(strhead.begin(), _netpack.HeadLen()));
		int tryIndex{ 0 };
		int tryCount = _retryCount;
		size_t len{ 0 };
		size_t hadsend{ 0 };
		//first send net head
		while (++tryIndex < tryCount && hadsend < _netpack.HeadLen())
		{
			BEGIN_ASIO;
			len = co_await _sock->async_send(asio::buffer(strhead.data() + hadsend, _netpack.HeadLen() - hadsend), asio::use_awaitable);
			END_ASIO;
			if (len > 0)
			{
				hadsend += len;
			}
		}
		if (hadsend != _netpack.HeadLen())
		{
			SPDLOG_WARN("send tcp pack header failed");
			co_return bret;
		}
		//second send msg body
		tryIndex = 0, hadsend = 0;
		while (++tryIndex < tryCount && hadsend < src.length())
		{
			BEGIN_ASIO;
			len = co_await _sock->async_send(asio::buffer(src.data() + hadsend, src.length() - hadsend), asio::use_awaitable);
			END_ASIO;
			if (len > 0)
			{
				hadsend += len;
			}
		}
		if (hadsend != src.length())
		{   
			co_return bret;
		}
		bret = true;
		co_return bret;
	}
	asio::awaitable<tuple<uint32_t, shared_ptr<string>>> TcpChannelImpl::recvPack()
	{
		SMNetwork::StreamNetDealer _netpack;
		int tryIndex{ 0 };
		int tryCount = _retryCount;
		size_t len{ 0 };
		_bRecvMsg->clear();
		uint32_t req{ 0 };
		//first get head
		while (++tryIndex < tryCount && _bRecv->length() < _netpack.HeadLen())
		{
			BEGIN_ASIO;
			len = co_await _sock->async_read_some(asio::buffer(_recvasio), asio::use_awaitable);
			END_ASIO;
			if (len > 0)
			{
				tryIndex = 0;
				_bRecv->append(_recvasio.begin(), _recvasio.begin() + len);
			}
			else
			{
				SPDLOG_WARN("current recv data len {}", len);
			}
		}
		tryIndex = 0;
		if (_bRecv->length() >= _netpack.HeadLen()) //unpack net head
		{
			if (_netpack.unpack(req, string_view(_bRecv->begin(), _bRecv->end())))
			{
				tryIndex = 0;
				while (++tryIndex < tryCount && _bRecv->length() < _netpack.HeadLen() + _netpack.Len())
				{
					BEGIN_ASIO;
					len = co_await _sock->async_read_some(asio::buffer(_recvasio), asio::use_awaitable);
					END_ASIO;
					SPDLOG_INFO("current recv binary data length {}", len);
					if (len > 0)
					{
						_bRecv->append(_recvasio.begin(), _recvasio.begin() + len);
						tryIndex = 0;
					}
				}
				if (_bRecv->length() >= _netpack.HeadLen() + _netpack.Len())
				{
					_bRecvMsg->clear();
					_bRecvMsg->append(_bRecv->begin() + _netpack.HeadLen() + _netpack.Len(), _bRecv->end());
					_bRecv->erase(_bRecv->begin() + _netpack.HeadLen() + _netpack.Len(), _bRecv->end());
					//swap _bRecv and bRecvMsg
					auto tempswap = std::move(_bRecvMsg);
					_bRecvMsg = std::move(_bRecv);
					_bRecv = std::move(tempswap);
					co_return std::make_tuple(_netpack.getNo(), make_shared<string>(_bRecvMsg->data() + _netpack.HeadLen(), _bRecvMsg->length() - _netpack.HeadLen()));
				}
				else
				{
					SPDLOG_WARN("recv buf length {}, need length in net head is {}", _bRecv->length(), _netpack.Len());
				}
			}
		}
		SPDLOG_WARN("recv packet failed");
		co_return make_tuple(_netpack.getNo(), make_shared<string>());
	}

	uint32_t TcpChannelImpl::sockNo()
	{
		return _sockNo;
	}

	
	void TcpChannelImpl::setSockNo(uint32_t sockno)
	{
		_sockNo = sockno;
	}

}
