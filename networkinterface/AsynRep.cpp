 #include "AsynRep.h"
 #include "spdlog/spdlog.h"
 #include "Utils.h"
#include "hotupdate.h"
 #include "magic_enum.hpp"
#include "asio/co_spawn.hpp"
#include "asio/awaitable.hpp"
#include "asio/detached.hpp"
#include "MessageSplitFuncs.h"
#include "nng/protocol/reqrep0/rep.h"

namespace SMNetwork
{

    static void _asynRepCallback(void* arg) {
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

AsynRep::AsynRep(string ip, uint16_t port, ChannelType ctype,  NNgTransType trans, NngSockImplType socktype/* = NngSockImplType::NormalSock*/, int concurrent /*= 1024*/)
    : NNGCommBase(ip, port, ctype, trans, socktype)
    , _concurrent(concurrent)
{
    
}

void AsynRep::init(ServeMode mode, shared_ptr<PackDealerBase> dealer) {
    int rv = 0;
    _serveMode = mode;
    string addr = getAddr();
    
    switch (Socktype())
    {
    case NngSockImplType::NormalSock:
    {
        rv = nng_rep0_open(&_socket);
    }break;
    case NngSockImplType::RawSock:
    {
        rv = nng_rep0_open_raw(&_socket);
    }break;
    default:
    {
        assert(0);
    }break;
    }
    SPDLOG_INFO("start async response node channel type {} on addr {}, sock type {}",
     magic_enum::enum_name(getChannelType()), addr, magic_enum::enum_name(Socktype()));
    _cons.clear();
    for (int i=0;i<_concurrent;++i)
    {
        auto w = std::make_shared<work>();
        w->_rep = this;
        w->_dealer = std::shared_ptr<SMNetwork::PackDealerBase>(dealer->clone());
        if ((rv = nng_aio_alloc(&w->aio, &_asynRepCallback, w.get()) != 0))
        {
        }
        if ((rv = nng_ctx_open(&w->ctx, _socket)) != 0)
        {
            
        }
        w->state = INIT;
        _cons.push_back(w);
       
    }
    switch (getServeMode())
    {
    case ServeMode::SBind:
    {
		rv = nng_listen(_socket, addr.c_str(), NULL, 0);
		if (rv != 0)
		{
			SPDLOG_ERROR("listen addr {} failed with error {} for rep, strerror {}", addr, rv, nng_strerror(rv));
		}
    }break;
    case ServeMode::SConnect:
    {
        rv = nng_dial(_socket, addr.c_str(), NULL, 0);
        if (rv != 0)
        {
            SPDLOG_ERROR("dial addr {} failed with error for rep", addr, rv);
        }
    }break;
    default:
        break;
    }
	for (auto c : _cons)
	{
		c->state = RECV;
		nng_ctx_recv(c->ctx, c->aio);
	}
    
   
    
}

void AsynRep::init(ServeMode mode)
{

}

asio::awaitable<void> AsynRep::messageTask(struct work* w, nng_msg* msg) { 
    
    w->msg = msg;
    w->state = WAIT;
    std::string recvmsg;
    recvmsg.assign((char*)nng_msg_body(msg), (char*)nng_msg_body(msg) + nng_msg_len(msg));
    string strreq;
    string strrep;
    BEGIN_ASIO;
    auto p2 = co_await w->_dealer->dealmsg(recvmsg);
    w->senddata(string_view(*p2));
    END_ASIO;
    co_return;
}

ServeMode AsynRep::getServeMode()
{
    return _serveMode;
}

int AsynRep::concurrentcount()
{
    return _concurrent;
}

}
