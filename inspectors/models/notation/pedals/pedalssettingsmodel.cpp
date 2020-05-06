#include "pedalssettingsmodel.h"

PedalsSettingsModel::PedalsSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_PEDAL);
    setTitle(tr("Pedals"));
    createProperties();
}

void PedalsSettingsModel::createProperties()
{
    m_hookType = buildPropertyItem(Ms::Pid::END_HOOK_TYPE);
    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_hookHeight = buildPropertyItem(Ms::Pid::END_HOOK_HEIGHT);
    m_lineStyle = buildPropertyItem(Ms::Pid::LINE_STYLE);
    m_dashLineLength = buildPropertyItem(Ms::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Ms::Pid::DASH_GAP_LEN);
    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);
}

void PedalsSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::PEDAL);
}

void PedalsSettingsModel::loadProperties()
{
    loadPropertyItem(m_hookType);

    auto formatDoubleFunc = [] (const QVariant& elementPropertyValue) -> QVariant {
        return QString::number(elementPropertyValue.toDouble(), 'f', 2).toDouble();
    };

    loadPropertyItem(m_thickness, formatDoubleFunc);

    loadPropertyItem(m_hookHeight, formatDoubleFunc);
    loadPropertyItem(m_lineStyle);

    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);

    loadPropertyItem(m_placement);
}

void PedalsSettingsModel::resetProperties()
{
    m_hookType->resetToDefault();
    m_thickness->resetToDefault();
    m_hookHeight->resetToDefault();
    m_lineStyle->resetToDefault();
    m_dashLineLength->resetToDefault();
    m_dashGapLength->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* PedalsSettingsModel::hookType() const
{
    return m_hookType;
}

PropertyItem* PedalsSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* PedalsSettingsModel::hookHeight() const
{
    return m_hookHeight;
}

PropertyItem* PedalsSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* PedalsSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* PedalsSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* PedalsSettingsModel::placement() const
{
    return m_placement;
}

bool PedalsSettingsModel::hasToShowBothHooks() const
{
    return m_hasToShowBothHooks;
}

void PedalsSettingsModel::setHasToShowBothHooks(bool hasToShowBothHooks)
{
    if (m_hasToShowBothHooks == hasToShowBothHooks)
        return;

    m_hasToShowBothHooks = hasToShowBothHooks;
    emit hasToShowBothHooksChanged(m_hasToShowBothHooks);
}
