
#include "SyncReq.h"
#include "spdlog/spdlog.h"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"

namespace SMNetwork
{

	SyncReq::SyncReq(string ip, uint16_t port, ChannelType ctype,  NNgTransType trans /*= NNgTransType::TCP*/, NngSockImplType socktype /*= NngSockImplType::NormalSock*/)
		: NNGCommBase(ip, port, ctype, trans, socktype)
	{

	}

	void SyncReq::init(ServeMode mode)
	{
		_dealer = nullptr;
		_mode = mode;
	}

	void SyncReq::init(ServeMode mode, shared_ptr<PackDealerBase> dealer)
	{
		_dealer = dealer;
		_mode = mode;
	}

	std::string SyncReq::reqrep(string_view req, uint16_t assc)
	{
		std::string ret;
		
		int nnop{ -1 };
		auto addr = getAddr();
		if (req.empty())
		{
			SPDLOG_WARN("send req to addr {} empty", addr);
			assert(!req.empty());
			return ret;
		}
		SPDLOG_INFO("nng req send data {}", req);
		nng_socket sock{ NNG_SOCKET_INITIALIZER };
		nng_dialer dialer{ NNG_DIALER_INITIALIZER };
		nnop = nng_req0_open(&sock);
		if (nnop != 0)
		{
			SPDLOG_WARN("open req socket with error {} {}", nnop, nng_strerror(nnop));
			return ret;
		}
		nnop = nng_dialer_create(&dialer, sock, addr.c_str());
		if (nnop != 0)
		{
			SPDLOG_WARN("nng_dialer_create on addr {} failed with {} {}", addr, nnop, nng_strerror(nnop));
			nng_close(sock);
			return ret;
		}
		nnop = nng_dialer_start(dialer, NNG_FLAG_NONBLOCK);
		if (nnop != 0)
		{
			SPDLOG_WARN("nng_dialer_start on addr {} failed with {} {}", addr, nnop, nng_strerror(nnop));
			nng_close(sock);
			return ret;
		}
		void* ptrep = nullptr;
		size_t len = 0;
		assert(_dealer != nullptr);
		if (_dealer == nullptr)
		{
			SPDLOG_WARN("dealer is nullptr");
			return ret;
		}
		*(_dealer->reqSeq()) = SMUtils::getSeqNum();
		auto senddata = _dealer->pack(_dealer->reqSeq(), string_view(req));
		std::string strreq{ senddata->begin(), senddata->end() };
		nnop = nng_send(sock, &strreq[0], strreq.length(), 0);
		if (nnop != 0)
		{
			assert(nnop==0);
			SPDLOG_WARN("send req to addr {} failed with {} {}", addr, nnop, nng_strerror(nnop));
			nng_close(sock);
			return ret;
		}
		nnop = nng_recv(sock, &ptrep, &len, NNG_FLAG_ALLOC);
		if (nnop != 0)
		{
			assert(nnop == 0);
			SPDLOG_WARN("recv rep to addr {} failed with {} {}", addr, nnop, nng_strerror(nnop));
			nng_close(sock);
			if (ptrep!=nullptr)
			{
				nng_free(ptrep, len);
			}
			ptrep = nullptr;
			return ret;
		}
		auto prep = _dealer->unpack(_dealer->reqSeq(), string_view{ (char*)ptrep, len });
		ret = *prep;
		nng_free(ptrep, len);
		ptrep = nullptr;
		nng_close(sock);
		sock = { NNG_SOCKET_INITIALIZER };
		return ret;
	}

}

