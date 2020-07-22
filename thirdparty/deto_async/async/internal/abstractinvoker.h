#ifndef DETO_ASYNC_ABSTRACTINVOKER_H
#define DETO_ASYNC_ABSTRACTINVOKER_H

#include <memory>
#include <vector>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>

#include "../asyncable.h"

namespace deto {
namespace async {
class NotifyData
{
public:
    NotifyData() {}

    template<typename T>
    void setArg(int i, const T& val)
    {
        IArg* p = new Arg<T>(val);
        m_args.insert(m_args.begin() + i, std::shared_ptr<IArg>(p));
    }

    template<typename T>
    T arg(int i = 0) const
    {
        IArg* p = m_args.at(i).get();
        if (!p) {
            return T();
        }
        Arg<T>* d = reinterpret_cast<Arg<T>*>(p);
        return d->val;
    }

    struct IArg {
        virtual ~IArg() = default;
    };

    template<typename T>
    struct Arg : public IArg {
        T val;
        Arg(const T& v)
            : IArg(), val(v) {}
    };

private:
    std::vector<std::shared_ptr<IArg> > m_args;
};

class QtQueuedInvoker;
class AbstractInvoker : public Asyncable::IConnectable
{
public:
    void disconnectAsync(Asyncable* receiver);

    void invoke(int callKey);
    void invoke(int callKey, const NotifyData& data);

    bool isConnected() const;

protected:
    explicit AbstractInvoker();
    ~AbstractInvoker();

    virtual void deleteCall(int type, void* call) = 0;
    virtual void onInvoke(int callKey, const NotifyData& data) = 0;

    void doInvoke(int callKey, int dataKey);

    struct CallBack {
        int type = 0;
        Asyncable* receiver = nullptr;
        void* call = nullptr;
        CallBack() {}
        CallBack(int t, Asyncable* cr, void* c)
            : type(t), receiver(cr), call(c) {}
    };

    class CallBacks : public std::vector<CallBack>
    {
    public:
        int receiverIndexOf(Asyncable* receiver) const;
        bool containsreceiver(Asyncable* receiver) const;
    };

    void setCallBack(int type, Asyncable* receiver, void* call, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetRepeat);
    void removeCallBack(int type, Asyncable* receiver);
    void removeAllCallBacks();
    std::vector<void*> calls(int type) const;

    int newKey();
    void invokeMethod(int callKey, const NotifyData& data);
    void pushData(int key, const NotifyData& e);
    NotifyData popData(int key);

    std::map<int /*type*/, CallBacks > m_callbacks;

    int m_key = 0;

    std::thread::id m_threadID;
    std::mutex m_mutex;
    std::map<int, NotifyData> m_data;
    QtQueuedInvoker* m_queuedInvoker = nullptr;
};
}
}

#endif // DETO_ASYNC_ABSTRACTINVOKER_H
