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

#include "glissandosrenderer.h"

#include "dom/glissando.h"

#include "playback/renderingcontext.h"
#include "playback/utils/expressionutils.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static std::vector<int> pitchSteps(const Note* note)
{
    const Glissando* glissando = nullptr;
    for (const Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            glissando = toGlissando(spanner);
            break;
        }
    }

    if (!glissando) {
        return {};
    }

    std::vector<int> pitchSteps;
    Glissando::pitchSteps(glissando, pitchSteps);

    return pitchSteps;
}

muse::mpe::duration_t GlissandosRenderer::discreteGlissandoStepDuration(const Note* note, const duration_t noteDuration)
{
    const std::vector<int> steps = pitchSteps(note);
    if (steps.empty()) {
        return 0;
    }

    return noteDuration / static_cast<float>(steps.size());
}

void GlissandosRenderer::renderDiscreteGlissando(const Note* note, NominalNoteCtx& ctx, mpe::PlaybackEventList& result)
{
    const std::vector<int> steps = pitchSteps(note);
    if (steps.empty()) {
        return;
    }

    const size_t stepsCount = steps.size();
    const float durationStep = ctx.duration / static_cast<float>(stepsCount);
    size_t firstStep = 0;

    if (note->tieBack()) {
        mpe::ArticulationMeta& meta = ctx.articulations.at(ArticulationType::DiscreteGlissando).meta;
        meta.timestamp += durationStep;
        meta.overallDuration -= durationStep;
        firstStep = 1;
    }

    for (size_t i = firstStep; i < stepsCount; ++i) {
        NominalNoteCtx noteCtx(ctx);
        noteCtx.duration = durationStep;
        noteCtx.timestamp += i * durationStep;
        noteCtx.pitchLevel += steps.at(i) * mpe::PITCH_LEVEL_STEP;

        const int utick = timestampToTick(noteCtx.chordCtx.score, noteCtx.timestamp);
        noteCtx.dynamicLevel = ctx.chordCtx.playbackCtx->appliableDynamicLevel(note->track(), utick);

        updateArticulationBoundaries(ArticulationType::DiscreteGlissando,
                                     noteCtx.timestamp,
                                     noteCtx.duration,
                                     noteCtx.articulations);

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}

void GlissandosRenderer::renderContinuousGlissando(const Note*, NominalNoteCtx& ctx, muse::mpe::PlaybackEventList& result)
{
    result.emplace_back(buildNoteEvent(ctx));
}
