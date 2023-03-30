#include "Https.h"
#include "Utils.h"
#include "FilePathMonitor.h"
#include "Configs.h"
#include "wss.h"
#include "spdlog/spdlog.h"
#include "asio/signal_set.hpp"
#include "networkinterface.h"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/awaitable.hpp"
#include "jwt-cpp/jwt.h"

#include <locale>
#include <memory>
#include <thread>

using std::make_shared;
using std::string;

namespace SMNetwork
{
	static tsl::htrie_map<char, HtmlBodyType> _contentTypes = {
			{".js", HtmlBodyType::JS},
		{".html", HtmlBodyType::Html},
		{".htm", HtmlBodyType::Html},
		{".json", HtmlBodyType::Json},
			{".woff", HtmlBodyType::Woff},
		{".jpg", HtmlBodyType::Jpeg},
			{".jpeg", HtmlBodyType::Jpeg},
			{".css", HtmlBodyType::Css},
			{".ico", HtmlBodyType::Ico} };
	static map<HtmlBodyType, map<string, string>> _respHeaders;
	static map<string, string> _resHeaders;
	static string _noAuthRep{ "need auth" };
	


	map<string, string>& Https::getRespHeaders(HtmlBodyType type)
	{
		static map<string, string> defaultheaders;
		auto it = _respHeaders.find(type);
		if (it != _respHeaders.end())
		{
			return it->second;
		}
		return defaultheaders;
	}

	HtmlBodyType Https::getRespBodyType(const string& context)
	{
		return getRespBodyType(string_view(context.data(), context.length()));
	}

	HtmlBodyType Https::getRespBodyType(string_view context)
	{
		HtmlBodyType ret = HtmlBodyType::Html;
		auto it = _contentTypes.find(context);
		if (it != _contentTypes.end())
		{
			ret = it.value();
		}
		return ret;
	}

	void Https::sInit()
	{
		_respHeaders[HtmlBodyType::JS].insert({ "content-type", "text/javascript" });

		_respHeaders[HtmlBodyType::Html].insert({ "content-type",  "text/html;charset=UTF-8" });
		_respHeaders[HtmlBodyType::Json].insert({ "content-type",  "application/json" });
		_respHeaders[HtmlBodyType::Woff].insert({ "content-type", "font/woff" });
		_respHeaders[HtmlBodyType::Jpeg].insert({ "content-type",  "image/jpeg" });
		_respHeaders[HtmlBodyType::Css].insert({ "content-type",  "text/css" });
		_respHeaders[HtmlBodyType::Ico].insert({ "content-type", "image/vnd.microsoft.icon" });
	}
}
