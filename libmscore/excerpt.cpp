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
#include "textframe.h"
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
#include "harmony.h"
#include "beam.h"
#include "utils.h"
#include "tremolo.h"
#include "barline.h"
#include "undo.h"

namespace Ms {

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Excerpt::read(XmlReader& e)
      {
      const QList<Part*>& pl = _oscore->parts();
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
      if (e._oscore != _oscore)
            return true;
      if (e._title != _title)
            return true;
      if (e._parts != _parts)
            return true;
      return false;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Excerpt::operator==(const Excerpt& e) const
      {
      if (e._oscore != _oscore)
            return false;
      if (e._title != _title)
            return false;
      if (e._parts != _parts)
            return false;
      return true;
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

void createExcerpt(Excerpt* excerpt)
      {
      MasterScore* oscore = excerpt->oscore();
      Score* score        = excerpt->partScore();

      QList<Part*>& parts = excerpt->parts();
      QList<int> srcStaves;

      // clone layer:
      for (int i = 0; i < 32; ++i) {
            score->layerTags()[i] = oscore->layerTags()[i];
            score->layerTagComments()[i] = oscore->layerTagComments()[i];
            }
      score->setCurrentLayer(oscore->currentLayer());
      score->layer().clear();
      foreach (const Layer& l, oscore->layer())
            score->layer().append(l);

      score->setPageNumberOffset(oscore->pageNumberOffset());

      foreach (Part* part, parts) {
            Part* p = new Part(score);
            p->setInstrument(*part->instrument());
            p->setPartName(part->partName());

            foreach (Staff* staff, *part->staves()) {
                  Staff* s = new Staff(score);
                  s->setPart(p);
                  s->setStaffType(staff->staffType());
                  s->setDefaultClefType(staff->defaultClefType());
                  score->undo(new LinkStaff(s, staff));
                  p->staves()->append(s);
                  score->staves().append(s);
                  srcStaves.append(oscore->staffIdx(staff));
                  }
            score->appendPart(p);
            }
      cloneStaves(oscore, score, srcStaves);

      //
      // create excerpt title
      //
      MeasureBase* measure = oscore->first();

      // create title frame for all scores if not already there
      if (!measure || (measure->type() != Element::Type::VBOX))
            measure = oscore->insertMeasure(Element::Type::VBOX, measure);

      VBox* titleFrameScore = static_cast<VBox*>(measure);
      measure = score->first();

      Q_ASSERT(measure->type() == Element::Type::VBOX);

      VBox* titleFramePart = static_cast<VBox*>(measure);
      titleFramePart->copyValues(titleFrameScore);
      QString partLabel = excerpt->title();     // parts.front()->longName();
      if (!partLabel.isEmpty()) {
            Text* txt = new Text(score);
            txt->setTextStyleType(TextStyleType::INSTRUMENT_EXCERPT);
            txt->setPlainText(partLabel);
            txt->setTrack(0);
            measure->add(txt);
            score->setMetaTag("partName", partLabel);
            }

      //
      // layout score
      //
      score->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
      score->doLayout();
      //
      // handle transposing instruments
      //
      if (oscore->styleB(StyleIdx::concertPitch) != score->styleB(StyleIdx::concertPitch)) {
            for (Staff* staff : score->staves()) {
                  if (staff->staffType()->group() == StaffGroup::PERCUSSION)
                        continue;
                  // if this staff has no transposition, and no instrument changes, we can skip it
                  Interval interval = staff->part()->instrument()->transpose();
                  if (interval.isZero() && staff->part()->instruments()->size() == 1)
                        continue;
                  bool flip = false;
                  if (oscore->styleB(StyleIdx::concertPitch)) {
                        interval.flip();  // flip the transposition for the original instrument
                        flip = true;      // transposeKeys() will flip transposition for each instrument change
                        }

                  int staffIdx   = staff->idx();
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;

                  int endTick = 0;
                  if (score->lastSegment())
                        endTick = score->lastSegment()->tick();
                  score->transposeKeys(staffIdx, staffIdx+1, 0, endTick, interval, true, flip);

                  for (auto segment = score->firstSegment(Segment::Type::ChordRest); segment; segment = segment->next1(Segment::Type::ChordRest)) {
                        Interval interval = staff->part()->instrument(segment->tick())->transpose();
                        if (interval.isZero())
                              continue;
                        if (oscore->styleB(StyleIdx::concertPitch))
                              interval.flip();

                        for (auto e : segment->annotations()) {
                              if ((e->type() != Element::Type::HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                                    continue;
                              Harmony* h  = static_cast<Harmony*>(e);
                              int rootTpc = Ms::transposeTpc(h->rootTpc(), interval, true);
                              int baseTpc = Ms::transposeTpc(h->baseTpc(), interval, true);
                              score->undoTransposeHarmony(h, rootTpc, baseTpc);
                              }
                        }
                  }
            }

      //
      // layout score
      //
      score->setPlaylistDirty();
      oscore->rebuildMidiMapping();
      oscore->updateChannel();

      score->setLayoutAll();
      score->doLayout();
      }

void deleteExcerpt(Excerpt* excerpt)
      {
      MasterScore* oscore = excerpt->oscore();
      Score* partScore    = excerpt->partScore();

      if (!partScore) {
            qDebug("deleteExcerpt: no partScore");
            return;
            }

      // unlink the staves in the excerpt
      for (Staff* s : partScore->staves()) {
            Staff* staff = nullptr;
            // find staff in the main score
            if (s->linkedStaves()) {
                  for (Staff* s2 : s->linkedStaves()->staves()) {
                        if ((s2->score() == oscore) && s2->primaryStaff()) {
                              staff = s2;
                              break;
                              }
                        }
                  }
            if (staff) {
                  int staffIdx = partScore->staffIdx(s);
                  // unlink the spanners
                  for (auto i = partScore->spanner().begin(); i != partScore->spanner().cend(); ++i) {
                        Spanner* s = i->second;
                        if (s->staffIdx() == staffIdx)
                              s->undoUnlink();
                        }
                  int sTrack = staffIdx * VOICES;
                  int eTrack = sTrack + VOICES;
                  // unlink elements and annotation
                  for (Segment* s = partScore->firstSegmentMM(); s; s = s->next1MM()) {
                        for (int track = eTrack - 1; track >= sTrack; --track) {
                              Element* el = s->element(track);
                              if (el)
                                    el->undoUnlink();
                              }
                        for (Element* e : s->annotations()) {
                              if (e->staffIdx() == staffIdx)
                                    e->undoUnlink();
                              }
                        }
                  // unlink the staff
                  oscore->undo(new UnlinkStaff(staff, s));
                  }
            }
      }

//---------------------------------------------------------
//   cloneSpanner
//---------------------------------------------------------

static void cloneSpanner(Spanner* s, Score* score, int dstTrack, int dstTrack2)
      {
      // dont clone voltas for track != 0
      if (s->type() == Element::Type::VOLTA && s->track() != 0)
            return;
      Spanner* ns = static_cast<Spanner*>(s->linkedClone());
      ns->setScore(score);
      ns->setParent(0);
      ns->setTrack(dstTrack);
      ns->setTrack2(dstTrack2);
      if (ns->type() == Element::Type::SLUR) {
            //
            // set start/end element for slur
            //
            ChordRest* cr1 = s->startCR();
            ChordRest* cr2 = s->endCR();

            ns->setStartElement(0);
            ns->setEndElement(0);
            if (cr1 && cr1->links()) {
                  for (ScoreElement* e : *cr1->links()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr == cr1)
                              continue;
                        if ((cr->score() == score) && (cr->tick() == ns->tick()) && cr->track() == dstTrack) {
                              ns->setStartElement(cr);
                              break;
                              }
                        }
                  }
            if (cr2 && cr2->links()) {
                  for (ScoreElement* e : *cr2->links()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr == cr2)
                              continue;
                        if ((cr->score() == score) && (cr->tick() == ns->tick2()) && cr->track() == dstTrack2) {
                              ns->setEndElement(cr);
                              break;
                              }
                        }
                  }
            if (!ns->startElement())
                  qDebug("clone Slur: no start element");
            if (!ns->endElement())
                  qDebug("clone Slur: no end element");
            }
      score->undo(new AddElement(ns));
      }

//---------------------------------------------------------
//   cloneTuplets
//---------------------------------------------------------

static void cloneTuplets(ChordRest* ocr, ChordRest* ncr, Tuplet* ot, TupletMap& tupletMap, Measure* m, int track)
      {
      ot->setTrack(ocr->track());
      Tuplet* nt = tupletMap.findNew(ot);
      if (nt == 0) {
            nt = static_cast<Tuplet*>(ot->linkedClone());
            nt->setTrack(track);
            nt->setParent(m);
            tupletMap.add(ot, nt);

            Tuplet* nt1 = nt;
            while (ot->tuplet()) {
                  Tuplet* nt = tupletMap.findNew(ot->tuplet());
                  if (nt == 0) {
                        nt = static_cast<Tuplet*>(ot->tuplet()->linkedClone());
                        nt->setTrack(track);
                        nt->setParent(m);
                        tupletMap.add(ot->tuplet(), nt);
                        }
                  nt->add(nt1);
                  nt1->setTuplet(nt);
                  ot = ot->tuplet();
                  nt1 = nt;
                  }
            }
      nt->add(ncr);
      ncr->setTuplet(nt);
      }

//---------------------------------------------------------
//   mapTrack
//---------------------------------------------------------

static int mapTrack(int srcTrack, const QList<int>& map)
      {
      if (srcTrack == -1)
            return -1;
      int track = -1;
      int st = 0;
      foreach(int staff, map) {
            if (staff == srcTrack / VOICES) {
                  track = (st * VOICES) + srcTrack % VOICES;
                  break;
                  }
            ++st;
            }
      return track;
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
            if (mb->type() == Element::Type::HBOX)
                  nmb = new HBox(score);
            else if (mb->type() == Element::Type::VBOX)
                  nmb = new VBox(score);
            else if (mb->type() == Element::Type::TBOX) {
                  nmb = new TBox(score);
                  Text* text = static_cast<TBox*>(mb)->text();
                  Element* ne = text->linkedClone();
                  ne->setScore(score);
                  nmb->add(ne);
                  }
            else if (mb->type() == Element::Type::MEASURE) {
                  Measure* m  = static_cast<Measure*>(mb);
                  Measure* nm = new Measure(score);
                  nmb = nm;
                  nm->setTick(m->tick());
                  nm->setLen(m->len());
                  nm->setTimesig(m->timesig());

                  nm->setRepeatCount(m->repeatCount());
                  nm->setRepeatStart(m->repeatStart());
                  nm->setRepeatEnd(m->repeatEnd());
                  nm->setRepeatMeasure(m->repeatMeasure());
                  nm->setRepeatJump(m->repeatJump());

                  nm->setIrregular(m->irregular());
                  nm->setNo(m->no());
                  nm->setNoOffset(m->noOffset());
                  nm->setBreakMultiMeasureRest(m->breakMultiMeasureRest());
//TODO                  nm->setEndBarLineType(
//                     m->endBarLineType(),
//                     m->endBarLineGenerated(),
//                     m->endBarLineVisible(),
//                     m->endBarLineColor());

                  // Fraction ts = nm->len();
                  int tracks = oscore->nstaves() * VOICES;
                  for (int srcTrack = 0; srcTrack < tracks; ++srcTrack) {
                        TupletMap tupletMap;    // tuplets cannot cross measure boundaries

                        int track = mapTrack(srcTrack, map);

                        Tremolo* tremolo = 0;
                        for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                              Segment* ns = nullptr; //create segment later, on demand
                              foreach (Element* e, oseg->annotations()) {
                                    if (e->generated())
                                          continue;
                                    if ((e->track() == srcTrack && track != -1)
                                       || (e->systemFlag() && srcTrack == 0)
                                       ) {
                                          Element* ne = e->linkedClone();
                                          ne->setUserOff(QPointF());  // reset user offset as most likely
                                                                      // it will not fit
                                          ne->setReadPos(QPointF());
                                          ne->setTrack(track == -1 ? 0 : track);
                                          ne->setScore(score);
                                          if (!ns)
                                                ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                                          ns->add(ne);
                                          // for chord symbols,
                                          // re-render with new style settings
                                          if (ne->type() == Element::Type::HARMONY) {
                                                Harmony* h = static_cast<Harmony*>(ne);
                                                h->render();
                                                }
                                          }
                                    }

                              if (track == -1)
                                    continue;

                              Element* oe = oseg->element(srcTrack);
                              int adjustedBarlineSpan = 0;
                              if (srcTrack % VOICES == 0 && oseg->segmentType() == Segment::Type::BarLine) {
                                    // mid-measure barline segment
                                    // may need to clone barline from a previous staff and/or adjust span
                                    int oIdx = srcTrack / VOICES;
                                    if (!oe) {
                                          // no barline on this staff in original score,
                                          // but check previous staves
                                          for (int i = oIdx - 1; i >= 0; --i) {
                                                oe = oseg->element(i * VOICES);
                                                if (oe)
                                                      break;
                                                }
                                          }
                                    if (oe) {
                                          // barline found, now check span
                                          BarLine* bl = static_cast<BarLine*>(oe);
                                          int oSpan1 = bl->staff()->idx();
                                          int oSpan2 = oSpan1 + bl->span();
                                          if (oSpan1 <= oIdx && oIdx < oSpan2) {
                                                // this staff is within span
                                                // calculate adjusted span for excerpt
                                                int oSpan = oSpan2 - oIdx;
                                                adjustedBarlineSpan = qMin(oSpan, score->nstaves());
                                                }
                                          else {
                                                // this staff is not within span
                                                oe = nullptr;
                                                }
                                          }
                                    }
                              if (oe == 0)
                                    continue;
                              Element* ne;
                              if (oe->generated())
                                    continue;
                              else
                                    ne = oe->linkedClone();
                              ne->setTrack(track);
                              ne->scanElements(score, localSetScore);   //necessary?
                              ne->setScore(score);
                              if (oe->type() == Element::Type::BAR_LINE && adjustedBarlineSpan) {
                                    BarLine* nbl = static_cast<BarLine*>(ne);
                                    nbl->setSpan(adjustedBarlineSpan);
                                    }
                              else if (oe->isChordRest()) {
                                    ChordRest* ocr = static_cast<ChordRest*>(oe);
                                    ChordRest* ncr = static_cast<ChordRest*>(ne);

                                    if (ocr->beam() && !ocr->beam()->empty() && ocr->beam()->elements().front() == ocr) {
                                          Beam* nb = ocr->beam()->clone();
                                          nb->clear();
                                          nb->setTrack(track);
                                          nb->setScore(score);
                                          nb->add(ncr);
                                          ncr->setBeam(nb);
                                          }

                                    Tuplet* ot = ocr->tuplet();

                                    if (ot)
                                          cloneTuplets(ocr, ncr, ot, tupletMap, m, track);

                                    if (oe->type() == Element::Type::CHORD) {
                                          Chord* och = static_cast<Chord*>(ocr);
                                          Chord* nch = static_cast<Chord*>(ncr);

                                          int n = och->notes().size();
                                          for (int i = 0; i < n; ++i) {
                                                Note* on = och->notes().at(i);
                                                Note* nn = nch->notes().at(i);
                                                if (on->tieFor()) {
                                                      Tie* tie = static_cast<Tie*>(on->tieFor()->linkedClone());
                                                      tie->setScore(score);
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
                                                            qDebug("cloneStaves: cannot find tie");
                                                            }
                                                      }
                                                // add back spanners (going back from end to start spanner element
                                                // makes sure the 'other' spanner anchor element is already set up)
                                                // 'on' is the old spanner end note and 'nn' is the new spanner end note
                                                for (Spanner* oldSp : on->spannerBack()) {
                                                      Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                                      if (newStart != nullptr) {
                                                            Spanner* newSp = static_cast<Spanner*>(oldSp->linkedClone());
                                                            newSp->setNoteSpan(newStart, nn);
                                                            score->addElement(newSp);
                                                            }
                                                      else {
                                                            qDebug("cloneStaves: cannot find spanner start note");
                                                            }
                                                      }
                                                }
                                          // two note tremolo
                                          if (och->tremolo() && och->tremolo()->twoNotes()) {
                                               if (och == och->tremolo()->chord1()) {
                                                      if (tremolo)
                                                            qDebug("unconnected two note tremolo");
                                                      tremolo = static_cast<Tremolo*>(och->tremolo()->linkedClone());
                                                      tremolo->setScore(nch->score());
                                                      tremolo->setParent(nch);
                                                      tremolo->setTrack(nch->track());
                                                      tremolo->setChords(nch, 0);
                                                      nch->setTremolo(tremolo);
                                                      }
                                                else if (och == och->tremolo()->chord2()) {
                                                      if (!tremolo)
                                                            qDebug("first note for two note tremolo missing");
                                                      else {
                                                            tremolo->setChords(tremolo->chord1(), nch);
                                                            nch->setTremolo(tremolo);
                                                            }
                                                      }
                                                else
                                                      qDebug("inconsistent two note tremolo");
                                                }
                                          }
                                    }
                              if (!ns)
                                    ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                              ns->add(ne);
                              }
                        }
                  }

            nmb->linkTo(mb);
            foreach (Element* e, mb->el()) {
                  if (e->type() == Element::Type::LAYOUT_BREAK) {
                        LayoutBreak::Type st = static_cast<LayoutBreak*>(e)->layoutBreakType();
                        if (st == LayoutBreak::Type::PAGE || st == LayoutBreak::Type::LINE)
                              continue;
                        }
                  int track = -1;
                  if (e->track() != -1) {
                        // try to map track
                        track = mapTrack(e->track(), map);
                        if (track == -1) {
                              // even if track not in excerpt, we need to clone system elements
                              if (e->systemFlag())
                                    track = 0;
                              else
                                    continue;
                              }
                        }

                  Element* ne;
                  // link text - title, subtitle, also repeats (eg, coda/segno)
                  // measure numbers are not stored in this list, but they should not be cloned anyhow
                  // layout breaks other than section were skipped above,
                  // but section breaks do need to be cloned & linked
                  // other measure-attached elements (?) are cloned but not linked
                  if (e->isText() || e->type() == Element::Type::LAYOUT_BREAK)
                        ne = e->linkedClone();
                  else
                        ne = e->clone();
                  ne->setScore(score);
                  ne->setTrack(track);
                  nmb->add(ne);
                  }
            nmbl->add(nmb);
            }

      int n = map.size();
      for (int dstStaffIdx = 0; dstStaffIdx < n; ++dstStaffIdx) {
            Staff* srcStaff = oscore->staff(map[dstStaffIdx]);
            Staff* dstStaff = score->staff(dstStaffIdx);
//            *dstStaff->clefList() = *srcStaff->clefList();
            if (srcStaff->primaryStaff()) {
                  int span = srcStaff->barLineSpan();
                  int sIdx = srcStaff->idx();
                  if (dstStaffIdx == 0 && span == 0) {
                        // this is first staff of new score,
                        // but it was somewhere within a barline span in the old score
                        // so, find beginning of span
                        for (int i = 0; i <= sIdx; ++i) {
                              span = oscore->staff(i)->barLineSpan();
                              if (i + span > sIdx) {
                                    sIdx = i;
                                    break;
                                    }
                              }
                        }
                  int eIdx = sIdx + span;
                  for (int staffIdx = sIdx; staffIdx < eIdx; ++staffIdx) {
                        if (!map.contains(staffIdx))
                             --span;
                        }
                  if (dstStaffIdx + span > n)
                        span = n - dstStaffIdx - 1;
                  dstStaff->setBarLineSpan(span);
                  int idx = 0;
                  foreach(BracketItem bi, srcStaff->brackets()) {
                        dstStaff->setBracket(idx, bi._bracket);
                        dstStaff->setBracketSpan(idx, bi._bracketSpan);
                        }
                  }
            }

      for (auto i : oscore->spanner()) {
            Spanner* s    = i.second;
            int staffIdx  = s->staffIdx();
            int dstTrack  = -1;
            int dstTrack2 = -1;
            int           st = 0;
            if (s->type() == Element::Type::VOLTA) {
                  //always export voltas to first staff in part
                  dstTrack = s->voice();
                  }
            else {            //export other spanner if staffidx matches
                  for (int index : map) {
                        if (index == staffIdx) {
                              dstTrack  = st * VOICES + s->voice();
                              dstTrack2 = st * VOICES + (s->track2() % VOICES);
                              break;
                              }
                        ++st;
                        }
                  }
            if (dstTrack == -1)
                  continue;

            cloneSpanner(s, score, dstTrack, dstTrack2);
            }
      }

//---------------------------------------------------------
//   cloneStaff
//    staves are in same score
//---------------------------------------------------------

void cloneStaff(Staff* srcStaff, Staff* dstStaff)
      {
      Score* score = srcStaff->score();
      TieMap tieMap;

      score->undo(new LinkStaff(srcStaff, dstStaff));

      int srcStaffIdx = score->staffIdx(srcStaff);
      int dstStaffIdx = score->staffIdx(dstStaff);

      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            int sTrack = srcStaffIdx * VOICES;
            int eTrack = sTrack + VOICES;
            for (int srcTrack = sTrack; srcTrack < eTrack; ++srcTrack) {
                  TupletMap tupletMap;    // tuplets cannot cross measure boundaries
                  int dstTrack = dstStaffIdx * VOICES + (srcTrack - sTrack);
                  Tremolo* tremolo = 0;
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        Element* oe = seg->element(srcTrack);
                        if (oe == 0 || oe->generated())
                              continue;
                        if (oe->type() == Element::Type::TIMESIG)
                              continue;
                        Element* ne = nullptr;
                        if (oe->type() == Element::Type::CLEF) {
                              // only clone clef if it matches staff group and does not exists yet
                              Clef* clef = static_cast<Clef*>(oe);
                              int   tick = seg->tick();
                              if (ClefInfo::staffGroup(clef->concertClef()) == dstStaff->staffGroup()
                                          && dstStaff->clefType(tick) != clef->clefTypeList()) {
                                    ne = oe->clone();
                                    }
                              }
                        else
                              ne = oe->linkedClone();
                        if (ne) {
                              ne->setTrack(dstTrack);
                              ne->setParent(seg);
                              ne->setScore(score);
                              if (ne->isChordRest()) {
                                    ChordRest* ncr = static_cast<ChordRest*>(ne);
                                    if (ncr->tuplet()) {
                                          ncr->setTuplet(0); //TODO nested tuplets
                                          }
                                    }
                              score->undoAddElement(ne);
                              }
                        if (oe->isChordRest()) {
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              ChordRest* ncr = static_cast<ChordRest*>(ne);
                              Tuplet* ot     = ocr->tuplet();
                              if (ot)
                                    cloneTuplets(ocr, ncr, ot, tupletMap, m, dstTrack);

                              // remove lyrics from chord
                              // since only one set of lyrics is used with linked staves
                              foreach (Lyrics* l, ncr->lyricsList()) {
                                    if (l)
                                          l->unlink();
                                    }
                              qDeleteAll(ncr->lyricsList());
                              ncr->lyricsList().clear();

                              foreach (Element* e, seg->annotations()) {
                                    if (e->generated() || e->systemFlag())
                                          continue;
                                    if (e->track() != srcTrack)
                                          continue;
                                    switch (e->type()) {
                                          // exclude certain element types
                                          // this should be same list excluded in Score::undoAddElement()
                                          case Element::Type::STAFF_TEXT:
                                          case Element::Type::FRET_DIAGRAM:
                                          case Element::Type::HARMONY:
                                          case Element::Type::FIGURED_BASS:
                                          case Element::Type::DYNAMIC:
                                          case Element::Type::LYRICS:   // not normally segment-attached
                                                continue;
                                          default:
                                                Element* ne = e->clone();
                                                ne->setTrack(dstTrack);
                                                ne->setParent(seg);
                                                ne->setScore(score);
                                                score->undoAddElement(ne);
                                          }
                                    }
                              if (oe->type() == Element::Type::CHORD) {
                                    Chord* och = static_cast<Chord*>(ocr);
                                    Chord* nch = static_cast<Chord*>(ncr);
                                    int n = och->notes().size();
                                    for (int i = 0; i < n; ++i) {
                                          Note* on = och->notes().at(i);
                                          Note* nn = nch->notes().at(i);
                                          if (on->tieFor()) {
                                                Tie* tie = static_cast<Tie*>(on->tieFor()->linkedClone());
                                                tie->setScore(score);
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
                                          // add back spanners (going back from end to start spanner element
                                          // makes sure the 'other' spanner anchor element is already set up)
                                          // 'on' is the old spanner end note and 'nn' is the new spanner end note
                                          for (Spanner* oldSp : on->spannerBack()) {
                                                Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                                if (newStart != nullptr) {
                                                      Spanner* newSp = static_cast<Spanner*>(oldSp->linkedClone());
                                                      newSp->setNoteSpan(newStart, nn);
                                                      score->addElement(newSp);
                                                      }
                                                else {
                                                      qDebug("cloneStave: cannot find spanner start note");
                                                      }
                                                }
                                          }
                                    // two note tremolo
                                    if (och->tremolo() && och->tremolo()->twoNotes()) {
                                          if (och == och->tremolo()->chord1()) {
                                                if (tremolo)
                                                      qDebug("unconnected two note tremolo");
                                                tremolo = static_cast<Tremolo*>(och->tremolo()->linkedClone());
                                                tremolo->setScore(nch->score());
                                                tremolo->setParent(nch);
                                                tremolo->setTrack(nch->track());
                                                tremolo->setChords(nch, 0);
                                                nch->setTremolo(tremolo);
                                                }
                                          else if (och == och->tremolo()->chord2()) {
                                                if (!tremolo)
                                                      qDebug("first note for two note tremolo missing");
                                                else {
                                                      tremolo->setChords(tremolo->chord1(), nch);
                                                      nch->setTremolo(tremolo);
                                                      }
                                                }
                                          else
                                                qDebug("inconsistent two note tremolo");
                                          }
                                    }
                              }
                        }
                  }
            }

      for (auto i : score->spanner()) {
            Spanner* s = i.second;
            int staffIdx = s->staffIdx();
            int dstTrack = -1;
            int dstTrack2 = -1;
            if (s->type() != Element::Type::VOLTA) {
                  //export other spanner if staffidx matches
                  if (srcStaffIdx == staffIdx) {
                        dstTrack = dstStaffIdx * VOICES + s->voice();
                        dstTrack2 = dstStaffIdx * VOICES + (s->track2() % VOICES);
                        }
                  }
            if (dstTrack == -1)
                  continue;
            cloneSpanner(s, score, dstTrack, dstTrack2);
            }
      }

//---------------------------------------------------------
//   cloneStaff2
//    staves are potentially in different scores
//---------------------------------------------------------

void cloneStaff2(Staff* srcStaff, Staff* dstStaff, int stick, int etick)
      {
      Score* oscore = srcStaff->score();
      Score* score  = dstStaff->score();
      Measure* m1   = oscore->tick2measure(stick);
      Measure* m2   = oscore->tick2measure(etick);

      if (m2->tick() < etick) // end of score
            m2 = 0;

      TieMap tieMap;

      int srcStaffIdx = oscore->staffIdx(srcStaff);
      int dstStaffIdx = score->staffIdx(dstStaff);

      for (Measure* m = m1; m && (m != m2); m = m->nextMeasure()) {
            Measure* nm = score->tick2measure(m->tick());
            int sTrack = srcStaffIdx * VOICES;
            int eTrack = sTrack + VOICES;
            for (int srcTrack = sTrack; srcTrack < eTrack; ++srcTrack) {
                  TupletMap tupletMap;    // tuplets cannot cross measure boundaries
                  int dstTrack = dstStaffIdx * VOICES + (srcTrack - sTrack);
                  for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                        Element* oe = oseg->element(srcTrack);
                        if (oe == 0 || oe->generated())
                              continue;
                        if (oe->type() == Element::Type::TIMESIG)
                              continue;
                        Segment* ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                        Element* ne = oe->linkedClone();
                        ne->setTrack(dstTrack);
                        ne->setParent(ns);
                        ne->setScore(score);
                        score->undoAddElement(ne);
                        if (oe->isChordRest()) {
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              ChordRest* ncr = static_cast<ChordRest*>(ne);
                              Tuplet* ot     = ocr->tuplet();
                              if (ot) {
                                    Tuplet* nt = tupletMap.findNew(ot);
                                    if (nt == 0) {
                                          // nt = new Tuplet(*ot);
                                          nt = static_cast<Tuplet*>(ot->linkedClone());
                                          nt->clear();
                                          nt->setTrack(dstTrack);
                                          nt->setParent(m);
                                          tupletMap.add(ot, nt);
                                          }
                                    ncr->setTuplet(nt);
                                    nt->add(ncr);
                                    }

                              foreach (Element* e, oseg->annotations()) {
                                    if (e->generated() || e->systemFlag())
                                          continue;
                                    if (e->track() != srcTrack)
                                          continue;
                                    switch (e->type()) {
                                          // exclude certain element types
                                          // this should be same list excluded in Score::undoAddElement()
                                          case Element::Type::STAFF_TEXT:
                                          case Element::Type::FRET_DIAGRAM:
                                          case Element::Type::HARMONY:
                                          case Element::Type::FIGURED_BASS:
                                          case Element::Type::DYNAMIC:
                                          case Element::Type::LYRICS:   // not normally segment-attached
                                                continue;
                                          default:
                                                Element* ne = e->clone();
                                                ne->setTrack(dstTrack);
                                                ne->setParent(ns);
                                                ne->setScore(score);
                                                score->undoAddElement(ne);
                                          }
                                    }
                              if (oe->type() == Element::Type::CHORD) {
                                    Chord* och = static_cast<Chord*>(ocr);
                                    Chord* nch = static_cast<Chord*>(ncr);
                                    int n = och->notes().size();
                                    for (int i = 0; i < n; ++i) {
                                          Note* on = och->notes().at(i);
                                          Note* nn = nch->notes().at(i);
                                          if (on->tieFor()) {
                                                Tie* tie = static_cast<Tie*>(on->tieFor()->linkedClone());
                                                tie->setScore(score);
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

      for (auto i : oscore->spanner()) {
            Spanner* s = i.second;
            if (!(s->tick() >= stick && s->tick2() < etick))
                  continue;

            int staffIdx = s->staffIdx();
            int dstTrack = -1;
            int dstTrack2 = -1;
            if (s->type() != Element::Type::VOLTA) {
                  //export other spanner if staffidx matches
                  if (srcStaffIdx == staffIdx) {
                        dstTrack  = dstStaffIdx * VOICES + s->voice();
                        dstTrack2 = dstStaffIdx * VOICES + (s->track2() % VOICES);
                        }
                  }
            if (dstTrack == -1)
                  continue;
            cloneSpanner(s, score, dstTrack, dstTrack2);
            }
      }

//---------------------------------------------------------
//   createAllExcerpt
//---------------------------------------------------------

QList<Excerpt*> Excerpt::createAllExcerpt(MasterScore *score)
      {
      QList<Excerpt*> all;
      for (Part* part : score->parts()) {
            if (part->show()) {
                  Excerpt* e   = new Excerpt(score);
                  e->parts().append(part);
                  QString name = createName(part->partName(), all);
                  e->setTitle(name);
                  all.append(e);
                  }
            }
      return all;
      }

//---------------------------------------------------------
//   createName
//---------------------------------------------------------

QString Excerpt::createName(const QString& partName, QList<Excerpt*> excerptList)
      {
      QString n = partName.simplified();
      QString name;
      int count = excerptList.count();
      for (int i = 0;; ++i) {
            name = i ? QString("%1-%2").arg(n).arg(i) : QString("%1").arg(n);
            Excerpt* ee = 0;
            for (int k = 0; k < count; ++k) {
                  ee = excerptList[k];
                  if (ee->title() == name)
                        break;
                  }
            if ((ee == 0) || (ee->title() != name))
                  break;
            }
      return name;
      }

}
