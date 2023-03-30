#pragma once
#include "hotupdateExport.h"
#include "concurrentqueue/concurrentqueue.h"
#include "PathMonitorImpl.h"
#include "FileOp.h"
#include "tsl/htrie_map.h"
#include "tsl/htrie_set.h"
#include "CoEvent.h"
#include <string>
#include <memory>
#include <set>
#include <map>
#if defined(ASIO_HAS_IO_URING) || defined(_WIN32) 
using asio::stream_file;
#endif
using asio::io_context;
using std::shared_ptr;
using std::mutex;
using std::lock_guard;

#define RESLOAD (SMHotupdate::FilePathMonitor::getInst())
namespace SMHotupdate
{
class HOTUPDATE_EXPORT FilePathMonitor
{
  public:
	  static shared_ptr<FilePathMonitor> getInst();
  public:
	FilePathMonitor();
	/**
	 * @brief add file monitor path 
	 * not thread safe
	 * 
	 * @param path 
	 * @param recursion 
	 * @return true 
	 * @return false 
	 */
	bool addMonitor(std::string path, bool recursion = false);
	void notifyLoadComplete(std::string path);
	void notifyFileModified(std::string path);
	void notifyConfigModified(std::string path);
    asio::awaitable<string_view> asyncGetResource(std::string path);
	asio::awaitable<void> waitHotUpdate();
	void setReady();
	bool isReady();
private:
	tsl::htrie_map<char, std::shared_ptr<fileOp>> _files;
	tsl::htrie_set<char> _paths;
	moodycamel::ConcurrentQueue<string> _pathsUpdate;
	moodycamel::ConcurrentQueue<string> _pathsErase;
	std::unique_ptr<efsw::FileWatcher> _fileWatcher;
	std::unique_ptr<PathMonitorImpl> _listener;
	shared_ptr<CoEvent> _eventHotUpdate = nullptr;
	bool _isReady = false;
};
}
