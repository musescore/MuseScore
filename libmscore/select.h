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

//---------------------------------------------------------
//   ElementPattern
//---------------------------------------------------------

struct ElementPattern {
      QList<Element*> el;
      int type;
      int subtype;
      int staff;
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

   public:
      Selection()                      { _score = 0; _state = SelState::NONE; }
      Selection(Score*);
      Score* score() const             { return _score; }
      SelState state() const           { return _state; }
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
      void setRange(Segment* a, Segment* b, int c, int d);
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
      };


}     // namespace Ms
#endif

