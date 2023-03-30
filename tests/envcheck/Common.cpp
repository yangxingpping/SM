

#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "catch.hpp"
#include "jwt-cpp/jwt.h"
#include "HttpCmdTag.h"
#include "Utils.h"
#include "networkinterface.h"
#include "Routers.h"
#include "NMessage.h"
#include "HttpCmdTag.h"
#include "asio/experimental/concurrent_channel.hpp"
#include "templatefuncs.h"
#include "spdlog/spdlog.h"
#include "oneshot.hpp"
#include "templatefuncs.h"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include <thread>
#include <map>
#include <optional>
#include <tuple>

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
	int add(Arg1 a, Arg2 b) { return a._a + b._b; }
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

int add2_ref(Arg1& a, Arg2& b)
{
	return a._a + b._b;
}

int increase(Arg1 a)
{
	return ++a._a;
}

optional<tuple<int, bool>> tuple_option_func(bool bflag)
{
	auto v = make_tuple(1, bflag);
	return bflag ? (optional<tuple<int, bool>>(make_tuple(1, bflag))) : nullopt;
}

class enable_shared : public std::enable_shared_from_this<enable_shared>
{
public:

};

TEST_CASE("oneshot", "use in map")
{
	map<int, map<int,oneshot::sender<void>>> signals;
	auto [s, r] = oneshot::create<void>();
	auto& vv = signals[0];
	vv.insert({ 1,move(s) });
}

TEST_CASE("oneshot with NMessage", "use in map")
{
	map<uint32_t, map<uint32_t, oneshot::sender<shared_ptr<SMNetwork::NMessage>>>> signals;
	auto [s, r] = oneshot::create<shared_ptr<SMNetwork::NMessage>>();
	auto& vv = signals[0];
	vv.insert({ 1,move(s) });
}

TEST_CASE("asio concurrent channel loop", "concurrent channel")
{
	using namespace asio::experimental;
	concurrent_channel<void(asio::error_code, uint32_t, bool)> ch1(*IOCTX, 10);
	asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
		for (uint32_t i = 0; i < 64; ++i)
		{
			co_await ch1.async_send(asio::error_code{}, i, i % 2 == 0 ? true : false, asio::use_awaitable);
			auto [f, s] = co_await ch1.async_receive(asio::use_awaitable);
			assert(f == i);
			assert(s == i % 2 == 0 ? true : false);
		}
		co_return;
		}, asio::detached);
	IOCTX->run();

}

TEST_CASE("optional", "optional with bind")
{
	if (auto v = tuple_option_func(true))
	{
		auto [f, s] = *v;
		REQUIRE(s);
	}
	else
	{
		REQUIRE(false);
	}
}

TEST_CASE("reserve", "for string")
{
	size_t newCap{ 555 };
	string s;
	auto ls = s.capacity();
	s.reserve(newCap);
	REQUIRE(s.capacity() >= newCap);
}

TEST_CASE("atomic size check")
{
	REQUIRE(sizeof(size_t) == 8);
	REQUIRE(sizeof(int) == 4);
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
	REQUIRE(!std::is_member_function_pointer<decltype(add2)>::value);
	REQUIRE(std::is_function_v<decltype(add2)>);
}

TEST_CASE("assert function return type")
{
	using Add2ReturnType = boost::callable_traits::return_type_t<decltype(add2)>;
	REQUIRE(std::is_same_v<Add2ReturnType, int>);
}

TEST_CASE("enable_shared_from_this")
{

}

TEST_CASE("operate tuple")
{
	using TA = std::tuple<int, float&>;
	REQUIRE(std::is_same_v<std::tuple_element_t<0, TA>, int>);
	REQUIRE(std::is_same_v<std::tuple_element_t<1, TA>, float&>);
	REQUIRE(std::tuple_size<TA>::value == 2);
}

TEST_CASE("tuple type check")
{
	using TA = std::tuple<int, float>;
	REQUIRE(is_tuple<TA>::value);
	REQUIRE(!is_tuple<int>::value);
	REQUIRE(!is_tuple<Arg1>::value);
}

TEST_CASE("pack and unpack tuple 1 element", "only 1 element")
{
	Arg1 arg1{ 10 };
	string str1;
	packArgs(str1, arg1);
	Arg1 arg2{ 20 };
	auto unpacked = unpackArgs<std::tuple<Arg1>>(std::string_view(str1.data(), str1.length()));
	auto [bflag, arg] = unpacked;
	REQUIRE(bflag);
	REQUIRE(arg._a == arg1._a);
}

TEST_CASE("pack and unpack tuple 2 element", "with 2 element")
{
	Arg1 arg1{ 10 };
	Arg2 arg2{ 20 };
	string str1;
	packArgs(str1, arg1, arg2);
	using TA = std::tuple<Arg1, Arg2>;
	auto unpacked = unpackArgs<TA>(std::string_view(str1.data(), str1.length()));
	auto [bflag, a1, a2] = unpacked;
	REQUIRE(bflag);
	REQUIRE(a1._a == arg1._a);
	REQUIRE(a2._b == arg2._b);
}

TEST_CASE("global function with 2 param", "pack and unpack args")
{

	Arg1 arg1{ 10 };
	Arg2 arg2{ 20 };
	string ret;
	packArgs(ret, arg1, arg2);
	auto ret2 = unpackArgs<decltype(add2)>(string_view(ret.data(), ret.length()));
	REQUIRE(std::tuple_size<decltype(ret2)>() == 3);
	auto o1 = std::get<0>(ret2);
	auto tparams = tuple_pop_front(ret2);
	REQUIRE(o1 == true);
	auto [o2, o3] = tparams;
	REQUIRE(o2._a == arg1._a);
	REQUIRE(o3._b == arg2._b);
}

TEST_CASE("global function with 1 param", "pack and unpack args")
{
	Arg1 arg1{ 10 };
	string ret;
	packArgs(ret, arg1);
	auto ret2 = unpackArgs<decltype(increase)>(string_view(ret.data(), ret.length()));
	REQUIRE(std::tuple_size<decltype(ret2)>() == 2);
	auto o1 = std::get<0>(ret2);
	auto tparams = tuple_pop_front(ret2);
	REQUIRE(o1 == true);
	auto [o2] = tparams;
	REQUIRE(o2._a == arg1._a);
}


TEST_CASE("parameter by value", "std apply")
{
	Arg1 a{ 1 };
	Arg2 b{ 2 };
	auto args = std::make_tuple(a, b);
	auto ret = std::apply(add2, args);
	REQUIRE(ret == (a._a + b._b));
}

TEST_CASE("parameter by ref", "std apply")
{
	Arg1 a{ 1 };
	Arg2 b{ 2 };
	auto args = std::make_tuple(a, b);
	auto ret = std::apply(add2_ref, args);
	REQUIRE(ret == (a._a + b._b));
}

TEST_CASE("member function with 2 param", "pack and unpack args")
{

	Arg1 arg1{ 10 };
	Arg2 arg2{ 20 };
	string ret;
	packArgs(ret, arg1, arg2);
	auto ret2 = unpackArgs<decltype(&demo1::add)>(string_view(ret.data(), ret.length()));
	REQUIRE(std::tuple_size<decltype(ret2)>() == 3);
	auto o1 = std::get<0>(ret2);
	auto tparams = tuple_pop_front(ret2);
	REQUIRE(o1 == true);
	auto [o2, o3] = tparams;
	REQUIRE(o2._a == arg1._a);
	REQUIRE(o3._b == arg2._b);
}

TEST_CASE("tuple operate", "tuple cat")
{
	using S1 = tuple_cat_t<std::tuple<bool>, boost::callable_traits::args_t<decltype(&add2)>>::type;
	using S2 = tuple<bool, Arg1, Arg2>;
	REQUIRE(std::is_same<S1, S2>::value);
}

TEST_CASE("htrie_map value list", "push_back success")
{
	tsl::htrie_map<char, std::list<int>> v;
	v["a"].push_back(1);
	v["a"].push_back(2);
	auto x = v.find("a");
	REQUIRE(x.value().size() == 2);
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

