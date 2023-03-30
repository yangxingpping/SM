#pragma once
#include "networkinterfaceExport.h"
#include <string_view>
#include <stdint.h>
#include <span>

using std::string_view;
using std::span;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT StreamNetDealer
	{
	public:
		StreamNetDealer();
		int HeadLen();
		int Len();
		bool unpack(uint32_t& no, string_view src);
		void pack(uint32_t& no, size_t len, span<char> dst);
		uint32_t getNo();
	private:
		int _len;
		uint32_t _no;
	};
	

}
