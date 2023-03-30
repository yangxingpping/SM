#include "PathMonitorImpl.h"

#include "FilePathMonitor.h"

#include "Configs.h"

#include <filesystem>

namespace SMHotupdate
{

PathMonitorImpl::PathMonitorImpl(FilePathMonitor* monitor)
	:_pathMonitor(monitor)
{

}

void PathMonitorImpl::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename)
{
	switch (action)
	{
	case efsw::Actions::Add:
	{
	}break;
	case efsw::Actions::Delete:
	{

	}break;
	case efsw::Actions::Modified:
	{
		std::filesystem::path p = dir;
		p.append(filename);
		auto localpath = p.string();
		if(localpath.ends_with(".toml"))
		{
			_pathMonitor->notifyConfigModified(localpath);
		}
		else
		{
			_pathMonitor->notifyFileModified(localpath);
		}
	}break;
	case efsw::Actions::Moved:
	{
	}break;
	default:
	{

	}break;
	}
}
}

