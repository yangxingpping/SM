
#include "CoEvent.h"
#include "asio/redirect_error.hpp"
#include "asio/use_awaitable.hpp"
using asio::awaitable;
using asio::use_awaitable;
using asio::redirect_error;

namespace SMHotupdate
{

CoEvent::CoEvent(
    asio::io_context& io_service, uint64_t milliseconds /* = 3600000*/)
    : timer_(io_service)
    , milliseconds_(milliseconds)
{
}

CoEvent::~CoEvent()
{

}

awaitable<void> CoEvent::async_wait()
{
    timer_.expires_after(std::chrono::milliseconds(milliseconds_));
    asio::error_code ec;
    co_await timer_.async_wait(redirect_error(use_awaitable, ec));
}

asio::awaitable<SMHotupdate::ETrigeType> CoEvent::async_time_wait()
{
    ETrigeType eRet{ ETrigeType::TimeOut };
    timer_.expires_after(std::chrono::milliseconds(milliseconds_));
    asio::error_code ec;
    co_await timer_.async_wait(redirect_error(use_awaitable, ec));
    eRet = getEventType(ec);
    co_return eRet;
}

void CoEvent::reset()
{
	timer_.cancel();
}

void CoEvent::trigger()
{
    timer_.cancel_one();
    
}
void CoEvent::trigger_all()
{ 
	timer_.cancel();
}

ETrigeType  getEventType(asio::error_code& ec)
{
	ETrigeType ret{ ETrigeType::TimeOut };
	if (ec.value() == asio::error::operation_aborted)
	{
		ret = SMHotupdate::ETrigeType::Cancel;
	}
	return ret;
	
}

}

