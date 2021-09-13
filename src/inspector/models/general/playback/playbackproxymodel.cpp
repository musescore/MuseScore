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
#include "playbackproxymodel.h"

#include "internal_models/noteplaybackmodel.h"
#include "internal_models/arpeggioplaybackmodel.h"
#include "internal_models/fermataplaybackmodel.h"
#include "internal_models/breathplaybackmodel.h"
#include "internal_models/glissandoplaybackmodel.h"
#include "internal_models/dynamicplaybackmodel.h"
#include "internal_models/hairpinplaybackmodel.h"

using namespace mu::inspector;

PlaybackProxyModel::PlaybackProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    addModel(new NotePlaybackModel(this, repository));
    addModel(new ArpeggioPlaybackModel(this, repository));
    addModel(new FermataPlaybackModel(this, repository));
    addModel(new BreathPlaybackModel(this, repository));
    addModel(new GlissandoPlaybackModel(this, repository));
    addModel(new DynamicPlaybackModel(this, repository));
    addModel(new HairpinPlaybackModel(this, repository));
}

bool PlaybackProxyModel::hasGeneralSettings() const
{
    static const QList<InspectorModelType> generalGroup {
        InspectorModelType::TYPE_NOTE,
        InspectorModelType::TYPE_ARPEGGIO,
        InspectorModelType::TYPE_FERMATA,
        InspectorModelType::TYPE_BREATH,
        InspectorModelType::TYPE_GLISSANDO
    };

    return !isGropEmpty(generalGroup);
}

bool PlaybackProxyModel::hasDynamicsSettings() const
{
    static const QList<InspectorModelType> dynamicsGroup {
        InspectorModelType::TYPE_DYNAMIC,
        InspectorModelType::TYPE_HAIRPIN
    };

    return !isGropEmpty(dynamicsGroup);
}

bool PlaybackProxyModel::isGropEmpty(const QList<InspectorModelType>& group) const
{
    for (const AbstractInspectorModel* model : modelList()) {
        if (group.contains(model->modelType()) && !model->isEmpty()) {
            return false;
        }
    }

    return true;
}
