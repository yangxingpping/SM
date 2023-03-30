
#include "UserManager.h"

#include <map>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;
using std::map;
using std::string;

namespace SMDB
{
	static UserManager _inst;
	static map<string, unique_ptr<UserGenInfoItem>> _phone2Info;
	UserManager& UserManager::getInst()
	{
		return _inst;
	}

	UserGenInfoItem* UserManager::getUserInfo(string& phone)
	{
		UserGenInfoItem* pret{ nullptr };
		auto it = _phone2Info.find(phone);
		if (it != _phone2Info.end())
		{
			pret = it->second.get();
		}
		return pret;
	}

	bool UserManager::addUserInfo(string& phone, UserGenInfoItem& info)
	{
		bool bret{ true };
		auto it = _phone2Info.find(phone);
		if (it != _phone2Info.end())
		{
			bret = false;
			it->second = make_unique<UserGenInfoItem>(info);
		}
		else
		{
			_phone2Info[phone] = make_unique<UserGenInfoItem>(info);
		}
		return bret;
	}

}
