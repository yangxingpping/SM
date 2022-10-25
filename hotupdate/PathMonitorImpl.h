#pragma once
#include "hotupdateExport.h"
#include "efsw/efsw.hpp"

namespace SMHotupdate
{


class FilePathMonitor;

class HOTUPDATE_EXPORT PathMonitorImpl : public efsw::FileWatchListener
{
  public:
    PathMonitorImpl(FilePathMonitor* monitor);
    void handleFileAction(efsw::WatchID watchid, const std::string& dir,
        const std::string& filename, efsw::Action action,
        std::string oldFilename) override;

  private:
    FilePathMonitor* _pathMonitor = nullptr;
};
}


