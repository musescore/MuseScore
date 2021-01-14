#ifndef MU_INSPECTOR_MARKERSETTINGSMODEL_H
#define MU_INSPECTOR_MARKERSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class MarkerSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * type READ type CONSTANT)
    Q_PROPERTY(PropertyItem * label READ label CONSTANT)
public:
    explicit MarkerSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* type() const;
    PropertyItem* label() const;

private:
    PropertyItem* m_type = nullptr;
    PropertyItem* m_label = nullptr;
};
}

#endif // MU_INSPECTOR_MARKERSETTINGSMODEL_H
