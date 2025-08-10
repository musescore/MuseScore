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
#include "textsettingsmodel.h"

#include <QFont>

#include "types/commontypes.h"
#include "types/texttypes.h"

#include "engraving/dom/dynamic.h"
#include "engraving/dom/textbase.h"
#include "engraving/types/typesconv.h"

#include "translation.h"
#include "log.h"
#include "realfn.h"

using namespace mu::inspector;
using namespace mu::engraving;

TextSettingsModel::TextSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_TEXT);
    setTitle(muse::qtrc("inspector", "Text"));
    createProperties();

    isTextEditingChanged().onNotify(this, [this]() {
        loadProperties();
        setIsSpecialCharactersInsertionAvailable(isTextEditingStarted());
    });

    setIsSpecialCharactersInsertionAvailable(isTextEditingStarted());
}

void TextSettingsModel::createProperties()
{
    m_fontFamily = buildPropertyItem(mu::engraving::Pid::FONT_FACE);
    m_fontStyle = buildPropertyItem(mu::engraving::Pid::FONT_STYLE);
    m_fontSize = buildPropertyItem(mu::engraving::Pid::FONT_SIZE);
    m_textLineSpacing = buildPropertyItem(mu::engraving::Pid::TEXT_LINE_SPACING);

    m_horizontalAlignment = buildPropertyItem(mu::engraving::Pid::ALIGN, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QVariantList({ newValue.toInt(), m_verticalAlignment->value().toInt() }));
    }, [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, QVariantList({ newValue.toInt(), m_verticalAlignment->value().toInt() }));

        emit requestReloadPropertyItems();
    });
    m_verticalAlignment = buildPropertyItem(mu::engraving::Pid::ALIGN, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QVariantList({ m_horizontalAlignment->value().toInt(), newValue.toInt() }));
    }, [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, QVariantList({ m_horizontalAlignment->value().toInt(), newValue.toInt() }));

        emit requestReloadPropertyItems();
    });

    m_isSizeSpatiumDependent = buildPropertyItem(mu::engraving::Pid::SIZE_SPATIUM_DEPENDENT);

    m_frameType = buildPropertyItem(mu::engraving::Pid::FRAME_TYPE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        updateFramePropertiesAvailability();
    });

    m_frameBorderColor = buildPropertyItem(mu::engraving::Pid::FRAME_FG_COLOR);
    m_frameFillColor = buildPropertyItem(mu::engraving::Pid::FRAME_BG_COLOR);
    m_frameThickness = buildPropertyItem(mu::engraving::Pid::FRAME_WIDTH);
    m_frameMargin = buildPropertyItem(mu::engraving::Pid::FRAME_PADDING);
    m_frameCornerRadius = buildPropertyItem(mu::engraving::Pid::FRAME_ROUND);

    m_textType = buildPropertyItem(mu::engraving::Pid::TEXT_STYLE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        emit requestReloadPropertyItems();
    });
    m_textPlacement = buildPropertyItem(mu::engraving::Pid::PLACEMENT);
    m_textScriptAlignment = buildPropertyItem(mu::engraving::Pid::TEXT_SCRIPT_ALIGN);
}

void TextSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::TEXT);
}

void TextSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::FONT_FACE,
        Pid::FONT_STYLE,
        Pid::FONT_SIZE,
        Pid::TEXT_LINE_SPACING,
        Pid::ALIGN,
        Pid::TEXT_SIZE_SPATIUM_DEPENDENT,
        Pid::FRAME_TYPE,
        Pid::FRAME_BG_COLOR,
        Pid::FRAME_FG_COLOR,
        Pid::FRAME_WIDTH,
        Pid::FRAME_PADDING,
        Pid::FRAME_ROUND,
        Pid::TEXT_STYLE,
        Pid::PLACEMENT,
        Pid::TEXT_SCRIPT_ALIGN
    };

    loadProperties(propertyIdSet);
}

void TextSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::FONT_FACE)) {
        loadPropertyItem(m_fontFamily, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toString() == mu::engraving::TextBase::UNDEFINED_FONT_FAMILY
                   ? QVariant() : elementPropertyValue.toString();
        });

        m_fontFamily->setIsEnabled(true);
    }

    if (muse::contains(propertyIdSet, Pid::FONT_STYLE)) {
        loadPropertyItem(m_fontStyle, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt() == static_cast<int>(mu::engraving::FontStyle::Undefined)
                   ? QVariant() : elementPropertyValue.toInt();
        });

        m_fontStyle->setIsEnabled(true);
    }

    if (muse::contains(propertyIdSet, Pid::FONT_SIZE)) {
        loadPropertyItem(m_fontSize, [](const QVariant& elementPropertyValue) -> QVariant {
            return muse::RealIsEqual(elementPropertyValue.toDouble(), mu::engraving::TextBase::UNDEFINED_FONT_SIZE)
                   ? QVariant() : elementPropertyValue.toDouble();
        });

        m_fontSize->setIsEnabled(true);
    }

    if (muse::contains(propertyIdSet, Pid::TEXT_LINE_SPACING)) {
        loadPropertyItem(m_textLineSpacing, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::ALIGN)) {
        loadPropertyItem(m_horizontalAlignment, [](const QVariant& elementPropertyValue) -> QVariant {
            QVariantList list = elementPropertyValue.toList();
            return list.size() >= 2 ? list[0] : QVariant();
        });

        loadPropertyItem(m_verticalAlignment, [](const QVariant& elementPropertyValue) -> QVariant {
            QVariantList list = elementPropertyValue.toList();
            return list.size() >= 2 ? list[1] : QVariant();
        });
    }

    if (muse::contains(propertyIdSet, Pid::TEXT_SIZE_SPATIUM_DEPENDENT)) {
        loadPropertyItem(m_isSizeSpatiumDependent);
    }

    if (muse::contains(propertyIdSet, Pid::FRAME_TYPE)) {
        loadPropertyItem(m_frameType);
    }
    if (muse::contains(propertyIdSet, Pid::FRAME_FG_COLOR)) {
        loadPropertyItem(m_frameBorderColor);
    }
    if (muse::contains(propertyIdSet, Pid::FRAME_BG_COLOR)) {
        loadPropertyItem(m_frameFillColor);
    }

    if (muse::contains(propertyIdSet, Pid::FRAME_WIDTH)) {
        loadPropertyItem(m_frameThickness, formatDoubleFunc);
    }
    if (muse::contains(propertyIdSet, Pid::FRAME_PADDING)) {
        loadPropertyItem(m_frameMargin, formatDoubleFunc);
    }
    if (muse::contains(propertyIdSet, Pid::FRAME_ROUND)) {
        loadPropertyItem(m_frameCornerRadius, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::TEXT_STYLE)) {
        loadPropertyItem(m_textType);
    }
    if (muse::contains(propertyIdSet, Pid::PLACEMENT)) {
        loadPropertyItem(m_textPlacement);
    }
    if (muse::contains(propertyIdSet, Pid::TEXT_SCRIPT_ALIGN)) {
        loadPropertyItem(m_textScriptAlignment, [](const QVariant& elementPropertyValue) -> QVariant {
            return elementPropertyValue.toInt() == static_cast<int>(mu::engraving::VerticalAlignment::AlignUndefined)
                   ? QVariant() : elementPropertyValue.toInt();
        });
    }

    updateFramePropertiesAvailability();
    updateStaffPropertiesAvailability();
    updateIsDynamicSpecificSettings();
    updateIsHorizontalAlignmentAvailable();
    updateIsSystemObjectBelowBottomStaff();
}

void TextSettingsModel::resetProperties()
{
    m_fontFamily->resetToDefault();
    m_fontStyle->resetToDefault();
    m_fontSize->resetToDefault();
    m_textLineSpacing->resetToDefault();
    m_isSizeSpatiumDependent->resetToDefault();

    m_frameType->resetToDefault();
    m_frameBorderColor->resetToDefault();
    m_frameFillColor->resetToDefault();
    m_frameThickness->resetToDefault();
    m_frameMargin->resetToDefault();
    m_frameCornerRadius->resetToDefault();

    m_textType->resetToDefault();
    m_textPlacement->resetToDefault();
    m_textScriptAlignment->resetToDefault();
}

void TextSettingsModel::onNotationChanged(const PropertyIdSet& changedProperyIds, const StyleIdSet& changedStyleIds)
{
    loadProperties(changedProperyIds);

    for (Sid s : {
        Sid::user1Name,
        Sid::user2Name,
        Sid::user3Name,
        Sid::user4Name,
        Sid::user5Name,
        Sid::user6Name,
        Sid::user7Name,
        Sid::user8Name,
        Sid::user9Name,
        Sid::user10Name,
        Sid::user11Name,
        Sid::user12Name
    }) {
        if (changedStyleIds.find(s) != changedStyleIds.cend()) {
            m_textStyles.clear();
            emit textStylesChanged();
            break;
        }
    }
}

void TextSettingsModel::insertSpecialCharacters()
{
    dispatcher()->dispatch("show-keys");
}

void TextSettingsModel::showStaffTextProperties()
{
    dispatcher()->dispatch("staff-text-properties");
}

PropertyItem* TextSettingsModel::fontFamily() const
{
    return m_fontFamily;
}

PropertyItem* TextSettingsModel::fontStyle() const
{
    return m_fontStyle;
}

PropertyItem* TextSettingsModel::fontSize() const
{
    return m_fontSize;
}

PropertyItem* TextSettingsModel::textLineSpacing() const
{
    return m_textLineSpacing;
}

PropertyItem* TextSettingsModel::horizontalAlignment() const
{
    return m_horizontalAlignment;
}

PropertyItem* TextSettingsModel::verticalAlignment() const
{
    return m_verticalAlignment;
}

PropertyItem* TextSettingsModel::isSizeSpatiumDependent() const
{
    return m_isSizeSpatiumDependent;
}

PropertyItem* TextSettingsModel::frameType() const
{
    return m_frameType;
}

PropertyItem* TextSettingsModel::frameBorderColor() const
{
    return m_frameBorderColor;
}

PropertyItem* TextSettingsModel::frameFillColor() const
{
    return m_frameFillColor;
}

PropertyItem* TextSettingsModel::frameThickness() const
{
    return m_frameThickness;
}

PropertyItem* TextSettingsModel::frameMargin() const
{
    return m_frameMargin;
}

PropertyItem* TextSettingsModel::frameCornerRadius() const
{
    return m_frameCornerRadius;
}

PropertyItem* TextSettingsModel::textType() const
{
    return m_textType;
}

PropertyItem* TextSettingsModel::textPlacement() const
{
    return m_textPlacement;
}

PropertyItem* TextSettingsModel::textScriptAlignment() const
{
    return m_textScriptAlignment;
}

QVariantList TextSettingsModel::textStyles()
{
    if (m_textStyles.empty()) {
        m_textStyles.reserve(int(TextStyleType::TEXT_TYPES));

        auto notation = currentNotation();
        Score* score = notation ? notation->elements()->msScore() : nullptr;

        for (int t = int(TextStyleType::DEFAULT) + 1; t < int(TextStyleType::TEXT_TYPES); ++t) {
            QVariantMap style;
            style["text"] = (score
                             ? score->getTextStyleUserName(static_cast<TextStyleType>(t))
                             : TConv::userName(static_cast<TextStyleType>(t)))
                            .qTranslated();
            style["value"] = t;
            m_textStyles << style;
        }
    }

    return m_textStyles;
}

bool TextSettingsModel::areStaffTextPropertiesAvailable() const
{
    return m_areStaffTextPropertiesAvailable;
}

bool TextSettingsModel::isSpecialCharactersInsertionAvailable() const
{
    return m_isSpecialCharactersInsertionAvailable;
}

bool TextSettingsModel::isDynamicSpecificSettings() const
{
    return m_isDynamicSpecificSettings;
}

bool TextSettingsModel::isHorizontalAlignmentAvailable() const
{
    return m_isHorizontalAlignmentAvailable;
}

void TextSettingsModel::setAreStaffTextPropertiesAvailable(bool areStaffTextPropertiesAvailable)
{
    if (m_areStaffTextPropertiesAvailable == areStaffTextPropertiesAvailable) {
        return;
    }

    m_areStaffTextPropertiesAvailable = areStaffTextPropertiesAvailable;
    emit areStaffTextPropertiesAvailableChanged(m_areStaffTextPropertiesAvailable);
}

void TextSettingsModel::setIsSpecialCharactersInsertionAvailable(bool isSpecialCharactersInsertionAvailable)
{
    if (m_isSpecialCharactersInsertionAvailable == isSpecialCharactersInsertionAvailable) {
        return;
    }

    m_isSpecialCharactersInsertionAvailable = isSpecialCharactersInsertionAvailable;
    emit isSpecialCharactersInsertionAvailableChanged(m_isSpecialCharactersInsertionAvailable);
}

void TextSettingsModel::setIsDynamicSpecificSettings(bool isOnlyDynamics)
{
    if (isOnlyDynamics == m_isDynamicSpecificSettings) {
        return;
    }

    m_isDynamicSpecificSettings = isOnlyDynamics;
    emit isDynamicSpecificSettingsChanged(m_isDynamicSpecificSettings);
}

void TextSettingsModel::setIsHorizontalAlignmentAvailable(bool isHorizontalAlignmentAvailable)
{
    if (isHorizontalAlignmentAvailable == m_isHorizontalAlignmentAvailable) {
        return;
    }

    m_isHorizontalAlignmentAvailable = isHorizontalAlignmentAvailable;
    emit isHorizontalAlignmentAvailableChanged(m_isHorizontalAlignmentAvailable);
}

void TextSettingsModel::updateFramePropertiesAvailability()
{
    bool isFrameVisible = static_cast<TextTypes::FrameType>(m_frameType->value().toInt())
                          != TextTypes::FrameType::FRAME_TYPE_NONE;

    m_frameThickness->setIsEnabled(isFrameVisible);
    m_frameBorderColor->setIsEnabled(isFrameVisible);
    m_frameFillColor->setIsEnabled(isFrameVisible);
    m_frameMargin->setIsEnabled(isFrameVisible);
    m_frameCornerRadius->setIsEnabled(
        static_cast<TextTypes::FrameType>(m_frameType->value().toInt()) == TextTypes::FrameType::FRAME_TYPE_SQUARE);
}

void TextSettingsModel::updateStaffPropertiesAvailability()
{
    bool isAvailable = static_cast<TextTypes::TextType>(m_textType->value().toInt())
                       == TextTypes::TextType::TEXT_TYPE_STAFF;

    setAreStaffTextPropertiesAvailable(isAvailable && !m_textType->isUndefined());
}

void TextSettingsModel::updateIsDynamicSpecificSettings()
{
    bool isOnlyDynamic = true;
    for (EngravingItem* item : m_elementList) {
        if (!item->isDynamic()) {
            isOnlyDynamic = false;
            break;
        }
    }
    setIsDynamicSpecificSettings(isOnlyDynamic);
}

void TextSettingsModel::updateIsHorizontalAlignmentAvailable()
{
    bool available = false;
    for (EngravingItem* item : m_elementList) {
        bool isPureAlignedDynamic = item->isDynamic() && toDynamic(item)->centerOnNotehead();
        if (!isPureAlignedDynamic) {
            available = true;
            break;
        }
    }
    setIsHorizontalAlignmentAvailable(available);
}

bool TextSettingsModel::isTextEditingStarted() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return false;
    }

    return context()->currentNotation()->interaction()->isTextEditingStarted();
}

muse::async::Notification TextSettingsModel::isTextEditingChanged() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return muse::async::Notification();
    }

    return context()->currentNotation()->interaction()->textEditingChanged();
}
