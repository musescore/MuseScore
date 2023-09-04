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

#include "glissandosrenderer.h"

#include "dom/glissando.h"

using namespace mu::engraving;
using namespace mu::mpe;

const ArticulationTypeSet& GlissandosRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::DiscreteGlissando, mpe::ArticulationType::ContinuousGlissando
    };

    return types;
}

void GlissandosRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType type,
                                  const RenderingContext& context,
                                  mpe::PlaybackEventList& result)
{
    const Note* note = toNote(item);

    IF_ASSERT_FAILED(note) {
        return;
    }

    if (type == ArticulationType::DiscreteGlissando) {
        renderDiscreteGlissando(note, context, result);
    } else {
        renderContinuousGlissando(note, context, result);
    }
}

void GlissandosRenderer::renderDiscreteGlissando(const Note* note, const RenderingContext& context, mpe::PlaybackEventList& result)
{
    const Glissando* glissando = nullptr;
    for (const Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            glissando = toGlissando(spanner);
            break;
        }
    }

    if (!glissando) {
        return;
    }

    std::vector<int> pitchSteps;
    if (!Glissando::pitchSteps(glissando, pitchSteps)) {
        return;
    }

    size_t stepsCount = pitchSteps.size();

    float durationStep = context.nominalDuration / static_cast<float>(stepsCount);

    for (size_t i = 0; i < stepsCount; ++i) {
        if (!isNotePlayable(note, context.commonArticulations)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, context);
        noteCtx.duration = durationStep;
        noteCtx.timestamp += i * durationStep;
        noteCtx.pitchLevel += pitchSteps.at(i) * mpe::PITCH_LEVEL_STEP;

        updateArticulationBoundaries(ArticulationType::DiscreteGlissando,
                                     noteCtx.timestamp,
                                     noteCtx.duration,
                                     noteCtx.chordCtx.commonArticulations);

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}

void GlissandosRenderer::renderContinuousGlissando(const Note* note, const RenderingContext& context, mpe::PlaybackEventList& result)
{
    if (!isNotePlayable(note, context.commonArticulations)) {
        return;
    }

    result.emplace_back(buildNoteEvent(note, context));
}

pitch_level_t GlissandosRenderer::pitchLevelStep(const mpe::ArticulationAppliedData& articulationData)
{
    if (articulationData.meta.overallPitchChangesRange < 0) {
        return -mpe::PITCH_LEVEL_STEP;
    }

    return mpe::PITCH_LEVEL_STEP;
}
