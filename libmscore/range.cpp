//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012-2016 Werner Schweer
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
#include "repeat.h"
#include "tremolo.h"

namespace Ms {

//---------------------------------------------------------
//   cleanupTuplet
//---------------------------------------------------------

static void cleanupTuplet(Tuplet* t)
      {
      const auto elements(t->elements());
      t->clear();
      for (DurationElement* e : elements) {
            e->setTuplet(nullptr); // prevent deleting the top tuplet by its children
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
//   appendTuplet
//---------------------------------------------------------

void TrackList::appendTuplet(Tuplet* srcTuplet, Tuplet* dstTuplet)
      {
      for (DurationElement* de : srcTuplet->elements()) {
            DurationElement* e = toDurationElement(de->clone());
            dstTuplet->add(e);
            if (de->isTuplet()) {
                  Tuplet* st = toTuplet(de);
                  Tuplet* dt = toTuplet(e);
                  appendTuplet(st, dt);
                  }
            else if (de->parent() && de->parent()->isSegment()) {
                  Segment* seg = toSegment(de->parent());
                  for (Element* ee : seg->annotations()) {
                        if (ee->track() == e->track())
                              _range->annotations.push_back({ e->tick(), ee->clone() });
                        }
                  }
            }
      }

//---------------------------------------------------------
//   combineTuplet
//---------------------------------------------------------

void TrackList::combineTuplet(Tuplet* dst, Tuplet* src)
      {
      dst->setTicks(dst->ticks() * 2);
      dst->setBaseLen(dst->baseLen().shift(-1));

      // try to combine tie'd notes
      unsigned idx = 0;
      if (dst->elements().back()->isChord() && src->elements().front()->isChord()) {
            Chord* chord = toChord(src->elements().front());
            bool akkumulateChord = true;
            for (Note* n : chord->notes()) {
                  if (!n->tieBack() || !n->tieBack()->generated()) {
                        akkumulateChord = false;
                        break;
                        }
                  }
            if (akkumulateChord) {
                  Chord* bc  = toChord(dst->elements().back());
                  bc->setTicks(bc->ticks() + chord->ticks());

                  // forward ties
                  int i = 0;
                  for (Note* n : bc->notes()) {
                        n->setTieFor(chord->notes()[i]->tieFor());
                        ++i;
                        }
                  idx = 1;    // skip first src element
                  }
            }

      for (; idx < src->elements().size(); ++idx) {
            DurationElement* de = src->elements()[idx];
            DurationElement* e = toDurationElement(de->clone());
            dst->add(e);
            if (de->isTuplet()) {
                  Tuplet* st = toTuplet(de);
                  Tuplet* dt = toTuplet(e);
                  appendTuplet(st, dt);
                  }
            }
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void TrackList::append(Element* e)
      {
      if (e->isDurationElement()) {
            _duration += toDurationElement(e)->ticks();

            bool accumulateRest = e->isRest() && !empty() && back()->isRest();
            Segment* s          = accumulateRest ? toRest(e)->segment() : 0;

            if (s && !s->score()->isSpannerStartEnd(s->tick(), e->track()) && !s->annotations().size()) {
                  // akkumulate rests
                  Rest* rest  = toRest(back());
                  Fraction du = rest->ticks();
                  du += toRest(e)->ticks();
                  rest->setTicks(du);
                  }
            else {
                  Element* element = 0;
                  if (e->isTuplet()) {
                        Tuplet* src = toTuplet(e);
                        if (src->generated() && !empty() && back()->isTuplet()) {
                              Tuplet* b = toTuplet(back());
                              combineTuplet(b, src);
                              }
                        else {
                              element = e->clone();
                              Tuplet* dst = toTuplet(element);
                              appendTuplet(src, dst);
                              }
                        }
                  else {
                        element = e->clone();
                        ChordRest* src = toChordRest(e);
                        Segment* s1 = src->segment();
                        for (Element* ee : s1->annotations()) {
                              if (ee->track() == e->track())
                                    _range->annotations.push_back({ s1->tick(), ee->clone() });
                              }
                        if (e->isChord()) {
                              Chord* chord = toChord(e);
                              bool akkumulateChord = true;
                              for (Note* n : chord->notes()) {
                                    if (!n->tieBack() || !n->tieBack()->generated()) {
                                          akkumulateChord = false;
                                          break;
                                          }
                                   }
                              if (akkumulateChord && !empty() && back()->isChord()) {
                                    Chord* bc   = toChord(back());
                                    const Fraction du = bc->ticks() + chord->ticks();
                                    bc->setTicks(du);

                                    // forward ties
                                    int idx = 0;
                                    for (Note* n : bc->notes()) {
                                          n->setTieFor(chord->notes()[idx]->tieFor());
                                          ++idx;
                                          }
                                    delete element;
                                    element = 0;
                                    }
                              }
                        }
                  if (element) {
                        element->setSelected(false);
                        QList<Element*>::append(element);
                        }
                  }
            }
      else {
            Element* c = e->clone();
            c->setParent(0);
            QList<Element*>::append(c);
            }
      }

//---------------------------------------------------------
//   appendGap
//---------------------------------------------------------

void TrackList::appendGap(const Fraction& du)
      {
      if (du.isZero())
            return;
      Element* e = empty() ? 0 : back();
      if (e && e->isRest()) {
            Rest* rest  = toRest(back());
            Fraction dd = rest->ticks();
            dd          += du;
            _duration   += du;
            rest->setTicks(dd);
            }
      else {
            Rest* rest = new Rest(0);
            rest->setTicks(du);
            QList<Element*>::append(rest);
            _duration   += du;
            }
      }

//---------------------------------------------------------
//   truncate
//    reduce len of last gap by f
//---------------------------------------------------------

bool TrackList::truncate(const Fraction& f)
      {
      if (empty())
            return true;
      Element* e = back();
      if (!e->isRest())
            return false;
      Rest* r = toRest(e);
      if (r->ticks() < f)
            return false;
      if (r->ticks() == f) {
            removeLast();
            delete r;
            }
      else
            r->setTicks(r->ticks() - f);
      _duration -= f;
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TrackList::read(const Segment* fs, const Segment* es)
      {
      Fraction tick = fs->tick();

      const Segment* s;
      for (s = fs; s && (s != es); s = s->next1()) {
            if (!s->enabled())
                  continue;
            Element* e = s->element(_track);
            if (!e || e->generated()) {
                  for (Element* ee : s->annotations()) {
                        if (ee->track() == _track)
                              _range->annotations.push_back({ s->tick(), ee->clone() });
                        }
                  continue;
                  }
            if (e->isRepeatMeasure()) {
                  // TODO: copy previous measure contents?
                  RepeatMeasure* rm = toRepeatMeasure(e);
                  Rest r(*rm);
                  append(&r);
                  tick += r.ticks();
                  }
            else if (e->isChordRest()) {
                  DurationElement* de = toDurationElement(e);
                  Fraction gap = s->tick() - tick;
                  if (de->tuplet()) {
                        Tuplet* t = de->topTuplet();
                        s  = skipTuplet(t);    // continue with first chord/rest after tuplet
                        de = t;
                        }

                  if (gap.isNotZero()) {
                        appendGap(gap);
                        tick += gap;
                        }
                  append(de);
                  tick += de->ticks();
                  }
            else if (e->isBarLine()) {
                  BarLine* bl = toBarLine(e);
                  if (bl->barLineType() != BarLineType::NORMAL)
                        append(e);
                  }
            else
                  append(e);
            }
      Fraction gap = es->tick() - tick;
      if (gap.isNotZero())
            appendGap(gap);

      //
      // connect ties
      //
      int n = size();
      for (int i = 0; i < n; ++i) {
            Element* e = at(i);
            if (!e->isChord())
                  continue;
            Chord* chord = toChord(e);
            for (Note* n1 : chord->notes()) {
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
//   checkRest
//---------------------------------------------------------

static bool checkRest(Fraction& rest, Measure*& m, const Fraction& d)
      {
      if (rest.isZero()) {
            if (m->nextMeasure()) {
                  m  = m->nextMeasure();
                  rest = m->ticks();
                  }
            else {
                  qDebug("premature end of measure list, rest %d/%d", d.numerator(), d.denominator());
                  return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   writeTuplet
//    measure - current measure
//    rest    - available time in measure
//---------------------------------------------------------

Tuplet* TrackList::writeTuplet(Tuplet* parent, Tuplet* tuplet, Measure*& measure, Fraction& rest) const
      {
      Score* score = measure->score();
      Tuplet* dt   = tuplet->clone();
      dt->setParent(measure);
      Fraction du  = tuplet->ticks();
      if (du > rest) {
            // we must split the tuplet
            dt->setTicks(du * Fraction(1, 2));
            dt->setBaseLen(tuplet->baseLen().shift(1));
            }
      if (parent)
            parent->add(dt);

      for (DurationElement* e : tuplet->elements()) {
            Fraction duration = e->globalTicks();
            Tuplet* tt        = dt;
            Fraction ratio    = Fraction(1, 1);
            while (tt) {
                  ratio *= tt->ratio();
                  tt = tt->tuplet();
                  }

            bool firstpart = true;
            while (duration > Fraction(0,1)) {
                  if (rest.isZero()) {
                        if (measure->nextMeasure()) {
                              measure = measure->nextMeasure();
                              rest    = measure->ticks();
                              if (e != tuplet->elements().back()) {
                                    // create second part of split tuplet
                                    dt = dt->clone();
                                    dt->setGenerated(true);
                                    dt->setParent(measure);
                                    Tuplet* pt = dt;
                                    while (parent) {
                                          Tuplet* tt1 = parent->clone();
                                          tt1->setGenerated(true);
                                          tt1->setParent(measure);
                                          tt1->add(pt);
                                          pt = tt1;
                                          parent = parent->tuplet();
                                          }
                                    }
                              }
                        else {
                              qFatal("premature end of measure list in track %d, rest %d/%d",
                                 _track, duration.numerator(), duration.denominator());
                              }
                        }
                  if (e->isChordRest()) {
                        Fraction dd = qMin(rest, duration) * ratio;
                        std::vector<TDuration> dl = toDurationList(dd, false);
                        for (const TDuration& k : dl) {
                              Segment* segment = measure->undoGetSegmentR(SegmentType::ChordRest, measure->ticks() - rest);
                              Fraction gd      = k.fraction() / ratio;
                              ChordRest* cr    = toChordRest(e->clone());
                              if (!firstpart)
                                    cr->removeMarkings(true);
                              cr->setScore(score);
                              cr->setTrack(_track);
                              segment->add(cr);
                              cr->setTicks(k.fraction());
                              cr->setDurationType(k);
                              rest     -= gd;
                              duration -= gd;

                              if (cr->isChord()) {
                                    for (Note* note : toChord(cr)->notes()) {
                                          if (!duration.isZero() && !note->tieFor()) {
                                                Tie* tie = new Tie(score);
                                                tie->setGenerated(true);
                                                note->add(tie);
                                                }
                                          }
                                    }
                              dt->add(cr);
                              firstpart = false;
                              }
                        }
                  else if (e->isTuplet()) {
                        Tuplet* tt1 = toTuplet(e);
                        Tuplet* ttt = writeTuplet(dt, tt1, measure, rest);
                        dt          = ttt->tuplet();
                        parent      = dt->tuplet();
                        duration    = Fraction();
                        }
                  firstpart = false;
                  }
            }
      return dt;
      }

//---------------------------------------------------------
//   write
//    rewrite notes into measure list measure
//---------------------------------------------------------

bool TrackList::write(Score* score, const Fraction& tick) const
      {
      if ((_track % VOICES) && size() == 1 && at(0)->isRest())     // don’t write rests in voice > 0
            return true;
      Measure* measure = score->tick2measure(tick);
      Measure* m       = measure;
      Fraction remains = m->endTick() - tick;
      Segment* segment = 0;

      for (Element* e : *this) {
            if (e->isDurationElement()) {
                  Fraction duration = toDurationElement(e)->ticks();
                  if (!checkRest(remains, m, duration)) {     // go to next measure, if necessary
                        MScore::setError(CORRUPTED_MEASURE);
                        return false;
                        }
                  if (duration > remains && e->isTuplet()) {
                        // experimental: allow tuplet split in the middle
                        if (duration != remains * 2) {
                              MScore::setError(CANNOT_SPLIT_TUPLET);
                              return false;
                              }
                        }
                  bool firstpart = true;
                  while (duration > Fraction(0,1)) {
                        if ((e->isRest() || e->isRepeatMeasure()) && (duration >= remains || e == back()) && (remains == m->ticks())) {
                              //
                              // handle full measure rest
                              //
                              Segment* seg = m->getSegmentR(SegmentType::ChordRest, m->ticks() - remains);
                              if ((_track % VOICES) == 0) {
                                    // write only for voice 1
                                    Rest* r = new Rest(score, TDuration::DurationType::V_MEASURE);
                                    // ideally we should be using stretchedLen
                                    // but this is not valid during rewrite when adding time signatures
                                    // since the time signature has not been added yet
                                    //Fraction stretchedLen = m->stretchedLen(staff);
                                    //r->setTicks(stretchedLen);
                                    r->setTicks(m->ticks());
                                    r->setTrack(_track);
                                    seg->add(r);
                                    }
                              duration -= m->ticks();
                              remains.set(0, 1);
                              }
                        else if (e->isChordRest()) {
                              Fraction du               = qMin(remains, duration);
                              std::vector<TDuration> dl = toDurationList(du, e->isChord());
                              if (dl.empty()) {
                                    MScore::setError(CORRUPTED_MEASURE);
                                    return false;
                                    }
                              for (const TDuration& k : dl) {
                                    segment       = m->undoGetSegmentR(SegmentType::ChordRest, m->ticks() - remains);
                                    ChordRest* cr = toChordRest(e->clone());
                                    if (!firstpart)
                                          cr->removeMarkings(true);
                                    cr->setTrack(_track);
                                    cr->setScore(score);
                                    Fraction gd = k.fraction();
                                    cr->setTicks(gd);
                                    cr->setDurationType(k);

                                    segment->add(cr);
                                    duration -= gd;
                                    remains  -= gd;

                                    if (cr->isChord()) {
                                          if (!firstpart && toChord(cr)->tremolo() && toChord(cr)->tremolo()->twoNotes()) { // remove partial two-note tremolo
                                                if (toChord(e)->tremolo()->chord1() == toChord(e))
                                                      toChord(cr)->tremolo()->setChords(toChord(cr),nullptr);
                                                else
                                                      toChord(cr)->tremolo()->setChords(nullptr,toChord(cr));
                                                Tremolo* tremoloPointer = toChord(cr)->tremolo();
                                                toChord(cr)->setTremolo(nullptr);
                                                delete tremoloPointer;
                                                }
                                          for (Note* note : toChord(cr)->notes()) {
                                                if (!duration.isZero() && !note->tieFor()) {
                                                      Tie* tie = new Tie(score);
                                                      tie->setGenerated(true);
                                                      note->add(tie);
                                                      }
                                                }
                                          }
                                    }
                              }
                        else if (e->isTuplet()) {
                              writeTuplet(0, toTuplet(e), m, remains);
                              duration = Fraction();
                              }
                        firstpart = false;
                        if (duration > Fraction(0,1)) {
                              if (!checkRest(remains, m, duration)) {     // go to next measure, if necessary
                                    MScore::setError(CORRUPTED_MEASURE);
                                    return false;
                                    }
                              }
                        }
                  }
            else if (e->isBarLine()) {
//                  if (pos.numerator() == 0 && m) {
//                        BarLineType t = toBarLine(e)->barLineType();
//                        Measure* pm = m->prevMeasure();
//TODO                        if (pm)
//                              pm->setEndBarLineType(t,0);
//                        }
                  }
            else if (e->isClef()) {
                  Segment* seg;
                  if (remains == m->ticks() && m->tick() > Fraction(0,1)) {
                        Measure* pm = m->prevMeasure();
                        seg = pm->getSegmentR(SegmentType::Clef, pm->ticks());
                        }
                  else if (remains != m->ticks())
                        seg = m->getSegmentR(SegmentType::Clef, m->ticks() - remains);
                  else
                        seg = m->getSegmentR(SegmentType::HeaderClef, Fraction(0,1));
                  Element* ne = e->clone();
                  ne->setScore(score);
                  ne->setTrack(_track);
                  seg->add(ne);
                  }
            else {
                  if (!m)
                        break;
                  // add the element in its own segment;
                  // but KeySig has to be at start of (current) measure

                  Segment* seg = m->getSegmentR(Segment::segmentType(e->type()), e->isKeySig() ? Fraction() : m->ticks() - remains);
                  Element* ne = e->clone();
                  ne->setScore(score);
                  ne->setTrack(_track);
                  seg->add(ne);
                  }
            }
      //
      // connect ties from measure->first() to segment
      //

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
//   read
//---------------------------------------------------------

void ScoreRange::read(Segment* first, Segment* last, bool readSpanner)
      {
      _first        = first;
      _last         = last;
      Score* score  = first->score();
      QList<int> sl = score->uniqueStaves();

      int startTrack = 0;
      int endTrack   = score->nstaves() * VOICES;

      spanner.clear();

      if (readSpanner) {
            Fraction stick = first->tick();
            Fraction etick = last->tick();
            for (auto i : first->score()->spanner()) {
                  Spanner* s = i.second;
                  if (s->tick() >= stick && s->tick() < etick && s->track() >= startTrack && s->track() < endTrack) {
                        Spanner* ns = toSpanner(s->clone());
                        ns->setParent(0);
                        ns->setStartElement(0);
                        ns->setEndElement(0);
                        ns->setTick(ns->tick() - stick);
                        spanner.push_back(ns);
                        }
                  }
            }
      for (int staffIdx : qAsConst(sl)) {
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

bool ScoreRange::write(Score* score, const Fraction& tick) const
      {
      for (TrackList* dl : tracks) {
            int track = dl->track();
            if (!dl->write(score, tick))
                  return false;
            if ((track % VOICES) == VOICES - 1)  {
                  // clone staff if appropriate after all voices have been copied
                  int staffIdx = track / VOICES;
                  Staff* ostaff = score->staff(staffIdx);
                  const LinkedElements* linkedStaves = ostaff->links();
                  if (linkedStaves) {
                        for (auto le : *linkedStaves) {
                              Staff* nstaff = toStaff(le);
                              if (nstaff == ostaff)
                                    continue;
                              Excerpt::cloneStaff2(ostaff, nstaff, tick, (tick + dl->ticks()));
                              }
                        }
                  }
            ++track;
            }
      for (Spanner* s : spanner) {
            s->setTick(s->tick() + tick);
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
                  if (slur->endCR() && slur->endCR()->isGrace()) {
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
      const Fraction oldDuration = ticks();
      Fraction oldEndTick = _first->tick() + oldDuration;
      for (auto t : qAsConst(tracks))
            t->appendGap(f);

      Fraction diff = ticks() - oldDuration;
      for (Spanner* sp : qAsConst(spanner)) {
            if (sp->tick2() >= oldEndTick && sp->tick() < oldEndTick)
                  sp->setTicks(sp->ticks() + diff);
            }
      }

//---------------------------------------------------------
//   truncate
//    reduce len of last gap by f
//---------------------------------------------------------

bool ScoreRange::truncate(const Fraction& f)
      {
      for (TrackList* dl : qAsConst(tracks)) {
            if (dl->empty())
                  continue;
            Element* e = dl->back();
            if (!e->isRest())
                  return false;
            Rest* r = toRest(e);
            if (r->ticks() < f)
                  return false;
            }
      for (TrackList* dl : qAsConst(tracks))
            dl->truncate(f);
      return true;
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

Fraction ScoreRange::ticks() const
      {
      return tracks.empty() ? Fraction() : tracks[0]->ticks();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void TrackList::dump() const
      {
      qDebug("elements %d, duration %d/%d", size(), _duration.numerator(), _duration.denominator());
      for (Element* e : *this) {
            if (e->isDurationElement()) {
                  Fraction du = toDurationElement(e)->ticks();
                  qDebug("   %s  %d/%d", e->name(), du.numerator(), du.denominator());
                  }
            else
                  qDebug("   %s", e->name());
            }
      }

}

