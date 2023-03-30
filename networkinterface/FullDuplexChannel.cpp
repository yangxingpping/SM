
#include "FullDuplexChannel.h"
#include "LocalNetManager.h"
using std::make_shared;
namespace SMNetwork
{
	FullDuplexChannel::FullDuplexChannel(int sock, shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>> sender, shared_ptr<concurrent_channel<void(asio::error_code, shared_ptr<string>)>> recv)
		:_socket(sock)
		, _sender(sender)
		, _recv(recv)
	{
		SPDLOG_INFO("create sock {}, this {}", sock, fmt::ptr(this));
	}
	FullDuplexChannel::FullDuplexChannel(FullDuplexChannel& copy)
		:_socket(copy._socket)
		, _sender(copy._sender)
		, _recv(copy._recv)
	{
		SPDLOG_INFO("call reference constructor for sock {} this {}", _socket, fmt::ptr(this));
		copy._socket = -1;
		copy._sender = nullptr;
		copy._recv = nullptr;
	}
	FullDuplexChannel::FullDuplexChannel(FullDuplexChannel&& right) 
		:_socket(right._socket)
		, _sender(right._sender)
		, _recv(right._recv)
	{
		SPDLOG_INFO("call right value reference constructor for sock {}, this {}", _socket, fmt::ptr(this));
		right._socket = -1;
		right._sender = nullptr;
		right._recv = nullptr;
	}

	FullDuplexChannel::~FullDuplexChannel()
	{
		SPDLOG_INFO("destructor for FullDuplexChannel sock {}", _socket);
		_sender = nullptr;
		_recv = nullptr;
	}

	asio::awaitable<bool> FullDuplexChannel::sendPack(string_view src, uint32_t no)
	{
		SPDLOG_INFO("at send call channel object pointer {}", fmt::ptr(this));
		bool bret{ false };
		string strhead(_netpack.HeadLen(), '\0');
		_netpack.pack(no, src.length(), span(strhead.begin(), _netpack.HeadLen()));
		assert(_sender != nullptr);
		//send head
		BEGIN_ASIO;
		SPDLOG_INFO("start send head from sock {}", _socket);
		co_await _sender->async_send(asio::error_code{}, make_shared<string>(strhead), asio::use_awaitable);
		SPDLOG_INFO("end send head from sock {}", _socket);
		//send body
		SPDLOG_INFO("start send body from sock {}", _socket);
		co_await _sender->async_send(asio::error_code{}, make_shared<string>(src), asio::use_awaitable);
		SPDLOG_INFO("end send body from sock {}", _socket);
		bret = true;
		END_ASIO;
		assert(bret);
		co_return bret;
	}
	asio::awaitable<tuple<uint32_t, shared_ptr<string>>> FullDuplexChannel::recvPack()
	{
		SPDLOG_INFO("at recv call channel object pointer {}", fmt::ptr(this));
		uint32_t no{ 0 };
		shared_ptr<string> head;
		shared_ptr<string> body;
		assert(_recv != nullptr);
		BEGIN_ASIO;
		SPDLOG_INFO("start recv head to sock {}", _socket);
		head= co_await _recv->async_receive(asio::use_awaitable);
		SPDLOG_INFO("end recv head to sock {}", _socket);
		SPDLOG_INFO("start recv body to sock {}", _socket);
		body = co_await _recv->async_receive(asio::use_awaitable);
		SPDLOG_INFO("end recv body to sock {}", _socket);
		END_ASIO;
		auto bret = _netpack.unpack(no, string_view(head->data(), head->length()));
		assert(bret);
		SPDLOG_INFO("success recv message from sock {}, message No. {}", _socket, _netpack.getNo());
		co_return make_tuple(_netpack.getNo(), body);
	}

	uint32_t FullDuplexChannel::sockNo()
	{
		return _socket;
	}

	void FullDuplexChannel::setSockNo(uint32_t sockno)
	{
		_socket = sockno;
	}

}
