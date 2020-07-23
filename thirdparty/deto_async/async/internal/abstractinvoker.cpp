#include "abstractinvoker.h"

#include <cassert>

using namespace deto::async;

AbstractInvoker::AbstractInvoker()
    : m_key(100), m_threadID(std::this_thread::get_id())
{
}

AbstractInvoker::~AbstractInvoker()
{
}

int AbstractInvoker::newKey()
{
    ++m_key;
    return m_key;
}

void AbstractInvoker::invokeMethod(int callKey, const NotifyData& data)
{
    if (std::this_thread::get_id() == m_threadID) {
        onInvoke(callKey, data);
    } else {
        // todo
        // assert(std::this_thread::get_id() == m_threadID);
        onInvoke(callKey, data); //! TODO Temporary solution
    }
}

void AbstractInvoker::invoke(int callKey)
{
    invokeMethod(callKey, NotifyData());
}

void AbstractInvoker::invoke(int callKey, const NotifyData& data)
{
    invokeMethod(callKey, data);
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

bool AbstractInvoker::CallBacks::containsreceiver(Asyncable* receiver) const
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

void AbstractInvoker::setCallBack(int type, Asyncable* receiver, void* call, Asyncable::AsyncMode mode)
{
    const CallBacks& callbacks = m_callbacks[type];
    if (callbacks.containsreceiver(receiver)) {
        switch (mode) {
        case Asyncable::AsyncMode::AsyncSetOnce:
            deleteCall(type, call);
            return;
        case Asyncable::AsyncMode::AsyncSetRepeat:
            removeCallBack(type, receiver);
            break;
        }
    }

    CallBack c(type, receiver, call);
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

std::vector<void*> AbstractInvoker::calls(int type) const
{
    std::vector<void*> cls;
    auto it = m_callbacks.find(type);
    if (it == m_callbacks.end()) {
        return cls;
    }

    const CallBacks& callbacks = it->second;
    for (const CallBack& c : callbacks) {
        cls.push_back(c.call);
    }

    return cls;
}

void AbstractInvoker::pushData(int key, const NotifyData& d)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data.insert({ key, d });
}

NotifyData AbstractInvoker::popData(int key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    NotifyData d = m_data.at(key);
    m_data.erase(key);
    return d;
}
