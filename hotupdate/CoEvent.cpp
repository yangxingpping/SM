
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

}

