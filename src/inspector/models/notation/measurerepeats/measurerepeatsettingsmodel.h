#ifndef MU_INSPECTOR_MEASUREREPEATSETTINGSMODEL_H
#define MU_INSPECTOR_MEASUREREPEATSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class MeasureRepeatSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * numberPosition READ numberPosition CONSTANT)

public:
    explicit MeasureRepeatSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* numberPosition() const;

private:
    PropertyItem* m_numberPosition = nullptr;
};
}

#endif // MU_INSPECTOR_MEASUREREPEATSETTINGSMODEL_H
