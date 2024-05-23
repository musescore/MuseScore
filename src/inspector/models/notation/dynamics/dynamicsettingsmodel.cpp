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

    m_frameType = buildPropertyItem(Pid::FRAME_TYPE, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        updateFramePropertiesAvailability();
    });
    m_frameBorderColor = buildPropertyItem(Pid::FRAME_FG_COLOR);
    m_frameFillColor = buildPropertyItem(Pid::FRAME_BG_COLOR);
    m_frameThickness = buildPropertyItem(Pid::FRAME_WIDTH);
    m_frameMargin = buildPropertyItem(Pid::FRAME_PADDING);
    m_frameCornerRadius = buildPropertyItem(Pid::FRAME_ROUND);
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

    loadPropertyItem(m_frameType);
    loadPropertyItem(m_frameBorderColor);
    loadPropertyItem(m_frameFillColor);
    loadPropertyItem(m_frameThickness, formatDoubleFunc);
    loadPropertyItem(m_frameMargin, formatDoubleFunc);
    loadPropertyItem(m_frameCornerRadius, formatDoubleFunc);

    updateFramePropertiesAvailability();
}

void DynamicsSettingsModel::resetProperties()
{
    InspectorModelWithVoiceAndPositionOptions::resetProperties();

    m_avoidBarLines->resetToDefault();
    m_dynamicSize->resetToDefault();
    m_centerOnNotehead->resetToDefault();

    m_frameType->resetToDefault();
    m_frameBorderColor->resetToDefault();
    m_frameFillColor->resetToDefault();
    m_frameThickness->resetToDefault();
    m_frameMargin->resetToDefault();
    m_frameCornerRadius->resetToDefault();
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

PropertyItem* DynamicsSettingsModel::frameType() const
{
    return m_frameType;
}

PropertyItem* DynamicsSettingsModel::frameBorderColor() const
{
    return m_frameBorderColor;
}

PropertyItem* DynamicsSettingsModel::frameFillColor() const
{
    return m_frameFillColor;
}

PropertyItem* DynamicsSettingsModel::frameThickness() const
{
    return m_frameThickness;
}

PropertyItem* DynamicsSettingsModel::frameMargin() const
{
    return m_frameMargin;
}

PropertyItem* DynamicsSettingsModel::frameCornerRadius() const
{
    return m_frameCornerRadius;
}

void DynamicsSettingsModel::updateFramePropertiesAvailability()
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
