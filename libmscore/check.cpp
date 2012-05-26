//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "slur.h"
#include "measure.h"
#include "tuplet.h"
#include "chordrest.h"
#include "rest.h"
#include "segment.h"
#include "staff.h"

//---------------------------------------------------------
//   checkSlurs
//    helper routine to check for sanity slurs
//---------------------------------------------------------

void Score::checkSlurs()
      {
#if 0 //TODO1
      foreach(Element* e, _gel) {
            if (e->type() != SLUR)
                  continue;
            Slur* s = (Slur*)e;
            Element* n1 = s->startElement();
            Element* n2 = s->endElement();
            if (n1 == 0 || n2 == 0 || n1 == n2) {
                  qDebug("unconnected slur: removing\n");
                  if (n1) {
                        ((ChordRest*)n1)->removeSlurFor(s);
                        ((ChordRest*)n1)->removeSlurBack(s);
                        }
                  if (n1 == 0)
                        qDebug("  start at %d(%d) not found\n", s->tick(), s->track());
                  if (n2 == 0)
                        qDebug("  end at %d(%d) not found\n", s->tick2(), s->track2());
                  if ((n1 || n2) && (n1==n2))
                        qDebug("  start == end\n");
                  int idx = _gel.indexOf(s);
                  _gel.removeAt(idx);
                  }
            }
#endif
      }

//---------------------------------------------------------
//   checkScore
//---------------------------------------------------------

void Score::checkScore()
      {
      if (!firstMeasure())
            return;
#if 0
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->segments()->check();
#endif
      for (Segment* s = firstMeasure()->first(); s;) {
            Segment* ns = s->next1();

            if (s->subtype() & (SegChordRest | SegGrace)) {
                  bool empty = true;
                  foreach(Element* e, s->elist()) {
                        if (e) {
                              empty = false;
                              break;
                              }
                        }
                  if (empty) {
                        // Measure* m = s->measure();
qDebug("checkScore: remove empty ChordRest segment\n");
//                        m->remove(s);
                        }
                  }
            s = ns;
            }

      checkSlurs();

      ChordRest* lcr = 0;
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            int tick  = 0;
            Staff* st = staff(staffIdx);
            for (Segment* s = firstMeasure()->first(SegChordRest); s; s = s->next1(SegChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  if (!cr)
                        continue;
                  if (s->tick() != tick) {
                        if (lcr) {
                              Fraction timeStretch = st->timeStretch(lcr->tick());
                              Fraction f = cr->globalDuration() * timeStretch;
                              qDebug("Chord/Rest gap at tick %d(%s+%d)-%d(%s) staffIdx %d measure %d (len = %d)\n",
                                 tick, lcr->name(), f.ticks(),
                                 s->tick(), cr->name(), staffIdx, cr->measure()->no(),
                                 cr->tick() - tick);
                              }
                        else {
                              qDebug("Chord/Rest gap at tick %d-%d(%s) staffIdx %d measure %d (len = %d)\n",
                                 tick,
                                 s->tick(), cr->name(), staffIdx, cr->measure()->no(),
                                 cr->tick() - tick);
                              }
#if 0
                        if (cr->tick() > tick) {
                              int ttick = tick;
                              int ticks = cr->tick() - tick;

                              Fraction f = Fraction::fromTicks(ticks) / st->timeStretch(ttick);
                              qDebug("  insert %d/%d\n", f.numerator(), f.denominator());

                              while (ticks > 0) {
                                    Measure* m = tick2measure(ttick);
                                    int len    = ticks;
                                    // split notes on measure boundary
                                    if ((ttick + len) > m->tick() + m->ticks())
                                          len = m->tick() + m->ticks() - ttick;
                                    Fraction timeStretch = st->timeStretch(ttick);
                                    Fraction ff          = Fraction::fromTicks(len);
qDebug("    - insert %d/%d\n", ff.numerator(), ff.denominator());
                                    if (ff.numerator() == 0)
                                          break;
                                    Fraction fff = ff / timeStretch;

                                    QList<Duration> dl = toDurationList(fff, true);
                                    foreach(Duration d, dl) {
                                          Rest* rest = new Rest(this);
                                          rest->setDurationType(d);
                                          rest->setDuration(d.fraction());
                                          rest->setColor(Qt::red);
qDebug("    -   Rest %d/%d\n", d.fraction().numerator(), d.fraction().denominator());
                                          rest->setTrack(track);
                                          Segment* s = m->getSegment(rest, ttick);
                                          s->add(rest);
                                          ttick += (d.fraction() * timeStretch).ticks();
                                          }
                                    ticks -= len;
                                    }
                              }
#endif
                        tick = s->tick();
                        }
                  Fraction timeStretch = st->timeStretch(tick);
                  Fraction f = cr->globalDuration() * timeStretch;
//                  qDebug("%s %d + %d = %d\n", cr->name(), tick, f.ticks(), tick + f.ticks());
                  tick      += f.ticks();
                  lcr        = cr;
                  }
            }
      }

