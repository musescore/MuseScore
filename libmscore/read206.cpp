//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "xml.h"
#include "score.h"
#include "staff.h"
#include "revisions.h"
#include "part.h"
#include "page.h"
#include "style.h"
#include "sym.h"
#include "audio.h"
#include "sig.h"
#include "barline.h"
#include "measure.h"
#include "ambitus.h"
#include "tuplet.h"
#include "systemdivider.h"
#include "spacer.h"
#include "keysig.h"
#include "stafftext.h"
#include "dynamic.h"
#include "drumset.h"
#include "timesig.h"
#include "slur.h"
#include "chord.h"
#include "rest.h"
#include "breath.h"
#include "repeat.h"
#include "utils.h"
#include "read206.h"
#include "excerpt.h"
#include "articulation.h"
#include "elementlayout.h"
#include "volta.h"
#include "pedal.h"
#include "hairpin.h"
#include "ottava.h"
#include "trill.h"
#include "rehearsalmark.h"
#include "box.h"
#include "textframe.h"
#include "fermata.h"
#include "stem.h"
#include "lyrics.h"
#include "tempotext.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif


namespace Ms {

//---------------------------------------------------------
//   StyleVal206
//---------------------------------------------------------

struct StyleVal2 {
            Sid idx;
            QVariant val;
            };
      static const StyleVal2 style206[] = {
      { Sid::staffUpperBorder,            Spatium(7.0)  },
      { Sid::staffLowerBorder,            Spatium(7.0)  },
      { Sid::staffDistance,               Spatium(6.5)  },
      { Sid::akkoladeDistance,            Spatium(6.5)  },
      { Sid::minSystemDistance,           Spatium(8.5)  },
      { Sid::maxSystemDistance,           Spatium(15.0) },

      { Sid::lyricsMinBottomDistance,     Spatium(4.0)  },
      { Sid::lyricsLineHeight,            QVariant(1.0) },
      { Sid::figuredBassFontFamily,       QVariant(QString("MScoreBC")) },
      { Sid::figuredBassFontSize,         QVariant(8.0) },
      { Sid::figuredBassYOffset,          QVariant(6.0) },
      { Sid::figuredBassLineHeight,       QVariant(1.0) },
      { Sid::figuredBassAlignment,        QVariant(0) },
      { Sid::figuredBassStyle,            QVariant(0) },
      { Sid::systemFrameDistance,         Spatium(7.0) },
      { Sid::frameSystemDistance,         Spatium(7.0) },
      { Sid::minMeasureWidth,             Spatium(5.0) },
      { Sid::barWidth,                    Spatium(0.16) },      // 0.1875
      { Sid::doubleBarWidth,              Spatium(0.16) },
      { Sid::endBarWidth,                 Spatium(0.5) },       // 0.5
      { Sid::doubleBarDistance,           Spatium(0.46) },     // 0.3 + doubleBarWidth
      { Sid::endBarDistance,              Spatium(0.65) },     // 0.3
      { Sid::repeatBarTips,               QVariant(false) },
      { Sid::startBarlineSingle,          QVariant(false) },
      { Sid::startBarlineMultiple,        QVariant(true) },
      { Sid::bracketWidth,                Spatium(0.45) },
      { Sid::bracketDistance,             Spatium(0.1) },
      { Sid::akkoladeWidth,               Spatium(1.6) },
      { Sid::akkoladeBarDistance,         Spatium(.4) },
      { Sid::clefLeftMargin,              Spatium(0.64) },
      { Sid::keysigLeftMargin,            Spatium(0.5) },
      { Sid::timesigLeftMargin,           Spatium(0.5) },
      { Sid::clefKeyRightMargin,          Spatium(1.75) },
      { Sid::clefBarlineDistance,         Spatium(0.5) },
      { Sid::stemWidth,                   Spatium(0.13) },      // 0.09375
      { Sid::shortenStem,                 QVariant(true) },
      { Sid::shortStemProgression,        Spatium(0.25) },
      { Sid::shortestStem,                Spatium(2.25) },
      { Sid::beginRepeatLeftMargin,       Spatium(1.0) },
      { Sid::minNoteDistance,             Spatium(0.25) },      // 0.4
      { Sid::barNoteDistance,             Spatium(1.2) },
      { Sid::barAccidentalDistance,       Spatium(.3) },
      { Sid::multiMeasureRestMargin,      Spatium(1.2) },
      { Sid::noteBarDistance,             Spatium(1.0) },
      { Sid::measureSpacing,              QVariant(1.2) },
      { Sid::staffLineWidth,              Spatium(0.08) },      // 0.09375
      { Sid::ledgerLineWidth,             Spatium(0.16) },     // 0.1875
      { Sid::ledgerLineLength,            Spatium(.6) },     // notehead width + this value
      { Sid::accidentalDistance,          Spatium(0.22) },
      { Sid::accidentalNoteDistance,      Spatium(0.22) },
      { Sid::beamWidth,                   Spatium(0.5) },           // was 0.48
      { Sid::beamDistance,                QVariant(0.5) },          // 0.25sp
      { Sid::beamMinLen,                  QVariant(1.32) },      // 1.316178 exactly notehead width
      { Sid::beamNoSlope,                 QVariant(false) },
      { Sid::dotMag,                      QVariant(1.0) },
      { Sid::dotNoteDistance,             QVariant(0.35) },
      { Sid::dotRestDistance,             QVariant(0.25) },
      { Sid::dotDotDistance,              QVariant(0.5) },
      { Sid::propertyDistanceHead,        QVariant(1.0) },
      { Sid::propertyDistanceStem,        QVariant(1.8) },
      { Sid::propertyDistance,            QVariant(1.0) },
      { Sid::articulationMag,             QVariant(1.0) },
      { Sid::lastSystemFillLimit,         QVariant(0.3) },
      { Sid::hairpinPosBelow,             QVariant(3.5) },
      { Sid::hairpinHeight,               QVariant(1.2) },
      { Sid::hairpinContHeight,           QVariant(0.5) },
      { Sid::hairpinLineWidth,            QVariant(0.13) },
      { Sid::pedalPosBelow,               QVariant(4) },
      { Sid::pedalLineWidth,              QVariant(.15) },
      { Sid::pedalLineStyle,              QVariant(int(Qt::SolidLine)) },
      { Sid::trillPosAbove,               QVariant(-1) },
      { Sid::harmonyY,                    QVariant(2.5) },
      { Sid::harmonyFretDist,             QVariant(0.5) },
      { Sid::minHarmonyDistance,          QVariant(0.5) },
      { Sid::maxHarmonyBarDistance,       QVariant(3.0) },
      { Sid::capoPosition,                QVariant(0) },
      { Sid::fretNumMag,                  QVariant(2.0) },
      { Sid::fretNumPos,                  QVariant(0) },
      { Sid::fretY,                       QVariant(2.0) },
      { Sid::showPageNumber,              QVariant(true) },
      { Sid::showPageNumberOne,           QVariant(false) },
      { Sid::pageNumberOddEven,           QVariant(true) },
      { Sid::showMeasureNumber,           QVariant(true) },
      { Sid::showMeasureNumberOne,        QVariant(false) },
      { Sid::measureNumberInterval,       QVariant(5) },
      { Sid::measureNumberSystem,         QVariant(true) },
      { Sid::measureNumberAllStaffs,      QVariant(false) },
      { Sid::smallNoteMag,                QVariant(.7) },
      { Sid::graceNoteMag,                QVariant(0.7) },
      { Sid::smallStaffMag,               QVariant(0.7) },
      { Sid::smallClefMag,                QVariant(0.8) },
      { Sid::genClef,                     QVariant(true) },
      { Sid::genKeysig,                   QVariant(true) },
      { Sid::genCourtesyTimesig,          QVariant(true) },
      { Sid::genCourtesyKeysig,           QVariant(true) },
      { Sid::genCourtesyClef,             QVariant(true) },
      { Sid::swingRatio,                  QVariant(60)   },
      { Sid::swingUnit,                   QVariant(QString("")) },
      { Sid::useStandardNoteNames,        QVariant(true) },
      { Sid::useGermanNoteNames,          QVariant(false) },
      { Sid::useFullGermanNoteNames,      QVariant(false) },
      { Sid::useSolfeggioNoteNames,       QVariant(false) },
      { Sid::useFrenchNoteNames,          QVariant(false) },
      { Sid::automaticCapitalization,     QVariant(true) },
      { Sid::lowerCaseMinorChords,        QVariant(false) },
      { Sid::lowerCaseBassNotes,          QVariant(false) },
      { Sid::allCapsNoteNames,            QVariant(false) },
      { Sid::chordStyle,                  QVariant(QString("std")) },
      { Sid::chordsXmlFile,               QVariant(false) },
      { Sid::chordDescriptionFile,        QVariant(QString("chords_std.xml")) },
      { Sid::concertPitch,                QVariant(false) },
      { Sid::createMultiMeasureRests,     QVariant(false) },
      { Sid::minEmptyMeasures,            QVariant(2) },
      { Sid::minMMRestWidth,              Spatium(4) },
      { Sid::hideEmptyStaves,             QVariant(false) },
      { Sid::dontHideStavesInFirstSystem, QVariant(true) },
      { Sid::hideInstrumentNameIfOneInstrument, QVariant(true) },
      { Sid::gateTime,                    QVariant(100) },
      { Sid::tenutoGateTime,              QVariant(100) },
      { Sid::staccatoGateTime,            QVariant(50) },
      { Sid::slurGateTime,                QVariant(100) },
      { Sid::ArpeggioNoteDistance,        QVariant(.5) },
      { Sid::ArpeggioLineWidth,           QVariant(.18) },
      { Sid::ArpeggioHookLen,             QVariant(.8) },
      { Sid::SlurEndWidth,                QVariant(.07) },
      { Sid::SlurMidWidth,                QVariant(.15) },
      { Sid::SlurDottedWidth,             QVariant(.1) },
      { Sid::MinTieLength,                QVariant(1.0) },
      { Sid::SectionPause,                QVariant(qreal(3.0)) },
      { Sid::MusicalSymbolFont,           QVariant(QString("Emmentaler")) },
      { Sid::MusicalTextFont,             QVariant(QString("MScore Text")) },
      { Sid::showHeader,                  QVariant(false) },
      { Sid::headerFirstPage,             QVariant(false) },
      { Sid::headerOddEven,               QVariant(true) },
      { Sid::evenHeaderL,                 QVariant(QString()) },
      { Sid::evenHeaderC,                 QVariant(QString()) },
      { Sid::evenHeaderR,                 QVariant(QString()) },
      { Sid::oddHeaderL,                  QVariant(QString()) },
      { Sid::oddHeaderC,                  QVariant(QString()) },
      { Sid::oddHeaderR,                  QVariant(QString()) },
      { Sid::showFooter,                  QVariant(true) },
      { Sid::footerFirstPage,             QVariant(true) },
      { Sid::footerOddEven,               QVariant(true) },
      { Sid::evenFooterL,                 QVariant(QString("$p")) },
      { Sid::evenFooterC,                 QVariant(QString("$:copyright:")) },
      { Sid::evenFooterR,                 QVariant(QString()) },
      { Sid::oddFooterL,                  QVariant(QString()) },
      { Sid::oddFooterC,                  QVariant(QString("$:copyright:")) },
      { Sid::oddFooterR,                  QVariant(QString("$p")) },
      { Sid::voltaY,                      QVariant(-3.0) },
      { Sid::voltaHook,                   QVariant(1.9) },
      { Sid::voltaLineWidth,              QVariant(.1) },
      { Sid::voltaLineStyle,              QVariant(int(Qt::SolidLine)) },
      { Sid::ottavaPosAbove,              QVariant(-3.0) },
      { Sid::ottavaHookAbove,             QVariant(1.9) },
      { Sid::ottavaHookBelow,             QVariant(-1.9) },
      { Sid::ottavaLineWidth,             QVariant(.1) },
      { Sid::ottavaLineStyle,             QVariant(int(Qt::DashLine)) },
      { Sid::ottavaNumbersOnly,           true },
      { Sid::tabClef,                     QVariant(int(ClefType::TAB)) },
      { Sid::tremoloWidth,                QVariant(1.2) },  // tremolo stroke width: notehead width
      { Sid::tremoloBoxHeight,            QVariant(0.65) },
      { Sid::tremoloStrokeWidth,          QVariant(0.5) },  // was 0.35
      { Sid::tremoloDistance,             QVariant(0.8) },
      // TODO { Sid::tremoloBeamLengthMultiplier, QVariant(0.62) },
      // TODO { Sid::tremoloMaxBeamLength,        QVariant(12.0) },
      { Sid::linearStretch,               QVariant(qreal(1.5)) },
      { Sid::crossMeasureValues,          QVariant(false) },
      { Sid::keySigNaturals,              QVariant(int(KeySigNatural::NONE)) },

      { Sid::tupletMaxSlope,              QVariant(qreal(0.5)) },
      { Sid::tupletOufOfStaff,            QVariant(true) },
      { Sid::tupletVHeadDistance,         QVariant(.5) },
      { Sid::tupletVStemDistance,         QVariant(.25) },
      { Sid::tupletStemLeftDistance,      QVariant(.5) },
      { Sid::tupletStemRightDistance,     QVariant(.5) },
      { Sid::tupletNoteLeftDistance,      QVariant(0.0) },
      { Sid::tupletNoteRightDistance,     QVariant(0.0) },

      { Sid::barreLineWidth,              QVariant(1.0) },
      { Sid::fretMag,                     QVariant(1.0) },
      { Sid::scaleBarlines,               QVariant(true) },
      { Sid::barGraceDistance,            QVariant(.6) },
      { Sid::rehearsalMarkFrameSquare,    QVariant(false)  },
      { Sid::rehearsalMarkFrameRound,     QVariant(20)    },
      { Sid::dynamicsFontItalic,          QVariant(false) },
      };

//---------------------------------------------------------
//   setPageFormat
//    set Style from PageFormat
//---------------------------------------------------------

void setPageFormat(MStyle* style, const PageFormat& pf)
      {
      style->set(Sid::pageWidth,            pf.size().width());
      style->set(Sid::pageHeight,           pf.size().height());
      style->set(Sid::pagePrintableWidth,   pf.printableWidth());
      style->set(Sid::pageEvenLeftMargin,   pf.evenLeftMargin());
      style->set(Sid::pageOddLeftMargin,    pf.oddLeftMargin());
      style->set(Sid::pageEvenTopMargin,    pf.evenTopMargin());
      style->set(Sid::pageEvenBottomMargin, pf.evenBottomMargin());
      style->set(Sid::pageOddTopMargin,     pf.oddTopMargin());
      style->set(Sid::pageOddBottomMargin,  pf.oddBottomMargin());
      style->set(Sid::pageTwosided,         pf.twosided());
      }

//---------------------------------------------------------
//   initPageFormat
//    initialize PageFormat from Style
//---------------------------------------------------------

void initPageFormat(MStyle* style, PageFormat* pf)
      {
      QSizeF sz;
      sz.setWidth(style->value(Sid::pageWidth).toReal());
      sz.setHeight(style->value(Sid::pageHeight).toReal());
      pf->setSize(sz);
      pf->setPrintableWidth(style->value(Sid::pagePrintableWidth).toReal());
      pf->setEvenLeftMargin(style->value(Sid::pageEvenLeftMargin).toReal());
      pf->setOddLeftMargin(style->value(Sid::pageOddLeftMargin).toReal());
      pf->setEvenTopMargin(style->value(Sid::pageEvenTopMargin).toReal());
      pf->setEvenBottomMargin(style->value(Sid::pageEvenBottomMargin).toReal());
      pf->setOddTopMargin(style->value(Sid::pageOddTopMargin).toReal());
      pf->setOddBottomMargin(style->value(Sid::pageOddBottomMargin).toReal());
      pf->setTwosided(style->value(Sid::pageTwosided).toBool());
      }

//---------------------------------------------------------
//   readPageFormat
//---------------------------------------------------------

void readPageFormat(MStyle* style, XmlReader& e)
      {
      PageFormat pf;
      initPageFormat(style, &pf);
      pf.read(e);
      setPageFormat(style, pf);
      }

//---------------------------------------------------------
//   readTextStyle
//---------------------------------------------------------

void readTextStyle206(MStyle* style, XmlReader& e)
      {
      Spatium frameWidth(0.0);
      QString name = e.attribute("name");
      QString family = "FreeSerif";
      double size = 10;
      bool bold = false;
      bool italic = false;
      bool underline = false;
      Align align = Align::LEFT;
      bool sizeIsSpatiumDependent = true;
      bool hasFrame = false;
      Spatium paddingWidth(0.0);
      int frameRound = 0;
      QColor frameColor = QColor(0, 0, 0, 255);
      QColor foregroundColor = QColor(0, 0, 0, 255);
      QColor backgroundColor = QColor(255, 255, 255, 0);
      bool circle = false;
      bool systemFlag = false;
      QPointF offset;
      OffsetType offsetType = OffsetType::SPATIUM;
      bool offsetValid = false;
      Placement placement = Placement::ABOVE;
      bool placementValid = false;
      qreal lineWidth = -1.0;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "name")
                  name = e.readElementText();
            else if (tag == "family")
                  family = e.readElementText();
            else if (tag == "size")
                  size = e.readDouble();
            else if (tag == "bold")
                  bold = e.readInt();
            else if (tag == "italic")
                  italic = e.readInt();
            else if (tag == "underline")
                  underline = e.readInt();
            else if (tag == "align")
                  align = Align(e.readInt());
            else if (tag == "anchor")     // obsolete
                  e.skipCurrentElement();

            else if (tag == "halign") {
                  const QString& val(e.readElementText());
                  if (val == "center")
                        align = align | Align::HCENTER;
                  else if (val == "right")
                        align = align | Align::RIGHT;
                  else if (val == "left")
                        ;
                  else
                        qDebug("Text::readProperties: unknown alignment: <%s>", qPrintable(val));
                  }
            else if (tag == "valign") {
                  const QString& val(e.readElementText());
                  if (val == "center")
                        align = align | Align::VCENTER;
                  else if (val == "bottom")
                        align = align | Align::BOTTOM;
                  else if (val == "baseline")
                        align = align | Align::BASELINE;
                  else if (val == "top")
                        ;
                  else
                        qDebug("Text::readProperties: unknown alignment: <%s>", qPrintable(val));
                  }
            else if (tag == "xoffset") {
                  qreal xo = e.readDouble();
                  if (offsetType == OffsetType::ABS)
                        xo /= INCH;
                  offset.setX(xo);
                  offsetValid = true;
                  }
            else if (tag == "yoffset") {
                  qreal yo = e.readDouble();
                  if (offsetType == OffsetType::ABS)
                        yo /= INCH;
                  offset.setY(yo);
                  offsetValid = true;
                  }
            else if (tag == "rxoffset" || tag == "ryoffset")         // obsolete
                  e.readDouble();
            else if (tag == "offsetType") {
                  const QString& val(e.readElementText());
                  OffsetType ot = OffsetType::ABS;
                  if (val == "spatium" || val == "1")
                        ot = OffsetType::SPATIUM;
                  if (ot != offsetType) {
                        offsetType = ot;
                        if (ot == OffsetType::ABS)
                              offset /= INCH;  // convert spatium -> inch
                        else
                              offset *= INCH;  // convert inch -> spatium
                        }
                  }
            else if (tag == "sizeIsSpatiumDependent" || tag == "spatiumSizeDependent")
                  sizeIsSpatiumDependent = e.readInt();
            else if (tag == "frameWidth") { // obsolete
                  hasFrame = true;
                  /*frameWidthMM =*/ e.readDouble();
                  }
            else if (tag == "frameWidthS") {
                  hasFrame = true;
                  frameWidth = Spatium(e.readDouble());
                  }
            else if (tag == "frame")
                  hasFrame = e.readInt();
            else if (tag == "paddingWidth")          // obsolete
                  /*paddingWidthMM =*/ e.readDouble();
            else if (tag == "paddingWidthS")
                  paddingWidth = Spatium(e.readDouble());
            else if (tag == "frameRound")
                  frameRound = e.readInt();
            else if (tag == "frameColor")
                  frameColor = e.readColor();
            else if (tag == "foregroundColor")
                  foregroundColor = e.readColor();
            else if (tag == "backgroundColor")
                  backgroundColor = e.readColor();
            else if (tag == "circle")
                  circle = e.readInt();
            else if (tag == "systemFlag")
                  systemFlag = e.readInt();
            else if (tag == "placement") {
                  QString value(e.readElementText());
                  if (value == "above")
                        placement = Placement::ABOVE;
                  else if (value == "below")
                        placement = Placement::BELOW;
                  placementValid = true;
                  }
            else if (tag == "lineWidth")
                  lineWidth = e.readDouble();
            else
                  e.unknown();
            }
      if (family == "MuseJazz")
            family = "MuseJazz Text";

      struct StyleTable {
            const char* name;
            SubStyleId ss;
            } styleTable[] = {
            { "",                        SubStyleId::DEFAULT },
            { "Title",                   SubStyleId::TITLE },
            { "Subtitle",                SubStyleId::SUBTITLE },
            { "Composer",                SubStyleId::COMPOSER },
            { "Lyricist",                SubStyleId::POET },
//            { "Lyrics Odd Lines",        SubStyleId::LYRIC_ODD },
//            { "Lyrics Even Lines",       SubStyleId::LYRIC_EVEN },
            { "Lyrics Odd Lines",        SubStyleId::LYRIC },
            { "Lyrics Even Lines",       SubStyleId::LYRIC },
            { "Fingering",               SubStyleId::FINGERING },
            { "LH Guitar Fingering",     SubStyleId::LH_GUITAR_FINGERING },
            { "RH Guitar Fingering",     SubStyleId::RH_GUITAR_FINGERING },
            { "String Number",           SubStyleId::STRING_NUMBER },
            { "Instrument Name (Long)",  SubStyleId::INSTRUMENT_LONG },
            { "Instrument Name (Short)", SubStyleId::INSTRUMENT_SHORT },
            { "Instrument Name (Part)",  SubStyleId::INSTRUMENT_EXCERPT },
            { "Dynamics",                SubStyleId::DYNAMICS },
            { "Technique",               SubStyleId::EXPRESSION },
            { "Tempo",                   SubStyleId::TEMPO },
            { "Metronome",               SubStyleId::METRONOME },
            { "Measure Number",          SubStyleId::MEASURE_NUMBER },
            { "Translator",              SubStyleId::TRANSLATOR },
            { "Tuplet",                  SubStyleId::TUPLET },
            { "System",                  SubStyleId::SYSTEM },
            { "Staff",                   SubStyleId::STAFF },
            { "Chord Symbol",            SubStyleId::HARMONY },
            { "Rehearsal Mark",          SubStyleId::REHEARSAL_MARK },
            { "Repeat Text",             SubStyleId::REPEAT_LEFT },
            { "Repeat Text Left",        SubStyleId::REPEAT_LEFT },
            { "Repeat Text Right",       SubStyleId::REPEAT_RIGHT },
            { "Frame",                   SubStyleId::FRAME },
            { "Text Line",               SubStyleId::TEXTLINE },
            { "Glissando",               SubStyleId::GLISSANDO },
            { "Ottava",                  SubStyleId::OTTAVA },
            { "Pedal",                   SubStyleId::PEDAL },
            { "Hairpin",                 SubStyleId::HAIRPIN },
            { "Bend",                    SubStyleId::BEND },
            { "Header",                  SubStyleId::HEADER },
            { "Footer",                  SubStyleId::FOOTER },
            { "Instrument Change",       SubStyleId::INSTRUMENT_CHANGE },
            { "Figured Bass",            SubStyleId::FIGURED_BASS },
            { "Volta",                   SubStyleId::VOLTA },
            };
      SubStyleId ss = SubStyleId::SUBSTYLES;
      for (const auto& i : styleTable) {
            if (name == i.name) {
                  ss = i.ss;
                  break;
                  }
            }
      if (ss == SubStyleId::SUBSTYLES) {
            ss = e.addUserTextStyle(name);
            if (ss == SubStyleId::SUBSTYLES) {
                  qDebug("unhandled substyle <%s>", qPrintable(name));
                  return;
                  }
            }

//      qDebug("read substyle <%s>", qPrintable(name));
      const std::vector<StyledProperty>& spl = subStyle(ss);
      for (const auto& i : spl) {
            QVariant value;
            if (i.sid == Sid::NOSTYLE)
                  break;
            switch (i.pid) {
                  case Pid::SUB_STYLE:
                        value = int(ss);
                        break;
                  case Pid::BEGIN_FONT_FACE:
                  case Pid::CONTINUE_FONT_FACE:
                  case Pid::END_FONT_FACE:
                  case Pid::FONT_FACE:
                        value = family;
                        break;
                  case Pid::BEGIN_FONT_SIZE:
                  case Pid::CONTINUE_FONT_SIZE:
                  case Pid::END_FONT_SIZE:
                  case Pid::FONT_SIZE:
                        value = size;
                        break;
                  case Pid::BEGIN_FONT_BOLD:
                  case Pid::CONTINUE_FONT_BOLD:
                  case Pid::END_FONT_BOLD:
                  case Pid::FONT_BOLD:
                        value = bold;
                        break;
                  case Pid::BEGIN_FONT_ITALIC:
                  case Pid::CONTINUE_FONT_ITALIC:
                  case Pid::END_FONT_ITALIC:
                  case Pid::FONT_ITALIC:
                        value = italic;
                        break;
                  case Pid::BEGIN_FONT_UNDERLINE:
                  case Pid::CONTINUE_FONT_UNDERLINE:
                  case Pid::END_FONT_UNDERLINE:
                  case Pid::FONT_UNDERLINE:
                        value = underline;
                        break;
                  case Pid::FRAME:
                        value = hasFrame;
                        break;
                  case Pid::FRAME_SQUARE:
                        value = false;
                        break;
                  case Pid::FRAME_CIRCLE:
                        value = circle;
                        break;
                  case Pid::FRAME_WIDTH:
                        value = frameWidth;
                        break;
                  case Pid::FRAME_PADDING:
                        value = paddingWidth;
                        break;
                  case Pid::FRAME_ROUND:
                        value = frameRound;
                        break;
                  case Pid::FRAME_FG_COLOR:
                        value = frameColor;
                        break;
                  case Pid::FRAME_BG_COLOR:
                        value = backgroundColor;
                        break;
                  case Pid::FONT_SPATIUM_DEPENDENT:
                        value = sizeIsSpatiumDependent;
                        break;
                  case Pid::BEGIN_TEXT_ALIGN:
                  case Pid::CONTINUE_TEXT_ALIGN:
                  case Pid::END_TEXT_ALIGN:
                  case Pid::ALIGN:
                        value = QVariant::fromValue(align);
                        break;
                  case Pid::OFFSET:
                        if (offsetValid) {
                              if (ss == SubStyleId::TEMPO) {
                                    style->set(Sid::tempoPosAbove, Spatium(offset.y()));
                                    offset = QPointF();
                                    }
                              else if (ss == SubStyleId::STAFF) {
                                    style->set(Sid::staffTextPosAbove, Spatium(offset.y()));
                                    offset = QPointF();
                                    }
                              else if (ss == SubStyleId::REHEARSAL_MARK) {
                                    style->set(Sid::rehearsalMarkPosAbove, Spatium(offset.y()));
                                    offset = QPointF();
                                    }
                              value = offset;
                              }
                        break;
                  case Pid::OFFSET_TYPE:
                        value = int(offsetType);
                        break;
                  case Pid::SYSTEM_FLAG:
                        value = systemFlag;
                        break;
                  case Pid::BEGIN_HOOK_HEIGHT:
                  case Pid::END_HOOK_HEIGHT:
                        value = QVariant();
                        break;
                  case Pid::PLACEMENT:
                        if (placementValid)
                              value = int(placement);
                        break;
                  case Pid::LINE_WIDTH:
                        if (lineWidth != -1.0)
                              value = lineWidth;
                        break;
                  default:
//                        qDebug("unhandled property <%s>%d", propertyName(i.pid), int (i.pid));
                        break;
                  }
            if (value.isValid())
                  style->set(i.sid, value);
//            else
//                  qDebug("invalid style value <%s> pid<%s>", MStyle::valueName(i.sid), propertyName(i.pid));
            }
      }

//---------------------------------------------------------
//   readAccidental
//---------------------------------------------------------

static void readAccidental(Accidental* a, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "bracket") {
                  int i = e.readInt();
                  if (i == 0 || i == 1)
                        a->setBracket(AccidentalBracket(i));
                  }
            else if (tag == "subtype") {
                  QString text = e.readElementText();
                  const static std::map<QString, AccidentalType> accMap = {
                     {"none",               AccidentalType::NONE},
                     {"sharp",              AccidentalType::SHARP},
                     {"flat",               AccidentalType::FLAT},
                     {"natural",            AccidentalType::NATURAL},
                     {"double sharp",       AccidentalType::SHARP2},
                     {"double flat",        AccidentalType::FLAT2},
                     {"flat-slash",         AccidentalType::FLAT_SLASH},
                     {"flat-slash2",        AccidentalType::FLAT_SLASH2},
                     {"mirrored-flat2",     AccidentalType::MIRRORED_FLAT2},
                     {"mirrored-flat",      AccidentalType::MIRRORED_FLAT},
                     {"sharp-slash",        AccidentalType::SHARP_SLASH},
                     {"sharp-slash2",       AccidentalType::SHARP_SLASH2},
                     {"sharp-slash3",       AccidentalType::SHARP_SLASH3},
                     {"sharp-slash4",       AccidentalType::SHARP_SLASH4},
                     {"sharp arrow up",     AccidentalType::SHARP_ARROW_UP},
                     {"sharp arrow down",   AccidentalType::SHARP_ARROW_DOWN},
                     {"flat arrow up",      AccidentalType::FLAT_ARROW_UP},
                     {"flat arrow down",    AccidentalType::FLAT_ARROW_DOWN},
                     {"natural arrow up",   AccidentalType::NATURAL_ARROW_UP},
                     {"natural arrow down", AccidentalType::NATURAL_ARROW_DOWN},
                     {"sori",               AccidentalType::SORI},
                     {"koron",              AccidentalType::KORON}
                     };
                  auto it = accMap.find(text);
                  if (it == accMap.end()) {
                        qDebug("invalid type %s", qPrintable(text));
                        a->setAccidentalType(AccidentalType::NONE);
                        }
                  else
                        a->setAccidentalType(it->second);
                  }
            else if (tag == "role") {
                  AccidentalRole r = AccidentalRole(e.readInt());
                  if (r == AccidentalRole::AUTO || r == AccidentalRole::USER)
                        a->setRole(r);
                  }
            else if (tag == "small")
                  a->setSmall(e.readInt());
            else if (a->Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

static NoteHead::Group convertHeadGroup(int i)
      {
      NoteHead::Group val;
      switch (i) {
            case 1:
                  val = NoteHead::Group::HEAD_CROSS;
                  break;
            case 2:
                  val = NoteHead::Group::HEAD_DIAMOND;
                  break;
            case 3:
                  val = NoteHead::Group::HEAD_TRIANGLE_DOWN;
                  break;
            case 4:
                  val = NoteHead::Group::HEAD_MI;
                  break;
            case 5:
                  val = NoteHead::Group::HEAD_SLASH;
                  break;
            case 6:
                  val = NoteHead::Group::HEAD_XCIRCLE;
                  break;
            case 7:
                  val = NoteHead::Group::HEAD_DO;
                  break;
            case 8:
                  val = NoteHead::Group::HEAD_RE;
                  break;
            case 9:
                  val = NoteHead::Group::HEAD_FA;
                  break;
            case 10:
                  val = NoteHead::Group::HEAD_LA;
                  break;
            case 11:
                  val = NoteHead::Group::HEAD_TI;
                  break;
            case 12:
                  val = NoteHead::Group::HEAD_SOL;
                  break;
            case 13:
                  val = NoteHead::Group::HEAD_BREVIS_ALT;
                  break;
            case 0:
            default:
                  val = NoteHead::Group::HEAD_NORMAL;
            }
      return val;
      }

static NoteHead::Type convertHeadType(int i)
      {
      NoteHead::Type val;
      switch (i) {
            case 0:
                  val = NoteHead::Type::HEAD_WHOLE;
                  break;
            case 1:
                  val = NoteHead::Type::HEAD_HALF;
                  break;
            case 2:
                  val = NoteHead::Type::HEAD_QUARTER;
                  break;
            case 3:
                  val = NoteHead::Type::HEAD_BREVIS;
                  break;
            default:
                  val = NoteHead::Type::HEAD_AUTO;;
            }
      return val;
      }

//---------------------------------------------------------
//   ArticulationNames
//---------------------------------------------------------

static struct ArticulationNames {
      SymId id;
      const char* name;
      } articulationNames[] = {
      { SymId::fermataAbove,              "fermata",                   },
      { SymId::fermataShortAbove,         "shortfermata",              },
      { SymId::fermataLongAbove,          "longfermata",               },
      { SymId::fermataVeryLongAbove,      "verylongfermata",           },
      { SymId::articAccentAbove,          "sforzato",                  },
      { SymId::articStaccatoAbove,        "staccato",                  },
      { SymId::articStaccatissimoAbove,   "staccatissimo",             },
      { SymId::articTenutoAbove,          "tenuto",                    },
      { SymId::articTenutoStaccatoAbove,  "portato",                   },
      { SymId::articMarcatoAbove,         "marcato",                   },
      { SymId::guitarFadeIn,              "fadein",                    },
      { SymId::guitarFadeOut,             "fadeout",                   },
      { SymId::guitarVolumeSwell,         "volumeswell",               },
      { SymId::wiggleSawtooth,            "wigglesawtooth",            },
      { SymId::wiggleSawtoothWide,        "wigglesawtoothwide",        },
      { SymId::wiggleVibratoLargeFaster,  "wigglevibratolargefaster",  },
      { SymId::wiggleVibratoLargeSlowest, "wigglevibratolargeslowest", },
      { SymId::brassMuteOpen,             "ouvert",                    },
      { SymId::brassMuteClosed,           "plusstop",                  },
      { SymId::stringsUpBow,              "upbow",                     },
      { SymId::stringsDownBow,            "downbow",                   },
      { SymId::ornamentTurnInverted,      "reverseturn",               },
      { SymId::ornamentTurn,              "turn",                      },
      { SymId::ornamentTrill,             "trill",                     },
      { SymId::ornamentMordent,           "prall",                     },
      { SymId::ornamentMordentInverted,   "mordent",                   },
      { SymId::ornamentTremblement,       "prallprall",                },
      { SymId::ornamentPrallMordent,      "prallmordent",              },
      { SymId::ornamentUpPrall,           "upprall",                   },
      { SymId::ornamentUpMordent,         "upmordent",                 },
      { SymId::ornamentDownMordent,       "downmordent",               },
      { SymId::ornamentPrallDown,         "pralldown",                 },
      { SymId::ornamentPrallUp,           "prallup",                   },
      { SymId::ornamentLinePrall,         "lineprall",                 },
      { SymId::ornamentPrecompSlide,      "schleifer",                 },
      { SymId::pluckedSnapPizzicatoAbove, "snappizzicato",             },
      { SymId::stringsThumbPosition,      "thumb",                     },
      { SymId::luteFingeringRHThumb,      "lutefingeringthumb",        },
      { SymId::luteFingeringRHFirst,      "lutefingering1st",          },
      { SymId::luteFingeringRHSecond,     "lutefingering2nd",          },
      { SymId::luteFingeringRHThird,      "lutefingering3rd",          },

      { SymId::ornamentPrecompMordentUpperPrefix, "downprall"   },
      { SymId::ornamentPrecompMordentUpperPrefix, "ornamentDownPrall"   },
      };

//---------------------------------------------------------
//   oldArticulationNames2SymId
//---------------------------------------------------------

SymId oldArticulationNames2SymId(const QString& s)
      {
      for (auto i : articulationNames) {
            if (i.name == s)
                  return i.id;
            }
      return SymId::noSym;
      }

//---------------------------------------------------------
//   readDrumset
//---------------------------------------------------------

static void readDrumset(Drumset* ds, XmlReader& e)
      {
      int pitch = e.intAttribute("pitch", -1);
      if (pitch < 0 || pitch > 127) {
            qDebug("load drumset: invalid pitch %d", pitch);
            return;
            }
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "head")
                  ds->drum(pitch).notehead = convertHeadGroup(e.readInt());
            else if (tag == "variants") {
                  while(e.readNextStartElement()) {
                        const QStringRef& tagv(e.name());
                        if (tagv == "variant") {
                              DrumInstrumentVariant div;
                              div.pitch = e.attribute("pitch").toInt();
                              while (e.readNextStartElement()) {
                                    const QStringRef& taga(e.name());
                                    if (taga == "articulation") {
                                          QString oldArticulationName = e.readElementText();
                                          SymId oldId = oldArticulationNames2SymId(oldArticulationName);
                                          div.articulationName = Articulation::symId2ArticulationName(oldId);
                                          }
                                    else if (taga == "tremolo") {
                                          div.tremolo = Tremolo::name2Type(e.readElementText());
                                          }
                                    }
                              ds->drum(pitch).addVariant(div);
                              }
                        }
                  }
            else if (ds->readProperties(e, pitch))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

static void readInstrument(Instrument *i, Part* p, XmlReader& e)
      {
      int program = -1;
      int bank    = 0;
      int chorus  = 30;
      int reverb  = 30;
      int volume  = 100;
      int pan     = 60;
      bool customDrumset = false;
      i->clearChannels();       // remove default channel
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Drum") {
                  // if we see one of this tags, a custom drumset will
                  // be created
                  if (!i->drumset())
                        i->setDrumset(new Drumset(*smDrumset));
                  if (!customDrumset) {
                        i->drumset()->clear();
                        customDrumset = true;
                        }
                  readDrumset(i->drumset(), e);
                  }

            else if (i->readProperties(e, p, &customDrumset))
                  ;
            else
                 e.unknown();
            }
      if (i->channel().empty()) {      // for backward compatibility
            Channel* a = new Channel;
            a->chorus  = chorus;
            a->reverb  = reverb;
            a->name    = "normal";
            a->program = program;
            a->bank    = bank;
            a->volume  = volume;
            a->pan     = pan;
            i->appendChannel(a);
            }
      if (i->useDrumset()) {
            if (i->channel()[0]->bank == 0)
                  i->channel()[0]->bank = 128;
            i->channel()[0]->updateInitList();
            }
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

static void readStaff(Staff* staff, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "type") {    // obsolete
                  int staffTypeIdx = e.readInt();
                  qDebug("obsolete: Staff::read staffTypeIdx %d", staffTypeIdx);
                  }
            else if (tag == "neverHide") {
                  bool v = e.readInt();
                  if (v)
                        staff->setHideWhenEmpty(Staff::HideMode::NEVER);
                  }
            else if (tag == "barLineSpan") {
                  staff->setBarLineFrom(e.intAttribute("from", 0));
                  staff->setBarLineTo(e.intAttribute("to", 0));
                  int span     = e.readInt();
                  staff->setBarLineSpan(span - 1);
                  }
            else if (staff->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

static void readPart(Part* part, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument") {
                  Instrument* i = part->instrument();
                  readInstrument(i, part, e);
                  Drumset* ds = i->drumset();
                  Staff*   s = part->staff(0);
                  int lld = s ? qRound(s->lineDistance(0)) : 1;
                  if (ds && s && lld > 1) {
                        for (int i = 0; i < DRUM_INSTRUMENTS; ++i)
                              ds->drum(i).line /= lld;
                        }
                  }
            else if (tag == "Staff") {
                  Staff* staff = new Staff(part->score());
                  staff->setPart(part);
                  part->score()->staves().push_back(staff);
                  part->staves()->push_back(staff);
                  readStaff(staff, e);
                  }
            else if (part->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readAmbitus
//---------------------------------------------------------

static void readAmbitus(Ambitus* ambitus, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "head")
                  ambitus->setNoteHeadGroup(convertHeadGroup(e.readInt()));
            else if (tag == "headType")
                  ambitus->setNoteHeadType(convertHeadType(e.readInt()));
            else if (ambitus->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

static void readNote(Note* note, XmlReader& e)
      {
      note->setTpc1(Tpc::TPC_INVALID);
      note->setTpc2(Tpc::TPC_INVALID);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Accidental") {
                  Accidental* a = new Accidental(note->score());
                  a->setTrack(note->track());
                  readAccidental(a, e);
                  note->add(a);
                  }
            else if (tag == "head") {
                  int i = e.readInt();
                  NoteHead::Group val = convertHeadGroup(i);
                  note->setHeadGroup(val);
                  }
            else if (tag == "headType") {
                  int i = e.readInt();
                  NoteHead::Type val = convertHeadType(i);
                  note->setHeadType(val);
                  }
            else if (note->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      // ensure sane values:
      note->setPitch(limit(note->pitch(), 0, 127));

      if (!tpcIsValid(note->tpc1()) && !tpcIsValid(note->tpc2())) {
            Key key = (note->staff() && note->chord()) ? note->staff()->key(note->chord()->tick()) : Key::C;
            int tpc = pitch2tpc(note->pitch(), key, Prefer::NEAREST);
            if (note->concertPitch())
                  note->setTpc1(tpc);
            else
                  note->setTpc2(tpc);
            }
      if (!(tpcIsValid(note->tpc1()) && tpcIsValid(note->tpc2()))) {
            int tick = note->chord() ? note->chord()->tick() : -1;
            Interval v = note->staff() ? note->part()->instrument(tick)->transpose() : Interval();
            if (tpcIsValid(note->tpc1())) {
                  v.flip();
                  if (v.isZero())
                        note->setTpc2(note->tpc1());
                  else
                        note->setTpc2(Ms::transposeTpc(note->tpc1(), v, true));
                  }
            else {
                  if (v.isZero())
                        note->setTpc1(note->tpc2());
                  else
                        note->setTpc1(Ms::transposeTpc(note->tpc2(), v, true));
                  }
            }

      // check consistency of pitch, tpc1, tpc2, and transposition
      // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
      // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
      // including perhaps some we don't know about yet,
      // we will attempt to fix some problems here regardless of version

      if (!e.pasteMode() && !MScore::testMode) {
            int tpc1Pitch = (tpc2pitch(note->tpc1()) + 12) % 12;
            int tpc2Pitch = (tpc2pitch(note->tpc2()) + 12) % 12;
            int concertPitch = note->pitch() % 12;
            if (tpc1Pitch != concertPitch) {
                  qDebug("bad tpc1 - concertPitch = %d, tpc1 = %d", concertPitch, tpc1Pitch);
                  note->setPitch(note->pitch() + tpc1Pitch - concertPitch);
                  }
            Interval v = note->staff()->part()->instrument(e.tick())->transpose();
            int transposedPitch = (note->pitch() - v.chromatic) % 12;
            if (tpc2Pitch != transposedPitch) {
                  qDebug("bad tpc2 - transposedPitch = %d, tpc2 = %d", transposedPitch, tpc2Pitch);
                  // just in case the staff transposition info is not reliable here,
                  // do not attempt to correct tpc
                  // except for older scores where we know there are tpc problems
                  v.flip();
                  note->setTpc2(Ms::transposeTpc(note->tpc1(), v, true));
                  }
            }
      }

//---------------------------------------------------------
//   readTextProperties
//---------------------------------------------------------

static bool readTextProperties206(XmlReader& e, TextBase* t, Element* be)
      {
      const QStringRef& tag(e.name());
      if (tag == "style") {
            QString s = e.readElementText();
            if (!be->isTuplet()) {      // Hack
                  SubStyleId ss;
                  ss = e.lookupUserTextStyle(s);
                  if (ss == SubStyleId::SUBSTYLES)
                        ss = subStyleFromName(s);
                  if (ss != SubStyleId::SUBSTYLES)
                        be->initSubStyle(ss);
                  }
            }
      else if (tag == "foregroundColor")  // same as "color" ?
            e.skipCurrentElement();
      else if (tag == "frame")
            t->setHasFrame(e.readBool());
      else if (tag == "halign") {
            Align align = Align(int(t->align()) & int(~Align::HMASK));
            const QString& val(e.readElementText());
            if (val == "center")
                  align = align | Align::HCENTER;
            else if (val == "right")
                  align = align | Align::RIGHT;
            else if (val == "left")
                  ;
            else
                  qDebug("unknown alignment: <%s>", qPrintable(val));
            t->setAlign(align);
            }
      else if (tag == "valign") {
            Align align = Align(int(t->align()) & int(~Align::VMASK));
            const QString& val(e.readElementText());
            if (val == "center")
                  align = align | Align::VCENTER;
            else if (val == "bottom")
                  align = align | Align::BOTTOM;
            else if (val == "baseline")
                  align = align | Align::BASELINE;
            else if (val == "top")
                  ;
            else
                  qDebug("unknown alignment: <%s>", qPrintable(val));
            t->setAlign(align);
            }
      else if (!t->readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   readText206
//---------------------------------------------------------

static void readText206(XmlReader& e, TextBase* t, Element* be)
      {
      while (e.readNextStartElement()) {
            if (!readTextProperties206(e, t, be))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

static void readTempoText(TempoText* t, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "tempo")
                  t->setTempo(e.readDouble());
            else if (tag == "followText")
                  t->setFollowText(e.readInt());
            else if (!readTextProperties206(e, t, t))
                  e.unknown();
            }
      // check sanity
      if (t->xmlText().isEmpty()) {
            t->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(lrint(60 * t->tempo())));
            t->setVisible(false);
            }
      else
            t->setXmlText(t->xmlText().replace("<sym>unicode", "<sym>met"));
      }

//---------------------------------------------------------
//   readTuplet
//---------------------------------------------------------

static void readTuplet(Tuplet* tuplet, XmlReader& e)
      {
      tuplet->setId(e.intAttribute("id", 0));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Number") {
                  Text* _number = new Text(tuplet->score());
                  _number->setParent(tuplet);
                  _number->setComposition(true);
                  tuplet->setNumber(_number);
                  // _number reads property defaults from parent tuplet as "composition" is set:
                  for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_BOLD, Pid::FONT_ITALIC, Pid::FONT_UNDERLINE, Pid::ALIGN })
                        _number->resetProperty(p);
                  readText206(e, _number, tuplet);
                  _number->setVisible(tuplet->visible());     //?? override saved property
                  _number->setTrack(tuplet->track());
                  // move property flags from _number
                  for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_BOLD, Pid::FONT_ITALIC, Pid::FONT_UNDERLINE, Pid::ALIGN })
                        tuplet->setPropertyFlags(p, _number->propertyFlags(p));
                  }
            else if (!tuplet->readProperties(e))
                  e.unknown();
            }
      Fraction r = (tuplet->ratio() == 1) ? tuplet->ratio() : tuplet->ratio().reduced();
      Fraction f(r.denominator(), tuplet->baseLen().fraction().denominator());
      tuplet->setDuration(f.reduced());
      }

//---------------------------------------------------------
//   readLyrics
//---------------------------------------------------------

static void readLyrics(Lyrics* lyrics, XmlReader& e)
      {
      int   iEndTick = 0;           // used for backward compatibility
      Text* _verseNumber = 0;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "endTick") {
                  // store <endTick> tag value until a <ticks> tag has been read
                  // which positions this lyrics element in the score
                  iEndTick = e.readInt();
                  }
            else if (tag == "Number") {
                  _verseNumber = new Text(lyrics->score());
                  _verseNumber->read(e);
                  _verseNumber->setParent(lyrics);
                  }
            else if (!lyrics->readProperties(e))
                  e.unknown();
            }

      // if any endTick, make it relative to current tick
      if (iEndTick)
            lyrics->setTicks(iEndTick - e.tick());
      if (_verseNumber) {
            // TODO: add text to main text
            delete _verseNumber;
            }
      lyrics->setAutoplace(true);
      lyrics->setUserOff(QPointF());
      lyrics->setOffset(QPointF());
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

static void readChord(Chord* chord, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Note") {
                  Note* note = new Note(chord->score());
                  // the note needs to know the properties of the track it belongs to
                  note->setTrack(chord->track());
                  note->setChord(chord);
                  readNote(note, e);
                  chord->add(note);
                  }
            else if (tag == "Articulation") {
                  Element* el = readArticulation(chord, e);
                  if (el->isFermata())
                        chord->segment()->add(el);
                  else
                        chord->add(el);
                  }
            else if (tag == "Stem") {
                  Stem* stem = new Stem(chord->score());
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "subtype")        // obsolete
                              e.skipCurrentElement();
                        else if (!stem->readProperties(e))
                              e.unknown();
                        }
                  chord->add(stem);
                  }
            else if (tag == "Lyrics") {
                  Lyrics* lyrics = new Lyrics(chord->score());
                  lyrics->setTrack(e.track());
                  readLyrics(lyrics, e);
                  chord->add(lyrics);
                  }
            else if (chord->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

static void readRest(Rest* rest, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Articulation") {
                  Articulation* atr = new Articulation(rest->score());
                  atr->setTrack(rest->track());
                  Element* el = readArticulation(rest, e);
                  if (el->isFermata())
                        rest->segment()->add(el);
                  else
                        rest->add(el);
                  }
            else if (rest->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readTextLineProperties
//---------------------------------------------------------

static bool readTextLineProperties(XmlReader& e, TextLineBase* tl)
      {
      const QStringRef& tag(e.name());

      if (tag == "beginText") {
            Text* text = new Text(tl->score());
            readText206(e, text, tl);
            tl->setBeginText(text->xmlText());
            delete text;
            }
      else if (tag == "continueText") {
            Text* text = new Text(tl->score());
            readText206(e, text, tl);
            tl->setContinueText(text->xmlText());
            delete text;
            }
      else if (tag == "endText") {
            Text* text = new Text(tl->score());
            readText206(e, text, tl);
            tl->setEndText(text->xmlText());
            delete text;
            }
      else if (tag == "beginHook")
            tl->setBeginHookType(e.readBool() ? HookType::HOOK_90 : HookType::NONE);
      else if (tag == "endHook")
            tl->setEndHookType(e.readBool() ? HookType::HOOK_90 : HookType::NONE);
      else if (tag == "beginHookType")
            tl->setBeginHookType(e.readInt() == 0 ? HookType::HOOK_90 : HookType::HOOK_45);
      else if (tag == "endHookType")
            tl->setEndHookType(e.readInt() == 0 ? HookType::HOOK_90 : HookType::HOOK_45);
      else if (tl->readProperties(e))
            return true;
      return true;
      }

//---------------------------------------------------------
//   readVolta206
//---------------------------------------------------------

static void readVolta206(XmlReader& e, Volta* volta)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  volta->endings().clear();
                  for (const QString& l : sl) {
                        int i = l.simplified().toInt();
                        volta->endings().append(i);
                        }
                  }
            else if (tag == "lineWidth") {
                  volta->setLineWidth(e.readDouble() * volta->spatium());
                  // TODO lineWidthStyle = PropertyStyle::UNSTYLED;
                  }
            else if (!readTextLineProperties(e, volta))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readPedal
//---------------------------------------------------------

static void readPedal(XmlReader& e, Pedal* pedal)
      {
      while (e.readNextStartElement()) {
            if (!readTextLineProperties(e, pedal))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readOttava
//---------------------------------------------------------

static void readOttava(XmlReader& e, Ottava* ottava)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype") {
                  QString s = e.readElementText();
                  bool ok;
                  int idx = s.toInt(&ok);
                  if (!ok) {
                        idx = 0;    // OttavaType::OTTAVA_8VA;
                        int i = 0;
                        for (auto p :  { "8va","8vb","15ma","15mb","22ma","22mb" } ) {
                              if (p == s) {
                                    idx = i;
                                    break;
                                    }
                              ++i;
                              }
                        }
                  ottava->setOttavaType(OttavaType(idx));
                  }
            else if (tag == "numbersOnly") {
                  ottava->setNumbersOnly(e.readBool());
                  //TODO numbersOnlyStyle = PropertyFlags::UNSTYLED;
                  }
            else if (!readTextLineProperties(e, ottava))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readHairpin
//---------------------------------------------------------

static void readHairpin(XmlReader& e, Hairpin* h)
      {
      bool useText = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  h->setHairpinType(HairpinType(e.readInt()));
            else if (tag == "lineWidth") {
                  h->setLineWidth(e.readDouble() * h->spatium());
                  // lineWidthStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "hairpinHeight") {
                  h->setHairpinHeight(Spatium(e.readDouble()));
                  // hairpinHeightStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "hairpinContHeight") {
                  h->setHairpinContHeight(Spatium(e.readDouble()));
                  // hairpinContHeightStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "hairpinCircledTip")
                  h->setHairpinCircledTip(e.readInt());
            else if (tag == "veloChange")
                  h->setVeloChange(e.readInt());
            else if (tag == "dynType")
                  h->setDynRange(Dynamic::Range(e.readInt()));
            else if (tag == "useTextLine") {      // < 206
                  e.readInt();
                  if (h->hairpinType() == HairpinType::CRESC_HAIRPIN)
                        h->setHairpinType(HairpinType::CRESC_LINE);
                  else if (h->hairpinType() == HairpinType::DECRESC_HAIRPIN)
                        h->setHairpinType(HairpinType::DECRESC_LINE);
                  useText = true;
                  }
            else if (!readTextLineProperties(e, h))
                  e.unknown();
            }
      if (!useText) {
            h->setBeginText("");
            h->setContinueText("");
            h->setEndText("");
            }
      h->spannerSegments().clear();
#if 0
      for (auto ss : h->spannerSegments()) {
            ss->setUserOff(QPointF());
            ss->setUserOff2(QPointF());
            }
#endif
      }

//---------------------------------------------------------
//   readTrill
//---------------------------------------------------------

static void readTrill(XmlReader& e, Trill* t)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  t->setTrillType(e.readElementText());
            else if (tag == "Accidental") {
                  Accidental* _accidental = new Accidental(t->score());
                  readAccidental(_accidental, e);
                  _accidental->setParent(t);
                  t->setAccidental(_accidental);
                  }
            else if ( tag == "ornamentStyle")
                  t->setProperty(Pid::ORNAMENT_STYLE, Ms::getProperty(Pid::ORNAMENT_STYLE, e));
            else if ( tag == "play")
                  t->setPlayArticulation(e.readBool());
            else if (!t->SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readTextLine206
//---------------------------------------------------------

void readTextLine206(XmlReader& e, TextLineBase* tlb)
      {
      while (e.readNextStartElement()) {
            if (!readTextLineProperties(e, tlb))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   setFermataPlacement
//    set fermata placement from old ArticulationAnchor
//    for backwards compatibility
//---------------------------------------------------------

static void setFermataPlacement(Element* el, ArticulationAnchor anchor, Direction direction)
      {
      if (direction == Direction::UP)
            el->setPlacement(Placement::ABOVE);
      else if (direction == Direction::DOWN)
            el->setPlacement(Placement::BELOW);
      else {
            switch (anchor) {
                  case ArticulationAnchor::TOP_STAFF:
                  case ArticulationAnchor::TOP_CHORD:
                        el->setPlacement(Placement::ABOVE);
                        break;

                  case ArticulationAnchor::BOTTOM_STAFF:
                  case ArticulationAnchor::BOTTOM_CHORD:
                        el->setPlacement(Placement::BELOW);
                        break;

                  case ArticulationAnchor::CHORD:
                        break;
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   readArticulation
//---------------------------------------------------------

Element* readArticulation(ChordRest* cr, XmlReader& e)
      {
      Element* el = 0;
      SymId sym = SymId::fermataAbove;          // default -- backward compatibility (no type = ufermata in 1.2)
      ArticulationAnchor anchor  = ArticulationAnchor::TOP_STAFF;
      Direction direction = Direction::AUTO;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype") {
                  QString s = e.readElementText();
                  if (s[0].isDigit()) {
                        int oldType = s.toInt();
                        sym = articulationNames[oldType].id;
                        }
                  else {
                        sym = oldArticulationNames2SymId(s);
                        if (sym == SymId::noSym) {
                              struct {
                                    const char* name;
                                    bool up;
                                    SymId id;
                                    } al[] =
                                    {
                                    { "fadein",                    true,  SymId::guitarFadeIn },
                                    { "fadeout",                   true,  SymId::guitarFadeOut },
                                    { "volumeswell",               true,  SymId::guitarVolumeSwell },
                                    { "wigglesawtooth",            true,  SymId::wiggleSawtooth },
                                    { "wigglesawtoothwide",        true,  SymId::wiggleSawtoothWide },
                                    { "wigglevibratolargefaster",  true,  SymId::wiggleVibratoLargeFaster },
                                    { "wigglevibratolargeslowest", true,  SymId::wiggleVibratoLargeSlowest },
                                    { "umarcato",                  true,  SymId::articMarcatoAbove },
                                    { "dmarcato",                  false, SymId::articMarcatoBelow },
                                    { "ufermata",                  true,  SymId::fermataAbove },
                                    { "dfermata",                  false, SymId::fermataBelow },
                                    { "ushortfermata",             true,  SymId::fermataShortAbove },
                                    { "dshortfermata",             false, SymId::fermataShortBelow },
                                    { "ulongfermata",              true,  SymId::fermataLongAbove },
                                    { "dlongfermata",              false, SymId::fermataLongBelow },
                                    { "uverylongfermata",          true,  SymId::fermataVeryLongAbove },
                                    { "dverylongfermata",          false, SymId::fermataVeryLongBelow },

                                    // watch out, bug in 1.2 uportato and dportato are reversed
                                    { "dportato",                  true,  SymId::articTenutoStaccatoAbove },
                                    { "uportato",                  false, SymId::articTenutoStaccatoBelow },
                                    { "ustaccatissimo",            true,  SymId::articStaccatissimoAbove },
                                    { "dstaccatissimo",            false, SymId::articStaccatissimoBelow }
                                    };
                              int i;
                              int n = sizeof(al) / sizeof(*al);
                              for (i = 0; i < n; ++i) {
                                    if (s == al[i].name) {
                                          sym       = al[i].id;
                                          bool up   = al[i].up;
                                          direction = up ? Direction::UP : Direction::DOWN;
                                          break;
                                          }
                                    }
                              if (i == n) {
                                    sym = Sym::name2id(s);
                                    if (sym == SymId::noSym)
                                          qDebug("Articulation: unknown type <%s>", qPrintable(s));
                                    }
                              }
                        }
                  switch (sym) {
                        case SymId::fermataAbove:
                        case SymId::fermataBelow:
                        case SymId::fermataShortAbove:
                        case SymId::fermataShortBelow:
                        case SymId::fermataLongAbove:
                        case SymId::fermataLongBelow:
                        case SymId::fermataVeryLongAbove:
                        case SymId::fermataVeryLongBelow:
                              el = new Fermata(sym, cr->score());
                              setFermataPlacement(el, anchor, direction);
                              break;
                        default:
                              el = new Articulation(sym, cr->score());
                              toArticulation(el)->setDirection(direction);
                              break;
                        };
                  }
            else if (tag == "anchor") {
                  if (!el)
                        anchor = ArticulationAnchor(e.readInt());
                  else {
                        if (el->isFermata()) {
                              anchor = ArticulationAnchor(e.readInt());
                              setFermataPlacement(el, anchor, direction);
                              }
                        else
                              el->readProperties(e);
                        }
                  }
            else  if (tag == "direction") {
                  if (!el)
                        direction = toDirection(e.readElementText());
                  else {
                        if (!el->isFermata())
                              el->readProperties(e);
                        }
                  }
            else {
                  if (!el) {
                        qDebug("not handled <%s>", qPrintable(tag.toString()));
                        }
                  if (!el || !el->readProperties(e))
                        e.unknown();
                  }
            }
      // Special case for "no type" = ufermata, with missing subtype tag
      if (!el) {
            el = new Fermata(sym, cr->score());
            setFermataPlacement(el, anchor, direction);
            }
      el->setTrack(cr->staffIdx() * VOICES);
      return el;
      }

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

static void readMeasure(Measure* m, int staffIdx, XmlReader& e)
      {
      Segment* segment = 0;
      qreal _spatium = m->spatium();
      Score* score = m->score();

      QList<Chord*> graceNotes;
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      m->createStaves(staffIdx);

      // tick is obsolete
      if (e.hasAttribute("tick"))
            e.initTick(score->fileDivision(e.intAttribute("tick")));

      bool irregular;
      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  m->setLen(Fraction(sl[0].toInt(), sl[1].toInt()));
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            irregular = true;
            score->sigmap()->add(m->tick(), SigEvent(m->len(), m->timesig()));
            score->sigmap()->add(m->endTick(), SigEvent(m->timesig()));
            }
      else
            irregular = false;

      Staff* staff = score->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(m->tick()));

      // keep track of tick of previous element
      // this allows markings that need to apply to previous element to do so
      // even though we may have already advanced to next tick position
      int lastTick = e.tick();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "move")
                  e.initTick(e.readFraction().ticks() + m->tick());
            else if (tag == "tick") {
                  e.initTick(score->fileDivision(e.readInt()));
                  lastTick = e.tick();
                  }
            else if (tag == "BarLine") {
                  BarLine* bl = new BarLine(score);
                  bl->setTrack(e.track());
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "subtype")
                              bl->setBarLineType(e.readElementText());
                        else if (tag == "customSubtype")                      // obsolete
                              e.readInt();
                        else if (tag == "span") {
                              //TODO bl->setSpanFrom(e.intAttribute("from", bl->spanFrom()));  // obsolete
                              // bl->setSpanTo(e.intAttribute("to", bl->spanTo()));            // obsolete
                              int span = e.readInt();
                              if (span)
                                    span--;
                              bl->setSpanStaff(span);
                              }
                        else if (tag == "spanFromOffset")
                              bl->setSpanFrom(e.readInt());
                        else if (tag == "spanToOffset")
                              bl->setSpanTo(e.readInt());
                        else if (tag == "Articulation") {
                              Articulation* a = new Articulation(score);
                              a->read(e);
                              bl->add(a);
                              }
                        else if (!bl->Element::readProperties(e))
                              e.unknown();
                        }
                  //
                  //  StartRepeatBarLine: always at the beginning tick of a measure, always BarLineType::START_REPEAT
                  //  BarLine:            in the middle of a measure, has no semantic
                  //  EndBarLine:         at the end tick of a measure
                  //  BeginBarLine:       first segment of a measure

                  SegmentType st;
                  if ((e.tick() != m->tick()) && (e.tick() != m->endTick()))
                        st = SegmentType::BarLine;
                  else if (bl->barLineType() == BarLineType::START_REPEAT && e.tick() == m->tick())
                        st = SegmentType::StartRepeatBarLine;
                  else if (e.tick() == m->tick() && segment == 0)
                        st = SegmentType::BeginBarLine;
                  else
                        st = SegmentType::EndBarLine;
                  segment = m->getSegment(st, e.tick());
                  segment->add(bl);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score);
                  chord->setTrack(e.track());
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  chord->setParent(segment);
                  readChord(chord, e);
                  if (chord->noteType() != NoteType::NORMAL)
                        graceNotes.push_back(chord);
                  else {
                        segment->add(chord);
                        for (int i = 0; i < graceNotes.size(); ++i) {
                              Chord* gc = graceNotes[i];
                              gc->setGraceIndex(i);
                              chord->add(gc);
                              }
                        graceNotes.clear();
                        int crticks = chord->actualTicks();
                        lastTick    = e.tick();
                        e.incTick(crticks);
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score);
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(m->timesig()/timeStretch);
                  rest->setTrack(e.track());
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  rest->setParent(segment);
                  readRest(rest, e);
                  segment->add(rest);

                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(m->timesig()/timeStretch);

                  lastTick = e.tick();
                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score);
                  breath->setTrack(e.track());
                  int tick = e.tick();
                  breath->read(e);
                  // older scores placed the breath segment right after the chord to which it applies
                  // rather than before the next chordrest segment with an element for the staff
                  // result would be layout too far left if there are other segments due to notes in other staves
                  // we need to find tick of chord to which this applies, and add its duration
                  int prevTick;
                  if (e.tick() < tick)
                        prevTick = e.tick();    // use our own tick if we explicitly reset to earlier position
                  else
                        prevTick = lastTick;    // otherwise use tick of previous tick/chord/rest tag
                  // find segment
                  Segment* prev = m->findSegment(SegmentType::ChordRest, prevTick);
                  if (prev) {
                        // find chordrest
                        ChordRest* lastCR = toChordRest(prev->element(e.track()));
                        if (lastCR)
                              tick = prevTick + lastCR->actualTicks();
                        }
                  segment = m->getSegment(SegmentType::Breath, tick);
                  segment->add(breath);
                  }
            else if (tag == "endSpanner") {
                  int id = e.attribute("id").toInt();
                  Spanner* spanner = e.findSpanner(id);
                  if (spanner) {
                        spanner->setTicks(e.tick() - spanner->tick());
                        // if (spanner->track2() == -1)
                              // the absence of a track tag [?] means the
                              // track is the same as the beginning of the slur
                        if (spanner->track2() == -1)
                              spanner->setTrack2(spanner->track() ? spanner->track() : e.track());
                        }
                  else {
                        // remember "endSpanner" values
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = e.track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  e.readNext();
                  }
            else if (tag == "Slur") {
                  Slur *sl = new Slur(score);
                  sl->setTick(e.tick());
                  sl->read(e);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sl);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sl->setTick2(sv->tick2);
                        sl->setTrack2(sv->track2);
                        }
                  score->addSpanner(sl);
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  Spanner* sp = toSpanner(Element::name2Element(tag, score));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  qDeleteAll(sp->spannerSegments());
                  sp->spannerSegments().clear();
                  e.addSpanner(e.intAttribute("id", -1), sp);

                  if (tag == "Volta")
                        readVolta206(e, toVolta(sp));
                  else if (tag == "Pedal")
                        readPedal(e, toPedal(sp));
                  else if (tag == "Ottava")
                        readOttava(e, toOttava(sp));
                  else if (tag == "HairPin")
                        readHairpin(e, toHairpin(sp));
                  else if (tag == "Trill")
                        readTrill(e, toTrill(sp));
                  else
                        readTextLine206(e, toTextLineBase(sp));
                  score->addSpanner(sp);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sp);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sp->setTicks(sv->tick2 - sp->tick());
                        sp->setTrack2(sv->track2);
                        }
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score);
                  rm->setTrack(e.track());
                  rm->read(e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(rm);
                  lastTick = e.tick();
                  e.incTick(m->ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score);
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);
                  if (e.tick() == 0) {
                        if (score->staff(staffIdx)->clef(0) != clef->clefType())
                              score->staff(staffIdx)->setDefaultClefType(clef->clefType());
                        if (clef->links() && clef->links()->size() == 1) {
                              e.linkIds().remove(clef->links()->lid());
                              qDebug("remove link %d", clef->links()->lid());
                              }
                        delete clef;
                        continue;
                        }
                  // there may be more than one clef segment for same tick position
                  if (!segment) {
                        // this is the first segment of measure
                        segment = m->getSegment(SegmentType::Clef, e.tick());
                        }
                  else {
                        bool firstSegment = false;
                        // the first clef may be missing and is added later in layout
                        for (Segment* s = m->segments().first(); s && s->tick() == e.tick(); s = s->next()) {
                              if (s->segmentType() == SegmentType::Clef
                                    // hack: there may be other segment types which should
                                    // generate a clef at current position
                                 || s->segmentType() == SegmentType::StartRepeatBarLine
                                 ) {
                                    firstSegment = true;
                                    break;
                                    }
                              }
                        if (firstSegment) {
                              Segment* ns = 0;
                              if (segment->next()) {
                                    ns = segment->next();
                                    while (ns && ns->tick() < e.tick())
                                          ns = ns->next();
                                    }
                              segment = 0;
                              for (Segment* s = ns; s && s->tick() == e.tick(); s = s->next()) {
                                    if (s->segmentType() == SegmentType::Clef) {
                                          segment = s;
                                          break;
                                          }
                                    }
                              if (!segment) {
                                    segment = new Segment(m, SegmentType::Clef, e.tick() - m->tick());
                                    m->segments().insert(segment, ns);
                                    }
                              }
                        else {
                              // this is the first clef: move to left
                              segment = m->getSegment(SegmentType::Clef, e.tick());
                              }
                        }
                  if (e.tick() != m->tick())
                        clef->setSmall(true);         // TODO: layout does this ?
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score);
                  ts->setTrack(e.track());
                  ts->read(e);
                  // if time sig not at beginning of measure => courtesy time sig
                  int currTick = e.tick();
                  bool courtesySig = (currTick > m->tick());
                  if (courtesySig) {
                        // if courtesy sig., just add it without map processing
                        segment = m->getSegment(SegmentType::TimeSigAnnounce, currTick);
                        segment->add(ts);
                        }
                  else {
                        // if 'real' time sig., do full process
                        segment = m->getSegment(SegmentType::TimeSig, currTick);
                        segment->add(ts);

                        timeStretch = ts->stretch().reduced();
                        m->setTimesig(ts->sig() / timeStretch);

                        if (irregular) {
                              score->sigmap()->add(m->tick(), SigEvent(m->len(), m->timesig()));
                              score->sigmap()->add(m->endTick(), SigEvent(m->timesig()));
                              }
                        else {
                              m->setLen(m->timesig());
                              score->sigmap()->add(m->tick(), SigEvent(m->timesig()));
                              }
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score);
                  ks->setTrack(e.track());
                  ks->read(e);
                  int curTick = e.tick();
                  if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick == 0) {
                        // ignore empty key signature
                        qDebug("remove keysig c at tick 0");
                        if (ks->links()) {
                              if (ks->links()->size() == 1)
                                    e.linkIds().remove(ks->links()->lid());
                              }
                        }
                  else {
                        // if key sig not at beginning of measure => courtesy key sig
                        bool courtesySig = (curTick == m->endTick());
                        segment = m->getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
                        segment->add(ks);
                        if (!courtesySig)
                              staff->setKey(curTick, ks->keySigEvent());
                        }
                  }
            else if (tag == "Text" || tag == "StaffText") {
                  StaffText* t = new StaffText(score);
                  t->setTrack(e.track());
                  readText206(e, t, t);
                  if (t->empty()) {
                        qDebug("reading empty text: deleted");
                        delete t;
                        }
                  else {
                        segment = m->getSegment(SegmentType::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score);
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "RehearsalMark") {
                  RehearsalMark* el = new RehearsalMark(score);
                  el->setTrack(e.track());
                  readText206(e, el, el);
                  el->setUserOff(el->userOff() - QPointF(0.0, el->score()->styleP(Sid::rehearsalMarkPosAbove)));
                  if (el->userOff().isNull())
                        el->setAutoplace(true);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(el);
                  }
#if 0
            else if (tag == "StaffText") {
                  StaffText* el = new StaffText(score);
                  el->setTrack(e.track());

                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "foregroundColor")
                              e.skipCurrentElement();
                        else if (!el->readProperties(e))
                              e.unknown();
                        }
                  TextBase* tt = static_cast<TextBase*>(el);
                  tt->setXmlText(tt->xmlText().replace("<sym>unicode", "<sym>met"));
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(el);
                  }
#endif
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "InstrumentChange"
               || tag == "StaffState"
               || tag == "FiguredBass"
               ) {
                  Element* el = Element::name2Element(tag, score);
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  el->setTrack(e.track());
                  el->read(e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(el);
                  }
            else if (tag == "Tempo") {
                  TempoText* tt = new TempoText(score);
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  tt->setTrack(e.track());
                  readTempoText(tt, e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(tt);
            }
            else if (tag == "Marker"
               || tag == "Jump"
               ) {
                  Element* el = Element::name2Element(tag, score);
                  el->setTrack(e.track());
                  el->read(e);
                  m->add(el);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Element* el = Element::name2Element(tag, score);
                        el->setTrack(e.track());
                        el->read(e);
                        segment = m->getSegment(SegmentType::ChordRest, e.tick());
                        segment->add(el);
                        }
                  }
            //----------------------------------------------------
            else if (tag == "stretch") {
                  double val = e.readDouble();
                  if (val < 0.0)
                        val = 0;
                  m->setUserStretch(val);
                  }
            else if (tag == "noOffset")
                  m->setNoOffset(e.readInt());
            else if (tag == "measureNumberMode")
                  m->setMeasureNumberMode(MeasureNumberMode(e.readInt()));
            else if (tag == "irregular")
                  m->setIrregular(e.readBool());
            else if (tag == "breakMultiMeasureRest")
                  m->setBreakMultiMeasureRest(e.readBool());
            else if (tag == "sysInitBarLineType") {
                  const QString& val(e.readElementText());
                  BarLine* barLine = new BarLine(score);
                  barLine->setTrack(e.track());
                  barLine->setBarLineType(val);
                  segment = m->getSegment(SegmentType::BeginBarLine, m->tick());
                  segment->add(barLine);
                  }
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score);
                  tuplet->setTrack(e.track());
                  tuplet->setTick(e.tick());
                  tuplet->setParent(m);
                  readTuplet(tuplet, e);
                  e.addTuplet(tuplet);
                  }
            else if (tag == "startRepeat") {
                  m->setRepeatStart(true);
                  e.readNext();
                  }
            else if (tag == "endRepeat") {
                  m->setRepeatCount(e.readInt());
                  m->setRepeatEnd(true);
                  }
            else if (tag == "vspacer" || tag == "vspacerDown") {
                  if (!m->vspacerDown(staffIdx)) {
                        Spacer* spacer = new Spacer(score);
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->vspacerDown(staffIdx)->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (!m->vspacerUp(staffIdx)) {
                        Spacer* spacer = new Spacer(score);
                        spacer->setSpacerType(SpacerType::UP);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->vspacerUp(staffIdx)->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  m->setStaffVisible(staffIdx, e.readInt());
            else if (tag == "slashStyle")
                  m->setStaffSlashStyle(staffIdx, e.readInt());
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score);
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  e.addBeam(beam);
                  }
            else if (tag == "Segment")
                  segment->read(e);
            else if (tag == "MeasureNumber") {
                  Text* noText = new Text(SubStyleId::MEASURE_NUMBER, score);
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
                  // noText->setFlag(ElementFlag::MOVABLE, false); ??
                  noText->setTrack(e.track());
                  noText->setParent(m);
                  m->setNoText(noText->staffIdx(), noText);
                  }
            else if (tag == "SystemDivider") {
                  SystemDivider* sd = new SystemDivider(score);
                  sd->read(e);
                  m->add(sd);
                  }
            else if (tag == "Ambitus") {
                  Ambitus* range = new Ambitus(score);
                  readAmbitus(range, e);
                  segment = m->getSegment(SegmentType::Ambitus, e.tick());
                  range->setParent(segment);          // a parent segment is needed for setTrack() to work
                  range->setTrack(trackZeroVoice(e.track()));
                  segment->add(range);
                  }
            else if (tag == "multiMeasureRest") {
                  m->setMMRestCount(e.readInt());
                  // set tick to previous measure
                  m->setTick(e.lastMeasure()->tick());
                  e.initTick(e.lastMeasure()->tick());
                  }
            else if (m->MeasureBase::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      e.checkTuplets();
      }

//---------------------------------------------------------
//   readBox
//---------------------------------------------------------

static void readBox(Box* b, XmlReader& e)
      {
      b->setLeftMargin(0.0);
      b->setRightMargin(0.0);
      b->setTopMargin(0.0);
      b->setBottomMargin(0.0);

      b->setBoxHeight(Spatium(0));     // override default set in constructor
      b->setBoxWidth(Spatium(0));
      bool keepMargins = false;        // whether original margins have to be kept when reading old file

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "HBox") {
                  HBox* hb = new HBox(b->score());
                  hb->read(e);
                  b->add(hb);
                  keepMargins = true;     // in old file, box nesting used outer box margins
                  }
            else if (tag == "VBox") {
                  VBox* vb = new VBox(b->score());
                  vb->read(e);
                  b->add(vb);
                  keepMargins = true;     // in old file, box nesting used outer box margins
                  }
            else if (tag == "Text") {
                  Text* t;
                  if (b->isTBox()) {
                        t = toTBox(b)->text();
                        readText206(e, t, t);
                        }
                  else {
                        t = new Text(b->score());
                        readText206(e, t, t);
                        if (t->empty()) {
                              qDebug("read empty text");
                              }
                        else
                              b->add(t);
                        }
                  }
            else if (!b->readProperties(e))
                  e.unknown();
            }

      // with .msc versions prior to 1.17, box margins were only used when nesting another box inside this box:
      // for backward compatibility set them to 0 in all other cases

      if (b->score()->mscVersion() <= 114 && (b->isHBox() || b->isVBox()) && !keepMargins)  {
            b->setLeftMargin(0.0);
            b->setRightMargin(0.0);
            b->setTopMargin(0.0);
            b->setBottomMargin(0.0);
            }
      }

//---------------------------------------------------------
//   readStaffContent
//---------------------------------------------------------

static void readStaffContent(Score* score, XmlReader& e)
      {
      int staff = e.intAttribute("id", 1) - 1;
      e.initTick(0);
      e.setTrack(staff * VOICES);

      if (staff == 0) {
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        Measure* measure = 0;
                        measure = new Measure(score);
                        measure->setTick(e.tick());
                        //
                        // inherit timesig from previous measure
                        //
                        Measure* m = e.lastMeasure(); // measure->prevMeasure();
                        Fraction f(m ? m->timesig() : Fraction(4,4));
                        measure->setLen(f);
                        measure->setTimesig(f);

                        readMeasure(measure, staff, e);
                        measure->checkMeasure(staff);
                        if (!measure->isMMRest()) {
                              score->measures()->add(measure);
                              e.setLastMeasure(measure);
                              e.initTick(measure->tick() + measure->ticks());
                              }
                        else {
                              // this is a multi measure rest
                              // always preceded by the first measure it replaces
                              Measure* m = e.lastMeasure();

                              if (m) {
                                    m->setMMRest(measure);
                                    measure->setTick(m->tick());
                                    }
                              }
                        }
                  else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                        Box* b = toBox(Element::name2Element(tag, score));
                        readBox(b, e);
                        b->setTick(e.tick());
                        score->measures()->add(b);
                        }
                  else if (tag == "tick")
                        e.initTick(score->fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      else {
            Measure* measure = score->firstMeasure();
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        if (measure == 0) {
                              qDebug("Score::readStaff(): missing measure!");
                              measure = new Measure(score);
                              measure->setTick(e.tick());
                              score->measures()->add(measure);
                              }
                        e.initTick(measure->tick());
                        readMeasure(measure, staff, e);
                        measure->checkMeasure(staff);
                        if (measure->isMMRest())
                              measure = e.lastMeasure()->nextMeasure();
                        else {
                              e.setLastMeasure(measure);
                              if (measure->mmRest())
                                    measure = measure->mmRest();
                              else
                                    measure = measure->nextMeasure();
                              }
                        }
                  else if (tag == "tick")
                        e.initTick(score->fileDivision(e.readInt()));
                  else
                        e.unknown();
                  }
            }
      }

//---------------------------------------------------------
//   readStyle
//---------------------------------------------------------

static void readStyle(MStyle* style, XmlReader& e)
      {
      QString oldChordDescriptionFile = style->value(Sid::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();

            if (tag == "TextStyle")
                  readTextStyle206(style, e);
            else if (tag == "Spatium")
                  style->set(Sid::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout")
                  readPageFormat(style, e);
            else if (tag == "displayInConcertPitch")
                  style->set(Sid::concertPitch, QVariant(bool(e.readInt())));
            else if (tag == "pedalY") {
                  qreal y = e.readDouble();
                  style->set(Sid::pedalPosBelow, QVariant(Spatium(y)));
                  }
            else if (tag == "lyricsDistance") {
                  qreal y = e.readDouble();
                  style->set(Sid::lyricsPosBelow, QVariant(Spatium(y)));
                  }
            else if (tag == "ottavaHook") {
                  qreal y = qAbs(e.readDouble());
                  style->set(Sid::ottavaHookAbove, y);
                  style->set(Sid::ottavaHookBelow, -y);
                  }
            else if (tag == "endBarDistance") {
                  double d = e.readDouble();
                  d += style->value(Sid::barWidth).toDouble();
                  d += style->value(Sid::endBarWidth).toDouble();
                  style->set(Sid::endBarDistance, QVariant(d));
                  }
            else if (tag == "ChordList") {
                  style->chordList()->clear();
                  style->chordList()->read(e);
                  style->setCustomChordList(true);
                  for (ChordFont f : style->chordList()->fonts) {
                        if (f.family == "MuseJazz") {
                              f.family = "MuseJazz Text";
                              }
                        }
                  chordListTag = true;
                  }
            else
                  if (!style->readProperties(e)) {
                        e.skipCurrentElement();
                        }
            }

      // if we just specified a new chord description file
      // and didn't encounter a ChordList tag
      // then load the chord description file

      QString newChordDescriptionFile = style->value(Sid::chordDescriptionFile).toString();
      if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
            if (!newChordDescriptionFile.startsWith("chords_") && style->value(Sid::chordStyle).toString() == "std") {
                  // should not normally happen,
                  // but treat as "old" (114) score just in case
                  style->set(Sid::chordStyle, QVariant(QString("custom")));
                  style->set(Sid::chordsXmlFile, QVariant(true));
                  qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
                  }
            if (style->value(Sid::chordStyle).toString() == "custom")
                  style->setCustomChordList(true);
            else
                  style->setCustomChordList(false);
            style->chordList()->unload();
            }

      // make sure we have a chordlist
      if (!style->chordList()->loaded() && !chordListTag) {
            if (style->value(Sid::chordsXmlFile).toBool())
                  style->chordList()->read("chords.xml");
            style->chordList()->read(newChordDescriptionFile);
            }
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

static bool readScore(Score* score, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaffContent(score, e);
            else if (tag == "siglist")
                  score->sigmap()->read(e, score->fileDivision());
            else if (tag == "Omr") {
#ifdef OMR
                  score->masterScore()->setOmr(new Omr(score));
                  score->masterScore()->omr()->read(e);
#else
                  e.skipCurrentElement();
#endif
                  }
            else if (tag == "Audio") {
                  score->setAudio(new Audio);
                  score->audio()->read(e);
                  }
            else if (tag == "showOmr")
                  score->masterScore()->setShowOmr(e.readInt());
            else if (tag == "playMode")
                  score->setPlayMode(PlayMode(e.readInt()));
            else if (tag == "LayerTag") {
                  int id = e.intAttribute("id");
                  const QString& tag = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        score->layerTags()[id] = tag;
                        score->layerTagComments()[id] = val;
                        }
                  }
            else if (tag == "Layer") {
                  Layer layer;
                  layer.name = e.attribute("name");
                  layer.tags = e.attribute("mask").toUInt();
                  score->layer().append(layer);
                  e.readNext();
                  }
            else if (tag == "currentLayer")
                  score->setCurrentLayer(e.readInt());
            else if (tag == "Synthesizer")
                  score->synthesizerState().read(e);
            else if (tag == "Division")
                  score->setFileDivision(e.readInt());
            else if (tag == "showInvisible")
                  score->setShowInvisible(e.readInt());
            else if (tag == "showUnprintable")
                  score->setShowUnprintable(e.readInt());
            else if (tag == "showFrames")
                  score->setShowFrames(e.readInt());
            else if (tag == "showMargins")
                  score->setShowPageborders(e.readInt());
            else if (tag == "Style") {
                  qreal sp = score->style().value(Sid::spatium).toDouble();
                  readStyle(&score->style(), e);
                  if (score->style().value(Sid::MusicalTextFont).toString() == "MuseJazz")
                        score->style().set(Sid::MusicalTextFont, "MuseJazz Text");
                  // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
                  if (score->layoutMode() == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        score->style().set(Sid::spatium, sp);
                        }
                  score->setScoreFont(ScoreFont::fontFactory(score->style().value(Sid::MusicalSymbolFont).toString()));
                  }
            else if (tag == "copyright" || tag == "rights") {
                  Text* text = new Text(score);
                  text->read(e);
                  score->setMetaTag("copyright", text->xmlText());
                  delete text;
                  }
            else if (tag == "movement-number")
                  score->setMetaTag("movementNumber", e.readElementText());
            else if (tag == "movement-title")
                  score->setMetaTag("movementTitle", e.readElementText());
            else if (tag == "work-number")
                  score->setMetaTag("workNumber", e.readElementText());
            else if (tag == "work-title")
                  score->setMetaTag("workTitle", e.readElementText());
            else if (tag == "source")
                  score->setMetaTag("source", e.readElementText());
            else if (tag == "metaTag") {
                  QString name = e.attribute("name");
                  score->setMetaTag(name, e.readElementText());
                  }
            else if (tag == "Part") {
                  Part* part = new Part(score);
                  readPart(part, e);
                  score->parts().push_back(part);
                  }
            else if ((tag == "HairPin")   // TODO: do this elements exist here?
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Slur")
                || (tag == "Pedal")) {
                  Spanner* s = toSpanner(Element::name2Element(tag, score));
                  s->read(e);
                  score->addSpanner(s);
                  }
            else if (tag == "Excerpt") {
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        if (score->isMaster()) {
                              Excerpt* ex = new Excerpt(static_cast<MasterScore*>(score));
                              ex->read(e);
                              score->excerpts().append(ex);
                              }
                        else {
                              qDebug("Score::read(): part cannot have parts");
                              e.skipCurrentElement();
                              }
                        }
                  }
            else if (tag == "Score") {          // recursion
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        e.tracks().clear();
                        MasterScore* m = score->masterScore();
                        Score* s = new Score(m, MScore::defaultStyle());
                        Excerpt* ex = new Excerpt(m);

                        ex->setPartScore(s);
                        ex->setTracks(e.tracks());
                        e.setLastMeasure(nullptr);
                        // s->read(e);
                        readScore(s, e);
                        m->addExcerpt(ex);
                        }
                  }
            else if (tag == "PageList")
                  e.skipCurrentElement();
            else if (tag == "name") {
                  QString n = e.readElementText();
                  if (!score->isMaster())             //ignore the name if it's not a child score
                        score->excerpt()->setTitle(n);
                  }
            else if (tag == "layoutMode") {
                  QString s = e.readElementText();
                  if (s == "line")
                        score->setLayoutMode(LayoutMode::LINE);
                  else if (s == "system")
                        score->setLayoutMode(LayoutMode::SYSTEM);
                  else
                        qDebug("layoutMode: %s", qPrintable(s));
                  }
            else
                  e.unknown();
            }
      if (e.error() != QXmlStreamReader::NoError) {
            qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
            MScore::lastError = QObject::tr("XML read error at line %1 column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
            return false;
            }

      score->connectTies();

      score->setFileDivision(MScore::division);

      //
      //    sanity check for barLineSpan
      //
#if 0 // TODO:barline
      for (Staff* st : score->staves()) {
            int barLineSpan = st->barLineSpan();
            int idx = st->idx();
            int n = score->nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span until last staff
                  barLineSpan = n - idx;
                  st->setBarLineSpan(barLineSpan);
                  }
            else if (idx == 0 && barLineSpan == 0) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  // span from the first staff until the start of the next span
                  barLineSpan = 1;
                  for (int i = 1; i < n; ++i) {
                        if (score->staff(i)->barLineSpan() == 0)
                              ++barLineSpan;
                        else
                              break;
                        }
                  st->setBarLineSpan(barLineSpan);
                  }
            // check spanFrom
            int minBarLineFrom = st->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (st->barLineFrom() < minBarLineFrom)
                  st->setBarLineFrom(minBarLineFrom);
            if (st->barLineFrom() > st->lines(0) * 2)
                  st->setBarLineFrom(st->lines(0) * 2);
            // check spanTo
            Staff* stTo = st->barLineSpan() <= 1 ? st : score->staff(idx + st->barLineSpan() - 1);
            // 1-line staves have special bar line spans
            int maxBarLineTo        = stTo->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines(0)*2;
            int defaultBarLineTo    = stTo->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (stTo->lines(0) - 1) * 2;
            if (st->barLineTo() == UNKNOWN_BARLINE_TO)
                  st->setBarLineTo(defaultBarLineTo);
            if (st->barLineTo() < MIN_BARLINE_SPAN_FROMTO)
                  st->setBarLineTo(MIN_BARLINE_SPAN_FROMTO);
            if (st->barLineTo() > maxBarLineTo)
                  st->setBarLineTo(maxBarLineTo);
            // on single staff span, check spanFrom and spanTo are distant enough
            if (st->barLineSpan() == 1) {
                  if (st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
                        st->setBarLineFrom(0);
                        st->setBarLineTo(defaultBarLineTo);
                        }
                  }
            }
#endif
      score->fixTicks();
      if (score->isMaster()) {
            MasterScore* ms = static_cast<MasterScore*>(score);
            if (!ms->omr())
                  ms->setShowOmr(false);
            ms->rebuildMidiMapping();
            ms->updateChannel();
            ms->createPlayEvents();
            }
      return true;
      }

//---------------------------------------------------------
//   read
//  <page-layout>
//      <page-height>
//      <page-width>
//      <landscape>1</landscape>
//      <page-margins type="both">
//         <left-margin>28.3465</left-margin>
//         <right-margin>28.3465</right-margin>
//         <top-margin>28.3465</top-margin>
//         <bottom-margin>56.6929</bottom-margin>
//         </page-margins>
//      </page-layout>
//---------------------------------------------------------

void PageFormat::read(XmlReader& e)
      {
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      QString type;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "page-margins") {
                  type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        qreal val = e.readDouble() * 0.5 / PPI;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              e.unknown();
                        }
                  _twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        _oddLeftMargin   = lm;
                        _oddRightMargin  = rm;
                        _oddTopMargin    = tm;
                        _oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        _evenLeftMargin   = lm;
                        _evenRightMargin  = rm;
                        _evenTopMargin    = tm;
                        _evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height")
                  _size.rheight() = e.readDouble() * 0.5 / PPI;
            else if (tag == "page-width")
                  _size.rwidth() = e.readDouble() * .5 / PPI;
            else
                  e.unknown();
            }
      qreal w1        = _size.width() - _oddLeftMargin - _oddRightMargin;
      qreal w2        = _size.width() - _evenLeftMargin - _evenRightMargin;
      _printableWidth = qMin(w1, w2);     // silently adjust right margins
      }


//---------------------------------------------------------
//   read206
//    import old version > 1.3  and < 3.x files
//---------------------------------------------------------

Score::FileError MasterScore::read206(XmlReader& e)
      {
//      qDebug("read206");

      for (unsigned int i = 0; i < sizeof(style206)/sizeof(*style206); ++i)
            style().set(style206[i].idx, style206[i].val);


      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readIntHex());
            else if (tag == "Score") {
                  if (!readScore(this, e))
                        return FileError::FILE_BAD_FORMAT;
                  }
            else if (tag == "Revision") {
                  Revision* revision = new Revision;
                  revision->read(e);
                  revisions()->add(revision);
                  }
            }
      int id = 1;
      for (LinkedElements* le : e.linkIds())
            le->setLid(this, id++);

      for (Staff* s : staves())
            s->updateOttava();

      // fix segment span
      SegmentType st = SegmentType::BarLineType;
      for (Segment* s = firstSegment(st); s; s = s->next1(st)) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  BarLine* b = toBarLine(s->element(staffIdx * VOICES));
                  if (!b)
                        continue;
                  int sp = b->spanStaff();
                  if (sp == 0)
                        continue;
                  for (int span = 1; span <= sp; ++span) {
                        BarLine* nb = toBarLine(s->element((staffIdx + span) * VOICES));
                        if (!nb) {
                              nb = b->clone();
                              nb->setTrack((staffIdx + span) * VOICES);
                              s->add(nb);
                              }
                        nb->setSpanStaff(sp - span);
                        }
                  staffIdx += sp;
                  }
            }
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* s = staff(staffIdx);
            int sp = s->barLineSpan();
            if (sp == 0)
                  continue;
            for (int span = 1; span <= sp; ++span) {
                  Staff* ns = staff(staffIdx + span);
                  ns->setBarLineSpan(sp - span);
                  }
            staffIdx += sp;
            }

      // treat reading a 2.06 file as import
      // on save warn if old file will be overwritten
      setCreated(true);
      // don't autosave (as long as there's no change to the score)
      setAutosaveDirty(false);

      return FileError::FILE_NO_ERROR;
      }

}

