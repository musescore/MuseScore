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

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

class MCursor {
      Score* _score;
      int _tick;
      int _track;

      void createMeasures();

   public:
      MCursor(Score* s = 0);
      void createScore(const QString& s);
      void saveScore();

      void addPart(const QString& instrument);
      void addChord(int pitch, const TDuration& duration);
      void addKeySig(int);
      void addTimeSig(const Fraction&);

      void move(int track, int tick);
      Score* score() const    { return _score; }
      void setScore(Score* s) { _score = s;    }
      };

#endif

