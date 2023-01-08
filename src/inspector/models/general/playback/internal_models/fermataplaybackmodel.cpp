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
#include "fermataplaybackmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

FermataPlaybackModel::FermataPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Fermatas"));
    setModelType(InspectorModelType::TYPE_FERMATA);

    createProperties();
}

void FermataPlaybackModel::createProperties()
{
    m_timeStretch = buildPropertyItem(mu::engraving::Pid::TIME_STRETCH, [](const QVariant& newValue) { return newValue.toDouble() / 100; });
}

void FermataPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::FERMATA);
}

void FermataPlaybackModel::loadProperties()
{
    loadPropertyItem(m_timeStretch, [](const engraving::PropertyValue& propertyValue) -> QVariant {
        return DataFormatter::roundDouble(propertyValue.toDouble()) * 100;
    });
}

void FermataPlaybackModel::resetProperties()
{
    m_timeStretch->resetToDefault();
}

PropertyItem* FermataPlaybackModel::timeStretch() const
{
    return m_timeStretch;
}
