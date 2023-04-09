#pragma once
#include "networkinterfaceExport.h"
#include "PlatformPackInterface.h"

namespace SMNetwork
{

    class NETWORKINTERFACE_EXPORT NoMainAssPlatPack : public PlatformPackInterface
    {
    public:
        NoMainAssPlatPack(int mainc);
        NoMainAssPlatPack(NoMainAssPlatPack& ref);
        virtual ~NoMainAssPlatPack();
        virtual int getMain() override;
        bool pack(span<char> dst) override;
        bool unpack(string_view src) override;
        size_t len() override;
        virtual PlatformPackInterface* clone() override;
    private:
        uint32_t _main;
    };
}
