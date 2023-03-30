
#include "FilePathMonitor.h"
#include "Utils.h"
#include "hotupdate.h"
#include "Configs.h"
#include "templatefuncs.h"
#include <filesystem>

using std::make_unique;
using std::make_shared;

namespace SMHotupdate
{
    static char _shttp_url_sep = '/';

shared_ptr<FilePathMonitor> _inst = nullptr;

shared_ptr<FilePathMonitor> FilePathMonitor::getInst()
{
    if (_inst == nullptr)
    {
        auto resconf = SMCONF::Configs::getInst2().getHttpConfig();
        if (!std::filesystem::exists(std::filesystem::path(resconf._resRootPath)))
        {
            SPDLOG_ERROR("http resource root path {} not exist", resconf._resRootPath);
        }
        _inst = make_shared<FilePathMonitor>();
        auto bsucc = _inst->addMonitor(resconf._resRootPath, true);
        if (bsucc == false)
        {
            SPDLOG_WARN("add monitor path {} failed", resconf._resRootPath);
            _inst = nullptr;
        }
        _inst->setReady();
        assert(bsucc);
    }
    return _inst;
}

FilePathMonitor::FilePathMonitor()
{
    _paths.clear();
    _fileWatcher = std::make_unique<efsw::FileWatcher>();
    _listener = std::make_unique<PathMonitorImpl>(this);
}

bool FilePathMonitor::addMonitor(std::string path, bool recursion /* = false*/)
{
    bool bret = true;
    if (_eventHotUpdate == nullptr)
    {
        _eventHotUpdate = make_shared<CoEvent>(*IOCTX);
    }
    if (_paths.find(path) != _paths.end())
    {
        return bret;
    }
    BEGIN_STD;
    _fileWatcher->addWatch(path, _listener.get(), recursion);
    END_STD;
    _paths.insert(path);
    _fileWatcher->watch();
    return bret;
}

void FilePathMonitor::notifyLoadComplete(std::string path) { }

void FilePathMonitor::notifyFileModified(std::string path)
{
    SPDLOG_INFO("file[{}] modified", path);
    _pathsUpdate.enqueue(path);
    _eventHotUpdate->trigger();
}

void FilePathMonitor::notifyConfigModified(std::string path)
{
    SPDLOG_INFO("config[{}] modified", path);
    _pathsUpdate.enqueue(path);
    _eventHotUpdate->trigger();
}

asio::awaitable<string_view> FilePathMonitor::asyncGetResource(std::string path)
{
    string_view rep;
    auto resconf = SMCONF::Configs::getInst2().getHttpConfig();
    std::filesystem::path p = resconf._resRootPath;
    assert(!path.empty());
    if (path.empty() || path[0] != _shttp_url_sep)
    {
        SPDLOG_WARN("http url router [{}] invalid", path);
        co_return rep;
    }
    p.append(&path[1]);
    string absPath = p.string();
    auto file = _files.find(absPath);
    shared_ptr<fileOp> filex;
    if (file == _files.end())
    {
        SPDLOG_INFO("current need load file [{}] from disc", absPath);
        filex = make_shared<fileOp>(absPath);
        _files.insert(absPath, filex);       
    }
    else
    {
        filex = file.value();
    }
    BEGIN_ASIO;
	rep = co_await filex->asyncGetData();
    END_ASIO;
    if (rep.empty())
    {
        SPDLOG_WARN("get url {} response empty", path);
    }
    SPDLOG_INFO("resoource {} return data length {}", path, rep.length());
    co_return rep;
}

asio::awaitable<void> FilePathMonitor::waitHotUpdate()
{
    while (_eventHotUpdate)
    {
        BEGIN_ASIO;
        co_await _eventHotUpdate->async_wait();
        END_ASIO;
        string path;
        while (_pathsUpdate.try_dequeue(path))
        {
            _files.erase(path);
        }
    }
    co_return;
}

void FilePathMonitor::setReady()
{
    _isReady = true;
}
bool FilePathMonitor::isReady()
{
    return _isReady;
}

}

