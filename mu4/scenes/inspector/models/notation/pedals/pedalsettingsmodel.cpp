#include "pedalsettingsmodel.h"

#include "dataformatter.h"

PedalSettingsModel::PedalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_PEDAL);
    setTitle(tr("Pedal"));
    createProperties();
}

void PedalSettingsModel::createProperties()
{
    m_hookType = buildPropertyItem(Ms::Pid::END_HOOK_TYPE);
    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_hookHeight = buildPropertyItem(Ms::Pid::END_HOOK_HEIGHT);
    m_lineStyle = buildPropertyItem(Ms::Pid::LINE_STYLE);
    m_dashLineLength = buildPropertyItem(Ms::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Ms::Pid::DASH_GAP_LEN);
    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);
}

void PedalSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::PEDAL);
}

void PedalSettingsModel::loadProperties()
{
    loadPropertyItem(m_hookType);

    auto formatDoubleFunc = [](const QVariant& elementPropertyValue) -> QVariant {
                                return DataFormatter::formatDouble(elementPropertyValue.toDouble());
                            };

    loadPropertyItem(m_thickness, formatDoubleFunc);

    loadPropertyItem(m_hookHeight, formatDoubleFunc);
    loadPropertyItem(m_lineStyle);

    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);

    loadPropertyItem(m_placement);
}

void PedalSettingsModel::resetProperties()
{
    m_hookType->resetToDefault();
    m_thickness->resetToDefault();
    m_hookHeight->resetToDefault();
    m_lineStyle->resetToDefault();
    m_dashLineLength->resetToDefault();
    m_dashGapLength->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* PedalSettingsModel::hookType() const
{
    return m_hookType;
}

PropertyItem* PedalSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* PedalSettingsModel::hookHeight() const
{
    return m_hookHeight;
}

PropertyItem* PedalSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* PedalSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* PedalSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* PedalSettingsModel::placement() const
{
    return m_placement;
}

bool PedalSettingsModel::hasToShowBothHooks() const
{
    return m_hasToShowBothHooks;
}

void PedalSettingsModel::setHasToShowBothHooks(bool hasToShowBothHooks)
{
    if (m_hasToShowBothHooks == hasToShowBothHooks) {
        return;
    }

    m_hasToShowBothHooks = hasToShowBothHooks;
    emit hasToShowBothHooksChanged(m_hasToShowBothHooks);
}
