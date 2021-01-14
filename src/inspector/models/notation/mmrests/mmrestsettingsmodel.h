#ifndef MU_INSPECTOR_MMRESTSETTINGSMODEL_H
#define MU_INSPECTOR_MMRESTSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class MMRestSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isNumberVisible READ isNumberVisible CONSTANT)
    Q_PROPERTY(PropertyItem * numberPosition READ numberPosition CONSTANT)

public:
    explicit MMRestSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isNumberVisible() const;
    PropertyItem* numberPosition() const;

private:
    PropertyItem* m_isNumberVisible = nullptr;
    PropertyItem* m_numberPosition = nullptr;
};
}

#endif // MU_INSPECTOR_MMRESTSETTINGSMODEL_H
