#ifndef NOTIFYLIST_H
#define NOTIFYLIST_H

#include <vector>
#include "changednotify.h"

namespace deto {
namespace async {
template<typename T>
class NotifyList : public std::vector<T>
{
public:
    NotifyList() {}
    NotifyList(ChangedNotify<T>* n)
        : m_notify(n) {}
    NotifyList(const std::vector<T>& l, ChangedNotify<T>* n)
        : std::vector<T>(l), m_notify(n) {}

    void setNotify(ChangedNotify<T>* n)
    {
        m_notify = n;
    }

    NotifyList<T>& operator =(const NotifyList<T>& nl)
    {
        std::vector<T>::operator=(nl);
        m_notify = nl.m_notify;
        return *this;
    }

    template<typename Call>
    void onChanged(Asyncable* caller, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->onChanged(caller, f, mode);
    }

    void resetOnChanged(Asyncable* caller)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->resetOnChanged(caller);
    }

    template<typename Call>
    void onItemChanged(Asyncable* caller, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->onItemChanged(caller, f, mode);
    }

    void resetOnItemChanged(Asyncable* caller)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->resetOnItemChanged(caller);
    }

    template<typename Call>
    void onItemAdded(Asyncable* caller, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->onItemAdded(caller, f, mode);
    }

    void resetOnItemAdded(Asyncable* caller)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->resetOnItemAdded(caller);
    }

    template<typename Call>
    void onItemRemoved(Asyncable* caller, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->onItemRemoved(caller, f, mode);
    }

    void resetOnItemRemoved(Asyncable* caller)
    {
        m_notify->resetOnItemRemoved(caller);
    }

    template<typename Call>
    void onItemReplaced(Asyncable* caller, Call f, Asyncable::AsyncMode mode = Asyncable::AsyncMode::AsyncSetOnce)
    {
        Q_ASSERT(m_notify);
        if (!m_notify) {
            return;
        }
        m_notify->onItemReplaced(caller, f, mode);
    }

    void resetOnItemReplaced(Asyncable* caller)
    {
        m_notify->resetOnItemReplaced(caller);
    }

private:

    ChangedNotify<T>* m_notify = nullptr;
};
}
}

#endif // NOTIFYLIST_H
