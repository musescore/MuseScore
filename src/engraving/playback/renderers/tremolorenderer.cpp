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

#include "tremolorenderer.h"

#include "dom/chord.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"

#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

const ArticulationTypeSet& TremoloRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Tremolo8th, mpe::ArticulationType::Tremolo16th,
        mpe::ArticulationType::Tremolo32nd, mpe::ArticulationType::Tremolo64th,
        mpe::ArticulationType::TremoloBuzz,
    };

    return types;
}

void TremoloRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType preferredType,
                               const RenderingContext& context,
                               mpe::PlaybackEventList& result)
{
    const Chord* chord = item_cast<const Chord*>(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    struct TremoloAdapter {
        const TremoloSingleChord* single = nullptr;
        const TremoloTwoChord* two = nullptr;

        TremoloAdapter(const Chord* ch)
            : single(ch->tremoloSingleChord()), two(ch->tremoloTwoChord()) {}

        bool hasTremolo() const { return single != nullptr || two != nullptr; }

        int lines() const { return single ? single->lines() : (two ? two->lines() : 0); }
    };

    TremoloAdapter tremolo = TremoloAdapter(chord);
    IF_ASSERT_FAILED(tremolo.hasTremolo()) {
        return;
    }

    // TODO: We need a member like articulationData.overallDurationTicks (ticks rather than duration),
    // so that we are not duplicating this calculation (see TremoloTwoMetaParser::doParse)
    //const ArticulationAppliedData& articulationData = context.commonArticulations.at(preferredType);
    int overallDurationTicks = context.nominalDurationTicks;
    if (tremolo.two && tremolo.two->chord1() && tremolo.two->chord2()) {
        overallDurationTicks = tremolo.two->chord1()->actualTicks().ticks() + tremolo.two->chord2()->actualTicks().ticks();
    }

    int stepDurationTicks = 0;
    if (preferredType == ArticulationType::TremoloBuzz) {
        stepDurationTicks = overallDurationTicks;
    } else {
        stepDurationTicks = TremoloRenderer::stepDurationTicks(chord, tremolo.lines());
    }

    if (stepDurationTicks <= 0) {
        LOGE() << "Unable to render unsupported tremolo type";
        return;
    }

    // ... and use that here
    const int stepsCount = std::round(overallDurationTicks / (float)stepDurationTicks);
    if (stepsCount == 0) {
        return;
    }

    stepDurationTicks = overallDurationTicks / stepsCount;

    if (tremolo.two) {
        const Chord* firstTremoloChord = tremolo.two->chord1();
        const Chord* secondTremoloChord = tremolo.two->chord2();

        IF_ASSERT_FAILED(firstTremoloChord && secondTremoloChord) {
            return;
        }

        for (int i = 0; i < stepsCount; ++i) {
            const Chord* currentChord = firstTremoloChord;

            if (i % 2 != 0) {
                currentChord = secondTremoloChord;
            }

            buildAndAppendEvents(currentChord, preferredType, stepDurationTicks, context.nominalPositionStartTick + i * stepDurationTicks,
                                 context, result);
        }

        return;
    }

    for (int i = 0; i < stepsCount; ++i) {
        buildAndAppendEvents(chord, preferredType, stepDurationTicks, context.nominalPositionStartTick + i * stepDurationTicks,
                             context, result);
    }
}

int TremoloRenderer::stepDurationTicks(const Chord* chord, int tremoloLines)
{
    int ticks = Constants::DIVISION / (1 << (chord->beams() + tremoloLines));
    if (ticks <= 0) {
        return 1;
    }
    return ticks * chord->timeStretchFactor();
}

void TremoloRenderer::buildAndAppendEvents(const Chord* chord, const ArticulationType type,
                                           const int stepDurationTicks,
                                           const int startTick, const RenderingContext& context,
                                           mpe::PlaybackEventList& result)
{
    const Score* score = chord->score();
    IF_ASSERT_FAILED(score) {
        return;
    }

    for (const Note* note : chord->notes()) {
        if (!isNotePlayable(note, context.commonArticulations)) {
            continue;
        }

        auto noteTnD = timestampAndDurationFromStartAndDurationTicks(
            score, startTick, stepDurationTicks, context.positionTickOffset);

        NominalNoteCtx noteCtx(note, context);
        noteCtx.duration = noteTnD.duration;
        noteCtx.timestamp = noteTnD.timestamp;

        int utick = timestampToTick(score, noteCtx.timestamp);
        noteCtx.dynamicLevel = context.playbackCtx->appliableDynamicLevel(note->track(), utick);

        NoteArticulationsParser::buildNoteArticulationMap(note, context, noteCtx.chordCtx.commonArticulations);
        updateArticulationBoundaries(type, noteCtx.timestamp, noteCtx.duration, noteCtx.chordCtx.commonArticulations);

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}
