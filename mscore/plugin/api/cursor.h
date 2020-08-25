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

#include "fraction.h"

namespace Ms {

class Element;
class InputState;
class Score;
class Chord;
class Rest;
class Note;
class Segment;
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
///   Cursor can be used by plugins to manipulate the score.
///   Cursor object for a score can be obtained with
///   \ref Score.newCursor method. After creating a cursor
///   it does not point to any location in a score. To define
///   its initial location use \ref rewind or \ref rewindToTick
///   methods. Alternatively, you can set its
///   \ref inputStateMode to \ref INPUT_STATE_SYNC_WITH_SCORE "Cursor.INPUT_STATE_SYNC_WITH_SCORE"
///   to make cursor location be synchronized with
///   user-visible note input state.
//---------------------------------------------------------

class Cursor : public QObject {
      Q_OBJECT
      /** Current track */
      Q_PROPERTY(int track      READ track     WRITE setTrack)
      /** Current staff (#track / 4) */
      Q_PROPERTY(int staffIdx   READ staffIdx  WRITE setStaffIdx)
      /** Current voice (#track % 4) */
      Q_PROPERTY(int voice      READ voice     WRITE setVoice)
      /**
       * Segment type filter, a bitmask from
       * PluginAPI::PluginAPI::Segment values.
       * Determines which segments this cursor will move to
       * on next() and nextMeasure() operations. The default
       * value is Ms::SegmentType::ChordRest so only segments
       * containing chords and rests are handled by default.
       */
      Q_PROPERTY(int filter     READ filter    WRITE setFilter)

      /** MIDI tick position, read only */
      Q_PROPERTY(int tick         READ tick) // FIXME: fraction transition
      /** Time at tick position, read only */
      Q_PROPERTY(double time      READ time)

      /** Tempo at current tick, read only */
      Q_PROPERTY(qreal tempo      READ tempo)

      /** Key signature of current staff at tick pos. (read only) */
      Q_PROPERTY(int keySignature READ qmlKeySignature)
      /** Associated score */
      Q_PROPERTY(Ms::PluginAPI::Score* score READ score    WRITE setScore)

      /** Current element at track, read only */
      Q_PROPERTY(Ms::PluginAPI::Element* element READ element)
      /** Current segment, read only */
      Q_PROPERTY(Ms::PluginAPI::Segment*  segment READ qmlSegment)
      /** Current measure, read only */
      Q_PROPERTY(Ms::PluginAPI::Measure*  measure READ measure)
      /**
       * A physical string number where this cursor currently at. This is useful
       * in conjunction with \ref InputStateMode.INPUT_STATE_SYNC_WITH_SCORE
       * cursor mode.
       * \since MuseScore 3.5
       */
      Q_PROPERTY(int stringNumber READ inputStateString WRITE setInputStateString)

   public:
      enum RewindMode {
            SCORE_START = 0, ///< Rewind to the start of a score
            SELECTION_START = 1, ///< Rewind to the start of a selection
            SELECTION_END = 2 ///< Rewind to the end of a selection
            };
      Q_ENUM(RewindMode);

      /** \since MuseScore 3.5 */
      enum InputStateMode {
            INPUT_STATE_INDEPENDENT, ///< Input state of cursor is independent of score input state (default)
            INPUT_STATE_SYNC_WITH_SCORE ///< Input state of cursor is synchronized with score input state
            };
      Q_ENUM(InputStateMode);

   private:
      /**
       * Behavior of input state (position, notes duration etc.) of this cursor
       * with respect to input state of the score. By default any changes in
       * score and in this Cursor are not synchronized.
       * \since MuseScore 3.5
       */
      Q_PROPERTY(InputStateMode inputStateMode READ inputStateMode WRITE setInputStateMode)

      Ms::Score* _score = nullptr;
//       bool _expandRepeats; // used?
      SegmentType _filter;
      std::unique_ptr<InputState> is;
      InputStateMode _inputStateMode = INPUT_STATE_INDEPENDENT;

      // utility methods
      void prevInTrack();
      void nextInTrack();
      void setScore(Ms::Score* s);
      Ms::Element* currentElement() const;

      InputState& inputState();
      const InputState& inputState() const { return const_cast<Cursor*>(this)->inputState(); }

      Ms::Segment* segment() const;
      void setSegment(Ms::Segment* seg);

      int inputStateString() const;
      void setInputStateString(int);

   public:
      /// \cond MS_INTERNAL
      Cursor(Ms::Score* s = nullptr);
//       Cursor(Score*, bool); // not implemented? what is bool?

      Score* score() const;
      void setScore(Score* s);

      int track() const;
      void setTrack(int v);

      int staffIdx() const;
      void setStaffIdx(int v);

      int voice() const;
      void setVoice(int v);

      int filter() const            { return int(_filter); }
      void setFilter(int f)         { _filter = SegmentType(f); }

      InputStateMode inputStateMode() const { return _inputStateMode; }
      void setInputStateMode(InputStateMode val);

      Element* element() const;
      Segment* qmlSegment() const;
      Measure* measure() const;

      int tick();
      double time();
      qreal tempo();

      int qmlKeySignature();
      /// \endcond

      Q_INVOKABLE void rewind(RewindMode mode);
      Q_INVOKABLE void rewindToTick(int tick);

      Q_INVOKABLE bool next();
      Q_INVOKABLE bool nextMeasure();
      Q_INVOKABLE bool prev();
      Q_INVOKABLE void add(Ms::PluginAPI::Element*);

      Q_INVOKABLE void addNote(int pitch, bool addToChord = false);
      Q_INVOKABLE void addRest();
      Q_INVOKABLE void addTuplet(Ms::PluginAPI::FractionWrapper* ratio, Ms::PluginAPI::FractionWrapper* duration);

      //@ set duration
      //@   z: numerator
      //@   n: denominator
      //@   Quarter, if n == 0
      Q_INVOKABLE void setDuration(int z, int n);
      };

}     // namespace PluginAPI
}     // namespace Ms
#endif

