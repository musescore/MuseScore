#include "crescendosettingsmodel.h"

#include "hairpin.h"
#include "types/crescendotypes.h"
#include "dataformatter.h"

CrescendoSettingsModel::CrescendoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_CRESCENDO);
    setTitle(tr("Crescendo"));
    createProperties();
}

void CrescendoSettingsModel::createProperties()
{
    m_isLineVisible = buildPropertyItem(Ms::Pid::LINE_VISIBLE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        updateLinePropertiesAvailability();
    });

    m_endHookType = buildPropertyItem(Ms::Pid::END_HOOK_TYPE);
    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_hookHeight = buildPropertyItem(Ms::Pid::END_HOOK_HEIGHT);

    m_lineStyle = buildPropertyItem(Ms::Pid::LINE_STYLE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        updateLinePropertiesAvailability();
    });

    m_dashLineLength = buildPropertyItem(Ms::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Ms::Pid::DASH_GAP_LEN);
    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);

    m_beginningText = buildPropertyItem(Ms::Pid::BEGIN_TEXT);
    m_beginningTextHorizontalOffset = buildPropertyItem(Ms::Pid::BEGIN_TEXT_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_beginningTextVerticalOffset->value().toDouble()));
    });

    m_beginningTextVerticalOffset = buildPropertyItem(Ms::Pid::BEGIN_TEXT_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_beginningTextHorizontalOffset->value().toDouble(),
                                                                  newValue.toDouble()));
    });

    m_continiousText = buildPropertyItem(Ms::Pid::CONTINUE_TEXT);
    m_continiousTextHorizontalOffset = buildPropertyItem(Ms::Pid::CONTINUE_TEXT_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_continiousTextVerticalOffset->value().toDouble()));
    });

    m_continiousTextVerticalOffset = buildPropertyItem(Ms::Pid::CONTINUE_TEXT_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_continiousTextHorizontalOffset->value().toDouble(),
                                                                  newValue.toDouble()));
    });
}

void CrescendoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN, [](const Ms::Element* element) -> bool {
        const Ms::Hairpin* hairpin = Ms::toHairpin(element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == Ms::HairpinType::CRESC_LINE || hairpin->hairpinType() == Ms::HairpinType::DECRESC_LINE;
    });
}

void CrescendoSettingsModel::loadProperties()
{
    loadPropertyItem(m_isLineVisible);
    loadPropertyItem(m_endHookType);

    auto formatDoubleFunc = [](const QVariant& elementPropertyValue) -> QVariant {
                                return DataFormatter::formatDouble(elementPropertyValue.toDouble());
                            };

    loadPropertyItem(m_thickness, formatDoubleFunc);
    loadPropertyItem(m_hookHeight, formatDoubleFunc);
    loadPropertyItem(m_lineStyle);
    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);
    loadPropertyItem(m_placement);

    loadPropertyItem(m_beginningText);
    loadPropertyItem(m_beginningTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });
    loadPropertyItem(m_beginningTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    loadPropertyItem(m_continiousText);
    loadPropertyItem(m_continiousTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });
    loadPropertyItem(m_continiousTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    updateLinePropertiesAvailability();
}

void CrescendoSettingsModel::resetProperties()
{
    m_isLineVisible->resetToDefault();
    m_endHookType->resetToDefault();
    m_thickness->resetToDefault();
    m_hookHeight->resetToDefault();
    m_lineStyle->resetToDefault();
    m_dashLineLength->resetToDefault();
    m_dashGapLength->resetToDefault();
    m_placement->resetToDefault();

    m_beginningText->resetToDefault();
    m_beginningTextHorizontalOffset->resetToDefault();
    m_beginningTextVerticalOffset->resetToDefault();

    m_continiousText->resetToDefault();
    m_continiousTextHorizontalOffset->resetToDefault();
    m_continiousTextVerticalOffset->resetToDefault();
}

PropertyItem* CrescendoSettingsModel::isLineVisible() const
{
    return m_isLineVisible;
}

PropertyItem* CrescendoSettingsModel::endHookType() const
{
    return m_endHookType;
}

PropertyItem* CrescendoSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* CrescendoSettingsModel::hookHeight() const
{
    return m_hookHeight;
}

PropertyItem* CrescendoSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* CrescendoSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* CrescendoSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* CrescendoSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* CrescendoSettingsModel::beginningText() const
{
    return m_beginningText;
}

PropertyItem* CrescendoSettingsModel::beginningTextHorizontalOffset() const
{
    return m_beginningTextHorizontalOffset;
}

PropertyItem* CrescendoSettingsModel::beginningTextVerticalOffset() const
{
    return m_beginningTextVerticalOffset;
}

PropertyItem* CrescendoSettingsModel::continiousText() const
{
    return m_continiousText;
}

PropertyItem* CrescendoSettingsModel::continiousTextHorizontalOffset() const
{
    return m_continiousTextHorizontalOffset;
}

PropertyItem* CrescendoSettingsModel::continiousTextVerticalOffset() const
{
    return m_continiousTextVerticalOffset;
}

void CrescendoSettingsModel::updateLinePropertiesAvailability()
{
    bool isLineAvailable = m_isLineVisible->value().toBool();

    m_endHookType->setIsEnabled(isLineAvailable);
    m_thickness->setIsEnabled(isLineAvailable);
    m_hookHeight->setIsEnabled(isLineAvailable);
    m_lineStyle->setIsEnabled(isLineAvailable);

    CrescendoTypes::LineStyle currentStyle = static_cast<CrescendoTypes::LineStyle>(m_lineStyle->value().toInt());

    bool areDashPropertiesAvailable = currentStyle == CrescendoTypes::LineStyle::LINE_STYLE_CUSTOM;

    m_dashLineLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
    m_dashGapLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
}
