#ifndef MU_INSPECTOR_GLISSANDOPLAYBACKMODEL_H
#define MU_INSPECTOR_GLISSANDOPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class GlissandoPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * styleType READ styleType CONSTANT)

public:
    explicit GlissandoPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* styleType() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_styleType = nullptr;
};
}

#endif // MU_INSPECTOR_GLISSANDOPLAYBACKMODEL_H
