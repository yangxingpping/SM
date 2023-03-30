#pragma once
#include "networkinterfaceExport.h"
#include <stdint.h>
#include <memory>
#include <string>
#include <string_view>

using std::shared_ptr;
using std::string_view;
using std::string;

namespace SMNetwork
{
	class NETWORKINTERFACE_EXPORT NMessage
	{
	public:
		NMessage(uint32_t no, shared_ptr<string> msg);
		NMessage(uint32_t no, string_view msg);
		NMessage(NMessage& copy);

		uint32_t Len();
		uint32_t No();
		shared_ptr<string> body();
		void body(shared_ptr<string> body);
	private:
		uint32_t _len{ 0 };
		uint32_t _no{ 0 };
		shared_ptr<string> _platBody{nullptr};
	};
}

