#ifndef MU_INSPECTOR_SPACERSETTINGSMODEL_H
#define MU_INSPECTOR_SPACERSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class SpacerSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * spacerHeight READ spacerHeight CONSTANT)

public:
    explicit SpacerSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* spacerHeight() const;

private:
    PropertyItem* m_spacerHeight = nullptr;
};
}

#endif // MU_INSPECTOR_SPACERSETTINGSMODEL_H
