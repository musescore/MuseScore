//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "range.h"
#include "measure.h"
#include "segment.h"
#include "rest.h"
#include "chord.h"
#include "score.h"
#include "slur.h"
#include "note.h"
#include "tuplet.h"
#include "utils.h"

//---------------------------------------------------------
//   cleanupTuplet
//---------------------------------------------------------

static void cleanupTuplet(Tuplet* t)
      {
      foreach(DurationElement* e, t->elements()) {
            if (e->type() == TUPLET)
                  cleanupTuplet(static_cast<Tuplet*>(e));
            delete e;
            }
      }

//---------------------------------------------------------
//   TrackList
//---------------------------------------------------------

TrackList::~TrackList()
      {
      int n = size();
      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (e->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  foreach(Spanner* sp, cr->spannerFor())
                        delete sp;
                  foreach(Element* el, cr->annotations())
                        delete el;
                  delete e;
                  }
            else if (e->type() == TUPLET) {
                  Tuplet* t = static_cast<Tuplet*>(e);
                  cleanupTuplet(t);
                  }
            else
                  delete e;
            }
      }

//---------------------------------------------------------
//   readSpanner
//---------------------------------------------------------

void TrackList::readSpanner(int track, const QList<Spanner*>& spannerFor,
   const QList<Spanner*>& spannerBack, ChordRest* dst,
   QHash<Spanner*,Spanner*>* map)
      {
      foreach (Spanner* oldSpanner, spannerFor) {
            if (track != -1 && oldSpanner->track() != track)
                  continue;
            Spanner* newSpanner = static_cast<Spanner*>(oldSpanner->clone());
            map->insert(oldSpanner, newSpanner);
            dst->addSpannerFor(newSpanner);
            newSpanner->setStartElement(dst);
            int etick = oldSpanner->endTick();
            if (etick >= range()->last()->tick()) {
                  newSpanner->setEndElement(oldSpanner->endElement());
                  oldSpanner->removeSpannerBack();
                  map->remove(oldSpanner);
                  }
            }
      foreach (Spanner* oldSpanner, spannerBack) {
            if (track != -1 && oldSpanner->track() != track)
                  continue;
            Spanner* newSpanner = map->value(oldSpanner);
            if (newSpanner) {
                  dst->addSpannerBack(newSpanner);
                  newSpanner->setEndElement(dst);
                  map->remove(oldSpanner);
                  }
            else {
                  int stick = oldSpanner->startTick();
                  if (stick < range()->first()->tick()) {
                        dst->addSpannerBack(oldSpanner);
                        }
                  else {
                        qDebug("readSpanner: %s not found", oldSpanner->name());
                        // TODO: handle failure case:
                        // - remove start spanner
                        // - delete spanner
                        }
                  }
            }
      }

//---------------------------------------------------------
//   writeSpanner
//---------------------------------------------------------

void TrackList::writeSpanner(int track, ChordRest* src, ChordRest* dst,
   Segment* segment, QHash<Spanner*, Spanner*>* map) const
      {
      foreach(Spanner* oldSpanner, src->spannerFor()) {
            Spanner* newSpanner = static_cast<Spanner*>(oldSpanner->clone());
            newSpanner->setTrack(track);
            map->insert(oldSpanner, newSpanner);
            if (newSpanner->type() == SLUR) {
                  dst->addSpannerFor(newSpanner);
                  newSpanner->setStartElement(dst);
                  }
            else {
                  segment->addSpannerFor(newSpanner);
                  newSpanner->setStartElement(segment);
                  }
            int etick = oldSpanner->endTick();
            if (etick >= range()->last()->tick()) {
                  newSpanner->setEndElement(oldSpanner->endElement());
                  newSpanner->addSpannerBack();
                  map->remove(oldSpanner);
                  }
            }

      foreach(Spanner* oldSpanner, src->spannerBack()) {
            Spanner* newSpanner = map->value(oldSpanner);
            if (newSpanner) {
                  if (newSpanner->type() == SLUR) {
                        dst->addSpannerBack(newSpanner);
                        newSpanner->setEndElement(dst);
                        }
                  else {
                        segment->addSpannerBack(newSpanner);
                        newSpanner->setEndElement(segment);
                        }
                  map->remove(oldSpanner);
                  }
            else {
                  int stick = oldSpanner->startTick();
                  if (stick < range()->first()->tick()) {
                        if (oldSpanner->type() == SLUR) {
                              dst->addSpannerBack(oldSpanner);
                              oldSpanner->setEndElement(dst);
                              }
                        else {
                              segment->addSpannerBack(oldSpanner);
                              oldSpanner->setEndElement(segment);
                              }
                        }
                  else {
                        qDebug("writeSpanner: %s not found", oldSpanner->name());
                        // TODO: handle failure case:
                        // - remove start slur from chord/rest
                        // - delete slur
                        }
                  }
            }
      foreach(Element* e, src->annotations()) {
            Element* element = e->clone();
            element->setTrack(track);
            segment->add(element);
            }
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void TrackList::append(Element* e, QHash<Spanner*,Spanner*>* map)
      {
      Q_ASSERT(map);

      if (e->isDurationElement()) {
            Fraction d = static_cast<DurationElement*>(e)->duration();
            _duration += d;

            bool accumulateRest = e->type() == REST && !isEmpty() && back()->type() == REST;
            Segment* s = accumulateRest ? static_cast<Rest*>(e)->segment() : 0;

            if (s && s->spannerFor().isEmpty() && s->spannerBack().isEmpty()) {
                  // akkumulate rests
                  Rest* rest = static_cast<Rest*>(back());
                  Fraction d = rest->duration();
                  d += static_cast<Rest*>(e)->duration();
                  rest->setDuration(d);
                  }
            else {
                  Element* element = e->clone();
                  QList<Element*>::append(element);
                  if (e->type() == TUPLET) {
                        Tuplet* srcTuplet = static_cast<Tuplet*>(e);
                        Tuplet* dstTuplet = static_cast<Tuplet*>(element);

                        foreach(const DurationElement* de, srcTuplet->elements())
                              dstTuplet->add(de->clone());
                        }
                  else {
                        ChordRest* src = static_cast<ChordRest*>(e);
                        ChordRest* dst = static_cast<ChordRest*>(element);

                        readSpanner(-1, src->spannerFor(), src->spannerBack(), dst, map);
                        Segment* s = src->segment();
                        readSpanner(src->track(), s->spannerFor(), s->spannerBack(), dst, map);
                        foreach(Element* ee, src->segment()->annotations()) {
                              if (ee->track() == e->track())
                                    dst->annotations().append(ee->clone());
                              }
                        }
                  }
            }
      else
            QList<Element*>::append(e->clone());
      }

//---------------------------------------------------------
//   appendGap
//---------------------------------------------------------

void TrackList::appendGap(const Fraction& d)
      {
      if (!isEmpty() && (back()->type() == REST)) {
            Rest* rest  = static_cast<Rest*>(back());
            Fraction dd = rest->duration();
            dd          += d;
            _duration   += d;
            rest->setDuration(dd);
            }
      else {
            Rest* rest = new Rest(0);
            rest->setDuration(d);
            QList<Element*>::append(rest);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TrackList::read(int track, const Segment* fs, const Segment* es, QHash<Spanner*,Spanner*>* map)
      {
      int tick = fs->tick();
      int gap  = 0;
      const Segment* s;
      for (s = fs; s; s = s->next1()) {
            Element* e = s->element(track);
            if (e && !e->generated()) {
                  if (e->isChordRest()) {
                        DurationElement* de = static_cast<DurationElement*>(e);
                        gap = s->tick() - tick;
                        if (de->tuplet()) {
                              Tuplet* tuplet = de->tuplet();
                              if (tuplet->elements().front() != de) {
                                    qDebug("TrackList::read: cannot start in middle of tuplet");
                                    abort();
                                    }
                              de = tuplet;

                              // find last chord/rest in (possibly nested) tuplet:
                              DurationElement* nde = tuplet;
                              while (nde) {
                                    nde = tuplet->elements().back();
                                    if (nde->type() != TUPLET)
                                          break;
                                    }
                              s = static_cast<ChordRest*>(nde)->segment();
                              // continue with first chord/rest after tuplet
                              }
                        if (gap)
                              appendGap(Fraction::fromTicks(gap));
                        append(de, map);
                        // add duration to ticks if not grace note
                        if (!(de->type() == CHORD && static_cast<Chord*>(de)->isGrace()))
                              tick += de->duration().ticks();;
                        }
                  else if (e->type() == BAR_LINE)
                        ;
                  else
                        append(e, map);
                  }
            if (s == es)
                  break;
            }
      gap = es->tick() - tick;
      if (gap)
            appendGap(Fraction::fromTicks(gap));

      //
      // connect ties
      //
      int n = size();
      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (e->type() != CHORD)
                  continue;
            Chord* chord = static_cast<Chord*>(e);
            foreach(Note* n1, chord->notes()) {
                  Tie* tie = n1->tieFor();
                  if (!tie)
                        continue;
                  for (int k = i+1; k < n; ++k) {
                        Element* ee = at(k);
                        if (ee->type() != CHORD)
                              continue;
                        Chord* c2 = static_cast<Chord*>(ee);
                        bool found = false;
                        foreach(Note* n2, c2->notes()) {
                              if (n1->pitch() == n2->pitch()) {
                                    tie->setEndNote(n2);
                                    n2->setTieBack(tie);
                                    found = true;
                                    break;
                                    }
                              }
                        if (!found)
                              printf("Tied note not found\n");
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   writeTuplet
//---------------------------------------------------------

Tuplet* TrackList::writeTuplet(Tuplet* tuplet, Measure* measure, int tick) const
      {
      Tuplet* dt = tuplet->clone();
      dt->setParent(measure);
      foreach (DurationElement* e, tuplet->elements()) {
            if (e->isChordRest()) {
                  Element* ne = e->clone();
                  Segment::SegmentType st;
                  if (ne->type() == CHORD && static_cast<Chord*>(ne)->noteType() != NOTE_NORMAL)
                        st = Segment::SegGrace;
                  else
                        st = Segment::SegChordRest;
                  Segment* segment = measure->getSegment(st, tick);
                  segment->add(ne);
                  dt->add(ne);
                  }
            else {
                  Tuplet* nt = writeTuplet(static_cast<Tuplet*>(e), measure, tick);
                  dt->add(nt);
                  }
            tick += e->globalDuration().ticks();
            }
      return dt;
      }

//---------------------------------------------------------
//   canWrite
//    check if list can be written to measure list m
//    check for tuplets crossing barlines
//---------------------------------------------------------

bool TrackList::canWrite(const Fraction& measureLen) const
      {
      Fraction pos;
      Fraction rest = measureLen;

      int n = size();
      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (e->isDurationElement()) {
                  Fraction duration = static_cast<DurationElement*>(e)->duration();
                  if (duration > rest && e->type() == TUPLET)
                        return false;
                  while (!duration.isZero()) {
                        if (e->type() == REST && duration >= rest && rest == measureLen) {
                              duration -= rest;
                              pos = measureLen;
                              }
                        else {
                              Fraction d = qMin(rest, duration);
                              duration -= d;
                              rest -= d;
                              pos += d;
                              }
                        if (pos == measureLen) {
                              pos  = Fraction();
                              rest = measureLen;
                              }
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   write
//    rewrite notes into measure list m
//---------------------------------------------------------

bool TrackList::write(int track, Measure* measure, QHash<Spanner*, Spanner*>* map) const
      {
      Fraction pos;
      Measure* m       = measure;
      Score* score     = m->score();
      Fraction rest    = m->len();
      Segment* segment = 0;
      int n = size();

      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (e->isDurationElement()) {
                  Fraction duration = static_cast<DurationElement*>(e)->duration();

                  if (e->type() == CHORD && static_cast<Chord*>(e)->isGrace()) {
                        segment = m->getSegment(Segment::SegGrace, m->tick() + pos.ticks());
                        Element* element = e->clone();
                        element->setTrack(track);
                        segment->add(element);
                        writeSpanner(track + i, static_cast<Chord*>(e),
                           static_cast<Chord*>(element), segment, map);
                        continue;
                        }
                  if (duration > rest && e->type() == TUPLET) {
                        // cannot split tuplet
                        qDebug("TrackList::write: cannot split tuplet\n");
                        return false;
                        }
                  //
                  // split note/rest
                  //

                  bool firstCRinSplit = true;
                  while (duration.numerator() > 0) {
                        // handle full measure rest
                        if (e->type() == REST
                           && (duration >= rest || e == back())
                           && rest == m->len())
                              {
                              segment = m->getSegment(e, m->tick() + pos.ticks());
                              //
                              // handle full measure rest
                              //
                              if ((track % VOICES) == 0) {
                                    // write only for voice 1
                                    Rest* r = new Rest(score, TDuration::V_MEASURE);
                                    r->setDuration(m->len());
                                    r->setTrack(track);
                                    segment->add(r);
                                    if (e->isChordRest() && firstCRinSplit) {
                                          ChordRest* src = static_cast<ChordRest*>(e);
                                          writeSpanner(track, src, r, segment, map);
                                          }
                                    }
                              duration -= m->len();
                              pos      += m->len();
                              rest.set(0, 1);
                              }
                        else {
                              Fraction d = qMin(rest, duration);
                              ChordRest* dst;
                              if (e->type() == REST) {
                                    segment = m->getSegment(Segment::SegChordRest, m->tick() + pos.ticks());
                                    Rest* r = new Rest(score, TDuration(d));
                                    dst = r;
                                    r->setTrack(track);
                                    segment->add(r);
                                    duration -= d;
                                    rest -= d;
                                    pos += d;
                                    }
                              else if (e->type() == CHORD) {
                                    segment = m->getSegment(e, m->tick() + pos.ticks());
                                    Chord* c = static_cast<Chord*>(e)->clone();
                                    dst = c;
                                    c->setScore(score);
                                    c->setTrack(track);
                                    c->setDuration(d);
                                    c->setDurationType(TDuration(d));
                                    segment->add(c);
                                    duration -= d;
                                    rest     -= d;
                                    pos      += d;
                                    foreach(Note* note, c->notes()) {
                                          if (!duration.isZero() || note->tieFor()) {
                                                Tie* tie = new Tie(score);
                                                note->add(tie);
                                                }
                                          else
                                                note->setTieFor(0);
                                          note->setTieBack(0);
                                          }
                                    }
                              else if (e->type() == TUPLET) {
                                    writeTuplet(static_cast<Tuplet*>(e), m, m->tick() + pos.ticks());
                                    duration -= d;
                                    rest     -= d;
                                    pos      += d;
                                    }
                              if (e->isChordRest() && firstCRinSplit) {
                                    ChordRest* src = static_cast<ChordRest*>(e);
                                    writeSpanner(track, src, dst, segment, map);
                                    }
                              }
                        if (pos == m->len()) {
                              if (m->nextMeasure()) {
                                    m    = m->nextMeasure();
                                    rest = m->len();
                                    pos  = Fraction();
                                    }
                              else
                                    rest = Fraction();
                              }
                        firstCRinSplit = false;
                        }
                  }
            else if (e->type() == KEYSIG) {
                  // keysig has to be at start of measure
                  }
            else if (e->type() == BAR_LINE)
                  ;
            else {
                  if (m == 0)
                        break;
                  segment = m->getSegment(e, m->tick() + pos.ticks());
                  Element* ne = e->clone();
                  ne->setScore(score);
                  ne->setTrack(track);
                  segment->add(ne);
                  }
            }

      //
      // connect ties
      //
      for (Segment* s = measure->first(); s; s = s->next1()) {
            Element* el = s->element(track);
            if (el == 0 || el->type() != CHORD)
                  continue;
            foreach(Note* n, static_cast<Chord*>(el)->notes()) {
                  Tie* tie = n->tieFor();
                  if (!tie)
                        continue;
                  Note* nn = searchTieNote(n);
                  if (nn) {
                        tie->setEndNote(nn);
                        nn->setTieBack(tie);
                        }
                  }
            if (s == segment)
                  break;
            }
      return true;
      }

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

ScoreRange::~ScoreRange()
      {
      qDeleteAll(tracks);
      }

//---------------------------------------------------------
//   canWrite
//---------------------------------------------------------

bool ScoreRange::canWrite(const Fraction& f) const
      {
      int n = tracks.size();
      for (int i = 0; i < n; ++i) {
            TrackList* dl = tracks[i];
            if (!dl->canWrite(f))
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreRange::read(Segment* first, Segment* last, int startTrack, int endTrack)
      {
      _first = first;
      _last  = last;
      spannerMap.clear();
      for (int track = startTrack; track < endTrack; ++track) {
            TrackList* dl = new TrackList(this);
            dl->read(track, first, last, &spannerMap);
            tracks.append(dl);
            }
      if (!spannerMap.isEmpty()) {
            printf("ScoreRange::read(): dangling Spanner\n");
            foreach(Spanner* s, spannerMap) {
                  printf("  <%s> end %p\n", s->name(), s->endElement());
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ScoreRange::write(int track, Measure* m) const
      {
      spannerMap.clear();
      int n = tracks.size();
      for (int i = 0; i < n; ++i) {
            const TrackList* dl = tracks[i];
            if (!dl->write(track + i, m, &spannerMap))
                  return false;
            }
      if (!spannerMap.isEmpty()) {
            printf("ScoreRange::write(): dangling Spanner\n");
            foreach(Spanner* s, spannerMap) {
                  printf("  <%s>\n", s->name());
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   fill
//---------------------------------------------------------

void ScoreRange::fill(const Fraction& f)
      {
      int n = tracks.size();
      for (int i = 0; i < n; ++i)
            tracks[i]->appendGap(f);
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

Fraction ScoreRange::duration() const
      {
      return tracks.isEmpty() ? Fraction() : tracks[0]->duration();
      }

