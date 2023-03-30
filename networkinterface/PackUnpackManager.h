#pragma once
#include "networkinterfaceExport.h"
#include "myconcept.h"

#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include <span>
#include <string_view>
#include <assert.h>
#include <stdint.h>
#include <map>
#include <memory>

using std::span;
using std::string_view;
using std::map;
using std::shared_ptr;

namespace SMNetwork
{
	class PlatformPackInterface
	{
	public:
		virtual PlatformPackInterface* clone()
		{
			assert(0);
			return nullptr;
		}
		virtual int getMain() 
		{
			assert(0);
			return 0;
		};
		virtual int getAss()
		{
			assert(0);
			return 0;
		};
		virtual void setAss(int ass)
		{
			(void)(ass);
			assert(0);
		};
		virtual bool pack(span<char> dst)
		{
			(void)(dst);
			assert(0);
			return false;
		};
		virtual size_t len()
		{
			assert(0);
			return 0;
		}
		virtual bool unpack(string_view src)
		{
			(void)(src);
			assert(0);
			return false;
		};
		virtual ~PlatformPackInterface(){}
	};

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
		PlatformPackInterface* clone() override;
	private:
		uint32_t _main;
		uint32_t _ass;
	};

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
		PlatformPackInterface* clone() override;
	private:
		uint32_t _main;
	};

	class NETWORKINTERFACE_EXPORT PackUnpackManager
	{
	public:
		static bool sInit();
		static PackUnpackManager* sInst();
		static bool sUninit();
	public:
		bool addPlatformPack(int mainc, shared_ptr<PlatformPackInterface> pack);
		shared_ptr<PlatformPackInterface> clone(int mainc);
	private:
		map<int, shared_ptr<PlatformPackInterface>> _platPacks;
	};

	template<EnumConcept MainType>
	bool addPlatformPack(shared_ptr<PlatformPackInterface> pack)
	{
		bool bret{ false };
		constexpr auto mains = magic_enum::enum_entries<MainType>();
		for (auto& entry : mains)
		{
			bret = PackUnpackManager::sInst()->addPlatformPack(magic_enum::enum_integer(entry.first), pack);
			if (!bret)
			{
				SPDLOG_WARN("add platform pack and unpack failed for main cmd {}", magic_enum::enum_name(entry.first));
				break;
			}
		}
		return bret;
	}

	template<EnumConcept MainType>
	shared_ptr<PlatformPackInterface> clonePlatformPack(MainType mainc)
	{
		return PackUnpackManager::sInst()->clone(magic_enum::enum_integer(mainc));
	}

	shared_ptr<PlatformPackInterface> NETWORKINTERFACE_EXPORT clonePlatformPack(int mainc);
}

#define PUM (SMNetwork::PackUnpackManager::sInst())

