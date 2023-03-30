
#include "FileOp.h"
#include "_fileOpSync.cpp"
#include "_fileOpAsync.cpp"

#include <filesystem>
#include <fstream>

using std::filesystem::exists;
namespace SMHotupdate
{
fileOp::fileOp(std::string path)
    : _path(path), _datalen(0), _exist(false), _ev(*IOCTXHOTUPDATE, 24*3600*1000)
{
}

string_view fileOp::getData()
{
    return string_view{ &_datas[0], _datalen };
}

const string fileOp::getPath()
{
    return _path;
}

asio::awaitable<string_view> fileOp::asyncGetData()
{
    string_view ret;
    bool loadflag = false;
    if (_datas.empty() && _datalen==0)
    {
        auto filesize = getFileSize();
        if (filesize == 0)
        {
            co_return ret;
        }
        BEGIN_ASIO;
#if defined(ASIO_HAS_IO_URING) || defined(_WIN32) 
        loadflag = co_await fileOpASync::getData(_datas, _path, 0, filesize);
#endif
        if (!loadflag)
        {
            loadflag = co_await fileOpSync::getData(_datas, _path, 0, filesize);
        }
        END_ASIO;
        if (!loadflag)
        {
            SPDLOG_WARN("load file {} failed ", _path);
        }
        _ev.trigger_all();
    }
    else if (_datalen > 0 && _datas.empty()) //async load already start, wait it finish
    {
        SPDLOG_INFO("resource {} async load data had started by other co, this co wait it finish", _path);
        co_await _ev.async_wait();
    }
    else
    {
        SPDLOG_INFO("resource {} already cached, direct load it", _path);
    }
    ret = _datas;
    co_return ret;
}


size_t fileOp::getFileSize()
{
    if (_datalen != 0)
    {
        SPDLOG_INFO("file {} size have already get size {}", _path, _datalen);
        return _datalen;
    }
    BEGIN_ASIO;
    if (std::filesystem::exists(std::filesystem::path(_path)))
    {
        _datalen = std::filesystem::file_size(std::filesystem::path(_path));
    }
    else
    {
        SPDLOG_WARN("get file {} failed as file not exist", _path);
    }
    END_ASIO;
    if (_datalen == 0)
    {
        SPDLOG_WARN("get file [{}] dat len=0", _path);
    }
    SPDLOG_INFO("file {} data length {}", _path, _datalen);
    return _datalen;
}

}

