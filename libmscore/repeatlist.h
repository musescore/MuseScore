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
class RepeatListElement;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment {
   private:
      QList<Measure const *> measureList;
   public:
      int tick;         // start tick
      int utick;
      qreal utime;
      qreal timeOffset;
      int playbackCount;

      RepeatSegment(int playbackCount);

      void addMeasure(Measure const * const);
      void addMeasures(Measure const * const);
      bool containsMeasure(Measure const * const) const;
      bool isEmpty() const;
      int len() const;
      void popMeasure();

      Measure const * firstMeasure() const { return measureList.empty() ? nullptr : measureList.front(); }
      Measure const * lastMeasure() const  { return measureList.empty() ? nullptr : measureList.back();  }

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

      std::set<std::pair<Jump const * const, int>> _jumpsTaken;   // take the jumps only once, so track them during unwind
      QList<QList<RepeatListElement*>*> _rlElements; // all elements of the score that influence the RepeatList

      void collectRepeatListElements();
      std::pair<QList<QList<RepeatListElement*>*>::const_iterator , QList<RepeatListElement*>::const_iterator> findMarker(
            QString label, QList<QList<RepeatListElement*>*>::const_iterator referenceSectionIt, QList<RepeatListElement*>::const_iterator referenceRepeatListElementIt) const;
      void performJump(QList<QList<RepeatListElement*>*>::const_iterator sectionIt, QList<RepeatListElement*>::const_iterator repeatListElementTargetIt,
                       bool withRepeats, int * const playbackCount,
                       Volta const * * const activeVolta, RepeatListElement const * * const startRepeatReference) const;
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
      int utime2utick(qreal) const;
      qreal utick2utime(int) const;
      void updateTempo();
      int ticks() const;
      };


}     // namespace Ms
#endif

