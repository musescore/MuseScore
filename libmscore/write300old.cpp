//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "config.h"
#include "score.h"
#include "xml.h"
#include "element.h"
#include "measure.h"
#include "segment.h"
#include "slur.h"
#include "chordrest.h"
#include "chord.h"
#include "tuplet.h"
#include "beam.h"
#include "revisions.h"
#include "page.h"
#include "part.h"
#include "staff.h"
#include "system.h"
#include "keysig.h"
#include "clef.h"
#include "text.h"
#include "ottava.h"
#include "volta.h"
#include "excerpt.h"
#include "mscore.h"
#include "stafftype.h"
#include "sym.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

#include "sig.h"
#include "undo.h"
#include "imageStore.h"
#include "audio.h"
#include "barline.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <stdio.h>
#endif

namespace Ms {

//---------------------------------------------------------
//   writeMeasure
//---------------------------------------------------------

static void writeMeasure(XmlWriter& xml, MeasureBase* m, int staffIdx, bool writeSystemElements, bool forceTimeSig)
      {
      //
      // special case multi measure rest
      //
      if (m->isMeasure() || staffIdx == 0)
            m->write300old(xml, staffIdx, writeSystemElements, forceTimeSig);

      if (m->score()->styleB(Sid::createMultiMeasureRests) && m->isMeasure() && toMeasure(m)->mmRest())
            toMeasure(m)->mmRest()->write300old(xml, staffIdx, writeSystemElements, forceTimeSig);

      xml.setCurTick(m->endTick());
      }

//---------------------------------------------------------
//   writeMovement300old
//---------------------------------------------------------

void Score::writeMovement300old(XmlWriter& xml, bool selectionOnly)
      {
      // if we have multi measure rests and some parts are hidden,
      // then some layout information is missing:
      // relayout with all parts set visible

      QList<Part*> hiddenParts;
      bool unhide = false;
      if (styleB(Sid::createMultiMeasureRests)) {
            for (Part* part : _parts) {
                  if (!part->show()) {
                        if (!unhide) {
                              startCmd();
                              unhide = true;
                              }
                        part->undoChangeProperty(Pid::VISIBLE, true);
                        hiddenParts.append(part);
                        }
                  }
            }
      if (unhide) {
            doLayout();
            for (Part* p : hiddenParts)
                  p->setShow(false);
            }

      xml.stag("Score");
      if (excerpt()) {
            Excerpt* e = excerpt();
            QMultiMap<int, int> trackList = e->tracks();
            QMapIterator<int, int> i(trackList);
            if (!(trackList.size() == e->parts().size() * VOICES) && !trackList.isEmpty()) {
                  while (i.hasNext()) {
                      i.next();
                      xml.tagE(QString("Tracklist sTrack=\"%1\" dstTrack=\"%2\"").arg(i.key()).arg(i.value()));
                      }
                  }
            }

      if (_layoutMode == LayoutMode::LINE)
            xml.tag("layoutMode", "line");

#ifdef OMR
      if (masterScore()->omr() && xml.writeOmr())
            masterScore()->omr()->write(xml);
#endif
      if (isMaster() && masterScore()->showOmr() && xml.writeOmr())
            xml.tag("showOmr", masterScore()->showOmr());
      if (_audio && xml.writeOmr()) {
            xml.tag("playMode", int(_playMode));
            _audio->write(xml);
            }

      for (int i = 0; i < 32; ++i) {
            if (!_layerTags[i].isEmpty()) {
                  xml.tag(QString("LayerTag id=\"%1\" tag=\"%2\"").arg(i).arg(_layerTags[i]),
                     _layerTagComments[i]);
                  }
            }
      int n = _layer.size();
      for (int i = 1; i < n; ++i) {       // dont save default variant
            const Layer& l = _layer[i];
            xml.tagE(QString("Layer name=\"%1\" mask=\"%2\"").arg(l.name).arg(l.tags));
            }
      xml.tag("currentLayer", _currentLayer);

      if (isTopScore() && !MScore::testMode)
            _synthesizerState.write(xml);

      if (pageNumberOffset())
            xml.tag("page-offset", pageNumberOffset());
      xml.tag("Division", MScore::division);
      xml.setCurTrack(-1);

      if (isTopScore())                   // only top score
            style().save(xml, true);       // save only differences to buildin style

      xml.tag("showInvisible",   _showInvisible);
      xml.tag("showUnprintable", _showUnprintable);
      xml.tag("showFrames",      _showFrames);
      xml.tag("showMargins",     _showPageborders);

      QMapIterator<QString, QString> i(_metaTags);
      while (i.hasNext()) {
            i.next();
            // do not output "platform" and "creationDate" in test mode
            if ((!MScore::testMode  && !MScore::saveTemplateMode) || (i.key() != "platform" && i.key() != "creationDate"))
                  xml.tag(QString("metaTag name=\"%1\"").arg(i.key().toHtmlEscaped()), i.value());
            }

      xml.setCurTrack(0);
      int staffStart;
      int staffEnd;
      MeasureBase* measureStart;
      MeasureBase* measureEnd;

      if (selectionOnly) {
            staffStart   = _selection.staffStart();
            staffEnd     = _selection.staffEnd();
            // make sure we select full parts
            Staff* sStaff = staff(staffStart);
            Part* sPart = sStaff->part();
            Staff* eStaff = staff(staffEnd - 1);
            Part* ePart = eStaff->part();
            staffStart = staffIdx(sPart);
            staffEnd = staffIdx(ePart) + ePart->nstaves();
            measureStart = _selection.startSegment()->measure();
            if (_selection.endSegment())
                  measureEnd   = _selection.endSegment()->measure()->next();
            else
                  measureEnd   = 0;
            }
      else {
            staffStart   = 0;
            staffEnd     = nstaves();
            measureStart = first();
            measureEnd   = 0;
            }

      // Let's decide: write midi mapping to a file or not
      masterScore()->checkMidiMapping();
      for (const Part* part : _parts) {
            if (!selectionOnly || ((staffIdx(part) >= staffStart) && (staffEnd >= staffIdx(part) + part->nstaves())))
                  part->write(xml);
            }

      xml.setCurTrack(0);
      xml.setTrackDiff(-staffStart * VOICES);
      if (measureStart) {
            for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                  xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1 - staffStart));
                  xml.setCurTick(measureStart->tick());
                  xml.setTickDiff(xml.curTick());
                  xml.setCurTrack(staffIdx * VOICES);
                  bool writeSystemElements = (staffIdx == staffStart);
                  bool firstMeasureWritten = false;
                  bool forceTimeSig = false;
                  for (MeasureBase* m = measureStart; m != measureEnd; m = m->next()) {
                        // force timesig if first measure and selectionOnly
                        if (selectionOnly && m->isMeasure()) {
                              if (!firstMeasureWritten) {
                                    forceTimeSig = true;
                                    firstMeasureWritten = true;
                                    }
                              else
                                    forceTimeSig = false;
                              }
                        writeMeasure(xml, m, staffIdx, writeSystemElements, forceTimeSig);
                        }
                  xml.etag();
                  }
            }
      xml.setCurTrack(-1);
      if (isMaster()) {
            if (!selectionOnly) {
                  for (const Excerpt* excerpt : excerpts()) {
                        if (excerpt->partScore() != this)
                              excerpt->partScore()->write300old(xml, false);       // recursion
                        }
                  }
            }
      else
            xml.tag("name", excerpt()->title());
      xml.etag();

      if (unhide) {
            endCmd();
            undoRedo(true, 0);   // undo
            }
      }

//---------------------------------------------------------
//   write300old
//---------------------------------------------------------

void Score::write300old(XmlWriter& xml, bool selectionOnly)
      {
      if (isMaster()) {
            MasterScore* score = static_cast<MasterScore*>(this);
            while (score->prev())
                  score = score->prev();
            while (score) {
                  score->writeMovement300old(xml, selectionOnly);
                  score = score->next();
                  }
            }
      else
            writeMovement300old(xml, selectionOnly);
      }

//---------------------------------------------------------
//   writeSegments300old
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void Score::writeSegments300old(XmlWriter& xml, int strack, int etrack,
   Segment* fs, Segment* ls, bool writeSystemElements, bool clip, bool needFirstTick, bool forceTimeSig)
      {
      int endTick = ls ? ls->tick() : lastMeasure()->endTick();
      // in clipboard mode, ls might be in an mmrest
      // since we are traversing regular measures,
      // force them out of mmRest
      if (clip) {
            Measure* lm = ls ? ls->measure() : 0;
            Measure* fm = fs ? fs->measure() : 0;
            if (lm && lm->isMMRest()) {
                  lm = lm->mmRestLast();
                  if (lm)
                        ls = lm->nextMeasure() ? lm->nextMeasure()->first() : lastSegment();
                  else
                        qDebug("writeSegments: no measure for end segment in mmrest");
                  }
            if (fm && fm->isMMRest()) {
                  fm = fm->mmRestFirst();
                  if (fm)
                        fs = fm->first();
                  }
            }

      QList<Spanner*> spanners;
#if 0
      auto endIt   = spanner().upper_bound(endTick);
      for (auto i = spanner().begin(); i != endIt; ++i) {
            Spanner* s = i->second;
#else
      auto sl = spannerMap().findOverlapping(fs->tick(), endTick);
      for (auto i : sl) {
            Spanner* s = i.value;
#endif
            if (s->generated() || !xml.canWrite(s))
                  continue;
            // don't write voltas to clipboard
            if (clip && s->isVolta())
                  continue;
            spanners.push_back(s);
            }

      for (int track = strack; track < etrack; ++track) {
            if (!xml.canWriteVoice(track))
                  continue;

            bool timeSigWritten = false; // for forceTimeSig
            bool crWritten = false;      // for forceTimeSig
            bool keySigWritten = false;  // for forceTimeSig

            for (Segment* segment = fs; segment && segment != ls; segment = segment->next1()) {
                  if (!segment->enabled())
                        continue;
                  if (track == 0)
                        segment->setWritten(false);
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  bool needTick = (needFirstTick && segment == fs) || (segment->tick() != xml.curTick());
                  if ((segment->isEndBarLineType()) && !e && writeSystemElements && ((track % VOICES) == 0)) {
                        // search barline:
                        for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                              if (segment->element(idx)) {
                                    int oDiff = xml.trackDiff();
                                    xml.setTrackDiff(idx);          // staffIdx should be zero
                                    segment->element(idx)->write300old(xml);
                                    xml.setTrackDiff(oDiff);
                                    break;
                                    }
                              }
                        }
                  for (Element* e : segment->annotations()) {
                        if (e->track() != track || e->generated() || (e->systemFlag() && !writeSystemElements))
                              continue;
                        if (needTick) {
                              // xml.tag("tick", segment->tick() - xml.tickDiff);
                              int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                              xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                              xml.setCurTick(segment->tick());
                              needTick = false;
                              }
                        e->write300old(xml);
                        }
                  Measure* m = segment->measure();
                  // don't write spanners for multi measure rests

                  if ((!(m && m->isMMRest())) && segment->isChordRestType()) {
                        for (Spanner* s : spanners) {
                              if (s->track() == track) {
                                    bool end = false;
                                    if (s->anchor() == Spanner::Anchor::CHORD || s->anchor() == Spanner::Anchor::NOTE)
                                          end = s->tick2() < endTick;
                                    else
                                          end = s->tick2() <= endTick;
                                    if (s->tick() == segment->tick() && (!clip || end)) {
                                          if (needTick) {
                                                // xml.tag("tick", segment->tick() - xml.tickDiff);
                                                int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                                                xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                                                xml.setCurTick(segment->tick());
                                                needTick = false;
                                                }
                                          s->write300old(xml);
                                          }
                                    }
                              if ((s->tick2() == segment->tick())
                                 && !s->isSlur()
                                 && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                                 && (!clip || s->tick() >= fs->tick())
                                 ) {
                                    if (needTick) {
                                          // xml.tag("tick", segment->tick() - xml.tickDiff);
                                          int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                                          xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                                          xml.setCurTick(segment->tick());
                                          needTick = false;
                                          }
                                    xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                                    }
                              }
                        }

                  if (!e || !xml.canWrite(e))
                        continue;
                  if (e->generated())
                        continue;
                  if (forceTimeSig && track2voice(track) == 0 && segment->segmentType() == SegmentType::ChordRest && !timeSigWritten && !crWritten) {
                        // we will miss a key sig!
                        if (!keySigWritten) {
                              Key k = score()->staff(track2staff(track))->key(segment->tick());
                              KeySig* ks = new KeySig(this);
                              ks->setKey(k);
                              ks->write300old(xml);
                              delete ks;
                              keySigWritten = true;
                              }
                        // we will miss a time sig!
                        Fraction tsf = sigmap()->timesig(segment->tick()).timesig();
                        TimeSig* ts = new TimeSig(this);
                        ts->setSig(tsf);
                        ts->write300old(xml);
                        delete ts;
                        timeSigWritten = true;
                        }
                  if (needTick) {
                        // xml.tag("tick", segment->tick() - xml.tickDiff);
                        int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                        xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                        xml.setCurTick(segment->tick());
                        needTick = false;
                        }
                  if (e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        cr->writeBeam(xml);
                        cr->writeTuplet(xml);
                        }
//                  if (segment->isEndBarLine() && (m->mmRestCount() < 0 || m->mmRest())) {
//                        BarLine* bl = toBarLine(e);
//TODO                        bl->setBarLineType(m->endBarLineType());
//                        bl->setVisible(m->endBarLineVisible());
//                        }
                  e->write300old(xml);
                  segment->write300old(xml);    // write only once
                  if (forceTimeSig) {
                        if (segment->segmentType() == SegmentType::KeySig)
                              keySigWritten = true;
                        if (segment->segmentType() == SegmentType::TimeSig)
                              timeSigWritten = true;
                        if (segment->segmentType() == SegmentType::ChordRest)
                              crWritten = true;
                        }
                  }

            //write spanner ending after the last segment, on the last tick
            if (clip || ls == 0) {
                  for (Spanner* s : spanners) {
                        if ((s->tick2() == endTick)
                          && !s->isSlur()
                          && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                          && (!clip || s->tick() >= fs->tick())
                          ) {
                              xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                              }
                        }
                  }
            }
      }

}

