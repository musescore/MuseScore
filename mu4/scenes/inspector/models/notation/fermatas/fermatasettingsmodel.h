#ifndef FERMATASETTINGSMODEL_H
#define FERMATASETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

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

#endif // FERMATASETTINGSMODEL_H
