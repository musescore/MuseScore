#ifndef PEDALSSETTINGSMODEL_H
#define PEDALSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class PedalSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * hookType READ hookType CONSTANT)
    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * hookHeight READ hookHeight CONSTANT)
    Q_PROPERTY(PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(PropertyItem * dashGapLength READ dashGapLength CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(bool hasToShowBothHooks READ hasToShowBothHooks WRITE setHasToShowBothHooks NOTIFY hasToShowBothHooksChanged)
public:
    explicit PedalSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* hookType() const;
    PropertyItem* thickness() const;
    PropertyItem* hookHeight() const;
    PropertyItem* lineStyle() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;
    PropertyItem* placement() const;

    bool hasToShowBothHooks() const;

public slots:
    void setHasToShowBothHooks(bool hasToShowBothHooks);

signals:
    void hasToShowBothHooksChanged(bool hasToShowBothHooks);

private:
    PropertyItem* m_hookType = nullptr;
    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_hookHeight = nullptr;
    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;
    PropertyItem* m_placement = nullptr;

    bool m_hasToShowBothHooks = false;
};

#endif // PEDALSSETTINGSMODEL_H
