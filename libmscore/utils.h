//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __UTILS_H__
#define __UTILS_H__

#include "mscore.h"
#include "interval.h"
#include "sym.h"

namespace Ms {

class KeySig;

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
class Element;
class Note;
class Tuplet;
class BarLine;
class Fraction;

enum class ClefType : signed char;

extern QRectF handleRect(const QPointF& pos);

extern int getStaff(System* system, const QPointF& p);
extern int pitchKeyAdjust(int note, Key);
extern int line2pitch(int line, ClefType clef, Key);
extern int y2pitch(qreal y, ClefType clef, qreal spatium);
extern int quantizeLen(int, int);
extern QString pitch2string(int v);
extern void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc,
   Interval, bool useDoubleSharpsFlats);
extern int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats);

constexpr int intervalListSize = 26;
extern Interval intervalList[intervalListSize];
extern int searchInterval(int steps, int semitones);
extern int chromatic2diatonic(int val);

int diatonicUpDown(Key, int pitch, int steps);

extern int version();
extern int majorVersion();
extern int minorVersion();
extern int updateVersion();
extern bool compareVersion(QString v1, QString v2);

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
extern std::vector<SymId> toTimeSigString(const QString&);
extern Fraction actualTicks(Fraction duration, Tuplet* tuplet, Fraction timeStretch);


extern double yStaffDifference(const System* system1, int staffIdx1, const System* system2, int staffIdx2);
extern bool isFirstSystemKeySig(const KeySig* ks);
}     // namespace Ms
#endif

