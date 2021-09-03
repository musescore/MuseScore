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
#include "verticalframesettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

VerticalFrameSettingsModel::VerticalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_VERTICAL_FRAME);
    setTitle(qtrc("inspector", "Vertical frame"));
    setIcon(ui::IconCode::Code::VERTICAL_FRAME);
    createProperties();
}

void VerticalFrameSettingsModel::createProperties()
{
    m_frameHeight = buildPropertyItem(Ms::Pid::BOX_HEIGHT);
    m_gapAbove = buildPropertyItem(Ms::Pid::TOP_GAP);
    m_gapBelow = buildPropertyItem(Ms::Pid::BOTTOM_GAP);
    m_frameLeftMargin = buildPropertyItem(Ms::Pid::LEFT_MARGIN);
    m_frameRightMargin = buildPropertyItem(Ms::Pid::RIGHT_MARGIN);
    m_frameTopMargin = buildPropertyItem(Ms::Pid::TOP_MARGIN);
    m_frameBottomMargin = buildPropertyItem(Ms::Pid::BOTTOM_MARGIN);
}

void VerticalFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::VBOX);
}

void VerticalFrameSettingsModel::loadProperties()
{
    loadPropertyItem(m_frameHeight, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_gapAbove);
    loadPropertyItem(m_gapBelow);
    loadPropertyItem(m_frameLeftMargin);
    loadPropertyItem(m_frameRightMargin);
    loadPropertyItem(m_frameTopMargin);
    loadPropertyItem(m_frameBottomMargin);
}

void VerticalFrameSettingsModel::resetProperties()
{
    m_frameHeight->resetToDefault();
    m_gapAbove->resetToDefault();
    m_gapBelow->resetToDefault();
    m_frameLeftMargin->resetToDefault();
    m_frameRightMargin->resetToDefault();
    m_frameTopMargin->resetToDefault();
    m_frameBottomMargin->resetToDefault();
}

PropertyItem* VerticalFrameSettingsModel::frameHeight() const
{
    return m_frameHeight;
}

PropertyItem* VerticalFrameSettingsModel::gapAbove() const
{
    return m_gapAbove;
}

PropertyItem* VerticalFrameSettingsModel::gapBelow() const
{
    return m_gapBelow;
}

PropertyItem* VerticalFrameSettingsModel::frameLeftMargin() const
{
    return m_frameLeftMargin;
}

PropertyItem* VerticalFrameSettingsModel::frameRightMargin() const
{
    return m_frameRightMargin;
}

PropertyItem* VerticalFrameSettingsModel::frameTopMargin() const
{
    return m_frameTopMargin;
}

PropertyItem* VerticalFrameSettingsModel::frameBottomMargin() const
{
    return m_frameBottomMargin;
}
