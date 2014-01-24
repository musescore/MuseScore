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

#ifndef __DURATIONLIST_H__
#define __DURATIONLIST_H__

#include "fraction.h"

namespace Ms {

class Element;
class Measure;
class Tuplet;
class Segment;
class Spanner;
class ScoreRange;
class ChordRest;

//---------------------------------------------------------
//   TrackList
//---------------------------------------------------------

class TrackList : public QList<Element*>
      {
      Fraction _duration;
      ScoreRange* _range;

      Tuplet* writeTuplet(Tuplet* tuplet, Measure* measure, int tick) const;
      void append(Element*);

   public:
      TrackList(ScoreRange* r) { _range = r; }
      ~TrackList();
      void read(int track, const Segment* fs, const Segment* ls);
      bool canWrite(const Fraction& f) const;
      bool write(int track, Measure*) const;
      Fraction duration() const  { return _duration; }
      ScoreRange* range() const { return _range; }
      void appendGap(const Fraction&);
      void dump() const;
      };

//---------------------------------------------------------
//   Annotation
//---------------------------------------------------------

struct Annotation {
      int tick;
      Element* e;
      };

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

class ScoreRange {
      QList<TrackList*> tracks;
      Segment* _first;
      Segment* _last;

   protected:
      QList<Spanner*> spanner;
      QList<Annotation> annotations;

   public:
      ScoreRange() {}
      ~ScoreRange();
      void read(Segment* first, Segment* last, int startTrack, int endTrack);
      bool canWrite(const Fraction&) const;
      bool write(int track, Measure*) const;
      void fixup(Measure*) const;
      Fraction duration() const;
      Segment* first() const { return _first; }
      Segment* last() const  { return _last;  }
      void fill(const Fraction&);

      friend class TrackList;
      };


}     // namespace Ms
#endif

