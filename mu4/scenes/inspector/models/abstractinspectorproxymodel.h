#ifndef ABSTRACTINSPECTORPROXYMODEL_H
#define ABSTRACTINSPECTORPROXYMODEL_H

#include "models/abstractinspectormodel.h"

#include <QHash>

class AbstractInspectorProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

public:
    explicit AbstractInspectorProxyModel(QObject* parent);

    Q_INVOKABLE QObject* modelByType(const InspectorModelType type);

    void createProperties() override {}
    void requestElements() override {}
    void loadProperties() override {}
    void resetProperties() override {}

    void requestResetToDefaults() override;
    bool hasAcceptableElements() const override;

protected:
    void addModel(AbstractInspectorModel* model);

private:
    QHash<int, AbstractInspectorModel*> m_modelsHash;
};

#endif // ABSTRACTINSPECTORPROXYMODEL_H
