#ifndef MU_INSPECTOR_GLISSANDOSETTINGSMODEL_H
#define MU_INSPECTOR_GLISSANDOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class GlissandoSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * lineType READ lineType CONSTANT)
public:
    explicit GlissandoSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* lineType() const;

private:
    PropertyItem* m_lineType = nullptr;
};
}

#endif // MU_INSPECTOR_GLISSANDOSETTINGSMODEL_H
