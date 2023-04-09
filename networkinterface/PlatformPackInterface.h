#pragma once
#include "networkinterfaceExport.h"
#include <string_view>
#include <span>
#include <assert.h>

using std::span;
using std::string_view;

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
		virtual size_t HeadLen()
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
		virtual ~PlatformPackInterface() {}
	};
}
