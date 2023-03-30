#pragma once

#include "asio/awaitable.hpp"
#include "enums.h"
#include <map>
#include <vector>
#include <span>
#include <concepts>
#include <memory>
#include <string>
#include <tuple>

using std::span;
using std::string;
using std::string_view;
using std::map;
using std::vector;
using std::same_as;
using std::shared_ptr;
using std::string;
using std::is_enum_v;
using std::tuple;

template <class T>
concept DBReq = requires(T t)
{
    {
        t.to_string()
    } ->same_as<string>;
};

template <class T>
concept DBRep = requires(T t, string& j)
{
    {
        t.parse(j)
    } -> same_as<bool>;
};

template <class T>
concept MessageDealer = requires(T t, uint16_t head, string_view str)
{
    {
        t.dealMsg(str, head)
    } ->same_as<asio::awaitable<string>>;
    {
        t.getMainCmd()
    } ->same_as<MainCmd>;
};

template <class T>
concept HttpGetDealer = requires(T t, string_view url, string_view querystr,std::vector<std::string> params, map<string, string>& respheads)
{
    {
        t.dealGet(url, querystr)
    } ->same_as<asio::awaitable<string>>;
};

template <class T>
concept EventTrigger = requires(T t)
{
    {
        t.trigger()
    }->same_as<void>;
    {
        t.trigger_all()
    }->same_as<void>;
};

template <class T, class DBReq, class DBRep>
concept DataBaseAccesser = requires(T t,DBReq req, DBRep& rep, AssDB op)
{  
    {
        t._execQuery(req, rep, op)
    }->same_as<asio::awaitable<bool>>;
};

template<typename T>
concept AppPackUnpack = requires(T t, string_view src, span<char> dst)
{
	{
		t.pack(src, dst)
	}->same_as<bool>;
	{
		t.unpack(src, dst)
	}->same_as<bool>;
};

template<typename T>
concept PlatformPackUnpack = requires(T t, string_view src, span<char> dst, MainCmd mainc, int assc)
{
    {
        t.getMain()
    }->same_as<MainCmd>;
    {
        t.setMain(mainc)
    }->same_as<void>;
    {
        t.getAss()
    }->same_as<int>;
    {
        t.setAss(assc)
    }->same_as<void>;
    {
        t.HeadLen()
    }->same_as<int>;
    {
        t.pack(dst)
    }->same_as<bool>;
    {
        t.unpack(src)
    }->same_as<bool>;
};

template<typename T>
concept NetworkPackUnpack = requires(T t, uint32_t& no, size_t len, string_view src, span<char> dst)
{
    {
        t.HeadLen()
    }->same_as<int>;
    {
        t.Len()
    }->same_as<int>;
    {
        t.unpack(no, src)
    }->same_as<bool>;
    {
        t.pack(no, len, dst)
    }->same_as<void>;
    {
        t.getNo()
    }->same_as<uint32_t>;
};

template<typename T>
concept PackUnpack = AppPackUnpack<T> && PlatformPackUnpack<T> && NetworkPackUnpack<T>;

template<typename T>
concept EnumConcept = is_enum_v<T>;


/** /************************************************************************/
/* concept for network channel                                                     
*/
/************************************************************************/ 
template<typename T>
concept NetInterface = requires(T imp, shared_ptr<string> msg, string_view src, uint32_t no)
{
    {
        imp.sendPack(src, no)
    }->same_as<asio::awaitable<bool>>;
    {
        imp.recvPack()
    }->same_as<asio::awaitable<tuple<uint32_t,shared_ptr<string>>>>;
    {
        imp.sockNo()
    }->same_as<uint32_t>;
    {
        imp.setSockNo(no)
    }->same_as<void>;
};
template<typename MessageType>
concept Messages = requires(MessageType imp, shared_ptr<string> src, uint32_t no)
{
	{
		imp.sendReq(src)
	}->same_as<int>;
	{
		imp.recvRep(no)
	}->same_as<shared_ptr<string>>;
};

template<typename T>
concept ChannelInterface = requires(T impl, shared_ptr<string> req)
{
    {
        impl.start()
    }->same_as<void>;
    {
        impl.reqrep(req)
    }->same_as < asio::awaitable<shared_ptr<string>>>;
    {
        impl.newReq()
    }->same_as<uint32_t>;
    {
        impl.MainCmd()
    }->same_as<int>;
};
