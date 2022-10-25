
#pragma once

#include "iguana/json.hpp"
#include "spdlog/spdlog.h"

#include <string>

using std::string;

#define BEGIN_STD try{
#define END_STD }\
	catch(const std::exception& e)\
	{\
		SPDLOG_WARN("return std exception {}", e.what());\
	}\
	catch(...)\
	{\
		SPDLOG_WARN("std return unexpected error ");\
	}

template<class T>
string my_to_string(T& t)
{
	string ret;
	iguana::string_stream ss;
	iguana::json::to_json(ss, t);
	ret = ss.str();
	return ret;
}

template<class T>
bool my_json_parse_from_string(T& ret, const string& str)
{
	bool bret = iguana::json::from_json0(ret, str.data());
	if (!bret)
	{
		SPDLOG_WARN("parse str [{}] failed", str);
	}
	return bret;
}

