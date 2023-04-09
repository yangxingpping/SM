

#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "catch.hpp"
#include "jwt-cpp/jwt.h"
#include "HttpCmdTag.h"
#include "Utils.h"
#include "templatefuncs.h"
#include "spdlog/spdlog.h"
#include "templatefuncs.h"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include <thread>

using namespace std;

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
