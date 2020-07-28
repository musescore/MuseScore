#ifndef NOTATIONINSPECTORPROXYMODEL_H
#define NOTATIONINSPECTORPROXYMODEL_H

#include "models/abstractinspectorproxymodel.h"

class NoteSettingsProxyModel : public AbstractInspectorProxyModel
{
    Q_OBJECT

public:
    explicit NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);
};

#endif // NOTATIONINSPECTORPROXYMODEL_H
