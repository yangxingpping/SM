
#include "MessageSplitFuncs.h"
#include "Utils.h"
#include <memory>
#include <string>
#include <string_view>
#include <assert.h>
using std::string;
using std::string_view;

thread_local string _packbuf;

namespace SMNetwork
{

const size_t MessageSplitFuncs::_fixMsgLen = 5;
const size_t MessageSplitFuncs::_fixHeadLen = 12;
const size_t MessageSplitFuncs::_fixSubHeadLen = 8;

bool MessageSplitFuncs::variableLenSplit(deque<uint8_t>& msg, cmdhead& head, string& dst)
{
    bool bret = false;
     dst.clear();
    if (!msg.empty())
    {
        head.mainc = MainCmd::Echo;
        dst.assign(msg.begin(), msg.begin() + msg.size());
        msg.clear();
        bret = true;
    }
    return bret;
}

bool MessageSplitFuncs::fixLenSplit(deque<uint8_t>& msg, cmdhead& head, string& dst)
{
    bool bret = false;
    dst.clear();
    if (msg.size() >= _fixMsgLen)
    {
        head.mainc = MainCmd::Echo;
        dst.assign(msg.begin(), msg.begin() + _fixMsgLen);
        msg.erase(msg.begin(), msg.begin() + _fixMsgLen);
        bret = true;
    }
    return bret;
}

bool MessageSplitFuncs::FixHeaderSplit(deque<uint8_t>& msg, cmdhead& head, string& dst)
{
    bool bret = false;
    if(head.mainc==MainCmd::MainCmdBegin) //need parse head first
    {
        if(msg.size() < _fixHeadLen)
        {
            return bret;
        }
        string strhead(msg.begin(), msg.begin() + _fixHeadLen);
        head = SMUtils::parsecmd(string_view{strhead.data(), strhead.length()});
        msg.erase(msg.begin(), msg.begin() + _fixHeadLen);
    }
    if (!msg.empty() && msg.size() >= head.len)
    {
        if (msg.size() >= head.len)
        {
            dst.clear();
            dst.assign(msg.begin(), msg.begin() + head.len);
            msg.erase(msg.begin(), msg.begin() + head.len);
            bret = true;
        }
    }
    return bret;
}

bool MessageSplitFuncs::FixHeaderSplit2(string_view msg, cmdhead& head, string& dst)
{
    bool bret = false;
    if (head.mainc == MainCmd::MainCmdBegin) // need parse head first
    {
        if (msg.size() < _fixHeadLen)
        {
            return bret;
        }
        string strhead(msg.begin(), msg.begin() + _fixHeadLen);
        head = SMUtils::parsecmd(string_view {strhead.data(), strhead.length()});
    }
    if (!msg.empty() && msg.size() >= _fixHeadLen)
    {
        if (msg.size() >= head.len)
        {
            dst.clear();
            dst.assign(msg.begin() + _fixHeadLen, msg.end());
            bret = true;
        }
    }
    return bret;
}

bool MessageSplitFuncs::FixHeaderSplit3(deque<uint8_t>& msg, cmdsubhead& head, string& dst)
{
    bool bret = false;
    {
        if(msg.size() < _fixSubHeadLen)
        {
            return bret;
        }
        string strhead(msg.begin(), msg.begin() + _fixSubHeadLen);
        head = SMUtils::parsesubcmd(string_view{strhead.data(), strhead.length()});
        msg.erase(msg.begin(), msg.begin() + _fixSubHeadLen);
    }
    if (!msg.empty() && msg.size() >= head.len)
    {
        if (msg.size() >= head.len)
        {
            dst.clear();
            dst.assign(msg.begin(), msg.begin() + head.len);
            msg.erase(msg.begin(), msg.begin() + head.len);
            bret = true;
        }
    }
    return bret;
}

bool MessageSplitFuncs::FixHeaderSplit4(string_view msg, cmdsubhead& head, string& dst)
{
    bool bret = false;
    {
        if (msg.size() < _fixSubHeadLen)
        {
            return bret;
        }
        string strhead(msg.begin(), msg.begin() + _fixSubHeadLen);
        head = SMUtils::parsesubcmd(string_view {strhead.data(), strhead.length()});
    }
    if (!msg.empty() && msg.size() >= _fixSubHeadLen)
    {
        if (msg.size() >= head.len)
        {
            dst.clear();
            dst.assign(msg.begin() + _fixSubHeadLen, msg.end());
            bret = true;
        }
    }
    return bret;
}


shared_ptr<string> MessagePackFuncs::PackMainAssPack(cmdhead head, string_view msg)
{
    shared_ptr<string> _packbuf = std::make_shared<string>();
    auto headfix = SMUtils::packcmdrep(head.mainc, head.assc, msg.length());
    _packbuf->insert(_packbuf->end(), headfix.begin(), headfix.end());
    _packbuf->insert(_packbuf->end(), msg.begin(), msg.end());
    return _packbuf;
}
shared_ptr<string> MessagePackFuncs::PackFixLenPack(cmdhead head, string_view msg)
{
    (void)(head);
    shared_ptr<string> _packbuf = std::make_shared<string>();
    _packbuf->insert(_packbuf->end(), msg.begin(), msg.end());
    return _packbuf;
}

}
