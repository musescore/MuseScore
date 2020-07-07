#include "barssettingsmodel.h"

BarsSettingsModel::BarsSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Bars"));
    setSectionType(SECTION_BAR);
}

void BarsSettingsModel::insertBars()
{
    parentScore()->startCmd();

    switch (m_barInsertionType) {
    case BarTypes::BarInsertionType::TYPE_PREPEND_TO_SCORE:
        parentScore()->prependMeasures(m_barCount);
        break;
    case BarTypes::BarInsertionType::TYPE_APPEND_TO_SCORE:
        parentScore()->appendMeasures(m_barCount);
        break;
    case BarTypes::BarInsertionType::TYPE_PREPEND_TO_SELECTION:
        parentScore()->insertMeasureBeforeSelection(m_barCount);
        break;
    case BarTypes::BarInsertionType::TYPE_APPEND_TO_SELECTION:
        parentScore()->insertMeasureAfterSelection(m_barCount);
        break;
    }

    parentScore()->endCmd(true);
}

void BarsSettingsModel::removeSelectedBars()
{
    parentScore()->startCmd();

    for (Ms::Element* element : m_elementList) {
        parentScore()->deleteItem(element);
    }

    parentScore()->endCmd(true);
}

void BarsSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::MEASURE);
}

int BarsSettingsModel::barCount() const
{
    return m_barCount;
}

BarTypes::BarInsertionType BarsSettingsModel::barInsertionType() const
{
    return m_barInsertionType;
}

void BarsSettingsModel::setBarCount(int barCount)
{
    if (m_barCount == barCount)
        return;

    m_barCount = barCount;
    emit barCountChanged(m_barCount);
}

void BarsSettingsModel::setBarInsertionType(BarTypes::BarInsertionType barInsertionType)
{
    if (m_barInsertionType == barInsertionType)
        return;

    m_barInsertionType = barInsertionType;
    emit barInsertionTypeChanged(m_barInsertionType);
}
