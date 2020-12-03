#include "fermatasettingsmodel.h"

FermataSettingsModel::FermataSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_FERMATA);
    setTitle(tr("Fermata"));
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
