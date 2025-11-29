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
#include "noteplaybackmodel.h"

#include "translation.h"
#include "dataformatter.h"

using namespace mu::inspector;

NotePlaybackModel::NotePlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(muse::qtrc("inspector", "Notes"));
    setModelType(InspectorModelType::TYPE_NOTE);

    createProperties();
}

void NotePlaybackModel::createProperties()
{
    m_tuning = buildPropertyItem(mu::engraving::Pid::TUNING);
    m_velocity = buildPropertyItem(mu::engraving::Pid::USER_VELOCITY);
}

void NotePlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::NOTEHEAD);
}

void NotePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tuning, formatDoubleFunc);
    loadPropertyItem(m_velocity, [](const QVariant& value) {
        //! NOTE: display 64 instead of 0 in the Velocity field to avoid confusing the user
        return value.toInt() == 0 ? 64 : value;
    });
}

void NotePlaybackModel::resetProperties()
{
    m_tuning->resetToDefault();
    m_velocity->resetToDefault();
}

PropertyItem* NotePlaybackModel::tuning() const
{
    return m_tuning;
}

PropertyItem* NotePlaybackModel::velocity() const
{
    return m_velocity;
}
