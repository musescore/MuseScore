#include "mmrestsettingsmodel.h"

MMRestSettingsModel::MMRestSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_MMREST);
    setTitle(tr("Multimeasure rest"));
    createProperties();
}

void MMRestSettingsModel::createProperties()
{
    m_isNumberVisible = buildPropertyItem(Ms::Pid::MMREST_NUMBER_VISIBLE);
    m_numberPosition = buildPropertyItem(Ms::Pid::MMREST_NUMBER_POS);
}

void MMRestSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::MMREST);
}

void MMRestSettingsModel::loadProperties()
{
    loadPropertyItem(m_isNumberVisible);
    loadPropertyItem(m_numberPosition);
}

void MMRestSettingsModel::resetProperties()
{
    m_isNumberVisible->resetToDefault();
    m_numberPosition->resetToDefault();
}

PropertyItem* MMRestSettingsModel::isNumberVisible() const
{
    return m_isNumberVisible;
}

PropertyItem* MMRestSettingsModel::numberPosition() const
{
    return m_numberPosition;
}
