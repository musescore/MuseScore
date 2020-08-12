#ifndef SPACERSETTINGSMODEL_H
#define SPACERSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

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

#endif // SPACERSETTINGSMODEL_H
