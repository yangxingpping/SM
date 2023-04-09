#include "ReqImpl.h"
#include "hotupdate.h"
#include "magic_enum.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "spdlog/spdlog.h"
#include "hotupdate.h"
#include "oneshot.hpp"
#include <memory>

#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/use_awaitable.hpp"
#include "asio/deferred.hpp"

using std::make_tuple;
using std::make_shared;
using std::move;

namespace SMNetwork
{
	ReqImpl::ReqImpl(nng_socket sock, uint32_t sockNo)
		:_sockno(sockNo)
		,_sock(sock)
	{ 
	}

	ReqImpl::ReqImpl(ReqImpl&& right)
		:_sockno(right._sockno)
		,_sock(right._sock)
	{
		right._sockno=0;
	}

	asio::awaitable<bool> ReqImpl::sendPack(string_view src, uint32_t no)
	{
		bool bret{ false };
		auto [s, r] = oneshot::create<bool>();
		TFNET->silent_async([this, &s, src]() {
			auto nnop = nng_send(this->_sock, (void*)(src.data()), src.length(), 0);
			auto bflag = nnop == src.length();
			asio::co_spawn(*IOCTX, [&s, bflag]()->asio::awaitable<void> {
				s.send(bflag);
				co_return;
				}(), asio::detached);
			});
		BEGIN_ASIO;
		co_await r.async_wait(asio::deferred);
		bret = r.get();
		END_ASIO;
		_msgno = no;
		co_return bret;
	}

	asio::awaitable<tuple<uint32_t, shared_ptr<string>>> ReqImpl::recvPack()
	{
		uint32_t msgno{ 0 };
		shared_ptr<string> msg{nullptr};
		auto [s, r] = oneshot::create<string_view>();
		TFNET->silent_async([this, &s]() {
			char* buf{ nullptr };
			size_t len{ 0 };
			auto nnop = nng_recv(this->_sock, &buf, &len, 0);
			assert(nnop > 0);
			asio::co_spawn(*IOCTX, [&s, buf, len]()->asio::awaitable<void> {
				s.send(string_view(buf, len));
				co_return;
				}(), asio::detached);
			});
		BEGIN_ASIO;
		co_await r.async_wait(asio::deferred);
		auto msgv = r.get();
		msg = make_shared<string>(msgv.data(), msgv.length());
		END_ASIO;
		co_return make_tuple(msgno, msg);
	}

	uint32_t ReqImpl::sockNo()
	{
		return _sockno;
	}

	void ReqImpl::setSockNo(uint32_t sockno)
	{
		_sockno = sockno;
	}
}
