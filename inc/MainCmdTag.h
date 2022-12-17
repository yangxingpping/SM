
#pragma once



#include "enums.h"

#include "asio/awaitable.hpp"

#include <string>
#include <string_view>
#include <stdint.h>
#include <map>

using std::string;
using std::string_view;
using asio::awaitable;
using std::map;

enum class ServeMode
{
    SBind,
    SConnect,
};


class MainCmdTag
{
    public:
    MainCmdTag()
    {}

    virtual void init(ServeMode mode) {};

    virtual awaitable<string> dealMsg(uint16_t ass, string_view msg)
    {
        assert(0);
        co_return "";
    }
    virtual awaitable<string> dealMsg(uint16_t ass, map<string, string>& query)
    {
        assert(0);
        co_return "";
    };
    virtual awaitable<string> dealMsg(map<string, string>& query)
    {
        assert(0);
        co_return "";
    };
    //virtual ServeMode getServeMode() = 0;
};
