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
      const System* system;
      bool subtypeValid;
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

enum class SelectionFilterType {
      NONE                    = 0,
      FIRST_VOICE             = 1 << 0,
      SECOND_VOICE            = 1 << 1,
      THIRD_VOICE             = 1 << 2,
      FOURTH_VOICE            = 1 << 3,
      DYNAMIC                 = 1 << 4,
      FINGERING               = 1 << 5,
      LYRICS                  = 1 << 6,
      CHORD_SYMBOL            = 1 << 7,
      OTHER_TEXT              = 1 << 8,
      ARTICULATION            = 1 << 9,
      SLUR                    = 1 << 10,
      FIGURED_BASS            = 1 << 11,
      OTTAVA                  = 1 << 12,
      PEDAL_LINE              = 1 << 13,
      OTHER_LINE              = 1 << 14,
      ARPEGGIO                = 1 << 15,
      GLISSANDO               = 1 << 16,
      FRET_DIAGRAM            = 1 << 17,
      BREATH                  = 1 << 18,
      TREMOLO                 = 1 << 19,
      GRACE_NOTE              = 1 << 20,
      ALL                     = -1
      };

class SelectionFilter {
      Score* _score;
      int _filtered;

public:
      SelectionFilter()                      { _score = 0; _filtered = (int)SelectionFilterType::ALL;}
      SelectionFilter(Score* score)          { _score = score; _filtered = (int)SelectionFilterType::ALL;}
      int& filtered()                        { return _filtered; }
      void setFiltered(SelectionFilterType type, bool set);
      bool isFiltered(SelectionFilterType type) const        { return _filtered & (int)type; }
      bool canSelect(const Element*) const;
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

      Segment* _activeSegment;
      int _activeTrack;

      QByteArray staffMimeData() const;
      QByteArray symbolListMimeData() const;
      SelectionFilter selectionFilter() const;
      bool canSelect(Element* e) { return selectionFilter().canSelect(e); }
      void appendFiltered(Element* e);
      void appendChord(Chord* chord);

   public:
      Selection()                      { _score = 0; _state = SelState::NONE; }
      Selection(Score*);
      Score* score() const             { return _score; }
      SelState state() const           { return _state; }
      bool isNone() const              { return _state == SelState::NONE; }
      bool isRange() const             { return _state == SelState::RANGE; }
      bool isList() const              { return _state == SelState::LIST; }
      void setState(SelState s);

      const QList<Element*>& elements() const { return _el; }
      QList<Note*> noteList(int track = -1) const;

      const QList<Element*> uniqueElements() const;
      QList<Note*> uniqueNotes(int track = -1) const;

      bool isSingle() const                   { return (_state == SelState::LIST) && (_el.size() == 1); }

      void add(Element*);
      void deselectAll();
      void remove(Element*);
      void clear();
      Element* element() const;
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
      Segment* activeSegment() const    { return _activeSegment; }
      void setActiveSegment(Segment* s) { _activeSegment = s; }
      ChordRest* activeCR() const;
      bool isStartActive() const;
      bool isEndActive() const;
      int tickStart() const;
      int tickEnd() const;
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
      void extendRangeSelection(Segment* seg, Segment* segAfter, int staffIdx, int tick, int etick);
      };


}     // namespace Ms
#endif

