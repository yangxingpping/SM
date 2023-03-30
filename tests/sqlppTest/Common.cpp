
#include "spdlog/spdlog.h"
#include "catch.hpp"
#include "../conf/dbTest.h"
#include "Configs.h"
#include "DBManager.h"
#include "database.h"
#include "conf.h"

using Catch::Matchers::Equals;

using namespace std;

TEST_CASE("select unconditionally", "success")
{
	SMDB::DBInst db;
	dbTest::Forsumgroup tb;
	tb.price = 0;
	
	auto& vx = *db;
	bool bret = false;
	for (const auto& row : vx(select(tb.name).from(tb).unconditionally()))
	{
		bret = true;
	}
	REQUIRE(bret);
}

TEST_CASE("select sum group by", "success")
{
	SMDB::DBInst db;
	dbTest::Forsumgroup tb;
	tb.price = 0;

	for (const auto& r : (*db)(select(sum(tb.price)).from(tb).group_by(tb.name).unconditionally()))
	{
		size_t v = r.sum.value();
		REQUIRE(v > 0);
	}
}

TEST_CASE("select sum, count group by", "success")
{
	SMDB::DBInst db;
	dbTest::Forsumgroup tb;
	tb.price = 0;

	for (const auto& r : (*db)(select(sum(tb.price), count(tb.name)).from(tb).group_by(tb.name).unconditionally()))
	{
		size_t v = r.sum.value();
		REQUIRE(v > 0);
		size_t c = r.count.value();
		REQUIRE(c > 0);
	}
}

TEST_CASE("select count", "success")
{
	SMDB::DBInst db;
	dbTest::Forsumgroup tb;
	tb.price = 0;

	auto vv = (*db)(select(count(tb.name)).from(tb).unconditionally()).front().count.value();
	REQUIRE(vv > 0);
}

int main(int argc, char* argv[]) 
{
	//setup ...
	auto dbconf = SMCONF::getDBConfig();
	dbconf->_sqlite._file = "dbTest.db";
	dbconf->_sqlite._filepath = "./conf";
	spdlog::set_level(spdlog::level::level_enum::warn);
	SMDB::init(true);
	int result = Catch::Session().run(argc, argv);
	//clean-up...
	
	return result;
}

