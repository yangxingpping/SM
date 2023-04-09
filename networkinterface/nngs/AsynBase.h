
#pragma once
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "spdlog/spdlog.h"
#include "nngpp/msg.h"
#include "PackDealerBase.h"

#include "CoEvent.h"
#include <string_view>

using std::string_view;

#define NN_LOG_NEED_ZERO_ASSIGN_BOOL(nnop, bret) if(nnop!=0)\
	{\
		SPDLOG_WARN("nng return {} , strerror {}", nnop, nng_strerror(nnop)); \
		bret = false;\
	}
#define NN_LOG_NEED_POSITIVE_ASSIGN_BOOL(nnop, bret) if(nnop<0)\
	{\
		SPDLOG_WARN("nng return {} , strerror {}", nnop, nng_strerror(nnop)); \
		bret = false;\
	}

#define NN_LOG_NEED_ZERO_RET_BOOL(nnop) if(nnop!=0)\
	{\
		SPDLOG_WARN("nng return {} , strerror {}", nnop, nng_strerror(nnop)); \
		return false;\
	}
#define NN_LOG_NEED_POSITIVE_RET_BOOL(nnop) if(nnop<0)\
	{\
		SPDLOG_WARN("nng return {} , strerror {}", nnop, nng_strerror(nnop)); \
		return false;\
	}

#define NN_LOG_NEED_ZERO_RET_VOID(nnop) if(nnop!=0)\
	{\
		SPDLOG_WARN("nng return {} , strerror {}", nnop, nng_strerror(nnop)); \
	}
#define NN_LOG_NEED_POSITIVE_RET_VOID(nnop) if(nnop<0)\
	{\
		SPDLOG_WARN("nng return {} , strerror {}", nnop, nng_strerror(nnop)); \
	}

namespace SMNetwork
{
	class AsyncBase
	{
	public:
		enum State
		{
			INIT,
			RECV,
			WAIT,
			SEND
		};
		class work
		{
		public:
			work() :aio(nullptr), msg(nullptr), _rep(nullptr), _ev(nullptr), _dealer(nullptr) {}
			asio::awaitable<string> reqrep(string_view req)
			{
				string ret;
				*(_dealer->reqSeq()) = SMUtils::getSeqNum();
				if (msg != nullptr)
				{
					nng_msg_clear(msg);
					msg = nullptr;
				}
				_ev = std::make_shared<SMHotupdate::CoEvent>(*IOCTX);
				auto ppx = _dealer->pack(_dealer->reqSeq(), req);
				resp = &ret;
				senddata(string_view(*ppx));
				BEGIN_ASIO;
				co_await _ev->async_wait();
				END_ASIO;
				co_return ret;
			}

			asio::awaitable<string> reqrep(string_view req, uint16_t assc)
			{
				string ret;
				_ev = std::make_shared<SMHotupdate::CoEvent>(*IOCTX);
				_dealer->setAssc(assc);
				*(_dealer->reqSeq()) = SMUtils::getSeqNum();
				auto ppx = _dealer->pack(_dealer->reqSeq(), req);
				resp = &ret;
				if (!senddata(string_view(*ppx)))
				{
					co_return ret;
				}
				BEGIN_ASIO;
				co_await _ev->async_wait();
				END_ASIO;
				co_return ret;
			}

			asio::awaitable<string> reqrep(string& rreq)
			{
				string_view req = string_view{ rreq };
				string ret;
				if (msg != nullptr)
				{
					nng_msg_clear(msg);
					msg = nullptr;
				}
				*(_dealer->reqSeq()) = SMUtils::getSeqNum();
				_ev = std::make_shared<SMHotupdate::CoEvent>(*IOCTX);
				auto ppx = _dealer->pack(_dealer->reqSeq(), req);
				resp = &ret;
				senddata(string_view(*ppx));
				BEGIN_ASIO;
				co_await _ev->async_wait();
				END_ASIO;
				co_return ret;
			}

			asio::awaitable<string> reqrep(string& rreq, uint16_t assc)
			{
				string_view req{ rreq };
				string ret;
				_ev = std::make_shared<SMHotupdate::CoEvent>(*IOCTX);
				_dealer->setAssc(assc);
				*(_dealer->reqSeq()) = SMUtils::getSeqNum();
				auto ppx = _dealer->pack(_dealer->reqSeq(), req);
				resp = &ret;
				if (!senddata(string_view(*ppx)))
				{
					co_return ret;
				}
				BEGIN_ASIO;
				co_await _ev->async_wait();
				END_ASIO;
				co_return ret;
			}

			bool senddata(std::string_view sendmsg)
			{
				bool bret = false;
				int nnop = 0;
				if (msg != nullptr)
				{
					nng_msg_clear(msg);
					msg = nullptr;
				}
				nnop = nng_msg_alloc(&msg, 0);
				if (nnop != 0)
				{
					SPDLOG_WARN("alloc msg failed, err {} str err {}", nnop, nng_strerror(nnop));
					return false;
				}
				nnop = nng_msg_append(msg, sendmsg.data(), sendmsg.length());
				if (nnop != 0)
				{
					SPDLOG_WARN("msg append data failed, err {} str err {}", nnop, nng_strerror(nnop));
					return false;
				}
				nng_aio_set_msg(aio, msg);
				state = SEND;
				nng_ctx_send(ctx, aio);
				return true;
			}
		public:
			State state;
			nng_aio* aio;
			nng_msg* msg;
			nng_ctx ctx;
			AsyncBase* _rep;
			std::shared_ptr<SMHotupdate::CoEvent> _ev;
			std::shared_ptr<SMNetwork::PackDealerBase> _dealer;
			std::string* resp;
		};
	public:
		virtual int concurrentcount() = 0;
		virtual asio::awaitable<void> messageTask(work* sock, nng_msg* msg) = 0;
		virtual shared_ptr<work> getWorker() { assert(0); return nullptr; }
	};

}
