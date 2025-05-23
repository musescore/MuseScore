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
#include "dynamicsettingsmodel.h"

#include "translation.h"

#include "types/texttypes.h"

using namespace mu::inspector;
using namespace mu::engraving;

DynamicsSettingsModel::DynamicsSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : InspectorModelWithVoiceAndPositionOptions(parent, repository)
{
    setModelType(InspectorModelType::TYPE_DYNAMIC);
    setTitle(muse::qtrc("inspector ", "Dynamics"));
    setIcon(muse::ui::IconCode::Code::DYNAMIC_FORTE);
    createProperties();
}

void DynamicsSettingsModel::createProperties()
{
    InspectorModelWithVoiceAndPositionOptions::createProperties();

    m_avoidBarLines = buildPropertyItem(Pid::AVOID_BARLINES);
    m_dynamicSize = buildPropertyItem(Pid::DYNAMICS_SIZE,
                                      [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    },
                                      [this](const Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, newValue.toDouble() / 100);
        emit requestReloadPropertyItems();
    });
    m_centerOnNotehead = buildPropertyItem(Pid::CENTER_ON_NOTEHEAD);

    m_borderType = buildPropertyItem(Pid::BORDER_TYPE, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        updateBorderPropertiesAvailability();
    });
    m_borderColor = buildPropertyItem(Pid::BORDER_FG_COLOR);
    m_borderFillColor = buildPropertyItem(Pid::BORDER_BG_COLOR);
    m_borderThickness = buildPropertyItem(Pid::BORDER_WIDTH);
    m_borderMargin = buildPropertyItem(Pid::BORDER_PADDING);
    m_borderCornerRadius = buildPropertyItem(Pid::BORDER_ROUND);
}

void DynamicsSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::DYNAMIC);
}

void DynamicsSettingsModel::loadProperties()
{
    InspectorModelWithVoiceAndPositionOptions::loadProperties();

    loadPropertyItem(m_avoidBarLines);
    loadPropertyItem(m_dynamicSize, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });
    loadPropertyItem(m_centerOnNotehead);

    loadPropertyItem(m_borderType);
    loadPropertyItem(m_borderColor);
    loadPropertyItem(m_borderFillColor);
    loadPropertyItem(m_borderThickness, formatDoubleFunc);
    loadPropertyItem(m_borderMargin, formatDoubleFunc);
    loadPropertyItem(m_borderCornerRadius, formatDoubleFunc);

    updateBorderPropertiesAvailability();
}

void DynamicsSettingsModel::resetProperties()
{
    InspectorModelWithVoiceAndPositionOptions::resetProperties();

    m_avoidBarLines->resetToDefault();
    m_dynamicSize->resetToDefault();
    m_centerOnNotehead->resetToDefault();

    m_borderType->resetToDefault();
    m_borderColor->resetToDefault();
    m_borderFillColor->resetToDefault();
    m_borderThickness->resetToDefault();
    m_borderMargin->resetToDefault();
    m_borderCornerRadius->resetToDefault();
}

PropertyItem* DynamicsSettingsModel::avoidBarLines() const
{
    return m_avoidBarLines;
}

PropertyItem* DynamicsSettingsModel::dynamicSize() const
{
    return m_dynamicSize;
}

PropertyItem* DynamicsSettingsModel::centerOnNotehead() const
{
    return m_centerOnNotehead;
}

PropertyItem* DynamicsSettingsModel::borderType() const
{
    return m_borderType;
}

PropertyItem* DynamicsSettingsModel::borderColor() const
{
    return m_borderColor;
}

PropertyItem* DynamicsSettingsModel::borderFillColor() const
{
    return m_borderFillColor;
}

PropertyItem* DynamicsSettingsModel::borderThickness() const
{
    return m_borderThickness;
}

PropertyItem* DynamicsSettingsModel::borderMargin() const
{
    return m_borderMargin;
}

PropertyItem* DynamicsSettingsModel::borderCornerRadius() const
{
    return m_borderCornerRadius;
}

void DynamicsSettingsModel::updateBorderPropertiesAvailability()
{
    bool isBorderVisible = static_cast<TextTypes::BorderType>(m_borderType->value().toInt())
                           != TextTypes::BorderType::BORDER_TYPE_NONE;

    m_borderThickness->setIsEnabled(isBorderVisible);
    m_borderColor->setIsEnabled(isBorderVisible);
    m_borderFillColor->setIsEnabled(isBorderVisible);
    m_borderMargin->setIsEnabled(isBorderVisible);
    m_borderCornerRadius->setIsEnabled(
        static_cast<TextTypes::BorderType>(m_borderType->value().toInt()) == TextTypes::BorderType::BORDER_TYPE_SQUARE);
}
