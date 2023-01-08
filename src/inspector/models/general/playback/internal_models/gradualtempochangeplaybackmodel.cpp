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

#include "gradualtempochangeplaybackmodel.h"

#include "dataformatter.h"

#include "engraving/types/types.h"

using namespace mu::inspector;
using namespace mu::engraving;

GradualTempoChangePlaybackModel::GradualTempoChangePlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository, ElementType::GRADUAL_TEMPO_CHANGE)
{
    setTitle(qtrc("inspector", "Tempo change"));
    setModelType(InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE);

    createProperties();
}

PropertyItem* GradualTempoChangePlaybackModel::tempoChangeFactor() const
{
    return m_tempoChangeFactor;
}

PropertyItem* GradualTempoChangePlaybackModel::tempoEasingMethod() const
{
    return m_tempoEasingMethod;
}

QVariantList GradualTempoChangePlaybackModel::possibleEasingMethods() const
{
    QVariantList methods {
        object(ChangeMethod::NORMAL, qtrc("inspector", "Normal")),
        object(ChangeMethod::EASE_IN, qtrc("inspector", "Ease in")),
        object(ChangeMethod::EASE_OUT, qtrc("inspector", "Ease out"))
    };

    return methods;
}

void GradualTempoChangePlaybackModel::createProperties()
{
    m_tempoChangeFactor = buildPropertyItem(Pid::TEMPO_CHANGE_FACTOR, [](const QVariant& newValue) {
        return newValue.toDouble() / 100;
    });

    m_tempoEasingMethod = buildPropertyItem(Pid::TEMPO_EASING_METHOD);
}

void GradualTempoChangePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tempoChangeFactor, [](const PropertyValue& propertyValue) -> QVariant {
        return static_cast<int>(DataFormatter::roundDouble(propertyValue.toDouble() * 100.0));
    });

    loadPropertyItem(m_tempoEasingMethod);
}

void GradualTempoChangePlaybackModel::resetProperties()
{
    m_tempoChangeFactor->resetToDefault();
    m_tempoEasingMethod->resetToDefault();
}
