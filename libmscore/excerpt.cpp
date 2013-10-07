//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "excerpt.h"
#include "score.h"
#include "part.h"
#include "xml.h"
#include "staff.h"
#include "box.h"
#include "style.h"
#include "page.h"
#include "text.h"
#include "slur.h"
#include "tie.h"
#include "sig.h"
#include "tempo.h"
#include "measure.h"
#include "rest.h"
#include "stafftype.h"
#include "tuplet.h"
#include "chord.h"
#include "note.h"
#include "lyrics.h"
#include "segment.h"
#include "tupletmap.h"
#include "tiemap.h"
#include "layoutbreak.h"

namespace Ms {

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Excerpt::read(XmlReader& e)
      {
      const QList<Part*>& pl = _score->parts();
      QString name;
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "name")
                  name = e.readElementText();
            else if (tag == "title")
                  _title = e.readElementText().trimmed();
            else if (tag == "part") {
                  int partIdx = e.readInt();
                  if (partIdx < 0 || partIdx >= pl.size())
                        qDebug("Excerpt::read: bad part index");
                  else
                        _parts.append(pl.at(partIdx));
                  }
            }
      if (_title.isEmpty())
            _title = name.trimmed();
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool Excerpt::operator!=(const Excerpt& e) const
      {
      if (e._score != _score)
            return true;
      if (e._title != _title)
            return true;
      if (e._parts != _parts)
            return true;
      return false;
      }

//---------------------------------------------------------
//   localSetScore
//---------------------------------------------------------

static void localSetScore(void* score, Element* element)
      {
      element->setScore((Score*)score);
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

Score* createExcerpt(const QList<Part*>& parts)
      {
      if (parts.isEmpty())
            return 0;
      QList<int> srcStaves;

      Score* oscore = parts.front()->score();
      Score* score = new Score(oscore);

      // clone layer:
      for (int i = 0; i < 32; ++i) {
            score->layerTags()[i] = oscore->layerTags()[i];
            score->layerTagComments()[i] = oscore->layerTagComments()[i];
            }
      score->setCurrentLayer(oscore->currentLayer());
      score->layer().clear();
      foreach(const Layer& l, oscore->layer())
            score->layer().append(l);

      score->setPageNumberOffset(oscore->pageNumberOffset());

      foreach (Part* part, parts) {
            Part* p = new Part(score);
            p->setInstrument(*part->instr());
            int idx = 0;
            foreach(Staff* staff, *part->staves()) {
                  Staff* s = new Staff(score, p, idx);
                  s->setStaffType(staff->staffType());
                  s->setUpdateKeymap(true);
                  s->linkTo(staff);
                  p->staves()->append(s);
                  score->staves().append(s);
                  srcStaves.append(oscore->staffIdx(staff));
                  ++idx;
                  }
            score->appendPart(p);
            }
      cloneStaves(oscore, score, srcStaves);

      //
      // create excerpt title
      //
      MeasureBase* measure = score->first();
      if (!measure || (measure->type() != Element::VBOX)) {
            MeasureBase* nmeasure = new VBox(score);
            nmeasure->setTick(0);
            score->addMeasure(nmeasure, measure);
            measure = nmeasure;
            }
      Text* txt = new Text(score);
      txt->setTextStyleType(TEXT_STYLE_INSTRUMENT_EXCERPT);
      txt->setText(parts.front()->longName().toPlainText());
      measure->add(txt);

      //
      // layout score
      //
      score->setPlaylistDirty(true);
      score->rebuildMidiMapping();
      score->updateChannel();

      score->setLayoutAll(true);
      score->addLayoutFlags(LayoutFlags(LAYOUT_FIX_TICKS | LAYOUT_FIX_PITCH_VELO));
      score->doLayout();
      return score;
      }

//---------------------------------------------------------
//   cloneStaves
//---------------------------------------------------------

void cloneStaves(Score* oscore, Score* score, const QList<int>& map)
      {
      TieMap  tieMap;

      MeasureBaseList* nmbl = score->measures();
      for (MeasureBase* mb = oscore->measures()->first(); mb; mb = mb->next()) {
            MeasureBase* nmb = 0;
            if (mb->type() == Element::HBOX)
                  nmb = new HBox(score);
            else if (mb->type() == Element::VBOX)
                  nmb = new VBox(score);
            else if (mb->type() == Element::MEASURE) {
                  Measure* m  = static_cast<Measure*>(mb);
                  Measure* nm = new Measure(score);
                  nmb = nm;
                  nm->setTick(m->tick());
                  nm->setLen(m->len());
                  nm->setTimesig(m->timesig());
                  nm->setRepeatCount(m->repeatCount());
                  nm->setRepeatFlags(m->repeatFlags());
                  nm->setIrregular(m->irregular());
                  nm->setNo(m->no());
                  nm->setNoOffset(m->noOffset());
                  nm->setBreakMultiMeasureRest(m->breakMultiMeasureRest());
                  nm->setEndBarLineType(
                     m->endBarLineType(),
                     m->endBarLineGenerated(),
                     m->endBarLineVisible(),
                     m->endBarLineColor());

                  // Fraction ts = nm->len();
                  int tracks = oscore->nstaves() * VOICES;
                  for (int srcTrack = 0; srcTrack < tracks; ++srcTrack) {
                        TupletMap tupletMap;    // tuplets cannot cross measure boundaries
                        int track = -1;
                        int st = 0;
                        foreach(int staff, map) {
                              if (staff == srcTrack/VOICES) {
                                    track = (st * VOICES) + srcTrack % VOICES;
                                    break;
                                    }
                              ++st;
                              }
                        if (((srcTrack % VOICES) == 0) && track != -1) {
                              }
                        for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                              Segment* ns = nm->getSegment(oseg->segmentType(), oseg->tick());

                              foreach (Element* e, oseg->annotations()) {
                                    if (e->generated())
                                          continue;
                                    if ((e->track() == srcTrack && track != -1)
                                       || (e->systemFlag() && srcTrack == 0)
                                       ) {
                                          Element* ne = e->clone();
                                          ne->setUserOff(QPointF());  // reset user offset as most likely
                                                                      // it will not fit
                                          ne->setReadPos(QPointF());
                                          ne->setTrack(track == -1 ? 0 : track);
                                          ns->add(ne);
                                          }
                                    }

                              if (track == -1)
                                    continue;

                              Element* oe = oseg->element(srcTrack);
                              if (oe == 0)
                                    continue;
                              Element* ne;
                              if (oe->generated() || oe->type() == Element::CLEF)
                                    ne = oe->clone();
                              else
                                    ne = oe->linkedClone();
                              ne->setTrack(track);
                              ne->scanElements(score, localSetScore);   //necessary?
                              ne->setScore(score);
                              if (oe->isChordRest()) {
                                    ChordRest* ocr = static_cast<ChordRest*>(oe);
                                    ChordRest* ncr = static_cast<ChordRest*>(ne);
                                    Tuplet* ot     = ocr->tuplet();
                                    if (ot) {
                                          Tuplet* nt = tupletMap.findNew(ot);
                                          if (nt == 0) {
                                                nt = new Tuplet(*ot);
                                                nt->clear();
                                                nt->setTrack(track);
                                                nt->setScore(score);
                                                tupletMap.add(ot, nt);
                                                }
                                          nt->add(ncr);
                                          ncr->setTuplet(nt);
                                          }

                                    if (oe->type() == Element::CHORD) {
                                          Chord* och = static_cast<Chord*>(ocr);
                                          Chord* nch = static_cast<Chord*>(ncr);
                                          int n      = och->notes().size();
                                          for (int i = 0; i < n; ++i) {
                                                Note* on = och->notes().at(i);
                                                Note* nn = nch->notes().at(i);
                                                if (on->tieFor()) {
                                                      Tie* tie = new Tie(score);
                                                      nn->setTieFor(tie);
                                                      tie->setStartNote(nn);
                                                      tie->setTrack(nn->track());
                                                      tieMap.add(on->tieFor(), tie);
                                                      }
                                                if (on->tieBack()) {
                                                      Tie* tie = tieMap.findNew(on->tieBack());
                                                      if (tie) {
                                                            nn->setTieBack(tie);
                                                            tie->setEndNote(nn);
                                                            }
                                                      else {
                                                            qDebug("cloneStave: cannot find tie");
                                                            }
                                                      }
                                                }
                                          }
                                    }
                              ns->add(ne);
                              }
                        }
                  }
            foreach(Element* e, *mb->el()) {
                  if (e->type() == Element::LAYOUT_BREAK) {
                        LayoutBreak::LayoutBreakType st = static_cast<LayoutBreak*>(e)->layoutBreakType();
                        if (st == LayoutBreak::PAGE || st == LayoutBreak::LINE)
                              continue;
                        }
                  Element* ne = e->clone();
                  ne->setScore(score);
                  nmb->add(ne);
                  }
            nmbl->add(nmb);
            }

      int n = map.size();
      for (int dstStaffIdx = 0; dstStaffIdx < n; ++dstStaffIdx) {
            Staff* srcStaff = oscore->staff(map[dstStaffIdx]);
            Staff* dstStaff = score->staff(dstStaffIdx);
            if (srcStaff->primaryStaff()) {
                  int span = srcStaff->barLineSpan();
                  int sIdx = srcStaff->idx();
                  int eIdx = sIdx + span;
                  for (int staffIdx = sIdx; staffIdx < eIdx; ++staffIdx) {
                        if (!map.contains(staffIdx))
                             --span;
                        }
                  dstStaff->setBarLineSpan(span);
                  int idx = 0;
                  foreach(BracketItem bi, srcStaff->brackets()) {
                        dstStaff->setBracket(idx, bi._bracket);
                        dstStaff->setBracketSpan(idx, bi._bracketSpan);
                        }
                  }
            }

      for (auto i : oscore->spanner()) {
            Spanner* s = i.second;
            int staffIdx = s->staffIdx();
            int dstTrack = -1;
            int st = 0;
            //always export voltas to first staff in part
            if(s->type() == Element::VOLTA)
                  dstTrack = s->voice();
            else { //export other spanner if staffidx matches
                  for (int index : map) {
                        if (index == staffIdx) {
                              dstTrack = st * VOICES + s->voice();
                              break;
                              }
                        ++st;
                        }
                  }
            if (dstTrack == -1)
                  continue;
            Spanner* ns = static_cast<Spanner*>(s->linkedClone());
            ns->setScore(score);
            ns->setParent(0);
            ns->setTrack(dstTrack);
            score->addSpanner(ns);
            }
      }

//---------------------------------------------------------
//   cloneStaff
//    srcStaff and dstStaff are in the same score
//---------------------------------------------------------

void cloneStaff(Staff* srcStaff, Staff* dstStaff)
      {
      Score* score = srcStaff->score();

      TieMap tieMap;

      int srcStaffIdx = score->staffIdx(srcStaff);
      int dstStaffIdx = score->staffIdx(dstStaff);

      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            int sTrack = srcStaffIdx * VOICES;
            int eTrack = sTrack + VOICES;
            for (int srcTrack = sTrack; srcTrack < eTrack; ++srcTrack) {
                  TupletMap tupletMap;    // tuplets cannot cross measure boundaries
                  int dstTrack = dstStaffIdx * VOICES + (srcTrack - sTrack);
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        Element* oe = seg->element(srcTrack);
                        if (oe == 0 || oe->generated())
                              continue;
                        if (oe->type() == Element::TIMESIG)
                              continue;
                        Element* ne;
                        if (oe->type() == Element::CLEF)
                              ne = oe->clone();
                        else
                              ne = oe->linkedClone();
                        ne->setTrack(dstTrack);
                        ne->setParent(seg);
                        score->undoAddElement(ne);
                        if (oe->isChordRest()) {
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              ChordRest* ncr = static_cast<ChordRest*>(ne);
                              Tuplet* ot     = ocr->tuplet();
                              if (ot) {
                                    Tuplet* nt = tupletMap.findNew(ot);
                                    if (nt == 0) {
                                          nt = new Tuplet(*ot);
                                          nt->clear();
                                          nt->setTrack(dstTrack);
                                          nt->setParent(m);
                                          tupletMap.add(ot, nt);
                                          }
                                    ncr->setTuplet(nt);
                                    nt->add(ncr);
                                    }
                              foreach (Element* e, seg->annotations()) {
                                    if (e->generated() || e->systemFlag())
                                          continue;
                                    if (e->track() != srcTrack)
                                          continue;
                                    Element* ne = e->clone();
                                    ne->setTrack(dstTrack);
                                    ne->setParent(seg);
                                    score->undoAddElement(ne);
                                    }
                              if (oe->type() == Element::CHORD) {
                                    Chord* och = static_cast<Chord*>(ocr);
                                    Chord* nch = static_cast<Chord*>(ncr);
                                    int n = och->notes().size();
                                    for (int i = 0; i < n; ++i) {
                                          Note* on = och->notes().at(i);
                                          Note* nn = nch->notes().at(i);
                                          if (on->tieFor()) {
                                                Tie* tie = new Tie(score);
                                                nn->setTieFor(tie);
                                                tie->setStartNote(nn);
                                                tie->setTrack(nn->track());
                                                tieMap.add(on->tieFor(), tie);
                                                }
                                          if (on->tieBack()) {
                                                Tie* tie = tieMap.findNew(on->tieBack());
                                                if (tie) {
                                                      nn->setTieBack(tie);
                                                      tie->setEndNote(nn);
                                                      }
                                                else {
                                                      qDebug("cloneStave: cannot find tie");
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
                  }
            }
      }

}

