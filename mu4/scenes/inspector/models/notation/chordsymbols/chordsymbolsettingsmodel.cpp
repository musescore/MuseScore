#include "chordsymbolsettingsmodel.h"

ChordSymbolSettingsModel::ChordSymbolSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_CHORD_SYMBOL);
    setTitle(tr("Chord symbol"));
    createProperties();
}

void ChordSymbolSettingsModel::createProperties()
{
    m_isLiteral = buildPropertyItem(Ms::Pid::HARMONY_VOICE_LITERAL);
    m_voicingType = buildPropertyItem(Ms::Pid::HARMONY_VOICING);
    m_durationType = buildPropertyItem(Ms::Pid::HARMONY_DURATION);
}

void ChordSymbolSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HARMONY);
}

void ChordSymbolSettingsModel::loadProperties()
{
    loadPropertyItem(m_isLiteral);
    loadPropertyItem(m_voicingType);
    loadPropertyItem(m_durationType);
}

void ChordSymbolSettingsModel::resetProperties()
{
    m_isLiteral->resetToDefault();
    m_voicingType->resetToDefault();
    m_durationType->resetToDefault();
}

PropertyItem* ChordSymbolSettingsModel::isLiteral() const
{
    return m_isLiteral;
}

PropertyItem* ChordSymbolSettingsModel::voicingType() const
{
    return m_voicingType;
}

PropertyItem* ChordSymbolSettingsModel::durationType() const
{
    return m_durationType;
}
