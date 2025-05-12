#pragma once

#include <QObject>

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"

class InteractiveTestModel : public QObject
{
    Q_OBJECT

    muse::Inject<muse::IInteractive> interactive;

public:
    InteractiveTestModel();

    Q_INVOKABLE void openDialog();
};
