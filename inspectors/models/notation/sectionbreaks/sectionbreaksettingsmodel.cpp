#include "sectionbreaksettingsmodel.h"

SectionBreakSettingsModel::SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_SECTIONBREAK);
    setTitle(tr("Section Break"));
    createProperties();
}

void SectionBreakSettingsModel::createProperties()
{
    m_startWithLongInstrNames = buildPropertyItem(Ms::Pid::START_WITH_LONG_NAMES);
    m_resetBarNums = buildPropertyItem(Ms::Pid::START_WITH_MEASURE_ONE);
    m_pause = buildPropertyItem(Ms::Pid::PAUSE);
}

void SectionBreakSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::LAYOUT_BREAK);
}

void SectionBreakSettingsModel::loadProperties()
{
    loadPropertyItem(m_startWithLongInstrNames);
    loadPropertyItem(m_resetBarNums);
    loadPropertyItem(m_pause, [] (const QVariant& elementPropertyValue) -> QVariant {
        return QString::number(elementPropertyValue.toDouble(), 'f', 2).toDouble();
    });
}

void SectionBreakSettingsModel::resetProperties()
{
    m_startWithLongInstrNames->resetToDefault();
    m_resetBarNums->resetToDefault();
    m_pause->resetToDefault();
}

PropertyItem* SectionBreakSettingsModel::startWithLongInstrNames() const
{
    return m_startWithLongInstrNames;
}

PropertyItem* SectionBreakSettingsModel::resetBarNums() const
{
    return m_resetBarNums;
}

PropertyItem* SectionBreakSettingsModel::pause() const
{
    return m_pause;
}
