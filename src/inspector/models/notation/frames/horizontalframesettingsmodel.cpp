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

using namespace mu::inspector;
using namespace mu::engraving;

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
    m_frameWidth = buildPropertyItem(Pid::BOX_WIDTH);
    m_leftGap= buildPropertyItem(Pid::TOP_GAP);
    m_rightGap = buildPropertyItem(Pid::BOTTOM_GAP);
    m_shouldDisplayKeysAndBrackets = buildPropertyItem(Pid::CREATE_SYSTEM_HEADER);
}

void HorizontalFrameSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::HBOX);
}

void HorizontalFrameSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::BOX_WIDTH,
        Pid::TOP_GAP,
        Pid::BOTTOM_GAP,
        Pid::CREATE_SYSTEM_HEADER,
    };

    loadProperties(propertyIdSet);
}

void HorizontalFrameSettingsModel::resetProperties()
{
    m_frameWidth->resetToDefault();
    m_leftGap->resetToDefault();
    m_rightGap->resetToDefault();
    m_shouldDisplayKeysAndBrackets->resetToDefault();
}

void HorizontalFrameSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void HorizontalFrameSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (mu::contains(propertyIdSet, Pid::BOX_WIDTH)) {
        loadPropertyItem(m_frameWidth, roundedDoubleElementInternalToUiConverter(Pid::BOX_WIDTH));
    }

    if (mu::contains(propertyIdSet, Pid::TOP_GAP)) {
        loadPropertyItem(m_leftGap);
    }

    if (mu::contains(propertyIdSet, Pid::BOTTOM_GAP)) {
        loadPropertyItem(m_rightGap);
    }

    if (mu::contains(propertyIdSet, Pid::CREATE_SYSTEM_HEADER)) {
        loadPropertyItem(m_shouldDisplayKeysAndBrackets);
    }
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
