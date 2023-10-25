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
#include "bendsettingsmodel.h"
#include "dom/guitarbend.h"

#include "translation.h"
#include "dataformatter.h"
#include "types/bendtypes.h"

using namespace mu::inspector;

BendSettingsModel::BendSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BEND);
    setTitle(qtrc("inspector", "Bend"));
    setIcon(ui::IconCode::Code::GUITAR_BEND);
    createProperties();
}

void BendSettingsModel::createProperties()
{
    m_bendType = buildPropertyItem(mu::engraving::Pid::BEND_TYPE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        if (newValue.toInt() != static_cast<int>(BendTypes::BendType::TYPE_CUSTOM)) {
            emit requestReloadPropertyItems();
        }
    });

    m_bendCurve = buildPropertyItem(mu::engraving::Pid::BEND_CURVE, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_lineThickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);

    m_bendDirection = buildPropertyItem(mu::engraving::Pid::DIRECTION);
    m_showHoldLine = buildPropertyItem(mu::engraving::Pid::BEND_SHOW_HOLD_LINE);
}

void BendSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::GUITAR_BEND_SEGMENT);

    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void BendSettingsModel::loadProperties()
{
    loadPropertyItem(m_bendType);
    loadPropertyItem(m_bendCurve);
    loadPropertyItem(m_lineThickness, formatDoubleFunc);
    loadPropertyItem(m_bendDirection);
    loadPropertyItem(m_showHoldLine);

    updateIsShowHoldLineAvailable();
}

void BendSettingsModel::resetProperties()
{
    m_bendType->resetToDefault();
    m_bendCurve->resetToDefault();
    m_lineThickness->resetToDefault();
    m_bendDirection->resetToDefault();
    m_showHoldLine->resetToDefault();
}

PropertyItem* BendSettingsModel::bendType() const
{
    return m_bendType;
}

PropertyItem* BendSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

bool BendSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // Bend inspector doesn't support multiple selection
}

void BendSettingsModel::updateIsShowHoldLineAvailable()
{
    bool available = true;
    for (EngravingItem* item : m_elementList) {
        GuitarBendSegment* seg = toGuitarBendSegment(item);
        if (seg->staffType() && !seg->staffType()->isTabStaff()) {
            available = false;
            break;
        }
    }

    if (m_isShowHoldLineAvailable != available) {
        m_isShowHoldLineAvailable = available;
        emit isShowHoldLineAvailableChanged(m_isShowHoldLineAvailable);
    }
}

PropertyItem* BendSettingsModel::bendCurve() const
{
    return m_bendCurve;
}

PropertyItem* BendSettingsModel::bendDirection() const
{
    return m_bendDirection;
}

PropertyItem* BendSettingsModel::showHoldLine() const
{
    return m_showHoldLine;
}

bool BendSettingsModel::isShowHoldLineAvailable() const
{
    return m_isShowHoldLineAvailable;
}
