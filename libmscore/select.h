//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SELECT_H__
#define __SELECT_H__

#include "pitchspelling.h"
#include "mscore.h"
#include "durationtype.h"

namespace Ms {

class Score;
class Page;
class System;
class ChordRest;
class Element;
class Segment;
class Note;
class Measure;
class Chord;
class Tuplet;

//---------------------------------------------------------
//   ElementPattern
//---------------------------------------------------------

struct ElementPattern {
      QList<Element*> el;
      int type;
      int subtype;
      int staffStart;
      int staffEnd; // exclusive
      int voice;
      bool subtypeValid;
      Fraction durationTicks {-1, 1};
      Fraction beat {0, 0};
      const Measure* measure = nullptr;
      const System* system = nullptr;
      };

//---------------------------------------------------------
//   NotePattern
//---------------------------------------------------------

struct NotePattern : ElementPattern {
      QList<Note*> el;
      int pitch = -1;
      int string = INVALID_STRING_INDEX;
      int tpc = Tpc::TPC_INVALID;
      NoteHead::Group notehead = NoteHead::Group::HEAD_INVALID;
      TDuration durationType = TDuration();
      NoteType type = NoteType::INVALID;
      };

//---------------------------------------------------------
//   SelState
//---------------------------------------------------------

enum class SelState : char {
      NONE,   // nothing is selected
      LIST,   // disjoint selection
      RANGE,  // adjacent selection, a range in one or more staves
                  // is selected
      };

//---------------------------------------------------------
//   SelectionFilterType
//   see also `static const char* labels[]` in mscore/selectionwindow.cpp
//   need to keep those in sync!
//---------------------------------------------------------

enum class SelectionFilterType {
      NONE                    = 0,
      FIRST_VOICE             = 1 << 0,
      SECOND_VOICE            = 1 << 1,
      THIRD_VOICE             = 1 << 2,
      FOURTH_VOICE            = 1 << 3,
      DYNAMIC                 = 1 << 4,
      HAIRPIN                 = 1 << 5,
      FINGERING               = 1 << 6,
      LYRICS                  = 1 << 7,
      CHORD_SYMBOL            = 1 << 8,
      OTHER_TEXT              = 1 << 9,
      ARTICULATION            = 1 << 10,
      ORNAMENT                = 1 << 11,
      SLUR                    = 1 << 12,
      FIGURED_BASS            = 1 << 13,
      OTTAVA                  = 1 << 14,
      PEDAL_LINE              = 1 << 15,
      OTHER_LINE              = 1 << 16,
      ARPEGGIO                = 1 << 17,
      GLISSANDO               = 1 << 18,
      FRET_DIAGRAM            = 1 << 19,
      BREATH                  = 1 << 20,
      TREMOLO                 = 1 << 21,
      GRACE_NOTE              = 1 << 22,
      ALL                     = -1
      };


//---------------------------------------------------------
//   SelectionFilter
//---------------------------------------------------------

class SelectionFilter {
      Score* _score;
      int _filtered;

public:
      SelectionFilter()                      { _score = 0; _filtered = (int)SelectionFilterType::ALL;}
      SelectionFilter(SelectionFilterType f) : _score(nullptr), _filtered(int(f)) {}
      SelectionFilter(Score* score)          { _score = score; _filtered = (int)SelectionFilterType::ALL;}
      int& filtered()                        { return _filtered; }
      void setFiltered(SelectionFilterType type, bool set);
      bool isFiltered(SelectionFilterType type) const        { return _filtered & (int)type; }
      bool canSelect(const Element*) const;
      bool canSelectVoice(int track) const;
      };

//-------------------------------------------------------------------
//   Selection
//    For SelState::LIST state only visible elements can be selected
//    (no Chord element etc.).
//-------------------------------------------------------------------

class Selection {
      Score* _score;
      SelState _state;
      QList<Element*> _el;          // valid in mode SelState::LIST

      int _staffStart;              // valid if selState is SelState::RANGE
      int _staffEnd;
      Segment* _startSegment;
      Segment* _endSegment;         // next segment after selection

      Fraction _plannedTick1 { -1, 1 }; // Will be actually selected on updateSelectedElements() call.
      Fraction _plannedTick2 { -1, 1 }; // Used by setRangeTicks() to restore proper selection after
                              // command end in case some changes are expected to segments'
                              // structure (e.g. MMRests reconstruction).

      Segment* _activeSegment;
      int _activeTrack;

      Fraction _currentTick;  // tracks the most recent selection
      int _currentTrack;

      QString _lockReason;

      QByteArray staffMimeData() const;
      QByteArray symbolListMimeData() const;
      SelectionFilter selectionFilter() const;
      bool canSelect(Element* e) const { return selectionFilter().canSelect(e); }
      bool canSelectVoice(int track) const { return selectionFilter().canSelectVoice(track); }
      void appendFiltered(Element* e);
      void appendChord(Chord* chord);
      void appendTupletHierarchy(Tuplet* innermostTuplet);

   public:
      Selection()                      { _score = 0; _state = SelState::NONE; }
      Selection(Score*);
      Score* score() const             { return _score; }
      SelState state() const           { return _state; }
      bool isNone() const              { return _state == SelState::NONE; }
      bool isRange() const             { return _state == SelState::RANGE; }
      bool isList() const              { return _state == SelState::LIST; }
      void setState(SelState s);

      //! NOTE If locked, the selected items should not be changed.
      void lock(const QString& reason)    { _lockReason = reason; }
      void unlock(const QString& reason)  { Q_UNUSED(reason); _lockReason.clear(); } // reason for clarity
      bool isLocked() const               { return  !_lockReason.isEmpty(); }
      const QString& lockReason() const   { return _lockReason; }

      const QList<Element*>& elements() const { return _el; }
      std::vector<Note*> noteList(int track = -1) const;

      const std::list<Element*> uniqueElements() const;
      std::list<Note*> uniqueNotes(int track = -1) const;

      bool isSingle() const                   { return (_state == SelState::LIST) && (_el.size() == 1); }

      void add(Element*);
      void deselectAll();
      void remove(Element*);
      void clear();
      Element* element() const;
      ChordRest* cr() const;
      Segment* firstChordRestSegment() const;
      ChordRest* firstChordRest(int track = -1) const;
      ChordRest* lastChordRest(int track = -1) const;
      Measure* findMeasure() const;
      void update();
      void updateState();
      void dump();
      QString mimeType() const;
      QByteArray mimeData() const;

      Segment* startSegment() const     { return _startSegment; }
      Segment* endSegment() const       { return _endSegment;   }
      void setStartSegment(Segment* s)  { _startSegment = s; }
      void setEndSegment(Segment* s)    { _endSegment = s; }
      void setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd);
      void setRangeTicks(const Fraction& tick1, const Fraction& tick2, int staffStart, int staffEnd);
      Segment* activeSegment() const    { return _activeSegment; }
      void setActiveSegment(Segment* s) { _activeSegment = s; }
      ChordRest* activeCR() const;
      bool isStartActive() const;
      bool isEndActive() const;
      ChordRest* currentCR() const;
      Fraction tickStart() const;
      Fraction tickEnd() const;
      int staffStart() const            { return _staffStart;  }
      int staffEnd() const              { return _staffEnd;    }
      int activeTrack() const           { return _activeTrack; }
      void setStaffStart(int v)         { _staffStart = v;  }
      void setStaffEnd(int v)           { _staffEnd = v;    }
      void setActiveTrack(int v)        { _activeTrack = v; }
      bool canCopy() const;
      void updateSelectedElements();
      bool measureRange(Measure** m1, Measure** m2) const;
      void extendRangeSelection(ChordRest* cr);
      void extendRangeSelection(Segment* seg, Segment* segAfter, int staffIdx, const Fraction& tick, const Fraction& etick);
      };


}     // namespace Ms
#endif

