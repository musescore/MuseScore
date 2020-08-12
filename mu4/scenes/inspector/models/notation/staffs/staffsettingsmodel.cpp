#include "staffsettingsmodel.h"

StaffSettingsModel::StaffSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_STAFF);
    setTitle(tr("Staff"));
    createProperties();
}

void StaffSettingsModel::createProperties()
{
    m_barlinesSpanFrom = buildPropertyItem(Ms::Pid::STAFF_BARLINE_SPAN_FROM);
    m_barlinesSpanTo = buildPropertyItem(Ms::Pid::STAFF_BARLINE_SPAN_TO);
}

void StaffSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::STAFF);
}

void StaffSettingsModel::loadProperties()
{
    loadPropertyItem(m_barlinesSpanFrom);
    loadPropertyItem(m_barlinesSpanTo);
}

void StaffSettingsModel::resetProperties()
{
    m_barlinesSpanFrom->resetToDefault();
    m_barlinesSpanTo->resetToDefault();
}

PropertyItem* StaffSettingsModel::barlinesSpanFrom() const
{
    return m_barlinesSpanFrom;
}

PropertyItem* StaffSettingsModel::barlinesSpanTo() const
{
    return m_barlinesSpanTo;
}
