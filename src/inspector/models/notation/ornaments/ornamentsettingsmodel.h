#ifndef MU_INSPECTOR_ORNAMENTSETTINGSMODEL_H
#define MU_INSPECTOR_ORNAMENTSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class OrnamentSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * performanceType READ performanceType CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

public:
    explicit OrnamentSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void openChannelAndMidiProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* performanceType() const;
    PropertyItem* placement() const;

private:
    PropertyItem* m_performanceType = nullptr;
    PropertyItem* m_placement = nullptr;
};
}

#endif // MU_INSPECTOR_ORNAMENTSETTINGSMODEL_H
