#include "qtqueuedinvoker.h"

#include <QThread>
#include <QDebug>
#include <QAbstractEventDispatcher>

using namespace deto::async;

void QtQueuedInvoker::invoke(int callKey, int dataKey)
{
    static const char* name = "doInvoke";

    if (this->thread()->eventDispatcher()) {
        QMetaObject::invokeMethod(this, name, Qt::QueuedConnection, Q_ARG(int, callKey), Q_ARG(int, dataKey));
    } else {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_manualQueue.push(std::make_pair(callKey, dataKey));
    }
}

void QtQueuedInvoker::doInvoke(int callKey, int dataKey)
{
    if (m_call) {
        m_call(callKey, dataKey);
    }
}

void QtQueuedInvoker::processEvents()
{
    QAbstractEventDispatcher* d = this->thread()->eventDispatcher();
    if (d) {
        d->processEvents(QEventLoop::AllEvents);
    } else {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_manualQueue.empty()) {
            const auto& p = m_manualQueue.front();
            doInvoke(p.first, p.second);
            m_manualQueue.pop();
        }
    }
}

void QtQueuedInvoker::onInvoked(const Call& func)
{
    m_call = func;
}
