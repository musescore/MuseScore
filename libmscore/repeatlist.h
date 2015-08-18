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
//    Each segment represents a string of consecutive measures played in order after repeats & jumps are unwound.
//    Only responsible for mapping uticks <-> ticks for each segment.
//    Not responsible for mapping uticks <-> utime, which is now handled by score's unrolled tempomap.
//---------------------------------------------------------

class RepeatSegment {
   public:
      int tick;         // start tick
      int len;
      int utick;

      RepeatSegment();

      void dump() const;      // debug print to console
      };

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

class RepeatList: public QList<RepeatSegment*>
      {
      Score* _score;

      mutable unsigned idx1, idx2;   // cached values

      RepeatSegment* rs;            // tmp value during unwind()

      Measure* jumpToStartRepeat(Measure*);

   public:
      RepeatList(Score* s);
      void unwind();
      int utick2tick(int tick) const;
      int tick2utick(int tick) const;
      void dump() const;
      void update();
      int ticks();
      };


}     // namespace Ms
#endif

