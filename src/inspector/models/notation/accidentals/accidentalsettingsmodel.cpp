#include "accidentalsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

AccidentalSettingsModel::AccidentalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_ACCIDENTAL);
    setTitle(qtrc("inspector", "Accidental"));
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
