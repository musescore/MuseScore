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
#include "beammodesmodel.h"

using namespace mu::inspector;

BeamModesModel::BeamModesModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();
}

void BeamModesModel::createProperties()
{
    m_mode = buildPropertyItem(mu::engraving::Pid::BEAM_MODE);
    m_isFeatheringAvailable = buildPropertyItem(mu::engraving::Pid::DURATION_TYPE_WITH_DOTS, [](const mu::engraving::Pid, const QVariant&) {
    });                                                                                                                                         //@note readonly property, there is no need to modify it
}

void BeamModesModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::CHORD) << m_repository->findElementsByType(
        mu::engraving::ElementType::REST);
}

void BeamModesModel::loadProperties()
{
    loadPropertyItem(m_mode);

    loadPropertyItem(m_isFeatheringAvailable, [](const QVariant& elementPropertyValue) -> QVariant {
        QVariantMap map = elementPropertyValue.toMap();
        if (map.isEmpty()) {
            return false;
        }

        mu::engraving::DurationType durationType = static_cast<mu::engraving::DurationType>(map["type"].toInt());

        switch (durationType) {
        case mu::engraving::DurationType::V_INVALID:
        case mu::engraving::DurationType::V_MEASURE:
        case mu::engraving::DurationType::V_ZERO:
        case mu::engraving::DurationType::V_LONG:
        case mu::engraving::DurationType::V_BREVE:
        case mu::engraving::DurationType::V_WHOLE:
        case mu::engraving::DurationType::V_HALF:
        case mu::engraving::DurationType::V_QUARTER:
        case mu::engraving::DurationType::V_EIGHTH:
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
