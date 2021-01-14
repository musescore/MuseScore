#ifndef MU_INSPECTOR_ACCIDENTALSETTINGSMODEL_H
#define MU_INSPECTOR_ACCIDENTALSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
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
}

#endif // MU_INSPECTOR_ACCIDENTALSETTINGSMODEL_H
