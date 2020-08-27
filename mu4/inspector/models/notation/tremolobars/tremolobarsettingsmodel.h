#ifndef TREMOLOBARSETTINGSMODEL_H
#define TREMOLOBARSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class TremoloBarSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * type READ type CONSTANT)
    Q_PROPERTY(PropertyItem * curve READ curve CONSTANT)
    Q_PROPERTY(PropertyItem * lineThickness READ lineThickness CONSTANT)
    Q_PROPERTY(PropertyItem * scale READ scale CONSTANT)

    Q_PROPERTY(bool areSettingsAvailable READ areSettingsAvailable NOTIFY areSettingsAvailableChanged)

public:
    explicit TremoloBarSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* type() const;
    PropertyItem* curve() const;
    PropertyItem* lineThickness() const;
    PropertyItem* scale() const;

    bool areSettingsAvailable() const;

signals:
    void areSettingsAvailableChanged(bool areSettingsAvailable);

private:
    PropertyItem* m_type = nullptr;
    PropertyItem* m_curve = nullptr;
    PropertyItem* m_lineThickness = nullptr;
    PropertyItem* m_scale = nullptr;
};

#endif // TREMOLOBARSETTINGSMODEL_H
