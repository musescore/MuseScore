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
    m_bendType = buildPropertyItem(Ms::Pid::BEND_TYPE, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        if (newValue.toInt() != static_cast<int>(BendTypes::BendType::TYPE_CUSTOM)) {
            emit requestReloadPropertyItems();
        }
    });

    m_bendCurve = buildPropertyItem(Ms::Pid::BEND_CURVE, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_lineThickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
}

void BendSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BEND);

    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void BendSettingsModel::loadProperties()
{
    loadPropertyItem(m_bendType);
    loadPropertyItem(m_bendCurve);
    loadPropertyItem(m_lineThickness, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble());
    });
}

void BendSettingsModel::resetProperties()
{
    m_bendType->resetToDefault();
    m_bendCurve->resetToDefault();
    m_lineThickness->resetToDefault();
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

PropertyItem* BendSettingsModel::bendCurve() const
{
    return m_bendCurve;
}
