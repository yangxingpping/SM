#include "AsynRepImpl.h"
#include "hotupdate.h"
#include "magic_enum.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "spdlog/spdlog.h"

using std::make_tuple;
using std::make_shared;

namespace SMNetwork
{
	
	void AsynRepImpl::rep_callback(void* pRep)
	{
		int nop{ 0 };
		assert(pRep);
		AsynRepImpl* rep = (AsynRepImpl*)(pRep);
		switch (rep->_nStatus)
		{
		case NngStatus::INIT:
		{
			rep->cleanReqRepCache();
			rep->_nStatus = NngStatus::RECV;
			nng_ctx_recv(rep->_ctx, rep->_aio);
		}break;
		case NngStatus::RECV:
		{
			nop = nng_aio_result(rep->_aio);
			if (nop != 0)
			{
				SPDLOG_ERROR("aio at state {}", magic_enum::enum_name(rep->_nStatus));
				rep->_nStatus = NngStatus::AIOERROR;
				break;
			}
			rep->_nStatus = NngStatus::WAIT;
			rep->_req = nng_aio_get_msg(rep->_aio);
			nng_aio_defer(rep->_aio, &AsynRepImpl::rep_cancel, rep);
			assert(rep->_req != nullptr);
			//start coro to send send request
			asio::co_spawn(*IOCTX, [rep]()->asio::awaitable<void> {
				auto pmsg = (char*)nng_msg_body(rep->_req);
				auto msglen = nng_msg_len(rep->_req);
				BEGIN_ASIO;
				co_await rep->_msgs->async_send(asio::error_code{}, string_view{pmsg, msglen}, asio::use_awaitable);
				END_ASIO;
				co_return;
				}, asio::detached);
		}break;
		default:
			break;
		}
	}

	void AsynRepImpl::rep_cancel(nng_aio* aio, void* arg, int rv)
	{
		assert(aio);
		(void)(arg);
		(void)(rv);
		nng_aio_finish(aio, rv);
	}

	AsynRepImpl::AsynRepImpl(uint32_t sockNo)
		:_sockno(sockNo)
	{ 
		_msgs = make_shared<asio::experimental::concurrent_channel<void(asio::error_code, string_view)>>(*IOCTX, 5);
	}

	AsynRepImpl::AsynRepImpl(AsynRepImpl&& right)
		:_aio(right._aio)
		,_ctx(right._ctx)
		,_sockno(right._sockno)
		,_msgs(right._msgs)
	{
		right._aio = nullptr;
		right._msgs = nullptr;
	}

	asio::awaitable<bool> AsynRepImpl::sendPack(string_view src, uint32_t no)
	{
		bool bret{ false };
		int nnop{ 0 };
		if (_rep != nullptr)
		{
			nng_msg_clear(_rep);
			_rep = nullptr;
		}
		nnop = nng_msg_alloc(&_rep, 0);
		if (nnop != 0)
		{
			SPDLOG_WARN("alloc msg failed, err {} str err {}", nnop, nng_strerror(nnop));
			co_return bret;
		}
		nnop = nng_msg_append(_rep, src.data(), src.length());
		if (nnop != 0)
		{
			SPDLOG_WARN("msg append data failed, err {} str err {}", nnop, nng_strerror(nnop));
			co_return bret;
		}
		bret = nng_aio_begin(_aio);
		assert(bret);
		if(!bret)
		{
			SPDLOG_WARN("sock no.{} call nng_aio_begin() failed", _sockno);
			_nStatus = NngStatus::AIOERROR;
			co_return bret;
		}
		nng_aio_set_msg(_aio, _rep);
		nng_ctx_send(_ctx, _aio);
		nng_aio_finish(_aio, 0);
		assert(_nStatus == NngStatus::WAIT);
		co_return bret;
	}

	asio::awaitable<tuple<uint32_t, shared_ptr<string>>> AsynRepImpl::recvPack()
	{
		uint32_t msgno{ 0 };
		shared_ptr<string> msg{nullptr};
		assert(_nStatus == NngStatus::INIT);
		BEGIN_ASIO;
		auto vmsg = co_await _msgs->async_receive(asio::use_awaitable);
		msg = make_shared<string>(vmsg.data(), vmsg.length());
		END_ASIO;
		co_return make_tuple(msgno, msg);
	}

	uint32_t AsynRepImpl::sockNo()
	{
		return _sockno;
	}

	void AsynRepImpl::setSockNo(uint32_t sockno)
	{
		_sockno = sockno;
	}

	void AsynRepImpl::cleanReqRepCache()
	{
		if (_req)
		{
			nng_msg_free(_req);
			_req = nullptr;
		}
		if (_rep)
		{
			nng_msg_free(_rep);
			_rep = nullptr;
		}
	}

}
