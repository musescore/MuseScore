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

#include "engraving/types/types.h"

using namespace mu::inspector;
using namespace mu::engraving;

TempoRangedChangePlaybackModel::TempoRangedChangePlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, ElementType::TEMPO_RANGED_CHANGE)
{
    setTitle(qtrc("inspector", "Tempo change"));
    setModelType(InspectorModelType::TYPE_TEMPO_RANGED_CHANGE);

    createProperties();
}

PropertyItem* TempoRangedChangePlaybackModel::tempoChangeFactor() const
{
    return m_tempoChangeFactor;
}

PropertyItem* TempoRangedChangePlaybackModel::tempoEasingMethod() const
{
    return m_tempoEasingMethod;
}

QVariantList TempoRangedChangePlaybackModel::possibleEasingMethods() const
{
    QVariantList methods {
        object(ChangeMethod::NORMAL, qtrc("inspector", "Normal")),
        object(ChangeMethod::EASE_IN, qtrc("inspector", "Ease in")),
        object(ChangeMethod::EASE_OUT, qtrc("inspector", "Ease out"))
    };

    return methods;
}

void TempoRangedChangePlaybackModel::createProperties()
{
    m_tempoChangeFactor = buildPropertyItem(Pid::TEMPO_CHANGE_FACTOR, [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });

    m_tempoEasingMethod = buildPropertyItem(Pid::TEMPO_EASING_METHOD);
}

void TempoRangedChangePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tempoChangeFactor, [](const QVariant& elementPropertyValue) -> QVariant {
        return static_cast<int>(DataFormatter::roundDouble(elementPropertyValue.toDouble() * 100.0));
    });

    loadPropertyItem(m_tempoEasingMethod);
}

void TempoRangedChangePlaybackModel::resetProperties()
{
    m_tempoChangeFactor->resetToDefault();
    m_tempoEasingMethod->resetToDefault();
}
