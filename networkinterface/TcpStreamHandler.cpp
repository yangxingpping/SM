#include "../configs/Configs.h"
#include "Utils.h"
#include "IOContextManager.h"
#include "MesageHandlerBase.h"
#include "StreamHandler.h"
#include "coros.h"
#include "asio/local/stream_protocol.hpp"
#include "TcpStreamHandler.h"
#include "spdlog/spdlog.h"
#include "MessageSplitFuncs.h"
#include "fmt/core.h"

#include "conf.h"

#include <memory>
#include <vector>
#include <tuple>
#include <stdbool.h>

using asio::ip::tcp;
using asio::steady_timer;
using std::shared_ptr;
using std::function;
using std::string;
using std::string_view;
using std::vector;
using std::tuple;
using std::make_tuple;

namespace SMNetwork
{

	TcpStreamHandler::TcpStreamHandler(TcpStreamHandler&& c)
	{
		_socket = c._socket;
		_sendbuf = std::move(c._sendbuf);
		_recvasio = std::move(c._recvasio);
		_recvbuf = std::move(c._recvbuf);
		_recvmsg = std::move(c._recvmsg);
		_recvmsgbin = std::move(c._recvmsgbin);
		_nethead = c._nethead;
	}

	TcpStreamHandler::TcpStreamHandler(shared_ptr<asio::ip::tcp::socket> sock, NetHeadType nettype):_socket(sock), _nethead(nettype)
	{
		assert(sock);
	}

	SMNetwork::TcpStreamHandler& TcpStreamHandler::operator=(TcpStreamHandler&& c)
	{
		_socket = c._socket;
		_sendbuf = std::move(c._sendbuf);
		_recvasio = std::move(c._recvasio);
		_recvbuf = std::move(c._recvbuf);
		_recvmsg = std::move(c._recvmsg);
		_recvmsgbin = std::move(c._recvmsgbin);
		_nethead = c._nethead;
		return *this;
	}

	TcpStreamHandler::~TcpStreamHandler() {}

	shared_ptr<asio::ip::tcp::socket> TcpStreamHandler::getHandler() 
	{
		return _socket;
	}

	string TcpStreamHandler::getFullAddr()
	{
		if (_fullAddr.empty())
		{
			auto rep = _socket->remote_endpoint();
			auto lep = _socket->local_endpoint();
			_fullAddr = fmt::format("[{}:{}]=[{}:{}]", rep.address().to_string(), rep.port(), lep.address().to_string(), lep.port());
		}
		return _fullAddr;
	}

	asio::awaitable<tuple<bool, std::string*>> TcpStreamHandler::recvPack(uint16_t retrycount/* = 60*/)
	{
		bool bsplit = false;
		auto timeoutmillsecond = SMCONF::getTransportConfig()->_timeout;
		_curpacklen = 0;
		if (retrycount > 100)
		{
			retrycount = 100;
			SPDLOG_WARN("retry count is too big, set to default max value {}", retrycount);
		}
		uint16_t tryindex = 0;
		while (!bsplit && tryindex++ < retrycount)
		{
			BEGIN_ASIO;
			_recvasio.fill('\0');
			size_t len = co_await _socket->async_read_some(asio::buffer(_recvasio), asio::use_awaitable);
			SPDLOG_INFO("the No. {} current recv data length {} from {}", tryindex, len, getFullAddr());
			if (len > 0)
			{
				_recvbuf.insert(_recvbuf.end(), _recvasio.begin(), _recvasio.begin() + len);
			}
			else
			{
				SPDLOG_WARN("current recv 0 length data from {}", getFullAddr());
			}
			END_ASIO;
			if (_recvbuf.empty())
			{
				SPDLOG_WARN("recv data from tcp socket return 0 {}, close it ?");
				co_return std::make_tuple(bsplit, &_recvmsg);
			}
			SPDLOG_INFO("pack type {}, current pack len {}, recv buf len {}", magic_enum::enum_name(getNetHeadType()), _curpacklen, _recvbuf.size());
			switch (getNetHeadType())
			{
			case NetHeadType::FixPackLenPlaceHolder:
			{
				if (_curpacklen == 0 && !_recvbuf.empty())
				{
					_recvmsg.clear();
					if (!SMUtils::unpackuint32(_recvbuf, _curpacklen))
					{
						SPDLOG_WARN("split message header failed from data {}", _recvbuf);
					}
				}
				if (_curpacklen>0 && _recvbuf.size() >= _curpacklen)
				{
					_recvmsg.assign(_recvbuf.begin(), _recvbuf.begin() + _curpacklen);
					_recvbuf.erase(_recvbuf.begin(), _recvbuf.begin() + _curpacklen);
					_curpacklen = 0;
					SPDLOG_INFO("success split 1 message, message length {}", _recvmsg.length());
					bsplit = true;
				}
				else //just get partial of message
				{
					SPDLOG_WARN("only get message length {} need length {}, split flag {}", _recvbuf.size(), _curpacklen, bsplit);
				}
			}break;
			case NetHeadType::NoPlaceHolderFixLen:
			{
				assert(_curpacklen != 0);
				if (_recvbuf.size() >= _curpacklen)
				{
					_recvmsg.assign(_recvbuf.begin(), _recvbuf.begin() + _curpacklen);
					_recvbuf.erase(_recvbuf.begin(), _recvbuf.begin() + _curpacklen);
					bsplit = true;
				}
				else //just get partial of message
				{
					SPDLOG_WARN("only get message length {} need length {}, split flag {}", _recvbuf.size(), _curpacklen, bsplit);
				}
			}break;
			case NetHeadType::NoPlaceHolderVarLen:
			{
				_recvmsg.assign(_recvbuf.begin(), _recvbuf.begin() + _recvbuf.size());
				_recvbuf.erase(_recvbuf.begin(), _recvbuf.begin() + _recvbuf.size());
				bsplit = true;
			}break;
			default:
			{
				assert(0);
			}break;
			}
		}
		co_return std::make_tuple(bsplit, &_recvmsg);
	}
	asio::awaitable<bool> TcpStreamHandler::sendPack(string_view msg)
	{
		bool bret = true;
		auto timeoutmillsecond = SMCONF::getTransportConfig()->_timeout;
		_sendbuf.clear();
		_packnethead(static_cast<uint32_t>(msg.size()));
		_sendbuf.insert(_sendbuf.end(), msg.begin(), msg.end());
		BEGIN_ASIO;
			while (!_sendbuf.empty())
			{
				vector<uint8_t> sends(_sendbuf.begin(), _sendbuf.end());
				size_t len = co_await _socket->async_send(asio::buffer(sends), asio::use_awaitable);
				SPDLOG_INFO("current need send length [{}], actual send length [{}]", _sendbuf.size(), len);
				if (len != sends.size()) //only send some, wait for can write
				{
					co_await _socket->async_wait(asio::local::stream_protocol::socket::wait_write, asio::use_awaitable);
				}
				_sendbuf.erase(_sendbuf.begin(), _sendbuf.begin() + len);
			}
		END_ASIO;
		if (!_sendbuf.empty())
		{
			SPDLOG_WARN("send data failed");
		}
		co_return bret;
	}

	void TcpStreamHandler::_packnethead(uint32_t packlen)
	{
		_sendbuf.clear();
		assert(packlen < UINT32_MAX);
		switch (getNetHeadType())
		{
		case NetHeadType::FixPackLenPlaceHolder:
		{
			SMUtils::packuint32(_sendbuf, packlen);
		}break;
		case NetHeadType::NoPlaceHolderFixLen:
		{
			assert(packlen == _curpacklen);
		}break;
		case NetHeadType::NoPlaceHolderVarLen:
		{

		}break;
		default:
		{
			assert(0);
		}break;
		}
	}

	NetHeadType TcpStreamHandler::getNetHeadType()
	{
		return _nethead;
	}

}
