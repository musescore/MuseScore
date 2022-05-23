#include "queuedinvoker.h"

using namespace deto::async;

QueuedInvoker* QueuedInvoker::instance()
{
    static QueuedInvoker i;
    return &i;
}

void QueuedInvoker::invoke(const std::thread::id& th, const Functor& f, bool isAlwaysQueued)
{
    if (m_onMainThreadInvoke) {
        if (th == m_mainThreadID) {
            m_onMainThreadInvoke(f, isAlwaysQueued);
        }
    }

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_queues[th].push(f);
}

void QueuedInvoker::processEvents()
{
    Queue q;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto n = m_queues.extract(std::this_thread::get_id());
        if (!n.empty()) {
            q = n.mapped();
        }
    }
    while (!q.empty()) {
        const auto& f = q.front();
        if (f) {
            f();
        }
        q.pop();
    }
}

void QueuedInvoker::onMainThreadInvoke(const std::function<void(const std::function<void()>&, bool)>& f)
{
    m_onMainThreadInvoke = f;
    m_mainThreadID = std::this_thread::get_id();
}
