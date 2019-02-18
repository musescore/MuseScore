//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MCURSOR_H__
#define __MCURSOR_H__

#include "fraction.h"

namespace Ms {

class MasterScore;
class Score;
class TDuration;
class Fraction;
class TimeSig;
class Chord;
enum class Key;

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

class MCursor {
      MasterScore* _score;
      Fraction _tick;
      int _track;
      Fraction _sig;

      void createMeasures();

   public:
      MCursor(MasterScore* s = 0);
      void createScore(const QString& s);
      void saveScore();

      void addPart(const QString& instrument);
      Chord* addChord(int pitch, const TDuration& duration);
      void addKeySig(Key);
      TimeSig* addTimeSig(const Fraction&);

      void move(int track, const Fraction& tick);
      MasterScore* score() const    { return _score; }
      void setScore(MasterScore* s) { _score = s;    }
      void setTimeSig(Fraction f)   { _sig = f; }
      };


}     // namespace Ms
#endif

