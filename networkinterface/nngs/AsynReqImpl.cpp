#include "AsynReqImpl.h"
#include "hotupdate.h"
#include "magic_enum.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "spdlog/spdlog.h"

using std::make_tuple;
using std::make_shared;

namespace SMNetwork
{
	AsynReqImpl::AsynReqImpl(nng_socket addr, uint32_t sockNo)
		:_sock(addr)
		,_sockno(sockNo)
	{ 
		_msgs = make_shared<asio::experimental::concurrent_channel<void(asio::error_code, string_view)>>(*IOCTX, 5);
	}

	AsynReqImpl::AsynReqImpl(AsynReqImpl&& right)
		:_sockno(right._sockno)
	{
		
	}

	asio::awaitable<bool> AsynReqImpl::sendPack(string_view src, uint32_t no)
	{
		bool bret{ false };
		int nnop{ 0 };
		
		co_return bret;
	}

	asio::awaitable<tuple<uint32_t, shared_ptr<string>>> AsynReqImpl::recvPack()
	{
		uint32_t msgno{ 0 };
		shared_ptr<string> msg{nullptr};
		
		co_return make_tuple(msgno, msg);
	}

	uint32_t AsynReqImpl::sockNo()
	{
		return _sockno;
	}

	void AsynReqImpl::setSockNo(uint32_t sockno)
	{
		_sockno = sockno;
	}
}
