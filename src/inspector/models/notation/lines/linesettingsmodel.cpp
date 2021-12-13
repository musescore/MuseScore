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
#include "linesettingsmodel.h"

#include "dataformatter.h"
#include "log.h"

#include "types/linetypes.h"

using namespace mu::inspector;
using namespace mu::engraving;

LineSettingsModel::LineSettingsModel(QObject* parent, IElementRepositoryService* repository, Ms::ElementType elementType)
    : AbstractInspectorModel(parent, repository, elementType)
{
    createProperties();
}

void LineSettingsModel::createProperties()
{
    auto applyPropertyValueAndUpdateAvailability = [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        onUpdateLinePropertiesAvailability();
    };

    m_lineStyle = buildPropertyItem(Ms::Pid::LINE_STYLE, applyPropertyValueAndUpdateAvailability);
    m_isLineVisible = buildPropertyItem(Ms::Pid::LINE_VISIBLE, applyPropertyValueAndUpdateAvailability);

    m_startHookType = buildPropertyItem(Ms::Pid::BEGIN_HOOK_TYPE, applyPropertyValueAndUpdateAvailability);
    m_endHookType = buildPropertyItem(Ms::Pid::END_HOOK_TYPE, applyPropertyValueAndUpdateAvailability);

    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);

    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
    m_dashLineLength = buildPropertyItem(Ms::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Ms::Pid::DASH_GAP_LEN);

    m_allowDiagonal = buildPropertyItem(Ms::Pid::DIAGONAL);

    m_hookHeight = buildPropertyItem(Ms::Pid::END_HOOK_HEIGHT, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        onPropertyValueChanged(Ms::Pid::BEGIN_HOOK_HEIGHT, newValue);
    });

    if (isTextVisible(BeginingText)) {
        m_beginingText = buildPropertyItem(Ms::Pid::BEGIN_TEXT);
        m_beginingTextHorizontalOffset = buildPropertyItem(Ms::Pid::BEGIN_TEXT_OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_beginingTextVerticalOffset->value().toDouble()));
        });

        m_beginingTextVerticalOffset = buildPropertyItem(Ms::Pid::BEGIN_TEXT_OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(m_beginingTextHorizontalOffset->value().toDouble(), newValue.toDouble()));
        });
    }

    if (isTextVisible(ContiniousText)) {
        m_continiousText = buildPropertyItem(Ms::Pid::CONTINUE_TEXT);
        m_continiousTextHorizontalOffset
            = buildPropertyItem(Ms::Pid::CONTINUE_TEXT_OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_continiousTextVerticalOffset->value().toDouble()));
        });

        m_continiousTextVerticalOffset = buildPropertyItem(Ms::Pid::CONTINUE_TEXT_OFFSET, [this](const Ms::Pid pid,
                                                                                                 const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(m_continiousTextHorizontalOffset->value().toDouble(), newValue.toDouble()));
        });
    }

    if (isTextVisible(EndText)) {
        m_endText = buildPropertyItem(Ms::Pid::END_TEXT);
        m_endTextHorizontalOffset = buildPropertyItem(Ms::Pid::END_TEXT_OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_endTextHorizontalOffset->value().toDouble()));
        });

        m_endTextVerticalOffset = buildPropertyItem(Ms::Pid::END_TEXT_OFFSET, [this](const Ms::Pid pid, const QVariant& newValue) {
            onPropertyValueChanged(pid, QPointF(m_endTextVerticalOffset->value().toDouble(), newValue.toDouble()));
        });
    }
}

void LineSettingsModel::loadProperties()
{
    auto formatDoubleFunc = [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble());
    };

    loadPropertyItem(m_lineStyle);
    loadPropertyItem(m_placement);

    loadPropertyItem(m_thickness, formatDoubleFunc);
    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);

    loadPropertyItem(m_isLineVisible);
    loadPropertyItem(m_allowDiagonal);

    loadPropertyItem(m_startHookType);
    loadPropertyItem(m_endHookType);
    loadPropertyItem(m_hookHeight);

    loadPropertyItem(m_beginingText);
    loadPropertyItem(m_beginingTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });
    loadPropertyItem(m_beginingTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    loadPropertyItem(m_continiousText);
    loadPropertyItem(m_continiousTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });
    loadPropertyItem(m_continiousTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    loadPropertyItem(m_endText);
    loadPropertyItem(m_endTextHorizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });
    loadPropertyItem(m_endTextVerticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    onUpdateLinePropertiesAvailability();
}

void LineSettingsModel::resetProperties()
{
    QList<PropertyItem*> allProperties {
        m_lineStyle,
        m_placement,
        m_thickness,
        m_dashLineLength,
        m_dashGapLength,
        m_isLineVisible,
        m_allowDiagonal,
        m_startHookType,
        m_endHookType,
        m_hookHeight,
        m_beginingText,
        m_beginingTextHorizontalOffset,
        m_beginingTextVerticalOffset,
        m_continiousText,
        m_continiousTextHorizontalOffset,
        m_continiousTextVerticalOffset,
        m_endText,
        m_endTextHorizontalOffset,
        m_endTextVerticalOffset
    };

    for (PropertyItem* property : allProperties) {
        if (property) {
            property->resetToDefault();
        }
    }
}

PropertyItem* LineSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* LineSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* LineSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* LineSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* LineSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* LineSettingsModel::isLineVisible() const
{
    return m_isLineVisible;
}

PropertyItem* LineSettingsModel::allowDiagonal() const
{
    return m_allowDiagonal;
}

PropertyItem* LineSettingsModel::startHookType() const
{
    return m_startHookType;
}

PropertyItem* LineSettingsModel::endHookType() const
{
    return m_endHookType;
}

PropertyItem* LineSettingsModel::hookHeight() const
{
    return m_hookHeight;
}

PropertyItem* LineSettingsModel::beginingText() const
{
    return m_beginingText;
}

PropertyItem* LineSettingsModel::beginingTextHorizontalOffset() const
{
    return m_beginingTextHorizontalOffset;
}

PropertyItem* LineSettingsModel::beginingTextVerticalOffset() const
{
    return m_beginingTextVerticalOffset;
}

PropertyItem* LineSettingsModel::continiousText() const
{
    return m_continiousText;
}

PropertyItem* LineSettingsModel::continiousTextHorizontalOffset() const
{
    return m_continiousTextHorizontalOffset;
}

PropertyItem* LineSettingsModel::continiousTextVerticalOffset() const
{
    return m_continiousTextVerticalOffset;
}

PropertyItem* LineSettingsModel::endText() const
{
    return m_endText;
}

PropertyItem* LineSettingsModel::endTextHorizontalOffset() const
{
    return m_endTextHorizontalOffset;
}

PropertyItem* LineSettingsModel::endTextVerticalOffset() const
{
    return m_endTextVerticalOffset;
}

QVariantList LineSettingsModel::possibleStartHookTypes() const
{
    return m_possibleStartHookTypes;
}

QVariantList LineSettingsModel::possibleEndHookTypes() const
{
    return m_possibleEndHookTypes;
}

QVariantList LineSettingsModel::hookTypesToObjList(const QList<HookTypeInfo>& types) const
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

void LineSettingsModel::onUpdateLinePropertiesAvailability()
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
    bool areDashPropertiesAvailable = currentStyle == LineTypes::LineStyle::LINE_STYLE_CUSTOM;

    m_dashLineLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
    m_dashGapLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
}

bool LineSettingsModel::isTextVisible(TextType type) const
{
    //! NOTE: the end text is hidden for most lines by default
    return type != TextType::EndText;
}

void LineSettingsModel::setPossibleStartHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleStartHookTypes = hookTypesToObjList(types);
}

void LineSettingsModel::setPossibleEndHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleEndHookTypes = hookTypesToObjList(types);
}
