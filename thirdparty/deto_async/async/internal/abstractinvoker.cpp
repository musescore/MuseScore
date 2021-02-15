#include "abstractinvoker.h"

#include <cassert>

#include "queuedinvoker.h"

using namespace deto::async;

AbstractInvoker::AbstractInvoker()
{
}

AbstractInvoker::~AbstractInvoker()
{
}

void AbstractInvoker::invoke(int type)
{
    invoke(type, NotifyData());
}

void AbstractInvoker::invoke(int type, const NotifyData& data)
{
    auto it = m_callbacks.find(type);
    if (it == m_callbacks.end()) {
        return;
    }

    std::thread::id threadID = std::this_thread::get_id();

    //! NOTE: explicit copy because collection can be modified from elsewhere
    CallBacks callbacks = it->second;

    for (const CallBack& c : callbacks) {
        if (c.threadID == threadID) {
            invokeCallback(type, c, data);
        } else {
            auto functor = [this, type, c, data]() { invokeCallback(type, c, data); };
            QueuedInvoker::instance()->invoke(c.threadID, functor);
        }
    }
}

void AbstractInvoker::invokeCallback(int type, const CallBack& c, const NotifyData& data)
{
    assert(c.threadID == std::this_thread::get_id());
    doInvoke(type, c.call, data);
}

void AbstractInvoker::processEvents()
{
    QueuedInvoker::instance()->processEvents();
}

void AbstractInvoker::onMainThreadInvoke(const std::function<void(const std::function<void()>&)>& f)
{
    QueuedInvoker::instance()->onMainThreadInvoke(f);
}

bool AbstractInvoker::isConnected() const
{
    for (auto it = m_callbacks.cbegin(); it != m_callbacks.cend(); ++it) {
        const CallBacks& cs = it->second;
        if (cs.size() > 0) {
            return true;
        }
    }
    return false;
}

int AbstractInvoker::CallBacks::receiverIndexOf(Asyncable* receiver) const
{
    for (size_t i = 0; i < size(); ++i) {
        if (at(i).receiver == receiver) {
            return int(i);
        }
    }
    return -1;
}

bool AbstractInvoker::CallBacks::containsReceiver(Asyncable* receiver) const
{
    return receiverIndexOf(receiver) > -1;
}

void AbstractInvoker::removeCallBack(int type, Asyncable* receiver)
{
    auto it = m_callbacks.find(type);
    if (it == m_callbacks.end()) {
        return;
    }

    CallBacks& callbacks = it->second;
    int index = callbacks.receiverIndexOf(receiver);
    if (index < 0) {
        return;
    }

    CallBack c = callbacks.at(index);
    if (c.receiver) {
        c.receiver->disconnectAsync(this);
    }
    callbacks.erase(callbacks.begin() + index);

    deleteCall(type, c.call);
}

void AbstractInvoker::removeAllCallBacks()
{
    for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) {
        for (CallBack& c : it->second) {
            if (c.receiver) {
                c.receiver->disconnectAsync(this);
            }

            deleteCall(c.type, c.call);
        }
    }
    m_callbacks.clear();
}

void AbstractInvoker::addCallBack(int type, Asyncable* receiver, void* call, Asyncable::AsyncMode mode)
{
    const CallBacks& callbacks = m_callbacks[type];
    if (callbacks.containsReceiver(receiver)) {
        switch (mode) {
        case Asyncable::AsyncMode::AsyncSetOnce:
            deleteCall(type, call);
            return;
        case Asyncable::AsyncMode::AsyncSetRepeat:
            removeCallBack(type, receiver);
            break;
        }
    }

    CallBack c(std::this_thread::get_id(), type, receiver, call);
    m_callbacks[type].push_back(c);

    if (c.receiver) {
        c.receiver->connectAsync(this);
    }
}

void AbstractInvoker::disconnectAsync(Asyncable* receiver)
{
    std::vector<int> types;
    for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) {
        for (CallBack& c : it->second) {
            if (c.receiver == receiver) {
                types.push_back(c.type);
            }
        }
    }

    for (int type : types) {
        removeCallBack(type, receiver);
    }
}
