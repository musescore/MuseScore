#include "dynamicplaybackmodel.h"

#include "dynamic.h"

DynamicPlaybackModel::DynamicPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
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

PropertyItem* DynamicPlaybackModel::velocityChangeSpeed() const
{
    return m_velocityChangeSpeed;
}

PropertyItem* DynamicPlaybackModel::scopeType() const
{
    return m_scopeType;
}
