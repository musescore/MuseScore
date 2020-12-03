#include "noteplaybackmodel.h"

#include "dataformatter.h"

NotePlaybackModel::NotePlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Notes"));

    createProperties();
}

void NotePlaybackModel::createProperties()
{
    m_tuning = buildPropertyItem(Ms::Pid::TUNING);
    m_velocity = buildPropertyItem(Ms::Pid::VELO_OFFSET);
    m_overrideDynamics = buildPropertyItem(Ms::Pid::VELO_TYPE);
}

void NotePlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::NOTE);
}

void NotePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tuning, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_velocity);

    loadPropertyItem(m_overrideDynamics, [](const QVariant& elementValue) -> QVariant {
        return static_cast<bool>(elementValue.toInt());
    });
}

void NotePlaybackModel::resetProperties()
{
    m_tuning->resetToDefault();
    m_velocity->resetToDefault();
    m_overrideDynamics->resetToDefault();
}

PropertyItem* NotePlaybackModel::tuning() const
{
    return m_tuning;
}

PropertyItem* NotePlaybackModel::velocity() const
{
    return m_velocity;
}

PropertyItem* NotePlaybackModel::overrideDynamics() const
{
    return m_overrideDynamics;
}
