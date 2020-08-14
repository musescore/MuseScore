#ifndef BENDSETTINGSMODEL_H
#define BENDSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class BendSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bendType READ bendType CONSTANT)
    Q_PROPERTY(PropertyItem * bendCurve READ bendCurve CONSTANT)
    Q_PROPERTY(PropertyItem * lineThickness READ lineThickness CONSTANT)

    Q_PROPERTY(bool areSettingsAvailable READ areSettingsAvailable NOTIFY areSettingsAvailableChanged)

public:
    explicit BendSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bendType() const;
    PropertyItem* bendCurve() const;
    PropertyItem* lineThickness() const;

    bool areSettingsAvailable() const;

signals:
    void areSettingsAvailableChanged(bool areSettingsAvailable);

private:
    PropertyItem* m_bendType = nullptr;
    PropertyItem* m_bendCurve = nullptr;
    PropertyItem* m_lineThickness = nullptr;
};

#endif // BENDSETTINGSMODEL_H
