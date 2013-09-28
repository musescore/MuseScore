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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "mscore.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "score.h"
#include "slur.h"
#include "system.h"
#include "select.h"
#include "sig.h"
#include "utils.h"
#include "text.h"
#include "segment.h"
#include "input.h"
#include "measure.h"
#include "page.h"
#include "barline.h"
#include "xml.h"
#include "lyrics.h"
#include "limits.h"
#include "tuplet.h"
#include "beam.h"
#include "textline.h"

namespace Ms {

//---------------------------------------------------------
//   Selection
//---------------------------------------------------------

Selection::Selection(Score* s)
      {
      _score         = s;
      _state         = SEL_NONE;
      _startSegment  = 0;
      _endSegment    = 0;
      _activeSegment = 0;
      _staffStart     = 0;
      _staffEnd       = 0;
      _activeTrack    = 0;
      }

//---------------------------------------------------------
//   tickStart
//---------------------------------------------------------

int Selection::tickStart() const
      {
      return _startSegment->tick();
      }

//---------------------------------------------------------
//   tickEnd
//---------------------------------------------------------

int Selection::tickEnd() const
      {
      if (_endSegment) {
            return _endSegment->tick();
            }
      else{ // endsegment == 0 if end of score
          Measure* m = _score->lastMeasure();
          return m->tick() + m->ticks();
          }
      }

//---------------------------------------------------------
//   isStartActive
//---------------------------------------------------------

bool Selection::isStartActive() const
      {
      return activeSegment() && activeSegment()->tick() == tickStart();
      }

//---------------------------------------------------------
//   isEndActive
//---------------------------------------------------------

bool Selection::isEndActive() const {
      return activeSegment() && activeSegment()->tick() == tickEnd();
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Selection::element() const
      {
      return _el.size() == 1 ? _el[0] : 0;
      }

//---------------------------------------------------------
//   activeCR
//---------------------------------------------------------

ChordRest* Selection::activeCR() const
      {
      if ((_state != SEL_RANGE) || !_activeSegment)
            return 0;
      if (_activeSegment == _startSegment)
            return firstChordRest(_activeTrack);
      else
            return lastChordRest(_activeTrack);
      }

//---------------------------------------------------------
//   firstChordRest
//---------------------------------------------------------

ChordRest* Selection::firstChordRest(int track) const
      {
      if (_el.size() == 1) {
            Element* el = _el[0];
            if (el->type() == Element::NOTE)
                  return static_cast<ChordRest*>(el->parent());
            else if (el->type() == Element::REST)
                  return static_cast<ChordRest*>(el);
            return 0;
            }
      ChordRest* cr = 0;
      foreach (Element* el, _el) {
            if (el->type() == Element::NOTE)
                  el = el->parent();
            if (el->isChordRest()) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (static_cast<ChordRest*>(el)->tick() < cr->tick())
                              cr = static_cast<ChordRest*>(el);
                        }
                  else
                        cr = static_cast<ChordRest*>(el);
                  }
            }
      return cr;
      }

//---------------------------------------------------------
//   lastChordRest
//---------------------------------------------------------

ChordRest* Selection::lastChordRest(int track) const
      {
      if (_el.size() == 1) {
            Element* el = _el[0];
            if (el && el->type() == Element::NOTE)
                  return static_cast<ChordRest*>(el->parent());
            else if (el->type() == Element::CHORD || el->type() == Element::REST)
                  return static_cast<ChordRest*>(el);
            return 0;
            }
      ChordRest* cr = 0;
      for (auto i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == Element::NOTE)
                  el = ((Note*)el)->chord();
            if (el->isChordRest() && static_cast<ChordRest*>(el)->segment()->segmentType() == Segment::SegChordRest) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (((ChordRest*)el)->tick() >= cr->tick())
                              cr = (ChordRest*)el;
                        }
                  else
                        cr = (ChordRest*)el;
                  }
            }
      return cr;
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

Measure* Selection::findMeasure() const
      {
      Measure *m = 0;
      if (_el.size() > 0) {
            Element* el = _el[0];
            m = static_cast<Measure*>(el->findMeasure());
            }
      return m;
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

void Selection::deselectAll()
      {
      if (_state == SEL_RANGE)
            _score->setUpdateAll();
      clear();
      updateState();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Selection::clear()
      {
      foreach(Element* e, _el) {
            _score->addRefresh(e->canvasBoundingRect());
            e->setSelected(false);
            _score->addRefresh(e->canvasBoundingRect());
            }
      _el.clear();
      _startSegment  = 0;
      _endSegment    = 0;
      _activeSegment = 0;
      _staffStart     = 0;
      _staffEnd       = 0;
      _activeTrack    = 0;
      setState(SEL_NONE);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Selection::remove(Element* el)
      {
      _el.removeOne(el);
      el->setSelected(false);
      updateState();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Selection::add(Element* el)
      {
      _el.append(el);
      update();
      }

//---------------------------------------------------------
//   updateSelectedElements
//---------------------------------------------------------

void Selection::updateSelectedElements()
      {
      foreach(Element* e, _el)
            e->setSelected(false);
      _el.clear();

      // assert:
      int staves = _score->nstaves();
      if (_staffStart < 0 || _staffStart >= staves || _staffEnd < 0 || _staffEnd > staves
         || _staffStart >= _staffEnd) {
            qDebug("updateSelectedElements: bad staff selection %d - %d, staves %d\n", _staffStart, _staffEnd, staves);
            _staffStart = 0;
            _staffEnd   = 0;
            }
      int startTrack = _staffStart * VOICES;
      int endTrack   = _staffEnd * VOICES;

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* s = _startSegment; s && (s != _endSegment); s = s->next1MM()) {
                  if (s->segmentType() == Segment::SegEndBarLine)  // do not select end bar line
                        continue;
                  Element* e = s->element(st);
                  if (!e)
                        continue;
                  if (e->type() == Element::CHORD) {
                        Chord* chord = static_cast<Chord*>(e);
                        foreach(Note* note, chord->notes())
                              _el.append(note);
                        }
                  else {
                        _el.append(e);
                        }
                  foreach(Element* e, s->annotations()) {
                        if (e->track() < startTrack || e->track() >= endTrack)
                              continue;
                        _el.append(e);
                        }
#if 0 // TODO-S
                  for(Spanner* sp = s->spannerFor(); sp; sp = sp->next()) {
                        if (sp->track() < startTrack || sp->track() >= endTrack)
                              continue;
                        if (sp->endElement()->type() == Element::SEGMENT) {
                              Segment* s2 = static_cast<Segment*>(sp->endElement());
                              if (_endSegment && (s2->tick() < _endSegment->tick()))
                                    _el.append(sp);
                              }
                        else {
                              qDebug("1spanner element type %s\n", sp->endElement()->name());
                              }
                        }
#endif
                  }
            // for each measure in the selection, check if it contains spanners within our selection
            Measure* sm = _startSegment->measure();
            Measure* em = _endSegment ? _endSegment->measure()->nextMeasure() : 0;
            // int endTick = _endSegment ? _endSegment->tick() : score()->lastMeasure()->endTick();
            for (Measure* m = sm; m && m != em; m = m->nextMeasure()) {
#if 0 // TODO-S
                  for(Spanner* sp = m->spannerFor(); sp; sp = sp->next()) {
                        // ignore spanners belonging to other tracks
                        if (sp->track() < startTrack || sp->track() >= endTrack)
                              continue;
                        // if spanner ends between _startSegment and _endSegment, select it
                        if (sp->endElement()->type() == Element::SEGMENT) {
                              Segment* s2 = static_cast<Segment*>(sp->endElement());
                              if (s2->tick() >= _startSegment->tick() && s2->tick() < endTick)
                                    _el.append(sp);
                              }
                        else if (sp->endElement()->type() == Element::MEASURE) {
                              Measure* s2 = static_cast<Measure*>(sp->endElement());
                              if (s2->tick() >= _startSegment->tick() && s2->tick() < endTick)
                                    _el.append(sp);
                              }
                        else {
                              qDebug("2spanner element type %s\n", sp->endElement()->name());
                              }
                        }
#endif
                  }
            }
      update();
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void Selection::setRange(Segment* a, Segment* b, int c, int d)
      {
      Q_ASSERT(d > c && c >= 0 && d >= 0 && d <= _score->nstaves());

      _startSegment  = a;
      _endSegment    = b;
      _activeSegment = b;
      _staffStart    = c;
      _staffEnd      = d;
      setState(SEL_RANGE);
      }

//---------------------------------------------------------
//   update
///   Set select flag for all Elements in select list.
//---------------------------------------------------------

void Selection::update()
      {
      foreach (Element* e, _el)
            e->setSelected(true);
      updateState();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Selection::dump()
      {
      qDebug("Selection dump: ");
      switch(_state) {
            case SEL_NONE:   qDebug("NONE\n"); return;
            case SEL_RANGE:  qDebug("RANGE\n"); break;
            case SEL_LIST:   qDebug("LIST\n"); break;
            }
      foreach(const Element* e, _el)
            qDebug("  %p %s\n", e, e->name());
      }

//---------------------------------------------------------
//   updateState
///   update selection and input state
//---------------------------------------------------------

void Selection::updateState()
      {
      int n = _el.size();
      Element* e = element();
      if (n == 0)
            setState(SEL_NONE);
      else if (_state == SEL_NONE)
            setState(SEL_LIST);
      if (!_score->noteEntryMode())
             _score->setInputState(e);
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Selection::setState(SelState s)
      {
//      if (_state != s) {
            _state = s;
            _score->setSelectionChanged(true);
//            }
      }

//---------------------------------------------------------
//   mimeType
//---------------------------------------------------------

QString Selection::mimeType() const
      {
      switch (_state) {
            default:
            case SEL_NONE:
                  return QString();
            case SEL_LIST:
                  return isSingle() ? mimeSymbolFormat : mimeSymbolListFormat;
            case SEL_RANGE:
                  return mimeStaffListFormat;
            }
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Selection::mimeData() const
      {
      QByteArray a;
      switch (_state) {
            case SEL_LIST:
                  if (isSingle()) {
                        Element* e = element();
                        if (e->type() == Element::TEXTLINE_SEGMENT)
                              e = static_cast<TextLineSegment*>(e)->textLine();
                        a = e->mimeData(QPointF());
                        }
                  break;
            case SEL_NONE:
                  break;
            case SEL_RANGE:
                  a = staffMimeData();
                  break;
            }
      return a;
      }

//---------------------------------------------------------
//   staffMimeData
//---------------------------------------------------------

QByteArray Selection::staffMimeData() const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();
      xml.clipboardmode = true;


      int ticks  = tickEnd() - tickStart();
      int staves = staffEnd() - staffStart();
      xml.stag(QString("StaffList version=\"" MSC_VERSION "\" tick=\"%1\" len=\"%2\" staff=\"%3\" staves=\"%4\"").arg(tickStart()).arg(ticks).arg(staffStart()).arg(staves));
      Segment* seg1 = _startSegment;
      Segment* seg2 = _endSegment;

      for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx));
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            score()->writeSegments(xml, 0, startTrack, endTrack, seg1, seg2, false, true, true);
            xml.etag();
            }

      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   noteList
//---------------------------------------------------------

QList<Note*> Selection::noteList(int selTrack) const
      {
      QList<Note*>nl;

      if (_state == SEL_LIST) {
            foreach(Element* e, _el) {
                  if (e->type() == Element::NOTE)
                        nl.append(static_cast<Note*>(e));
                  }
            }
      else if (_state == SEL_RANGE) {
            for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (Segment* seg = _startSegment; seg && seg != _endSegment; seg = seg->next1()) {
                        if (!(seg->segmentType() & (Segment::SegChordRest)))
                              continue;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* e = seg->element(track);
                              if (e == 0 || e->type() != Element::CHORD
                                 || (selTrack != -1 && selTrack != track))
                                    continue;
                              Chord* c = static_cast<Chord*>(e);
                              nl.append(c->notes());
                              for (Chord* g : c->graceNotes()) {
                                    nl.append(g->notes());
                                    }
                              }
                        }
                  }
            }
      return nl;
      }

//---------------------------------------------------------
//   checkStart
//     return false if element is NOT a tuplet or is start of a tuplet
//     return true  if element is part of a tuplet, but not the start
//---------------------------------------------------------

static bool checkStart(Element* e)
      {
      if (e == 0 || !e->isChordRest())
            return false;
      ChordRest* cr = static_cast<ChordRest*>(e);
      if (!cr->tuplet())
            return false;
      Tuplet* tuplet = cr->tuplet();
      while (tuplet) {
            if (tuplet->elements().front() == e)
                  return false;
            tuplet = tuplet->tuplet();
            }
      return true;
      }

//---------------------------------------------------------
//   checkEnd
//     return false if element is NOT a tuplet or is end of a tuplet
//     return true  if element is part of a tuplet, but not the end
//---------------------------------------------------------

static bool checkEnd(Element* e)
      {
      if (e == 0 || !e->isChordRest())
            return false;
      ChordRest* cr = static_cast<ChordRest*>(e);
      if (!cr->tuplet())
            return false;
      Tuplet* tuplet = cr->tuplet();
      while (tuplet) {
            if (tuplet->elements().back() == e)
                  return false;
            tuplet = tuplet->tuplet();
            }
      return true;
      }

//---------------------------------------------------------
//   canCopy
//    return false if range selection intersects a tuplet
//---------------------------------------------------------

bool Selection::canCopy() const
      {
      if (_state != SEL_RANGE)
            return true;

      for (int staffIdx = _staffStart; staffIdx != _staffEnd; ++staffIdx)
            for (int voice = 0; voice < VOICES; ++voice) {
                  int track = staffIdx * VOICES + voice;
                  if (checkStart(_startSegment->element(track)))
                        return false;

                  if (! _endSegment)
                        continue;

                  // find last segment in the selection.
                  // Note that _endSegment is the first segment after the selection
                  Segment *endSegmentSelection = _startSegment;
                  while (endSegmentSelection->nextCR(track) &&
                        (endSegmentSelection->nextCR(track)->tick() < _endSegment->tick()))
                        endSegmentSelection = endSegmentSelection->nextCR(track);

                  if (checkEnd(endSegmentSelection->element(track)))
                        return false;
                  }
      return true;
      }

//---------------------------------------------------------
//   measureRange
//    return false if no measure range selected
//---------------------------------------------------------

bool Selection::measureRange(Measure** m1, Measure** m2) const
      {
      if (state() != SEL_RANGE)
            return false;
      *m1 = startSegment()->measure();
      *m2 = endSegment()->measure();
      if (m1 == m2)
            return true;
      if (*m2 && (*m2)->tick() == endSegment()->tick())
            *m2 = (*m2)->prevMeasure();
      return true;
      }


}

