
#pragma once
#include "hotupdateExport.h"
#include "spdlog/spdlog.h"
#include "asio/steady_timer.hpp"
#include "myconcept.h"
#include "IOContextManager.h"

namespace SMHotupdate
{
	enum class ETrigeType
	{
		TimeOut,
		Cancel,
	};
	ETrigeType HOTUPDATE_EXPORT getEventType(asio::error_code& ec);

class HOTUPDATE_EXPORT CoEvent
{
public:
	explicit CoEvent(asio::io_context& io_service, uint64_t milliseconds=3600000);
	CoEvent(const CoEvent& ) = delete;
	CoEvent(const CoEvent&&) = delete;
	~CoEvent();
	asio::awaitable<void> async_wait();
	asio::awaitable<SMHotupdate::ETrigeType> async_time_wait();
	void reset();
	void trigger();
	void trigger_all();
private:
	asio::steady_timer timer_;
	uint64_t milliseconds_;
};

}

