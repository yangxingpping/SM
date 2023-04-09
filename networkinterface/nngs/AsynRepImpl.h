#pragma once
#include "networkinterfaceExport.h"
#include "asio/awaitable.hpp"
#include "asio/experimental/concurrent_channel.hpp"
#include "nng/nng.h"
#include <stdint.h>
#include <stdbool.h>
#include <memory>
#include <tuple>
#include <string>
#include <string_view>

using std::string_view;
using std::unique_ptr;
using std::tuple;
using std::shared_ptr;
using std::string;

enum class EStateType
{
	WaitReq,
	WaitRep,
};

namespace SMNetwork
{
	enum class NngStatus
	{
		INIT,	//context initialized
		RECV,	//context recv finish
		WAIT,	//context wait response
		SEND,	//context send response finish
		AIOERROR, //aio operation error
	};

	class NETWORKINTERFACE_EXPORT AsynRepImpl
	{
	public:
		static void rep_callback(void* pRep);
		static void rep_cancel(nng_aio* aio, void* arg, int rv);
	public:
		AsynRepImpl( uint32_t sockNo);
		AsynRepImpl(const AsynRepImpl& ref) = delete;
		AsynRepImpl(AsynRepImpl&& right);
		asio::awaitable<bool> sendPack(string_view src, uint32_t no);
		asio::awaitable<tuple<uint32_t, shared_ptr<string>>> recvPack();
		uint32_t sockNo();
		void setSockNo(uint32_t sockno);
		void cleanReqRepCache();
	public:
		nng_aio* _aio;
		nng_ctx _ctx;
		uint32_t _sockno;
		EStateType _state{ EStateType::WaitReq };
		NngStatus _nStatus{ NngStatus::INIT };
		nng_msg* _req{ nullptr };
		nng_msg* _rep{ nullptr };
		shared_ptr<asio::experimental::concurrent_channel<void(asio::error_code, string_view)>> _msgs;
	};
}
