#include "measuresettingsmodel.h"

MeasureSettingsModel::MeasureSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Bars"));
    setSectionType(SECTION_BAR);
}

void MeasureSettingsModel::insertMeasures()
{
    parentScore()->startCmd();

    switch (m_barInsertionType) {
    case MeasureTypes::MeasureInsertionType::TYPE_PREPEND_TO_SCORE:
        parentScore()->prependMeasures(m_measureCount);
        break;
    case MeasureTypes::MeasureInsertionType::TYPE_APPEND_TO_SCORE:
        parentScore()->appendMeasures(m_measureCount);
        break;
    case MeasureTypes::MeasureInsertionType::TYPE_PREPEND_TO_SELECTION:
        parentScore()->insertMeasureBeforeSelection(m_measureCount);
        break;
    case MeasureTypes::MeasureInsertionType::TYPE_APPEND_TO_SELECTION:
        parentScore()->insertMeasureAfterSelection(m_measureCount);
        break;
    }

    parentScore()->endCmd(true);
}

void MeasureSettingsModel::removeSelectedMeasures()
{
    parentScore()->startCmd();

    for (Ms::Element* element : m_elementList) {
        parentScore()->deleteItem(element);
    }

    parentScore()->endCmd(true);
}

void MeasureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::MEASURE);
}

int MeasureSettingsModel::measureCount() const
{
    return m_measureCount;
}

MeasureTypes::MeasureInsertionType MeasureSettingsModel::measureInsertionType() const
{
    return m_barInsertionType;
}

void MeasureSettingsModel::setMeasureCount(int measureCount)
{
    if (m_measureCount == measureCount)
        return;

    m_measureCount = measureCount;
    emit measureCountChanged(m_measureCount);
}

void MeasureSettingsModel::setMeasureInsertionType(MeasureTypes::MeasureInsertionType barInsertionType)
{
    if (m_barInsertionType == barInsertionType)
        return;

    m_barInsertionType = barInsertionType;
    emit measureInsertionTypeChanged(m_barInsertionType);
}
