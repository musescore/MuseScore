//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

class Score;
class TDuration;
class Fraction;
class TimeSig;
class Chord;

#include "fraction.h"

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

class MCursor {
      Score* _score;
      int _tick;
      int _track;
      Fraction _sig;

      void createMeasures();

   public:
      MCursor(Score* s = 0);
      void createScore(const QString& s);
      void saveScore();

      void addPart(const QString& instrument);
      Chord* addChord(int pitch, const TDuration& duration);
      void addKeySig(int);
      TimeSig* addTimeSig(const Fraction&);

      void move(int track, int tick);
      Score* score() const    { return _score; }
      void setScore(Score* s) { _score = s;    }
      void setTimesig(Fraction f) { _sig = f; }
      };

#endif

