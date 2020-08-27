#include "markersettingsmodel.h"

MarkerSettingsModel::MarkerSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_MARKER);
    setTitle(tr("Marker"));
    createProperties();
}

void MarkerSettingsModel::createProperties()
{
    m_type = buildPropertyItem(Ms::Pid::MARKER_TYPE);
    m_label = buildPropertyItem(Ms::Pid::LABEL);
}

void MarkerSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::MARKER);
}

void MarkerSettingsModel::loadProperties()
{
    loadPropertyItem(m_type);
    loadPropertyItem(m_label);
}

void MarkerSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_label->resetToDefault();
}

PropertyItem* MarkerSettingsModel::type() const
{
    return m_type;
}

PropertyItem* MarkerSettingsModel::label() const
{
    return m_label;
}
