#pragma once
#include "networkinterfaceExport.h"
#include "enums.h"
#include <string_view>
#include <span>

using std::string_view;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT MainAssPlatformDealer
	{
	public:
		MainAssPlatformDealer();
		MainAssPlatformDealer(MainCmd mainc);
		MainCmd getMain();
		void setMain(MainCmd mainc);
		int getAss();
		void setAss(int ass);
		bool pack(std::span<char> dst);
		bool unpack(string_view src);
		int HeadLen();
	private:
		MainCmd _main;
		int _ass;
	};
	
}
