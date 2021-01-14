#ifndef MU_INSPECTOR_HAIRPINPLAYBACKMODEL_H
#define MU_INSPECTOR_HAIRPINPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class HairpinPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * velocityChange READ velocityChange CONSTANT)
    Q_PROPERTY(PropertyItem * velocityChangeType READ velocityChangeType CONSTANT)

public:
    explicit HairpinPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* velocityChange() const;
    PropertyItem* velocityChangeType() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_velocityChange = nullptr;
    PropertyItem* m_velocityChangeType = nullptr;
};
}

#endif // MU_INSPECTOR_HAIRPINPLAYBACKMODEL_H
