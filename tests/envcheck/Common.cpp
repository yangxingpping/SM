

#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "catch.hpp"
#include "jwt-cpp/jwt.h"
#include <thread>

using namespace std;

#include "Utils.h"





TEST_CASE("jwt 1", "encode and decode right")
{
	auto ver = jwt::verify().allow_algorithm(jwt::algorithm::hs256{"key"}).with_issuer("alqaz");
	auto token = jwt::create();
	auto strtoken = token.set_issuer("alqaz").set_type("JWS").set_payload_claim("sample", jwt::claim(std::string("test")))
		.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(3600))
		.sign(jwt::algorithm::hs256{ "key" });

	auto decoded = jwt::decode(strtoken);
	auto payloads = decoded.get_payload();
	std::error_code ec;
	ver.verify(decoded, ec);
	if (ec.value() != 0)
	{
		SPDLOG_ERROR("verify error code {}, error msg {}", ec.value(), ec.message());
	}
	REQUIRE(ec.value()==0);
}

TEST_CASE("jwt 2", "encode and decode with wrong key")
{
	auto ver = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ "key" }).with_issuer("alqaz");
	auto token = jwt::create();
	auto strtoken = token.set_issuer("alqaz").set_type("JWS").set_payload_claim("sample", jwt::claim(std::string("test")))
		.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(3600))
		.sign(jwt::algorithm::hs256{ "keyx" });

	auto decoded = jwt::decode(strtoken);
	auto payloads = decoded.get_payload();
	std::error_code ec;
	ver.verify(decoded, ec);
	REQUIRE_FALSE(ec.value() == 0);
}

TEST_CASE("jwt 3", "encode and decode with timeout")
{
	auto ver = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ "key" }).with_issuer("alqaz");
	auto token = jwt::create();
	auto strtoken = token.set_issuer("alqaz").set_type("JWS").set_payload_claim("sample", jwt::claim(std::string("test")))
		.set_expires_at(std::chrono::system_clock::now() + std::chrono::microseconds(1))
		.sign(jwt::algorithm::hs256{ "key" });
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	auto decoded = jwt::decode(strtoken);
	auto payloads = decoded.get_payload();
	std::error_code ec;
	ver.verify(decoded, ec);
	REQUIRE_FALSE(ec.value() == 0);
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

