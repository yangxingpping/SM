
#pragma once

#include "networkinterfaceExport.h"
#include "NNGCommBase.h"
#include "AsynBase.h"

#include "concurrentqueue/concurrentqueue.h"

#include <vector>
using std::vector;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT AsyncReq : public NNGCommBase, public AsyncBase
	{
	public:
		AsyncReq(string ip, uint16_t port, ChannelType ctype, NNgTransType trans = NNgTransType::TCP, NngSockImplType socktype = NngSockImplType::NormalSock, int concurrent = 1024);
		void init(ServeMode mode, shared_ptr<SMNetwork::PackDealerBase> dealer);
		virtual void init(ServeMode mode) override;
		virtual int concurrentcount() override;
		virtual asio::awaitable<void> messageTask(work* sock, nng_msg* msg) override;
		asio::awaitable<std::string*> reqrep(std::string req);
		virtual shared_ptr<work> getWorker() override;
	private:
		moodycamel::ConcurrentQueue<shared_ptr<work>> _cons;
		int _concurrentcount;

		ServeMode _mode;
	};
}
