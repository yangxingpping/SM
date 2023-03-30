#pragma once
#include "networkinterfaceExport.h"
#include "TimeoutMessage.h"
#include "oneshot.hpp"
#include <map>
#include <memory>
#include <string>
#include <stdint.h>
#include <list>

using std::string;
using std::unique_ptr;
using std::move;
using std::shared_ptr;
using std::list;
using std::map;
using std::pair;

using SMHotupdate::ETrigeType;

namespace SMNetwork
{
    class NETWORKINTERFACE_EXPORT SendRecvPacks
    {
    public:
        SendRecvPacks();
        SendRecvPacks(const SendRecvPacks& copy) = delete;

        asio::awaitable<shared_ptr<TimeoutMessage>> getFirstSend();

        //called from io
        asio::awaitable<shared_ptr<TimeoutMessage>> popNeeSend();
        void pushRecv(uint32_t no, shared_ptr<string> msg);

        void addPending(shared_ptr<TimeoutMessage> msg);

        void addSendingMessage(shared_ptr<TimeoutMessage> msg);

        asio::awaitable<shared_ptr<TimeoutMessage>> popPending(uint32_t no = 0);

        //called by coro from app
        void pushSend(shared_ptr<TimeoutMessage> msg);
        asio::awaitable <shared_ptr<TimeoutMessage>> popRecv();

        void notifyMesssageSent(uint32_t no);

        asio::awaitable<bool> waitPendingFinish(uint32_t no);

        asio::awaitable<shared_ptr<string>> waitRep(uint32_t no);

        shared_ptr<string> getPendingMessage(uint32_t no);

        bool isWait(uint32_t no);

        //follow 5 function called in app io
        bool initForReqRep(uint32_t no);
        bool initWaitSent(uint32_t no);
        asio::awaitable<bool> waitMessageSent(uint32_t no);
        bool initWaitRecv(uint32_t no);
        asio::awaitable<shared_ptr<NMessage>> waitMessageRecv(uint32_t no);
       
    private:
		map<uint32_t,shared_ptr<TimeoutMessage>> _sendbufs;
        map<uint32_t, shared_ptr<TimeoutMessage>> _needSendBufs;
        map<uint32_t, shared_ptr<TimeoutMessage>> _sendingBufs;
        map<uint32_t, shared_ptr<TimeoutMessage>> _waittingRepBufs;
        map<uint32_t, shared_ptr<TimeoutMessage>> _pendingbufs;
		map<uint32_t, shared_ptr<TimeoutMessage>> _recvbufs;
        shared_ptr<SMHotupdate::CoEvent> _needSendNotify{nullptr};
        shared_ptr<SMHotupdate::CoEvent> _needRecvNotify{nullptr};
        shared_ptr<SMHotupdate::CoEvent> _pendingNotify{nullptr};

        oneshot::sender<void> _notifyNeedSend;
        oneshot::receiver<void> _waitNeedSend;

        oneshot::sender<void> _notifyRecv;
        oneshot::receiver<void> _waitRecv;

        map < uint32_t, oneshot::sender<bool>> _swaitSent;
        map < uint32_t, oneshot::receiver<bool>> _rwaitSent;
        map < uint32_t, oneshot::sender<shared_ptr<NMessage>>> _swaitRecv;
        map < uint32_t, oneshot::receiver<shared_ptr<NMessage>>> _rwaitRecv;
    };
} // namespace SMNetwork

