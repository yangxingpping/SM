#pragma once
#include "hotupdateExport.h"
#include "CoEvent.h"
#include "asio/system_timer.hpp"
#include "asio/awaitable.hpp"
#include <string>
#include <memory>

#if defined(ASIO_HAS_IO_URING) || defined(_WIN32) 
#include "asio/stream_file.hpp"
using asio::stream_file;
#endif
using asio::io_context;
using std::shared_ptr;
using std::string;
using std::string_view;

namespace SMHotupdate
{
class HOTUPDATE_EXPORT fileOp
{
public:
	fileOp(std::string path);
	fileOp(const fileOp&) = delete;
	fileOp(const fileOp&& cv) = delete;
	string_view getData();
	const string getPath();
    asio::awaitable<string_view> asyncGetData();
	size_t getFileSize();
  private:
	std::string _path;
	size_t _datalen;
	bool _exist;
	string _datas;
	string _filepath;
	SMHotupdate::CoEvent _ev;
};
}

