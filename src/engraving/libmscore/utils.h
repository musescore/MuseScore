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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "types/types.h"

#include "mscore.h"
#include "interval.h"

#include "infrastructure/draw/geometry.h"

namespace Ms {
enum class Key;

//---------------------------------------------------------
//   cycles
//---------------------------------------------------------

/*static inline unsigned long long cycles()
      {
      unsigned long long rv;
      __asm__ __volatile__("rdtsc" : "=A" (rv));
      return rv;
      }*/

class Measure;
class Segment;
class System;
class EngravingItem;
class Note;
class Tuplet;
class BarLine;

extern mu::RectF handleRect(const mu::PointF& pos);

extern int getStaff(System* system, const mu::PointF& p);
extern int pitchKeyAdjust(int note, Key);
extern int line2pitch(int line, ClefType clef, Key);
extern int y2pitch(qreal y, ClefType clef, qreal spatium);
extern int quantizeLen(int, int);
extern QString pitch2string(int v);
extern void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, Interval, bool useDoubleSharpsFlats);
extern int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats);

constexpr int intervalListSize = 26;
extern Interval intervalList[intervalListSize];
extern int searchInterval(int steps, int semitones);
extern int chromatic2diatonic(int val);

int diatonicUpDown(Key, int pitch, int steps);

extern int version();
extern int majorVersion();
extern int minorVersion();
extern int patchVersion();

extern Note* nextChordNote(Note* note);
extern Note* prevChordNote(Note* note);
extern Segment* nextSeg1(Segment* s, int& track);
extern Segment* prevSeg1(Segment* seg, int& track);

extern Note* searchTieNote(Note* note);
extern Note* searchTieNote114(Note* note);

extern int absStep(int pitch);
extern int absStep(int tpc, int pitch);

extern int absStep(int line, ClefType clef);
extern int relStep(int line, ClefType clef);
extern int relStep(int pitch, int tpc, ClefType clef);
extern int pitch2step(int pitch);
extern int step2pitch(int step);

extern Segment* skipTuplet(Tuplet* tuplet);
extern SymIdList timeSigSymIdsFromString(const QString&);
extern Fraction actualTicks(Fraction duration, Tuplet* tuplet, Fraction timeStretch);
}     // namespace Ms
#endif
