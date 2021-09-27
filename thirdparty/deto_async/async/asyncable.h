#ifndef DETO_ASYNC_ASYNCABLE_H
#define DETO_ASYNC_ASYNCABLE_H

#include <set>
#include <cstdint>

namespace deto {
namespace async {
class Asyncable
{
public:

    enum class AsyncMode {
        AsyncSetOnce = 0,
        AsyncSetRepeat
    };

    virtual ~Asyncable()
    {
        disconnectAll();
    }

    struct IConnectable {
        virtual ~IConnectable() {}
        virtual void disconnectAsync(Asyncable* a) = 0;
    };

    bool isConnectedAsync() const { return !m_connects.empty(); }

    void connectAsync(IConnectable* c)
    {
        if (c && m_connects.count(c) == 0) {
            m_connects.insert(c);
        }
    }

    void disconnectAsync(IConnectable* c)
    {
        m_connects.erase(c);
    }

    void disconnectAll()
    {
        auto copy = m_connects;
        for (IConnectable* c : copy) {
            c->disconnectAsync(this);
        }
    }

private:
    std::set<IConnectable*> m_connects;
};
}
}

#endif // DETO_ASYNC_ASYNCABLE_H
