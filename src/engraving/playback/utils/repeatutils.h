/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include "dom/note.h"
#include "dom/partialtie.h"
#include "dom/repeatlist.h"

#include "playback/renderingcontext.h"

namespace mu::engraving {
struct PartiallyTiedNoteInfo {
    const RepeatSegment* repeat = nullptr;
    const Note* note = nullptr;

    bool isValid() const { return repeat && note; }
};

inline PartiallyTiedNoteInfo findOutgoingNoteInPreviousRepeat(const Note* incomingNote, const RenderingContext& ctx)
{
    const RepeatList& repeats = ctx.score->repeatList();
    auto currRepeatIt = repeats.findRepeatSegmentFromUTick(ctx.nominalPositionStartTick + ctx.positionTickOffset);
    if (currRepeatIt == repeats.cbegin() || currRepeatIt == repeats.cend()) {
        return PartiallyTiedNoteInfo();
    }

    if (ctx.nominalPositionStartTick + ctx.positionTickOffset != (*currRepeatIt)->utick) {
        return PartiallyTiedNoteInfo(); // the incoming note is not the first note in this repeat
    }

    auto prevRepeatIt = std::prev(currRepeatIt);
    const RepeatSegment* prevRepeat = (*prevRepeatIt);
    const Measure* lastMeasure = prevRepeat->lastMeasure();
    if (!lastMeasure) {
        return PartiallyTiedNoteInfo();
    }

    const ChordRest* lastChordRest = lastMeasure->lastChordRest(incomingNote->track());
    if (!lastChordRest || !lastChordRest->isChord()) {
        return PartiallyTiedNoteInfo();
    }

    const Note* outgoingNote = toChord(lastChordRest)->findNote(incomingNote->pitch());
    const Tie* tie = outgoingNote ? outgoingNote->tieFor() : nullptr;
    if (tie && tie->playSpanner()) {
        return { prevRepeat, outgoingNote };
    }

    return PartiallyTiedNoteInfo();
}

inline PartiallyTiedNoteInfo findIncomingNoteInNextRepeat(const Note* outgoingNote, const RenderingContext& ctx)
{
    const RepeatList& repeats = ctx.score->repeatList();
    auto currRepeatIt = repeats.findRepeatSegmentFromUTick(ctx.nominalPositionStartTick + ctx.positionTickOffset);
    if (currRepeatIt == repeats.cend()) {
        return PartiallyTiedNoteInfo();
    }

    const RepeatSegment* currRepeat = (*currRepeatIt);
    const int currRepeatLastTick = currRepeat->utick + currRepeat->len();
    if (ctx.nominalPositionStartTick + ctx.positionTickOffset != currRepeatLastTick - ctx.nominalDurationTicks) {
        return PartiallyTiedNoteInfo(); // the outgoing note is not the last note in this repeat
    }

    auto nextRepeatIt = std::next(currRepeatIt);
    if (nextRepeatIt == repeats.cend()) {
        return PartiallyTiedNoteInfo();
    }

    const RepeatSegment* nextRepeat = (*nextRepeatIt);
    const Measure* firstMeasure = nextRepeat->firstMeasure();
    if (!firstMeasure) {
        return PartiallyTiedNoteInfo();
    }

    const ChordRest* firstChordRest = firstMeasure->firstChordRest(outgoingNote->track());
    if (!firstChordRest || !firstChordRest->isChord()) {
        return PartiallyTiedNoteInfo();
    }

    const Note* incomingNote = toChord(firstChordRest)->findNote(outgoingNote->pitch());
    const PartialTie* partialTie = incomingNote ? incomingNote->incomingPartialTie() : nullptr;
    if (partialTie && partialTie->playSpanner()) {
        return { nextRepeat, incomingNote };
    }

    return PartiallyTiedNoteInfo();
}

inline bool notesInSameRepeat(const Score* score, const Note* note1, const Note* note2, const int tickPositionOffset)
{
    const RepeatList& repeats = score->repeatList();
    if (repeats.size() == 1) {
        return true;
    }

    const int firstNoteTick = note1->tick().ticks();
    const int secondNoteTick = note2->tick().ticks();

    for (const RepeatSegment* repeat : repeats) {
        const int offset = repeat->utick - repeat->tick;
        if (offset != tickPositionOffset) {
            continue;
        }

        if (firstNoteTick >= repeat->tick && secondNoteTick >= repeat->tick) {
            const int lastRepeatTick = repeat->tick + repeat->len();
            if (firstNoteTick < lastRepeatTick && secondNoteTick < lastRepeatTick) {
                return true;
            }
        }
    }

    return false;
}
}
