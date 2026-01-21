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

PlaybackProxyModel::PlaybackProxyModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent, iocCtx, repository)
{
    QList<AbstractInspectorModel*> models {
        new NotePlaybackModel(this, iocCtx, repository),
        new ArpeggioPlaybackModel(this, iocCtx, repository),
        new FermataPlaybackModel(this, iocCtx, repository),
        new BreathPlaybackModel(this, iocCtx, repository),
        new GlissandoPlaybackModel(this, iocCtx, repository),
        new GradualTempoChangePlaybackModel(this, iocCtx, repository)
    };

    for (AbstractInspectorModel* model : models) {
        model->init();
    }

    setModels(models);
}
