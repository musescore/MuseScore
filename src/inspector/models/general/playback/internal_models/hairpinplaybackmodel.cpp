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
#include "hairpinplaybackmodel.h"

#include "translation.h"

using namespace mu::inspector;

HairpinPlaybackModel::HairpinPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, Ms::ElementType::HAIRPIN)
{
    setTitle(qtrc("inspector", "Hairpins, crescendos & diminuendos"));
    setModelType(InspectorModelType::TYPE_HAIRPIN);

    createProperties();
}

PropertyItem* HairpinPlaybackModel::scopeType() const
{
    return m_scopeType;
}

PropertyItem* HairpinPlaybackModel::velocityChange() const
{
    return m_velocityChange;
}

PropertyItem* HairpinPlaybackModel::useSingleNoteDynamics() const
{
    return m_useSingleNoteDynamics;
}

PropertyItem* HairpinPlaybackModel::velocityChangeType() const
{
    return m_velocityChangeType;
}

void HairpinPlaybackModel::createProperties()
{
    m_scopeType = buildPropertyItem(Ms::Pid::DYNAMIC_RANGE);
    m_velocityChange = buildPropertyItem(Ms::Pid::VELO_CHANGE);
    m_useSingleNoteDynamics = buildPropertyItem(Ms::Pid::SINGLE_NOTE_DYNAMICS);
    m_velocityChangeType = buildPropertyItem(Ms::Pid::VELO_CHANGE_METHOD);
}

void HairpinPlaybackModel::loadProperties()
{
    loadPropertyItem(m_scopeType);
    loadPropertyItem(m_velocityChange);
    loadPropertyItem(m_useSingleNoteDynamics);
    loadPropertyItem(m_velocityChangeType);
}

void HairpinPlaybackModel::resetProperties()
{
    m_scopeType->resetToDefault();
    m_velocityChange->resetToDefault();
    m_useSingleNoteDynamics->resetToDefault();
    m_velocityChangeType->resetToDefault();
}
