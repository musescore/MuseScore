#include "dynamicplaybackmodel.h"

#include "dynamic.h"

DynamicPlaybackModel::DynamicPlaybackModel(QObject* parent, IElementRepositoryService* repository) : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Dynamics"));

    createProperties();
}

void DynamicPlaybackModel::createProperties()
{
    m_scopeType = buildPropertyItem(Ms::Pid::DYNAMIC_RANGE);
    m_velocity = buildPropertyItem(Ms::Pid::VELOCITY);
    m_velocityChangeSpeed = buildPropertyItem(Ms::Pid::VELO_CHANGE_SPEED);
    m_velocityChange = buildPropertyItem(Ms::Pid::VELO_CHANGE);

    //@note readonly property, there is no need to modify it
    m_isVelocityChangeAvailable = buildPropertyItem(Ms::Pid::DYNAMIC_TYPE, [](const int, const QVariant&){});
}

void DynamicPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::DYNAMIC);
}

void DynamicPlaybackModel::loadProperties()
{
    loadPropertyItem(m_scopeType);
    loadPropertyItem(m_velocity);
    loadPropertyItem(m_velocityChange);
    loadPropertyItem(m_velocityChangeSpeed);

    loadPropertyItem(m_isVelocityChangeAvailable, [] (const QVariant& elementPropertyValue) -> QVariant {
        Ms::Dynamic::Type dynamicType = elementPropertyValue.value<Ms::Dynamic::Type>();

        //@note Velocity change property changes are only available for the set of types (SF, SFPP, FP, RFZ)
        switch (dynamicType) {
        case Ms::Dynamic::Type::SF:
        case Ms::Dynamic::Type::SFPP:
        case Ms::Dynamic::Type::FP:
        case Ms::Dynamic::Type::RFZ:
            return true;

        default:
            return false;
        }
    });
}

void DynamicPlaybackModel::resetProperties()
{
    m_scopeType->resetToDefault();
    m_velocity->resetToDefault();
    m_velocityChange->resetToDefault();
    m_velocityChangeSpeed->resetToDefault();
}

PropertyItem* DynamicPlaybackModel::velocity() const
{
    return m_velocity;
}

PropertyItem* DynamicPlaybackModel::velocityChange() const
{
    return m_velocityChange;
}

PropertyItem* DynamicPlaybackModel::isVelocityChangeAvailable() const
{
    return m_isVelocityChangeAvailable;
}

PropertyItem* DynamicPlaybackModel::velocityChangeSpeed() const
{
    return m_velocityChangeSpeed;
}

PropertyItem* DynamicPlaybackModel::scopeType() const
{
    return m_scopeType;
}
