#ifndef EXPRESSIONSETTINGSMODEL_H
#define EXPRESSIONSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ExpressionSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * snapExpression READ snapExpression CONSTANT)

public:
    explicit ExpressionSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* snapExpression() const;

private:
    PropertyItem* m_snapExpression = nullptr;
};
}

#endif // EXPRESSIONSETTINGSMODEL_H
