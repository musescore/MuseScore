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
#include "tremolo.h"
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

//---------------------------------------------------------
//   resetSystems
//    in linear mode there is only one page
//    which contains one system
//---------------------------------------------------------

void Score::resetSystems(bool layoutAll, LayoutContext& lc)
      {
      Page* page = 0;
      if (layoutAll) {
            for (System* s : qAsConst(_systems)) {
                  for (SpannerSegment* ss : s->spannerSegments())
                        ss->setParent(0);
                  }
            qDeleteAll(_systems);
            _systems.clear();
            qDeleteAll(pages());
            pages().clear();
            if (!firstMeasure()) {
                  qDebug("no measures");
                  return;
                  }

            for (MeasureBase* mb = first(); mb; mb = mb->next())
                  mb->setSystem(0);

            page = new Page(this);
            pages().push_back(page);
            page->bbox().setRect(0.0, 0.0, loWidth(), loHeight());
            page->setNo(0);

            System* system = new System(this);
            _systems.push_back(system);
            page->appendSystem(system);
            system->adjustStavesNumber(nstaves());
            }
      else {
            if (pages().isEmpty())
                  return;
            page = pages().front();
            System* system = systems().front();
            system->clear();
            system->adjustStavesNumber(nstaves());
            }
      lc.page = page;
      }

//---------------------------------------------------------
//   collectLinearSystem
//   Append all measures to System. VBox is not included to System
//---------------------------------------------------------

 void Score::collectLinearSystem(LayoutContext& lc)
      {
      System* system = systems().front();
      system->setInstrumentNames(/* longNames */ true);

      QPointF pos;
      bool firstMeasure = true;     //lc.startTick.isZero();

      //set first measure to lc.nextMeasures for following
      //utilizing in getNextMeasure()
      lc.nextMeasure = _measures.first();
      lc.tick = Fraction(0, 1);
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
                        system->layoutSystem(pos.rx());
                        if (m->repeatStart()) {
                              Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0,1));
                              if (!s->enabled())
                                    s->setEnabled(true);
                              }
                        m->addSystemHeader(true);
                        pos.rx() += system->leftMargin();
                        firstMeasure = false;
                        }
                  else if (m->header())
                        m->removeSystemHeader();
                  if (m->trailer())
                        m->removeSystemTrailer();
                  if (m->tick() >= lc.startTick && m->tick() <= lc.endTick) {
                        // for measures in range, do full layout
                        m->createEndBarLines(false);
                        m->computeMinWidth();
                        ww = m->width();
                        m->stretchMeasure(ww);
                        }
                  else {
                        // for measures not in range, use existing layout
                        ww = m->width();
                        if (m->pos() != pos) {
                              // fix beam positions
                              // other elements with system as parent are processed in layoutSystemElements()
                              // but full beam processing is expensive and not needed if we adjust position here
                              QPointF p = pos - m->pos();
                              for (const Segment& s : m->segments()) {
                                    if (!s.isChordRestType())
                                          continue;
                                    for (int track = 0; track < ntracks(); ++track) {
                                          Element* e = s.element(track);
                                          if (e) {
                                                ChordRest* cr = toChordRest(e);
                                                if (cr->beam() && cr->beam()->elements().front() == cr)
                                                      cr->beam()->rpos() += p;
                                                }
                                          }
                                    }
                              }
                        }
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
      lc.score = this;
      resetSystems(layoutAll, lc);

      collectLinearSystem(lc);
//      hideEmptyStaves(systems().front(), true);     this does not make sense

      lc.layoutLinear();
      }

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void LayoutContext::layoutLinear()
      {
      System* system = score->systems().front();

      score->layoutSystemElements(system, *this);

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
                              if (m->tick() < startTick || m->tick() > endTick)
                                    continue;
                              if (!score->staff(track2staff(track))->show())
                                    continue;
                              ChordRest* cr = toChordRest(e);
                              if (notTopBeam(cr))                   // layout cross staff beams
                                    cr->beam()->layout();
                              if (notTopTuplet(cr)) {
                                    // fix layout of tuplets
                                    DurationElement* de = cr;
                                    while (de->tuplet() && de->tuplet()->elements().front() == de) {
                                          Tuplet* t = de->tuplet();
                                          t->layout();
                                          de = de->tuplet();
                                          }
                                    }

                              if (cr->isChord()) {
                                    Chord* c = toChord(cr);
                                    for (Chord* cc : c->graceNotes()) {
                                          if (cc->beam() && cc->beam()->elements().front() == cc)
                                                cc->beam()->layout();
                                          cc->layoutSpanners();
                                          for (Element* element : cc->el()) {
                                                if (element->isSlur())
                                                      element->layout();
                                                }
                                          }
                                    c->layoutArpeggio2();
                                    c->layoutSpanners();
                                    if (c->tremolo()) {
                                          Tremolo* t = c->tremolo();
                                          Chord* c1 = t->chord1();
                                          Chord* c2 = t->chord2();
                                          if (t->twoNotes() && c1 && c2 && (c1->staffMove() || c2->staffMove()))
                                                t->layout();
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
      // Set buffer space after the last system to avoid problems with mouse input.
      // Mouse input divides space between systems equally (see Score::searchSystem),
      // hence the choice of the value.
      const qreal buffer = 0.5 * score->styleS(Sid::maxSystemDistance).val() * score->spatium();
      page->setHeight(system->height() + system->pos().y() + buffer);
      page->rebuildBspTree();
      }

} // namespace Ms

