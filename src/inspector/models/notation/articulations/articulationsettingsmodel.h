#ifndef ARTICULATIONSETTINGSMODEL_H
#define ARTICULATIONSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class ArticulationSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

public:
    explicit ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void openChannelAndMidiProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* direction() const;
    PropertyItem* placement() const;

private:
    PropertyItem* m_direction = nullptr;
    PropertyItem* m_placement = nullptr;
};

#endif // ARTICULATIONSETTINGSMODEL_H
