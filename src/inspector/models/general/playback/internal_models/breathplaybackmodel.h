#ifndef MU_INSPECTOR_BREATHPLAYBACKMODEL_H
#define MU_INSPECTOR_BREATHPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class BreathPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * pauseTime READ pauseTime CONSTANT)

public:
    explicit BreathPlaybackModel(QObject* parent, IElementRepositoryService* repository);

public:
    PropertyItem* pauseTime() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_pauseTime = nullptr;
};
}

#endif // BREATHPLAYBACKMODEL_H
