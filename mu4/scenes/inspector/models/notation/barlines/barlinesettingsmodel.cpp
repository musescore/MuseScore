#include "barlinesettingsmodel.h"
#include "types/barlinetypes.h"
#include "barline.h"

BarlineSettingsModel::BarlineSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_BARLINE);
    setTitle(tr("Barline"));
    createProperties();
}

void BarlineSettingsModel::createProperties()
{
    m_type = buildPropertyItem(Ms::Pid::BARLINE_TYPE);
    m_isSpanToNextStaff = buildPropertyItem(Ms::Pid::BARLINE_SPAN);
    m_spanFrom = buildPropertyItem(Ms::Pid::BARLINE_SPAN_FROM);
    m_spanTo = buildPropertyItem(Ms::Pid::BARLINE_SPAN_TO);
    m_hasToShowTips = buildPropertyItem(Ms::Pid::BARLINE_SHOW_TIPS);
}

void BarlineSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BAR_LINE);
}

void BarlineSettingsModel::loadProperties()
{
    loadPropertyItem(m_type, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt();
    });

    loadPropertyItem(m_isSpanToNextStaff, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toBool();
    });

    loadPropertyItem(m_spanFrom, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt();
    });

    loadPropertyItem(m_spanTo, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt();
    });

    loadPropertyItem(m_hasToShowTips);
}

void BarlineSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_isSpanToNextStaff->resetToDefault();
    m_spanFrom->resetToDefault();
    m_spanTo->resetToDefault();
    m_hasToShowTips->resetToDefault();
}

void BarlineSettingsModel::applyToAllStaffs()
{
    onPropertyValueChanged(Ms::Pid::STAFF_BARLINE_SPAN_FROM, m_spanFrom->value());
    onPropertyValueChanged(Ms::Pid::STAFF_BARLINE_SPAN_TO, m_spanTo->value());
}

void BarlineSettingsModel::applySpanPreset(const int presetType)
{
    BarlineTypes::SpanPreset type = static_cast<BarlineTypes::SpanPreset>(presetType);
    switch (type) {
    case BarlineTypes::SpanPreset::PRESET_DEFAULT:
        m_isSpanToNextStaff->resetToDefault();
        m_spanFrom->resetToDefault();
        m_spanTo->resetToDefault();
        break;
    case BarlineTypes::SpanPreset::PRESET_TICK_1:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_TICK1_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_TICK1_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_TICK_2:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_TICK2_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_TICK2_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_SHORT_1:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_SHORT1_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_SHORT1_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_SHORT_2:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_SHORT2_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_SHORT2_TO);
        break;
    default:
        break;
    }
}

PropertyItem* BarlineSettingsModel::type() const
{
    return m_type;
}

PropertyItem* BarlineSettingsModel::isSpanToNextStaff() const
{
    return m_isSpanToNextStaff;
}

PropertyItem* BarlineSettingsModel::spanFrom() const
{
    return m_spanFrom;
}

PropertyItem* BarlineSettingsModel::spanTo() const
{
    return m_spanTo;
}

PropertyItem* BarlineSettingsModel::hasToShowTips() const
{
    return m_hasToShowTips;
}
