#pragma once
#include "networkinterfaceExport.h"
#include <string_view>
#include <stdint.h>
#include <span>

using std::string_view;
using std::span;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT MainPlatformDealer
	{
	public:
		MainPlatformDealer();
		MainPlatformDealer(uint32_t mainc);
		int HeadLen();
		void setMain(uint32_t mainc);
		uint32_t getMain();
		void setAss(int ass);
		bool unpack(uint32_t& no, string_view src);
		void pack(uint32_t& no, size_t len, span<char> dst);
		int getAss();
		uint32_t getNo();
	private:
		int _len;
		uint32_t _no;
		uint32_t _main;
	};
	

}
