#include "queuedinvoker.h"

using namespace deto::async;

void QueuedInvoker::invoke(const std::thread::id& th, const Functor& f)
{
    if (m_onMainThreadInvoke) {
        if (th == m_mainThreadID) {
            m_onMainThreadInvoke(f);
        }
    }

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_queues[th].push(f);
}

void QueuedInvoker::processEvents()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    Queue& q = m_queues[std::this_thread::get_id()];
    while (!q.empty()) {
        const auto& f = q.front();
        f();
        q.pop();
    }
}

void QueuedInvoker::onMainThreadInvoke(const std::function<void(const std::function<void()>&)>& f)
{
    m_onMainThreadInvoke = f;
    m_mainThreadID = std::this_thread::get_id();
}
