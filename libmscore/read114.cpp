//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "score.h"
#include "slur.h"
#include "staff.h"
#include "excerpt.h"
#include "chord.h"
#include "rest.h"
#include "keysig.h"
#include "volta.h"
#include "measure.h"
#include "beam.h"
#include "page.h"
#include "segment.h"
#include "ottava.h"
#include "stafftype.h"
#include "text.h"
#include "part.h"
#include "sig.h"
#include "box.h"
#include "dynamic.h"
#include "drumset.h"
#include "style.h"

//---------------------------------------------------------
//   style114
//---------------------------------------------------------

static const StyleVal style114[] = {
      StyleVal(ST_staffUpperBorder, Spatium(7.0)),
      StyleVal(ST_staffLowerBorder, Spatium(7.0)),
      StyleVal(ST_staffDistance, Spatium(6.5)),
      StyleVal(ST_akkoladeDistance, Spatium(6.5)),
//      StyleVal(ST_systemDistance, Spatium(9.25)),
      StyleVal(ST_lyricsDistance, Spatium(2)),
      StyleVal(ST_lyricsMinBottomDistance, Spatium(2)),
      StyleVal(ST_systemFrameDistance, Spatium(7.0)),
      StyleVal(ST_frameSystemDistance, Spatium(1.0)),
      StyleVal(ST_minMeasureWidth, Spatium(4.0)),

      StyleVal(ST_barWidth, Spatium(0.16)),
      StyleVal(ST_doubleBarWidth, Spatium(0.16)),
      StyleVal(ST_endBarWidth, Spatium(0.3)),
      StyleVal(ST_doubleBarDistance, Spatium(0.30)),
      StyleVal(ST_endBarDistance, Spatium(0.30)),
      StyleVal(ST_repeatBarTips, false),
      StyleVal(ST_startBarlineSingle, false),
      StyleVal(ST_startBarlineMultiple, true),

      StyleVal(ST_bracketWidth, Spatium(0.35)),
      StyleVal(ST_bracketDistance, Spatium(0.25)),

      StyleVal(ST_clefLeftMargin, Spatium(0.5)),
      StyleVal(ST_keysigLeftMargin, Spatium(0.5)),
      StyleVal(ST_timesigLeftMargin, Spatium(0.5)),

      StyleVal(ST_clefKeyRightMargin, Spatium(1.75)),
      StyleVal(ST_clefBarlineDistance, Spatium(0.18)),
      StyleVal(ST_stemWidth, Spatium(0.13)),
      StyleVal(ST_shortenStem, true),
      StyleVal(ST_shortStemProgression, Spatium(0.25)),
      StyleVal(ST_shortestStem,Spatium(2.25)),
      StyleVal(ST_beginRepeatLeftMargin,Spatium(1.0)),
      StyleVal(ST_minNoteDistance,Spatium(0.4)),
//      StyleVal(ST_barNoteDistance,Spatium(1.2)),
      StyleVal(ST_barNoteDistance,Spatium(.6)),
      StyleVal(ST_noteBarDistance,Spatium(1.0)),

      StyleVal(ST_measureSpacing, qreal(1.2)),

      StyleVal(ST_staffLineWidth,Spatium(0.08)),
      StyleVal(ST_ledgerLineWidth,Spatium(0.12)),
      StyleVal(ST_akkoladeWidth,Spatium(1.6)),
      StyleVal(ST_accidentalDistance,Spatium(0.22)),
      StyleVal(ST_accidentalNoteDistance,Spatium(0.22)),
      StyleVal(ST_beamWidth,Spatium(0.48)),
      StyleVal(ST_beamDistance,qreal(0.5)),
      StyleVal(ST_beamMinLen,Spatium(1.25)),
      StyleVal(ST_beamMinSlope, qreal(0.05)),

      StyleVal(ST_beamMaxSlope, qreal(0.2)),
      StyleVal(ST_maxBeamTicks, MScore::division),
      StyleVal(ST_dotNoteDistance,Spatium(0.35)),
      StyleVal(ST_dotRestDistance,Spatium(0.25)),
      StyleVal(ST_dotDotDistance,Spatium(0.5)),
      StyleVal(ST_propertyDistanceHead,Spatium(1.0)),
      StyleVal(ST_propertyDistanceStem,Spatium(0.5)),
      StyleVal(ST_propertyDistance,Spatium(1.0)),
//      StyleVal(ST_pageFillLimit,0.7),
      StyleVal(ST_lastSystemFillLimit, qreal(0.3)),

      StyleVal(ST_hairpinHeight,Spatium(1.2)),
      StyleVal(ST_hairpinContHeight,Spatium(0.5)),
      StyleVal(ST_hairpinWidth,Spatium(0.13)),
      StyleVal(ST_showPageNumber,true),
      StyleVal(ST_showPageNumberOne,false),
      StyleVal(ST_pageNumberOddEven,true),
      StyleVal(ST_showMeasureNumber,true),
      StyleVal(ST_showMeasureNumberOne,false),
      StyleVal(ST_measureNumberInterval,5),
      StyleVal(ST_measureNumberSystem,true),

      StyleVal(ST_measureNumberAllStaffs,false),
      StyleVal(ST_smallNoteMag, qreal(0.7)),
      StyleVal(ST_graceNoteMag, qreal(0.7)),
      StyleVal(ST_smallStaffMag, qreal(0.7)),
      StyleVal(ST_smallClefMag, qreal(0.8)),
      StyleVal(ST_genClef,true),
      StyleVal(ST_genKeysig,true),
      StyleVal(ST_genTimesig,true),
      StyleVal(ST_genCourtesyTimesig, true),
      StyleVal(ST_genCourtesyKeysig, true),

      StyleVal(ST_useGermanNoteNames, false),
      StyleVal(ST_chordDescriptionFile, QString("stdchords.xml")),
      StyleVal(ST_concertPitch, false),
      StyleVal(ST_createMultiMeasureRests, false),
      StyleVal(ST_minEmptyMeasures, 2),
      StyleVal(ST_minMMRestWidth, Spatium(4)),
      StyleVal(ST_hideEmptyStaves, false),
      StyleVal(ST_stemDir1, MScore::UP),
      StyleVal(ST_stemDir2, MScore::DOWN),
      StyleVal(ST_stemDir3, MScore::UP),
      StyleVal(ST_stemDir4, MScore::DOWN),

      StyleVal(ST_gateTime, 100),
      StyleVal(ST_tenutoGateTime, 100),
      StyleVal(ST_staccatoGateTime, 50),
      StyleVal(ST_slurGateTime, 100),
#if 0
      StyleVal(ST_UfermataAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DfermataAnchor, int(A_BOTTOM_STAFF)),
      StyleVal(ST_UshortfermataAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DshortfermataAnchor, int(A_BOTTOM_STAFF)),
      StyleVal(ST_UlongfermataAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DlongfermataAnchor, int(A_BOTTOM_STAFF)),
      StyleVal(ST_UverylongfermataAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DverylongfermataAnchor, int(A_BOTTOM_STAFF)),

      StyleVal(ST_ThumbAnchor, int(A_CHORD)),
      StyleVal(ST_SforzatoaccentAnchor, int(A_CHORD)),
      StyleVal(ST_EspressivoAnchor, int(A_CHORD)),
      StyleVal(ST_StaccatoAnchor, int(A_CHORD)),
      StyleVal(ST_UstaccatissimoAnchor, int(A_CHORD)),
      StyleVal(ST_DstaccatissimoAnchor, int(A_CHORD)),
      StyleVal(ST_TenutoAnchor, int(A_CHORD)),
      StyleVal(ST_UportatoAnchor, int(A_CHORD)),
      StyleVal(ST_DportatoAnchor, int(A_CHORD)),
      StyleVal(ST_UmarcatoAnchor, int(A_CHORD)),
      StyleVal(ST_DmarcatoAnchor, int(A_CHORD)),
      StyleVal(ST_OuvertAnchor, int(A_CHORD)),
      StyleVal(ST_PlusstopAnchor, int(A_CHORD)),
      StyleVal(ST_UpbowAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DownbowAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_ReverseturnAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_TurnAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_TrillAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_PrallAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_MordentAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_PrallPrallAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_PrallMordentAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_UpPrallAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DownPrallAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_UpMordentAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_DownMordentAnchor, int(A_TOP_STAFF)),
      StyleVal(ST_SnappizzicatoAnchor, int(A_CHORD)),
#endif

      StyleVal(ST_ArpeggioNoteDistance, Spatium(.5)),
      StyleVal(ST_ArpeggioLineWidth, Spatium(.18)),
      StyleVal(ST_ArpeggioHookLen, Spatium(.8)),
      StyleVal(ST_FixMeasureNumbers, 0),
      StyleVal(ST_FixMeasureWidth, false)
      };


//---------------------------------------------------------
//   resolveSymCompatibility
//---------------------------------------------------------

static SymId resolveSymCompatibility(SymId i, QString programVersion)
      {
      if (!programVersion.isEmpty() && programVersion < "1.1")
            i = SymId(i + 5);
      switch(i) {
            case 197:   return pedalPedSym;
            case 191:   return pedalasteriskSym;
            case 193:   return pedaldotSym;
            case 192:   return pedaldashSym;
            case 139:   return trillSym;
            default:    return noSym;
            }
      }

//---------------------------------------------------------
//   Staff::read114
//---------------------------------------------------------

void Staff::read114(XmlReader& e, ClefList& _clefList)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "lines")
                  setLines(e.readInt());
            else if (tag == "small")
                  setSmall(e.readInt());
            else if (tag == "invisible")
                  setInvisible(e.readInt());
            else if (tag == "slashStyle")
                  e.skipCurrentElement();
            else if (tag == "cleflist")
                  _clefList.read(e, _score);
            else if (tag == "keylist")
                  _keymap->read(e, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket = BracketType(e.intAttribute("type", -1));
                  b._bracketSpan = e.intAttribute("span", 0);
                  _brackets.append(b);
                  e.readNext();
                  }
            else if (tag == "barLineSpan")
                  _barLineSpan = e.readInt();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Part::read114
//---------------------------------------------------------

void Part::read114(XmlReader& e)
      {
      int rstaff = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score, this, rstaff);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  ClefList* cl = new ClefList;
                  e.clefListList().append(cl);
                  staff->read114(e, *cl);
                  ++rstaff;
                  }
            else if (tag == "Instrument") {
                  instr(0)->read(e);
                  Drumset* d = instr(0)->drumset();
                  Staff*   st = staff(0);
                  if (d && st && st->lines() != 5) {
                        int n = 0;
                        if (st->lines() == 1)
                              n = 4;
                        for (int  i = 0; i < DRUM_INSTRUMENTS; ++i)
                              d->drum(i).line -= n;
                        }
                  }
            else if (tag == "name") {
                  Text* t = new Text(score());
                  t->read(e);
                  instr(0)->setLongName(t->getFragment());
                  delete t;
                  }
            else if (tag == "shortName") {
                  Text* t = new Text(score());
                  t->read(e);
                  instr(0)->setShortName(t->getFragment());
                  delete t;
                  }
            else if (tag == "trackName")
                  _partName = e.readElementText();
            else if (tag == "show")
                  _show = e.readInt();
            else
                  e.unknown();
            }
      if (_partName.isEmpty())
            _partName = instr(0)->trackName();

      if (instr(0)->useDrumset()) {
            foreach(Staff* staff, _staves) {
                  int lines = staff->lines();
                  staff->setStaffType(score()->staffType(PERCUSSION_STAFF_TYPE));
                  staff->setLines(lines);
                  }
            }
      }

//---------------------------------------------------------
//   read114
//    import old version 1.2 files
//---------------------------------------------------------

Score::FileError Score::read114(XmlReader& e)
      {
      if (parentScore())
            setMscVersion(parentScore()->mscVersion());

      for (unsigned int i = 0; i < sizeof(style114)/sizeof(*style114); ++i)
            style()->set(style114[i]);

      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff(e);
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(this);
                  ks->read(e);
                  customKeysigs.append(ks);
                  }
            else if (tag == "siglist")
                  _sigmap->read(e, _fileDivision);
            else if (tag == "programVersion") {
                  _mscoreVersion = e.readElementText();
                  parseVersion(_mscoreVersion);
                  }
            else if (tag == "programRevision")
                  _mscoreRevision = e.readInt();
            else if (tag == "Mag"
               || tag == "MagIdx"
               || tag == "xoff"
               || tag == "tempolist"
               || tag == "Symbols"
               || tag == "cursorTrack"
               || tag == "yoff")
                  e.skipCurrentElement();       // obsolete
            else if (tag == "playMode")
                  _playMode = PlayMode(e.readInt());
            else if (tag == "SyntiSettings") {
                  _syntiState.clear();
                  _syntiState.read(e);
                  }
            else if (tag == "Spatium")
                  _style.setSpatium (e.readDouble() * MScore::DPMM);
            else if (tag == "page-offset")
                  setPageNumberOffset(e.readInt());
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  _showInvisible = e.readInt();
            else if (tag == "showFrames")
                  _showFrames = e.readInt();
            else if (tag == "showMargins")
                  _showPageborders = e.readInt();
            else if (tag == "Style") {
                  qreal sp = _style.spatium();
                  _style.load(e);
                  if (_layoutMode == LayoutFloat) {
                        // style should not change spatium in
                        // float mode
                        _style.setSpatium(sp);
                        }
                  }
            else if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(e);

                  // Change 1.2 Poet to Lyricist
                  if (s.name() == "Poet")
                        s.setName("Lyricist");

                  _style.setTextStyle(s);
                  }
            else if (tag == "page-layout") {
                  if (_layoutMode != LayoutFloat && _layoutMode != LayoutSystem) {
                        PageFormat pf;
                        pf.copy(*pageFormat());
                        pf.read(e);
                        setPageFormat(pf);
                        }
                  else
                        e.skipCurrentElement();
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(e);
                  setMetaTag("copyright", text->text());
                  delete text;
                  }
            else if (tag == "movement-number")
                  setMetaTag("movementNumber", e.readElementText());
            else if (tag == "movement-title")
                  setMetaTag("movementTitle", e.readElementText());
            else if (tag == "work-number")
                  setMetaTag("workNumber", e.readElementText());
            else if (tag == "work-title")
                  setMetaTag("workTitle", e.readElementText());
            else if (tag == "source")
                  setMetaTag("source", e.readElementText());
            else if (tag == "metaTag") {
                  QString name = e.attribute("name");
                  setMetaTag(name, e.readElementText());
                  }
            else if (tag == "Part") {
                  Part* part = new Part(this);
                  part->read114(e);
                  _parts.push_back(part);
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(this);
                  slur->read(e);
                  e.addSpanner(slur);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Pedal")) {
                  Spanner* s = static_cast<Spanner*>(Element::name2Element(tag, this));
                  s->read(e);
                  if (s->track() == -1)
                        s->setTrack(e.track());
                  else
                        e.setTrack(s->track());       // update current track
                  if (s->__tick1() == -1)
                        s->__setTick1(e.tick());
                  else
                        e.setTick(s->__tick1());      // update current tick
                  if (s->__tick2() == -1) {
                        delete s;
                        qDebug("invalid spanner %s tick2: %d\n",
                           s->name(), s->__tick2());
                        }
                  else
                        e.addSpanner(s);
                  // qDebug("Spanner <%s> %d %d track %d", s->name(),
                  //   s->__tick1(), s->__tick2(), s->track());
                  }
            else if (tag == "Excerpt") {
                  Excerpt* ex = new Excerpt(this);
                  ex->read(e);
                  _excerpts.append(ex);
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(this);
                  beam->read(e);
                  beam->setParent(0);
                  // _beams.append(beam);
                  }
            else if (tag == "name")
                  setName(e.readElementText());
            else
                  e.unknown();
            }

      int n = nstaves();
      for (int idx = 0; idx < n; ++idx) {
            Staff* s = _staves[idx];
            int track = idx * VOICES;

            // check barLineSpan
            if (s->barLineSpan() > (n - idx)) {
                  qDebug("read114: invalid bar line span %d (max %d)",
                     s->barLineSpan(), n - idx);
                  s->setBarLineSpan(n - idx);
                  }

            ClefList* cl = e.clefListList().at(idx);
            for (auto i = cl->constBegin(); i != cl->constEnd(); ++i) {
                  int tick = i.key();
                  ClefType clefId = i.value()._concertClef;
                  Measure* m = tick2measure(tick);
                  if ((tick == m->tick()) && m->prevMeasure())
                        m = m->prevMeasure();
                  Segment* seg = m->getSegment(Segment::SegClef, tick);
                  if (seg->element(track))
                        static_cast<Clef*>(seg->element(track))->setGenerated(false);
                  else {
                        Clef* clef = new Clef(this);
                        clef->setClefType(clefId);
                        clef->setTrack(track);
                        clef->setParent(seg);
                        clef->setGenerated(false);
                        seg->add(clef);
                        }
                  }

            KeyList* km = s->keymap();
            for (auto i = km->begin(); i != km->end(); ++i) {
                  int tick = i->first;
                  if (tick < 0) {
                        qDebug("read114: Key tick %d", tick);
                        continue;
                        }
                  KeySigEvent ke = i->second;
                  Measure* m = tick2measure(tick);
                  Segment* seg = m->getSegment(Segment::SegKeySig, tick);
                  if (seg->element(track))
                        static_cast<KeySig*>(seg->element(track))->setGenerated(false);
                  else {
                        KeySig* ks = keySigFactory(ke);
                        if (ks) {
                              ks->setParent(seg);
                              ks->setTrack(track);
                              ks->setGenerated(false);
                              seg->add(ks);
                              }
                        }
                  }
            }
      qDeleteAll(e.clefListList());

      foreach(Spanner* s, e.spanner()) {
            if (s->type() == Element::OTTAVA
               || (s->type() == Element::TEXTLINE)
               || (s->type() == Element::VOLTA)
               || (s->type() == Element::PEDAL))
                  {
                  TextLine* tl = static_cast<TextLine*>(s);
                  tl->setBeginSymbol(resolveSymCompatibility(tl->beginSymbol(), mscoreVersion()));
                  tl->setContinueSymbol(resolveSymCompatibility(tl->continueSymbol(), mscoreVersion()));
                  tl->setEndSymbol(resolveSymCompatibility(tl->endSymbol(), mscoreVersion()));
                  }

            if (s->type() == Element::SLUR) {
                  Slur* slur = static_cast<Slur*>(s);

                  if (!slur->startElement() || !slur->endElement()) {
                        qDebug("incomplete Slur");
                        if (slur->startElement()) {
                              qDebug("  front %d", static_cast<ChordRest*>(slur->startElement())->tick());
                              static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                              }
                        if (slur->endElement()) {
                              qDebug("  back %d", static_cast<ChordRest*>(slur->endElement())->tick());
                              static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
                              }
                        }
                  else {
                        ChordRest* cr1 = (ChordRest*)(slur->startElement());
                        ChordRest* cr2 = (ChordRest*)(slur->endElement());
                        if (cr1->tick() > cr2->tick()) {
                              qDebug("Slur invalid start-end tick %d-%d", cr1->tick(), cr2->tick());
                              slur->setStartElement(cr2);
                              slur->setEndElement(cr1);
                              }
                        int n1 = 0;
                        int n2 = 0;
                        for (Spanner* s = cr1->spannerFor(); s; s = s->next()) {
                              if (s == slur)
                                    ++n1;
                              }
                        for (Spanner* s = cr2->spannerBack(); s; s = s->next()) {
                              if (s == slur)
                                    ++n2;
                              }
                        if (n1 != 1 || n2 != 1) {
                              qDebug("Slur references bad: %d %d", n1, n2);
                              }
                        }
                  }
            else {
                  Segment* s1 = tick2segment(s->__tick1());
                  Segment* s2 = tick2segment(s->__tick2());
                  if (s1 == 0 || s2 == 0) {
                        qDebug("cannot place %s at tick %d - %d",
                           s->name(), s->__tick1(), s->__tick2());
                        continue;
                        }
                  if (s->type() == Element::VOLTA) {
                        Volta* volta = static_cast<Volta*>(s);
                        volta->setAnchor(Spanner::ANCHOR_MEASURE);
                        volta->setStartMeasure(s1->measure());
                        Measure* m2 = s2->measure();
                        if (s2->tick() == m2->tick())
                              m2 = m2->prevMeasure();
                        volta->setEndMeasure(m2);
                        s1->measure()->add(s);
                        int n = volta->spannerSegments().size();
                        if (n) {
                              // volta->setYoff(-styleS(ST_voltaHook).val());
                              // LineSegment* ls = volta->segmentAt(0);
                              // ls->setReadPos(QPointF());
                              }
                        }
                  else {
                        s->setStartElement(s1);
                        s->setEndElement(s2);
                        s1->add(s);
                        }
                  }

            if (s->type() == Element::OTTAVA) {
                  // fix ottava position
                  Ottava* volta = static_cast<Ottava*>(s);
                  int n = volta->spannerSegments().size();
                  for (int i = 0; i < n; ++i) {
                        LineSegment* seg = volta->segmentAt(i);
                        if (!seg->userOff().isNull())
                              seg->setUserYoffset(seg->userOff().y() - styleP(ST_ottavaY));
                        }
                  }
            }
      connectTies();

      //
      // remove "middle beam" flags from first ChordRest in
      // measure
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            int tracks = nstaves() * VOICES;
            for (int track = 0; track < tracks; ++track) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->segmentType() != Segment::SegChordRest)
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (cr) {
                              switch(cr->beamMode()) {
                                    case BeamMode::AUTO:
                                    case BeamMode::BEGIN:
                                    case BeamMode::END:
                                    case BeamMode::NONE:
                                          break;
                                    case BeamMode::MID:
                                    case BeamMode::BEGIN32:
                                    case BeamMode::BEGIN64:
                                          cr->setBeamMode(BeamMode::BEGIN);
                                          break;
                                    case BeamMode::INVALID:
                                          if (cr->type() == Element::CHORD)
                                                cr->setBeamMode(BeamMode::AUTO);
                                          else
                                                cr->setBeamMode(BeamMode::NONE);
                                          break;
                                    }
                              break;
                              }
                        }
                  }
            }
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() == Element::VBOX) {
                  Box* b  = static_cast<Box*>(mb);
                  qreal y = point(styleS(ST_staffUpperBorder));
                  b->setBottomGap(y);
                  }
            }

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan
      //
      foreach(Staff* staff, _staves) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            }

      // adjust some styles
      if (styleS(ST_lyricsDistance).val() == MScore::baseStyle()->valueS(ST_lyricsDistance).val())
            style()->set(ST_lyricsDistance, Spatium(2.0));
      if (styleS(ST_voltaY).val() == MScore::baseStyle()->valueS(ST_voltaY).val())
            style()->set(ST_voltaY, Spatium(-2.0));
      if (styleB(ST_hideEmptyStaves) == true) // http://musescore.org/en/node/16228
            style()->set(ST_dontHideStavesInFirstSystem, false);

      _showOmr = false;

      // create excerpts
      foreach(Excerpt* excerpt, _excerpts) {
            Score* nscore = ::createExcerpt(excerpt->parts());
            if (nscore) {
                  nscore->setParentScore(this);
                  nscore->setName(excerpt->title());
                  nscore->rebuildMidiMapping();
                  nscore->updateChannel();
                  nscore->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
                  nscore->doLayout();
                  excerpt->setScore(nscore);
                  }
            }

      //
      // check for soundfont,
      // add default soundfont if none found
      // (for compatibility with old scores)
      //
      bool hasSoundfont = false;
      foreach(const SyntiParameter& sp, _syntiState) {
            if (sp.name() == "soundfont") {
                  QFileInfo fi(sp.sval());
                  if (fi.exists())
                        hasSoundfont = true;
                  }
            }
      if (!hasSoundfont)
            _syntiState.append(SyntiParameter("soundfont", MScore::soundFont));

//      _mscVersion = MSCVERSION;     // for later drag & drop usage
      fixTicks();
      renumberMeasures();
      rebuildMidiMapping();
      updateChannel();
      updateNotes();    // only for parts needed?

      doLayout();

      //
      // move some elements
      //
      for (Segment* s = firstSegment(); s; s = s->next1()) {
            foreach (Element* e, s->annotations()) {
                  if (e->type() == Element::TEMPO_TEXT) {
                        // reparent from measure to segment
                        e->setUserOff(QPointF(e->userOff().x() - s->pos().x(),
                           e->userOff().y()));
                        }
                  }
            }

      return FILE_NO_ERROR;
      }
