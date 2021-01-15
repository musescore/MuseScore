#include "fermatasettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

FermataSettingsModel::FermataSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FERMATA);
    setTitle(qtrc("inspector", "Fermata"));
    createProperties();
}

void FermataSettingsModel::createProperties()
{
    m_placementType = buildPropertyItem(Ms::Pid::PLACEMENT);
}

void FermataSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::FERMATA);
}

void FermataSettingsModel::loadProperties()
{
    loadPropertyItem(m_placementType);
}

void FermataSettingsModel::resetProperties()
{
    m_placementType->resetToDefault();
}

PropertyItem* FermataSettingsModel::placementType() const
{
    return m_placementType;
}
