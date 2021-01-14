#ifndef MU_INSPECTOR_ARPEGGIOPLAYBACKMODEL_H
#define MU_INSPECTOR_ARPEGGIOPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ArpeggioPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * stretch READ stretch CONSTANT)
public:
    explicit ArpeggioPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* stretch() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_stretch = nullptr;
};
}

#endif // MU_INSPECTOR_ARPEGGIOPLAYBACKMODEL_H
