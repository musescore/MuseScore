#ifndef DETO_ASYNC_ASYNCIMPL_H
#define DETO_ASYNC_ASYNCIMPL_H

#include <mutex>
#include <map>
#include <thread>
#include "../asyncable.h"

namespace deto {
namespace async {
class AsyncImpl : public Asyncable::IConnectable
{
public:

    static AsyncImpl* instance();

    struct IFunction {
        virtual ~IFunction() {}
        virtual void call() = 0;
    };

    template<typename F>
    struct Functor : public IFunction {
        F functor;
        Functor(const F fn)
            : functor(fn) {}
        void call() { functor(); }
    };

    template<typename F, typename Arg1>
    struct FunctorArg1 : public IFunction {
        F functor;
        Arg1 arg1;
        FunctorArg1(const F fn, Arg1 a1)
            : functor(fn), arg1(a1) {}
        void call() { functor(arg1); }
    };

    void call(Asyncable* caller, IFunction* f, const std::thread::id& th = std::this_thread::get_id());
    void disconnectAsync(Asyncable* caller);

private:
    AsyncImpl() = default;

    void onCall(uint64_t key);

    struct Call {
        Asyncable* caller = nullptr;
        IFunction* f = nullptr;
        Call() {}
        Call(Asyncable* c, IFunction* _f)
            : caller(c), f(_f) {}
    };

    std::mutex m_mutex;
    std::map<uint64_t, Call> m_calls;
};
}
}

#endif // DETO_ASYNC_ASYNCIMPL_H
