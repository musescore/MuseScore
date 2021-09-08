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
#include "dynamicplaybackmodel.h"

#include "dynamic.h"

#include "translation.h"

using namespace mu::inspector;

DynamicPlaybackModel::DynamicPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Dynamics"));
    setModelType(InspectorModelType::TYPE_DYNAMIC);

    createProperties();
}

void DynamicPlaybackModel::createProperties()
{
    m_scopeType = buildPropertyItem(Ms::Pid::DYNAMIC_RANGE);
    m_velocity = buildPropertyItem(Ms::Pid::VELOCITY);
    m_velocityChangeSpeed = buildPropertyItem(Ms::Pid::VELO_CHANGE_SPEED);
    m_velocityChange = buildPropertyItem(Ms::Pid::VELO_CHANGE);
}

void DynamicPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::DYNAMIC);
}

void DynamicPlaybackModel::loadProperties()
{
    loadPropertyItem(m_scopeType);
    loadPropertyItem(m_velocity);
    loadPropertyItem(m_velocityChange);
    loadPropertyItem(m_velocityChangeSpeed);
}

void DynamicPlaybackModel::resetProperties()
{
    m_scopeType->resetToDefault();
    m_velocity->resetToDefault();
    m_velocityChange->resetToDefault();
    m_velocityChangeSpeed->resetToDefault();
}

PropertyItem* DynamicPlaybackModel::velocity() const
{
    return m_velocity;
}

PropertyItem* DynamicPlaybackModel::velocityChange() const
{
    return m_velocityChange;
}

PropertyItem* DynamicPlaybackModel::velocityChangeSpeed() const
{
    return m_velocityChangeSpeed;
}

PropertyItem* DynamicPlaybackModel::scopeType() const
{
    return m_scopeType;
}
