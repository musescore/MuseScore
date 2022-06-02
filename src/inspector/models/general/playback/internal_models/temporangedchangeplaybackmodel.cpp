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

#include "temporangedchangeplaybackmodel.h"

using namespace mu::inspector;

TempoRangedChangePlaybackModel::TempoRangedChangePlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, Ms::ElementType::TEMPO_RANGED_CHANGE)
{
    setTitle(qtrc("inspector", "Accelerando & Ritardando"));
    setModelType(InspectorModelType::TYPE_TEMPO_RANGED_CHANGE);

    createProperties();
}

PropertyItem* TempoRangedChangePlaybackModel::tempoChangeFactor() const
{
    return m_tempoChangeFactor;
}

void TempoRangedChangePlaybackModel::createProperties()
{
    m_tempoChangeFactor = buildPropertyItem(Ms::Pid::TEMPO_CHANGE_FACTOR, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });
}

void TempoRangedChangePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tempoChangeFactor, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });
}

void TempoRangedChangePlaybackModel::resetProperties()
{
    m_tempoChangeFactor->resetToDefault();
}
