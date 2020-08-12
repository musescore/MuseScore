#ifndef CLEFSETTINGSMODEL_H
#define CLEFSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

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

#endif // CLEFSETTINGSMODEL_H
