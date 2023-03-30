
#include "CommonDBProcess.h"
#include "DBManager.h"
#include "MessageSplitFuncs.h"
#include "Utils.h"
#include "dbCommon.h"
#include "CoEvent.h"
#include "sha256.h"
#include "jwt-cpp/jwt.h"

#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/update.h"
#include "sqlpp11/remove.h"

#include <string>
#include <set>
#include <mutex>

using std::string;
using std::set;
using std::mutex;
using std::lock_guard;


namespace SMDB
{
	static mutex _genmut;
	static map<string, map<string, UserGenInfoItem>> _genUsers;

	//static SHA256 _sha256;

	RegistRep CommonDBProcess::sregist(RegistReq req)
	{
		RegistRep rep;
		DBInst db;
		dbCommon::Users user;
		user.name = "";
		auto have = (*db)(select(count(user.name)).from(user).where(user.name == req.username)).front().count.value();
		if (have != 0)
		{
			SPDLOG_WARN("insert into user failed as name {} already exist", req.username);
			rep.code = to_underlying(statusCode::registNameRepeat);
		}
		else
		{
			req.password = req.password;// _sha256(req.phone + req.password);
			(*db)(insert_into(user).set(user.name = req.username, user.passwd = req.password, user.phone = req.phone));
			auto token = jwt::create();
			rep.token = token.set_issuer("alqaz").set_type("JWS").set_payload_claim("sample", jwt::claim(std::string("test")))
				.sign(jwt::algorithm::hs256{ "key" });
		}
		rep.user = req.username;
		return rep;
	}

	LoginRep CommonDBProcess::slogin(LoginReq req)
	{
		LoginRep rep;
		rep.username = req.username;
		DBInst db;
		dbCommon::Users user;
		user.name = req.username;
		bool bchecked = false;
		if (!req.username.empty() && !req.phone.empty())
		{
			req.password = req.password;// _sha256(req.phone + req.password);
			auto have = (*db)(select(count(user.name)).from(user).where(user.name == req.username and user.passwd == req.password and user.phone == req.phone)).front().count.value();
			rep.username = req.username;
		}
		else if (!req.username.empty())
		{
			for (const auto& row : (*db)(select(all_of(user)).from(user).where(user.name == req.username and user.passwd == req.password)))
			{

				rep.username = row.name;
				bchecked = true;
				break;
			}
		}
		else if (!req.phone.empty())
		{
			req.password = req.password;// _sha256(req.phone + req.password);
			for (const auto& row : (*db)(select(all_of(user)).from(user).where(user.phone == req.phone and user.passwd == req.password)))
			{

				rep.username = row.name;
				bchecked = true;
				break;
			}
		}
		else
		{
			rep.code = to_underlying(statusCode::invalidUserOrPhone);
			SPDLOG_WARN("no username or phone in login");
			return rep;
		}
		if (!bchecked)
		{
			SPDLOG_WARN("validate user, phone or pass failed");
			rep.code = to_underlying(statusCode::wrongUserOrPhoneOrPass);
		}
		else
		{
			auto token = jwt::create();
			rep.token = SMNetwork::getjwttoken();
		}
		SPDLOG_INFO("finish send query to database");
		return rep;
	}

	getAllUserRep CommonDBProcess::sgetAllUsers(getAllUserReq req)
	{
		getAllUserRep rep;
		DBInst db;
		dbCommon::Users user;
		user.name = "";
		for (const auto& row : (*db)(select(all_of(user)).from(user).unconditionally()))
		{
			userItemRep u;
			u.user = row.name;
			u.phone = row.phone;
			rep.users.push_back(u);
		}
		return rep;
	}

	addSysAdminRep CommonDBProcess::saddSysAdmin(addSysAdminReq req)
	{
		addSysAdminRep rep;
		DBInst db;
		dbCommon::Sysadmin user;
		user.admin = "";
		for (const auto& i : req.admins)
		{
			auto have = (*db)(select(count(user.admin)).from(user).where(user.user == req.user and user.phone == req.phone and user.admin == i.name and user.role == (int)(i.role))).front().count.value();
			if (have == 0)
			{
				(*db)(insert_into(user).set(user.user = req.user, user.phone = req.phone, user.admin = i.name, user.role = (int)(i.role)));
			}
		}
		return rep;
	}

	getSysAdminRep CommonDBProcess::sgetSysAdmin(getSysAdminReq req)
	{
		getSysAdminRep rep;
		DBInst db;
		dbCommon::Sysadmin user;
		user.admin = "";
		for (const auto& row : (*db)(select(all_of(user)).from(user).where(user.user == req.user and user.phone == req.phone)))
		{
			sysAdminItem u;
			u.name = row.admin;
			u.role = (adminRole)row.role.value();
			rep.admins.push_back(u);
		}
		return rep;
	}

	delSysAdminRep CommonDBProcess::sdelSysAdmin(delSysAdminReq req)
	{
		delSysAdminRep rep;
		DBInst db;
		dbCommon::Sysadmin user;
		user.admin = "";
		for (const auto& i : req.admins)
		{
			(*db)(remove_from(user).where(user.user == req.user and user.phone == req.phone and user.admin == i.name and user.role == (int)i.role));
		}
		return rep;
	}

	getUserGenInfoRep CommonDBProcess::sgetUserGenInfo(getUserGenInfoReq req)
	{
		getUserGenInfoRep rep;
		DBInst db;
		dbCommon::Users user;
		user.name = "";
		if (req.fromdb != 0) //from db
		{
			auto sss = sqlpp::dynamic_select(*db).dynamic_columns(user.name, user.phone, user.expiredate).from(user);

			auto s = sss.dynamic_where();
			if (!req.user.empty())
			{
				s.where.add(user.name == req.user);
			}
			if (!req.phone.empty())
			{
				s.where.add(user.phone == req.phone);
			}
			{
				lock_guard<mutex> lock(_genmut);
				for (const auto& row : (*db)(s))
				{
					UserGenInfoItem i;
					i.user = row.name;
					i.phone = row.phone;
					i.expire = row.expiredate.value().time_since_epoch().count();
					_genUsers[i.phone].clear();
					_genUsers[i.phone][i.user] = i;
				}
			}
		}
		else
		{
			lock_guard<mutex> lock(_genmut);
			auto it1 = _genUsers.find(req.phone);
			if (it1 != _genUsers.end())
			{
				if (!req.user.empty())
				{
					auto it2 = it1->second.find(req.user);
					if (it2 != it1->second.end())
					{
						rep.users.push_back(it2->second);
					}
					else
					{
						rep.code = 20001;
					}
				}
				for (auto& it2 : it1->second)
				{
					rep.users.push_back(it2.second);
				}
			}
			else
			{
				rep.code = 20002;
			}
		}
		return rep;
	}

	void CommonDBProcess::init(ServeMode mode)
	{
		BEGIN_STD;
		getUserGenInfoReq req;
		req.fromdb = 1;
		sgetUserGenInfo(req);
		END_STD;
	}

}
