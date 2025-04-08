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
#include "playback/utils/expressionutils.h"
#include "playback/utils/repeatutils.h"

#include "noterenderer.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static bool containEqualTremolo(const Chord* c1, const Chord* c2)
{
    if (c1->tremoloSingleChord() && c2->tremoloSingleChord()) {
        return c1->tremoloSingleChord()->tremoloType() == c2->tremoloSingleChord()->tremoloType();
    }

    if (c1->tremoloTwoChord() && c2->tremoloTwoChord()) {
        return c1->tremoloTwoChord()->tremoloType() == c2->tremoloTwoChord()->tremoloType();
    }

    return false;
}

static timestamp_t tremoloTimestampFrom(const Note* note, const RenderingContext& ctx)
{
    const Note* originalNote = note;
    std::unordered_set<const Note*> notes { note };
    int positionTickOffset = ctx.positionTickOffset;

    if (note->incomingPartialTie()) {
        const PartiallyTiedNoteInfo outgoingNoteInfo = findOutgoingNote(note, ctx);
        if (outgoingNoteInfo.isValid() && containEqualTremolo(outgoingNoteInfo.note->chord(), note->chord())) {
            note = outgoingNoteInfo.note;
            positionTickOffset = outgoingNoteInfo.repeat->utick - outgoingNoteInfo.repeat->tick;
        }
    }

    while (note && note->tieBackNonPartial() && note->tieBackNonPartial()->playSpanner()) {
        const Note* prevNote = note->tieBackNonPartial()->startNote();
        const Chord* prevChord = prevNote ? prevNote->chord() : nullptr;

        if (!prevChord || !containEqualTremolo(prevChord, note->chord())) {
            break;
        }

        if (muse::contains(notes, prevNote)) {
            break; // prevents infinite loop
        }

        note = prevNote;
        notes.insert(note);
    }

    if (note && originalNote != note) {
        return timestampFromTicks(ctx.score, note->tick().ticks() + positionTickOffset);
    }

    return ctx.nominalTimestamp;
}

static duration_t tremoloDuration(const Note* note, const timestamp_t tremoloTimestampFrom, const RenderingContext& ctx)
{
    std::unordered_set<const Note*> notes { note };
    int positionTickOffset = ctx.positionTickOffset;

    if (note->outgoingPartialTie()) {
        const PartiallyTiedNoteInfo incomingNoteInfo = findIncomingNote(note, ctx);
        if (incomingNoteInfo.isValid() && containEqualTremolo(incomingNoteInfo.note->chord(), note->chord())) {
            note = incomingNoteInfo.note;
            positionTickOffset = incomingNoteInfo.repeat->utick - incomingNoteInfo.repeat->tick;
        }
    }

    while (note && note->tieForNonPartial() && note->tieForNonPartial()->playSpanner()) {
        const Note* nextNote = note->tieForNonPartial()->endNote();
        const Chord* nextChord = nextNote ? nextNote->chord() : nullptr;

        if (!nextChord || !containEqualTremolo(nextChord, note->chord())) {
            break;
        }

        if (muse::contains(notes, nextNote)) {
            break; // prevents infinite loop
        }

        note = nextNote;
        notes.insert(note);
    }

    if (note) {
        const int tickTo = note->chord()->endTick().ticks();
        const timestamp_t tremoloTimestampTo = timestampFromTicks(ctx.score, tickTo + positionTickOffset);
        return tremoloTimestampTo - tremoloTimestampFrom;
    }

    return ctx.nominalDuration;
}

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
                               const RenderingContext& ctx,
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
    int overallDurationTicks = ctx.nominalDurationTicks;
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

    TremoloTimeCache tremoloTimeCache;

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

            buildAndAppendEvents(currentChord, preferredType, stepDurationTicks, ctx.nominalPositionStartTick + i * stepDurationTicks,
                                 ctx, tremoloTimeCache, result);
        }

        return;
    }

    for (int i = 0; i < stepsCount; ++i) {
        buildAndAppendEvents(chord, preferredType, stepDurationTicks, ctx.nominalPositionStartTick + i * stepDurationTicks,
                             ctx, tremoloTimeCache, result);
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
                                           const int startTick, const RenderingContext& ctx,
                                           TremoloTimeCache& tremoloCache,
                                           mpe::PlaybackEventList& result)
{
    for (const Note* note : chord->notes()) {
        if (!NoteRenderer::shouldRender(note, ctx, ctx.commonArticulations)) {
            continue;
        }

        auto noteTnD = timestampAndDurationFromStartAndDurationTicks(
            ctx.score, startTick, stepDurationTicks, ctx.positionTickOffset);

        NominalNoteCtx noteCtx(note, ctx);
        noteCtx.duration = noteTnD.duration;
        noteCtx.timestamp = noteTnD.timestamp;

        int utick = timestampToTick(ctx.score, noteCtx.timestamp);
        noteCtx.dynamicLevel = ctx.playbackCtx->appliableDynamicLevel(note->track(), utick);

        NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.articulations);

        const TimestampAndDuration& tremoloTnD = tremoloTimeAndDuration(note, ctx, tremoloCache);
        muse::mpe::ArticulationAppliedData& articulationData = noteCtx.articulations.at(type);
        articulationData.meta.timestamp = tremoloTnD.timestamp;
        articulationData.meta.overallDuration = tremoloTnD.duration;

        updateArticulationBoundaries(type, noteCtx.timestamp, noteCtx.duration, noteCtx.articulations);

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}

const TimestampAndDuration& TremoloRenderer::tremoloTimeAndDuration(const Note* note, const RenderingContext& ctx,
                                                                    TremoloTimeCache& cache)
{
    auto cacheIt = cache.find(note);
    if (cacheIt != cache.end()) {
        return cacheIt->second;
    }

    TimestampAndDuration& tnd = cache[note];
    tnd.timestamp = tremoloTimestampFrom(note, ctx);
    tnd.duration = tremoloDuration(note, tnd.timestamp, ctx);

    return tnd;
}
