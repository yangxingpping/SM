
#include "AsynReq.h"

#include "nng/protocol/reqrep0/req.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "magic_enum.hpp"
#include "hotupdate.h"

namespace SMNetwork
{

	static void _asynReqCallback(void* arg) {
		struct AsyncBase::work* w = (struct AsyncBase::work*)arg;
		nng_msg* msg;
		int rv;
		switch (w->state)
		{
		case AsyncBase::State::RECV:
		{
			if ((rv = nng_aio_result(w->aio)) != 0)
			{
			}
			msg = nng_aio_get_msg(w->aio);
			size_t msglen = nng_msg_len(msg);
			SPDLOG_INFO("recv data len {}", msglen);

			asio::co_spawn(*IOCTX, w->_rep->messageTask(w, msg), asio::detached);
		}
		break;
		case AsyncBase::State::SEND:
			if ((rv = nng_aio_result(w->aio)) != 0)
			{
				nng_msg_free(w->msg);
			}
			w->state = AsyncBase::State::RECV;
			nng_ctx_recv(w->ctx, w->aio);
			break;
		default:
			break;
		}
	}

	AsyncReq::AsyncReq(string ip, uint16_t port, ChannelType ctype, NNgTransType trans /*= NNgTransType::TCP*/, NngSockImplType socktype /*= NngSockImplType::NormalSock*/, int concurrent /*= 1024*/)
		: NNGCommBase(ip, port, ctype, trans, socktype)
		, _concurrentcount(concurrent)
	{

	}

	void AsyncReq::init(ServeMode mode, shared_ptr<SMNetwork::PackDealerBase> dealer)
	{
		int rv = 0;
		string addr = getAddr();
		_mode = mode;
		
		switch (Socktype())
		{
		case NngSockImplType::NormalSock:
		{
			rv = nng_req0_open(&_socket);
		}break;
		case NngSockImplType::RawSock:
		{
			rv = nng_req0_open_raw(&_socket);
		}break;
		default:
			break;
		}

		SPDLOG_INFO("start async response node channel type {} on addr {}, sock type {}, mode type {}",
			magic_enum::enum_name(getChannelType()), addr, magic_enum::enum_name(Socktype()), magic_enum::enum_name(_mode));
		for (int i = 0; i < concurrentcount(); ++i)
		{
			auto w = std::make_shared<work>();
			w->_rep = this;
			w->_dealer = shared_ptr<SMNetwork::PackDealerBase>(dealer->clone());
			if ((rv = nng_aio_alloc(&w->aio, &_asynReqCallback, w.get()) != 0))
			{
				assert(0);
			}
			if ((rv = nng_ctx_open(&w->ctx, _socket)) != 0)
			{
				assert(0);
			}
			w->state = INIT;
			_cons.enqueue(w);

		}
		switch (_mode)
		{
		case ServeMode::SBind:
		{
			rv = nng_listen(_socket, addr.c_str(), NULL, 0);
			assert(rv == 0);
		}break;
		case ServeMode::SConnect:
		{
			rv = nng_dial(_socket, addr.c_str(), NULL, 0);
			assert(rv == 0);
		}break;
		default:
		{
			assert(0);
		}break;
		}
		
	}

	void AsyncReq::init(ServeMode mode)
	{

	}

	int AsyncReq::concurrentcount()
	{
		return _concurrentcount;
	}

	asio::awaitable<void> AsyncReq::messageTask(work* sock, nng_msg* msg)
	{
		std::string nnmsg;
		nnmsg.clear();
		assert(sock);
		nnmsg.assign((char*)nng_msg_body(msg), (char*)nng_msg_body(msg) + nng_msg_len(msg));
		string strreq;
		string strrep;
		BEGIN_ASIO;
		(*sock->resp) = *(co_await sock->_dealer->dealmsgc(nnmsg));
		END_ASIO;
		sock->_ev->trigger_all();
		
		co_return;
	}

	asio::awaitable<std::string*> AsyncReq::reqrep(std::string req)
	{
		std::string* rep = nullptr;
		shared_ptr<work> w = nullptr;
		if (_cons.try_dequeue(w))
		{
			if (w->msg != nullptr)
			{
				nng_msg_clear(w->msg);
				w->msg = nullptr;
			}
			*(w->_dealer->reqSeq()) = SMUtils::getSeqNum();
			w->_ev = std::make_shared<SMHotupdate::CoEvent>(*IOCTX);
			auto ppx = w->_dealer->pack(w->_dealer->reqSeq(), string_view(req));
			nng_msg_alloc(&w->msg, 0);
			nng_msg_append(w->msg, ppx->data(), ppx->length());
			nng_aio_set_msg(w->aio, w->msg);
			w->state = SEND;
			nng_ctx_send(w->ctx, w->aio);
			BEGIN_ASIO;
			co_await w->_ev->async_wait();
			rep = w->resp;
			END_ASIO;
			_cons.enqueue(w);
		}
		co_return rep;
	}

	shared_ptr<SMNetwork::AsyncBase::work> AsyncReq::getWorker()
	{
		shared_ptr<SMNetwork::AsyncBase::work> ret{ nullptr };
		if (!_cons.try_dequeue(ret))
		{
			SPDLOG_WARN("get worker for channel {} failed, may be u should increase concurrent count", magic_enum::enum_name(getChannelType()));
		}
		return ret;
	}

}
