#ifndef DYNAMICPLAYBACKMODEL_H
#define DYNAMICPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

class DynamicPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * scopeType READ scopeType CONSTANT)
    Q_PROPERTY(PropertyItem * velocity READ velocity CONSTANT)
    Q_PROPERTY(PropertyItem * velocityChange READ velocityChange CONSTANT)
    Q_PROPERTY(PropertyItem * velocityChangeSpeed READ velocityChangeSpeed CONSTANT)

public:
    explicit DynamicPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* scopeType() const;
    PropertyItem* velocity() const;
    PropertyItem* velocityChange() const;
    PropertyItem* velocityChangeSpeed() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_velocity = nullptr;
    PropertyItem* m_velocityChange = nullptr;
    PropertyItem* m_velocityChangeSpeed = nullptr;
    PropertyItem* m_scopeType = nullptr;
};

#endif // DYNAMICPLAYBACKMODEL_H
