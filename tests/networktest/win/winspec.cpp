
#include "catch.hpp"

#include <windows.h>
#include <winsock.h>

TEST_CASE("type check", "32 bit")
{
	REQUIRE(sizeof(u_long) == 4);
}
