/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include "ornamentplaybackmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

OrnamentPlaybackModel::OrnamentPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Ornament"));
    setModelType(InspectorModelType::TYPE_ORNAMENT);

    createProperties();
}

void OrnamentPlaybackModel::createProperties()
{
    m_timeStretch = buildPropertyItem(Ms::Pid::TIME_STRETCH, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });
}

void OrnamentPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::ARTICULATION, [](const Ms::EngravingItem* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Articulation* articulation = Ms::toArticulation(element);
        IF_ASSERT_FAILED(articulation) {
            return false;
        }

        return articulation->isOrnament();
    });
}

void OrnamentPlaybackModel::loadProperties()
{
    loadPropertyItem(m_timeStretch, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });
}

void OrnamentPlaybackModel::resetProperties()
{
    m_timeStretch->resetToDefault();
}

PropertyItem* OrnamentPlaybackModel::timeStretch() const
{
    return m_timeStretch;
}
