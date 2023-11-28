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

#include "arpeggiorenderer.h"

#include "dom/chord.h"
#include "dom/arpeggio.h"

#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

const ArticulationTypeSet& ArpeggioRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Arpeggio, mpe::ArticulationType::ArpeggioDown,
        mpe::ArticulationType::ArpeggioStraightDown, mpe::ArticulationType::ArpeggioStraightUp,
        mpe::ArticulationType::ArpeggioUp
    };

    return types;
}

void ArpeggioRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType preferredType,
                                const RenderingContext& context,
                                mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    const Arpeggio* arpeggio = chord->arpeggio();
    IF_ASSERT_FAILED(arpeggio) {
        return;
    }

    int stepsCount = static_cast<int>(chord->notes().size());
    mpe::percentage_t percentageStep = mpe::HUNDRED_PERCENT / stepsCount;
    msecs_t offsetStep = timestampOffsetStep(context, stepsCount);
    double stretch = arpeggio->Stretch();

    auto buildEvent = [&](NominalNoteCtx& noteCtx, const int stepNumber) {
        noteCtx.chordCtx.commonArticulations.updateOccupiedRange(preferredType, stepNumber * percentageStep,
                                                                 (stepNumber + 1) * percentageStep);
        timestamp_t offset = offsetStep * stepNumber * stretch;
        noteCtx.timestamp += offset;
        noteCtx.duration -= offset;

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    };

    std::map<pitch_level_t, NominalNoteCtx> noteCtxMap = arpeggioNotes(chord, context);

    if (isDirectionUp(preferredType)) {
        for (auto it = noteCtxMap.begin(); it != noteCtxMap.end(); ++it) {
            buildEvent(it->second, std::distance(noteCtxMap.begin(), it));
        }
    } else {
        for (auto it = noteCtxMap.rbegin(); it != noteCtxMap.rend(); ++it) {
            buildEvent(it->second, std::distance(noteCtxMap.rbegin(), it));
        }
    }
}

bool ArpeggioRenderer::isDirectionUp(const mpe::ArticulationType type)
{
    switch (type) {
    case mpe::ArticulationType::ArpeggioDown:
    case mpe::ArticulationType::ArpeggioStraightDown:
        return false;
    case mpe::ArticulationType::Arpeggio:
    case mpe::ArticulationType::ArpeggioUp:
    case mpe::ArticulationType::ArpeggioStraightUp:
        return true;
    default:
        return false;
    }
}

msecs_t ArpeggioRenderer::timestampOffsetStep(const RenderingContext& ctx, int stepCount)
{
    constexpr int MAX_TIMESTAMP_OFFSET_STEP = 60000;

    int offsetStep = ctx.nominalDuration / stepCount;
    if (offsetStep < MAX_TIMESTAMP_OFFSET_STEP) {
        return offsetStep;
    }

    if (RealIsEqualOrMore(ctx.beatsPerSecond.val, PRESTISSIMO_BPS_BOUND)) {
        return MAX_TIMESTAMP_OFFSET_STEP * 1.5;
    }

    if (RealIsEqualOrMore(ctx.beatsPerSecond.val, PRESTO_BPS_BOUND)) {
        return MAX_TIMESTAMP_OFFSET_STEP * 1.25;
    }

    if (RealIsEqualOrMore(ctx.beatsPerSecond.val, MODERATO_BPS_BOUND)) {
        return MAX_TIMESTAMP_OFFSET_STEP;
    }

    return MAX_TIMESTAMP_OFFSET_STEP;
}

std::map<pitch_level_t, NominalNoteCtx> ArpeggioRenderer::arpeggioNotes(const Chord* chord, const RenderingContext& ctx)
{
    std::map<pitch_level_t, NominalNoteCtx> result;

    const Score* score = chord->score();

    for (const Note* note : chord->notes()) {
        if (!isNotePlayable(note, ctx.commonArticulations)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, ctx);
        noteCtx.duration = timestampFromTicks(score, note->playTicks());

        NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.chordCtx.commonArticulations);

        result.emplace(noteCtx.pitchLevel, std::move(noteCtx));
    }

    return result;
}
