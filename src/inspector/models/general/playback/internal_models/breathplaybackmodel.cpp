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
#include "breathplaybackmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

BreathPlaybackModel::BreathPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Breaths & pauses"));

    createProperties();
}

void BreathPlaybackModel::createProperties()
{
    m_pauseTime = buildPropertyItem(Ms::Pid::PAUSE);
}

void BreathPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BREATH);
}

void BreathPlaybackModel::loadProperties()
{
    loadPropertyItem(m_pauseTime, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });
}

void BreathPlaybackModel::resetProperties()
{
    m_pauseTime->resetToDefault();
}

PropertyItem* BreathPlaybackModel::pauseTime() const
{
    return m_pauseTime;
}
