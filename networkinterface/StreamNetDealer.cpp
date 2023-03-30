
#include "StreamNetDealer.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "hotupdate.h"
#include "Routers.h"
#include <string>
#include <cassert>

using std::string;

namespace SMNetwork
{
	StreamNetDealer::StreamNetDealer()
	{

	}

	int StreamNetDealer::HeadLen()
	{
		return sizeof(_len) + sizeof(_no);
	}

	int StreamNetDealer::Len()
	{
		return _len;
	}

	bool StreamNetDealer::unpack(uint32_t& no, string_view src)
	{
		bool bret{ false };
		uint32_t temp{ 0 };
		auto [bbret, tail] = SMUtils::unpackuint32(src, temp);
		if (bbret)
		{
			_len = (int)(temp);
			auto [bbret, _] = SMUtils::unpackuint32(tail, temp);
			if (bbret)
			{
				_no = temp;
				bret = true;
			}
		}
		else
		{

		}
		return bret;
	}

	void StreamNetDealer::pack(uint32_t& no, size_t len, span<char> dst)
	{
		_len = len;
		_no = no;
		SMUtils::packuint32(span<char>(dst.data(), sizeof(_len)), _len);
		SMUtils::packuint32(span<char>(dst.data() + sizeof(_len), sizeof(_no)), _no);
	}

	uint32_t StreamNetDealer::getNo()
	{
		return _no;
	}
	
}
