#include "horizontalframesettingsmodel.h"

#include "dataformatter.h"

HorizontalFrameSettingsModel::HorizontalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_HORIZONTAL_FRAME);
    setTitle(tr("Horizontal frame"));
    createProperties();
}

void HorizontalFrameSettingsModel::createProperties()
{
    m_frameWidth = buildPropertyItem(Ms::Pid::BOX_WIDTH);
    m_leftGap= buildPropertyItem(Ms::Pid::TOP_GAP);
    m_rightGap = buildPropertyItem(Ms::Pid::BOTTOM_GAP);
    m_shouldDisplayKeysAndBrackets = buildPropertyItem(Ms::Pid::CREATE_SYSTEM_HEADER);
}

void HorizontalFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HBOX);
}

void HorizontalFrameSettingsModel::loadProperties()
{
    loadPropertyItem(m_frameWidth, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_leftGap);
    loadPropertyItem(m_rightGap);
    loadPropertyItem(m_shouldDisplayKeysAndBrackets);
}

void HorizontalFrameSettingsModel::resetProperties()
{
    m_frameWidth->resetToDefault();
    m_leftGap->resetToDefault();
    m_rightGap->resetToDefault();
    m_shouldDisplayKeysAndBrackets->resetToDefault();
}

PropertyItem* HorizontalFrameSettingsModel::frameWidth() const
{
    return m_frameWidth;
}

PropertyItem* HorizontalFrameSettingsModel::leftGap() const
{
    return m_leftGap;
}

PropertyItem* HorizontalFrameSettingsModel::rightGap() const
{
    return m_rightGap;
}

PropertyItem* HorizontalFrameSettingsModel::shouldDisplayKeysAndBrackets() const
{
    return m_shouldDisplayKeysAndBrackets;
}
