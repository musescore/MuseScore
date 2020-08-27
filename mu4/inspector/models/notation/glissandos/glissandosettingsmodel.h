#ifndef GLISSANDOSETTINGSMODEL_H
#define GLISSANDOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

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

#endif // GLISSANDOSETTINGSMODEL_H
