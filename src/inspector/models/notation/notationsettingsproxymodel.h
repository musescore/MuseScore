#ifndef MU_INSPECTOR_NOTATIONSETTINGSPROXYMODEL_H
#define MU_INSPECTOR_NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"

namespace mu::inspector {
class NotationSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

public:
    explicit NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);
};
}

#endif // MU_INSPECTOR_NOTATIONSETTINGSPROXYMODEL_H
