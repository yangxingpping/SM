
#include "asio/use_awaitable.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "spdlog/spdlog.h"
#include "hotupdate.h"
#include "CoEvent.h"


namespace SMHotupdate
{

	class fileOpSync
	{
	public:
		static asio::awaitable<bool> getData(string& dst, string_view path, off_t offset, size_t len)
		{
			bool bret = true;
			
			if (len == 0)
			{
				co_return false;
			}
			CoEvent event(*IOCTX);
			asio::co_spawn(*IOCTX, [&]()->asio::awaitable<void> {
				std::ifstream ifs;
				ifs.open(path.data());
				if (ifs.good())
				{
					ifs.seekg(offset);
					ifs.read(&dst[0], len);
					SPDLOG_INFO("sync get data length {} for path {}", len, path);
				}
				else
				{
					SPDLOG_WARN("get data from file [{}] failed", path);
				}
				event.trigger_all();
				co_return;
				}, asio::detached);
			BEGIN_ASIO;
			co_await event.async_wait();
			END_ASIO;
			co_return bret;
		}
	};

}

