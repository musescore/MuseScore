#ifndef MU_INSPECTOR_FERMATASETTINGSMODEL_H
#define MU_INSPECTOR_FERMATASETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class FermataSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * placementType READ placementType CONSTANT)
public:
    explicit FermataSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* placementType() const;

private:
    PropertyItem* m_placementType = nullptr;
};
}

#endif // MU_INSPECTOR_FERMATASETTINGSMODEL_H
