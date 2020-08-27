#include "breathplaybackmodel.h"

#include "dataformatter.h"

BreathPlaybackModel::BreathPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Breaths & pauses"));

    createProperties();
}

void BreathPlaybackModel::createProperties()
{
    m_pauseTime = buildPropertyItem(Ms::Pid::PAUSE);
}

void BreathPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BREATH);
}

void BreathPlaybackModel::loadProperties()
{
    loadPropertyItem(m_pauseTime, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void BreathPlaybackModel::resetProperties()
{
    m_pauseTime->resetToDefault();
}

PropertyItem* BreathPlaybackModel::pauseTime() const
{
    return m_pauseTime;
}
