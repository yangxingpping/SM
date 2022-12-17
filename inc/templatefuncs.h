
#pragma once

#include "iguana/json.hpp"
#include "spdlog/spdlog.h"

#include "commUsing.h"

#include <string>
#include <string_view>

using std::string;
using std::string_view;

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
RouterFuncReturnType my_to_string(T& t)
{
	RouterFuncReturnType ret;
	iguana::string_stream ss;
	iguana::json::to_json(ss, t);
	ret = std::make_shared<std::string>(ss.str());
	return ret;
}

template<class T>
bool my_json_parse_from_string(T& ret, const string& str)
{
	bool bret = iguana::json::from_json0(ret, str.data(), str.length());
	if (!bret)
	{
		SPDLOG_WARN("parse str [{}] failed", str);
	}
	return bret;
}

template<class T>
bool my_json_parse_from_string(T& ret, string_view str)
{
	bool bret = iguana::json::from_json0(ret, str.data(), str.length());
	if (!bret)
	{
		SPDLOG_WARN("parse str [{}] failed", str);
	}
	return bret;
}

template<typename T, typename Seq>
struct tuple_cdr_impl;

template<typename T, std::size_t I0, std::size_t... I>
struct tuple_cdr_impl<T, std::index_sequence<I0, I...>>
{
	using type = std::tuple<typename std::tuple_element<I, T>::type...>;
};

template<typename T>
struct tuple_cdr
	: tuple_cdr_impl<T, std::make_index_sequence<std::tuple_size<T>::value>>
{ };

template<typename T, std::size_t I0, std::size_t... I>
typename tuple_cdr<typename std::remove_reference<T>::type>::type
cdr_impl(T&& t, std::index_sequence<I0, I...>)
{
	return std::make_tuple(std::get<I>(t)...);
}

template<typename T>
typename tuple_cdr<typename std::remove_reference<T>::type>::type
cdr(T&& t)
{
	return cdr_impl(std::forward<T>(t),
		std::make_index_sequence<std::tuple_size<T>::value>{});
}
