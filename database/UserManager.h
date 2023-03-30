
#pragma once

#include "reqreps.h"

#include <string>

#define UMR (SMDB::UserManager::getInst())

using std::string;

namespace SMDB
{

	class UserManager
	{
	public:
		static UserManager& getInst();

	public:
		UserGenInfoItem* getUserInfo(string& phone);
		bool addUserInfo(string& phone, UserGenInfoItem& info);
	};
}
