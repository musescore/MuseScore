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
#include "stringdata.h"
#include "tempo.h"
#include "tempotext.h"
#include "clef.h"
#include "barline.h"

namespace Ms {

static int g_guitarStrings[] = {40,45,50,55,59,64};
static int g_bassStrings[]   = {28,33,38,43};
static int g_violinStrings[] = {55,62,69,76};
static int g_violaStrings[]  = {48,55,62,69};
static int g_celloStrings[]  = {36,43,50,57};

//---------------------------------------------------------
//   StyleVal114
//---------------------------------------------------------

struct StyleVal2 {
      StyleIdx idx;
      QVariant val;
      };

static const StyleVal2 style114[] = {
      { StyleIdx::staffUpperBorder,             QVariant(7.0) },
      { StyleIdx::staffLowerBorder,             QVariant(7.0) },
      { StyleIdx::staffDistance,                QVariant(6.5) },
      { StyleIdx::akkoladeDistance,             QVariant(6.5) },
      { StyleIdx::lyricsDistance,               QVariant(2) },
      { StyleIdx::lyricsMinBottomDistance,      QVariant(2) },
      { StyleIdx::systemFrameDistance,          QVariant(7.0) },
      { StyleIdx::frameSystemDistance,          QVariant(1.0) },
      { StyleIdx::minMeasureWidth,              QVariant(4.0) },
      { StyleIdx::barWidth,                     QVariant(0.16) },
      { StyleIdx::doubleBarWidth,               QVariant(0.16) },
      { StyleIdx::endBarWidth,                  QVariant(0.5) },
      { StyleIdx::doubleBarDistance,            QVariant(0.30) },
      { StyleIdx::endBarDistance,               QVariant(0.30) },
      { StyleIdx::repeatBarTips,                QVariant(false) },
      { StyleIdx::startBarlineSingle,           QVariant(false) },
      { StyleIdx::startBarlineMultiple,         QVariant(true) },
      { StyleIdx::bracketWidth,                 QVariant(0.35) },
      { StyleIdx::bracketDistance,              QVariant(0.25) },
      { StyleIdx::clefLeftMargin,               QVariant(0.5) },
      { StyleIdx::keysigLeftMargin,             QVariant(0.5) },
      { StyleIdx::timesigLeftMargin,            QVariant(0.5) },
      { StyleIdx::clefKeyRightMargin,           QVariant(1.75) },
      { StyleIdx::clefBarlineDistance,          QVariant(0.18) },
      { StyleIdx::stemWidth,                    QVariant(0.13) },
      { StyleIdx::shortenStem,                  QVariant(true) },
      { StyleIdx::shortStemProgression,         QVariant(0.25) },
      { StyleIdx::shortestStem,                 QVariant(2.25) },
      { StyleIdx::beginRepeatLeftMargin,        QVariant(1.0) },
      { StyleIdx::minNoteDistance,              QVariant(0.4) },
      { StyleIdx::barNoteDistance,              QVariant(1.2) },
      { StyleIdx::noteBarDistance,              QVariant(1.0) },
      { StyleIdx::measureSpacing,               QVariant(1.2) },
      { StyleIdx::staffLineWidth,               QVariant(0.08) },
      { StyleIdx::ledgerLineWidth,              QVariant(0.12) },
      { StyleIdx::akkoladeWidth,                QVariant(1.6) },
      { StyleIdx::accidentalDistance,           QVariant(0.22) },
      { StyleIdx::accidentalNoteDistance,       QVariant(0.22) },
      { StyleIdx::beamWidth,                    QVariant(0.48) },
      { StyleIdx::beamDistance,                 QVariant(0.5) },
      { StyleIdx::beamMinLen,                   QVariant(1.25) },
      { StyleIdx::dotNoteDistance,              QVariant(0.35) },
      { StyleIdx::dotRestDistance,              QVariant(0.25) },
      { StyleIdx::dotDotDistance,               QVariant(0.5) },
      { StyleIdx::propertyDistanceHead,         QVariant(1.0) },
      { StyleIdx::propertyDistanceStem,         QVariant(0.5) },
      { StyleIdx::propertyDistance,             QVariant(1.0) },
      { StyleIdx::articulationMag,              QVariant(qreal(1.0)) },
      { StyleIdx::lastSystemFillLimit,          QVariant(0.3) },
//      { StyleIdx::MinTieLength,                 QVariant(1.0) }, // use default value
      { StyleIdx::hairpinHeight,                QVariant(1.2) },
      { StyleIdx::hairpinContHeight,            QVariant(0.5) },
      { StyleIdx::hairpinLineWidth,             QVariant(0.13) },
      { StyleIdx::showPageNumber,               QVariant(true) },
      { StyleIdx::showPageNumberOne,            QVariant(false) },
      { StyleIdx::pageNumberOddEven,            QVariant(true) },
      { StyleIdx::showMeasureNumber,            QVariant(true) },
      { StyleIdx::showMeasureNumberOne,         QVariant(false) },
      { StyleIdx::measureNumberInterval,        QVariant(5) },
      { StyleIdx::measureNumberSystem,          QVariant(true) },
      { StyleIdx::measureNumberAllStaffs,       QVariant(false) },
      { StyleIdx::smallNoteMag,                 QVariant(qreal(0.7)) },
      { StyleIdx::graceNoteMag,                 QVariant(qreal(0.7)) },
      { StyleIdx::smallStaffMag,                QVariant(qreal(0.7)) },
      { StyleIdx::smallClefMag,                 QVariant(qreal(0.8)) },
      { StyleIdx::genClef,                      QVariant(true) },
      { StyleIdx::genKeysig,                    QVariant(true) },
//      { StyleIdx::genTimesig,                   QVariant(true) },
      { StyleIdx::genCourtesyTimesig,           QVariant(true) },
      { StyleIdx::genCourtesyKeysig,            QVariant(true) },
      { StyleIdx::useStandardNoteNames,         QVariant(true) },
      { StyleIdx::useGermanNoteNames,           QVariant(false) },
      { StyleIdx::useFullGermanNoteNames,       QVariant(false) },
      { StyleIdx::useSolfeggioNoteNames,        QVariant(false) },
      { StyleIdx::useFrenchNoteNames,           QVariant(false) },
      { StyleIdx::chordDescriptionFile,         QVariant(QString("stdchords.xml")) },
      { StyleIdx::chordStyle,                   QVariant(QString("custom")) },
      { StyleIdx::chordsXmlFile,                QVariant(true) },
      { StyleIdx::harmonyY,                     QVariant(0.0) },
      { StyleIdx::concertPitch,                 QVariant(false) },
      { StyleIdx::createMultiMeasureRests,      QVariant(false) },
      { StyleIdx::minEmptyMeasures,             QVariant(2) },
      { StyleIdx::minMMRestWidth,               QVariant(4.0) },
      { StyleIdx::hideEmptyStaves,              QVariant(false) },
      { StyleIdx::gateTime,                     QVariant(100) },
      { StyleIdx::tenutoGateTime,               QVariant(100) },
      { StyleIdx::staccatoGateTime,             QVariant(50) },
      { StyleIdx::slurGateTime,                 QVariant(100) },
      { StyleIdx::ArpeggioNoteDistance,         QVariant(.5) },
      { StyleIdx::ArpeggioLineWidth,            QVariant(.18) },
      { StyleIdx::ArpeggioHookLen,              QVariant(.8) },
      { StyleIdx::keySigNaturals,               QVariant(int(KeySigNatural::BEFORE)) },
      { StyleIdx::tupletMaxSlope,               QVariant(qreal(0.5)) },
      { StyleIdx::tupletOufOfStaff,             QVariant(false) },
      { StyleIdx::tupletVHeadDistance,          QVariant(.5) },
      { StyleIdx::tupletVStemDistance,          QVariant(.25) },
      { StyleIdx::tupletStemLeftDistance,       QVariant(.5) },
      { StyleIdx::tupletStemRightDistance,      QVariant(.5) },
      { StyleIdx::tupletNoteLeftDistance,       QVariant(0.0) },
      { StyleIdx::tupletNoteRightDistance,      QVariant(0.0) },
      { StyleIdx::hideInstrumentNameIfOneInstrument, QVariant(false) }
      };


//---------------------------------------------------------
//   Staff::read114
//---------------------------------------------------------

void Staff::read114(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "lines") {
                  int lines = e.readInt();
                  setLines(lines);
                  if (lines != 5) {
                        _barLineFrom = (lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                        _barLineTo   = (lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (lines - 1) * 2);
                        }
                  }
            else if (tag == "small")
                  setSmall(e.readInt());
            else if (tag == "invisible")
                  setInvisible(e.readInt());
            else if (tag == "slashStyle")
                  e.skipCurrentElement();
            else if (tag == "cleflist") {
                  QList<std::pair<int, ClefType>>& cl = e.clefs(idx());
                  cl.clear();
                  while (e.readNextStartElement()) {
                        if (e.name() == "clef") {
                              int tick    = e.intAttribute("tick", 0);
                              ClefType ct = Clef::clefType(e.attribute("idx", "0"));
                              cl.append(std::pair<int,ClefType>(score()->fileDivision(tick), ct));
                              e.readNext();
                              }
                        else
                              e.unknown();
                        }
                  if (cl.empty())
                        cl.append(std::pair<int,ClefType>(0, ClefType::G));
                  }
            else if (tag == "keylist")
                  _keys.read(e, _score);
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
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score);
                  staff->setPart(this);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  staff->read114(e);
                  }
            else if (tag == "Instrument") {
                  Instrument* i = instrument();
                  i->read(e);
                  // add string data from MIDI program number, if possible
                  if (i->stringData()->strings() == 0
                     && i->channel().count() > 0
                     && i->drumset() == nullptr) {
                        int program = i->channel(0)->program;
                        if (program >= 24 && program <= 30)       // guitars
                              i->setStringData(StringData(19, 6, g_guitarStrings));
                        else if ( (program >= 32 && program <= 39) || program == 43)      // bass / double-bass
                              i->setStringData(StringData(24, 4, g_bassStrings));
                        else if (program == 40)                   // violin and other treble string instr.
                              i->setStringData(StringData(24, 4, g_violinStrings));
                        else if (program == 41)                   // viola and other alto string instr.
                              i->setStringData(StringData(24, 4, g_violaStrings));
                        else if (program == 42)                   // cello and other bass string instr.
                              i->setStringData(StringData(24, 4, g_celloStrings));
                        }
                  Drumset* d = i->drumset();
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
                  instrument()->setLongName(t->xmlText());
                  delete t;
                  }
            else if (tag == "shortName") {
                  Text* t = new Text(score());
                  t->read(e);
                  instrument()->setShortName(t->xmlText());
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
            _partName = instrument()->trackName();

      if (instrument()->useDrumset()) {
            foreach(Staff* staff, _staves) {
                  int lines = staff->lines();
                  int bf    = staff->barLineFrom();
                  int bt    = staff->barLineTo();
                  staff->setStaffType(StaffType::getDefaultPreset(StaffGroup::PERCUSSION));

                  // this allows 2/3-line percussion staves to keep the double spacing they had in 1.3

                  if (lines == 2 || lines == 3)
                        staff->staffType()->setLineDistance(Spatium(2.0));

                  staff->setLines(lines);       // this also sets stepOffset
                  staff->setBarLineFrom(bf);
                  staff->setBarLineTo(bt);
                  }
            }
      //set default articulations
      QList<MidiArticulation> articulations;
      articulations.append(MidiArticulation("", "", 100, 100));
      articulations.append(MidiArticulation("staccato", "", 100, 50));
      articulations.append(MidiArticulation("tenuto", "", 100, 100));
      articulations.append(MidiArticulation("sforzato", "", 120, 100));
      instrument()->setArticulation(articulations);
      }

//---------------------------------------------------------
//   convertOldTextStyleNames
//---------------------------------------------------------

QString convertOldTextStyleNames(const QString& s)
      {
      QString rs(s);
      // convert 1.2 text styles
      if (s == "Chordname")
            rs = "Chord Symbol";
      else if (s == "Lyrics odd lines")
            rs = "Lyrics Odd Lines";
      else if (s == "Lyrics even lines")
            rs = "Lyrics Even Lines";
      else if (s == "InstrumentsLong")
            rs = "Instrument Name (Long)";
      else if (s == "InstrumentsShort")
            rs = "Instrument Name (Short)";
      else if (s == "InstrumentsExcerpt")
            rs = "Instrument Name (Part)";
      else if (s == "Poet")
            rs = "Lyricist";
      else if (s == "Technik")
            rs = "Technique";
      else if (s == "TextLine")
            rs = "Text Line";
      else if (s == "Tuplets")
            rs = "Tuplet";
      return rs;
      }

//---------------------------------------------------------
//   read114
//    import old version <= 1.3 files
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
      TempoMap tm;
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaff(e);
            else if (tag == "KeySig") {               // not supported
                  KeySig* ks = new KeySig(this);
                  ks->read(e);
                  // customKeysigs.append(ks);
                  delete ks;
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
               || tag == "Symbols"
               || tag == "cursorTrack"
               || tag == "yoff")
                  e.skipCurrentElement();       // obsolete
            else if (tag == "tempolist") {
                  // store the tempo list to create invisible tempo text later
                  qreal tempo = e.attribute("fix","2.0").toDouble();
                  tm.setRelTempo(tempo);
                  while (e.readNextStartElement()) {
                        if (e.name() == "tempo") {
                              int tick = e.attribute("tick").toInt();
                              double tmp = e.readElementText().toDouble();
                              tick = (tick * MScore::division + _fileDivision/2) / _fileDivision;
                              auto pos = tm.find(tick);
                              if (pos != tm.end())
                                    tm.erase(pos);
                              tm.setTempo(tick, tmp);
                        }
                        else if (e.name() == "relTempo")
                              e.readElementText();
                        else
                              e.unknown();
                  }
            }
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
                  // adjust this now so chords render properly on read
                  // other style adjustments can wait until reading is finished
                  if (style(StyleIdx::useGermanNoteNames).toBool())
                        style()->set(StyleIdx::useStandardNoteNames, false);
                  if (_layoutMode == LayoutMode::FLOAT) {
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
                  s.setName(convertOldTextStyleNames(s.name()));

                  if (s.name() == "Lyrics Odd Lines" || s.name() == "Lyrics Even Lines")
                        s.setAlign((s.align() & ~ Align(AlignmentFlags::VMASK)) | AlignmentFlags::BASELINE);

                  _style.setTextStyle(s);
                  }
            else if (tag == "page-layout") {
                  if (_layoutMode != LayoutMode::FLOAT && _layoutMode != LayoutMode::SYSTEM) {
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
                        e.initTick(s->tick());      // update current tick
                  if (s->track2() == -1)
                        s->setTrack2(s->track());
                  if (s->ticks() == 0) {
                        delete s;
                        qDebug("zero spanner %s ticks: %d", s->name(), s->ticks());
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

      if (e.error() != XmlStreamReader::NoError)
            return FileError::FILE_BAD_FORMAT;

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
            for (auto i : e.clefs(idx)) {
                  int tick = i.first;
                  ClefType clefId = i.second;
                  Measure* m = tick2measure(tick);
                  if (!m)
                        continue;
                  if ((tick == m->tick()) && m->prevMeasure())
                        m = m->prevMeasure();
                  Segment* seg = m->getSegment(Segment::Type::Clef, tick);
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

            // create missing KeySig
            KeyList* km = s->keyList();
            for (auto i = km->begin(); i != km->end(); ++i) {
                  int tick = i->first;
                  if (tick < 0) {
                        qDebug("read114: Key tick %d", tick);
                        continue;
                        }
                  if (tick == 0 && i->second.key() == Key::C)
                        continue;
                  Measure* m = tick2measure(tick);
                  if (!m)           //empty score
                        break;
                  Segment* seg = m->getSegment(Segment::Type::KeySig, tick);
                  if (seg->element(track))
                        static_cast<KeySig*>(seg->element(track))->setGenerated(false);
                  else {
                        KeySigEvent ke = i->second;
                        KeySig* ks = new KeySig(this);
                        ks->setKeySigEvent(ke);
                        ks->setParent(seg);
                        ks->setTrack(track);
                        ks->setGenerated(false);
                        seg->add(ks);
                        }
                  }
            }

      for (std::pair<int,Spanner*> p : spanner()) {
            Spanner* s = p.second;
            if (s->type() != Element::Type::SLUR) {
                  if (s->type() == Element::Type::VOLTA) {
                        Volta* volta = static_cast<Volta*>(s);
                        volta->setAnchor(Spanner::Anchor::MEASURE);
                        }
                  }

            if (s->type() == Element::Type::OTTAVA
                || s->type() == Element::Type::PEDAL
                || s->type() == Element::Type::TRILL
                || s->type() == Element::Type::TEXTLINE) {
                  qreal yo = 0;
                  if (s->type() == Element::Type::OTTAVA) {
                      // fix ottava position
                      yo = styleS(StyleIdx::ottavaY).val() * spatium();
                      if (s->placement() == Element::Placement::BELOW)
                            yo = -yo + s->staff()->height();
                      }
                  else if (s->type() == Element::Type::PEDAL) {
                        yo = styleS(StyleIdx::pedalY).val() * spatium();
                        }
                  else if (s->type() == Element::Type::TRILL) {
                        yo = styleS(StyleIdx::trillY).val() * spatium();
                        }
                  else if (s->type() == Element::Type::TEXTLINE) {
                        yo = -5.0 * spatium();
                  }
                  if (!s->spannerSegments().isEmpty()) {
                        for (SpannerSegment* seg : s->spannerSegments()) {
                              if (!seg->userOff().isNull())
                                    seg->setUserYoffset(seg->userOff().y() - yo);
                              }
                        }
                  else {
                        s->setUserYoffset(-yo);
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
                        if (s->segmentType() != Segment::Type::ChordRest)
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (cr) {
                              if(cr->type() == Element::Type::REST) {
                                    Rest* r = static_cast<Rest*>(cr);
                                    if (!r->userOff().isNull()) {
                                          int lineOffset = r->computeLineOffset();
                                          qreal lineDist = r->staff() ? r->staff()->staffType()->lineDistance().val() : 1.0;
                                          r->rUserYoffset() -= (lineOffset * .5 * lineDist * r->spatium());
                                          }
                                    }
                              if(!first) {
                                    switch(cr->beamMode()) {
                                          case Beam::Mode::AUTO:
                                          case Beam::Mode::BEGIN:
                                          case Beam::Mode::END:
                                          case Beam::Mode::NONE:
                                                break;
                                          case Beam::Mode::MID:
                                          case Beam::Mode::BEGIN32:
                                          case Beam::Mode::BEGIN64:
                                                cr->setBeamMode(Beam::Mode::BEGIN);
                                                break;
                                          case Beam::Mode::INVALID:
                                                if (cr->type() == Element::Type::CHORD)
                                                      cr->setBeamMode(Beam::Mode::AUTO);
                                                else
                                                      cr->setBeamMode(Beam::Mode::NONE);
                                                break;
                                          }
                                    first = false;
                                    }
                              }
                        }
                  }
            }
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() == Element::Type::VBOX) {
                  Box* b  = static_cast<Box*>(mb);
                  qreal y = point(styleS(StyleIdx::staffUpperBorder));
                  b->setBottomGap(y);
                  }
            }

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan and update ottavas
      //
      foreach(Staff* staff, _staves) {
            int barLineSpan = staff->barLineSpan();
            int idx = staffIdx(staff);
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            staff->updateOttava();
            }

      // adjust some styles
      qreal lmbd = styleD(StyleIdx::lyricsMinBottomDistance);
      style()->set(StyleIdx::lyricsMinBottomDistance, lmbd + 4.0);
      if (style(StyleIdx::hideEmptyStaves).toBool()) // http://musescore.org/en/node/16228
            style()->set(StyleIdx::dontHideStavesInFirstSystem, false);
      if (style(StyleIdx::showPageNumberOne).toBool()) { // http://musescore.org/en/node/21207
            style()->set(StyleIdx::evenFooterL, QString("$P"));
            style()->set(StyleIdx::oddFooterR, QString("$P"));
            }
      if (style(StyleIdx::minEmptyMeasures).toInt() == 0)
            style()->set(StyleIdx::minEmptyMeasures, 1);
      // hack: net overall effect of layout changes has been for things to take slightly more room
      qreal adjustedSpacing = qMax(styleD(StyleIdx::measureSpacing) * 0.95, 1.0);
      style()->set(StyleIdx::measureSpacing, adjustedSpacing);

      _showOmr = false;

      // add invisible tempo text if necessary
      // some 1.3 scores have tempolist but no tempo text
      fixTicks();
      for (auto i = tm.begin(); i != tm.end(); ++i) {
            int tick = i->first;
            qreal tempo = i->second.tempo;
            if (tempomap()->tempo(tick) != tempo) {
                  TempoText* tt = new TempoText(this);
                  tt->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(qRound(tempo*60)));
                  tt->setTempo(tempo);
                  tt->setTrack(0);
                  tt->setVisible(false);
                  Measure* m = tick2measure(tick);
                  if (m) {
                        Segment* seg = m->getSegment(Segment::Type::ChordRest, tick);
                        seg->add(tt);
                        setTempo(tick, tempo);
                        }
                  else {
                        delete tt;
                        }
                  }
            }

      // create excerpts

      foreach (Excerpt* excerpt, _excerpts) {
            if (excerpt->parts().isEmpty()) {         // ignore empty parts
                  _excerpts.removeOne(excerpt);
                  continue;
                  }
            if (!excerpt->parts().isEmpty()) {
                  Score* nscore = new Score(this);
                  excerpt->setPartScore(nscore);
                  nscore->setName(excerpt->title());
                  nscore->style()->set(StyleIdx::createMultiMeasureRests, true);
                  Ms::createExcerpt(excerpt);
                  }
            }

      // volta offsets in older scores are hardcoded to be relative to a voltaY of -2.0sp
      // we'll force this and live with it for the score
      // but we wait until now to do it so parts don't have this issue
      if (style(StyleIdx::voltaY) == MScore::baseStyle()->value(StyleIdx::voltaY))
            style()->set(StyleIdx::voltaY, -2.0f);

      fixTicks();
      rebuildMidiMapping();
      updateChannel();

      // treat reading a 1.2 file as import
      // on save warn if old file will be overwritten
      setCreated(true);

      return FileError::FILE_NO_ERROR;
      }

}

