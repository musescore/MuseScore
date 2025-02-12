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

#pragma once

#include "../types/types.h"
#include "types.h"

#include "interval.h"

#include "draw/types/geometry.h"

namespace mu::engraving {
class Score;
class Chord;
class EngravingItem;
class KeySig;
class Note;
class Rest;
class Measure;
class Score;
class Segment;
class System;
class Staff;
class Tuplet;
class Volta;
struct NoteVal;

enum class Key : signed char;

extern RectF handleRect(const PointF& pos);

extern int pitchKeyAdjust(int note, Key);
extern int line2pitch(int line, ClefType clef, Key);
extern int y2pitch(double y, ClefType clef, double spatium);
extern int quantizeLen(int, int);

extern String pitch2string(int v, bool useFlats = false);
extern int string2pitch(const String& s);
extern String convertPitchStringFlatsAndSharpsToUnicode(const String& str);

extern void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, Interval, bool useDoubleSharpsFlats);
extern int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats);

constexpr int intervalListSize = 26;
extern Interval intervalList[intervalListSize];
extern int searchInterval(int steps, int semitones);
extern int chromatic2diatonic(int val);

int diatonicUpDown(Key, int pitch, int steps);

extern Note* nextChordNote(Note* note);
extern Note* prevChordNote(Note* note);
extern Segment* nextSeg1(Segment* s);
extern Segment* prevSeg1(Segment* seg);

extern Volta* findVolta(const Segment* seg, const Score* score);
extern Note* searchTieNote(const Note* note, const Segment* nextSegment = nullptr, const bool disableOverRepeats = true);
extern Note* searchTieNote114(Note* note);

extern int absStep(int pitch);
extern int absStep(int tpc, int pitch);

extern int absStep(int line, ClefType clef);
extern int relStep(int line, ClefType clef);
extern int relStep(int pitch, int tpc, ClefType clef);
extern int pitch2step(int pitch);
extern int step2pitch(int step);
int chromaticPitchSteps(const Note* noteL, const Note* noteR, const int nominalDiatonicSteps);
extern int noteValToLine(const NoteVal& nval, const Staff* staff, const Fraction& tick);
extern AccidentalVal noteValToAccidentalVal(const NoteVal& nval, const Staff* staff, const Fraction& tick);
extern int compareNotesPos(const Note* n1, const Note* n2);

extern Segment* skipTuplet(Tuplet* tuplet);
extern SymIdList timeSigSymIdsFromString(const String&, TimeSigStyle timeSigStyle = TimeSigStyle::NORMAL);
extern Fraction actualTicks(Fraction duration, Tuplet* tuplet, Fraction timeStretch);

extern double yStaffDifference(const System* system1, const System* system2, staff_idx_t staffIdx1);

extern bool allowRemoveWhenRemovingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff = 0);
extern bool moveDownWhenAddingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff = 0);

extern void collectChordsAndRest(Segment* segment, staff_idx_t staffIdx, std::vector<Chord*>& chords, std::vector<Rest*>& rests);
extern void collectChordsOverlappingRests(Segment* segment, staff_idx_t staffIdx, std::vector<Chord*>& chords);
extern std::vector<EngravingItem*> collectSystemObjects(const Score* score, const std::vector<Staff*>& staves = {});

extern Interval ornamentIntervalToGeneralInterval(OrnamentInterval interval);

extern String formatUniqueExcerptName(const String& baseName, const StringList& allExcerptLowerNames);

extern bool isFirstSystemKeySig(const KeySig* ks);

extern String bendAmountToString(int fulls, int quarts);

extern InstrumentTrackId makeInstrumentTrackId(const EngravingItem* item);

extern std::vector<Measure*> findFollowingRepeatMeasures(const Measure* measure);
extern std::vector<Measure*> findPreviousRepeatMeasures(const Measure* measure);
extern bool repeatHasPartialLyricLine(const Measure* endRepeatMeasure);
extern bool segmentsAreAdjacentInRepeatStructure(const Segment* firstSeg, const Segment* secondSeg);

extern bool chordContainsNoteVal(const Chord* chord, const NoteVal& nval);
} // namespace mu::engraving
