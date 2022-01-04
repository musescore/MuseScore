#ifndef DETO_ASYNC_ABSTRACTINVOKER_H
#define DETO_ASYNC_ABSTRACTINVOKER_H

#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <functional>

#include "../asyncable.h"

namespace deto {
namespace async {
class NotifyData
{
public:
    NotifyData() {}

    template<typename ... T>
    void setArg(int i, const T&... val)
    {
        IArg* p = new Arg<T...>(val ...);
        m_args.insert(m_args.begin() + i, std::shared_ptr<IArg>(p));
    }

    template<typename ... T>
    std::tuple<T...> arg(int i = 0) const
    {
        IArg* p = m_args.at(i).get();
        if (!p) {
            return {};
        }
        Arg<T...>* d = reinterpret_cast<Arg<T...>*>(p);
        return d->val;
    }

    struct IArg {
        virtual ~IArg() = default;
    };

    template<typename ... T>
    struct Arg : public IArg {
        std::tuple<T...> val;
        Arg(const T&... v)
            : IArg(), val(v ...) {}
    };

private:
    std::vector<std::shared_ptr<IArg> > m_args;
};

class QueuedInvoker;
class AbstractInvoker : public Asyncable::IConnectable
{
public:
    void disconnectAsync(Asyncable* receiver);

    void invoke(int type);
    void invoke(int type, const NotifyData& data);

    bool isConnected() const;

    static void processEvents();
    static void onMainThreadInvoke(const std::function<void(const std::function<void()>&, bool)>& f);

protected:
    explicit AbstractInvoker();
    ~AbstractInvoker();

    virtual void deleteCall(int type, void* call) = 0;
    virtual void doInvoke(int type, void* call, const NotifyData& data) = 0;

    struct CallBack {
        std::thread::id threadID;
        int type = 0;
        Asyncable* receiver = nullptr;
        void* call = nullptr;
        CallBack() {}
        CallBack(std::thread::id threadID, int t, Asyncable* cr, void* c)
            : threadID(threadID), type(t), receiver(cr), call(c) {}
    };

    class CallBacks : public std::vector<CallBack>
    {
    public:
        int receiverIndexOf(Asyncable* receiver) const;
        bool containsReceiver(Asyncable* receiver) const;
    };

    struct QInvoker
    {
        std::mutex mutex;
        AbstractInvoker* invoker = nullptr;
        int type = -1;
        CallBack call;
        NotifyData data;

        QInvoker(AbstractInvoker* i, int t, CallBack c, NotifyData d)
            : invoker(i), type(t), call(c), data(d)
        {
            invoker->addQInvoker(this);
        }

        ~QInvoker()
        {
            if (invoker) {
                invoker->removeQInvoker(this);
            }
        }

        void invoke()
        {
            AbstractInvoker* inv = nullptr;
            {
                std::lock_guard<std::mutex> lock(mutex);
                inv = invoker;
            }

            if (inv) {
                inv->invokeCallback(type, call, data);
            }
        }

        void invalidate()
        {
            std::lock_guard<std::mutex> lock(mutex);
            invoker = nullptr;
        }
    };

    void invokeCallback(int type, const CallBack& c, const NotifyData& data);

    void addCallBack(int type, Asyncable* receiver, void* call, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetRepeat);
    void removeCallBack(int type, Asyncable* receiver);
    void removeAllCallBacks();

    void addQInvoker(QInvoker* qi);
    void removeQInvoker(QInvoker* qi);

    std::map<int /*type*/, CallBacks > m_callbacks;

    std::mutex m_qInvokersMutex;
    std::list<QInvoker*> m_qInvokers;
};

inline void processEvents()
{
    AbstractInvoker::processEvents();
}

inline void onMainThreadInvoke(const std::function<void(const std::function<void()>&, bool)>& f)
{
    AbstractInvoker::onMainThreadInvoke(f);
}
}
}

#endif // DETO_ASYNC_ABSTRACTINVOKER_H
