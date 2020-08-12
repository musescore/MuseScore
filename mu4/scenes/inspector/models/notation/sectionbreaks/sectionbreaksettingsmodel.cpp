#include "sectionbreaksettingsmodel.h"

#include "dataformatter.h"

SectionBreakSettingsModel::SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_SECTIONBREAK);
    setTitle(tr("Section Break"));
    createProperties();
}

void SectionBreakSettingsModel::createProperties()
{
    m_shouldStartWithLongInstrNames = buildPropertyItem(Ms::Pid::START_WITH_LONG_NAMES);
    m_shouldResetBarNums = buildPropertyItem(Ms::Pid::START_WITH_MEASURE_ONE);
    m_pauseDuration = buildPropertyItem(Ms::Pid::PAUSE);
}

void SectionBreakSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::LAYOUT_BREAK);
}

void SectionBreakSettingsModel::loadProperties()
{
    loadPropertyItem(m_shouldStartWithLongInstrNames);
    loadPropertyItem(m_shouldResetBarNums);
    loadPropertyItem(m_pauseDuration, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void SectionBreakSettingsModel::resetProperties()
{
    m_shouldStartWithLongInstrNames->resetToDefault();
    m_shouldResetBarNums->resetToDefault();
    m_pauseDuration->resetToDefault();
}

PropertyItem* SectionBreakSettingsModel::shouldStartWithLongInstrNames() const
{
    return m_shouldStartWithLongInstrNames;
}

PropertyItem* SectionBreakSettingsModel::shouldResetBarNums() const
{
    return m_shouldResetBarNums;
}

PropertyItem* SectionBreakSettingsModel::pauseDuration() const
{
    return m_pauseDuration;
}
