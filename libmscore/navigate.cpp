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

#include "navigate.h"
#include "element.h"
#include "clef.h"
#include "score.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "system.h"
#include "segment.h"
#include "harmony.h"
#include "utils.h"
#include "input.h"
#include "measure.h"
#include "page.h"
#include "spanner.h"
#include "system.h"
#include "staff.h"
#include "barline.h"

namespace Ms {

//---------------------------------------------------------
//   nextChordRest
//    return next Chord or Rest
//---------------------------------------------------------

ChordRest* nextChordRest(ChordRest* cr, bool skipGrace)
      {
      if (!cr)
            return 0;

      if (cr->isGrace()) {
            //
            // cr is a grace note

            Chord* c  = toChord(cr);
            Chord* pc = toChord(cr->parent());

            if (skipGrace) {
                  cr = toChordRest(cr->parent());
                  }
            else if (cr->isGraceBefore()) {
                  QVector<Chord*> cl = pc->graceNotesBefore();
                  auto i = std::find(cl.begin(), cl.end(), c);
                  if (i == cl.end())
                        return 0;   // unable to find self?
                  ++i;
                  if (i != cl.end())
                        return *i;
                  // if this was last grace note before, return parent
                  return pc;
                  }
            else {
                  QVector<Chord*> cl = pc->graceNotesAfter();
                  auto i = std::find(cl.begin(), cl.end(), c);
                  if (i == cl.end())
                        return 0;   // unable to find self?
                  ++i;
                  if (i != cl.end())
                        return *i;
                  // if this was last grace note after, fall through to find next main note
                  cr = pc;
                  }
            }
      else {
            //
            // cr is not a grace note
            if (cr->isChord() && !skipGrace) {
                  Chord* c = toChord(cr);
                  if (!c->graceNotes().empty()) {
                        QVector<Chord*> cl = c->graceNotesAfter();
                        if (!cl.empty())
                              return cl.first();
                        }
                  }
            }

      int track = cr->track();
      SegmentType st = SegmentType::ChordRest;

      for (Segment* seg = cr->segment()->next1MM(st); seg; seg = seg->next1MM(st)) {
            ChordRest* e = toChordRest(seg->element(track));
            if (e) {
                  if (e->isChord() && !skipGrace) {
                        Chord* c = toChord(e);
                        if (!c->graceNotes().empty()) {
                              QVector<Chord*> cl = c->graceNotesBefore();
                              if (!cl.empty())
                                    return cl.first();
                              }
                        }
                  return e;
                  }
            }

      return 0;
      }

//---------------------------------------------------------
//   prevChordRest
//    return previous Chord or Rest
//    if grace is true, include grace notes
//---------------------------------------------------------

ChordRest* prevChordRest(ChordRest* cr, bool skipGrace)
      {
      if (!cr)
            return 0;

      if (cr->isGrace()) {
            //
            // cr is a grace note

            Chord* c  = toChord(cr);
            Chord* pc = toChord(cr->parent());

            if (skipGrace) {
                  cr = toChordRest(cr->parent());
                  }
            else if (cr->isGraceBefore()) {
                  QVector<Chord*> cl = pc->graceNotesBefore();
                  auto i = std::find(cl.begin(), cl.end(), c);
                  if (i == cl.end())
                        return 0;   // unable to find self?
                  if (i != cl.begin())
                        return *--i;
                  // if this was first grace note before, fall through to find previous main note
                  cr = pc;
                  }
            else {
                  QVector<Chord*> cl = pc->graceNotesAfter();
                  auto i = std::find(cl.begin(), cl.end(), c);
                  if (i == cl.end())
                        return 0;   // unable to find self?
                  if (i != cl.begin())
                        return *--i;
                  // if this was first grace note after, return parent
                  return pc;
                  }
            }
      else {
            //
            // cr is not a grace note
            if (cr->isChord() && !skipGrace) {
                  Chord* c = toChord(cr);
                  QVector<Chord*> cl = c->graceNotesBefore();
                  if (!cl.empty())
                        return cl.last();
                  }
            }

      int track = cr->track();
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = cr->segment()->prev1MM(st); seg; seg = seg->prev1MM(st)) {
            ChordRest* e = toChordRest(seg->element(track));
            if (e) {
                  if (e->type() == ElementType::CHORD && !skipGrace) {
                        QVector<Chord*> cl = toChord(e)->graceNotesAfter();
                        if (!cl.empty())
                              return cl.last();
                        }
                  return e;
                  }
            }

      return 0;
      }

//---------------------------------------------------------
//   upAlt
//    element: Note() or Rest()
//    return: Note() or Rest()
//
//    return next higher pitched note in chord
//    move to previous track if at top of chord
//---------------------------------------------------------

Element* Score::upAlt(Element* element)
      {
      Element* re = 0;
      if (element->isRest())
            re = prevTrack(toRest(element));
      else if (element->isNote()) {
            Note* note = toNote(element);
            Chord* chord = note->chord();
            const std::vector<Note*>& notes = chord->notes();
            auto i = std::find(notes.begin(), notes.end(), note);
            ++i;
            if (i != notes.end())
                  re = *i;
            else {
                  re = prevTrack(chord);
                  if (re->track() == chord->track())
                        re = element;
                  }
            }
      if (re == 0)
            return 0;
      if (re->isChord())
            re = toChord(re)->notes().front();
      return re;
      }

//---------------------------------------------------------
//   upAltCtrl
//    select top note in chord
//---------------------------------------------------------

Note* Score::upAltCtrl(Note* note) const
      {
      return note->chord()->upNote();
      }

//---------------------------------------------------------
//   downAlt
//    return next lower pitched note in chord
//    move to next track if at bottom of chord
//---------------------------------------------------------

Element* Score::downAlt(Element* element)
      {
      Element* re = 0;
      if (element->isRest())
            re = nextTrack(toRest(element));
      else if (element->isNote()) {
            Note* note   = toNote(element);
            Chord* chord = note->chord();
            const std::vector<Note*>& notes = chord->notes();
            auto i = std::find(notes.begin(), notes.end(), note);
            if (i != notes.begin()) {
                  --i;
                  re = *i;
                  }
            else {
                  re = nextTrack(chord);
                  if (re->track() == chord->track())
                        re = element;
                  }
            }
      if (re == 0)
            return 0;
      if (re->isChord())
            re = toChord(re)->notes().back();
      return re;
      }

//---------------------------------------------------------
//   downAltCtrl
//    niedrigste Note in Chord selektieren
//---------------------------------------------------------

Note* Score::downAltCtrl(Note* note) const
      {
      return note->chord()->downNote();
      }

//---------------------------------------------------------
//   firstElement
//---------------------------------------------------------

Element* Score::firstElement()
      {
      Segment *s = firstSegment(SegmentType::All);
      return s ? s->element(0) : nullptr;
      }

//---------------------------------------------------------
//   lastElement
//---------------------------------------------------------

Element* Score::lastElement()
      {
      Element* re = 0;
      Segment* seg = lastSegment();
      if (!seg)
            return nullptr;
      while (true) {
            for (int i = (staves().size() -1) * VOICES; i < staves().size() * VOICES; i++) {
                  if (seg->element(i))
                        re = seg->element(i);
                  }
            if (re) {
                  if (re->isChord()) {
                        return toChord(re)->notes().front();
                        }
                  return re;
                  }
            seg = seg->prev1MM(SegmentType::All);
            }
      }

//---------------------------------------------------------
//   upStaff
//---------------------------------------------------------

ChordRest* Score::upStaff(ChordRest* cr)
      {
      Segment* segment = cr->segment();

      if (cr->staffIdx() == 0)
            return cr;

      for (int track = (cr->staffIdx() - 1) * VOICES; track >= 0; --track) {
            Element* el = segment->element(track);
            if (!el)
                  continue;
            if (el->isNote())
                  el = toNote(el)->chord();
            if (el->isChordRest())
                  return toChordRest(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   downStaff
//---------------------------------------------------------

ChordRest* Score::downStaff(ChordRest* cr)
      {
      Segment* segment = cr->segment();
      int tracks = nstaves() * VOICES;

      if (cr->staffIdx() == nstaves() - 1)
            return cr;

      for (int track = (cr->staffIdx() + 1) * VOICES; track < tracks; --track) {
            Element* el = segment->element(track);
            if (!el)
                  continue;
            if (el->isNote())
                  el = toNote(el)->chord();
            if (el->isChordRest())
                  return toChordRest(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   nextTrack
//    returns note at or just before current (cr) position
//    in next track for this measure
//    that contains such an element
//---------------------------------------------------------

ChordRest* Score::nextTrack(ChordRest* cr)
      {
      if (!cr)
            return 0;

      ChordRest* el = 0;
      Measure* measure = cr->measure();
      int track = cr->track();
      int tracks = nstaves() * VOICES;

      while (!el) {
            // find next non-empty track
            while (++track < tracks) {
                  if (measure->hasVoice(track))
                        break;
                  }
            // no more tracks, return original element
            if (track == tracks)
                  return cr;
            // find element at same or previous segment within this track
            for (Segment* segment = cr->segment(); segment; segment = segment->prev(SegmentType::ChordRest)) {
                  el = toChordRest(segment->element(track));
                  if (el)
                        break;
                  }
            }
      return el;
      }

//---------------------------------------------------------
//   prevTrack
//    returns ChordRest at or just before current (cr) position
//    in previous track for this measure
//    that contains such an element
//---------------------------------------------------------

ChordRest* Score::prevTrack(ChordRest* cr)
      {
      if (!cr)
            return 0;

      ChordRest* el = 0;
      Measure* measure = cr->measure();
      int track = cr->track();

      while (!el) {
            // find next non-empty track
            while (--track >= 0){
                  if (measure->hasVoice(track))
                        break;
                  }
            // no more tracks, return original element
            if (track < 0)
                  return cr;
            // find element at same or previous segment within this track
            for (Segment* segment = cr->segment(); segment; segment = segment->prev(SegmentType::ChordRest)) {
                  el = toChordRest(segment->element(track));
                  if (el)
                        break;
                  }
            }
      return el;
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

ChordRest* Score::nextMeasure(ChordRest* element, bool selectBehavior, bool mmRest)
      {
      if (!element)
            return 0;

      Measure* measure = 0;
      if (mmRest)
            measure = element->measure()->nextMeasureMM();
      else
            measure = element->measure()->nextMeasure();

      if (!measure)
            return 0;

      int endTick = element->measure()->last()->nextChordRest(element->track(), true)->tick();
      bool last   = false;

      if (selection().isRange()) {
            if (element->tick() != endTick && selection().tickEnd() <= endTick) {
                  measure = element->measure();
                  last = true;
                  }
            else if (element->tick() == endTick && selection().isEndActive())
                  last = true;
            }
      else if (element->tick() != endTick && selectBehavior) {
            measure = element->measure();
            last = true;
            }
      if (!measure) {
            measure = element->measure();
            last = true;
            }
      int staff = element->staffIdx();

      Segment* startSeg = last ? measure->last() : measure->first();
      for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return toChordRest(pel);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

ChordRest* Score::prevMeasure(ChordRest* element, bool mmRest)
      {
      if (!element)
            return 0;

      Measure* measure =  0;
      if (mmRest)
            measure = element->measure()->prevMeasureMM();
      else
            measure = element->measure()->prevMeasure();

      int startTick = element->measure()->first()->nextChordRest(element->track())->tick();
      bool last = false;

      if (selection().isRange() && selection().isEndActive() && selection().startSegment()->tick() <= startTick)
            last = true;
      else if (element->tick() != startTick) {
            measure = element->measure();
            }
      if (!measure) {
            measure = element->measure();
            last = false;
            }

      int staff = element->staffIdx();

      Segment* startSeg = last ? measure->last() : measure->first();
      for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return toChordRest(pel);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Score::nextElement()
      {
      Element* e = getSelectedElement();
      if (!e)
            return nullptr;
      int staffId = e->staffIdx();
      while (e) {
            switch (e->type()) {
                  case ElementType::NOTE:
                  case ElementType::CHORD: {
                        Element* next = e->nextElement();
                        if (next)
                              return next;
                        else
                              break;
                       }
                  case ElementType::SEGMENT: {
                        Segment* s = toSegment(e);
                        Element* next = s->nextElement(staffId);
                        if (next)
                              return next;
                        else
                              break;
                        }
                  case ElementType::MEASURE: {
                        Measure* m = toMeasure(e);
                        Element* next = m->nextElementStaff(staffId);
                        if (next)
                              return next;
                        else
                              break;
                        }
                  case ElementType::CLEF:
                  case ElementType::KEYSIG:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE: {
                       for (; e && e->type() != ElementType::SEGMENT; e = e->parent()) {
                             ;
                             }
                       Segment* s = toSegment(e);
                       Element* next = s->nextElement(staffId);
                       if (next)
                             return next;
                       else
                             return score()->firstElement();
                       }
                  case ElementType::SLUR_SEGMENT:
                  case ElementType::TEXTLINE_SEGMENT:
                  case ElementType::HAIRPIN_SEGMENT:
                  case ElementType::OTTAVA_SEGMENT:
                  case ElementType::TRILL_SEGMENT:
                  case ElementType::VIBRATO_SEGMENT:
                  case ElementType::VOLTA_SEGMENT:
                  case ElementType::PEDAL_SEGMENT: {
                        SpannerSegment* s = toSpannerSegment(e);
                        Spanner* sp = s->spanner();
                        Spanner* nextSp = sp->nextSpanner(sp, staffId);
                        if (nextSp)
                              return nextSp->spannerSegments().front();

                        Segment* seg = tick2segment(sp->tick());
                        if (seg) {
                              Segment* nextSegment = seg->next1();
                              while (nextSegment) {
                                    Element* nextEl = nextSegment->firstElementOfSegment(nextSegment, staffId);
                                    if (nextEl)
                                          return nextEl;
                                    nextSegment = nextSegment->next1();
                                    }
                              }
                        break;
                        }
                  case ElementType::GLISSANDO_SEGMENT:
                  case ElementType::TIE_SEGMENT: {
                        SpannerSegment* s = toSpannerSegment(e);
                        Spanner* sp = s->spanner();
                        Element* elSt = sp->startElement();
                        Note* n = toNote(elSt);
                        Element* next =  n->nextElement();
                        if (next)
                              return next;
                        else
                              break;
                        }
                  default:
                        break;
                  }
            e = e->parent();
            }
      return score()->firstElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Score::prevElement()
      {
      Element* e = getSelectedElement();
      if (!e)
            return nullptr;
      int staffId = e->staffIdx();
      while (e) {
            switch (e->type()) {
                  case ElementType::NOTE:
                  case ElementType::REST:
                  case ElementType::CHORD: {
                        Element* prev = e->prevElement();
                        if (prev)
                              return prev;
                        else
                              break;
                        }
                  case ElementType::SEGMENT: {
                        Segment* s = toSegment(e);
                        Element* prev = s->prevElement(staffId);
                        if (prev)
                              return prev;
                        else
                              break;
                        }
                  case ElementType::MEASURE: {
                        Measure* m = toMeasure(e);
                        return m->prevElementStaff(staffId);
                        }
                  case ElementType::CLEF:
                  case ElementType::KEYSIG:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE: {
                        for (; e && e->type() != ElementType::SEGMENT; e = e->parent()) {
                              ;
                              }
                        Segment* s = toSegment(e);
                        return s->prevElement(staffId);
                        }
                  case ElementType::SLUR_SEGMENT:
                  case ElementType::TEXTLINE_SEGMENT:
                  case ElementType::HAIRPIN_SEGMENT:
                  case ElementType::OTTAVA_SEGMENT:
                  case ElementType::TRILL_SEGMENT:
                  case ElementType::VIBRATO_SEGMENT:
                  case ElementType::VOLTA_SEGMENT:
                  case ElementType::PEDAL_SEGMENT: {
                        SpannerSegment* s = toSpannerSegment(e);
                        Spanner* sp = s->spanner();
                        Element* stEl = sp->startElement();
                        Spanner* prevSp = sp->prevSpanner(sp, staffId);
                        if (prevSp)
                              return prevSp->spannerSegments().front();
                        else {
                              Segment* startSeg = sp->startSegment();
                              if (!startSeg->annotations().empty()) {
                                    Element* last = startSeg->lastAnnotation(startSeg, staffId);
                                    if (last)
                                          return last;
                                    }
                              Element* el = startSeg->lastElementOfSegment(startSeg, staffId);
                              if (stEl->type() == ElementType::CHORD || stEl->type() == ElementType::REST
                                       || stEl->type() == ElementType::REPEAT_MEASURE || stEl->type() == ElementType::NOTE) {
                                    ChordRest* cr = startSeg->cr(stEl->track());
                                    if (cr) {
                                          Element* elCr = cr->lastElementBeforeSegment();
                                          if (elCr) {
                                                return elCr;
                                                }
                                          }
                                    }
                              if (el->isChord())
                                    return toChord(el)->lastElementBeforeSegment();
                              else if (el->isNote()) {
                                    Chord* c = toNote(el)->chord();
                                    return c->lastElementBeforeSegment();
                                    }
                              else {
                                    return el;
                                    }
                             }
                        }
                  case ElementType::GLISSANDO_SEGMENT:
                  case ElementType::TIE_SEGMENT: {
                        SpannerSegment* s = toSpannerSegment(e);
                        Spanner* sp = s->spanner();
                        Element* elSt = sp->startElement();
                        Q_ASSERT(elSt->type() == ElementType::NOTE);
                        Note* n = toNote(elSt);
                        Element* prev =  n->prevElement();
                        if(prev)
                              return prev;
                        else
                              break;
                        }
                  default:
                        break;
                  }
            e = e->parent();
            }
      return score()->lastElement();
      }

}

