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
#include "barline.h"
#include "beam.h"
#include "chord.h"
#include "figuredbass.h"
#include "harmony.h"
#include "input.h"
#include "limits.h"
#include "lyrics.h"
#include "measure.h"
#include "note.h"
#include "notedot.h"
#include "page.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sig.h"
#include "slur.h"
#include "stem.h"
#include "tie.h"
#include "system.h"
#include "text.h"
#include "textline.h"
#include "tuplet.h"
#include "utils.h"
#include "xml.h"
#include "staff.h"
#include "part.h"

namespace Ms {

//---------------------------------------------------------
//   Selection
//---------------------------------------------------------

Selection::Selection(Score* s)
      {
      _score         = s;
      _state         = SelState::NONE;
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
      if ((_state != SelState::RANGE) || !_activeSegment)
            return 0;
      if (_activeSegment == _startSegment)
            return firstChordRest(_activeTrack);
      else
            return lastChordRest(_activeTrack);
      }

Segment* Selection::firstChordRestSegment() const
      {
      if (!isRange()) return 0;

      for (Segment* s = _startSegment; s && (s != _endSegment); s = s->next1MM()) {
            if (s->segmentType() == Segment::Type::ChordRest)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   firstChordRest
//---------------------------------------------------------

ChordRest* Selection::firstChordRest(int track) const
      {
      if (_el.size() == 1) {
            Element* el = _el[0];
            if (el->type() == Element::Type::NOTE)
                  return static_cast<ChordRest*>(el->parent());
            else if (el->type() == Element::Type::REST)
                  return static_cast<ChordRest*>(el);
            return 0;
            }
      ChordRest* cr = 0;
      foreach (Element* el, _el) {
            if (el->type() == Element::Type::NOTE)
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
            if (el && el->type() == Element::Type::NOTE)
                  return static_cast<ChordRest*>(el->parent());
            else if (el->type() == Element::Type::CHORD || el->type() == Element::Type::REST)
                  return static_cast<ChordRest*>(el);
            return 0;
            }
      ChordRest* cr = 0;
      for (auto i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == Element::Type::NOTE)
                  el = ((Note*)el)->chord();
            if (el->isChordRest() && static_cast<ChordRest*>(el)->segment()->segmentType() == Segment::Type::ChordRest) {
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
      if (_state == SelState::RANGE)
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
      setState(SelState::NONE);
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
            qDebug("updateSelectedElements: bad staff selection %d - %d, staves %d", _staffStart, _staffEnd, staves);
            _staffStart = 0;
            _staffEnd   = 0;
            }
      int startTrack = _staffStart * VOICES;
      int endTrack   = _staffEnd * VOICES;

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* s = _startSegment; s && (s != _endSegment); s = s->next1MM()) {
                  if (s->segmentType() == Segment::Type::EndBarLine)  // do not select end bar line
                        continue;
                  Element* e = s->element(st);
                  if (!e)
                        continue;
                  if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        for (Element* e : cr->lyricsList()) {
                              if (e)
                                    _el.append(e);
                              }
                        }
                  if (e->type() == Element::Type::CHORD) {
                        Chord* chord = static_cast<Chord*>(e);
                        foreach (Articulation* art, chord->articulations()) {
                              _el.append(art);
                              }
                        if (chord->beam()) _el.append(chord->beam());
                        if (chord->stem()) _el.append(chord->stem());
                        foreach(Note* note, chord->notes()) {
                              _el.append(note);
                              if (note->accidental()) _el.append(note->accidental());

                              for(int x = 0; x < MAX_DOTS; x++) {
                                    if (note->dot(x) != 0) _el.append(note->dot(x));
                                                                        }
                                    if (note->tieFor() && (note->tieFor()->endElement() != 0)) {
                                          if (note->tieFor()->endElement()->type() == Element::Type::NOTE) {
                                          Note* endNote = static_cast<Note*>(note->tieFor()->endElement());
                                          Segment* s = endNote->chord()->segment();
                                          if (_endSegment && (s->tick() < _endSegment->tick()))
                                                _el.append(note->tieFor());
                                          }
                                    }
                              }
                        }
                  else {
                        _el.append(e);
                        }
                  foreach(Element* e, s->annotations()) {
                        if (e->track() < startTrack || e->track() >= endTrack)
                              continue;
                        if (e->systemFlag()) //exclude system text
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
                              qDebug("1spanner element type %s", sp->endElement()->name());
                              }
                        }
#endif
                  }

#if 0 // TODO-S
            // for each measure in the selection, check if it contains spanners within our selection
            Measure* sm = _startSegment->measure();
            Measure* em = _endSegment ? _endSegment->measure()->nextMeasure() : 0;
            // int endTick = _endSegment ? _endSegment->tick() : score()->lastMeasure()->endTick();
            for (Measure* m = sm; m && m != em; m = m->nextMeasure()) {

                  for(Spanner* sp = m->spannerFor(); sp; sp = sp->next()) {
                        // ignore spanners belonging to other tracks
                        if (sp->track() < startTrack || sp->track() >= endTrack)
                              continue;
                        // if spanner ends between _startSegment and _endSegment, select it
                        if (sp->endElement()->type() == Element::Type::SEGMENT) {
                              Segment* s2 = static_cast<Segment*>(sp->endElement());
                              if (s2->tick() >= _startSegment->tick() && s2->tick() < endTick)
                                    _el.append(sp);
                              }
                        else if (sp->endElement()->type() == Element::Type::MEASURE) {
                              Measure* s2 = static_cast<Measure*>(sp->endElement());
                              if (s2->tick() >= _startSegment->tick() && s2->tick() < endTick)
                                    _el.append(sp);
                              }
                        else {
                              qDebug("2spanner element type %s", sp->endElement()->name());
                              }
                        }

                  }
#endif
            }
      int stick = startSegment()->tick();
      int etick = tickEnd();

      for (auto i = score()->spanner().begin(); i != score()->spanner().end(); ++i) {
            Spanner* sp = (*i).second;
            // ignore spanners belonging to other tracks
            if (sp->track() < startTrack || sp->track() >= endTrack)
                  continue;
            if (sp->type() == Element::Type::SLUR
                && ((sp->tick() >= stick && sp->tick() < etick) || (sp->tick2() >= stick && sp->tick2() < etick)))
                  _el.append(sp); // slur with start or end in range selection
            else if((sp->tick() >= stick && sp->tick() < etick) && (sp->tick2() >= stick && sp->tick2() < etick))
                  _el.append(sp); // spanner with start and end in range selection
            }
      update();
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void Selection::setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd)
      {
      Q_ASSERT(staffEnd > staffStart && staffStart >= 0 && staffEnd >= 0 && staffEnd <= _score->nstaves());

      _startSegment  = startSegment;
      _endSegment    = endSegment;
      _activeSegment = endSegment;
      _staffStart    = staffStart;
      _staffEnd      = staffEnd;
      setState(SelState::RANGE);
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
            case SelState::NONE:   qDebug("NONE"); return;
            case SelState::RANGE:  qDebug("RANGE"); break;
            case SelState::LIST:   qDebug("LIST"); break;
            }
      foreach(const Element* e, _el)
            qDebug("  %p %s", e, e->name());
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
            setState(SelState::NONE);
      else if (_state == SelState::NONE)
            setState(SelState::LIST);
      if (!_score->noteEntryMode())
             _score->inputState().update(e);
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
            case SelState::NONE:
                  return QString();
            case SelState::LIST:
                  return isSingle() ? mimeSymbolFormat : mimeSymbolListFormat;
            case SelState::RANGE:
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
            case SelState::LIST:
                  if (isSingle()) {
                        Element* e = element();
                        if (e->type() == Element::Type::TEXTLINE_SEGMENT)
                              e = static_cast<TextLineSegment*>(e)->textLine();
                        a = e->mimeData(QPointF());
                        }
                  else
                        a = symbolListMimeData();
                  break;
            case SelState::NONE:
                  break;
            case SelState::RANGE:
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
            Staff* staff = score()->staff(staffIdx);
            Part* part = staff->part();
            Interval interval = part->instr(seg1->tick())->transpose();
            if (interval.chromatic)
                  xml.tag("transposeChromatic", interval.chromatic);
            if (interval.diatonic)
                  xml.tag("transposeDiatonic", interval.diatonic);
            score()->writeSegments(xml, startTrack, endTrack, seg1, seg2, false, true, true);
            xml.etag();
            }

      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   symbolListMimeData
//---------------------------------------------------------

QByteArray Selection::symbolListMimeData() const
      {

      struct MAPDATA {
            Element* e;
            Segment* s;
            };

      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();
      xml.clipboardmode = true;

      int         topTrack    = 1000000;
      int         bottomTrack = 0;
      Segment*    firstSeg    = nullptr;
      int         firstTick   = 0x7FFFFFFF;
      MAPDATA     mapData;
      Segment*    seg         = nullptr;
      int         track;
      std::multimap<qint64, MAPDATA> map;

      // scan selection element list, inserting relevant elements in a tick-sorted map
      foreach (Element* e, _el) {
            switch (e->type()) {
/* All these element types are ignored:

Enabling copying of more element types requires enabling pasting in Score::pasteSymbols() in libmscore/paste.cpp

                  case Element::Type::SYMBOL:
                  case Element::Type::TEXT:
                  case Element::Type::INSTRUMENT_NAME:
                  case Element::Type::SLUR_SEGMENT:
                  case Element::Type::STAFF_LINES:
                  case Element::Type::BAR_LINE:
                  case Element::Type::STEM_SLASH:
                  case Element::Type::LINE:
                  case Element::Type::BRACKET:
                  case Element::Type::ARPEGGIO:
                  case Element::Type::ACCIDENTAL:
                  case Element::Type::STEM:
                  case Element::Type::NOTE:
                  case Element::Type::CLEF:
                  case Element::Type::KEYSIG:
                  case Element::Type::TIMESIG:
                  case Element::Type::REST:
                  case Element::Type::BREATH:
                  case Element::Type::GLISSANDO:
                  case Element::Type::REPEAT_MEASURE:
                  case Element::Type::IMAGE:
                  case Element::Type::TIE:
                  case Element::Type::CHORDLINE:
                  case Element::Type::DYNAMIC:
                  case Element::Type::BEAM:
                  case Element::Type::HOOK:
                  case Element::Type::MARKER:
                  case Element::Type::JUMP:
                  case Element::Type::FINGERING:
                  case Element::Type::TUPLET:
                  case Element::Type::TEMPO_TEXT:
                  case Element::Type::STAFF_TEXT:
                  case Element::Type::REHEARSAL_MARK:
                  case Element::Type::INSTRUMENT_CHANGE:
                  case Element::Type::FRET_DIAGRAM:
                  case Element::Type::BEND:
                  case Element::Type::TREMOLOBAR:
                  case Element::Type::VOLTA:
                  case Element::Type::HAIRPIN_SEGMENT:
                  case Element::Type::OTTAVA_SEGMENT:
                  case Element::Type::TRILL_SEGMENT:
                  case Element::Type::TEXTLINE_SEGMENT:
                  case Element::Type::VOLTA_SEGMENT:
                  case Element::Type::PEDAL_SEGMENT:
                  case Element::Type::LAYOUT_BREAK:
                  case Element::Type::SPACER:
                  case Element::Type::STAFF_STATE:
                  case Element::Type::LEDGER_LINE:
                  case Element::Type::NOTEHEAD:
                  case Element::Type::NOTEDOT:
                  case Element::Type::TREMOLO:
                  case Element::Type::MEASURE:
                  case Element::Type::SELECTION:
                  case Element::Type::LASSO:
                  case Element::Type::SHADOW_NOTE:
                  case Element::Type::RUBBERBAND:
                  case Element::Type::TAB_DURATION_SYMBOL:
                  case Element::Type::FSYMBOL:
                  case Element::Type::PAGE:
                  case Element::Type::HAIRPIN:
                  case Element::Type::OTTAVA:
                  case Element::Type::PEDAL:
                  case Element::Type::TRILL:
                  case Element::Type::TEXTLINE:
                  case Element::Type::NOTELINE:
                  case Element::Type::SEGMENT:
                  case Element::Type::SYSTEM:
                  case Element::Type::COMPOUND:
                  case Element::Type::CHORD:
                  case Element::Type::SLUR:
                  case Element::Type::ELEMENT:
                  case Element::Type::ELEMENT_LIST:
                  case Element::Type::STAFF_LIST:
                  case Element::Type::MEASURE_LIST:
                  case Element::Type::LAYOUT:
                  case Element::Type::HBOX:
                  case Element::Type::VBOX:
                  case Element::Type::TBOX:
                  case Element::Type::FBOX:
                  case Element::Type::ACCIDENTAL_BRACKET:
                  case Element::Type::ICON:
                  case Element::Type::OSSIA:
                  case Element::Type::BAGPIPE_EMBELLISHMENT:
                        continue;
*/
                  case Element::Type::ARTICULATION:
                        // ignore articulations not attached to chords/rest
                        if (e->parent()->type() == Element::Type::CHORD) {
                              Chord* par = static_cast<Chord*>( (static_cast<Articulation*>(e))->parent() );
                              seg = par->segment();
                              break;
                              }
                        else if (e->parent()->type() == Element::Type::REST) {
                              Rest* par = static_cast<Rest*>( (static_cast<Articulation*>(e))->parent() );
                              seg = par->segment();
                              break;
                              }
                        continue;
                  case Element::Type::FIGURED_BASS:
                        seg = (static_cast<FiguredBass*>(e))->segment();
                        break;
                  case Element::Type::HARMONY:
                        // ignore chord sybols not attached to segment
                        if (e->parent()->type() == Element::Type::SEGMENT) {
                              seg = static_cast<Segment*>( (static_cast<Harmony*>(e))->parent() );
                              break;
                              }
                        continue;
                  case Element::Type::LYRICS:
                        seg = (static_cast<Lyrics*>(e))->segment();
                        break;
                  default:
                        continue;
                  }
            track = e->track();
            if (track < topTrack)
                  topTrack = track;
            if (track > bottomTrack)
                  bottomTrack = track;
            if (seg->tick() < firstTick) {
                  firstSeg  = seg;
                  firstTick = seg->tick();
                  }
            mapData.e = e;
            mapData.s = seg;
            map.insert(std::pair<qint64,MAPDATA>( ((qint64)track << 32) + seg->tick(), mapData));
            }

      xml.stag(QString("SymbolList version=\"" MSC_VERSION "\" fromtrack=\"%1\" totrack=\"%2\"")
                  .arg(topTrack).arg(bottomTrack));
      // scan the map, outputting elements each with a relative <track> tag on track change,
      // a relative tick and the number of CR segments to skip
      int   currTrack = -1;
      for (auto iter = map.cbegin(); iter != map.cend(); ++iter) {
            int   numSegs;
            int   track = (int)(iter->first >> 32);
            if (currTrack != track) {
                  xml.tag("trackOffset", track - topTrack);
                  currTrack = track;
                  seg       = firstSeg;
                  }
            xml.tag("tickOffset", (int)(iter->first & 0xFFFFFFFF) - firstTick);
            numSegs = 0;
            // with figured bass, we need to look for the proper segment
            // not only according to ChordRest elements, but also annotations
            if (iter->second.e->type() == Element::Type::FIGURED_BASS) {
                  bool done = false;
                  for ( ; seg; seg = seg->next1()) {
                        if (seg->segmentType() == Segment::Type::ChordRest) {
                              // if no ChordRest in right track, look in anotations
                              if (seg->element(currTrack) == nullptr) {
                                    foreach (Element* el, seg->annotations()) {
                                          // do annotations include our element?
                                          if (el == iter->second.e) {
                                                done = true;
                                                break;
                                                }
                                          // do annotations include any f.b.?
                                          if (el->type() == Element::Type::FIGURED_BASS && el->track() == track) {
                                                numSegs++;  //yes: it counts as a step
                                                break;
                                                }
                                          }
                                    if (done)
                                          break;
                                    continue;               // segment is not relevant: no ChordRest nor f.b.
                                    }
                              else {
                                    if (iter->second.s == seg)
                                          break;
                                    }
                              numSegs++;
                              }
                        }
                  }
            else {
                  while (seg && iter->second.s != seg) {
                              seg = seg->nextCR(currTrack);
                              numSegs++;
                              }
                  }
            xml.tag("segDelta", numSegs);
            iter->second.e->write(xml);
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

      if (_state == SelState::LIST) {
            foreach(Element* e, _el) {
                  if (e->type() == Element::Type::NOTE)
                        nl.append(static_cast<Note*>(e));
                  }
            }
      else if (_state == SelState::RANGE) {
            for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (Segment* seg = _startSegment; seg && seg != _endSegment; seg = seg->next1()) {
                        if (!(seg->segmentType() & (Segment::Type::ChordRest)))
                              continue;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* e = seg->element(track);
                              if (e == 0 || e->type() != Element::Type::CHORD
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
      if (_state != SelState::RANGE)
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
      if (!isRange())
            return false;
      *m1 = startSegment()->measure();
      *m2 = endSegment()->measure();
      if (m1 == m2)
            return true;
      if (*m2 && (*m2)->tick() == endSegment()->tick())
            *m2 = (*m2)->prevMeasure();
      return true;
      }

//---------------------------------------------------------
//   uniqueElements
//    Return list of selected elements.
//    If some elements are linked, only one of the linked
//    elements show up in the list.
//---------------------------------------------------------

const QList<Element*> Selection::uniqueElements() const
      {
      QList<Element*> l;

      for (Element* e : elements()) {
            bool alreadyThere = false;
            for (Element* ee : l) {
                  if ((ee->links() && ee->links()->contains(e)) || e == ee) {
                        alreadyThere = true;
                        break;
                        }
                  }
            if (!alreadyThere)
                  l.append(e);
            }
      return l;
      }

//---------------------------------------------------------
//   uniqueNotes
//    Return list of selected notes.
//    If some notes are linked, only one of the linked
//    elements show up in the list.
//---------------------------------------------------------

QList<Note*> Selection::uniqueNotes(int track) const
      {
      QList<Note*> l;

      for (Note* note : noteList(track)) {
            while (note->tieBack())
                  note = note->tieBack()->startNote();
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  bool alreadyThere = false;
                  for (Note* n : l) {
                        if ((n->links() && n->links()->contains(note)) || n == note) {
                              alreadyThere = true;
                              break;
                              }
                        }
                  if (!alreadyThere)
                        l.append(note);
                  }
            }
      return l;
      }

//---------------------------------------------------------
//   extendRangeSelection
//    Extends the range selection to contain the given
//    chord rest.
//---------------------------------------------------------

void Selection::extendRangeSelection(ChordRest* cr)
      {
            extendRangeSelection(cr->segment(),cr->segment()->nextCR(cr->track()),cr->staffIdx(),cr->tick(),cr->tick());
      }

//---------------------------------------------------------
//   extendRangeSelection
//    Extends the range selection to contain the given
//    segment. SegAfter should represent the segment
//    that is after seg. Tick and etick represent
//    the start and end tick of an element. Useful when
//    extending by a chord rest.
//---------------------------------------------------------

void Selection::extendRangeSelection(Segment* seg, Segment* segAfter, int staffIdx, int tick, int etick)
      {
      bool activeIsFirst = false;

      if (staffIdx < _staffStart)
            _staffStart = staffIdx;
      else if (staffIdx >= _staffEnd)
            _staffEnd = staffIdx + 1;

      if (tick < tickStart()) {
            _startSegment = seg;
            activeIsFirst = true;
            }
      else if (etick >= tickEnd()) {
            _endSegment = segAfter;
            }
      else {
            if (_activeSegment == _startSegment) {
                  _startSegment = seg;
                  activeIsFirst = true;
                  }
            else {
                  _endSegment = segAfter;
                  }
            }
      activeIsFirst ? _activeSegment = _startSegment : _activeSegment = _endSegment;
      }


}

