#ifndef DETO_ASYNC_QTQUEUEDINVOKER_H
#define DETO_ASYNC_QTQUEUEDINVOKER_H

#include <QObject>
#include <functional>
#include <queue>
#include <mutex>

namespace deto {
namespace async {
class QtQueuedInvoker : public QObject
{
    Q_OBJECT
public:
    QtQueuedInvoker() = default;

    using Call = std::function<void (int callKey, int dataKey)>;

    void invoke(int callKey, int dataKey);
    void processEvents();

    void onInvoked(const Call& func);

public slots:
    void doInvoke(int callKey, int dataKey);

private:

    Call m_call;
    std::mutex m_mutex;
    std::queue<std::pair<int, int> > m_manualQueue;
};
}
}

#endif // DETO_ASYNC_QTQUEUEDINVOKER_H
