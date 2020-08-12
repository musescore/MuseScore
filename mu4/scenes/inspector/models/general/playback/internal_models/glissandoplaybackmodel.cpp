#include "glissandoplaybackmodel.h"

GlissandoPlaybackModel::GlissandoPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Glissando"));

    createProperties();
}

void GlissandoPlaybackModel::createProperties()
{
    m_styleType = buildPropertyItem(Ms::Pid::GLISSANDO_STYLE);
}

void GlissandoPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::GLISSANDO);
}

void GlissandoPlaybackModel::loadProperties()
{
    loadPropertyItem(m_styleType);
}

void GlissandoPlaybackModel::resetProperties()
{
    m_styleType->resetToDefault();
}

PropertyItem* GlissandoPlaybackModel::styleType() const
{
    return m_styleType;
}
