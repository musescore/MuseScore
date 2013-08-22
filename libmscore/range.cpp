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

#include "range.h"
#include "measure.h"
#include "segment.h"
#include "rest.h"
#include "chord.h"
#include "score.h"
#include "slur.h"
#include "tie.h"
#include "note.h"
#include "tuplet.h"
#include "utils.h"

namespace Ms {

//---------------------------------------------------------
//   cleanupTuplet
//---------------------------------------------------------

static void cleanupTuplet(Tuplet* t)
      {
      foreach(DurationElement* e, t->elements()) {
            if (e->type() == Element::TUPLET)
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
            if (e->type() == Element::TUPLET) {
                  Tuplet* t = static_cast<Tuplet*>(e);
                  cleanupTuplet(t);
                  }
            else
                  delete e;
            }
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void TrackList::append(Element* e)
      {
      if (e->isDurationElement()) {
            Fraction d = static_cast<DurationElement*>(e)->duration();
            _duration += d;

            bool accumulateRest = e->type() == Element::REST && !isEmpty()
               && back()->type() == Element::REST;
            Segment* s = accumulateRest ? static_cast<Rest*>(e)->segment() : 0;

            if (s && !s->score()->isSpannerStartEnd(s->tick(), e->track())) {
                  // akkumulate rests
                  Rest* rest = static_cast<Rest*>(back());
                  Fraction d = rest->duration();
                  d += static_cast<Rest*>(e)->duration();
                  rest->setDuration(d);
                  }
            else
                  {
                  Element* element = e->clone();
                  QList<Element*>::append(element);
                  if (e->type() == Element::TUPLET) {
                        Tuplet* srcTuplet = static_cast<Tuplet*>(e);
                        Tuplet* dstTuplet = static_cast<Tuplet*>(element);

                        foreach(const DurationElement* de, srcTuplet->elements())
                              dstTuplet->add(de->clone());
                        }
                  else {
                        ChordRest* src = static_cast<ChordRest*>(e);
                        Segment* s = src->segment();
                        foreach(Element* ee, s->annotations()) {
                              if (ee->track() == e->track())
                                    _range->annotations.push_back({ s->tick(), ee->clone() });
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
      Element* e = isEmpty() ? 0 : back();
      if (e && (e->type() == Element::REST)) {
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
            _duration   += d;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TrackList::read(int track, const Segment* fs, const Segment* es)
      {
      int tick = fs->tick();
      int gap  = 0;
      const Segment* s;
      for (s = fs; s && (s != es); s = s->next1()) {
            Element* e = s->element(track);
            if (!e || e->generated())
                  continue;
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
                              if (nde->type() != Element::TUPLET)
                                    break;
                              }
                        s = static_cast<ChordRest*>(nde)->segment();
                        // continue with first chord/rest after tuplet
                        }
                  if (gap) {
                        appendGap(Fraction::fromTicks(gap));
                        tick += gap;
                        }
                  append(de);
                  tick += de->duration().ticks();;
                  }
            else if (e->type() == Element::BAR_LINE)
                  ;
            else
                  append(e);
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
            if (e->type() != Element::CHORD)
                  continue;
            Chord* chord = static_cast<Chord*>(e);
            foreach(Note* n1, chord->notes()) {
                  Tie* tie = n1->tieFor();
                  if (!tie)
                        continue;
                  for (int k = i+1; k < n; ++k) {
                        Element* ee = at(k);
                        if (ee->type() != Element::CHORD)
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
                              qDebug("Tied note not found");
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
                  Segment::SegmentType st = Segment::SegChordRest;
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
                  if (duration > rest && e->type() == Element::TUPLET)
                        return false;
                  while (!duration.isZero()) {
                        if (e->type() == Element::REST && duration >= rest && rest == measureLen) {
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
//   dump
//---------------------------------------------------------

void TrackList::dump() const
      {
      printf("TrackList: elements %d, duration %d/%d\n", size(), _duration.numerator(), _duration.denominator());
      for (int i = 0; i < size(); ++i) {
            Element* e = at(i);
            printf("   %s\n", e->name());
            if (e->isDurationElement()) {
                  Fraction d = static_cast<DurationElement*>(e)->duration();
                  printf("     duration %d/%d\n", d.numerator(), d.denominator());
                  }
            }
      }

//---------------------------------------------------------
//   write
//    rewrite notes into measure list m
//---------------------------------------------------------

bool TrackList::write(int track, Measure* measure) const
      {
      Fraction pos;
      Measure* m       = measure;
      Score* score     = m->score();
      Fraction rest    = m->len();
      Segment* segment = 0;
      int n            = size();

      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (e->isDurationElement()) {
                  Fraction duration = static_cast<DurationElement*>(e)->duration();

                  if (duration > rest && e->type() == Element::TUPLET) {
                        // cannot split tuplet
                        qDebug("TrackList::write: cannot split tuplet\n");
                        return false;
                        }
                  //
                  // split note/rest
                  //

                  while (duration.numerator() > 0) {
                        if (e->type() == Element::REST
                           && (duration >= rest || e == back())
                           && (rest == m->len()))
                              {
                              //
                              // handle full measure rest
                              //
                              segment = m->getSegment(e, m->tick() + pos.ticks());
                              if ((track % VOICES) == 0) {
                                    // write only for voice 1
                                    Rest* r = new Rest(score, TDuration::V_MEASURE);
                                    r->setDuration(m->len());
                                    r->setTrack(track);
                                    segment->add(r);
                                    }
                              duration -= m->len();
                              pos      += m->len();
                              rest.set(0, 1);
                              }
                        else {
                              Fraction d = qMin(rest, duration);
                              if (e->type() == Element::REST) {
                                    segment = m->getSegment(Segment::SegChordRest, m->tick() + pos.ticks());
                                    Rest* r = new Rest(score, TDuration(d));
                                    r->setTrack(track);
                                    segment->add(r);
                                    duration -= d;
                                    rest     -= d;
                                    pos      += d;
                                    }
                              else if (e->type() == Element::CHORD) {
                                    segment = m->getSegment(e, m->tick() + pos.ticks());
                                    Chord* c = static_cast<Chord*>(e)->clone();
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
                              else if (e->type() == Element::TUPLET) {
                                    writeTuplet(static_cast<Tuplet*>(e), m, m->tick() + pos.ticks());
                                    duration -= d;
                                    rest     -= d;
                                    pos      += d;
                                    }
                              }
                        if (pos == m->len()) {
                              if (m->nextMeasure()) {
                                    m    = m->nextMeasure();
                                    rest = m->len();
                                    pos  = Fraction();
                                    }
                              else {
                                    if (!duration.isZero()) {
                                          qDebug("Tracklist::write: premature end of measure list in track %d, rest %d/%d",
                                             track, duration.numerator(), duration.denominator());
                                          ++i;
                                          printf("%d elements missing\n", n-i);
                                          for (; i < n; ++i) {
                                                Element* e = at(i);
                                                printf("    <%s>\n", e->name());
                                                if (e->isChordRest()) {
                                                      ChordRest* cr = static_cast<ChordRest*>(e);
                                                      printf("       %d/%d\n",
                                                         cr->duration().numerator(),
                                                         cr->duration().denominator());
                                                      }
                                                }
                                          rest     = Fraction();
                                          duration = Fraction();
                                          abort();
                                          }
                                    }
                              }
                        }
                  }
            else if (e->type() == Element::KEYSIG) {
                  // keysig has to be at start of measure
                  }
            else if (e->type() == Element::BAR_LINE)
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
            if (el == 0 || el->type() != Element::CHORD)
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
      spanner.clear();
      for (auto i : first->score()->spanner()) {
            Spanner* s = i.second;
            if (s->tick() >= first->tick() && s->tick() < last->tick() &&
               s->track() >= startTrack && s->track() < endTrack) {
                  Spanner* ns = static_cast<Spanner*>(s->clone());
                  ns->setTick(ns->tick() - first->tick());
                  spanner.push_back(ns);
                  }
            }
      for (int track = startTrack; track < endTrack; ++track) {
            TrackList* dl = new TrackList(this);
            dl->read(track, first, last);
            tracks.append(dl);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ScoreRange::write(int track, Measure* m) const
      {
      int n = tracks.size();
      for (int i = 0; i < n; ++i) {
            const TrackList* dl = tracks[i];
            if (!dl->write(track + i, m))
                  return false;
            }
      Score* score = m->score();
      for (Spanner* s : spanner) {
            s->setTick(s->tick() + m->tick());
            score->undoAddElement(s);
            }
      for (const Annotation& a : annotations) {
            Segment* s = score->tick2segment(a.tick);
            if (s) {
                  a.e->setParent(s);
                  score->undoAddElement(a.e);
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

}

