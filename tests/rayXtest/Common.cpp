

#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "catch.hpp"
#include "jwt-cpp/jwt.h"
#include "Utils.h"
#include "endian/big_endian.hpp"
#include "endian/little_endian.hpp"
#include "endian/network.hpp"
#include "endian/stream_reader.hpp"
#include "endian/stream_writer.hpp"
#include "templatefuncs.h"
#include "spdlog/spdlog.h"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include <thread>


#include "Utils.h"

#include "ray/api.h"

#include "rayx.h"

int Plus(int a, int b)
{
	return a + b;
}
RAY_REMOTE(Plus);

int main(int argc, char* argv[]) 
{
	// your setup ...
	RAYX::Init();
	int result = Catch::Session().run(argc, argv);
	// your clean-up...
	RAYX::Shutdown();
	//https://github.com/gabime/spdlog/issues/1533
	spdlog::shutdown();
	return result;
}

TEST_CASE("int value", "put and get")
{
	/*auto object = ray::Put(int{100});
	auto put_get_result = *(ray::Get(object));*/
}

TEST_CASE("big endian", "endian convert")
{
    uint64_t a = 111, b = 222;
    uint64_t x, y;

    

    uint8_t buffer[sizeof(uint64_t) * 2];

    endian::network::put(a, buffer);
    endian::network::put(b, buffer + sizeof(a));

    endian::network::get(x, buffer);
    endian::network::get(y, buffer + sizeof(x));

	REQUIRE(a == x);
	REQUIRE(b == y);
}
