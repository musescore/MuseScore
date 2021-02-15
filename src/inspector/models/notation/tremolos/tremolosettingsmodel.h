#ifndef MU_INSPECTOR_TREMOLOSETTINGSMODEL_H
#define MU_INSPECTOR_TREMOLOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TremoloSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * style READ style CONSTANT)

public:
    explicit TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* style() const;

private:
    PropertyItem* m_style = nullptr;
};
}

#endif // MU_INSPECTOR_TREMOLOSETTINGSMODEL_H
