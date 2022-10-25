
#pragma once
#include "hotupdateExport.h"
#include "spdlog/spdlog.h"
#include "asio/steady_timer.hpp"
#include "myconcept.h"
#include "IOContextManager.h"
namespace SMHotupdate
{
class HOTUPDATE_EXPORT CoEvent
{
public:
	explicit CoEvent(asio::io_context& io_service=IOCTX, uint64_t milliseconds=3600000);
	CoEvent(const CoEvent& ) = delete;
	CoEvent(const CoEvent&&) = delete;
	~CoEvent();
	asio::awaitable<void> async_wait();
	void reset();
	void trigger();
	void trigger_all();
private:
	asio::steady_timer timer_;
	uint64_t milliseconds_;
};

}

