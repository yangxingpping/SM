#pragma once

#include <memory>

#include "TcpStreamHandler.h"

namespace SMNetwork
{
    class ManageTcpChannel
    {
    public:
		virtual void addChannel(std::shared_ptr<TcpStreamHandler> handle) = 0;
        virtual void removeChannel(std::shared_ptr<TcpStreamHandler> handle) = 0;
    };
}
