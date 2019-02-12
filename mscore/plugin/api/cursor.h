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

namespace Ms {

class Element;
class Score;
class Chord;
class Rest;
class Note;
class Segment;
class RepeatSegment;
class ChordRest;
class StaffText;
class Measure;

enum class SegmentType;

namespace PluginAPI {

class Element;
class Measure;
class Segment;
class Score;

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

      Q_PROPERTY(int tick         READ tick)
      Q_PROPERTY(double time      READ time)

      //@ get tempo at current tick
      Q_PROPERTY(qreal tempo      READ tempo)

      Q_PROPERTY(int keySignature READ qmlKeySignature)
      Q_PROPERTY(Ms::PluginAPI::Score* score READ score    WRITE setScore)

      Q_PROPERTY(Ms::PluginAPI::Element* element READ element)
      Q_PROPERTY(Ms::PluginAPI::Segment*  segment READ segment)
      Q_PROPERTY(Ms::PluginAPI::Measure*  measure READ measure)

   public:
      enum RewindMode {
            SCORE_START = 0,
            SELECTION_START = 1,
            SELECTION_END = 2
            };
      Q_ENUM(RewindMode)

   private:
      Ms::Score* _score = nullptr;
      int _track = 0;
//       bool _expandRepeats; // used?

      //state
      Ms::Segment* _segment = nullptr;
      SegmentType _filter;

      // utility methods
      void nextInTrack();
      void setScore(Ms::Score* s);

   public:
      Cursor(Ms::Score* s = nullptr);
//       Cursor(Score*, bool); // not implemented? what is bool?

      Score* score() const;
      void setScore(Score* s);

      int track() const             { return _track;    }
      void setTrack(int v);

      int staffIdx() const;
      void setStaffIdx(int v);

      int voice() const;
      void setVoice(int v);

      int filter() const            { return int(_filter); }
      void setFilter(int f)         { _filter = SegmentType(f); }

      Element* element() const;
      Segment* segment() const;
      Measure* measure() const;

      int tick();
      double time();
      qreal tempo();

      int qmlKeySignature();

      //@ rewind cursor
      //@   mode=SCORE_START      rewind to start of score
      //@   mode=SELECTION_START  rewind to start of selection
      //@   mode=SELECTION_END    rewind to end of selection
      Q_INVOKABLE void rewind(RewindMode mode);

      Q_INVOKABLE bool next();
      Q_INVOKABLE bool nextMeasure();
      Q_INVOKABLE void add(Ms::PluginAPI::Element*);

      Q_INVOKABLE void addNote(int pitch);

      //@ set duration
      //@   z: numerator
      //@   n: denominator
      //@   Quarter, if n == 0
      Q_INVOKABLE void setDuration(int z, int n);
      };

}     // namespace PluginAPI
}     // namespace Ms
#endif

