#include "jumpsettingsmodel.h"

JumpSettingsModel::JumpSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_JUMP);
    setTitle(tr("Jump"));
    createProperties();
}

void JumpSettingsModel::createProperties()
{
    m_jumpTo = buildPropertyItem(Ms::Pid::JUMP_TO);
    m_playUntil = buildPropertyItem(Ms::Pid::PLAY_UNTIL);
    m_continueAt = buildPropertyItem(Ms::Pid::CONTINUE_AT);
    m_hasToPlayRepeats = buildPropertyItem(Ms::Pid::PLAY_REPEATS);
}

void JumpSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::JUMP);
}

void JumpSettingsModel::loadProperties()
{
    loadPropertyItem(m_jumpTo);
    loadPropertyItem(m_playUntil);
    loadPropertyItem(m_continueAt);
    loadPropertyItem(m_hasToPlayRepeats);
}

void JumpSettingsModel::resetProperties()
{
    m_jumpTo->resetToDefault();
    m_playUntil->resetToDefault();
    m_continueAt->resetToDefault();
    m_hasToPlayRepeats->resetToDefault();
}

PropertyItem* JumpSettingsModel::jumpTo() const
{
    return m_jumpTo;
}

PropertyItem* JumpSettingsModel::playUntil() const
{
    return m_playUntil;
}

PropertyItem* JumpSettingsModel::continueAt() const
{
    return m_continueAt;
}

PropertyItem* JumpSettingsModel::hasToPlayRepeats() const
{
    return m_hasToPlayRepeats;
}
