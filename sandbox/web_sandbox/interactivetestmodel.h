#pragma once

#include <QObject>
#include <QEventLoop>

#include "global/modularity/ioc.h"
#include "interactive/iinteractive.h"

class InteractiveTestModel : public QObject
{
    Q_OBJECT

    muse::ContextInject<muse::IInteractive> interactive;

public:
    InteractiveTestModel();

    Q_INVOKABLE void openAsyncDialog();
    Q_INVOKABLE void openSyncDialog();

    Q_INVOKABLE QString runLoop();
    Q_INVOKABLE void exitLoop();

    Q_INVOKABLE QString runSleep();
    Q_INVOKABLE void exitSleep();

    bool m_running = false;
    QEventLoop m_loop;
};
