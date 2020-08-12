#include "accidentalsettingsmodel.h"

AccidentalSettingsModel::AccidentalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_ACCIDENTAL);
    setTitle(tr("Accidental"));
    createProperties();
}

void AccidentalSettingsModel::createProperties()
{
    m_bracketType = buildPropertyItem(Ms::Pid::ACCIDENTAL_BRACKET);
}

void AccidentalSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::ACCIDENTAL);
}

void AccidentalSettingsModel::loadProperties()
{
    loadPropertyItem(m_bracketType);
}

void AccidentalSettingsModel::resetProperties()
{
    m_bracketType->resetToDefault();
}

PropertyItem* AccidentalSettingsModel::bracketType() const
{
    return m_bracketType;
}
