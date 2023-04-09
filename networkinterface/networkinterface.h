#pragma once
#include "networkinterfaceExport.h"
#include "magic_enum.hpp"
#include "templatefuncs.h"
#include "asio/awaitable.hpp"
#include "fmt/format.h"
#include "Utils.h"
#include "oneshot.hpp"
#include "NMessage.h"
#include "PlatformPackInterface.h"
#include "myconcept.h"
#include "tsl/htrie_map.h"
#include <map>
#include <string>
#include <stdbool.h>
#include <functional>
#include <tuple>
#include <sstream>
#include <list>

using std::map;
using std::string;
using std::shared_ptr;
using std::tuple;
using std::make_tuple;
using tsl::htrie_map;
using std::list;
using std::move;

typedef std::function<asio::awaitable<RouterFuncReturnType>(string&, string)> RouterFuncType;
typedef shared_ptr<RouterFuncType> ConRouterType;
typedef map<string, list<ConRouterType>> RoutersType;
typedef RoutersType* PtRoutersType;
typedef RoutersType& RefRoutersType;
typedef htrie_map<char, ConRouterType> CodeRoutersType;

typedef map<HtmlBodyType, RoutersType>* PtHtmlRoutersType;

typedef map<int, ConRouterType> TransRouterElement;
typedef TransRouterElement& RefTransRouterElement;
typedef TransRouterElement* PtTransRouterElement;

typedef map<int, TransRouterElement> TransRoutersType;

namespace SMNetwork
{
	class PrepareReqRepRecver
	{
	public:
		PrepareReqRepRecver(oneshot::receiver<bool> r1, oneshot::receiver<shared_ptr<NMessage>> r2)
			:_r1(move(r1))
			, _r2(move(r2))
			, _valid(false)
		{

		}
		PrepareReqRepRecver(PrepareReqRepRecver& ref)
			:_r1(move(ref._r1))
			, _r2(move(ref._r2))
			, _valid(ref._valid)
		{
			ref._valid = false;
		}
		PrepareReqRepRecver(PrepareReqRepRecver&& right)
			:_r1(move(right._r1))
			, _r2(move(right._r2))
			, _valid(right._valid)
		{
			right._valid = false;
		}
		PrepareReqRepRecver()
			:_valid(false)
		{

		}

		oneshot::receiver<bool> _r1;
		oneshot::receiver<shared_ptr<NMessage>> _r2;
		bool _valid{ false };
	};

    NETWORKINTERFACE_EXPORT void initjwtconfig(string_view issuer = "alqaz", string_view type = "JWS", string_view key = "key", uint32_t expiresecond = 3600);
    NETWORKINTERFACE_EXPORT void initjwtconfig(JWTConf& jwtconf);
    NETWORKINTERFACE_EXPORT bool isjwttokenright(string_view token);
    NETWORKINTERFACE_EXPORT string getjwttoken();
    NETWORKINTERFACE_EXPORT bool initNetwork();
	NETWORKINTERFACE_EXPORT void uninitNetwork();

	NETWORKINTERFACE_EXPORT PrepareReqRepRecver prepareReqRep(uint32_t sock, uint32_t msgno);
	NETWORKINTERFACE_EXPORT bool FinishReqRepRecver(uint32_t sock, uint32_t msgno);

	NETWORKINTERFACE_EXPORT bool prepareSignalRecvNetRep(uint32_t sock, uint32_t msgno);

	NETWORKINTERFACE_EXPORT bool notifySendNetComplete(uint32_t sock, uint32_t msgno, bool flag);
	NETWORKINTERFACE_EXPORT bool notifyNetRepComplete(uint32_t sock, shared_ptr<NMessage> msg);
	NETWORKINTERFACE_EXPORT shared_ptr<oneshot::receiver<shared_ptr<NMessage>>> prepareAppRep(uint32_t sock, uint32_t msgno);
	NETWORKINTERFACE_EXPORT shared_ptr<oneshot::receiver<shared_ptr<NMessage>>> prepareNetRep(uint32_t sock, uint32_t msgno);

	NETWORKINTERFACE_EXPORT asio::awaitable<bool> addPackFromIo(uint32_t sock, int mainc, shared_ptr<NMessage> msg);

	NETWORKINTERFACE_EXPORT asio::awaitable<bool> addPackFromApp(uint32_t sock, shared_ptr<NMessage> msg);

	NETWORKINTERFACE_EXPORT uint32_t newSockNo();

	NETWORKINTERFACE_EXPORT bool addPlatformDealer(shared_ptr<PlatformPackInterface> pack);
	NETWORKINTERFACE_EXPORT shared_ptr<PlatformPackInterface> clonePlatformPack(int mainc);

template <class MainC, class AssC>
tuple<string, string> combinePath(MainC mainc, AssC assc)
{
    auto v = SMUtils::packheads(magic_enum::enum_integer(mainc), magic_enum::enum_integer(assc));

    return make_tuple<string, string>(std::move(v), fmt::format("/{}/{}", magic_enum::enum_name(mainc), magic_enum::enum_name(assc)));
}

}
