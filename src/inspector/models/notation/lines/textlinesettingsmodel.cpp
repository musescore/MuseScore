/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

using IconCode = muse::ui::IconCode::Code;

TextLineSettingsModel::TextLineSettingsModel(QObject* parent, IElementRepositoryService* repository, mu::engraving::ElementType elementType)
    : InspectorModelWithVoiceAndPositionOptions(parent, repository, elementType)
{
    setModelType(InspectorModelType::TYPE_TEXT_LINE);
    setTitle(muse::qtrc("inspector", "Text line"));
    setIcon(muse::ui::IconCode::Code::TEXT_BELOW_STAFF);

    static const QList<HookTypeInfo> startHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, muse::qtrc("inspector", "Normal") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_START_HOOK, muse::qtrc("inspector", "Hooked 90°") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_START_HOOK, muse::qtrc("inspector", "Hooked 45°") },
        { mu::engraving::HookType::HOOK_90T, IconCode::LINE_WITH_T_LINE_START_HOOK, muse::qtrc("inspector", "Hooked 90° T-style") }
    };

    setPossibleStartHookTypes(startHookTypes);

    static const QList<HookTypeInfo> endHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, muse::qtrc("inspector", "Normal") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, muse::qtrc("inspector", "Hooked 90°") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, muse::qtrc("inspector", "Hooked 45°") },
        { mu::engraving::HookType::HOOK_90T, IconCode::LINE_WITH_T_LIKE_END_HOOK, muse::qtrc("inspector", "Hooked 90° T-style") }
    };

    setPossibleEndHookTypes(endHookTypes);

    createProperties();
}

void TextLineSettingsModel::createProperties()
{
    InspectorModelWithVoiceAndPositionOptions::createProperties();

    auto applyPropertyValueAndUpdateAvailability = [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        onUpdateLinePropertiesAvailability();
    };

    m_isLineVisible = buildPropertyItem(Pid::LINE_VISIBLE, applyPropertyValueAndUpdateAvailability);
    m_isLineVisible->setIsVisible(false);

    m_allowDiagonal = buildPropertyItem(Pid::DIAGONAL);
    m_allowDiagonal->setIsVisible(false);

    m_lineStyle = buildPropertyItem(Pid::LINE_STYLE, applyPropertyValueAndUpdateAvailability);

    m_startHookType = buildPropertyItem(Pid::BEGIN_HOOK_TYPE, applyPropertyValueAndUpdateAvailability);
    m_endHookType = buildPropertyItem(Pid::END_HOOK_TYPE, applyPropertyValueAndUpdateAvailability);

    m_startHookHeight = buildPropertyItem(Pid::BEGIN_HOOK_HEIGHT);
    m_endHookHeight = buildPropertyItem(Pid::END_HOOK_HEIGHT);

    m_gapBetweenTextAndLine = buildPropertyItem(Pid::GAP_BETWEEN_TEXT_AND_LINE, applyPropertyValueAndUpdateAvailability);

    m_thickness = buildPropertyItem(Pid::LINE_WIDTH);
    m_dashLineLength = buildPropertyItem(Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Pid::DASH_GAP_LEN);

    m_placement = buildPropertyItem(Pid::PLACEMENT);

    m_beginningText = buildPropertyItem(Pid::BEGIN_TEXT, applyPropertyValueAndUpdateAvailability);
    m_beginningTextOffset = buildPointFPropertyItem(Pid::BEGIN_TEXT_OFFSET);

    m_continuousText = buildPropertyItem(Pid::CONTINUE_TEXT, applyPropertyValueAndUpdateAvailability);
    m_continuousTextOffset = buildPointFPropertyItem(Pid::CONTINUE_TEXT_OFFSET);

    m_endText = buildPropertyItem(Pid::END_TEXT, applyPropertyValueAndUpdateAvailability);
    m_endTextOffset = buildPointFPropertyItem(Pid::END_TEXT_OFFSET);
}

void TextLineSettingsModel::loadProperties()
{
    InspectorModelWithVoiceAndPositionOptions::loadProperties();

    static const PropertyIdSet propertyIdSet {
        Pid::LINE_VISIBLE,
        Pid::DIAGONAL,
        Pid::LINE_STYLE,
        Pid::BEGIN_HOOK_TYPE,
        Pid::END_HOOK_TYPE,
        Pid::LINE_WIDTH,
        Pid::DASH_LINE_LEN,
        Pid::DASH_GAP_LEN,
        Pid::END_HOOK_HEIGHT,
        Pid::BEGIN_HOOK_HEIGHT,
        Pid::GAP_BETWEEN_TEXT_AND_LINE,
        Pid::PLACEMENT,
        Pid::BEGIN_TEXT,
        Pid::BEGIN_TEXT_OFFSET,
        Pid::CONTINUE_TEXT,
        Pid::CONTINUE_TEXT_OFFSET,
        Pid::END_TEXT,
        Pid::END_TEXT_OFFSET,
    };

    loadProperties(propertyIdSet);

    updateIsSystemObjectBelowBottomStaff();
}

void TextLineSettingsModel::resetProperties()
{
    InspectorModelWithVoiceAndPositionOptions::resetProperties();

    QList<PropertyItem*> allProperties {
        m_isLineVisible,
        m_allowDiagonal,
        m_lineStyle,
        m_thickness,
        m_dashLineLength,
        m_dashGapLength,
        m_startHookType,
        m_endHookType,
        m_startHookHeight,
        m_endHookHeight,
        m_gapBetweenTextAndLine,
        m_placement,
        m_beginningText,
        m_beginningTextOffset,
        m_continuousText,
        m_continuousTextOffset,
        m_endText,
        m_endTextOffset
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

PropertyItem* TextLineSettingsModel::startHookHeight() const
{
    return m_startHookHeight;
}

PropertyItem* TextLineSettingsModel::endHookHeight() const
{
    return m_endHookHeight;
}

PropertyItem* TextLineSettingsModel::gapBetweenTextAndLine() const
{
    return m_gapBetweenTextAndLine;
}

PropertyItem* TextLineSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* TextLineSettingsModel::beginningText() const
{
    return m_beginningText;
}

PropertyItem* TextLineSettingsModel::beginningTextOffset() const
{
    return m_beginningTextOffset;
}

PropertyItem* TextLineSettingsModel::continuousText() const
{
    return m_continuousText;
}

PropertyItem* TextLineSettingsModel::continuousTextOffset() const
{
    return m_continuousTextOffset;
}

PropertyItem* TextLineSettingsModel::endText() const
{
    return m_endText;
}

PropertyItem* TextLineSettingsModel::endTextOffset() const
{
    return m_endTextOffset;
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
    m_startHookHeight->setIsEnabled(isLineAvailable && hasStartHook);
    m_endHookHeight->setIsEnabled(isLineAvailable && hasEndHook);
    m_lineStyle->setIsEnabled(isLineAvailable);
    m_thickness->setIsEnabled(isLineAvailable);

    m_gapBetweenTextAndLine->setIsEnabled(isLineAvailable);

    auto currentStyle = static_cast<LineTypes::LineStyle>(m_lineStyle->value().toInt());
    bool areDashPropertiesAvailable = currentStyle == LineTypes::LineStyle::LINE_STYLE_DASHED;

    m_dashLineLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
    m_dashGapLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);

    bool hasBeginText = !m_beginningText->value().toString().isEmpty();
    bool hasContinueText = !m_continuousText->value().toString().isEmpty();
    bool hasEndText = !m_endText->value().toString().isEmpty();

    m_beginningTextOffset->setIsEnabled(hasBeginText);
    m_continuousTextOffset->setIsEnabled(hasContinueText);
    m_endTextOffset->setIsEnabled(hasEndText);
    m_gapBetweenTextAndLine->setIsEnabled(isLineAvailable && (hasBeginText || hasContinueText || hasEndText));
}

void TextLineSettingsModel::setPossibleStartHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleStartHookTypes = hookTypesToObjList(types);
}

void TextLineSettingsModel::setPossibleEndHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleEndHookTypes = hookTypesToObjList(types);
}

void TextLineSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet& styleIdSet)
{
    loadProperties(changedPropertyIdSet);
    InspectorModelWithVoiceAndPositionOptions::onNotationChanged(changedPropertyIdSet, styleIdSet);
}

void TextLineSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::LINE_VISIBLE)) {
        loadPropertyItem(m_isLineVisible);
    }

    if (muse::contains(propertyIdSet, Pid::DIAGONAL)) {
        loadPropertyItem(m_allowDiagonal);
    }

    if (muse::contains(propertyIdSet, Pid::LINE_STYLE)) {
        loadPropertyItem(m_lineStyle);
    }

    if (muse::contains(propertyIdSet, Pid::LINE_WIDTH)) {
        loadPropertyItem(m_thickness, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::DASH_LINE_LEN)) {
        loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::DASH_GAP_LEN)) {
        loadPropertyItem(m_dashGapLength, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_HOOK_TYPE)) {
        loadPropertyItem(m_startHookType);
    }

    if (muse::contains(propertyIdSet, Pid::END_HOOK_TYPE)) {
        loadPropertyItem(m_endHookType);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_HOOK_HEIGHT)) {
        loadPropertyItem(m_startHookHeight);
    }

    if (muse::contains(propertyIdSet, Pid::END_HOOK_HEIGHT)) {
        loadPropertyItem(m_endHookHeight);
    }

    if (muse::contains(propertyIdSet, Pid::GAP_BETWEEN_TEXT_AND_LINE)) {
        loadPropertyItem(m_gapBetweenTextAndLine);
    }

    if (muse::contains(propertyIdSet, Pid::PLACEMENT)) {
        loadPropertyItem(m_placement);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_TEXT)) {
        loadPropertyItem(m_beginningText);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_TEXT_OFFSET)) {
        loadPropertyItem(m_beginningTextOffset);
    }

    if (muse::contains(propertyIdSet, Pid::CONTINUE_TEXT)) {
        loadPropertyItem(m_continuousText);
    }

    if (muse::contains(propertyIdSet, Pid::CONTINUE_TEXT_OFFSET)) {
        loadPropertyItem(m_continuousTextOffset);
    }

    if (muse::contains(propertyIdSet, Pid::END_TEXT)) {
        loadPropertyItem(m_endText);
    }

    if (muse::contains(propertyIdSet, Pid::END_TEXT_OFFSET)) {
        loadPropertyItem(m_endTextOffset);
    }

    onUpdateLinePropertiesAvailability();
}
