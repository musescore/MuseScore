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

#ifndef MU_ENGRAVING_UTILS_H
#define MU_ENGRAVING_UTILS_H

#include "types/types.h"

#include "interval.h"

#include "draw/types/geometry.h"

namespace mu::engraving {
class Chord;
class EngravingItem;
class KeySig;
class Note;
class Rest;
class Segment;
class System;
class Tuplet;

enum class Key;

extern mu::RectF handleRect(const mu::PointF& pos);

extern int getStaff(System* system, const mu::PointF& p);
extern int pitchKeyAdjust(int note, Key);
extern int line2pitch(int line, ClefType clef, Key);
extern int y2pitch(double y, ClefType clef, double spatium);
extern int quantizeLen(int, int);
extern String pitch2string(int v, bool useFlats = false);
extern int string2pitch(const String& s);
extern void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, Interval, bool useDoubleSharpsFlats);
extern int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats);

constexpr int intervalListSize = 26;
extern Interval intervalList[intervalListSize];
extern int searchInterval(int steps, int semitones);
extern int chromatic2diatonic(int val);

int diatonicUpDown(Key, int pitch, int steps);

extern Note* nextChordNote(Note* note);
extern Note* prevChordNote(Note* note);
extern Segment* nextSeg1(Segment* s, track_idx_t& track);
extern Segment* prevSeg1(Segment* seg, track_idx_t& track);

extern Note* searchTieNote(Note* note);
extern Note* searchTieNote114(Note* note);

extern int absStep(int pitch);
extern int absStep(int tpc, int pitch);

extern int absStep(int line, ClefType clef);
extern int relStep(int line, ClefType clef);
extern int relStep(int pitch, int tpc, ClefType clef);
extern int pitch2step(int pitch);
extern int step2pitch(int step);
int chromaticPitchSteps(const Note* noteL, const Note* noteR, const int nominalDiatonicSteps);

extern Segment* skipTuplet(Tuplet* tuplet);
extern SymIdList timeSigSymIdsFromString(const String&);
extern Fraction actualTicks(Fraction duration, Tuplet* tuplet, Fraction timeStretch);

extern double yStaffDifference(const System* system1, staff_idx_t staffIdx1, const System* system2, staff_idx_t staffIdx2);

extern bool allowRemoveWhenRemovingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff = 0);
extern bool moveDownWhenAddingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff = 0);

extern void collectChordsAndRest(Segment* segment, staff_idx_t staffIdx, std::vector<Chord*>& chords, std::vector<Rest*>& rests);
extern void collectChordsOverlappingRests(Segment* segment, staff_idx_t staffIdx, std::vector<Chord*>& chords);

extern Interval ornamentIntervalToGeneralInterval(OrnamentInterval interval);

extern String formatUniqueExcerptName(const String& baseName, const StringList& allExcerptLowerNames);

extern bool isFirstSystemKeySig(const KeySig* ks);

extern String bendAmountToString(int fulls, int quarts);
} // namespace mu::engraving
#endif
