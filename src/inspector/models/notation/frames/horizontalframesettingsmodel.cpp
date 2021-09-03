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
#include "horizontalframesettingsmodel.h"

#include "translation.h"
#include "dataformatter.h"

using namespace mu::inspector;

HorizontalFrameSettingsModel::HorizontalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_HORIZONTAL_FRAME);
    setTitle(qtrc("inspector", "Horizontal frame"));
    setIcon(ui::IconCode::Code::HORIZONTAL_FRAME);
    createProperties();
}

void HorizontalFrameSettingsModel::createProperties()
{
    m_frameWidth = buildPropertyItem(Ms::Pid::BOX_WIDTH);
    m_leftGap= buildPropertyItem(Ms::Pid::TOP_GAP);
    m_rightGap = buildPropertyItem(Ms::Pid::BOTTOM_GAP);
    m_shouldDisplayKeysAndBrackets = buildPropertyItem(Ms::Pid::CREATE_SYSTEM_HEADER);
}

void HorizontalFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HBOX);
}

void HorizontalFrameSettingsModel::loadProperties()
{
    loadPropertyItem(m_frameWidth, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_leftGap);
    loadPropertyItem(m_rightGap);
    loadPropertyItem(m_shouldDisplayKeysAndBrackets);
}

void HorizontalFrameSettingsModel::resetProperties()
{
    m_frameWidth->resetToDefault();
    m_leftGap->resetToDefault();
    m_rightGap->resetToDefault();
    m_shouldDisplayKeysAndBrackets->resetToDefault();
}

PropertyItem* HorizontalFrameSettingsModel::frameWidth() const
{
    return m_frameWidth;
}

PropertyItem* HorizontalFrameSettingsModel::leftGap() const
{
    return m_leftGap;
}

PropertyItem* HorizontalFrameSettingsModel::rightGap() const
{
    return m_rightGap;
}

PropertyItem* HorizontalFrameSettingsModel::shouldDisplayKeysAndBrackets() const
{
    return m_shouldDisplayKeysAndBrackets;
}
