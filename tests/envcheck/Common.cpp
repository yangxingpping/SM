

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

class demo1
{
public:
	int add(Arg1 a, Arg1 b) { return a._a + b._a; }
};

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
	};
	Arg1 a{ 1 };
	Arg2 b{ 2 };
	string params = SMUtils::packstring(string_view(*my_to_string(a)));
	params += SMUtils::packstring(string_view(*my_to_string(b)));

	int i = 0;
}

int add2(Arg1 a, Arg2 b)
{
	return a._a + b._b;
}

TEST_CASE("test parse and unparse message", "Utils")
{
	using namespace SMUtils;
	
}

TEST_CASE("is member pointer", "class member function")
{
	REQUIRE(std::is_member_function_pointer<decltype(& demo1::add)>::value);
}

TEST_CASE("normal function", "is member pointer")
{
	REQUIRE(!std::is_member_function_pointer<decltype(&add2)>::value);
}

TEST_CASE("pack and unpack args", "global function with 2 param")
{

	Arg1 arg1{ 1 };
	Arg2 arg2{ 2 };
	string ret;
	packArgs(ret, arg1, arg2);
	auto ret2 = unpackArgs<decltype(add2)>(string_view(ret.data(), ret.length()));
	auto [o1, o2, o3] = ret2;
	REQUIRE(o1 == true);
	REQUIRE(o2._a == arg1._a);
	REQUIRE(o3._b == arg2._b);
}

TEST_CASE("tuple operate", "tuple cat")
{
	using S1 = tuple_cat_t<std::tuple<bool>, boost::callable_traits::args_t<decltype(&add2)>>::type;
	using S2 = tuple<bool, Arg1, Arg2>;
	REQUIRE(std::is_same<S1, S2>::value);
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

