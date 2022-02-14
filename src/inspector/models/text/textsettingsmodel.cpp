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
#include "textsettingsmodel.h"

#include <QFont>

#include "types/commontypes.h"
#include "types/texttypes.h"

#include "libmscore/textbase.h"

#include "translation.h"
#include "log.h"

using namespace mu::inspector;
using namespace mu::engraving;

TextSettingsModel::TextSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_TEXT);
    setTitle(qtrc("inspector", "Text"));
    createProperties();

    isTextEditingChanged().onNotify(this, [this]() {
        loadProperties();
        setIsSpecialCharactersInsertionAvailable(isTextEditingStarted());
    });
}

void TextSettingsModel::createProperties()
{
    m_fontFamily = buildPropertyItem(Ms::Pid::FONT_FACE);
    m_fontStyle = buildPropertyItem(Ms::Pid::FONT_STYLE);
    m_fontSize = buildPropertyItem(Ms::Pid::FONT_SIZE);

    m_horizontalAlignment = buildPropertyItem(Ms::Pid::ALIGN, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QVariantList({ newValue.toInt(), m_verticalAlignment->value().toInt() }));
    });
    m_verticalAlignment = buildPropertyItem(Ms::Pid::ALIGN, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QVariantList({ m_horizontalAlignment->value().toInt(), newValue.toInt() }));
    });

    m_isSizeSpatiumDependent = buildPropertyItem(Ms::Pid::SIZE_SPATIUM_DEPENDENT);

    m_frameType = buildPropertyItem(Ms::Pid::FRAME_TYPE, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        updateFramePropertiesAvailability();
    });

    m_frameBorderColor = buildPropertyItem(Ms::Pid::FRAME_FG_COLOR);
    m_frameHighlightColor = buildPropertyItem(Ms::Pid::FRAME_BG_COLOR);
    m_frameThickness = buildPropertyItem(Ms::Pid::FRAME_WIDTH);
    m_frameMargin = buildPropertyItem(Ms::Pid::FRAME_PADDING);
    m_frameCornerRadius = buildPropertyItem(Ms::Pid::FRAME_ROUND);

    m_textType = buildPropertyItem(Ms::Pid::TEXT_STYLE);
    m_textPlacement = buildPropertyItem(Ms::Pid::PLACEMENT);
    m_textScriptAlignment = buildPropertyItem(Ms::Pid::TEXT_SCRIPT_ALIGN);
}

void TextSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TEXT);
}

void TextSettingsModel::loadProperties()
{
    loadPropertyItem(m_fontFamily, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toString() == Ms::TextBase::UNDEFINED_FONT_FAMILY
               ? QVariant() : elementPropertyValue.toString();
    });

    loadPropertyItem(m_fontStyle, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt() == static_cast<int>(Ms::FontStyle::Undefined)
               ? QVariant() : elementPropertyValue.toInt();
    });

    loadPropertyItem(m_fontSize, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt() == Ms::TextBase::UNDEFINED_FONT_SIZE
               ? QVariant() : elementPropertyValue.toInt();
    });

    loadPropertyItem(m_horizontalAlignment, [](const QVariant& elementPropertyValue) -> QVariant {
        QVariantList list = elementPropertyValue.toList();
        return list.size() >= 2 ? list[0] : QVariant();
    });

    loadPropertyItem(m_verticalAlignment, [](const QVariant& elementPropertyValue) -> QVariant {
        QVariantList list = elementPropertyValue.toList();
        return list.size() >= 2 ? list[1] : QVariant();
    });

    loadPropertyItem(m_isSizeSpatiumDependent);

    loadPropertyItem(m_frameType);
    loadPropertyItem(m_frameBorderColor);
    loadPropertyItem(m_frameHighlightColor);

    loadPropertyItem(m_frameThickness, formatDoubleFunc);
    loadPropertyItem(m_frameMargin, formatDoubleFunc);
    loadPropertyItem(m_frameCornerRadius, formatDoubleFunc);

    loadPropertyItem(m_textType);
    loadPropertyItem(m_textPlacement);
    loadPropertyItem(m_textScriptAlignment, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt() == static_cast<int>(Ms::VerticalAlignment::AlignUndefined)
               ? QVariant() : elementPropertyValue.toInt();
    });

    updateFramePropertiesAvailability();
    updateStaffPropertiesAvailability();
}

void TextSettingsModel::resetProperties()
{
    m_fontFamily->resetToDefault();
    m_fontStyle->resetToDefault();
    m_fontSize->resetToDefault();
    m_isSizeSpatiumDependent->resetToDefault();

    m_frameType->resetToDefault();
    m_frameBorderColor->resetToDefault();
    m_frameHighlightColor->resetToDefault();
    m_frameThickness->resetToDefault();
    m_frameMargin->resetToDefault();
    m_frameCornerRadius->resetToDefault();

    m_textType->resetToDefault();
    m_textPlacement->resetToDefault();
    m_textScriptAlignment->resetToDefault();
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

PropertyItem* TextSettingsModel::frameHighlightColor() const
{
    return m_frameHighlightColor;
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

bool TextSettingsModel::areStaffTextPropertiesAvailable() const
{
    return m_areStaffTextPropertiesAvailable;
}

bool TextSettingsModel::isSpecialCharactersInsertionAvailable() const
{
    return m_isSpecialCharactersInsertionAvailable;
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

void TextSettingsModel::updateFramePropertiesAvailability()
{
    bool isFrameVisible = static_cast<TextTypes::FrameType>(m_frameType->value().toInt())
                          != TextTypes::FrameType::FRAME_TYPE_NONE;

    m_frameThickness->setIsEnabled(isFrameVisible);
    m_frameBorderColor->setIsEnabled(isFrameVisible);
    m_frameHighlightColor->setIsEnabled(isFrameVisible);
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

bool TextSettingsModel::isTextEditingStarted() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return false;
    }

    return context()->currentNotation()->interaction()->isTextEditingStarted();
}

mu::async::Notification TextSettingsModel::isTextEditingChanged() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return mu::async::Notification();
    }

    return context()->currentNotation()->interaction()->textEditingChanged();
}
