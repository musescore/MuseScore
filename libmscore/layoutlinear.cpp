//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "page.h"
#include "system.h"
#include "measure.h"
#include "layout.h"
#include "bracket.h"
#include "spanner.h"
#include "barline.h"
#include "tie.h"
#include "chord.h"
#include "staff.h"
#include "box.h"
#include "spacer.h"
#include "sym.h"
#include "systemdivider.h"
#include "tuplet.h"
#include "dynamic.h"
#include "stafflines.h"
#include "tempotext.h"
#include "hairpin.h"
#include "part.h"
#include "keysig.h"
#include "sig.h"
#include "breath.h"
#include "tempo.h"
#include "fermata.h"
#include "lyrics.h"

namespace Ms {

extern bool isTopBeam(ChordRest* cr);
extern bool notTopBeam(ChordRest* cr);
void layoutTies(Chord* ch, System* system, int stick);
void layoutDrumsetChord(Chord* c, const Drumset* drumset, StaffType* st, qreal spatium);

//---------------------------------------------------------
//   processLines
//---------------------------------------------------------

static void processLines(System* system, std::vector<Spanner*> lines, bool align)
      {
      std::vector<SpannerSegment*> segments;
      for (Spanner* sp : lines) {
            SpannerSegment* ss = sp->layoutSystem(system);     // create/layout spanner segment for this system
            if (ss->autoplace())
                  segments.push_back(ss);
            }

      if (align && segments.size() > 1) {
            qreal y = segments[0]->userOff().y();
            for (unsigned i = 1; i < segments.size(); ++i)
                  y = qMax(y, segments[i]->userOff().y());
            for (auto ss : segments)
                  ss->rUserYoffset() = y;
            }
      }

//---------------------------------------------------------
//   resetSystems
//    in linear mode there is only one page
//    which contains one system
//---------------------------------------------------------

 void Score::resetSystems(bool layoutAll, LayoutContext& lc)
      {
// if (layoutAll) {
      qDeleteAll(systems());
      systems().clear();
      qDeleteAll(pages());
      pages().clear();
      if (!firstMeasure())
            return;

      for (MeasureBase* mb = first(); mb; mb = mb->next())
            mb->setSystem(nullptr);

      auto page = new Page(this);
      pages().push_back(page);
      page->bbox().setRect(0.0, 0.0, loWidth(), loHeight());
      page->setNo(0);

      auto system = new System(this);
      systems().append(system);
      page->appendSystem(system);
      for (int i = 0; i < nstaves(); ++i)
            system->insertStaff(i);
//            }
//      else {
//            if (pages().isEmpty())
//                  return;
//            page = pages().front();
//            system = systems().front();
//            }

      lc.page = page;
      }

//---------------------------------------------------------
//   collectLinearSystem
//   Append all measures to System. VBox is not included to System
//---------------------------------------------------------

 void Score::collectLinearSystem(LayoutContext& lc)
      {
      System* system = systems().front();

      QPointF pos;
      bool firstMeasure = true;

      //set first measure to lc.nextMeasures for following
      //utilizing in getNextMeasure()
      lc.nextMeasure = _measures.first();
      getNextMeasure(lc);

      while (lc.curMeasure) {
            qreal ww = 0.0;
            if (lc.curMeasure->isVBox() || lc.curMeasure->isTBox()) {
                  lc.curMeasure->setParent(nullptr);
                  getNextMeasure(lc);
                  continue;
                  }
            system->appendMeasure(lc.curMeasure);
            if (lc.curMeasure->isMeasure()) {
                  Measure* m = toMeasure(lc.curMeasure);
                        if (m->mmRest()) {
                              m->mmRest()->setSystem(nullptr);
                              }
                  if (firstMeasure) {
                        system->layoutSystem(0.0);
                        if (m->repeatStart()) {
                              Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, 0);
                              if (!s->enabled())
                                    s->setEnabled(true);
                              }
                        m->addSystemHeader(true);
                        pos.rx() += system->leftMargin();
                        firstMeasure = false;
                        }
                  else if (m->header())
                        m->removeSystemHeader();
                  m->createEndBarLines(true);
                  m->computeMinWidth();
                  ww = m->width();
                  m->stretchMeasure(ww);
                  m->setPos(pos);
                  m->layoutStaffLines();
                  }
            else if (lc.curMeasure->isHBox()) {
                  lc.curMeasure->setPos(pos + QPointF(toHBox(lc.curMeasure)->topGap(), 0.0));
                  lc.curMeasure->layout();
                  ww = lc.curMeasure->width();
                  }
            pos.rx() += ww;

            getNextMeasure(lc);
            }

      system->setWidth(pos.x());
      }

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Score::layoutLinear(bool layoutAll, LayoutContext& lc)
      {
      resetSystems(layoutAll, lc);

      collectLinearSystem(lc);

      hideEmptyStaves(systems().front(), true);

      lc.layoutLinear();
      }

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void LayoutContext::layoutLinear()
      {
      System* system = score->systems().front();

      //
      // layout
      //    - beams
      //    - RehearsalMark, StaffText
      //    - Dynamic
      //    - update the segment shape + measure shape
      //
      //
      int stick = system->measures().front()->tick();
      int etick = system->measures().back()->endTick();

      //
      // layout slurs
      //
      if (etick > stick) {    // ignore vbox
            auto spanners = score->spannerMap().findOverlapping(stick, etick);

            std::vector<Spanner*> spanner;
            for (auto interval : spanners) {
                  Spanner* sp = interval.value;
                  if (sp->tick() < etick && sp->tick2() >= stick) {
                        if (sp->isSlur())
                              spanner.push_back(sp);
                        }
                  }
            processLines(system, spanner, false);
            }

      std::vector<Dynamic*> dynamics;
      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            SegmentType st = SegmentType::ChordRest;
            Measure* m = toMeasure(mb);
            for (Segment* s = m->first(st); s; s = s->next(st)) {
                  for (Element* e : s->elist()) {
                        if (!e)
                              continue;
                        if (e->isChordRest()) {
                              ChordRest* cr = toChordRest(e);
                              if (isTopBeam(cr))
                                    cr->beam()->layout();
                              if (e->isChord()) {
                                    Chord* c = toChord(e);
                                    for (Chord* ch : c->graceNotes())
                                          layoutTies(ch, system, stick);
                                    layoutTies(c, system, stick);
                                    c->layoutArticulations2();
                                    }
                              }
                        }
                  for (Element* e : s->annotations()) {
                        if (e->visible() && e->isDynamic()) {
                              Dynamic* d = toDynamic(e);
                              d->layout();
                              }
                        else if (e->isFiguredBass())
                              e->layout();
                        }
                  }
            }

      //
      // layout tuplet
      //

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            Measure* m = toMeasure(mb);
            static const SegmentType st { SegmentType::ChordRest };
            for (int track = 0; track < score->ntracks(); ++track) {
                  if (!score->staff(track / VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  for (Segment* s = m->first(st); s; s = s->next(st)) {
                        ChordRest* cr = s->cr(track);
                        if (!cr)
                              continue;
                        DurationElement* de = cr;
                        while (de->tuplet() && de->tuplet()->elements().front() == de) {
                              Tuplet* t = de->tuplet();
                              t->layout();
                              de = de->tuplet();
                              }
                        }
                  }
            }

      //
      //    layout SpannerSegments for current system
      //

      if (etick > stick) {    // ignore vbox
            auto spanners = score->spannerMap().findOverlapping(stick, etick);

            std::vector<Spanner*> ottavas;
            std::vector<Spanner*> spanner;
            std::vector<Spanner*> pedal;

            for (auto interval : spanners) {
                  Spanner* sp = interval.value;
                  if (sp->tick() < etick && sp->tick2() > stick) {
                        if (sp->isOttava())
                              ottavas.push_back(sp);
                        else if (sp->isPedal())
                              pedal.push_back(sp);
                        else if (!sp->isSlur())             // slurs are already handled
                              spanner.push_back(sp);
                        }
                  }
            processLines(system, ottavas, false);
            processLines(system, pedal, true);
            processLines(system, spanner, false);

            //
            // vertical align volta segments
            //
            std::vector<SpannerSegment*> voltaSegments;
            for (SpannerSegment* ss : system->spannerSegments()) {
                  if (ss->isVoltaSegment())
                       voltaSegments.push_back(ss);
                 }
            if (voltaSegments.size() > 1) {
                  qreal y = 0;
                  for (SpannerSegment* ss : voltaSegments)
                        y = qMin(y, ss->userOff().y());
                  for (SpannerSegment* ss : voltaSegments)
                        ss->setUserYoffset(y);
                  }
            for (Spanner* sp : score->unmanagedSpanners()) {
                  if (sp->tick() >= etick || sp->tick2() < stick)
                        continue;
                  sp->layout();
                  }
            }

      //
      // TempoText, Fermata
      //

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            SegmentType st = SegmentType::ChordRest;
            Measure* m = toMeasure(mb);
            for (Segment* s = m->first(st); s; s = s->next(st)) {
                  for (Element* e : s->annotations()) {
                        if (e->isTempoText()) {
                              TempoText* tt = toTempoText(e);
                              if (score->isMaster())
                                    score->setTempo(tt->segment(), tt->tempo());
                              tt->layout();
                              }
                        else if (e->isFermata())
                              e->layout();
                        }
                  }
            }

      //
      // Jump, Marker
      //

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            Measure* m = toMeasure(mb);
            for (Element* e : m->el()) {
                  if (e->visible() && (e->isJump() || e->isMarker()))
                        e->layout();
                  }
            }

      //
      // RehearsalMark, StaffText, FretDiagram
      //

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            SegmentType st = SegmentType::ChordRest;
            Measure* m = toMeasure(mb);
            for (Segment* s = m->first(st); s; s = s->next(st)) {
                  // layout in specific order
                  for (Element* e : s->annotations()) {
                        if (e->visible() && e->isFretDiagram())
                              e->layout();
                        }
                  for (Element* e : s->annotations()) {
                        if (e->visible() && (e->isStaffText() || e->isHarmony()))
                              e->layout();
                        }
                  for (Element* e : s->annotations()) {
                        if (e->visible() && e->isRehearsalMark())
                              e->layout();
                        }
                  }
            }

      score->layoutLyrics(system);

      system->layout2();   // compute staff distances

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            Measure* m = toMeasure(mb);

            for (int track = 0; track < score->ntracks(); ++track) {
                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        Element* e = segment->element(track);
                        if (!e)
                              continue;
                        if (e->isChordRest()) {
                              if (!score->staff(track2staff(track))->show())
                                    continue;
                              ChordRest* cr = toChordRest(e);
                              if (notTopBeam(cr))                   // layout cross staff beams
                                    cr->beam()->layout();

                              if (cr->isChord()) {
                                    Chord* c = toChord(cr);
                                    for (Chord* cc : c->graceNotes()) {
                                          if (cc->beam() && cc->beam()->elements().front() == cc)
                                                cc->beam()->layout();
                                          for (Note* n : cc->notes()) {
                                                Tie* tie = n->tieFor();
                                                if (tie)
                                                      tie->layout();
                                                for (Spanner* sp : n->spannerFor())
                                                      sp->layout();
                                                }
                                          for (Element* element : cc->el()) {
                                                if (element->isSlur())
                                                      element->layout();
                                                }
                                          }
                                    c->layoutArpeggio2();
                                    for (Note* n : c->notes()) {
                                          Tie* tie = n->tieFor();
                                          if (tie)
                                                tie->layout();
                                          for (Spanner* sp : n->spannerFor())
                                                sp->layout();
                                          }
                                    }
                              }
                        else if (e->isBarLine())
                              toBarLine(e)->layout2();
                        }
                  }
            m->layout2();
            }
      page->setPos(0, 0);
      system->setPos(page->lm(), page->tm() + score->styleP(Sid::staffUpperBorder));
      page->setWidth(system->width() + system->pos().x());
      page->rebuildBspTree();
      }

//---------------------------------------------------------
//   layoutMeasureLinear
//---------------------------------------------------------

void LayoutContext::layoutMeasureLinear(MeasureBase* mb)
      {
      adjustMeasureNo(mb);

      if (!mb->isMeasure()) {
            mb->setTick(tick);
            return;
            }

      //-----------------------------------------
      //    process one measure
      //-----------------------------------------

      Measure* measure = toMeasure(mb);
      measure->moveTicks(tick - measure->tick());

      //
      //  implement section break rest
      //
      if (measure->sectionBreak() && measure->pause() != 0.0)
            score->setPause(measure->endTick(), measure->pause());

      //
      // calculate accidentals and note lines,
      // create stem and set stem direction
      //
      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            Staff* staff           = score->staff(staffIdx);
            const Drumset* drumset = staff->part()->instrument()->useDrumset() ? staff->part()->instrument()->drumset() : 0;
            AccidentalState as;      // list of already set accidentals for this measure
            as.init(staff->keySigEvent(measure->tick()), staff->clef(measure->tick()));

            for (Segment& segment : measure->segments()) {
                  if (segment.isKeySigType()) {
                        KeySig* ks = toKeySig(segment.element(staffIdx * VOICES));
                        if (!ks)
                              continue;
                        int tick = segment.tick();
                        as.init(staff->keySigEvent(tick), staff->clef(tick));
                        ks->layout();
                        }
                  else if (segment.isChordRestType()) {
                        StaffType* st = staff->staffType(segment.tick());
                        int track     = staffIdx * VOICES;
                        int endTrack  = track + VOICES;

                        for (int t = track; t < endTrack; ++t) {
                              ChordRest* cr = segment.cr(t);
                              if (!cr)
                                    continue;
                              qreal m = staff->mag(segment.tick());
                              if (cr->small())
                                    m *= score->styleD(Sid::smallNoteMag);

                              if (cr->isChord()) {
                                    Chord* chord = toChord(cr);
                                    chord->cmdUpdateNotes(&as);
                                    for (Chord* c : chord->graceNotes()) {
                                          c->setMag(m * score->styleD(Sid::graceNoteMag));
                                          c->computeUp();
                                          if (c->stemDirection() != Direction::AUTO)
                                                c->setUp(c->stemDirection() == Direction::UP);
                                          else
                                                c->setUp(!(t % 2));
                                          if (drumset)
                                                layoutDrumsetChord(c, drumset, st, score->spatium());
                                          c->layoutStem1();
                                          }
                                    if (drumset)
                                          layoutDrumsetChord(chord, drumset, st, score->spatium());
                                    chord->computeUp();
                                    chord->layoutStem1();   // create stems needed to calculate spacing
                                                            // stem direction can change later during beam processing
                                    }
                              cr->setMag(m);
                              }
                        }
                  else if (segment.isClefType()) {
                        Element* e = segment.element(staffIdx * VOICES);
                        if (e) {
                              toClef(e)->setSmall(true);
                              e->layout();
                              }
                        }
                  else if (segment.isType(SegmentType::TimeSig | SegmentType::Ambitus | SegmentType::HeaderClef)) {
                        Element* e = segment.element(staffIdx * VOICES);
                        if (e)
                              e->layout();
                        }
                  }
            }

      score->createBeams(measure);

      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            for (Segment& segment : measure->segments()) {
                  if (segment.isChordRestType()) {
                        score->layoutChords1(&segment, staffIdx);
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = segment.cr(staffIdx * VOICES + voice);
                              if (cr) {
                                    for (Lyrics* l : cr->lyrics()) {
                                          if (l)
                                                l->layout();
                                          }
                                    if (cr->isChord())
                                          toChord(cr)->layoutArticulations();
                                    }
                              }
                        }
                  }
            }

      for (Segment& segment : measure->segments()) {
            if (segment.isBreathType()) {
                  qreal length = 0.0;
                  int tick = segment.tick();
                  // find longest pause
                  for (int i = 0, n = score->ntracks(); i < n; ++i) {
                        Element* e = segment.element(i);
                        if (e && e->isBreath()) {
                              Breath* b = toBreath(e);
                              b->layout();
                              length = qMax(length, b->pause());
                              }
                        }
                  if (length != 0.0)
                        score->setPause(tick, length);
                  }
            else if (segment.isTimeSigType()) {
                  for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                        TimeSig* ts = toTimeSig(segment.element(staffIdx * VOICES));
                        if (ts)
                              score->staff(staffIdx)->addTimeSig(ts);
                        }
                  }
            else if (score->isMaster() && segment.isChordRestType()) {
                  for (Element* e : segment.annotations()) {
                        if (!(e->isTempoText()
                           || e->isDynamic()
                           || e->isFermata()
                           || e->isRehearsalMark()
                           || e->isFretDiagram()
                           || e->isHarmony()
                           || e->isStaffText()              // ws: whats left?
                           || e->isFiguredBass())) {
                              e->layout();
                              }
                        }
                  // TODO, this is not going to work, we just cleaned the tempomap
                  // it breaks the test midi/testBaroqueOrnaments.mscx where first note has stretch 2
                  // Also see fixTicks
                  qreal stretch = 0.0;
                  for (Element* e : segment.annotations()) {
                        if (e->isFermata())
                              stretch = qMax(stretch, toFermata(e)->timeStretch());
                        }
                  if (stretch != 0.0 && stretch != 1.0) {
                        qreal otempo = score->tempomap()->tempo(segment.tick());
                        qreal ntempo = otempo / stretch;
                        score->setTempo(segment.tick(), ntempo);
                        int etick = segment.tick() + segment.ticks() - 1;
                        auto e = score->tempomap()->find(etick);
                        if (e == score->tempomap()->end())
                              score->setTempo(etick, otempo);
                        }
                  }
            else if (segment.isChordRestType()) {
                  // chord symbols need to be layouted in parts too
                  for (Element* e : segment.annotations()) {
                        if (e->isHarmony())
                              e->layout();
                        }
                  }
            }

      // update time signature map
      // create event if measure len and time signature are different
      // even if they are equivalent 4/4 vs 2/2
      // also check if nominal time signature has changed

      if (score->isMaster() && ((!measure->len().identical(sig) && measure->len() != sig * measure->mmRestCount())
         || (prevMeasure && prevMeasure->isMeasure()
         && !measure->timesig().identical(toMeasure(prevMeasure)->timesig()))))
            {
            if (measure->isMMRest())
                  sig = measure->mmRestFirst()->len();
            else
                  sig = measure->len();
            score->sigmap()->add(tick, SigEvent(sig, measure->timesig(), measure->no()));
            }

      Segment* seg = measure->findSegmentR(SegmentType::StartRepeatBarLine, 0);
      if (measure->repeatStart()) {
            if (!seg)
                  seg = measure->getSegmentR(SegmentType::StartRepeatBarLine, 0);
            measure->barLinesSetSpan(seg);      // this also creates necessary barlines
            for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                  BarLine* b = toBarLine(seg->element(staffIdx * VOICES));
                  if (b) {
                        b->setBarLineType(BarLineType::START_REPEAT);
                        b->layout();
                        }
                  }
            }
      else if (seg)
            score->undoRemoveElement(seg);

      for (Segment& s : measure->segments()) {
            // DEBUG: relayout grace notes as beaming/flags may have changed
            if (s.isChordRestType()) {
                  for (Element* e : s.elist()) {
                        if (e && e->isChord()) {
                              Chord* chord = toChord(e);
                              chord->layout();
                              if (chord->tremolo())            // debug
                                    chord->tremolo()->layout();
                              }
                        }
                  }
            else if (s.isEndBarLineType())
                  continue;
            s.createShapes();
            }

      tick += measure->ticks();
      }

} // namespace Ms

