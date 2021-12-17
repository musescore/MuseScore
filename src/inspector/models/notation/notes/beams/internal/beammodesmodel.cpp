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
#include "beammodesmodel.h"

using namespace mu::inspector;

BeamModesModel::BeamModesModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();
}

void BeamModesModel::createProperties()
{
    m_mode = buildPropertyItem(Ms::Pid::BEAM_MODE);
    m_isFeatheringAvailable = buildPropertyItem(Ms::Pid::DURATION_TYPE_WITH_DOTS, [](const Ms::Pid, const QVariant&) {}); //@note readonly property, there is no need to modify it
}

void BeamModesModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::CHORD);
}

void BeamModesModel::loadProperties()
{
    loadPropertyItem(m_mode);

    loadPropertyItem(m_isFeatheringAvailable, [](const QVariant& elementPropertyValue) -> QVariant {
        Ms::TDuration durationType = elementPropertyValue.value<Ms::TDuration>();

        switch (durationType.type()) {
        case Ms::DurationType::V_INVALID:
        case Ms::DurationType::V_MEASURE:
        case Ms::DurationType::V_ZERO:
        case Ms::DurationType::V_LONG:
        case Ms::DurationType::V_BREVE:
        case Ms::DurationType::V_WHOLE:
        case Ms::DurationType::V_HALF:
        case Ms::DurationType::V_QUARTER:
        case Ms::DurationType::V_EIGHTH:
            return false;

        default:
            return true;
        }
    });
}

void BeamModesModel::resetProperties()
{
    m_mode->resetToDefault();
}

PropertyItem* BeamModesModel::mode() const
{
    return m_mode;
}

PropertyItem* BeamModesModel::isFeatheringAvailable() const
{
    return m_isFeatheringAvailable;
}
