#pragma once

#include "asio/awaitable.hpp"
#include "enums.h"
#include <map>
#include <vector>

using std::string;
using std::string_view;
using std::map;
using std::vector;

template <class T>
concept DBReq = requires(T t)
{
    {
        t.to_string()
    } -> std::same_as<string>;
};

template <class T>
concept DBRep = requires(T t, string& j)
{
    {
        t.parse(j)
    } -> std::same_as<bool>;
};

template <class T>
concept MessageDealer = requires(T t, uint16_t head, string_view str)
{
    {
        t.dealMsg(str, head)
    } -> std::same_as<asio::awaitable<string>>;
    {
        t.getMainCmd()
    } -> std::same_as<MainCmd>;
};

template <class T>
concept HttpGetDealer = requires(T t, string_view url, string_view querystr,std::vector<std::string> params, map<string, string>& respheads)
{
    {
        t.dealGet(url, querystr)
    } ->std::same_as<asio::awaitable<string>>;
};

template <class T>
concept EventTrigger = requires(T t)
{
    {
        t.trigger()
    }->std::same_as<void>;
    {
        t.trigger_all()
    }->std::same_as<void>;
};

template <class T, class DBReq, class DBRep>
concept DataBaseAccesser = requires(T t,DBReq req, DBRep& rep, AssDB op)
{
    {
        t._execQuery(req, rep, op)
    }->std::same_as<asio::awaitable<bool>>;
};
