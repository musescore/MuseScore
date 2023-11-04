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
#include "dynamicsettingsmodel.h"

#include "dataformatter.h"
#include "translation.h"

#include "types/texttypes.h"

using namespace mu::inspector;

DynamicsSettingsModel::DynamicsSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_DYNAMIC);
    setTitle(qtrc("inspector ", "Dynamics"));
    setIcon(ui::IconCode::Code::DYNAMIC_FORTE);
    createProperties();
}

void DynamicsSettingsModel::createProperties()
{
    m_avoidBarLines = buildPropertyItem(mu::engraving::Pid::AVOID_BARLINES);
    m_dynamicSize = buildPropertyItem(mu::engraving::Pid::DYNAMICS_SIZE, [](const QVariant& newValue) {
        return newValue.toDouble() / 100;
    });
    m_centerOnNotehead = buildPropertyItem(mu::engraving::Pid::CENTER_ON_NOTEHEAD);
    m_placement = buildPropertyItem(mu::engraving::Pid::PLACEMENT);

    m_frameType = buildPropertyItem(mu::engraving::Pid::FRAME_TYPE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        default_setProperty_callback(mu::engraving::Pid::FRAME_TYPE)(pid, newValue);
        updateFramePropertiesAvailability();
    });
    m_frameBorderColor = buildPropertyItem(mu::engraving::Pid::FRAME_FG_COLOR);
    m_frameFillColor = buildPropertyItem(mu::engraving::Pid::FRAME_BG_COLOR);
    m_frameThickness = buildPropertyItem(mu::engraving::Pid::FRAME_WIDTH);
    m_frameMargin = buildPropertyItem(mu::engraving::Pid::FRAME_PADDING);
    m_frameCornerRadius = buildPropertyItem(mu::engraving::Pid::FRAME_ROUND);
}

void DynamicsSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::DYNAMIC);
}

void DynamicsSettingsModel::loadProperties()
{
    loadPropertyItem(m_avoidBarLines);
    loadPropertyItem(m_dynamicSize, [](const engraving::PropertyValue& propertyValue) -> QVariant {
        return DataFormatter::roundDouble(propertyValue.toDouble()) * 100;
    });
    loadPropertyItem(m_centerOnNotehead);
    loadPropertyItem(m_placement);

    loadPropertyItem(m_frameType);
    loadPropertyItem(m_frameBorderColor);
    loadPropertyItem(m_frameFillColor);
    loadPropertyItem(m_frameThickness, roundedDouble_internalToUi_converter(mu::engraving::Pid::FRAME_WIDTH));
    loadPropertyItem(m_frameMargin, roundedDouble_internalToUi_converter(mu::engraving::Pid::FRAME_PADDING));
    loadPropertyItem(m_frameCornerRadius, roundedDouble_internalToUi_converter(mu::engraving::Pid::FRAME_ROUND));

    updateFramePropertiesAvailability();
}

void DynamicsSettingsModel::resetProperties()
{
    m_avoidBarLines->resetToDefault();
    m_dynamicSize->resetToDefault();
    m_centerOnNotehead->resetToDefault();
    m_placement->resetToDefault();

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

PropertyItem* DynamicsSettingsModel::placement() const
{
    return m_placement;
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
