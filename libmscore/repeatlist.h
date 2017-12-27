//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __REPEATLIST_H__
#define __REPEATLIST_H__

namespace Ms {

class Score;
class Measure;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment {
   public:
      int tick;         // start tick
      int len;
      int utick;
      qreal utime;
      qreal timeOffset;

      RepeatSegment();
      };

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

class RepeatList: public QList<RepeatSegment*>
      {
      Score* _score;
      mutable unsigned idx1, idx2;   // cached values

      RepeatSegment* rs;            // tmp value during unwind()

      void unwindSection(Measure* fm, Measure* em);
      Measure* findStartRepeat(Measure*);
      int findStartFromRepeatCount(Measure * const startFrom, Measure * const sectionEndMeasure);

   public:
      RepeatList(Score* s);
      void unwind();
      int utick2tick(int tick) const;
      int tick2utick(int tick) const;
      void dump() const;
      int utime2utick(qreal) const;
      qreal utick2utime(int) const;
      void update();
      int ticks();
      };


}     // namespace Ms
#endif

