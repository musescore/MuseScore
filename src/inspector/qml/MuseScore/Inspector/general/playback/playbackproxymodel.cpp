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
#include "playbackproxymodel.h"

#include "abstractinspectormodel.h"
#include "internal/noteplaybackmodel.h"
#include "internal/arpeggioplaybackmodel.h"
#include "internal/fermataplaybackmodel.h"
#include "internal/breathplaybackmodel.h"
#include "internal/glissandoplaybackmodel.h"
#include "internal/gradualtempochangeplaybackmodel.h"

using namespace mu::inspector;

PlaybackProxyModel::PlaybackProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, repository)
{
    QList<AbstractInspectorModel*> models {
        new NotePlaybackModel(this, repository),
        new ArpeggioPlaybackModel(this, repository),
        new FermataPlaybackModel(this, repository),
        new BreathPlaybackModel(this, repository),
        new GlissandoPlaybackModel(this, repository),
        new GradualTempoChangePlaybackModel(this, repository)
    };

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);
}
