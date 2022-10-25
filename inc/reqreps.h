
#pragma once

#include "spdlog/spdlog.h"

#include "iguana/json.hpp"

#include "enums.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

enum class adminRole
{
    Viewer,
    All,
};

struct EchoReq
{
    string username;
    string password;
    string token;
};
REFLECTION(EchoReq, username, password, token);

struct RegistReq
{
    string username;
    string password;
	string phone;
	string token;
};
REFLECTION(RegistReq, username, password, phone,token);

struct LoginReq
{
    string username;
    string phone;
	string password;
	string token;
};
REFLECTION(LoginReq, username, password, phone, token);

struct GetUserInfoReq
{
    string token;
};
REFLECTION(GetUserInfoReq, token);

struct getAllUserReq
{
    string token;
};
REFLECTION(getAllUserReq, token);

struct EchoRep
{
    int code = to_underlying(statusCode::ok);
};
REFLECTION(EchoRep, code)



struct RegistRep
{
    int code = to_underlying(statusCode::ok);
    string user;
    string token;
};
REFLECTION(RegistRep, code, user, token);

struct LoginRep
{
    int code = to_underlying(statusCode::ok);
    string username;
    uint64_t _logincount=0;
    string token;
};
REFLECTION(LoginRep, code, username, _logincount, token)

struct GetUserInfoRep
{
    int code = to_underlying(statusCode::ok);
    string user;
    string roles;
    string avatar;
};
REFLECTION(GetUserInfoRep, code, user, roles, avatar)

struct userItemRep
{
    string user;
    string phone;
};
REFLECTION(userItemRep, user, phone);

struct getAllUserRep
{
    int code = to_underlying(statusCode::ok);
    vector<userItemRep> users;
};
REFLECTION(getAllUserRep, code, users);

struct sysAdminItem
{
    string name;
    adminRole role;
};
REFLECTION(sysAdminItem, name, role);

struct addSysAdminReq
{
    string user;
    string phone;
    string token;
    vector<sysAdminItem> admins;
};
REFLECTION(addSysAdminReq, user, phone, token, admins);

struct addSysAdminRep
{
    int code = to_underlying(statusCode::ok);
};
REFLECTION(addSysAdminRep, code);

struct delSysAdminReq
{
	string user;
	string phone;
	string token;
	vector<sysAdminItem> admins;
};
REFLECTION(delSysAdminReq, user, phone, token, admins);

struct delSysAdminRep
{
	int code = to_underlying(statusCode::ok);
};
REFLECTION(delSysAdminRep, code);

struct getSysAdminReq
{
    string user;
    string phone;
    string token;
};
REFLECTION(getSysAdminReq, user, phone, token);

struct getSysAdminRep
{
    int code = to_underlying(statusCode::ok);
	string user;
	string phone;
    vector<sysAdminItem> admins;
};
REFLECTION(getSysAdminRep, code, user, phone, admins);

struct getUserGenInfoReq
{
    int fromdb = 0;
    string user;
    string phone;
    string token;
};
REFLECTION(getUserGenInfoReq, fromdb, user, phone, token);

struct UserGenInfoItem
{
    string user;
    string phone;
    int expire;
    int logincount;
};
REFLECTION(UserGenInfoItem, user, phone, expire, logincount);


struct getUserGenInfoRep
{
    int code = to_underlying(statusCode::ok);
    vector<UserGenInfoItem> users;
};
REFLECTION(getUserGenInfoRep, code, users);

struct invalidJSONRep
{
    int code = to_underlying(statusCode::invalidJson);
};
REFLECTION(invalidJSONRep, code);

struct jwtCheckFailedJSONRep
{
	int code = to_underlying(statusCode::jwtCheckFailed);
};
REFLECTION(jwtCheckFailedJSONRep, code);
