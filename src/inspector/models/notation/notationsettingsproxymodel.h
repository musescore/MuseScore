#ifndef NOTATIONSETTINGSPROXYMODEL_H
#define NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"

namespace mu::inspector {
class NotationSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

public:
    explicit NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);
};
}

#endif // NOTATIONSETTINGSPROXYMODEL_H
