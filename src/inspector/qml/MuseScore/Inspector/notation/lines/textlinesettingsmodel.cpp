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

TextLineSettingsModel::TextLineSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                             IElementRepositoryService* repository, mu::engraving::ElementType elementType)
    : InspectorModelWithVoiceAndPositionOptions(parent, iocCtx, repository, elementType)
{
    setModelType(InspectorModelType::TYPE_TEXT_LINE);
    setTitle(muse::qtrc("inspector", "Text line"));
    setIcon(muse::ui::IconCode::Code::TEXT_BELOW_STAFF);

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

    m_startLineArrowHeight = buildPropertyItem(Pid::BEGIN_LINE_ARROW_HEIGHT);
    m_startLineArrowWidth = buildPropertyItem(Pid::BEGIN_LINE_ARROW_WIDTH);
    m_endLineArrowHeight = buildPropertyItem(Pid::END_LINE_ARROW_HEIGHT);
    m_endLineArrowWidth = buildPropertyItem(Pid::END_LINE_ARROW_WIDTH);
    m_startFilledArrowHeight = buildPropertyItem(Pid::BEGIN_FILLED_ARROW_HEIGHT);
    m_startFilledArrowWidth = buildPropertyItem(Pid::BEGIN_FILLED_ARROW_WIDTH);
    m_endFilledArrowHeight = buildPropertyItem(Pid::END_FILLED_ARROW_HEIGHT);
    m_endFilledArrowWidth = buildPropertyItem(Pid::END_FILLED_ARROW_WIDTH);

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
        Pid::BEGIN_LINE_ARROW_HEIGHT,
        Pid::BEGIN_LINE_ARROW_WIDTH,
        Pid::END_LINE_ARROW_HEIGHT,
        Pid::END_LINE_ARROW_WIDTH,
        Pid::BEGIN_FILLED_ARROW_HEIGHT,
        Pid::BEGIN_FILLED_ARROW_WIDTH,
        Pid::END_FILLED_ARROW_HEIGHT,
        Pid::END_FILLED_ARROW_WIDTH,
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
    updatemeasurementUnits();

    updateStartAndEndHookTypes();
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
        m_startLineArrowHeight,
        m_startLineArrowWidth,
        m_endLineArrowHeight,
        m_endLineArrowWidth,
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

PropertyItem* TextLineSettingsModel::startLineArrowHeight() const
{
    return m_startLineArrowHeight;
}

PropertyItem* TextLineSettingsModel::startLineArrowWidth() const
{
    return m_startLineArrowWidth;
}

PropertyItem* TextLineSettingsModel::endLineArrowHeight() const
{
    return m_endLineArrowHeight;
}

PropertyItem* TextLineSettingsModel::endLineArrowWidth() const
{
    return m_endLineArrowWidth;
}

PropertyItem* TextLineSettingsModel::startFilledArrowHeight() const
{
    return m_startFilledArrowHeight;
}

PropertyItem* TextLineSettingsModel::startFilledArrowWidth() const
{
    return m_startFilledArrowWidth;
}

PropertyItem* TextLineSettingsModel::endFilledArrowHeight() const
{
    return m_endFilledArrowHeight;
}

PropertyItem* TextLineSettingsModel::endFilledArrowWidth() const
{
    return m_endFilledArrowWidth;
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

bool TextLineSettingsModel::showStartHookHeight() const
{
    return m_showStartHookHeight;
}

bool TextLineSettingsModel::showEndHookHeight() const
{
    return m_showEndHookHeight;
}

bool TextLineSettingsModel::showStartArrowSettings() const
{
    return m_showStartArrowSettings;
}

bool TextLineSettingsModel::showEndArrowSettings() const
{
    return m_showEndArrowSettings;
}

bool TextLineSettingsModel::startFilledArrow() const
{
    return m_startFilledArrow;
}

bool TextLineSettingsModel::endFilledArrow() const
{
    return m_endFilledArrow;
}

QVariantList TextLineSettingsModel::hookTypesToObjList(const QList<HookTypeInfo>& types, bool start) const
{
    QVariantList result;

    for (HookTypeInfo typeInfo : types) {
        QVariantMap obj;
        obj["id"] = typeInfo.type;
        obj["icon"] = static_cast<int>(typeInfo.icon);
        obj["wideIcon"] = true;
        obj["title"] = typeInfo.title;
        obj["checkable"] = true;
        obj["checked"] = (start ? startHookType()->value().toInt() : endHookType()->value().toInt()) == typeInfo.type;

        result << obj;
    }

    return result;
}

void TextLineSettingsModel::onUpdateLinePropertiesAvailability()
{
    auto hasHook = [](const PropertyItem* item) {
        HookType type = static_cast<HookType>(item->value().toInt());
        return type != HookType::NONE && type != HookType::ARROW && type != HookType::ARROW_FILLED && type != HookType::ROSETTE;
    };
    auto hasLineArrow = [](const PropertyItem* item) {
        HookType type = static_cast<HookType>(item->value().toInt());
        return type == HookType::ARROW;
    };
    auto hasFilledArrow = [](const PropertyItem* item) {
        HookType type = static_cast<HookType>(item->value().toInt());
        return type == HookType::ARROW_FILLED;
    };

    bool isLineAvailable = m_isLineVisible->value().toBool();

    m_showStartHookHeight = hasHook(m_startHookType);
    emit showStartHookHeightChanged(m_showStartHookHeight);
    m_showEndHookHeight = hasHook(m_endHookType);
    emit showEndHookHeightChanged(m_showEndHookHeight);
    m_showStartArrowSettings = hasLineArrow(m_startHookType) || hasFilledArrow(m_startHookType);
    emit showStartArrowSettingsChanged(m_showStartArrowSettings);
    m_showEndArrowSettings = hasLineArrow(m_endHookType) || hasFilledArrow(m_endHookType);
    emit showEndArrowSettingsChanged(m_showEndArrowSettings);

    m_startFilledArrow = hasFilledArrow(m_startHookType) && !hasLineArrow(m_startHookType);
    emit startFilledArrowChanged(m_startFilledArrow);
    m_endFilledArrow = hasFilledArrow(m_endHookType) && !hasLineArrow(m_endHookType);
    emit endFilledArrowChanged(m_endFilledArrow);

    m_startHookType->setIsEnabled(isLineAvailable);
    m_endHookType->setIsEnabled(isLineAvailable);
    m_startHookHeight->setIsEnabled(isLineAvailable && m_showStartHookHeight);
    m_endHookHeight->setIsEnabled(isLineAvailable && m_showEndHookHeight);
    m_startLineArrowHeight->setIsEnabled(isLineAvailable && m_showStartArrowSettings);
    m_startLineArrowWidth->setIsEnabled(isLineAvailable && m_showStartArrowSettings);
    m_endLineArrowHeight->setIsEnabled(isLineAvailable && m_showEndArrowSettings);
    m_endLineArrowWidth->setIsEnabled(isLineAvailable && m_showEndArrowSettings);
    m_startFilledArrowHeight->setIsEnabled(isLineAvailable && m_showStartArrowSettings);
    m_startFilledArrowWidth->setIsEnabled(isLineAvailable && m_showStartArrowSettings);
    m_endFilledArrowHeight->setIsEnabled(isLineAvailable && m_showEndArrowSettings);
    m_endFilledArrowWidth->setIsEnabled(isLineAvailable && m_showEndArrowSettings);
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

void TextLineSettingsModel::updateStartAndEndHookTypes()
{
    static const QList<HookTypeInfo> startHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, muse::qtrc("inspector", "Normal", "hook type") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_START_HOOK, muse::qtrc("inspector", "Hooked 90°", "hook type") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_START_HOOK, muse::qtrc("inspector", "Hooked 45°", "hook type") },
        { mu::engraving::HookType::HOOK_90T, IconCode::LINE_WITH_T_LINE_START_HOOK,
          muse::qtrc("inspector", "Hooked 90° T-style", "hook type") },
        { mu::engraving::HookType::ARROW, IconCode::LINE_ARROW_LEFT, muse::qtrc("inspector", "Line arrow", "hook type") },
        { mu::engraving::HookType::ARROW_FILLED, IconCode::FILLED_ARROW_LEFT, muse::qtrc("inspector", "Filled arrow", "hook type") }
    };

    setPossibleStartHookTypes(startHookTypes);

    static const QList<HookTypeInfo> endHookTypes {
        { mu::engraving::HookType::NONE, IconCode::LINE_NORMAL, muse::qtrc("inspector", "Normal", "hook type") },
        { mu::engraving::HookType::HOOK_90, IconCode::LINE_WITH_END_HOOK, muse::qtrc("inspector", "Hooked 90°", "hook type") },
        { mu::engraving::HookType::HOOK_45, IconCode::LINE_WITH_ANGLED_END_HOOK, muse::qtrc("inspector", "Hooked 45°", "hook type") },
        { mu::engraving::HookType::HOOK_90T, IconCode::LINE_WITH_T_LIKE_END_HOOK,
          muse::qtrc("inspector", "Hooked 90° T-style", "hook type") },
        { mu::engraving::HookType::ARROW, IconCode::LINE_ARROW_RIGHT, muse::qtrc("inspector", "Line arrow", "hook type") },
        { mu::engraving::HookType::ARROW_FILLED, IconCode::FILLED_ARROW_RIGHT, muse::qtrc("inspector", "Filled arrow", "hook type") }
    };

    setPossibleEndHookTypes(endHookTypes);
}

void TextLineSettingsModel::setPossibleStartHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleStartHookTypes = hookTypesToObjList(types, true);
    emit possibleStartHookTypesChanged(m_possibleStartHookTypes);
}

void TextLineSettingsModel::setPossibleEndHookTypes(const QList<HookTypeInfo>& types)
{
    m_possibleEndHookTypes = hookTypesToObjList(types, false);
    emit possibleEndHookTypesChanged(m_possibleEndHookTypes);
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

    if (muse::contains(propertyIdSet, Pid::BEGIN_LINE_ARROW_HEIGHT)) {
        loadPropertyItem(m_startLineArrowHeight);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_LINE_ARROW_WIDTH)) {
        loadPropertyItem(m_startLineArrowWidth);
    }

    if (muse::contains(propertyIdSet, Pid::END_LINE_ARROW_HEIGHT)) {
        loadPropertyItem(m_endLineArrowHeight);
    }

    if (muse::contains(propertyIdSet, Pid::END_LINE_ARROW_WIDTH)) {
        loadPropertyItem(m_endLineArrowWidth);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_FILLED_ARROW_HEIGHT)) {
        loadPropertyItem(m_startFilledArrowHeight);
    }

    if (muse::contains(propertyIdSet, Pid::BEGIN_FILLED_ARROW_WIDTH)) {
        loadPropertyItem(m_startFilledArrowWidth);
    }

    if (muse::contains(propertyIdSet, Pid::END_FILLED_ARROW_HEIGHT)) {
        loadPropertyItem(m_endFilledArrowHeight);
    }

    if (muse::contains(propertyIdSet, Pid::END_FILLED_ARROW_WIDTH)) {
        loadPropertyItem(m_endFilledArrowWidth);
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
