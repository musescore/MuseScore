#ifndef DETO_ASYNC_CHANNEL_H
#define DETO_ASYNC_CHANNEL_H

#include <memory>
#include "internal/abstractinvoker.h"

namespace deto {
namespace async {
template<typename T>
class Channel
{
public:
    Channel() = default;
    Channel(const Channel& ch)
        : m_ptr(ch.ptr()) {}
    ~Channel() {}

    Channel& operator=(const Channel& ch)
    {
        if (m_ptr == ch.ptr()) {
            return *this;
        }

        m_ptr = ch.ptr();
        return *this;
    }

    void send(const T& d)
    {
        NotifyData nd;
        nd.setArg<T>(0, d);
        ptr()->invoke(Receive, nd);
    }

    template<typename Func>
    void onReceive(const Asyncable* receiver, Func f, Asyncable::AsyncMode mode = Asyncable::AsyncSetOnce)
    {
        ptr()->setCallBack(Receive, const_cast<Asyncable*>(receiver), new ReceiveCall<Func, T>(f), mode);
    }

    void resetOnReceive(const Asyncable* receiver)
    {
        ptr()->removeCallBack(Receive, const_cast<Asyncable*>(receiver));
    }

    void close()
    {
        ptr()->invoke(Close);
    }

    template<typename Func>
    void onClose(const Asyncable* receiver, Func f, Asyncable::AsyncMode mode = Asyncable::AsyncSetOnce)
    {
        ptr()->setCallBack(Close, const_cast<Asyncable*>(receiver), new CloseCall<Func>(f), mode);
    }

    bool isConnected() const
    {
        return ptr()->isConnected();
    }

private:

    enum CallType {
        Undefined = 0,
        Receive,
        Close
    };

    struct IReceive {
        virtual ~IReceive() {}
        virtual void received(const NotifyData& d) = 0;
    };

    template<typename Call, typename Arg>
    struct ReceiveCall : public IReceive {
        Call f;
        ReceiveCall(Call _f)
            : f(_f) {}
        void received(const NotifyData& d) { f(d.arg<Arg>()); }
    };

    struct IClose {
        virtual ~IClose() {}
        virtual void closed() = 0;
    };

    template<typename Call>
    struct CloseCall : public IClose {
        Call f;
        CloseCall(Call _f)
            : f(_f) {}
        void closed() { f(); }
    };

    struct ChannelInvoker : public AbstractInvoker
    {
        friend class Channel;

        ChannelInvoker() = default;
        ~ChannelInvoker()
        {
            removeAllCallBacks();
        }

        void deleteCall(int _type, void* call) override
        {
            CallType type = static_cast<CallType>(_type);
            switch (type) {
            case Undefined: {} break;
            case Receive: {
                delete static_cast<IReceive*>(call);
            } break;
            case Close: {
                delete static_cast<IClose*>(call);
            } break;
            }
        }

        void onInvoke(int callKey, const NotifyData& d) override
        {
            CallType type = static_cast<CallType>(callKey);
            std::vector<void*> cls = calls(callKey);
            for (void* c : cls) {
                switch (type) {
                case Undefined:  break;
                case Receive:
                    static_cast<IReceive*>(c)->received(d);
                    break;
                case Close:
                    static_cast<IClose*>(c)->closed();
                    break;
                }
            }
        }
    };

    std::shared_ptr<ChannelInvoker> ptr() const
    {
        if (!m_ptr) {
            m_ptr = std::make_shared<ChannelInvoker>();
        }
        return m_ptr;
    }

    mutable std::shared_ptr<ChannelInvoker> m_ptr = nullptr;
};
}
}

#endif // DETO_ASYNC_CHANNEL_H
