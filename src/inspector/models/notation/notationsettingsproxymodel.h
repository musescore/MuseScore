#ifndef NOTATIONSETTINGSPROXYMODEL_H
#define NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"

class NotationSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

public:
    explicit NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);
};

#endif // NOTATIONSETTINGSPROXYMODEL_H
