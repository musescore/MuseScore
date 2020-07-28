#include "qtqueuedinvoker.h"

using namespace deto::async;

void QtQueuedInvoker::invoke(int callKey, int dataKey)
{
    static const char* name = "doInvoke";
    QMetaObject::invokeMethod(this, name, Qt::QueuedConnection, Q_ARG(int, callKey), Q_ARG(int, dataKey));
}

void QtQueuedInvoker::doInvoke(int callKey, int dataKey)
{
    if (m_call) {
        m_call(callKey, dataKey);
    }
}

void QtQueuedInvoker::onInvoked(const Call& func)
{
    m_call = func;
}
