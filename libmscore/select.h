//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: select.h 5582 2012-04-27 19:16:19Z wschweer $
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

enum SelState {
      SEL_NONE,   // nothing is selected
      SEL_LIST,   // disjoint selection
      SEL_RANGE,  // adjacent selection, a range in one or more staves
                  // is selected
      };

//-------------------------------------------------------------------
//   Selection
//    For SEL_LIST state only visible elements can be selected
//    (no Chord element etc.).
//-------------------------------------------------------------------

class Selection {
      Score* _score;
      SelState _state;
      QList<Element*> _el;          // valid in mode SEL_LIST

      int _staffStart;              // valid if selState is SEL_RANGE
      int _staffEnd;
      Segment* _startSegment;
      Segment* _endSegment;         // next segment after selection

      Segment* _activeSegment;
      int _activeTrack;

      QByteArray staffMimeData() const;

   public:
      Selection()                      { _score = 0; _state = SEL_NONE; }
      Selection(Score*);
      Score* score() const             { return _score; }
      SelState state() const           { return _state; }
      void setState(SelState s);

      void searchSelectedElements();
      const QList<Element*>& elements() const { return _el; }
      bool isSingle() const                   { return (_state == SEL_LIST) && (_el.size() == 1); }
      QList<Note*> noteList(int track = -1) const;
      void add(Element*);
      void deselectAll();
      void remove(Element*);
      void clear();
      Element* element() const;
      ChordRest* firstChordRest(int track = -1) const;
      ChordRest* lastChordRest(int track = -1) const;
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
      void reconstructElementList();
      void updateSelectedElements();
      bool measureRange(Measure** m1, Measure** m2) const;
      };

#endif

