#include "clefsettingsmodel.h"

ClefSettingsModel::ClefSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_CLEF);
    setTitle(tr("Clef"));
    createProperties();
}

void ClefSettingsModel::createProperties()
{
    m_shouldShowCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
}

void ClefSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::CLEF);
}

void ClefSettingsModel::loadProperties()
{
    loadPropertyItem(m_shouldShowCourtesy);
}

void ClefSettingsModel::resetProperties()
{
    m_shouldShowCourtesy->resetToDefault();
}

PropertyItem* ClefSettingsModel::shouldShowCourtesy() const
{
    return m_shouldShowCourtesy;
}
