/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Internal interface shared across the emitters-*.cpp files: the per-measure and per-element
// context structs, element handlers (note/rest/ornament/chord symbol), staff/voice routing,
// measure gap-fill and overfull-fit helpers, lyrics and tempo emission.

#ifndef MU_IEX_ENCORE_NOTELOOP_INTERNAL_H
#define MU_IEX_ENCORE_NOTELOOP_INTERNAL_H

#include "ctx.h"
#include "emitters-tuplets.h"
#include "../parser/elem.h"
#include "engraving/types/types.h"
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <vector>
#include <array>
#include <optional>

namespace mu::engraving {
class Measure;
class Note;
class ChordRest;
class Tuplet;
}

namespace mu::iex::enc {
// Spare voice reserved for Encore cue notes (and cue-ified dangling graces), kept separate from the
// principal line. Voice index 1 within the staff (track = staffIdx*VOICES + kCueVoice).
inline constexpr int kCueVoice = 1;

// faceValue low nibble: 1=whole, 2=half ... 8=256th; 0 and 9..15 are invalid.
bool isValidFaceValue(quint8 faceValue);
void applyConcertPitch(mu::engraving::Note* n, int semitone);

struct MeasEmitCtx {
    mu::engraving::Measure* measure = nullptr;
    const EncMeasure* encMeas = nullptr;
    mu::engraving::Fraction measTick;
    int measIdx = 0;
    int nLineStaves = 0;
    const std::vector<int>* lineStaffInstrIdx = nullptr;
    const std::vector<int>* lineStaffWithin = nullptr;
    const std::array<int, 256>* lineSlotByRawByte = nullptr;

    std::set<const EncMeasureElem*> validTupletGroupMember;
    std::set<const EncMeasureElem*> partialEndGroup;
    std::vector<NestedTupletInfo> nestedInfos;
    // All notes that belong to an INNER group (the notes inside the nested sub-tuplet).
    std::set<const EncMeasureElem*> innerGroupMembers;
    // Override (actualN, normalN) for notes detected as uniform-fill groups (e.g. 15→[15:8]).
    std::map<const EncMeasureElem*, std::pair<int, int> > overrideGroupRatios;
    // Lookup: first elem of inner group → NestedTupletInfo*
    std::map<const EncMeasureElem*, const NestedTupletInfo*> nestedByInnerFirst;
    // Lookup: last elem of inner group → NestedTupletInfo*
    std::map<const EncMeasureElem*, const NestedTupletInfo*> nestedByInnerLast;
    // key={si,v,tick}, value=sourcePosition from EncTie (+14); -1 means all notes at that tick
    std::multimap<std::tuple<int, int, int>, int8_t> tieStartSet;
    std::set<int> noteTicks;
    std::set<int> voice4NoteTicks;
    // (staffIdx, voice, tick) of every NOTE. A plain (non-tuplet) REST at a position that also holds
    // a note in the same voice is a redundant placeholder Encore writes for the voice; it must be
    // dropped so the note keeps the beat instead of being pushed after the rest.
    std::set<std::tuple<int, int, int> > noteStaffVoiceTicks;
    // Staves (raw index) carrying a real voice-0..3 note in this measure. A whole-measure
    // rest that arrives on Encore's "voice 4" (the silent-voice placeholder) is redundant on
    // such a staff and would corrupt the bar if merged into voice 0; it is skipped.
    std::set<int> stavesWithRealNote;
    std::map<int, int> v0NoteCountAtTick;
    std::map<int, int> ornFingCountAtTick;
    int maxVoice0Tick = -1;
    std::set<std::tuple<int, int, int> > filteredTieSenderPitches;
    // True when at least one note in the measure has au in 0x39..0x40 (scale string anchors).
    // When set, notes with options bit 0 and no other artic byte also show string numbers.
    bool hasScaleStringAnchors { false };

    // notePosition=-1 matches any note (used for bypass checks); otherwise only matches the given position.
    bool isTieStartAt(int si, int v, int tick, int notePosition = -1) const;
    void closeTupletWithFill(BuildCtx& ctx, TupletTracker& tt, std::pair<int, int> trackKey);
};

// Per-element context: computed in the main element loop before dispatch.
struct NoteElemCtx {
    const EncMeasureElem* e = nullptr;
    EncElemType et {};
    int staffIdx = 0;
    int voice = 0;
    int msVoice = 0;
    mu::engraving::track_idx_t track = 0;
    std::pair<int, int> trackKey;
    std::pair<int, int> encVoiceKey;
    bool isChordExt = false;
    bool isNoteOrRest = false;
    mu::engraving::Fraction elemTick;
    int savedPrevMidiTick = -1;
    bool hadLastChordPos = false;
    mu::engraving::Fraction savedLastChordPos;
};

void handleNote(BuildCtx& ctx, MeasEmitCtx& mc, NoteElemCtx& ec);
void handleRest(BuildCtx& ctx, MeasEmitCtx& mc, NoteElemCtx& ec);

// Resolved MuseScore destination for an element: staff/voice and the derived track + lookup keys.
struct RoutedTrack {
    int staffIdx { 0 };
    int voice { 0 };
    int msVoice { 0 };
    mu::engraving::track_idx_t track { 0 };
    std::pair<int, int> trackKey;     // (staffIdx, msVoice)
    std::pair<int, int> encVoiceKey;  // (staffIdx, voice)
};

// Route an element's raw (staffIdx, voice, staffWithin) to a MuseScore (staffIdx, voice, track).
// Returns nullopt if the element should be skipped. (emitters.cpp)
std::optional<RoutedTrack> routeElementStaffVoice(
    const EncMeasureElem* e, bool isNoteOrRest, const std::array<int, 256>& lineSlotByRawByte, const MeasEmitCtx& mc, const BuildCtx& ctx);

// Case-B pickup adjustment: shorten measure 0 if loop placed less than its nominal length. (emitters-fill.cpp)
void adjustPickupMeasure(BuildCtx& ctx, mu::engraving::Measure* measure, int measIdx);
// Pre-fill trailing silence with invisible gap rests. (emitters-fill.cpp)
void fillTrailingGaps(BuildCtx& ctx, mu::engraving::Measure* measure, mu::engraving::Fraction measTick);
// Fix over/undershoots up to 1/24. (emitters-fill.cpp)
void correctMeasureLength(BuildCtx& ctx, mu::engraving::Measure* measure);
// Extend the measure to the max voice content (IrregularMeasure / Stretch fallback). (emitters-fill.cpp)
void extendMeasureIrregular(BuildCtx& ctx, mu::engraving::Measure* measure);
// Nuclear hard-cap: remove trailing elements and fill deficit. (emitters-fill.cpp)
void capMeasureLength(BuildCtx& ctx, mu::engraving::Measure* measure);
// Resolve overfull voices per the overfill strategy (Remove / Stretch / Irregular). (emitters-overfill.cpp)
void fitOverfullMeasure(BuildCtx& ctx, mu::engraving::Measure* measure);

// Collect the ordered ChordRests of one track in a measure; returns their total actual ticks. (emitters-overfill.cpp)
mu::engraving::Fraction collectVoice(mu::engraving::Measure* measure, mu::engraving::track_idx_t tr,
                                     std::vector<mu::engraving::ChordRest*>& out);

// Dissolve a tuplet whole: detach every member (revert to plain face value), remove the empty
// tuplet from its parent and delete it. A tuplet is atomic, never leave a partial one. (emitters-overfill.cpp)
void dissolveTuplet(mu::engraving::Tuplet* t);

// Apply per-measure BPM marks as TempoText elements. (emitters-tempo.cpp)
void applyMeasureBpmMarks(BuildCtx& ctx);

// Render tempo text (beatTicks in display ticks: 240=quarter, 360=dotted-quarter, ...). (emitters-tempo.cpp)
mu::engraving::String tempoXmlText(int displayBpm, int beatTicks);

// Decode the ORN tempo beat unit from the `noto` byte to display ticks, or 0 if unset. (emitters-tempo.cpp)
int notoToBeatTicks(quint8 noto);
} // namespace mu::iex::enc

#endif // MU_IEX_ENCORE_NOTELOOP_INTERNAL_H
