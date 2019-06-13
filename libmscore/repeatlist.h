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
class Volta;
class Jump;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment {
   private:
      QList<std::pair<Measure*, int>> measureList; //measure, playbackCount
   public:
      int tick;         // start tick
      int utick;
      qreal utime;
      qreal timeOffset;

      RepeatSegment();
      RepeatSegment(RepeatSegment * const, Measure * const fromMeasure, Measure * const untilMeasure);
      void addMeasure(Measure * const);
      bool containsMeasure(Measure const * const) const;
      int len() const;
      int playbackCount(Measure * const) const;

      Measure* firstMeasure() const { return measureList.empty() ? nullptr : measureList.front().first; }
      Measure* lastMeasure() const  { return measureList.empty() ? nullptr : measureList.back().first;  }

      friend class RepeatList;
      };

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

class RepeatList: public QList<RepeatSegment*>
      {
      Score* _score;
      mutable unsigned idx1, idx2;   // cached values

      bool _expanded = false;
      bool _scoreChanged = true;

      std::map<Volta*, Measure*> _voltaRanges; // open volta possibly ends past the end of its spanner, used during unwind
      std::set<Jump*> _jumpsTaken;   // take the jumps only once, so track them during unwind

      void preProcessVoltas();
      std::map<Volta*, Measure*>::const_iterator searchVolta(Measure * const) const;
      void unwindSection(Measure * const fm, Measure * const em);
      Measure* findStartRepeat(Measure * const) const;
      int findStartFromRepeatCount(Measure * const startFrom) const;
      bool isFinalPlaythrough(Measure * const measure, QList<RepeatSegment*>::const_iterator repeatSegmentIt) const;

      void unwind();
      void flatten();

   public:
      RepeatList(Score* s);
      RepeatList(const RepeatList&) = delete;
      RepeatList& operator=(const RepeatList&) = delete;
      ~RepeatList();
      void update(bool expand);
      void setScoreChanged() { _scoreChanged = true; }
      const Score* score() const { return _score; }
      int utick2tick(int tick) const;
      int tick2utick(int tick) const;
      void dump() const;
      int utime2utick(qreal) const;
      qreal utick2utime(int) const;
      void updateTempo();
      int ticks() const;
      };


}     // namespace Ms
#endif

