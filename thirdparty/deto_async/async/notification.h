#ifndef DETO_ASYNC_NOTIFICATION_H
#define DETO_ASYNC_NOTIFICATION_H

#include "channel.h"

namespace deto {
namespace async {
class Notification
{
public:

    void notify()
    {
        m_ch.send(0);
    }

    template<typename Func>
    void onNotify(const Asyncable* receiver, Func f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_ch.onReceive(receiver, [f](int) { f(); }, mode);
    }

    void resetOnNotify(const Asyncable* receiver)
    {
        m_ch.resetOnReceive(receiver);
    }

    void close()
    {
        m_ch.close();
    }

    template<typename Func>
    void onClose(const Asyncable* receiver, Func f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        m_ch.onClose(receiver, f, mode);
    }

    bool isConnected() const
    {
        return m_ch.isConnected();
    }

private:
    Channel<int> m_ch;
};
}
}

#endif // DETO_ASYNC_NOTIFICATION_H
