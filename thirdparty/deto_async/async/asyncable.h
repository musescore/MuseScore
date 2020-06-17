#ifndef DETO_ASYNC_ASYNCABLE_H
#define DETO_ASYNC_ASYNCABLE_H

#include <set>
#include <cstdint>

namespace deto {
namespace async {
class Asyncable
{
public:

    enum AsyncMode {
        AsyncSetOnce = 0,
        AsyncSetRepeat
    };

    virtual ~Asyncable()
    {
        for (uintptr_t k : m_connects) {
            ptr(k)->disconnectAsync(this);
        }
    }

    struct IConnectable {
        virtual ~IConnectable() {}
        virtual void disconnectAsync(Asyncable* a) = 0;
    };

    void connectAsync(IConnectable* d)
    {
        if (d && m_connects.count(key(d)) == 0) {
            m_connects.insert(key(d));
        }
    }

    void disconnectAsync(IConnectable* d)
    {
        m_connects.erase(key(d));
    }

private:
    uintptr_t key(IConnectable* p) { return reinterpret_cast<uintptr_t>(p); }
    IConnectable* ptr(uintptr_t k) { return reinterpret_cast<IConnectable*>(k); }
    std::set<uintptr_t> m_connects;
};
}
}

#endif // DETO_ASYNC_ASYNCABLE_H
