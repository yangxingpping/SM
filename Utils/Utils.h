#pragma once
#include "UtilsExport.h"
#include "enums.h"
#include "magic_enum.hpp"
#include "Structs.h"

#include <string_view>
#include <stdint.h>
#include <string>
#include <span>
#include <vector>
#include <deque>
#include <map>
#include <tuple>
#include <functional>

using std::vector;
using std::string;
using std::map;
using std::string_view;
using std::deque;

using seqNumType = uint32_t;

enum class ServeType : uint64_t
{
	HttpGet = 1,
	HttpPost = HttpGet << 1,
	WebSocketBin = HttpGet << 2,
	WebSocketText = HttpGet << 3,
};



namespace SMUtils
{
	UTILS_EXPORT void packuint16(std::string& dst, uint16_t src);

	UTILS_EXPORT void packuint32(std::string& dst, uint32_t src);

	UTILS_EXPORT void packuint16(std::span<char> dst, uint16_t src);

	UTILS_EXPORT void packuint32(std::span<char> dst, uint32_t src);

	UTILS_EXPORT bool unpackuint16(std::string& src, uint16_t& dst);

	UTILS_EXPORT bool unpackuint32(std::string& src, uint32_t& dst);


	UTILS_EXPORT string packheads(uint16_t mainc, short assc);

	UTILS_EXPORT string packlen(size_t len);

	UTILS_EXPORT std::tuple<bool, string_view> unpackuint16(string_view src, uint16_t& dst);

	UTILS_EXPORT std::tuple<bool, string_view> unpackuint32(string_view src, uint32_t& dst);

	UTILS_EXPORT cmdhead parsecmd(std::string_view header);

	UTILS_EXPORT cmdsubhead parsesubcmd(std::string_view header);

	UTILS_EXPORT deque<uint8_t> packcmdrep(MainCmd mainc, short assc, size_t len);

	UTILS_EXPORT seqNumType getSeqNum();

	UTILS_EXPORT string getConfigPrefixPath();

	UTILS_EXPORT map<string, string> parseQuery(string_view query);

	UTILS_EXPORT int convertDateToSeconds(string_view dFormat, string_view date);

	template<class AssType>
	static void  packmaincmd3(MainCmd mainc, AssType assc, string& ret, string_view msg)
	{
		packuint16(ret, static_cast<uint16_t>(mainc));
		packuint16(ret, static_cast<uint16_t>(assc));
		packuint32(ret, static_cast<uint32_t>(msg.size()));
		ret.insert(ret.end(), msg.begin(), msg.end());
	}

	template<class AssType>
	static void  packmaincmd3(seqNumType seqnum, MainCmd mainc, AssType assc, string& ret, string_view msg)
	{
		packuint32(ret, static_cast<uint32_t>(seqnum));
		packuint16(ret, static_cast<uint16_t>(mainc));
		packuint16(ret, static_cast<uint16_t>(assc));
		packuint32(ret, static_cast<uint32_t>(msg.size()));
		ret.insert(ret.end(), msg.begin(), msg.end());
	}

	UTILS_EXPORT void  packmaincmd3(uint16_t assc, string& ret, string_view msg);

	UTILS_EXPORT void  packmaincmd3(string& ret, string_view msg);

	UTILS_EXPORT map<string, string> parseQueryString(string str);

	UTILS_EXPORT bool addOpDBValue2Name(int v, string_view name);

	UTILS_EXPORT string_view getOpDBNameByValue(int v);

	UTILS_EXPORT bool addMainValue2Name(int v, string_view name);

	UTILS_EXPORT string packstring(string_view msg);
	UTILS_EXPORT string_view unpackstring(string_view msg);

	UTILS_EXPORT string_view getMainNameByValue(int v);

	template<class AssType, class MsgContainer>
	static bool  uppackmaincmd3(seqNumType* seqnum, MainCmd& mainc, AssType& assc, size_t& msglen, string& src, MsgContainer& msg)
	{
		uint32_t iseqnum;
		uint16_t imainc;
		uint16_t iassc;
		uint32_t imsglen;
		bool bret{ false };
		bret = unpackuint32(src, iseqnum);
		if (!bret)
		{
			return false;
		}
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
		*seqnum = iseqnum;
		mainc = (MainCmd)(imainc);
		assc = (AssType)(iassc);
		msglen = (size_t)(imsglen);

		return true;
	}

	template<class AssType, class MsgContainer>
	static bool  uppackmaincmd3(seqNumType* seqnum, MainCmd& mainc, AssType& assc, size_t& msglen, string_view src, MsgContainer& msg)
	{
		uint32_t iseq;
		uint16_t imainc;
		uint16_t iassc;
		uint32_t imsglen;
		auto [s0, t0] = unpackuint32(src, iseq);
		if (!s0)
		{
			return false;
		}
		auto [s1, t1] = unpackuint16(t0, imainc);
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
		*seqnum = iseq;
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

	template<class DBEnums>
	bool cacheDBValuesNames()
	{
		bool bret = true;
		auto pairs = magic_enum::enum_entries<DBEnums>();
		for(const auto& p: pairs)
		{
			bret = addOpDBValue2Name(magic_enum::enum_integer(std::get<0>(p)), std::get<1>(p));
			if(!bret)
			{
				break;
			}
		}
		assert(bret);
		return bret;
	}

	template<class MainEnums>
	bool cacheMainValuesNames()
	{
		bool bret = true;
		auto pairs = magic_enum::enum_entries<MainEnums>();
		for (const auto& p : pairs)
		{
			bret = addMainValue2Name(magic_enum::enum_integer(std::get<0>(p)), std::get<1>(p));
			if (!bret)
			{
				break;
			}
		}
		assert(bret);
		return bret;
	}

	//https://stackoverflow.com/questions/1198260/how-can-you-iterate-over-the-elements-of-an-stdtuple

	template <
		size_t Index = 0, // start iteration at 0 index
		typename TTuple,  // the tuple type
		size_t Size =
		std::tuple_size_v<
		std::remove_reference_t<TTuple>>, // tuple size
		typename TCallable, // the callable to bo invoked for each tuple item
		typename... TArgs   // other arguments to be passed to the callable 
		>
		void for_each(TTuple&& tuple, TCallable&& callable, TArgs&&... args)
	{
		if constexpr (Index < Size)
		{
			if constexpr (std::is_assignable_v<bool&, std::invoke_result_t<TCallable&&, TArgs&&..., decltype(std::get<Index>(tuple))>>)
			{
				if (!std::invoke(callable, args..., std::get<Index>(tuple)))
					return;
			}
			else
			{
				std::invoke(callable, args..., std::get<Index>(tuple));
			}

			if constexpr (Index + 1 < Size)
				for_each<Index + 1>(
					std::forward<TTuple>(tuple),
					std::forward<TCallable>(callable),
					std::forward<TArgs>(args)...);
		}
	}
	
}
