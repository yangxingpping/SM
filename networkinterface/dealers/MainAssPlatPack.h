#pragma once
#include "networkinterfaceExport.h"
#include "PlatformPackInterface.h"



namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT MainAssPlatPack : public PlatformPackInterface
	{
	public:
		MainAssPlatPack(int mainc);
		MainAssPlatPack(MainAssPlatPack& ref);
		virtual ~MainAssPlatPack();
		virtual int getMain() override;
		void setAss(int ass) override;
		int getAss() override;
		bool pack(span<char> dst) override;
		bool unpack(string_view src) override;
		size_t len() override;
		size_t HeadLen() override;
		virtual PlatformPackInterface* clone() override;
	private:
		uint32_t _main;
		uint32_t _ass;
	};
}

