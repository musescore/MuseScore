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
#include "noteplaybackmodel.h"

#include "translation.h"
#include "dataformatter.h"

using namespace mu::inspector;

NotePlaybackModel::NotePlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Notes"));

    createProperties();
}

void NotePlaybackModel::createProperties()
{
    m_tuning = buildPropertyItem(Ms::Pid::TUNING);
    m_velocity = buildPropertyItem(Ms::Pid::VELO_OFFSET);
    m_overrideDynamics = buildPropertyItem(Ms::Pid::VELO_TYPE);
}

void NotePlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::NOTE);
}

void NotePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tuning, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_velocity);

    loadPropertyItem(m_overrideDynamics, [](const QVariant& elementValue) -> QVariant {
        return static_cast<bool>(elementValue.toInt());
    });
}

void NotePlaybackModel::resetProperties()
{
    m_tuning->resetToDefault();
    m_velocity->resetToDefault();
    m_overrideDynamics->resetToDefault();
}

PropertyItem* NotePlaybackModel::tuning() const
{
    return m_tuning;
}

PropertyItem* NotePlaybackModel::velocity() const
{
    return m_velocity;
}

PropertyItem* NotePlaybackModel::overrideDynamics() const
{
    return m_overrideDynamics;
}
