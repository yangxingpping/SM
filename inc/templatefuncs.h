
#pragma once

#include "iguana/json.hpp"
#include "spdlog/spdlog.h"
#include "boost/callable_traits/function_type.hpp"
#include "boost/callable_traits/return_type.hpp"
#include "boost/callable_traits/args.hpp"
#include "commUsing.h"

#include "../Utils/Utils.h"

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

//https://stackoverflow.com/questions/13101061/detect-if-a-type-is-a-stdtuple
template <typename> struct is_tuple : std::false_type {};
template <typename ...T> struct is_tuple<std::tuple<T...>> : std::true_type {};

//https://stackoverflow.com/questions/53394100/concatenating-tuples-as-types`
template <class, class>
struct tuple_cat_t;
template <class... First, class... Second>
struct tuple_cat_t<std::tuple<First...>, std::tuple<Second...>> {
	using type = std::tuple<First..., Second...>;
};


template <typename T, typename ...>
struct tuple_cat_t2
{
	using type = T;
};

template <typename ... Ts1, typename ... Ts2, typename ... Ts3>
struct tuple_cat_t2<std::tuple<Ts1...>, std::tuple<Ts2...>, Ts3...>
	: public tuple_cat_t2<std::tuple<Ts1..., Ts2...>, Ts3...>
{ };

template<typename Tuple>
constexpr auto tuple_pop_front(Tuple tuple) {
	static_assert(std::tuple_size<Tuple>::value > 0, "Cannot pop from an empty tuple");
	return std::apply(
		[](auto, auto... rest) { return std::make_tuple(rest...); },
		tuple);
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


/**
 * @brief remove tuple first element
 * 
 * @tparam T 
 * @param t 
 * @return tuple_cdr<typename std::remove_reference<T>::type>::type 
 */
template<typename T>
typename tuple_cdr<typename std::remove_reference<T>::type>::type
cdr(T&& t)
{
	return cdr_impl(std::forward<T>(t),
		std::make_index_sequence<std::tuple_size<T>::value>{});
}

/**
 * @brief unpack global function parameters 
 * 
 * @tparam Func 
 * @tparam std::enable_if<std::is_function<Func>::value, Func>::type 
 * @param msg 
 * @return tuple_cat_t<std::tuple<bool>,boost::callable_traits::args_t<Func>>::type 
 */
template<typename Func, typename std::enable_if<std::is_function<Func>::value, Func>::type* = nullptr>
auto unpackArgs(string_view msg) -> typename tuple_cat_t<std::tuple<bool>,boost::callable_traits::args_t<Func>>::type
{
	bool bret{ true };
	using Args = boost::callable_traits::args_t<Func>;
	Args args;
	int argIndex{ 0 };
	if constexpr(std::tuple_size_v <Args> == 1) //only 1 parameter
	{
		auto& req = std::get<0>(args);
		bret = my_json_parse_from_string(req, msg);
	}
	else
	{
		size_t offset{ 0 };
		SMUtils::for_each(args, [&](auto& arg) {
			auto strparam = SMUtils::unpackstring(string_view(msg.data() + offset, msg.length() - offset));
		bret = my_json_parse_from_string(arg, strparam);
		if (!bret)
		{
			return;
		}
		offset += (strparam.length() + sizeof(uint32_t));
		++argIndex;
			});
	}
	if (!bret)
	{
		SPDLOG_WARN("parse the {} param failed", argIndex);
	}
	return std::tuple_cat(std::make_tuple(bret), args);
	
}

/**
 * @brief unpack class member function parameters 
 * 
 * @tparam Func 
 * @tparam std::enable_if<std::is_member_function_pointer<Func>::value, Func>::type 
 * @param msg 
 * @return tuple_cat_t<std::tuple<bool>, decltype(cdr(std::declval<boost::callable_traits::args_t<Func>>()))>::type 
 */
template<typename Func, typename std::enable_if<std::is_member_function_pointer<Func>::value, Func>::type* = nullptr>
auto unpackArgs(string_view msg) -> typename tuple_cat_t<std::tuple<bool>, decltype(cdr(std::declval<boost::callable_traits::args_t<Func>>()))>::type
{
	bool bret{ true };
	using Args = decltype(cdr(std::declval<boost::callable_traits::args_t<Func>>()));
	Args args;
	int argIndex{ 0 };
	if constexpr (std::tuple_size_v <Args> == 1) //only 1 parameter
	{
		auto& req = std::get<0>(args);
		bret = my_json_parse_from_string(req, msg);
	}
	else
	{
		size_t offset{ 0 };
		SMUtils::for_each(args, [&](auto& arg) {
			auto strparam = SMUtils::unpackstring(string_view(msg.data() + offset, msg.length() - offset));
		bret = my_json_parse_from_string(arg, strparam);
		if(!bret)
		{
			return;
		}
		offset += (strparam.length() + sizeof(uint32_t));
		++argIndex;
			});
	}
	if (!bret)
	{
		SPDLOG_WARN("parse the {} param failed", argIndex);
	}
	return std::tuple_cat(std::make_tuple(bret), args);
}


template<typename Args, typename std::enable_if<is_tuple<Args>::value, Args>::type* = nullptr>
auto unpackArgs(string_view msg) -> typename tuple_cat_t<std::tuple<bool>, Args>::type
{
	bool bret{ true };
	Args args;
	int argIndex{ 0 };
	if constexpr (std::tuple_size_v <Args> == 1) //only 1 parameter
	{
		auto& req = std::get<0>(args);
		bret = my_json_parse_from_string(req, msg);
	}
	else
	{
		size_t offset{ 0 };
		SMUtils::for_each(args, [&](auto& arg) {
			auto strparam = SMUtils::unpackstring(string_view(msg.data() + offset, msg.length() - offset));
		bret = my_json_parse_from_string(arg, strparam);
		if(!bret)
		{
			return;
		}
		offset += (strparam.length() + sizeof(uint32_t));
		++argIndex;
			});
	}
	if (!bret)
	{
		SPDLOG_WARN("parse the {} param failed", argIndex);
	}
	return std::tuple_cat(std::make_tuple(bret), args);
}

template<typename Arg>
void packArgs(string& ret, Arg& arg)
{
	auto strarg = my_to_string(arg);
	if (ret.empty())
	{
		ret = *strarg;
	}
	else
	{
		ret += SMUtils::packstring(string_view(strarg->data(), strarg->length()));
	}
}

template<typename Arg, typename... Args>
void packArgs(string& ret, Arg& arg, Args... args)
{
	auto strarg = my_to_string(arg);
	ret += SMUtils::packstring(string_view(strarg->data(), strarg->length()));
	packArgs(ret, args...);
}


//https://stackoverflow.com/questions/12742877/remove-reference-from-stdtuple-members
template <typename... T>
using tuple_with_removed_refs = std::tuple<typename std::remove_reference<T>::type...>;

template <typename... T>
tuple_with_removed_refs<T...> remove_ref_from_tuple_members(std::tuple<T...> const& t) {
	return tuple_with_removed_refs{ t };
}
