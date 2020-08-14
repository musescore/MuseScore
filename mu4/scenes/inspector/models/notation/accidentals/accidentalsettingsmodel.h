#ifndef ACCIDENTALSETTINGSMODEL_H
#define ACCIDENTALSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class AccidentalSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bracketType READ bracketType CONSTANT)
public:
    explicit AccidentalSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bracketType() const;

private:
    PropertyItem* m_bracketType = nullptr;
};

#endif // ACCIDENTALSETTINGSMODEL_H
