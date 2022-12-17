

#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "catch.hpp"
#include "jwt-cpp/jwt.h"
#include "Utils.h"
#include "spdlog/spdlog.h"
#include "templatefuncs.h"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include <thread>

using namespace std;

#include "Utils.h"

struct Arg1
{
	int _a;
};
REFLECTION(Arg1, _a);
struct Arg2
{
	int _b;
};
REFLECTION(Arg2, _b);

template<typename F>
void func(F func)
{
	using RetType = boost::callable_traits::return_type_t<F>;
	using Args = boost::callable_traits::args_t<F>;
	RetType ret;
	Args args;
	auto x = [](string msg)
	{
		size_t offset{ 0 };
		auto lenx = std::tuple_size<Args>::value;
		/*for (int i = 0; i < lenx; ++i)
		{
			uint32_t paramlen = unpackuint32(string_view(msg.data + offset, msg.length() - offset));
			
		}*/
	};
	Arg1 a{ 1 };
	Arg2 b{ 2 };
	string params = SMUtils::packstring(string_view(*my_to_string(a)));
	params += SMUtils::packstring(string_view(*my_to_string(b)));

	int i = 0;
}

int add(Arg1 a, Arg2 b)
{
	return a._a + b._b;
}

TEST_CASE("test parse and unparse message", "Utils")
{
	using namespace SMUtils;
	
}

TEST_CASE("test tuple unpack", "Utils")
{
	using namespace SMUtils;
	Arg1 a{ 1 };
	Arg2 b{ 2 };
	auto pp = my_to_string(a);
	string params = packstring(*pp);
	pp = my_to_string(b);
	params += packstring(*pp);
	a._a = 10;
	b._b = 20;
	size_t offset{ 0 };
	auto tt = std::make_tuple(a, b);
	SMUtils::for_each(tt, [&](auto& item) {
		auto strparam = unpackstring(string_view(params.data() + offset, params.length() - offset));
	if (!my_json_parse_from_string(item, strparam))
	{
		return;
		}
	offset += (strparam.length() + sizeof(uint32_t));
		
		});

	int i = 1;
}


TEST_CASE("local byte order to network order", "Utils")
{
	using namespace SMUtils;
	
	{
		uint16_t src = 0x0102;
		string dst;
		SMUtils::packuint16(dst, src);
		REQUIRE(dst.length() == 2);
		REQUIRE(dst[0] == 0x01);
		REQUIRE(dst[1] == 0x02);
	}
	{
		uint32_t src = 0x01020304;
		string dst;
		SMUtils::packuint32(dst, src);
		REQUIRE(dst.length() == 4);
		 REQUIRE(dst[0] == 0x01);
		 REQUIRE(dst[1] == 0x02);
		 REQUIRE(dst[2] == 0x03);
		 REQUIRE(dst[3] == 0x04);
	}
}

TEST_CASE("check if with nullptr, NULL", "if")
{
	shared_ptr<int> a{nullptr};
	int* b{nullptr};
	int* c{ NULL };
	unique_ptr<int> d{ nullptr };
	if (a)
	{
		REQUIRE(false);
	}
	if (b)
	{
		REQUIRE(false);
	}
	
	if (c)
	{
		REQUIRE(false);
	}
	if (d)
	{
		REQUIRE(false);
	}
}

TEST_CASE("network order to local byte order", "Utils")
{
	using namespace SMUtils;

	{
		uint16_t src = 0x0102;
		uint16_t src2 = 0;
		string dst{0x01,0x02};
		SMUtils::unpackuint16(dst, src2);
		REQUIRE(src2 == src);
	}
	{
		uint32_t src = 0x01020304;
		uint32_t src2 = 0;
		string dst{ 0x01,0x02,0x03,0x04 };
		SMUtils::unpackuint32(dst, src2);
		REQUIRE(src2 == src);
	}
	{
		uint32_t src = 0x01020304;
		uint32_t src2 = 0;
		string dst{ 0x01,0x02,0x03,0x04,0x05,0x06 };
		SMUtils::unpackuint32(dst, src2);
		REQUIRE(src2 == src);
	}
	{
		uint32_t src = 0x01020304;
		uint32_t src2 = 0;
		string dst{ 0x01,0x02,0x03 };
		REQUIRE_FALSE(SMUtils::unpackuint32(dst, src2));
		REQUIRE(src2 != src);
	}
}

TEST_CASE("convert from char* to string_view", "string_view")
{
	{
		char msg[] = "hello.world";
		string_view sv{msg};
		REQUIRE(strlen(msg)==sv.length());
	}

	{
		char msg[] = "h\0w";
		string strmsg{msg, 3};
		REQUIRE(strmsg.length()==3);
		REQUIRE(strmsg[1] == '\0');
	}
}

