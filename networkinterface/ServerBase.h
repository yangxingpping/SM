#pragma once
#include "networkinterfaceExport.h"
#include "Utils.h"
#include "coros.h"
#include "uwebsockets/App.h"
#include "enums.h"


namespace SMNetwork
{
class NETWORKINTERFACE_EXPORT ServerBase
{
public:
    ServerBase()  {

    }
    ServerBase(const ServerBase& cons) = delete;
    ServerBase(const ServerBase&& cons) = delete;
    virtual void init() = 0;
};
}
