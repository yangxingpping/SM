#pragma once
#include "networkinterfaceExport.h"
#include "enums.h"
#include <memory>
#include <string>
#include <string_view>
#include <deque>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif


#include <cassert>

using std::string;
using std::string_view;
using std::deque;
using std::shared_ptr;

namespace SMNetwork
{
class NETWORKINTERFACE_EXPORT MessageSplitFuncs
{
  public:
    const static size_t _fixMsgLen;
    const static size_t _fixHeadLen;
    const static size_t _fixSubHeadLen;
  public:
    static bool fixLenSplit(deque<uint8_t>& msg, cmdhead& head, string& dst);
    static bool variableLenSplit(deque<uint8_t>& msg, cmdhead& head, string& dst);
    static bool FixHeaderSplit(deque<uint8_t>& msg, cmdhead& head, string& dst);
    static bool FixHeaderSplit2(string_view msg, cmdhead& head, string& dst);

    static bool FixHeaderSplit3(deque<uint8_t>& msg, cmdsubhead& head, string& dst);
    static bool FixHeaderSplit4(string_view msg, cmdsubhead& head, string& dst);
};

class NETWORKINTERFACE_EXPORT MessagePackFuncs
{
public:
  static shared_ptr<string> PackMainAssPack(cmdhead head, string_view msg);
  static shared_ptr<string> PackFixLenPack(cmdhead head, string_view msg);
};

typedef decltype(&MessageSplitFuncs::variableLenSplit) split_func_type;

typedef decltype(&MessagePackFuncs::PackMainAssPack) pack_func_type;

template<NetHeadType headtype, size_t fixLen>
class NetEncodeDecodeImpl
{
public:
    size_t dencode(std::string& req, std::string& rep)
    {
        size_t ret = 0;
        switch (headtype)
        {
        case NetHeadType::FixPackLenPlaceHolder:
        {
            if (req.length() > fixLen)
            {
                size_t t = ntohl(*(u_long*)(req.data()));
                if (req.length() >= (fixLen + t))
                {
                    rep = req.data() + fixLen;
                    ret = t;
                    req.erase(req.begin(), req.begin() + fixLen);
                }
            }
        }break;
        case NetHeadType::NoPlaceHolderFixLen:
        {
            if (req.length() >= fixLen)
            {
                rep.assign(req.begin(), req.begin() + fixLen);
                ret = fixLen;
                req.erase(req.begin(), req.begin() + fixLen);
            }
        }break;
        case NetHeadType::NoPlaceHolderVarLen:
        {
            ret = req.length();
            rep = req;
            req.clear();
        }break;
        default:
        {
            assert(0);
        }break;
        }
        return ret;
    }

    void encode(std::deque<uint8_t>& req, size_t protocolMsgLen)
    {
        switch (headtype)
        {
        case NetHeadType::FixPackLenPlaceHolder:
        {

        }break;
		case NetHeadType::NoPlaceHolderFixLen: //just check req length = fixLen
		{
            assert(req.size() == fixLen);
		}break;
        case NetHeadType::NoPlaceHolderVarLen: //do nothing
        {
        }break;
        default:
        {
            assert(0);
        }break;
        }
    }
};

template<PackType pt>
class PackEncodeDecodeImpl
{
public:
	bool decode(std::string& req, cmdhead& head, std::string& rep)
	{
        bool ret = true;
        switch (pt)
        {
        case PackType::NoHead:
        {
            rep = req;
            req.clear();
        }break;
        case PackType::FixMainSubHead:
        {
            head.mainc = static_cast<MainCmd>((ntohl(*(u_long*)(req.data()))));
            req.erase(req.begin(), req.begin() + sizeof(u_long));
            head.assc = ntohs(*(u_short*)(req.data()));
            req.erase(req.begin(), req.begin() + sizeof(u_short));
            head.len = static_cast<uint32_t>((ntohl(*(u_long*)(req.data()))));
            req.erase(req.begin(), req.begin() + sizeof(u_long));
            rep = req;
            req.clear();
        }break;
        case PackType::FixMainHead:
        {
			head.mainc = static_cast<MainCmd>((ntohl(*(u_long*)(req.data()))));
			req.erase(req.begin(), req.begin() + sizeof(u_long));
			head.len = static_cast<uint32_t>((ntohl(*(u_long*)(req.data()))));
			req.erase(req.begin(), req.begin() + sizeof(u_long));
			rep = req;
			req.clear();
        }break;
        case PackType::FixSubHead:
        {
			head.assc = ntohs(*(u_short*)(req.data()));
			req.erase(req.begin(), req.begin() + sizeof(u_short));
			head.len = static_cast<uint32_t>((ntohl(*(u_long*)(req.data()))));
			req.erase(req.begin(), req.begin() + sizeof(u_long));
			rep = req;
			req.clear();
        }break;
        case PackType::Max:
        {
            ret = false;
            assert(0);
        }break;
        default:
        {
            ret = false;
            assert(0);
        }break;
        assert(req.empty());
        }
		return ret;
	}
};

}
