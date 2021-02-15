#ifndef MU_INSPECTOR_STAFFSETTINGSMODEL_H
#define MU_INSPECTOR_STAFFSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class StaffSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * barlinesSpanFrom READ barlinesSpanFrom CONSTANT)
    Q_PROPERTY(PropertyItem * barlinesSpanTo READ barlinesSpanTo CONSTANT)
public:
    explicit StaffSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* barlinesSpanFrom() const;
    PropertyItem* barlinesSpanTo() const;

private:
    PropertyItem* m_barlinesSpanFrom = nullptr;
    PropertyItem* m_barlinesSpanTo = nullptr;
};
}

#endif // MU_INSPECTOR_STAFFSETTINGSMODEL_H
