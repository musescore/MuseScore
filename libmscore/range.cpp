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
#include "barline.h"
#include "utils.h"
#include "staff.h"
#include "excerpt.h"

namespace Ms {

//---------------------------------------------------------
//   cleanupTuplet
//---------------------------------------------------------

static void cleanupTuplet(Tuplet* t)
      {
      foreach (DurationElement* e, t->elements()) {
            if (e->isTuplet())
                  cleanupTuplet(toTuplet(e));
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
            if (e->isTuplet()) {
                  Tuplet* t = toTuplet(e);
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

            bool accumulateRest = e->isRest() && !empty() && back()->isRest();
            Segment* s          = accumulateRest ? toRest(e)->segment() : 0;

            if (s && !s->score()->isSpannerStartEnd(s->tick(), e->track()) && !s->annotations().size()) {
                  // akkumulate rests
                  Rest* rest = toRest(back());
                  Fraction d = rest->duration();
                  d += toRest(e)->duration();
                  rest->setDuration(d);
                  }
            else
                  {
                  Element* element = e->clone();
                  QList<Element*>::append(element);
                  if (e->isTuplet()) {
                        Tuplet* srcTuplet = toTuplet(e);
                        Tuplet* dstTuplet = toTuplet(element);

                        foreach(const DurationElement* de, srcTuplet->elements())
                              dstTuplet->add(de->clone());
                        }
                  else {
                        ChordRest* src = toChordRest(e);
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
      if (d.isZero())
            return;
      Element* e = empty() ? 0 : back();
      if (e && e->isRest()) {
            Rest* rest  = toRest(back());
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

void TrackList::read(const Segment* fs, const Segment* es)
      {
      int tick = fs->tick();
      int gap  = 0;

      while (fs && !fs->enabled())
            fs = fs->next1();
      const Segment* s;
      for (s = fs; s && (s != es); s = s->next1enabled()) {
            Element* e = s->element(_track);
            if (!e || e->generated()) {
                  for (Element* ee : s->annotations()) {
                        if (ee->track() == _track)
                              _range->annotations.push_back({ s->tick(), ee->clone() });
                        }
                  continue;
                  }
            if (e->isChordRest()) {
                  DurationElement* de = static_cast<DurationElement*>(e);
                  gap = s->tick() - tick;
                  if (de->tuplet()) {
                        Tuplet* tuplet = de->tuplet();
                        if (tuplet->elements().front() != de) {
                              qFatal("TrackList::read: cannot start in middle of tuplet");
                              }
                        de = tuplet;
                        s = skipTuplet(tuplet);
                        // continue with first chord/rest after tuplet
                        }
                  if (gap) {
                        appendGap(Fraction::fromTicks(gap));
                        tick += gap;
                        }
                  append(de);
                  tick += de->duration().ticks();
                  }
            else if (e->isBarLine()) {
                  BarLine* bl = toBarLine(e);
                  if (bl->barLineType() != BarLineType::NORMAL)
                        append(e);
                  }
//            else if (e->type() == Element::REPEAT_MEASURE) {
//                  // TODO: copy previous measure contents?
//                  }
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
            if (!e->isChord())
                  continue;
            Chord* chord = toChord(e);
            foreach(Note* n1, chord->notes()) {
                  Tie* tie = n1->tieFor();
                  if (!tie)
                        continue;
                  for (int k = i+1; k < n; ++k) {
                        Element* ee = at(k);
                        if (!ee->isChord())
                              continue;
                        Chord* c2 = toChord(ee);
                        bool found = false;
                        for (Note* n2 : c2->notes()) {
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
                  Segment::Type st = Segment::Type::ChordRest;
                  Segment* segment = measure->getSegment(st, tick);
                  segment->add(ne);
                  dt->add(ne);
                  }
            else {
                  Tuplet* nt = writeTuplet(toTuplet(e), measure, tick);
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
                  if (duration > rest && e->isTuplet())
                        return false;
                  while (!duration.isZero()) {
                        if (e->isRest() && duration >= rest && rest == measureLen) {
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
      qDebug("TrackList: elements %d, duration %d/%d", size(), _duration.numerator(), _duration.denominator());
      for (int i = 0; i < size(); ++i) {
            Element* e = at(i);
            qDebug("   %s", e->name());
            if (e->isDurationElement()) {
                  Fraction d = static_cast<DurationElement*>(e)->duration();
                  qDebug("     duration %d/%d", d.numerator(), d.denominator());
                  }
            }
      }

//---------------------------------------------------------
//   write
//    rewrite notes into measure list measure
//---------------------------------------------------------

bool TrackList::write(Measure* measure) const
      {
      Fraction pos;
      Measure* m       = measure;
      Score* score     = m->score();
      Fraction rest    = m->len();
      //Staff* staff     = score->staff(track2staff(_track));
      Segment* segment = 0;
      int n            = size();
      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (e->isDurationElement()) {
                  Fraction duration = static_cast<DurationElement*>(e)->duration();

                  if (duration > rest && e->isTuplet()) {
                        // cannot split tuplet
                        qDebug("TrackList::write: cannot split tuplet");
                        return false;
                        }
                  //
                  // split note/rest
                  //

                  bool firstpart = true;
                  while (duration.numerator() > 0) {
                        if ((e->isRest() || e->isRepeatMeasure()) && (duration >= rest || e == back()) && (rest == m->len())) {
                              //
                              // handle full measure rest
                              //
                              segment = m->getSegment(Segment::Type::ChordRest, m->tick() + pos.ticks());
                              if ((_track % VOICES) == 0) {
                                    // write only for voice 1
                                    Rest* r = new Rest(score, TDuration::DurationType::V_MEASURE);
                                    // ideally we should be using stretchedLen
                                    // but this is not valid during rewrite when adding time signatures
                                    // since the time signature has not been added yet
                                    //Fraction stretchedLen = m->stretchedLen(staff);
                                    //r->setDuration(stretchedLen);
                                    r->setDuration(m->len());
                                    r->setTrack(_track);
                                    segment->add(r);
                                    }
                              duration -= m->len();
                              pos      += m->len();
                              rest.set(0, 1);
                              }
                        else {
                              Fraction d = qMin(rest, duration);
                              if (e->isRest() || e->isRepeatMeasure()) {
                                    for (const TDuration& k : toDurationList(d, false)) {
                                          Rest* r = new Rest(score, k);
                                          Fraction dd(k.fraction());
                                          r->setTrack(_track);
                                          segment = m->getSegment(Segment::Type::ChordRest, m->tick() + pos.ticks());
                                          segment->add(r);
                                          duration -= dd;
                                          rest     -= dd;
                                          pos      += dd;
                                          }
                                    }
                              else if (e->isChord()) {
                                    segment = m->getSegment(Segment::Type::ChordRest, m->tick() + pos.ticks());
                                    Chord* c = toChord(e)->clone();
                                    if (!firstpart)
                                          c->removeMarkings(true);
                                    c->setScore(score);
                                    c->setTrack(_track);
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
                              else if (e->isTuplet()) {
                                    writeTuplet(toTuplet(e), m, m->tick() + pos.ticks());
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
                                             _track, duration.numerator(), duration.denominator());
                                          ++i;
                                          qDebug("%d elements missing", n-i);
                                          for (; i < n; ++i) {
                                                Element* e = at(i);
                                                qDebug("    <%s>", e->name());
                                                if (e->isChordRest()) {
                                                      ChordRest* cr = toChordRest(e);
                                                      qDebug("       %d/%d",
                                                         cr->duration().numerator(),
                                                         cr->duration().denominator());
                                                      }
                                                }
                                          rest     = Fraction();
                                          duration = Fraction();
                                          Q_ASSERT(false);
                                          }
                                    }
                              }
                        firstpart = false;
                        }
                  }
            else if (e->isBarLine()) {
                  if (pos.numerator() == 0 && m) {
//                        BarLineType t = toBarLine(e)->barLineType();
//                        Measure* pm = m->prevMeasure();
//TODO                        if (pm)
//                              pm->setEndBarLineType(t,0);
                        }
                  }
            else if (e->isClef()) {
                  Segment* segment;
                  if (pos.ticks() == 0 && m->tick() > 0) {
                        Measure* pm = m->prevMeasure();
                        segment = pm->getSegmentR(Segment::Type::Clef, pm->endTick());
                        }
                  else if (pos.ticks() != 0)
                        segment = m->getSegmentR(Segment::Type::Clef, pos.ticks());
                  else
                        segment = m->getSegmentR(Segment::Type::HeaderClef, 0);
                  Element* ne = e->clone();
                  ne->setScore(score);
                  ne->setTrack(_track);
                  segment->add(ne);
                  }
            else {
                  if (!m)
                        break;
                  // add the element in its own segment;
                  // but KeySig has to be at start of (current) measure
                  Segment* segment = m->getSegmentR(Segment::segmentType(e->type()), m->tick() + ((e->isKeySig()) ? 0 : pos.ticks()));
                  Element* ne = e->clone();
                  ne->setScore(score);
                  ne->setTrack(_track);
                  segment->add(ne);
                  }
            }

      //
      // connect ties
      //

      if (!segment)
            return true;

      for (Segment* s = measure->first(); s; s = s->next1()) {
            Element* e = s->element(_track);
            if (!e || !e->isChord())
                  continue;
            Chord* chord = toChord(e);
            for (Note* n : chord->notes()) {
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

void ScoreRange::read(Segment* first, Segment* last)
      {
      _first = first;
      _last  = last;
      Score* score  = first->score();
      QList<int> sl = score->uniqueStaves();

      int startTrack = 0;
      int endTrack = score->nstaves() * VOICES;

      spanner.clear();
      for (auto i : first->score()->spanner()) {
            Spanner* s = i.second;
            if (s->tick() >= first->tick() && s->tick() < last->tick() &&
               s->track() >= startTrack && s->track() < endTrack) {
                  Spanner* ns = static_cast<Spanner*>(s->clone());
                  ns->setTick(ns->tick() - first->tick());
                  ns->setTick2(ns->tick2() - first->tick());
                  spanner.push_back(ns);
                  }
            }
      for (int staffIdx : sl) {
            int sTrack = staffIdx * VOICES;
            int eTrack = sTrack + VOICES;
            for (int track = sTrack; track < eTrack; ++track) {
                  TrackList* dl = new TrackList(this);
                  dl->setTrack(track);
                  dl->read(first, last);
                  tracks.append(dl);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ScoreRange::write(Score* score, int tick) const
      {
      for (TrackList* dl : tracks) {
            int track = dl->track();
            if (!dl->write(score->tick2measure(tick)))
                  return false;
            if ((track % VOICES) == VOICES - 1)  {
                  // clone staff if appropriate after all voices have been copied
                  int staffIdx = track / VOICES;
                  Staff* ostaff = score->staff(staffIdx);
                  LinkedStaves* linkedStaves = ostaff->linkedStaves();
                  if (linkedStaves) {
                        for (Staff* nstaff : linkedStaves->staves()) {
                              if (nstaff == ostaff)
                                    continue;
                              Excerpt::cloneStaff2(ostaff, nstaff, tick, tick + dl->duration().ticks());
                              }
                        }
                  }
            ++track;
            }
      for (Spanner* s : spanner) {
            s->setTick(s->tick() + first()->tick());
            s->setTick2(s->tick2() + first()->tick());
            if (s->isSlur()) {
                  Slur* slur = toSlur(s);
                  if (slur->startCR()->isGrace()) {
                        Chord* sc = slur->startChord();
                        int idx   = sc->graceIndex();
                        Chord* dc = toChord(score->findCR(s->tick(), s->track()));
                        s->setStartElement(dc->graceNotes()[idx]);
                        }
                  else
                        s->setStartElement(0);
                  if (slur->endCR()->isGrace()) {
                        Chord* sc = slur->endChord();
                        int idx   = sc->graceIndex();
                        Chord* dc = toChord(score->findCR(s->tick2(), s->track2()));
                        s->setEndElement(dc->graceNotes()[idx]);
                        }
                  else
                        s->setEndElement(0);
                  }
            score->undoAddElement(s);
            }
      for (const Annotation& a : annotations) {
            Measure* tm = score->tick2measure(a.tick);
            Segment *op = toSegment(a.e->parent());
            Segment* s = tm->undoGetSegment(op->segmentType(), a.tick);
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
      return tracks.empty() ? Fraction() : tracks[0]->duration();
      }

}

