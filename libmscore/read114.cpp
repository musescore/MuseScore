//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "sym.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   StyleVal114
//---------------------------------------------------------

struct StyleVal2 {
      StyleIdx idx;
      QVariant val;
      };

static const StyleVal2 style114[] = {
      { ST_staffUpperBorder,             QVariant(7.0) },
      { ST_staffLowerBorder,             QVariant(7.0) },
      { ST_staffDistance,                QVariant(6.5) },
      { ST_akkoladeDistance,             QVariant(6.5) },
      { ST_lyricsDistance,               QVariant(2) },
      { ST_lyricsMinBottomDistance,      QVariant(2) },
      { ST_systemFrameDistance,          QVariant(7.0) },
      { ST_frameSystemDistance,          QVariant(1.0) },
      { ST_minMeasureWidth,              QVariant(4.0) },
      { ST_barWidth,                     QVariant(0.16) },
      { ST_doubleBarWidth,               QVariant(0.16) },
      { ST_endBarWidth,                  QVariant(0.5) },
      { ST_doubleBarDistance,            QVariant(0.30) },
      { ST_endBarDistance,               QVariant(0.30) },
      { ST_repeatBarTips,                QVariant(false) },
      { ST_startBarlineSingle,           QVariant(false) },
      { ST_startBarlineMultiple,         QVariant(true) },
      { ST_bracketWidth,                 QVariant(0.35) },
      { ST_bracketDistance,              QVariant(0.25) },
      { ST_clefLeftMargin,               QVariant(0.5) },
      { ST_keysigLeftMargin,             QVariant(0.5) },
      { ST_timesigLeftMargin,            QVariant(0.5) },
      { ST_clefKeyRightMargin,           QVariant(1.75) },
      { ST_clefBarlineDistance,          QVariant(0.18) },
      { ST_stemWidth,                    QVariant(0.13) },
      { ST_shortenStem,                  QVariant(true) },
      { ST_shortStemProgression,         QVariant(0.25) },
      { ST_shortestStem,                 QVariant(2.25) },
      { ST_beginRepeatLeftMargin,        QVariant(1.0) },
      { ST_minNoteDistance,              QVariant(0.4) },
      { ST_barNoteDistance,              QVariant(1.2) },
      { ST_noteBarDistance,              QVariant(1.0) },
      { ST_measureSpacing,               QVariant(1.2) },
      { ST_staffLineWidth,               QVariant(0.08) },
      { ST_ledgerLineWidth,              QVariant(0.12) },
      { ST_akkoladeWidth,                QVariant(1.6) },
      { ST_accidentalDistance,           QVariant(0.22) },
      { ST_accidentalNoteDistance,       QVariant(0.22) },
      { ST_beamWidth,                    QVariant(0.48) },
      { ST_beamDistance,                 QVariant(0.5) },
      { ST_beamMinLen,                   QVariant(1.25) },
      { ST_dotNoteDistance,              QVariant(0.35) },
      { ST_dotRestDistance,              QVariant(0.25) },
      { ST_dotDotDistance,               QVariant(0.5) },
      { ST_propertyDistanceHead,         QVariant(1.0) },
      { ST_propertyDistanceStem,         QVariant(0.5) },
      { ST_propertyDistance,             QVariant(1.0) },
      { ST_articulationMag,              QVariant(qreal(1.0)) },
      { ST_lastSystemFillLimit,          QVariant(0.3) },
      { ST_MinTieLength,                 QVariant(0.0) },
      { ST_hairpinHeight,                QVariant(1.2) },
      { ST_hairpinContHeight,            QVariant(0.5) },
      { ST_hairpinLineWidth,             QVariant(0.13) },
      { ST_showPageNumber,               QVariant(true) },
      { ST_showPageNumberOne,            QVariant(false) },
      { ST_pageNumberOddEven,            QVariant(true) },
      { ST_showMeasureNumber,            QVariant(true) },
      { ST_showMeasureNumberOne,         QVariant(false) },
      { ST_measureNumberInterval,        QVariant(5) },
      { ST_measureNumberSystem,          QVariant(true) },
      { ST_measureNumberAllStaffs,       QVariant(false) },
      { ST_smallNoteMag,                 QVariant(qreal(0.7)) },
      { ST_graceNoteMag,                 QVariant(qreal(0.7)) },
      { ST_smallStaffMag,                QVariant(qreal(0.7)) },
      { ST_smallClefMag,                 QVariant(qreal(0.8)) },
      { ST_genClef,                      QVariant(true) },
      { ST_genKeysig,                    QVariant(true) },
      { ST_genTimesig,                   QVariant(true) },
      { ST_genCourtesyTimesig,           QVariant(true) },
      { ST_genCourtesyKeysig,            QVariant(true) },
      { ST_useStandardNoteNames,         QVariant(true) },
      { ST_useGermanNoteNames,           QVariant(false) },
      { ST_useSolfeggioNoteNames,        QVariant(false) },
      { ST_chordDescriptionFile,         QVariant(QString("stdchords.xml")) },
      { ST_chordStyle,                   QVariant(QString("custom")) },
      { ST_chordsXmlFile,                QVariant(true) },
      { ST_harmonyY,                     QVariant(0.0) },
      { ST_concertPitch,                 QVariant(false) },
      { ST_createMultiMeasureRests,      QVariant(false) },
      { ST_minEmptyMeasures,             QVariant(2) },
      { ST_minMMRestWidth,               QVariant(4.0) },
      { ST_hideEmptyStaves,              QVariant(false) },
      { ST_gateTime,                     QVariant(100) },
      { ST_tenutoGateTime,               QVariant(100) },
      { ST_staccatoGateTime,             QVariant(50) },
      { ST_slurGateTime,                 QVariant(100) },
      { ST_ArpeggioNoteDistance,         QVariant(.5) },
      { ST_ArpeggioLineWidth,            QVariant(.18) },
      { ST_ArpeggioHookLen,              QVariant(.8) },
      { ST_FixMeasureNumbers,            QVariant(0) },
      { ST_FixMeasureWidth,              QVariant(false) },
      { ST_keySigNaturals,               QVariant(NAT_BEFORE) },
      { ST_tupletMaxSlope,               QVariant(qreal(0.5)) },
      { ST_tupletOufOfStaff,             QVariant(false) },
      { ST_tupletVHeadDistance,          QVariant(.5) },
      { ST_tupletVStemDistance,          QVariant(.25) },
      { ST_tupletStemLeftDistance,       QVariant(.5) },
      { ST_tupletStemRightDistance,      QVariant(.5) },
      { ST_tupletNoteLeftDistance,       QVariant(0.0) },
      { ST_tupletNoteRightDistance,      QVariant(0.0) },
      { ST_hideInstrumentNameIfOneInstrument, QVariant(false) }
      };


//---------------------------------------------------------
//   Staff::read114
//---------------------------------------------------------

void Staff::read114(XmlReader& e)
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
            else if (tag == "cleflist") {
                  clefs.read(e, _score);
                  if (clefs.empty()) {
                        ClefType ct = Clef::clefType("0");
                        clefs.setClef(0, ClefTypeList(ct, ct));
                        }
                  }
            else if (tag == "keylist")
                  _keymap.read(e, _score);
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
                  staff->read114(e);
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
                  instr(0)->setLongName(t->text());
                  delete t;
                  }
            else if (tag == "shortName") {
                  Text* t = new Text(score());
                  t->read(e);
                  instr(0)->setShortName(t->text());
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
                  staff->setStaffType(score()->staffType(PERC_DEFAULT_STAFF_TYPE));
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
            style()->set(style114[i].idx, style114[i].val);

      // old text style defaults
      TextStyle ts = style()->textStyle("Chord Symbol");
      ts.setYoff(-4.0);
      style()->setTextStyle(ts);

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
            else if (tag == "SyntiSettings")
                  _synthesizerState.read(e);
            else if (tag == "Spatium")
                  _style.setSpatium (e.readDouble() * MScore::DPMM);
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

                  qreal spMM = _style.spatium() / MScore::DPMM;
                  if (s.frameWidthMM() != 0.0)
                        s.setFrameWidth(Spatium(s.frameWidthMM() / spMM));
                  if (s.paddingWidthMM() != 0.0)
                        s.setPaddingWidth(Spatium(s.paddingWidthMM() / spMM));
\
                  // convert 1.2 text styles
                  if (s.name() == "Chordname")
                        s.setName("Chord Symbol");
                  else if (s.name() == "Lyrics odd lines")
                        s.setName("Lyrics Odd Lines");
                  else if (s.name() == "Lyrics even lines")
                        s.setName("Lyrics Even Lines");
                  else if (s.name() == "InstrumentsLong")
                        s.setName("Instrument Name (Long)");
                  else if (s.name() == "InstrumentsShort")
                        s.setName("Instrument Name (Short)");
                  else if (s.name() == "InstrumentsExcerpt")
                        s.setName("Instrument Name (Part)");
                  else if (s.name() == "Poet")
                        s.setName("Lyricist");
                  else if (s.name() == "Technik")
                        s.setName("Technique");
                  else if (s.name() == "TextLine")
                        s.setName("Text Line");
                  else if (s.name() == "Tuplets")
                        s.setName("Tuplet");

                  if (s.name() == "Lyrics Odd Lines" || s.name() == "Lyrics Even Lines")
                        s.setAlign((s.align() & ~ ALIGN_VMASK) | Align(ALIGN_BASELINE));

                  _style.setTextStyle(s);
                  }
            else if (tag == "page-layout") {
                  if (_layoutMode != LayoutFloat && _layoutMode != LayoutSystem) {
                        PageFormat pf;
                        pf.copy(*pageFormat());
                        pf.read(e, this);
                        setPageFormat(pf);
                        }
                  else
                        e.skipCurrentElement();
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(this);
                  text->read(e);
                  text->layout();
                  setMetaTag("copyright", text->plainText());
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
                  addSpanner(slur);
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
                  if (s->tick() == -1)
                        s->setTick(e.tick());
                  else
                        e.setTick(s->tick());      // update current tick
                  if (s->track2() == -1)
                        s->setTrack2(s->track());
                  if (s->tick2() == -1) {
                        delete s;
                        qDebug("invalid spanner %s tick2: %d", s->name(), s->tick2());
                        }
                  else {
                        addSpanner(s);
                        }
                  }
            else if (tag == "Excerpt") {
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        Excerpt* ex = new Excerpt(this);
                        ex->read(e);
                        _excerpts.append(ex);
                        }
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

      if (e.error() != QXmlStreamReader::NoError)
            return FILE_BAD_FORMAT;

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
            // first clef can be implicit in 1.3 #22607
            if(s->clefList()->count(0) == 0) {
                  Segment* seg = firstMeasure()->getSegment(Segment::SegClef, 0);
                  ClefType ct = Clef::clefType("0");
                  Clef* clef = new Clef(this);
                  clef->setClefType(ct);
                  clef->setTrack(track);
                  clef->setParent(seg);
                  clef->setGenerated(false);
                  seg->add(clef);
                  }
            for (auto i = s->clefList()->cbegin(); i != s->clefList()->cend(); ++i) {
                  int tick = i->first;
                  ClefType clefId = i->second._concertClef;
                  Measure* m = tick2measure(tick);
                  if (!m)
                        continue;
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
                  if(!m) //empty score
                        break;
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

      for (std::pair<int,Spanner*> p : spanner()) {
            Spanner* s = p.second;
            if (s->type() != Element::SLUR) {
                  if (s->type() == Element::VOLTA) {
                        Volta* volta = static_cast<Volta*>(s);
                        volta->setAnchor(Spanner::ANCHOR_MEASURE);
                        }
                  }

            if (s->type() == Element::OTTAVA || s->type() == Element::PEDAL || s->type() == Element::TRILL) {
                  qreal yo = 0;
                  if (s->type() == Element::OTTAVA) {
                      // fix ottava position
                      Ottava* ottava = static_cast<Ottava*>(s);
                      ottava->staff()->updateOttava(ottava);
                      yo = styleS(ST_ottavaY).val() * spatium();
                      if (s->placement() == Element::BELOW)
                            yo = -yo + s->staff()->height();
                      }
                  else if (s->type() == Element::PEDAL) {
                        yo = styleS(ST_pedalY).val() * spatium();
                        }
                  else if (s->type() == Element::TRILL) {
                        yo = styleS(ST_trillY).val() * spatium();
                        }
                  for (SpannerSegment* seg : s->spannerSegments()) {
                        if (!seg->userOff().isNull())
                              seg->setUserYoffset(seg->userOff().y() - yo);
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
            bool first = true;
            for (int track = 0; track < tracks; ++track) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->segmentType() != Segment::SegChordRest)
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (cr) {
                              if(cr->type() == Element::REST) {
                                    Rest* r = static_cast<Rest*>(cr);
                                    if (!r->userOff().isNull()) {
                                          int lineOffset = r->computeLineOffset();
                                          qreal lineDist = r->staff() ? r->staff()->staffType()->lineDistance().val() : 1.0;
                                          r->rUserYoffset() -= (lineOffset * .5 * lineDist * r->spatium());
                                          }
                                    }
                              if(!first) {
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
                                    first = false;
                                    }
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
      if (style(ST_lyricsDistance) == MScore::baseStyle()->value(ST_lyricsDistance))
            style()->set(ST_lyricsDistance, 2.0f);
      if (style(ST_voltaY) == MScore::baseStyle()->value(ST_voltaY))
            style()->set(ST_voltaY, -2.0f);
      if (style(ST_hideEmptyStaves).toBool()) // http://musescore.org/en/node/16228
            style()->set(ST_dontHideStavesInFirstSystem, false);
      if (style(ST_useGermanNoteNames).toBool())
            style()->set(ST_useStandardNoteNames, false);
      if (style(ST_showPageNumberOne).toBool()) { // http://musescore.org/en/node/21207
            style()->set(ST_evenFooterL, QString("$P"));
            style()->set(ST_oddFooterR, QString("$P"));
            }
      if (style(ST_minEmptyMeasures).toInt() == 0)
            style()->set(ST_minEmptyMeasures, 1);

      _showOmr = false;

      // create excerpts
      foreach (Excerpt* excerpt, _excerpts) {
            if (excerpt->parts().isEmpty()) {         // ignore empty parts
                  _excerpts.removeOne(excerpt);
                  continue;
                  }
            Score* nscore = Ms::createExcerpt(excerpt->parts());
            if (nscore) {
                  nscore->setName(excerpt->title());
                  nscore->rebuildMidiMapping();
                  nscore->updateChannel();
                  nscore->updateNotes();
                  nscore->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
                  nscore->doLayout();
                  excerpt->setScore(nscore);
                  }
            }

//      _mscVersion = MSCVERSION;     // for later drag & drop usage
      fixTicks();
      rebuildMidiMapping();
      updateChannel();
      updateNotes();    // only for parts needed?

      return FILE_NO_ERROR;
      }

}

