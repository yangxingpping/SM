#include "Utils.h"
#include "templatefuncs.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <cppcodec/base32_crockford.hpp>
#include <cppcodec/base64_rfc4648.hpp>

#include "jwt-cpp/jwt.h"
#include "os/os.h"

#include <memory>
#include <atomic>

using base32 = cppcodec::base32_crockford;
using base64 = cppcodec::base64_rfc4648;

using std::make_shared;
using std::unique_ptr;
using std::shared_ptr;
using std::make_unique;
using std::vector;
using std::pair;
using std::make_tuple;

namespace SMUtils
{

	static map<int, string> _dbValue2Name;
	static map<int, string> _mainValue2Name;
	
	UTILS_EXPORT void packuint16(std::string& dst, uint16_t src)
	{
		uint8_t tmp[sizeof(src)];
#ifdef _WIN32
		auto t = htons(static_cast<u_short>(src));
#else
		auto t = htons(src);
#endif
		memcpy(tmp, &t, sizeof(src));
		for (size_t i = 0; i < sizeof(src); ++i)
		{
			dst.push_back(tmp[i]);
		}
	}

	UTILS_EXPORT void packuint32(std::string& dst, uint32_t src)
	{
		uint8_t tmp[sizeof(src)];
#ifdef _WIN32
		auto t = htonl(static_cast<u_long>(src));

#else
		auto t = htonl(src);
#endif
		memcpy(tmp, &t, sizeof(src));
		for (size_t i = 0; i < sizeof(src); ++i)
		{
			dst.push_back(tmp[i]);
		}
	}


	UTILS_EXPORT void packuint16(std::span<char> dst, uint16_t src)
	{
		assert(dst.size() == sizeof(src));
#ifdef _WIN32
		auto t = htons(static_cast<u_short>(src));
#else
		auto t = htons(src);
#endif
		memcpy(dst.data(), &t, dst.size());
		
	}

	UTILS_EXPORT void packuint32(std::span<char> dst, uint32_t src)
	{
		assert(dst.size() == sizeof(src));
#ifdef _WIN32
		auto t = htonl(static_cast<u_long>(src));

#else
		auto t = htonl(src);
#endif
		memcpy(dst.data(), &t, dst.size());
		
	}

	UTILS_EXPORT bool unpackuint16(std::string& src, uint16_t& dst)
	{
		bool bret = false;
		if (src.size() < sizeof(dst))
		{
			return bret;
		}
		uint8_t tmp[sizeof(dst)];
		memcpy(tmp, src.data(), sizeof(dst));
		src.erase(0, sizeof(dst));
#ifdef _WIN32
		dst = static_cast<uint16_t>(ntohs(static_cast<u_short>(*(uint16_t*)tmp)));
#else
		dst = ntohs(*(uint16_t*)tmp);
#endif
		bret = true;
		return bret;
	}

	UTILS_EXPORT bool unpackuint32(std::string& src, uint32_t& dst)
	{
		bool bret = false;
		if (src.size() < sizeof(dst))
		{
			return bret;
		}
		uint8_t tmp[sizeof(dst)];
		memcpy(tmp, src.data(), sizeof(dst));
		src.erase(0, sizeof(dst));
#ifdef _WIN32
		dst = static_cast<uint32_t>(ntohl(static_cast<u_long>(*(uint32_t*)tmp)));
#else
		dst = ntohl(*(uint32_t*)tmp);
#endif
		bret = true;
		return bret;
	}

	std::tuple<bool, string_view> unpackuint16(string_view src, uint16_t& dst)
	{
		bool bret = false;
		if (src.size() < sizeof(dst))
		{
			return std::make_tuple<bool, string_view>(std::move(bret), string_view());
		}
		const char* tmp = src.data();
#ifdef _WIN32
		dst = static_cast<uint16_t>(ntohs(static_cast<u_short>(*(uint16_t*)tmp)));
#else
		dst = ntohs(*(uint16_t*)tmp);
#endif
		bret = true;
		return make_tuple(std::move(bret), std::string_view(src.data() + sizeof(dst), src.length() - sizeof(dst)));
	}

	std::tuple<bool, string_view> unpackuint32(string_view src, uint32_t& dst)
	{
		bool bret = false;
		if (src.size() < sizeof(dst))
		{
 			return std::make_tuple<bool, string_view>(std::move(bret), string_view());
		}
		const char* tmp = src.data();
		uint32_t nu32 = *((uint32_t*)(tmp));
#ifdef _WIN32
		
		dst = static_cast<uint32_t>(ntohl(nu32));
#else
		dst = ntohl(nu32);
#endif
		bret = true;
		return std::make_tuple(std::move(bret), string_view(src.data() + sizeof(dst), src.length() - sizeof(dst)));
	}

	string packlen(size_t len)
	{
		string strret;
		deque<uint8_t> ret;
		uint32_t ilen = static_cast<uint32_t>(len);
		char ch[4] = { 0 };
		uint32_t lc = static_cast<uint32_t>(htonl(ilen));
		memcpy(ch, &lc, sizeof(lc));
		ret = base64::encode<deque<uint8_t>>(ch, 4);
		std::ostringstream convert;
		for (size_t a = 0; a < ret.size(); a++) {
			convert << ret[a];
		}
		strret = convert.str();
		return strret;
	}

	cmdhead parsecmd(std::string_view header)
	{
		cmdhead ret;
		BEGIN_STD;
		std::vector<uint8_t> decodex = base64::decode(header);
		if (decodex.size() != 8)
		{
			return ret;
		}
		ret.mainc = (MainCmd)ntohs((u_short)(*(u_short*)(&decodex[0])));
		ret.assc = ntohs((u_short)(*(u_short*)(&decodex[2])));
		ret.len = ntohl(*((u_long*)(&decodex[4])));
		END_STD;
		return ret;
	}

	cmdsubhead parsesubcmd(std::string_view header)
	{
		cmdsubhead ret;
		BEGIN_STD;
		std::vector<uint8_t> decodex = base64::decode(header);
		if (decodex.size() != 6)
		{
			return ret;
		}
		ret.assc = ntohs((u_short)(*(u_short*)(&decodex[0])));
		ret.len = ntohl((u_long)(*(u_long*)(&decodex[2])));
		END_STD;

		return ret;
	}

	deque<uint8_t> packcmdrep(MainCmd mainc, short assc, size_t len)
		 {
		assert(sizeof(u_short) == 2);
		deque<uint8_t> ret;
		uint32_t ilen = static_cast<uint32_t>(len);
		char ch[9] = { 0 };
		uint16_t mc = static_cast<uint16_t>(htons((u_short)(mainc)));
		uint16_t ac = static_cast<uint16_t>(htons((u_short)(assc)));
		uint32_t lc = static_cast<uint32_t>(htonl(ilen));
		memcpy(ch, &mc, sizeof(mc));
		memcpy(ch + 2, &ac, sizeof(ac));
		memcpy(ch + 4, &lc, sizeof(lc));
		ret = base64::encode<deque<uint8_t>>(ch, 8);
		return ret;
		}
	static std::atomic<uint32_t> _u32tSeq = 0;
	UTILS_EXPORT seqNumType getSeqNum()
	{
		seqNumType ret = ++_u32tSeq;
		return ret;
	}

	string packheads(uint16_t mainc, short assc) {
		string strret;
		deque<uint8_t> ret;
		char ch[5] = { 0 };
		auto mc = htons((u_short)(mainc));
		auto ac = htons(static_cast<u_short>(assc));
		memcpy(ch, &mc, sizeof(mc));
		memcpy(ch + 2, &ac, sizeof(ac));
		ret = base64::encode<deque<uint8_t>>(ch, 4);
		std::ostringstream convert;
		for (size_t a = 0; a < ret.size(); a++) 
		{
			convert << ret[a];
		}
		strret = convert.str();
		return strret;
	}

	

	

	string getConfigPrefixPath()
	{
#ifdef _WIN32
		return "/tmp/";
#else
		return "config/";
#endif
	}

	map<string, string> parseQuery(string_view query)
	{
		(void)(query);
		map<string, string> ret;
		vector<string> qs;
		return ret;
	}

	int convertDateToSeconds(string_view dFormat, string_view date)
	{
		int ret{ 0 };
		std::istringstream ss{ string(date.data(), date.length()) };
		std::chrono::sys_days tp;
#ifdef _WIN32
		ss >> std::chrono::parse("%F", tp);
#else
		//ss >> date::parse("%F", tp);
#endif
		if (ss.bad())
		{
			SPDLOG_WARN("parse date {} failed for fomrat {} upload order", date, dFormat);
		}
		else
		{
			ret = tp.time_since_epoch().count();
		}
		return ret;
	}

	void  packmaincmd3(uint16_t assc, string& ret, string_view msg)
	{
		packuint16(ret, static_cast<uint16_t>(assc));
		packuint32(ret, static_cast<uint32_t>(msg.size()));
		ret.insert(ret.end(), msg.begin(), msg.end());
	}

	void  packmaincmd3(string& ret, string_view msg)
	{
		packuint32(ret, static_cast<uint32_t>(msg.size()));
		ret.insert(ret.end(), msg.begin(), msg.end());
	}

	map<string, string> parseQueryString(string str)
	{
		map<string, string> ret;
		vector<string> vtemp;
		while (!str.empty())
		{
			auto pos = str.find('&');
			if (pos != string::npos)
			{
				vtemp.emplace_back(string(str.c_str(), pos));
			}
			else
			{
				vtemp.push_back(str);
				str.clear();
			}
			str.erase(0, pos + 1);
		}
		while (!vtemp.empty())
		{
			auto it = vtemp.begin();
			auto pos = it->find('=');
			if (pos != string::npos)
			{
				ret.insert({ it->substr(0, pos), it->substr(pos + 1, it->size() - pos - 1) });
			}
			else
			{
				ret.insert({ *it, "" });
			}
			vtemp.erase(it);
		}
		return ret;
	}

	bool addOpDBValue2Name(int v, string_view name)
	{
		if(_dbValue2Name.contains(v))
		{
			SPDLOG_WARN("value[{}] have name [{}], new name [{}] can not set", v, _dbValue2Name[v], name);
			return false;
		}
		_dbValue2Name[v] = name;
		return true;
	}

	UTILS_EXPORT string_view getOpDBNameByValue(int v)
	{
		static string dbNotFound = fmt::format("db {} not found", v);
		auto it = _dbValue2Name.find(v);
		if(it!=_dbValue2Name.end())
		{
			return string_view(it->second);
		}
		SPDLOG_WARN("db main value {} not found", v);
		return string_view(dbNotFound);
	}

	UTILS_EXPORT bool addMainValue2Name(int v, string_view name)
	{
		if (_mainValue2Name.contains(v))
		{
			SPDLOG_WARN("value[{}] have name [{}], new name [{}] can not set", v, _mainValue2Name[v], name);
			return false;
		}
		_mainValue2Name[v] = name;
		return true;
	}

	UTILS_EXPORT string packstring(string_view msg)
	{
		thread_local string ret;
		ret.clear();
		packuint32(ret, (uint32_t)(msg.length()));
		ret += msg;
		return ret;
	}

	UTILS_EXPORT string_view unpackstring(string_view msg)
	{
		uint32_t strlen = 0;
		auto ret = unpackuint32(msg, strlen);
		if (!std::get<0>(ret) || strlen+sizeof(uint32_t)>msg.length())
		{
			return string_view("");
		}
		return string_view(std::get<1>(ret).data(), strlen);
	}

	UTILS_EXPORT string_view getMainNameByValue(int v)
	{
		static string mainNotFound = fmt::format("main {} not found", v);
		auto it = _mainValue2Name.find(v);
		if (it != _mainValue2Name.end())
		{
			return string_view(it->second);
		}
		SPDLOG_WARN("main value {} not found", v);
		return string_view(mainNotFound);
	}

};
