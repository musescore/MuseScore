#ifndef DETO_ASYNC_QUEUEDINVOKER_H
#define DETO_ASYNC_QUEUEDINVOKER_H

#include <functional>
#include <queue>
#include <map>
#include <mutex>
#include <thread>

namespace deto {
namespace async {
class QueuedInvoker
{
public:

    static QueuedInvoker* instance();

    using Functor = std::function<void ()>;

    void invoke(const std::thread::id& th, const Functor& f, bool isAlwaysQueued = false);
    void processEvents();
    void onMainThreadInvoke(const std::function<void(const std::function<void()>&, bool)>& f);

private:

    QueuedInvoker() = default;

    using Queue = std::queue<Functor>;

    std::recursive_mutex m_mutex;
    std::map<std::thread::id, Queue > m_queues;

    std::function<void(const std::function<void()>&, bool)> m_onMainThreadInvoke;
    std::thread::id m_mainThreadID;
};
}
}

#endif // DETO_ASYNC_QUEUEDINVOKER_H
