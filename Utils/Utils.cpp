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
	static std::string _issuer{"alqaz"};
	static std::string _type{"JWS"};
	static std::string _key{"key"};
	static uint32_t _expiresecond{3600};

	UTILS_EXPORT void initjwtconfig(string_view issuer/*="alqaz"*/, string_view type/*="JWS"*/, string_view key/*="key"*/, uint32_t expiresecond/*=3600*/)
	{
		_issuer = issuer;
		_type = type;
		_key = key;
		_expiresecond = expiresecond;
	}

	UTILS_EXPORT bool isjwttokenright(string_view token)
	{
		bool bret{ false };
		if (token.empty())
		{
			return bret;
		}
		auto ver = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ _key }).with_issuer(_issuer);
		BEGIN_STD;
		auto decoded = jwt::decode(token.data());
		std::error_code ec;
		ver.verify(decoded, ec);
		if (ec.value() != 0)
		{
			SPDLOG_WARN("jwt token {} failed with {} {}", token, ec.value(), ec.message());
		}
		bret = (ec.value() == 0) ? true : false;
		END_STD;
		
		return bret;
	}

	UTILS_EXPORT string getjwttoken()
	{
		string ret;
		auto token = jwt::create();
		ret = token.set_issuer(_issuer).set_type(_type).set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(_expiresecond)).set_payload_claim("name", jwt::claim(std::string("alqaz"))).sign(jwt::algorithm::hs256{ _key });
		return ret;
	}

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
#ifdef _WIN32
		dst = static_cast<uint32_t>(ntohl(static_cast<u_short>(*(uint32_t*)tmp)));
#else
		dst = ntohl(*(uint32_t*)tmp);
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

	string packheads(MainCmd mainc, short assc) {
		string strret;
		deque<uint8_t> ret;
		char ch[5] = { 0 };
		auto mc = htons((u_short)(mainc));
		auto ac = htons((u_short)(assc));
		memcpy(ch, &mc, sizeof(mc));
		memcpy(ch + 2, &ac, sizeof(ac));
		ret = base64::encode<deque<uint8_t>>(ch, 4);
		std::ostringstream convert;
		for (size_t a = 0; a < ret.size(); a++) {
			convert << ret[a];
		}
		strret = convert.str();
		return strret;
	}

	static LogRemoteConfig _remote;
	static LogContextConfig _context;
	static vector<shared_ptr<spdlog::sinks::sink>> _sinks;
	static shared_ptr<spdlog::logger> _loger = { nullptr };

	bool SInitLog(const LogConfig& conf)
	{
		return InitLog(conf);
	}

	

	bool InitLog(const LogConfig& conf)
	{
		bool bret = true;
		_remote = conf._remote;
		_context = conf._context;

		auto outsink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
		outsink->set_level((spdlog::level::level_enum)_context._level);
		_sinks.push_back(outsink);

		auto rotatesink = make_shared<spdlog::sinks::rotating_file_sink_mt>(_context._filepath + _context._file, _context._fileRollSize, _context.rollfilecount);
		rotatesink->set_level((spdlog::level::level_enum)_context._level);
		_sinks.push_back(rotatesink);

		_loger = shared_ptr<spdlog::logger>(new spdlog::logger("loger", _sinks.begin(), _sinks.end()));
		_loger->set_pattern(_context._format);
		_loger->set_level((spdlog::level::level_enum)_context._level);
		spdlog::set_default_logger(_loger);
		spdlog::default_logger()->flush_on(spdlog::level::info);
		return bret;
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
				vtemp.push_back(string(str.c_str(), pos));
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

};
