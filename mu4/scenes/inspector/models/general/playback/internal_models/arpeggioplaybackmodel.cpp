#include "arpeggioplaybackmodel.h"

#include "dataformatter.h"

ArpeggioPlaybackModel::ArpeggioPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Arpeggio"));

    createProperties();
}

void ArpeggioPlaybackModel::createProperties()
{
    m_stretch = buildPropertyItem(Ms::Pid::TIME_STRETCH);
}

void ArpeggioPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::ARPEGGIO);
}

void ArpeggioPlaybackModel::loadProperties()
{
    loadPropertyItem(m_stretch, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void ArpeggioPlaybackModel::resetProperties()
{
    m_stretch->resetToDefault();
}

PropertyItem* ArpeggioPlaybackModel::stretch() const
{
    return m_stretch;
}
