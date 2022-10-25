#pragma once
#include "UtilsExport.h"
#include "enums.h"

#include "Structs.h"

#include <string_view>
#include <stdint.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <tuple>

using std::vector;
using std::string;
using std::map;
using std::string_view;
using std::deque;



enum class ServeType : uint64_t
{
	HttpGet = 1,
	HttpPost = HttpGet << 1,
	WebSocketBin = HttpGet << 2,
	WebSocketText = HttpGet << 3,
};



namespace SMUtils
{
	
	UTILS_EXPORT bool SInitLog(const LogConfig& conf);
	UTILS_EXPORT bool InitLog(const LogConfig& conf);
	

	const static size_t _headLen = sizeof(cmdhead);
	static std::string _invalidReq = "invalid request";
	static std::string _invalidHead = "invalid head";
	const static int _invalidAss = 0;

	/*************************************
	 * as jwt-cpp depend openssl, may be follow jwt-cpp api should remove to other module(networkinterface) later
	 * ***********************************/
	UTILS_EXPORT void initjwtconfig(string_view issuer="alqaz", string_view type="JWS", string_view key="key", uint32_t expiresecond=3600);
	UTILS_EXPORT bool isjwttokenright(string_view token);
	UTILS_EXPORT string getjwttoken();

	UTILS_EXPORT void packuint16(std::string& dst, uint16_t src);

	UTILS_EXPORT void packuint32(std::string& dst, uint32_t src);

	UTILS_EXPORT bool unpackuint16(std::string& src, uint16_t& dst);

	UTILS_EXPORT bool unpackuint32(std::string& src, uint32_t& dst);

	UTILS_EXPORT string packheads(MainCmd mainc, short assc);

	UTILS_EXPORT string packlen(size_t len);

	UTILS_EXPORT std::tuple<bool, string_view> unpackuint16(string_view src, uint16_t& dst);

	UTILS_EXPORT std::tuple<bool, string_view> unpackuint32(string_view src, uint32_t& dst);

	UTILS_EXPORT cmdhead parsecmd(std::string_view header);

	UTILS_EXPORT cmdsubhead parsesubcmd(std::string_view header);

	UTILS_EXPORT deque<uint8_t> packcmdrep(MainCmd mainc, short assc, size_t len);


	UTILS_EXPORT string getConfigPrefixPath();

	UTILS_EXPORT map<string, string> parseQuery(string_view query);

	template<class AssType>
	static void  packmaincmd3(MainCmd mainc, AssType assc, string& ret, string_view msg)
	{
		packuint16(ret, static_cast<uint16_t>(mainc));
		packuint16(ret, static_cast<uint16_t>(assc));
		packuint32(ret, static_cast<uint32_t>(msg.size()));
		ret.insert(ret.end(), msg.begin(), msg.end());
	}

	UTILS_EXPORT void  packmaincmd3(uint16_t assc, string& ret, string_view msg);

	UTILS_EXPORT void  packmaincmd3(string& ret, string_view msg);

	UTILS_EXPORT map<string, string> parseQueryString(string str);

	template<class AssType, class MsgContainer>
	static bool  uppackmaincmd3(MainCmd& mainc, AssType& assc, size_t& msglen, string& src, MsgContainer& msg)
	{
		uint16_t imainc;
		uint16_t iassc;
		uint32_t imsglen;
		bool bret{ false };
		bret = unpackuint16(src, imainc);
		if (!bret)
		{
			return false;
		}
		bret = unpackuint16(src, iassc);
		if (!bret)
		{
			return false;
		}
		bret = unpackuint32(src, imsglen);
		if (!bret)
		{
			return false;
		}
		msg.insert(msg.end(), src.begin(), src.end());
		mainc = (MainCmd)(imainc);
		assc = (AssType)(iassc);
		msglen = (size_t)(imsglen);

		return true;
	}

	template<class AssType, class MsgContainer>
	static bool  uppackmaincmd3(MainCmd& mainc, AssType& assc, size_t& msglen, string_view src, MsgContainer& msg)
	{
		uint16_t imainc;
		uint16_t iassc;
		uint32_t imsglen;
		auto [s1, t1] = unpackuint16(src, imainc);
		if (!s1)
		{
			return false;
		}
		auto [s2, t2] = unpackuint16(t1, iassc);
		if (!s2)
		{
			return false;
		}
		auto [s3, t3] = unpackuint32(t2, imsglen);
		if (!s3)
		{
			return false;
		}
		msg.insert(msg.end(), t3.begin(), t3.end());
		mainc = (MainCmd)(imainc);
		assc = (AssType)(iassc);
		msglen = (size_t)(imsglen);
		return true;
	}


	template<class MsgContainer>
	static bool  uppackmaincmd3(size_t& msglen, string& src, MsgContainer& msg)
	{
		uint32_t imsglen;
		bool bret{ false };
		bret = unpackuint32(src, imsglen);
		if (!bret)
		{
			return false;
		}
		msg.insert(msg.end(), src.begin(), src.end());
		msglen = (size_t)(imsglen);

		return true;
	}

	template<class MsgContainer>
	static bool  uppackmaincmd3(size_t& msglen, string_view src, MsgContainer& msg)
	{
		uint32_t imsglen;
		auto [s3, t3] = unpackuint32(src, imsglen);
		if (!s3)
		{
			return false;
		}
		msg.insert(msg.end(), t3.begin(), t3.end());
		msglen = (size_t)(imsglen);
		return true;
	}

	template<class MsgContainer>
	static bool  uppackmaincmd3(size_t& msglen, uint16_t& assc, std::string& src, MsgContainer& msg)
	{
		uint32_t imsglen;
		uint16_t asscc{ 0xffff };
		bool bret = unpackuint16(src, asscc);
		if (!bret)
		{
			return false;
		}
		bret = unpackuint32(src, imsglen);
		if (!bret)
		{
			return false;
		}
		msg.insert(msg.end(), src.begin(), src.end());
		msglen = (size_t)(imsglen);
		assc = asscc;
		return true;
	}

	template<class MsgContainer>
	static bool  uppackmaincmd3(size_t& msglen, uint16_t& assc, string_view src, MsgContainer& msg)
	{
		uint32_t imsglen;
		auto [s2, t2] = unpackuint16(src, assc);
		if (!s2)
		{
			return false;
		}
		auto [s3, t3] = unpackuint32(src, imsglen);
		if (!s3)
		{
			return false;
		}
		msg.insert(msg.end(), t3.begin(), t3.end());
		msglen = (size_t)(imsglen);
		return true;
	}

}
