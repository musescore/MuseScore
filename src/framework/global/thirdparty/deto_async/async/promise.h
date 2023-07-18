#ifndef DETO_ASYNC_PROMISE_H
#define DETO_ASYNC_PROMISE_H

#include <memory>
#include <string>

#include <QtCore/qcompilerdetection.h>

#include "internal/abstractinvoker.h"
#include "async.h"

namespace deto {
namespace async {
template<typename ... T>
class Promise;
template<typename ... T>
class Promise
{
public:
    // Dummy struct, with the purpose to enforce that the body
    // of a Promise resolves OR rejects exactly once
    struct Result {
        static Result unchecked()
        {
            return {};
        }

    private:
        Result() = default;

        friend struct Resolve;
        friend struct Reject;
    };

    struct Resolve
    {
        Resolve(Promise<T...> _p)
            : p(_p) {}

        Q_REQUIRED_RESULT
        Result operator ()(const T& ... val) const
        {
            p.resolve(val ...);
            return {};
        }

    private:
        mutable Promise<T...> p;
    };

    struct Reject
    {
        Reject(Promise<T...> _p)
            : p(_p) {}

        Q_REQUIRED_RESULT
        Result operator ()(int code, const std::string& msg) const
        {
            p.reject(code, msg);
            return {};
        }

    private:
        mutable Promise<T...> p;
    };

    enum class AsynchronyType {
        ProvidedByPromise,
        ProvidedByBody
    };

    using Body = std::function<Result(Resolve, Reject)>;

    Promise(Body body, AsynchronyType type)
    {
        Resolve res(*this);
        Reject rej(*this);

        switch (type) {
        case AsynchronyType::ProvidedByPromise:
            Async::call(nullptr, [res, rej](Body body) mutable {
                body(res, rej);
            }, body);
            break;

        case AsynchronyType::ProvidedByBody:
            body(res, rej);
            break;
        }
    }

    Promise(Body body, const std::thread::id& th = std::this_thread::get_id())
    {
        Resolve res(*this);
        Reject rej(*this);

        Async::call(nullptr, [res, rej](Body body) mutable {
            body(res, rej);
        }, body, th);
    }

    Promise(const Promise& p)
        : m_ptr(p.ptr()) {}

    ~Promise() {}

    Promise& operator=(const Promise& p)
    {
        if (m_ptr == p.ptr()) {
            return *this;
        }

        m_ptr = p.ptr();
        return *this;
    }

    template<typename Call>
    Promise<T...>& onResolve(const Asyncable* caller, Call f)
    {
        ptr()->addCallBack(OnResolve, const_cast<Asyncable*>(caller), new ResolveCall<Call, T...>(f));
        return *this;
    }

    template<typename Call>
    Promise<T...>& onReject(const Asyncable* caller, Call f)
    {
        ptr()->addCallBack(OnReject, const_cast<Asyncable*>(caller), new RejectCall<Call>(f));
        return *this;
    }

private:
    Promise() = default;

    void resolve(const T& ... d)
    {
        NotifyData nd;
        nd.setArg<T...>(0, d ...);
        nd.setArg<std::shared_ptr<PromiseInvoker> >(1, ptr());
        ptr()->invoke(OnResolve, nd);
    }

    void reject(int code, const std::string& msg)
    {
        NotifyData nd;
        nd.setArg<int>(0, code);
        nd.setArg<std::string>(1, msg);
        nd.setArg<std::shared_ptr<PromiseInvoker> >(2, ptr());
        ptr()->invoke(OnReject, nd);
    }

    enum CallType {
        Undefined = 0,
        OnResolve,
        OnReject
    };

    struct IResolve {
        virtual ~IResolve() {}
        virtual void resolved(const NotifyData& e) = 0;
    };

    template<typename Call, typename ... Arg>
    struct ResolveCall : public IResolve {
        Call f;
        ResolveCall(Call _f)
            : f(_f) {}
        void resolved(const NotifyData& e) { std::apply(f, e.args<Arg...>()); }
    };

    struct IReject {
        virtual ~IReject() {}
        virtual void rejected(const NotifyData& e) = 0;
    };

    template<typename Call>
    struct RejectCall : public IReject {
        Call f;
        RejectCall(Call _f)
            : f(_f) {}
        void rejected(const NotifyData& e) { f(e.arg<int>(0), e.arg<std::string>(1)); }
    };

    struct PromiseInvoker : public AbstractInvoker
    {
        friend class Promise;

        PromiseInvoker() = default;
        ~PromiseInvoker()
        {
            removeAllCallBacks();
        }

        void deleteCall(int _type, void* call) override
        {
            CallType type = static_cast<CallType>(_type);
            switch (type) {
            case Undefined: {} break;
            case OnResolve: {
                delete static_cast<IResolve*>(call);
            } break;
            case OnReject: {
                delete static_cast<IReject*>(call);
            } break;
            }
        }

        void doInvoke(int callKey, void* call, const NotifyData& d) override
        {
            CallType type = static_cast<CallType>(callKey);
            switch (type) {
            case Undefined:  break;
            case OnResolve:
                static_cast<IResolve*>(call)->resolved(d);
                break;
            case OnReject:
                static_cast<IReject*>(call)->rejected(d);
                break;
            }
        }
    };

    std::shared_ptr<PromiseInvoker> ptr() const
    {
        if (!m_ptr) {
            m_ptr = std::make_shared<PromiseInvoker>();
        }
        return m_ptr;
    }

    mutable std::shared_ptr<PromiseInvoker> m_ptr = nullptr;
};
}
}

#endif // DETO_ASYNC_PROMISE_H
