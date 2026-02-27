#pragma once

#include <QObject>
#include <qqmlintegration.h>

namespace mu::project {
class QSModel : public QObject
{
    Q_OBJECT

    QML_ELEMENT

public:
    explicit QSModel(QObject* parent = nullptr);

    Q_INVOKABLE void run();
};
}
