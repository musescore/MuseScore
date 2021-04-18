#ifndef MU_INSPECTOR_CLEFSETTINGSMODEL_H
#define MU_INSPECTOR_CLEFSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ClefSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * shouldShowCourtesy READ shouldShowCourtesy CONSTANT)

public:
    explicit ClefSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* shouldShowCourtesy() const;

private:
    PropertyItem* m_shouldShowCourtesy = nullptr;
};
}

#endif // MU_INSPECTOR_CLEFSETTINGSMODEL_H
