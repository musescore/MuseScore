#include "asyncimpl.h"

#include "queuedinvoker.h"

using namespace deto::async;

AsyncImpl* AsyncImpl::instance()
{
    static AsyncImpl a;
    return &a;
}

void AsyncImpl::disconnectAsync(Asyncable* caller)
{
    std::lock_guard locker(m_mutex);
    uint64_t key = 0;
    std::map<uint64_t, Call>::const_iterator it = m_calls.cbegin(), end = m_calls.cend();
    for (; it != end; ++it) {
        if (it->second.caller == caller) {
            key = it->first;
            break;
        }
    }

    if (key) {
        m_calls.erase(key);
    }
}

void AsyncImpl::call(Asyncable* caller, IFunction* f, const std::thread::id& th)
{
    if (caller) {
        caller->connectAsync(this);
    }

    uint64_t key = reinterpret_cast<uint64_t>(f);
    {
        std::lock_guard locker(m_mutex);
        m_calls[key] = Call(caller, f);
    }

    auto functor = [this, key]() { onCall(key); };
    QueuedInvoker::instance()->invoke(th, functor, true);
}

void AsyncImpl::onCall(uint64_t key)
{
    Call c;
    {
        std::lock_guard locker(m_mutex);
        auto it = m_calls.find(key);

        //! NOTE Probably disconnected
        if (it == m_calls.end()) {
            return;
        }

        c = it->second;
        m_calls.erase(it);
    }

    c.f->call();

    if (c.caller) {
        c.caller->disconnectAsync(this);
    }

    delete c.f;
}
