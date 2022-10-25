#pragma once

#include <stdint.h>
#include <type_traits>

//https://stackoverflow.com/questions/8357240/how-to-automatically-convert-strongly-typed-enum-into-int
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept 
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

enum class ChannelIsInitiative
{
    Initiative,
    Passive,
};

enum class HtmlBodyType
{
    Json,
    Html,
};

enum class ChannelType 
{
    EchoServer,
    EchoClient,
    TestClient,
    TestServer,
    DBServer,
    DBClient,
    RDBServer,
    RDBClient,
    PLCBegin,
    PCLConnect,
    PLCBind,
    PLCEnd,
};

enum class NngSockImplType
{
    NormalSock,
    RawSock,
};

enum class ChannelImplType 
{
    HttpGet,
    HttpPost,
    WebSocket,
    NNG,
    Max,
};

enum class NNgTransType 
{
    TCP,
    WS,
    Max,
};

enum class RepCode 
{
    RepOk,
    InvalidLen,
    InvalidCmd,
    Max,
};

/**
 * this is used for network layer
 */
enum class NetHeadType
{
    FixPackLenPlaceHolder,  //net pack first 4 bytes is pack length
    NoPlaceHolderFixLen,    //net pack is started with fixed length payload
    NoPlaceHolderVarLen,    //net pack is started with variable length payload
};

/**
 * this is used for protocol layer 
 */
enum class PackType 
{
    NoHead,
    FixMainSubHead,
    FixMainHead,
    FixSubHead,
    Max,
};

enum class MainCmd 
{
    Invalid = 1,
    Echo,
    User,
    PlaceMsg,
    DBQuery,
    RDBQuery,

    PLCBegin,
    PLCReq,
    PLCResp,
    DealPLC,
    DealOdd,
    PLCEnd,

    R8Begin,
    UploadRecord,
    PayOrders,
    CategryMenus,
    GroupKeywords,
    GetRecord,
    R8End,
  
};

enum class AssDB 
{
    Regist=1,
    Login,
    Logout,
    AddUser,
    UserInfo,
    GetUserGenInfo,
    getAllUsers,
	AddSysAdmin,
	GetSysAdmin,
	DelSysAdmin,
    DBEcho,

    ShopBegin,
    GetShopTime,
    GetOrderList,
    GetFoodList,
    GetCutList,
    WxPay,
    ShopEnd,

    RedisBegin,
    getGroupKeyword,
    setGroupKeyword,
    RedisEnd,

    PLCBegin,
    GetPLCWarns,
    GetDiscInfo,
    GetPLCType,
    TriggerWarn,
    SetOddError,
    GetOddError,
    getOddList,
    PLCEnd,

    R8Begin,
    addGroup,
    removeGroup,
    getGroup,
    addCategy,
    removeCategy,
    getCategy,
    addMenu,
    updatMenu,
    removeMenu,
    upLoadOrderRecord,
    upLoadPayRecord,
    getOrder,
    getGreetState,
    setGreetState,
    getMenu,
    updateMenuCategy,
    deleteMenuCategy,
    getOrderPrint,
    getDayProfile,
	R8End,

    Max,
};

enum class AssAuth
{
    Invalid,
    Regist = 1,
    Login = 2,
    Logout,
    AddUser,
    UserInfo,
    GetUserGenInfo,
    AddSysAdmin,
    GetSysAdmin,
    DelSysAdmin,
    Max,
};
enum class AssPlace 
{
    Invalid,
    PlaceInvite = 1,
    PlaceSendMsg = 2,
    PlaceUserList = 3,
    TransationList = 4,
};

enum class AssEcho 
{
    Echo,
    EchoReq,
    EchoRep,
    Max,
};

class nethead
{
public:
    uint32_t len; //pack exclude nethead length
};

class packhead
{
public:
    uint16_t mainc;
    uint16_t assc;
    uint32_t len;   //pack exclude mainc and assc length
};

class cmdhead
{
public:
    MainCmd mainc = MainCmd::Invalid;
    uint16_t assc = 0;
    uint32_t len = 0;
};

class cmdsubhead
{
public:
    uint16_t assc = 0;
    uint32_t len = 0;
};

enum class statusCode
{
    ok = 20000,
    invalidUserOrPhone = 30000,
    jwtCheckFailed,
    registNameRepeat,
    wrongUserOrPhoneOrPass,
    invalidJson=40000,
    networkError,
    MaxCommStatusCode=80000,
};
