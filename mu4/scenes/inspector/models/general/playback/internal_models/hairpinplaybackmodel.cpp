#include "hairpinplaybackmodel.h"

HairpinPlaybackModel::HairpinPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Hairpins, Crescendo & Diminuendo"));

    createProperties();
}

void HairpinPlaybackModel::createProperties()
{
    m_velocityChange = buildPropertyItem(Ms::Pid::VELO_CHANGE);
    m_velocityChangeType = buildPropertyItem(Ms::Pid::VELO_CHANGE_METHOD);
}

void HairpinPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN);
}

void HairpinPlaybackModel::loadProperties()
{
    loadPropertyItem(m_velocityChange);
    loadPropertyItem(m_velocityChangeType);
}

void HairpinPlaybackModel::resetProperties()
{
    m_velocityChange->resetToDefault();
    m_velocityChangeType->resetToDefault();
}

PropertyItem* HairpinPlaybackModel::velocityChange() const
{
    return m_velocityChange;
}

PropertyItem* HairpinPlaybackModel::velocityChangeType() const
{
    return m_velocityChangeType;
}
