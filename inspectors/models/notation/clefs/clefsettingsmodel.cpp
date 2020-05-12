#include "clefsettingsmodel.h"

ClefSettingsModel::ClefSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_CLEF);
    setTitle(tr("Clef"));
    createProperties();
}

void ClefSettingsModel::createProperties()
{
    m_showCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
}

void ClefSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::CLEF);
}

void ClefSettingsModel::loadProperties()
{
    loadPropertyItem(m_showCourtesy);
}

void ClefSettingsModel::resetProperties()
{
    m_showCourtesy->resetToDefault();
}

PropertyItem* ClefSettingsModel::showCourtesy() const
{
    return m_showCourtesy;
}
