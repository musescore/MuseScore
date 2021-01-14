#ifndef MU_INSPECTOR_NOTATIONINSPECTORPROXYMODEL_H
#define MU_INSPECTOR_NOTATIONINSPECTORPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"

namespace mu::inspector {
class NoteSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

public:
    explicit NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);
};
}

#endif // MU_INSPECTOR_NOTATIONINSPECTORPROXYMODEL_H
