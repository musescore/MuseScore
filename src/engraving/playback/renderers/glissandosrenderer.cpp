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

using namespace mu::engraving;
using namespace mu::mpe;

const ArticulationTypeSet& GlissandosRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::DiscreteGlissando, mpe::ArticulationType::ContinuousGlissando
    };

    return types;
}

void GlissandosRenderer::doRender(const Ms::EngravingItem* item, const mpe::ArticulationType type, const RenderingContext& context,
                                  mpe::PlaybackEventList& result)
{
    const Ms::Note* note = Ms::toNote(item);

    IF_ASSERT_FAILED(note) {
        return;
    }

    if (type == ArticulationType::DiscreteGlissando) {
        renderDiscreteGlissando(note, context, result);
    } else {
        renderContinuousGlissando(note, context, result);
    }
}

void GlissandosRenderer::renderDiscreteGlissando(const Ms::Note* note, const RenderingContext& context,
                                                 mpe::PlaybackEventList& result)
{
    const mpe::ArticulationAppliedData& articulationData = context.commonArticulations.at(ArticulationType::DiscreteGlissando);
    int stepsCount = pitchStepsCount(articulationData.meta.overallPitchChangesRange);
    float durationStep = context.nominalDuration / static_cast<float>(stepsCount);

    for (int i = 0; i < stepsCount; ++i) {
        if (!isNotePlayable(note)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, context);
        noteCtx.duration = durationStep;
        noteCtx.timestamp += i * durationStep;
        noteCtx.pitchLevel += i * mpe::PITCH_LEVEL_STEP;

        updateArticulationBoundaries(ArticulationType::DiscreteGlissando,
                                     noteCtx.timestamp,
                                     noteCtx.duration,
                                     noteCtx.chordCtx.commonArticulations);

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}

void GlissandosRenderer::renderContinuousGlissando(const Ms::Note* note, const RenderingContext& context,
                                                   mpe::PlaybackEventList& result)
{
    if (!isNotePlayable(note)) {
        return;
    }

    result.emplace_back(buildNoteEvent(note, context));
}
