#ifndef DETO_ASYNC_ASYNC_H
#define DETO_ASYNC_ASYNC_H

#include "asyncable.h"
#include "internal/asyncimpl.h"

namespace deto {
namespace async {
class Async
{
public:

    template<typename F>
    static void call(const Asyncable* caller, F f, const std::thread::id& th = std::this_thread::get_id())
    {
        AsyncImpl::instance()->call(const_cast<Asyncable*>(caller), new AsyncImpl::Functor<F>(f), th);
    }

    template<typename F, typename Arg1>
    static void call(const Asyncable* caller, F f, Arg1 a1, const std::thread::id& th = std::this_thread::get_id())
    {
        AsyncImpl::instance()->call(const_cast<Asyncable*>(caller), new AsyncImpl::FunctorArg1<F, Arg1>(f, a1), th);
    }

    static void disconnectAsync(Asyncable* a)
    {
        AsyncImpl::instance()->disconnectAsync(a);
    }
};
}
}

#endif // DETO_ASYNC_ASYNC_H
