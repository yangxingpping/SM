
#pragma once

#include "PackDealerCommon.h"
#include "Routers.h"
#include "hotupdate.h"
#include <span>


namespace SMNetwork
{
    template<AppPackUnpack App, PlatformPackUnpack Plat>
    class AppPlatformDealer
    {
    public:
        AppPlatformDealer(App app, Plat plat):_app(app), _plat(plat)
        {

        }
        asio::awaitable<shared_ptr<string>> dealMessage(shared_ptr<string> req)
        {
            auto funcs = SMCONF::getRouterTransByMainC(_plat.getMain());
            assert(funcs);
            _plat.unpack(string_view(*req));
            auto func = funcs->find(_plat.getAss());
            if (func == funcs->end())
            {
                co_return nullptr;
            }
			RouterFuncReturnType ret;
            string appmsg(req->data() + _plat.HeadLen(), req->length() - _plat.HeadLen());
			string token;
			BEGIN_ASIO;
			ret = co_await(*(func->second))(appmsg, token);
			END_ASIO;
			SPDLOG_INFO("send to client data: [{}], size:[{}]", *ret, ret->length());
			if (ret->empty())
			{
				SPDLOG_ERROR("  mainc = {}, assc = {} response 0", magic_enum::enum_name(_plat.getMain()), _plat.getAss());
				co_return nullptr;
			}
            ret->reserve(ret->length() + _plat.HeadLen());
            _plat.pack(span(ret->data() + ret->length(), _plat.HeadLen()));
            co_return ret;
        }
    private:
        App _app;
        Plat _plat;
    };

    class NETWORKINTERFACE_EXPORT PackDealerMainSub : public PackDealerCommon
    {
    public:
        PackDealerMainSub(MainCmd mainc, ChannelType channel);
        PackDealerMainSub(const PackDealerMainSub& c);
        virtual void setAssc(int) override;
        virtual int getAssc() override;
        virtual MainCmd getMainc() override;
		
		virtual std::string* unpack(seqNumType* seqnum, string& pack)  override;
        virtual std::string* unpack(seqNumType* seqnum, std::string_view pack) override;
        virtual std::string* unpack(std::string_view pack);
		virtual std::string* pack(seqNumType* seqnum, std::string_view msg)  override;
        virtual std::string* pack(seqNumType* seqnum, std::string_view msg, uint16_t assc) override;
        virtual std::string* pack(std::string_view msg) override;
        virtual asio::awaitable<std::string*> dealmsg(string& msg) override;
        virtual PackDealerBase* clone() override;
        
    private:
        int _assc;
        MainCmd _mainc;  
    };
}
