#ifndef MU_INSPECTOR_HOOKSETTINGSMODEL_H
#define MU_INSPECTOR_HOOKSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class HookSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * horizontalOffset READ horizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)

public:
    explicit HookSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* horizontalOffset() const;
    PropertyItem* verticalOffset() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:

    PropertyItem* m_horizontalOffset = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
};
}

#endif // MU_INSPECTOR_HOOKSETTINGSMODEL_H
