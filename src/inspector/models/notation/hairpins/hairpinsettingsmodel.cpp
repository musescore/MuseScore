#include "hairpinsettingsmodel.h"

#include <QPointF>

#include "types/hairpintypes.h"
#include "hairpin.h"
#include "dataformatter.h"

HairpinSettingsModel::HairpinSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_HAIRPIN);
    setTitle(tr("Hairpin"));
    createProperties();
}

void HairpinSettingsModel::createProperties()
{
    m_lineStyle = buildPropertyItem(Ms::Pid::LINE_STYLE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        updateLinePropertiesAvailability();
    });

    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);

    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_dashLineLength = buildPropertyItem(Ms::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Ms::Pid::DASH_GAP_LEN);
    m_height = buildPropertyItem(Ms::Pid::HAIRPIN_HEIGHT);
    m_continiousHeight = buildPropertyItem(Ms::Pid::HAIRPIN_CONT_HEIGHT);

    m_isDiagonalLocked = buildPropertyItem(Ms::Pid::DIAGONAL);
    m_isNienteCircleVisible = buildPropertyItem(Ms::Pid::HAIRPIN_CIRCLEDTIP);

    m_beginingText = buildPropertyItem(Ms::Pid::BEGIN_TEXT);
    m_beginingTextHorizontalOffset = buildPropertyItem(Ms::Pid::BEGIN_TEXT_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_beginingTextVerticalOffset->value().toDouble()));
    });

    m_beginingTextVerticalOffset = buildPropertyItem(Ms::Pid::BEGIN_TEXT_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_beginingTextHorizontalOffset->value().toDouble(), newValue.toDouble()));
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

void HairpinSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN, [](const Ms::Element* element) -> bool {
        const Ms::Hairpin* hairpin = Ms::toHairpin(element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == Ms::HairpinType::CRESC_HAIRPIN || hairpin->hairpinType() == Ms::HairpinType::DECRESC_HAIRPIN;
    });
}

void HairpinSettingsModel::loadProperties()
{
    auto formatDoubleFunc = [](const QVariant& elementPropertyValue) -> QVariant {
                                return DataFormatter::formatDouble(elementPropertyValue.toDouble());
                            };

    loadPropertyItem(m_lineStyle);
    loadPropertyItem(m_placement);

    loadPropertyItem(m_thickness, formatDoubleFunc);
    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);
    loadPropertyItem(m_height, formatDoubleFunc);
    loadPropertyItem(m_continiousHeight, formatDoubleFunc);

    loadPropertyItem(m_isDiagonalLocked);
    loadPropertyItem(m_isNienteCircleVisible);

    loadPropertyItem(m_beginingText);
    loadPropertyItem(m_beginingTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });
    loadPropertyItem(m_beginingTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });

    loadPropertyItem(m_continiousText);
    loadPropertyItem(m_continiousTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });
    loadPropertyItem(m_continiousTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });

    updateLinePropertiesAvailability();
}

void HairpinSettingsModel::resetProperties()
{
    m_lineStyle->resetToDefault();
    m_placement->resetToDefault();

    m_thickness->resetToDefault();
    m_dashLineLength->resetToDefault();
    m_dashGapLength->resetToDefault();
    m_height->resetToDefault();
    m_continiousHeight->resetToDefault();

    m_isDiagonalLocked->resetToDefault();
    m_isNienteCircleVisible->resetToDefault();

    m_beginingText->resetToDefault();
    m_beginingTextHorizontalOffset->resetToDefault();
    m_beginingTextVerticalOffset->resetToDefault();

    m_continiousText->resetToDefault();
    m_continiousTextHorizontalOffset->resetToDefault();
    m_continiousTextVerticalOffset->resetToDefault();
}

PropertyItem* HairpinSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* HairpinSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* HairpinSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* HairpinSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* HairpinSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* HairpinSettingsModel::isDiagonalLocked() const
{
    return m_isDiagonalLocked;
}

PropertyItem* HairpinSettingsModel::isNienteCircleVisible() const
{
    return m_isNienteCircleVisible;
}

PropertyItem* HairpinSettingsModel::beginingText() const
{
    return m_beginingText;
}

PropertyItem* HairpinSettingsModel::beginingTextHorizontalOffset() const
{
    return m_beginingTextHorizontalOffset;
}

PropertyItem* HairpinSettingsModel::beginingTextVerticalOffset() const
{
    return m_beginingTextVerticalOffset;
}

PropertyItem* HairpinSettingsModel::continiousText() const
{
    return m_continiousText;
}

PropertyItem* HairpinSettingsModel::continiousTextHorizontalOffset() const
{
    return m_continiousTextHorizontalOffset;
}

PropertyItem* HairpinSettingsModel::continiousTextVerticalOffset() const
{
    return m_continiousTextVerticalOffset;
}

void HairpinSettingsModel::updateLinePropertiesAvailability()
{
    HairpinTypes::LineStyle currentStyle = static_cast<HairpinTypes::LineStyle>(m_lineStyle->value().toInt());

    bool isAvailable = currentStyle == HairpinTypes::LineStyle::LINE_STYLE_CUSTOM;

    m_dashGapLength->setIsEnabled(isAvailable);
    m_dashLineLength->setIsEnabled(isAvailable);
}

PropertyItem* HairpinSettingsModel::height() const
{
    return m_height;
}

PropertyItem* HairpinSettingsModel::continiousHeight() const
{
    return m_continiousHeight;
}
