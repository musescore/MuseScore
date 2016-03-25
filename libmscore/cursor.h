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

#ifndef __CURSOR_H__
#define __CURSOR_H__

#include "segment.h"

namespace Ms {

class Element;
class Score;
class Chord;
class Rest;
class Note;
// class Segment;
class RepeatSegment;
class ChordRest;
class StaffText;
class Measure;

//---------------------------------------------------------
//   @@ Cursor
//   @P track     int           current track
//   @P staffIdx  int           current staff (track / 4)
//   @P voice     int           current voice (track % 4)
//   @P filter    enum          segment type filter
//   @P element   Ms::Element*  current element at track, read only
//   @P segment   Ms::Segment*  current segment, read only
//   @P measure   Ms::Measure*  current measure, read only
//   @P tick      int           midi tick position, read only
//   @P time      double        time at tick position, read only
//   @P keySignature int        key signature of current staff at tick pos. (read only)
//   @P score     Ms::Score*    associated score
//---------------------------------------------------------

class Cursor : public QObject {
      Q_OBJECT
      Q_PROPERTY(int track      READ track     WRITE setTrack)
      Q_PROPERTY(int staffIdx   READ staffIdx  WRITE setStaffIdx)
      Q_PROPERTY(int voice      READ voice     WRITE setVoice)
      Q_PROPERTY(int filter     READ filter    WRITE setFilter)

      Q_PROPERTY(Ms::Element* element READ element)
      Q_PROPERTY(Ms::Segment* segment READ segment)
      Q_PROPERTY(Ms::Measure* measure READ measure)

      Q_PROPERTY(int tick         READ tick)
      Q_PROPERTY(double time      READ time)

      //@ get tempo at current tick
      Q_PROPERTY(qreal tempo      READ tempo)

      Q_PROPERTY(int keySignature READ qmlKeySignature)
      Q_PROPERTY(Ms::Score* score READ score    WRITE setScore)

      Score* _score;
      int _track;
      bool _expandRepeats;

      //state
      Segment* _segment;
      Segment::Type _filter { Segment::Type::ChordRest };

      // utility methods
      void nextInTrack();

   public:
      Cursor(Score* c = 0);
      Cursor(Score*, bool);

      Score* score() const          { return _score;    }
      void setScore(Score* s);

      int track() const             { return _track;    }
      void setTrack(int v);

      int staffIdx() const;
      void setStaffIdx(int v);

      int voice() const;
      void setVoice(int v);

      int filter() const            { return int(_filter); }
      void setFilter(int f)         { _filter = Segment::Type(f); }

      Element* element() const;
      Segment* segment() const      { return _segment;  }
      Measure* measure() const;

      int tick();
      double time();
      qreal tempo();

      int qmlKeySignature();

      //@ rewind cursor
      //@   type=0      rewind to start of score
      //@   type=1      rewind to start of selection
      //@   type=2      rewind to end of selection
      Q_INVOKABLE void rewind(int type);

      Q_INVOKABLE bool next();
      Q_INVOKABLE bool nextMeasure();
      Q_INVOKABLE void add(Ms::Element*);

      Q_INVOKABLE void addNote(int pitch);

      //@ set duration
      //@   z: numerator
      //@   n: denominator
      //@   Quarter, if n == 0
      Q_INVOKABLE void setDuration(int z, int n);
      };

}     // namespace Ms
#endif

