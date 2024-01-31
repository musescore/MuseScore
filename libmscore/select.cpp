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

#include "global/log.h"

#include "mscore.h"
#include "arpeggio.h"
#include "barline.h"
#include "beam.h"
#include "chord.h"
#include "dynamic.h"
#include "element.h"
#include "figuredbass.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "fret.h"
#include "hook.h"
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
#include "stemslash.h"
#include "tie.h"
#include "system.h"
#include "text.h"
#include "tremolo.h"
#include "tuplet.h"
#include "utils.h"
#include "xml.h"
#include "staff.h"
#include "part.h"
#include "accidental.h"
#include "articulation.h"
#include "stafftext.h"
#include "sticking.h"

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
      _staffStart    = 0;
      _staffEnd      = 0;
      _activeTrack   = 0;
      _currentTick   = Fraction(-1, 1);
      _currentTrack  = 0;
      }

//---------------------------------------------------------
//   tickStart
//---------------------------------------------------------

Fraction Selection::tickStart() const
      {
      switch (_state) {
            case SelState::RANGE:
                  return _startSegment ? _startSegment->tick() : Fraction(-1,1);
            case SelState::LIST: {
                  ChordRest* cr = firstChordRest();
                  return (cr) ? cr->tick() : Fraction(-1,1);
                  }
            default:
                  return Fraction(-1,1);
            }
      }

//---------------------------------------------------------
//   tickEnd
//---------------------------------------------------------

Fraction Selection::tickEnd() const
      {
      switch (_state) {
            case SelState::RANGE: {
                  if (_endSegment)
                        return _endSegment->tick();
                  else { // endsegment == 0 if end of score
                      Measure* m = _score->lastMeasure();
                      return m->endTick();
                      }
                  break;
                  }
            case SelState::LIST: {
                  ChordRest* cr = lastChordRest();
                  return (cr) ? cr->segment()->tick() : Fraction(-1,1);
                  break;
                  }
            default:
                  return Fraction(-1,1);
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

bool Selection::isEndActive() const
      {
      return activeSegment() && activeSegment()->tick() == tickEnd();
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Selection::element() const
      {
      return ((state() != SelState::RANGE) && (_el.size() == 1)) ? _el[0] : 0;
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* Selection::cr() const
      {
      Element* e = element();
      if (!e)
            return 0;
      if (e->isNote())
            e = e->parent();
      if (e->isChordRest())
            return toChordRest(e);
      return 0;
      }

//---------------------------------------------------------
//   currentCR
//---------------------------------------------------------

ChordRest* Selection::currentCR() const
      {
      // no selection yet - start at very beginning, not first cr
      if (_currentTick == Fraction(-1, 1))
            return nullptr;
      Segment* s = score()->tick2rightSegment(_currentTick, true);
      if (!s)
            return nullptr;
      int track = _currentTrack;
      // staff may have been removed - start at top
      if (track < 0 || track >= score()->ntracks())
            track = 0;
      Element* e = s->element(track);
      if (e && e->isChordRest())
            return toChordRest(e);
      else
            return nullptr;
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
      if (!isRange())
            return 0;

      for (Segment* s = _startSegment; s && (s != _endSegment); s = s->next1MM()) {
            if (!s->enabled())
                  continue;
            if (s->isChordRestType())
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
            if (el->isNote())
                  return toChordRest(el->parent());
            else if (el->isRest())
                  return toChordRest(el);
            return 0;
            }
      ChordRest* cr = 0;
      for (Element* el : _el) {
            if (el->isNote())
                  el = el->parent();
            if (el->isChordRest()) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (toChordRest(el)->tick() < cr->tick())
                              cr = toChordRest(el);
                        }
                  else
                        cr = toChordRest(el);
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
            if (el) {
                  if (el->isNote())
                        return toChordRest(el->parent());
                  else if (el->isChord() || el->isRest() || el->isRepeatMeasure())
                        return toChordRest(el);
                  }
            return nullptr;
            }
      ChordRest* cr = nullptr;
      for (auto el : _el) {
            if (el->isNote())
                  el = toNote(el)->chord();
            if (el->isChordRest() && toChordRest(el)->segment()->isChordRestType()) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (toChordRest(el)->tick() >= cr->tick())
                              cr = toChordRest(el);
                        }
                  else
                        cr = toChordRest(el);
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
            m = toMeasure(el->findMeasure());
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
//   changeSelection
//---------------------------------------------------------

static QRectF changeSelection(Element* e, bool b)
      {
      QRectF r = e->canvasBoundingRect();
      e->setSelected(b);
      r |= e->canvasBoundingRect();
      return r;
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Selection::clear()
      {

      IF_ASSERT_FAILED(!isLocked()) {
            LOGE() << "selection locked, reason: " << lockReason();
            return;
            }

      for (Element* e : qAsConst(_el)) {
            if (e->isSpanner()) {   // TODO: only visible elements should be selectable?
                  Spanner* sp = toSpanner(e);
                  for (auto s : sp->spannerSegments())
                        e->score()->addRefresh(changeSelection(s, false));
                  }
            else
                  e->score()->addRefresh(changeSelection(e, false));
            }
      _el.clear();
      _startSegment  = 0;
      _endSegment    = 0;
      _activeSegment = 0;
      _staffStart    = 0;
      _staffEnd      = 0;
      _activeTrack   = 0;
      setState(SelState::NONE);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Selection::remove(Element* el)
      {
      const bool removed = _el.removeOne(el);
      el->setSelected(false);
      if (removed)
            updateState();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Selection::add(Element* el)
      {
      IF_ASSERT_FAILED(!isLocked()) {
            LOGE() << "selection locked, reason: " << lockReason();
            return;
            }
      _el.append(el);
      update();
      }

//---------------------------------------------------------
//   canSelect
//   see also `static const char* labels[]` in selectionwindow.cpp
//---------------------------------------------------------

bool SelectionFilter::canSelect(const Element* e) const
      {
      if (e->isDynamic())
            return isFiltered(SelectionFilterType::DYNAMIC);
      if (e->isHairpin())
            return isFiltered(SelectionFilterType::HAIRPIN);
      if ((e->isArticulation() && !toArticulation(e)->isOrnament()) || e->isVibrato() || e->isFermata())
            return isFiltered(SelectionFilterType::ARTICULATION);
      if ((e->isArticulation() && toArticulation(e)->isOrnament()) || e->isTrill())
            return isFiltered(SelectionFilterType::ORNAMENT);
      if (e->type() == ElementType::LYRICS)
            return isFiltered(SelectionFilterType::LYRICS);
      if (e->type() == ElementType::FINGERING)
            return isFiltered(SelectionFilterType::FINGERING);
      if (e->type() == ElementType::HARMONY)
            return isFiltered(SelectionFilterType::CHORD_SYMBOL);
      if (e->type() == ElementType::SLUR)
            return isFiltered(SelectionFilterType::SLUR);
      if (e->type() == ElementType::FIGURED_BASS)
            return isFiltered(SelectionFilterType::FIGURED_BASS);
      if (e->type() == ElementType::OTTAVA)
            return isFiltered(SelectionFilterType::OTTAVA);
      if (e->type() == ElementType::PEDAL)
            return isFiltered(SelectionFilterType::PEDAL_LINE);
      if (e->type() == ElementType::ARPEGGIO)
            return isFiltered(SelectionFilterType::ARPEGGIO);
      if (e->type() == ElementType::GLISSANDO)
            return isFiltered(SelectionFilterType::GLISSANDO);
      if (e->type() == ElementType::FRET_DIAGRAM)
            return isFiltered(SelectionFilterType::FRET_DIAGRAM);
      if (e->type() == ElementType::BREATH)
            return isFiltered(SelectionFilterType::BREATH);
      if (e->isTextBase()) // only TEXT, INSTRCHANGE and STAFFTEXT are caught here, rest are system thus not in selection
            return isFiltered(SelectionFilterType::OTHER_TEXT);
      if (e->isSLine()) // NoteLine, Volta
            return isFiltered(SelectionFilterType::OTHER_LINE);
      if (e->isTremolo())
            return isFiltered(SelectionFilterType::TREMOLO);
      if (e->isChord() && toChord(e)->isGrace())
            return isFiltered(SelectionFilterType::GRACE_NOTE);
      return true;
      }

//---------------------------------------------------------
//   canSelectVoice
//---------------------------------------------------------

bool SelectionFilter::canSelectVoice(int track) const
      {
      int voice = track % VOICES;
      switch (voice) {
            case 0:
                  return isFiltered(SelectionFilterType::FIRST_VOICE);
            case 1:
                  return isFiltered(SelectionFilterType::SECOND_VOICE);
            case 2:
                  return isFiltered(SelectionFilterType::THIRD_VOICE);
            case 3:
                  return isFiltered(SelectionFilterType::FOURTH_VOICE);
            }
      return true;
      }

//---------------------------------------------------------
//   appendFiltered
//---------------------------------------------------------

void Selection::appendFiltered(Element* e)
      {
      IF_ASSERT_FAILED(!isLocked()) {
            LOGE() << "selection locked, reason: " << lockReason();
            return;
            }
      if (selectionFilter().canSelect(e))
            _el.append(e);
      }

//---------------------------------------------------------
//   appendChord
//---------------------------------------------------------

void Selection::appendChord(Chord* chord)
      {
      IF_ASSERT_FAILED(!isLocked()) {
            LOGE() << "selection locked, reason: " << lockReason();
            return;
            }
      if (chord->beam() && !_el.contains(chord->beam()))
            _el.append(chord->beam());
      if (chord->stem())
            _el.append(chord->stem());
      if (chord->hook())
            _el.append(chord->hook());
      if (chord->arpeggio())
            appendFiltered(chord->arpeggio());
      if (chord->stemSlash())
            _el.append(chord->stemSlash());
      if (chord->tremolo())
            appendFiltered(chord->tremolo());
      for (Note* note : chord->notes()) {
            _el.append(note);
            if (note->accidental()) _el.append(note->accidental());
            for (Element* el : note->el())
                  appendFiltered(el);
            for (NoteDot* dot : note->dots())
                  _el.append(dot);

            if (note->tieFor() && (note->tieFor()->endElement() != 0)) {
                  if (note->tieFor()->endElement()->isNote()) {
                        Note* endNote = toNote(note->tieFor()->endElement());
                        Segment* s = endNote->chord()->segment();
                        if (!s || s->tick() < tickEnd())
                              _el.append(note->tieFor());
                        }
                  }
            for (Spanner* sp : note->spannerFor()) {
                  if (sp->endElement()->isNote()) {
                        Note* endNote = toNote(sp->endElement());
                        Segment* s = endNote->chord()->segment();
                        if (!s || s->tick() < tickEnd())
                              _el.append(sp);
                        }
                  }
            }
      }

void Selection::appendTupletHierarchy(Tuplet* innermostTuplet)
      {
      if (_el.contains(innermostTuplet))
            return;

      appendFiltered(innermostTuplet);

      // Recursively append upwards/outwards
      Tuplet* outerTuplet = innermostTuplet->tuplet();
      if (outerTuplet && !_el.contains(outerTuplet->tuplet()))
            appendTupletHierarchy(outerTuplet);
      }

//---------------------------------------------------------
//   updateSelectedElements
//---------------------------------------------------------

void Selection::updateSelectedElements()
      {
      IF_ASSERT_FAILED(!isLocked()) {
            LOGE() << "selection locked, reason: " << lockReason();
            return;
            }
      if (_state != SelState::RANGE) {
            update();
            return;
            }
      if (_state == SelState::RANGE && _plannedTick1 != Fraction(-1,1) && _plannedTick2 != Fraction(-1,1)) {
            const int staffStart = _staffStart;
            const int staffEnd = _staffEnd;
            deselectAll();
            Segment* s1 = _score->tick2segmentMM(_plannedTick1);
            Segment* s2 = _score->tick2segmentMM(_plannedTick2, /* first */ true);
            if (s2 && s2->measure()->isMMRest())
                  s2 = s2->prev1MM(); // HACK both this and the previous "true"
                                      // are needed to prevent bug #173381.
                                      // This should exclude any segments belonging
                                      // to MM-rest range from the selection.
            if (s1 && s2 && s1->tick() + s1->ticks() > s2->tick()) {
                  // can happen with MM rests as tick2measure returns only
                  // the first segment for them.
                  return;
                  }
            if (s2 && s2 == s2->measure()->first())
                  s2 = s2->prev1();   // we want the last segment of the previous measure
            setRange(s1, s2, staffStart, staffEnd);
            _plannedTick1 = Fraction(-1,1);
            _plannedTick2 = Fraction(-1,1);
            }

      for (Element* e : qAsConst(_el))
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
            if (!canSelectVoice(st))
                  continue;
            for (Segment* s = _startSegment; s && (s != _endSegment); s = s->next1MM()) {
                  if (!s->enabled() || s->isEndBarLineType())  // do not select end bar line
                        continue;
                  for (Element* e : s->annotations()) {
                        if (e->track() != st)
                              continue;
                        if (e->isFretDiagram()) {
                              FretDiagram* fd = toFretDiagram(e);
                              if (Harmony* harm = fd->harmony())
                                    appendFiltered(harm);
                              }
                        appendFiltered(e);
                        }
                  Element* e = s->element(st);
                  if (!e || e->generated() || e->isTimeSig() || e->isKeySig())
                        continue;
                  if (e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        for (Element* el : cr->lyrics()) {
                              if (el)
                                    appendFiltered(el);
                              }
                        Tuplet* tuplet = cr->tuplet();
                        if (tuplet)
                              appendTupletHierarchy(tuplet);
                        }
                  if (e->isChord()) {
                        Chord* chord = toChord(e);
                        for (Chord* graceNote : chord->graceNotes())
                              if (canSelect(graceNote)) appendChord(graceNote);
                        appendChord(chord);
                        for (Articulation* art : chord->articulations())
                              appendFiltered(art);
                        }
                  else {
                        appendFiltered(e);
                        if (e->isRest()) {
                              Rest* r = toRest(e);
                              for (int i = 0; i < r->dots(); ++i)
                                    appendFiltered(r->dot(i));
                              }
                        }
                  }
            }
      Fraction stick = startSegment()->tick();
      Fraction etick = tickEnd();

      for (auto i = _score->spanner().begin(); i != _score->spanner().end(); ++i) {
            Spanner* sp = (*i).second;
            // ignore spanners belonging to other tracks
            if (sp->track() < startTrack || sp->track() >= endTrack)
                  continue;
            if (!canSelectVoice(sp->track()))
                  continue;
            // ignore voltas
            if (sp->isVolta())
                  continue;
            if (sp->isSlur()) {
                  // ignore if start & end elements not calculated yet
                  if (!sp->startElement() || !sp->endElement())
                        continue;
                  if ((sp->tick() >= stick && sp->tick() < etick) || (sp->tick2() >= stick && sp->tick2() < etick))
                        if (canSelect(sp->startCR()) && canSelect(sp->endCR()))
                              appendFiltered(sp);     // slur with start or end in range selection
            }
            else if ((sp->tick() >= stick && sp->tick() < etick) && (sp->tick2() >= stick && sp->tick2() <= etick))
                  appendFiltered(sp); // spanner with start and end in range selection
            }
      update();
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void Selection::setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd)
      {
      Q_ASSERT(staffEnd > staffStart && staffStart >= 0 && staffEnd >= 0 && staffEnd <= _score->nstaves());
      Q_ASSERT(!(endSegment && !startSegment));

      _startSegment  = startSegment;
      _endSegment    = endSegment;
      _activeSegment = endSegment;
      _staffStart    = staffStart;
      _staffEnd      = staffEnd;
      setState(SelState::RANGE);
      }

//---------------------------------------------------------
//   setRangeTicks
//    sets the range to be selected on next
//    updateSelectedElements() call. Can be used if some
//    segment structure changes are expected (e.g. if
//    creating MM rests is pending).
//---------------------------------------------------------

void Selection::setRangeTicks(const Fraction& tick1, const Fraction& tick2, int staffStart, int staffEnd)
      {
      Q_ASSERT(staffEnd > staffStart && staffStart >= 0 && staffEnd >= 0 && staffEnd <= _score->nstaves());

      deselectAll();
      _plannedTick1 = tick1;
      _plannedTick2 = tick2;
      _startSegment = _endSegment = _activeSegment = nullptr;
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
      for (Element* e : qAsConst(_el))
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
      for (const Element* e : _el)
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
      if (e) {
            if (e->isSpannerSegment())
                  _currentTick = toSpannerSegment(e)->spanner()->tick();
            else
                  _currentTick = e->tick();
            // ignore system elements (e.g., frames)
            if (e->track() >= 0)
                  _currentTrack = e->track();
            }
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Selection::setState(SelState s)
      {
      _state = s;
      _score->setSelectionChanged(true);
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
                  if (isSingle())
                        a = element()->mimeData(QPointF());
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
//   hasElementInTrack
//---------------------------------------------------------

bool hasElementInTrack(Segment* startSeg, Segment* endSeg, int track)
      {
      for (Segment* seg = startSeg; seg != endSeg; seg = seg->next1MM()) {
            if (!seg->enabled())
                  continue;
            if (seg->element(track))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   firstElementInTrack
//---------------------------------------------------------

static Fraction firstElementInTrack(Segment* startSeg, Segment* endSeg, int track)
      {
      for (Segment* seg = startSeg; seg != endSeg; seg = seg->next1MM()) {
            if (!seg->enabled())
                  continue;
            if (seg->element(track))
                  return seg->tick();
            }
      return Fraction(-1,1);
      }

//---------------------------------------------------------
//   staffMimeData
//---------------------------------------------------------

QByteArray Selection::staffMimeData() const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      XmlWriter xml(score(), &buffer);
      xml.header();
      xml.setClipboardmode(true);
      xml.setFilter(selectionFilter());

      Fraction ticks  = tickEnd() - tickStart();
      int staves = staffEnd() - staffStart();
      if (!MScore::testMode) {
            xml.stag(QString("StaffList version=\"" MSC_VERSION "\" tick=\"%1\" len=\"%2\" staff=\"%3\" staves=\"%4\"").arg(tickStart().toString()).arg(ticks.toString()).arg(staffStart()).arg(staves));
            }
      else {
            xml.stag(QString("StaffList version=\"2.00\" tick=\"%1\" len=\"%2\" staff=\"%3\" staves=\"%4\"").arg(tickStart().toString()).arg(ticks.toString()).arg(staffStart()).arg(staves));
            }
      Segment* seg1 = _startSegment;
      Segment* seg2 = _endSegment;

      for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;

            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx));

            Staff* staff = _score->staff(staffIdx);
            Part* part = staff->part();
            Interval interval = part->instrument(seg1->tick())->transpose();
            if (interval.chromatic)
                  xml.tag("transposeChromatic", interval.chromatic);
            if (interval.diatonic)
                  xml.tag("transposeDiatonic", interval.diatonic);
            xml.stag("voiceOffset");
            for (int voice = 0; voice < VOICES; voice++) {
                  if (hasElementInTrack(seg1, seg2, startTrack + voice)
                     && xml.canWriteVoice(voice)) {
                        Fraction offset = firstElementInTrack(seg1, seg2, startTrack+voice) - tickStart();
                        xml.tag(QString("voice id=\"%1\"").arg(voice), offset.ticks());
                        }
                  }
            xml.etag(); // </voiceOffset>
            xml.setCurTrack(startTrack);
            _score->writeSegments(xml, startTrack, endTrack, seg1, seg2, false, false);
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
      struct MapData {
            Element* e;
            Segment* s;
            };

      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      XmlWriter xml(score(), &buffer);
      xml.header();
      xml.setClipboardmode(true);

      int         topTrack    = 1000000;
      int         bottomTrack = 0;
      Segment*    firstSeg    = 0;
      Fraction    firstTick   = Fraction(0x7FFFFFFF,1);
      MapData     mapData;
      Segment*    seg         = 0;
      std::multimap<qint64, MapData> map;

      // scan selection element list, inserting relevant elements in a tick-sorted map
      for (Element* e : _el) {
            switch (e->type()) {
/* All these element types are ignored:

Enabling copying of more element types requires enabling pasting in Score::pasteSymbols() in libmscore/paste.cpp

                  case ElementType::SYMBOL:
                  case ElementType::TEXT:
                  case ElementType::INSTRUMENT_NAME:
                  case ElementType::SLUR_SEGMENT:
                  case ElementType::TIE_SEGMENT:
                  case ElementType::STAFF_LINES:
                  case ElementType::BAR_LINE:
                  case ElementType::STEM_SLASH:
                  case ElementType::LINE:
                  case ElementType::BRACKET:
                  case ElementType::ARPEGGIO:
                  case ElementType::ACCIDENTAL:
                  case ElementType::STEM:
                  case ElementType::NOTE:
                  case ElementType::CLEF:
                  case ElementType::KEYSIG:
                  case ElementType::TIMESIG:
                  case ElementType::REST:
                  case ElementType::BREATH:
                  case ElementType::GLISSANDO:
                  case ElementType::REPEAT_MEASURE:
                  case ElementType::IMAGE:
                  case ElementType::TIE:
                  case ElementType::CHORDLINE:
                  case ElementType::BEAM:
                  case ElementType::HOOK:
                  case ElementType::MARKER:
                  case ElementType::JUMP:
                  case ElementType::FINGERING:
                  case ElementType::TUPLET:
                  case ElementType::TEMPO_TEXT:
                  case ElementType::STAFF_TEXT:
                  case ElementType::SYSTEM_TEXT:
                  case ElementType::REHEARSAL_MARK:
                  case ElementType::INSTRUMENT_CHANGE:
                  case ElementType::BEND:
                  case ElementType::TREMOLOBAR:
                  case ElementType::VOLTA:
                  case ElementType::OTTAVA_SEGMENT:
                  case ElementType::TRILL_SEGMENT:
                  case ElementType::VIBRATO_SEGMENT:
                  case ElementType::TEXTLINE_SEGMENT:
                  case ElementType::VOLTA_SEGMENT:
                  case ElementType::PEDAL_SEGMENT:
                  case ElementType::LAYOUT_BREAK:
                  case ElementType::SPACER:
                  case ElementType::STAFF_STATE:
                  case ElementType::LEDGER_LINE:
                  case ElementType::NOTEHEAD:
                  case ElementType::NOTEDOT:
                  case ElementType::TREMOLO:
                  case ElementType::MEASURE:
                  case ElementType::SELECTION:
                  case ElementType::LASSO:
                  case ElementType::SHADOW_NOTE:
                  case ElementType::RUBBERBAND:
                  case ElementType::TAB_DURATION_SYMBOL:
                  case ElementType::FSYMBOL:
                  case ElementType::PAGE:
                  case ElementType::OTTAVA:
                  case ElementType::PEDAL:
                  case ElementType::TRILL:
                  case ElementType::TEXTLINE:
                  case ElementType::NOTELINE:
                  case ElementType::SEGMENT:
                  case ElementType::SYSTEM:
                  case ElementType::COMPOUND:
                  case ElementType::CHORD:
                  case ElementType::SLUR:
                  case ElementType::ELEMENT:
                  case ElementType::ELEMENT_LIST:
                  case ElementType::STAFF_LIST:
                  case ElementType::MEASURE_LIST:
                  case ElementType::LAYOUT:
                  case ElementType::HBOX:
                  case ElementType::VBOX:
                  case ElementType::TBOX:
                  case ElementType::FBOX:
                  case ElementType::ICON:
                  case ElementType::OSSIA:
                  case ElementType::BAGPIPE_EMBELLISHMENT:
                        continue;
*/
                  case ElementType::ARTICULATION:
                        // ignore articulations not attached to chords/rest
                        if (e->parent()->isChord()) {
                              Chord* par = toChord(e->parent());
                              seg = par->segment();
                              break;
                              }
                        else if (e->parent()->isRest()) {
                              Rest* par = toRest(e->parent());
                              seg = par->segment();
                              break;
                              }
                        continue;
                  case ElementType::STAFF_TEXT:
                        seg = toStaffText(e)->segment();
                        break;
                  case ElementType::STICKING:
                        seg = toSticking(e)->segment();
                        break;
                  case ElementType::FIGURED_BASS:
                        seg = toFiguredBass(e)->segment();
                        break;
                  case ElementType::HARMONY:
                  case ElementType::FRET_DIAGRAM:
                        // ignore chord symbols or fret diagrams not attached to segment
                        if (e->parent()->isSegment()) {
                              seg = toSegment(e->parent());
                              break;
                              }
                        continue;
                  case ElementType::LYRICS:
                        seg = toLyrics(e)->segment();
                        break;
                  case ElementType::DYNAMIC:
                        seg = toDynamic(e)->segment();
                        break;
                  case ElementType::HAIRPIN_SEGMENT:
                        e = toHairpinSegment(e)->hairpin();
                        // fall through
                  case ElementType::HAIRPIN:
                        seg = toHairpin(e)->startSegment();
                        break;
                  default:
                        continue;
                  }
            int track = e->track();
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
            map.insert(std::pair<qint64,MapData>( ((qint64)track << 32) + seg->tick().ticks(), mapData));
            }

      xml.stag(QString("SymbolList version=\"" MSC_VERSION "\" fromtrack=\"%1\" totrack=\"%2\"")
                  .arg(topTrack).arg(bottomTrack));
      // scan the map, outputting elements each with a relative <track> tag on track change,
      // a relative tick and the number of CR segments to skip
      int   currTrack = -1;
      for (auto iter = map.cbegin(); iter != map.cend(); ++iter) {
            int   numSegs;
            int   track = static_cast<int>(iter->first >> 32);
            if (currTrack != track) {
                  xml.tag("trackOffset", track - topTrack);
                  currTrack = track;
                  seg       = firstSeg;
                  }
            xml.tag("tickOffset", static_cast<int>(iter->first & 0xFFFFFFFF) - firstTick.ticks());
            numSegs = 0;
            // with figured bass, we need to look for the proper segment
            // not only according to ChordRest elements, but also annotations
            if (iter->second.e->type() == ElementType::FIGURED_BASS) {
                  bool done = false;
                  for ( ; seg; seg = seg->next1()) {
                        if (seg->isChordRestType()) {
                              // if no ChordRest in right track, look in anotations
                              if (seg->element(currTrack) == nullptr) {
                                    for (Element* el : seg->annotations()) {
                                          // do annotations include our element?
                                          if (el == iter->second.e) {
                                                done = true;
                                                break;
                                                }
                                          // do annotations include any f.b.?
                                          if (el->type() == ElementType::FIGURED_BASS && el->track() == track) {
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

std::vector<Note*> Selection::noteList(int selTrack) const
      {
      std::vector<Note*>nl;

      if (_state == SelState::LIST) {
            for (Element* e : _el) {
                  if (e->isNote())
                        nl.push_back(toNote(e));
                  }
            }
      else if (_state == SelState::RANGE) {
            for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (Segment* seg = _startSegment; seg && seg != _endSegment; seg = seg->next1()) {
                        if (!(seg->segmentType() & (SegmentType::ChordRest)))
                              continue;
                        for (int track = startTrack; track < endTrack; ++track) {
                              if (!canSelectVoice(track))
                                  continue;
                              Element* e = seg->element(track);
                              if (e == 0 || e->type() != ElementType::CHORD
                                 || (selTrack != -1 && selTrack != track))
                                    continue;
                              Chord* c = toChord(e);
                              nl.insert(nl.end(), c->notes().begin(), c->notes().end());
                              for (Chord* g : c->graceNotes()) {
                                    nl.insert(nl.end(), g->notes().begin(), g->notes().end());
                                    }
                              }
                        }
                  }
            }
      return nl;
      }

//---------------------------------------------------------
//   checkStart
//     return false if element is NOT a tuplet or is start of a tuplet/tremolo
//     return true  if element is part of a tuplet/tremolo, but not the start
//---------------------------------------------------------

static bool checkStart(Element* e)
      {
      if (e == 0 || !e->isChordRest())
            return false;
      ChordRest* cr = toChordRest(e);
      bool rv = false;
      if (cr->tuplet()) {
            // check that complete tuplet is selected, all the way up to top level
            Tuplet* tuplet = cr->tuplet();
            while (tuplet) {
                  if (tuplet->elements().front() != e)
                        return true;
                  e = tuplet;
                  tuplet = tuplet->tuplet();
                  }
            }
      else if (cr->type() == ElementType::CHORD) {
            rv = false;
            Chord* chord = toChord(cr);
            if (chord->tremolo() && chord->tremolo()->twoNotes())
                  rv = chord->tremolo()->chord2() == chord;
            }
      return rv;
      }

//---------------------------------------------------------
//   checkEnd
//     return false if element is NOT a tuplet or is end of a tuplet
//     return true  if element is part of a tuplet, but not the end
//---------------------------------------------------------

static bool checkEnd(Element* e, const Fraction& endTick)
      {
      if (e == 0 || !e->isChordRest())
            return false;
      ChordRest* cr = toChordRest(e);
      bool rv = false;
      if (cr->tuplet()) {
            // check that complete tuplet is selected, all the way up to top level
            Tuplet* tuplet = cr->tuplet();
            while (tuplet) {
                  if (tuplet->elements().back() != e)
                        return true;
                  e = tuplet;
                  tuplet = tuplet->tuplet();
                  }
            // also check that the selection extends to the end of the top-level tuplet
            tuplet = toTuplet(e);
            if (tuplet->elements().front()->tick() + tuplet->actualTicks() > endTick)
                  return true;
            }
      else if (cr->type() == ElementType::CHORD) {
            rv = false;
            Chord* chord = toChord(cr);
            if (chord->tremolo() && chord->tremolo()->twoNotes())
                  rv = chord->tremolo()->chord1() == chord;
            }
      return rv;
      }

//---------------------------------------------------------
//   canCopy
//    return false if range selection intersects a tuplet
//    or a tremolo, or a local time signature
//---------------------------------------------------------

bool Selection::canCopy() const
      {
      if (_state != SelState::RANGE)
            return true;

      Fraction endTick = _endSegment ? _endSegment->tick() : _score->lastSegment()->tick();

      for (int staffIdx = _staffStart; staffIdx != _staffEnd; ++staffIdx) {

            for (int voice = 0; voice < VOICES; ++voice) {
                  int track = staffIdx * VOICES + voice;
                  if (!canSelectVoice(track))
                        continue;

                  // check first cr in track within selection
                  ChordRest* check = _startSegment->nextChordRest(track);
                  if (check && check->tick() < endTick && checkStart(check))
                        return false;

                  if (! _endSegment)
                        continue;

                  // find last segment in the selection.
                  // Note that _endSegment is the first segment after the selection

                  Segment *endSegmentSelection = _startSegment;
                  while (endSegmentSelection->nextCR(track) &&
                        (endSegmentSelection->nextCR(track)->tick() < _endSegment->tick()))
                        endSegmentSelection = endSegmentSelection->nextCR(track);

                  if (checkEnd(endSegmentSelection->element(track), endTick))
                        return false;
                  }

            // loop through measures on this staff checking for local time signatures
            for (Measure* m = _startSegment->measure(); m && m->tick() < endTick; m = m->nextMeasure()) {
                  if (_score->staff(staffIdx)->isLocalTimeSignature(m->tick()))
                        return false;
                  }

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
      Segment* s2 = endSegment();
      *m2 = s2 ? s2->measure() : _score->lastMeasure();
      if (*m1 == *m2)
            return true;
      // if selection extends to last segment of a measure,
      // then endSegment() will point to next measure
      // this won't normally happen because end barlines are excluded from range selection
      // but just in case, detect this and back up one measure
      if (*m2 && s2 && (*m2)->tick() == s2->tick())
            *m2 = (*m2)->prevMeasure();
      return true;
      }

//---------------------------------------------------------
//   uniqueElements
//    Return list of selected elements.
//    If some elements are linked, only one of the linked
//    elements show up in the list.
//---------------------------------------------------------

const std::list<Element*> Selection::uniqueElements() const
      {
      std::list<Element*> l;

      for (Element* e : elements()) {
            bool alreadyThere = false;
            for (Element* ee : l) {
                  if ((ee->links() && ee->links()->contains(e)) || e == ee) {
                        alreadyThere = true;
                        break;
                        }
                  }
            if (!alreadyThere)
                  l.push_back(e);
            }
      return l;
      }

//---------------------------------------------------------
//   uniqueNotes
//    Return list of selected notes.
//    If some notes are linked, only one of the linked
//    elements show up in the list.
//---------------------------------------------------------

std::list<Note*> Selection::uniqueNotes(int track) const
      {
      std::list<Note*> l;

      for (Note* nn : noteList(track)) {
            for (Note* note : nn->tiedNotes()) {
                  bool alreadyThere = false;
                  for (Note* n : l) {
                        if ((n->links() && n->links()->contains(note)) || n == note) {
                              alreadyThere = true;
                              break;
                              }
                        }
                  if (!alreadyThere)
                        l.push_back(note);
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
      extendRangeSelection(cr->segment(),
         cr->nextSegmentAfterCR(SegmentType::ChordRest
            | SegmentType::EndBarLine
            | SegmentType::Clef),
            cr->staffIdx(),
            cr->tick(),
            cr->tick());
      }

//---------------------------------------------------------
//   extendRangeSelection
//    Extends the range selection to contain the given
//    segment. SegAfter should represent the segment
//    that is after seg. Tick and etick represent
//    the start and end tick of an element. Useful when
//    extending by a chord rest.
//---------------------------------------------------------

void Selection::extendRangeSelection(Segment* seg, Segment* segAfter, int staffIdx, const Fraction& tick, const Fraction& etick)
      {
      bool activeIsFirst = false;
      int activeStaff = _activeTrack / VOICES;

      if (staffIdx < _staffStart)
            _staffStart = staffIdx;
      else if (staffIdx >= _staffEnd)
            _staffEnd = staffIdx + 1;
      else if (_staffEnd - _staffStart > 1) { // at least 2 staff selected
            if (staffIdx == _staffStart + 1 && activeStaff == _staffStart) // going down
                  _staffStart = staffIdx;
            else if (staffIdx == _staffEnd - 2 && activeStaff == _staffEnd - 1) // going up
                  _staffEnd = staffIdx + 1;
            }

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
      _score->setSelectionChanged(true);
      Q_ASSERT(!(_endSegment && !_startSegment));
      }

//---------------------------------------------------------
//   selectionFilter
//---------------------------------------------------------

SelectionFilter Selection::selectionFilter() const
      {
      return _score->selectionFilter();
      }

//---------------------------------------------------------
//   setFiltered
//---------------------------------------------------------

void SelectionFilter::setFiltered(SelectionFilterType type, bool set)
      {
      if (set)
            _filtered = _filtered | (int)type;
      else
            _filtered = _filtered & ~(int)type;
      }
}

