/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "textlinesettingsmodel.h"

#include "types/commontypes.h"
#include "types/linetypes.h"

#include "ui/view/iconcodes.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::engraving;

using IconCode = mu::ui::IconCode::Code;

TextLineSettingsModel::TextLineSettingsModel(QObject* parent, IElementRepositoryService* repository, mu::engraving::ElementType elementType)
    : AbstractInspectorModel(parent, repository, elementType)
{
    setModelType(InspectorModelType::TYPE_TEXT_LINE);
    setTitle(qtrc("inspector", "Text line"));

    static const QList<HookTypeInfo> endHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, qtrc("inspector", "Normal") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, qtrc("inspector", "Hooked 90") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, qtrc("inspector", "Hooked 45") },
        { mu::engraving::HookType::HOOK_90T, IconCode::LINE_WITH_T_LIKE_END_HOOK, qtrc("inspector", "Hooked 90 T-style") }
    };

    setPossibleEndHookTypes(endHookTypes);

    createProperties();
}

void TextLineSettingsModel::createProperties()
{
    auto applyPropertyValueAndUpdateAvailability = [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        onUpdateLinePropertiesAvailability();
    };

    m_isLineVisible = buildPropertyItem(mu::engraving::Pid::LINE_VISIBLE, applyPropertyValueAndUpdateAvailability);
    m_isLineVisible->setIsVisible(false);

    m_allowDiagonal = buildPropertyItem(mu::engraving::Pid::DIAGONAL);
    m_allowDiagonal->setIsVisible(false);

    m_lineStyle = buildPropertyItem(mu::engraving::Pid::LINE_STYLE, applyPropertyValueAndUpdateAvailability);

    m_startHookType = buildPropertyItem(mu::engraving::Pid::BEGIN_HOOK_TYPE, applyPropertyValueAndUpdateAvailability);
    m_endHookType = buildPropertyItem(mu::engraving::Pid::END_HOOK_TYPE, applyPropertyValueAndUpdateAvailability);

    m_thickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
    m_dashLineLength = buildPropertyItem(mu::engraving::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(mu::engraving::Pid::DASH_GAP_LEN);

    m_hookHeight = buildPropertyItem(mu::engraving::Pid::END_HOOK_HEIGHT, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        onPropertyValueChanged(mu::engraving::Pid::BEGIN_HOOK_HEIGHT, newValue);
    });

    m_placement = buildPropertyItem(mu::engraving::Pid::PLACEMENT);
    m_placement->setIsVisible(false);

    if (isTextVisible(BeginningText)) {
        m_beginningText = buildPropertyItem(mu::engraving::Pid::BEGIN_TEXT);

        m_beginningTextVerticalOffset
            = buildPropertyItem(mu::engraving::Pid::BEGIN_TEXT_OFFSET, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(0, newValue.toDouble()));
        });
    }

    if (isTextVisible(ContinuousText)) {
        m_continuousText = buildPropertyItem(mu::engraving::Pid::CONTINUE_TEXT);

        m_continuousTextVerticalOffset
            = buildPropertyItem(mu::engraving::Pid::CONTINUE_TEXT_OFFSET, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(0,
                                                newValue.toDouble()));
        });
    }

    if (isTextVisible(EndText)) {
        m_endText = buildPropertyItem(mu::engraving::Pid::END_TEXT);

        m_endTextVerticalOffset
            = buildPropertyItem(mu::engraving::Pid::END_TEXT_OFFSET, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(0, newValue.toDouble()));
        });
    }
}

void TextLineSettingsModel::loadProperties()
{
    loadPropertyItem(m_isLineVisible);
    loadPropertyItem(m_allowDiagonal);

    loadPropertyItem(m_lineStyle);

    loadPropertyItem(m_thickness, formatDoubleFunc);
    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);

    loadPropertyItem(m_startHookType);
    loadPropertyItem(m_endHookType);
    loadPropertyItem(m_hookHeight);

    loadPropertyItem(m_placement);

    loadPropertyItem(m_beginningText);
    loadPropertyItem(m_beginningTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    loadPropertyItem(m_continuousText);
    loadPropertyItem(m_continuousTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    loadPropertyItem(m_endText);
    loadPropertyItem(m_endTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    onUpdateLinePropertiesAvailability();
}

void TextLineSettingsModel::resetProperties()
{
    QList<PropertyItem*> allProperties {
        m_isLineVisible,
        m_allowDiagonal,
        m_lineStyle,
        m_thickness,
        m_dashLineLength,
        m_dashGapLength,
        m_startHookType,
        m_endHookType,
        m_hookHeight,
        m_placement,
        m_beginningText,
        m_beginningTextVerticalOffset,
        m_continuousText,
        m_continuousTextVerticalOffset,
        m_endText,
        m_endTextVerticalOffset
    };

    for (PropertyItem* property : allProperties) {
        if (property) {
            property->resetToDefault();
        }
    }
}

PropertyItem* TextLineSettingsModel::isLineVisible() const
{
    return m_isLineVisible;
}

PropertyItem* TextLineSettingsModel::allowDiagonal() const
{
    return m_allowDiagonal;
}

PropertyItem* TextLineSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* TextLineSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* TextLineSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* TextLineSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* TextLineSettingsModel::startHookType() const
{
    return m_startHookType;
}

PropertyItem* TextLineSettingsModel::endHookType() const
{
    return m_endHookType;
}

PropertyItem* TextLineSettingsModel::hookHeight() const
{
    return m_hookHeight;
}

PropertyItem* TextLineSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* TextLineSettingsModel::beginningText() const
{
    return m_beginningText;
}

PropertyItem* TextLineSettingsModel::beginningTextVerticalOffset() const
{
    return m_beginningTextVerticalOffset;
}

PropertyItem* TextLineSettingsModel::continuousText() const
{
    return m_continuousText;
}

PropertyItem* TextLineSettingsModel::continuousTextVerticalOffset() const
{
    return m_continuousTextVerticalOffset;
}

PropertyItem* TextLineSettingsModel::endText() const
{
    return m_endText;
}

PropertyItem* TextLineSettingsModel::endTextVerticalOffset() const
{
    return m_endTextVerticalOffset;
}

QVariantList TextLineSettingsModel::possibleStartHookTypes() const
{
    return m_possibleStartHookTypes;
}

QVariantList TextLineSettingsModel::possibleEndHookTypes() const
{
    return m_possibleEndHookTypes;
}

QVariantList TextLineSettingsModel::hookTypesToObjList(const QList<HookTypeInfo>& types) const
{
    QVariantList result;

    for (HookTypeInfo typeInfo : types) {
        QVariantMap obj;
        obj["value"] = typeInfo.type;
        obj["iconCode"] = static_cast<int>(typeInfo.icon);
        obj["title"] = typeInfo.title;

        result << obj;
    }

    return result;
}

void TextLineSettingsModel::onUpdateLinePropertiesAvailability()
{
    auto hasHook = [](const PropertyItem* item) {
        return static_cast<HookType>(item->value().toInt()) != HookType::NONE;
    };

    bool isLineAvailable = m_isLineVisible->value().toBool();
    bool hasStartHook = hasHook(m_startHookType);
    bool hasEndHook = hasHook(m_endHookType);

    m_startHookType->setIsEnabled(isLineAvailable);
    m_endHookType->setIsEnabled(isLineAvailable);
    m_thickness->setIsEnabled(isLineAvailable);
    m_hookHeight->setIsEnabled(isLineAvailable && (hasStartHook || hasEndHook));
    m_lineStyle->setIsEnabled(isLineAvailable);

    auto currentStyle = static_cast<LineTypes::LineStyle>(m_lineStyle->value().toInt());
    bool areDashPropertiesAvailable = currentStyle == LineTypes::LineStyle::LINE_STYLE_DASHED;

    m_dashLineLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
    m_dashGapLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
}

bool TextLineSettingsModel::isTextVisible(TextType type) const
{
    //! NOTE: the end text is hidden for most lines by default
    return type != TextType::EndText;
}

void TextLineSettingsModel::setPossibleStartHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleStartHookTypes = hookTypesToObjList(types);
}

void TextLineSettingsModel::setPossibleEndHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleEndHookTypes = hookTypesToObjList(types);
}

void TextLineSettingsModel::updatePropertiesOnNotationChanged()
{
    loadProperties();
}
