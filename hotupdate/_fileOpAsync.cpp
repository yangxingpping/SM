
#if defined(ASIO_HAS_IO_URING) || defined(_WIN32) 

#include <string>
#include <string_view>
#include "hotupdate.h"
#include "asio/buffer.hpp"
#include "asio/use_awaitable.hpp"
#include "asio/stream_file.hpp"
#include "asio/random_access_file.hpp"
#include "asio/awaitable.hpp"
#include "spdlog/spdlog.h"

using std::string;
using std::string_view;

namespace SMHotupdate
{
	class fileOpASync
	{
	public:
		static asio::awaitable<bool> getData(string& dst, string_view path, off_t offset, size_t len)
		{
			bool bret = true;
			if (len == 0)
			{
				co_return false;
			}
			dst.resize(len);

			BEGIN_ASIO;
			asio::random_access_file _stream_file(*IOCTX);
			asio::error_code ec;
			asio::error_code openerr = _stream_file.open(path.data(), asio::stream_file::read_only, ec);
			if (openerr.value() != 0)
			{
				SPDLOG_WARN("async open path {} failed with error code {}", path, openerr.value());
				co_return false;
			}
			co_await _stream_file.async_read_some_at(offset, asio::buffer(dst), asio::use_awaitable);
			bret = true;
			END_ASIO;
			if (!bret)
			{
				SPDLOG_WARN("current not support asio read from file, use sync read from file");
			}
			co_return bret;
		}
	};

}
#else

#endif

