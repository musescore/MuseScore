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
#include "timesig.h"
#include "tuplet.h"
#include "spacer.h"
#include "stafftext.h"
#include "repeat.h"
#include "breath.h"
#include "tremolo.h"
#include "articulation.h"
#include "utils.h"
#include "accidental.h"
#include "fingering.h"
#include "marker.h"
#include "read206.h"
#include "bracketItem.h"

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
      { StyleIdx::lyricsMinBottomDistance,      Spatium(2) },
      { StyleIdx::frameSystemDistance,          Spatium(1.0) },
      { StyleIdx::minMeasureWidth,              Spatium(4.0) },
      { StyleIdx::endBarDistance,               Spatium(0.30) },

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
      { StyleIdx::hideInstrumentNameIfOneInstrument, QVariant(false) },
      };

#define MM(x) ((x)/INCH)

//---------------------------------------------------------
//   PaperSize
//---------------------------------------------------------

struct PaperSize {
      const char* name;
      qreal w, h;            // size in inch
      PaperSize(const char* n, qreal wi, qreal hi)
         : name(n), w(wi), h(hi) {}
      };

static const PaperSize paperSizes114[] = {
      PaperSize("Custom",    MM(1),    MM(1)),
      PaperSize("A4",        MM(210),  MM(297)),
      PaperSize("B5",        MM(176),  MM(250)),
      PaperSize("Letter",    8.5,      11),
      PaperSize("Legal",     8.5,      14),
      PaperSize("Executive", 7.5,      10),
      PaperSize("A0",        MM(841),  MM(1189)),
      PaperSize("A1",        MM(594),  MM(841)),
      PaperSize("A2",        MM(420),  MM(594)),
      PaperSize("A3",        MM(297),  MM(420)),
      PaperSize("A5",        MM(148),  MM(210)),
      PaperSize("A6",        MM(105),  MM(148)),
      PaperSize("A7",        MM(74),   MM(105)),
      PaperSize("A8",        MM(52),   MM(74)),
      PaperSize("A9",        MM(37),   MM(52)),
      PaperSize("A10",       MM(26),   MM(37)),
      PaperSize("B0",        MM(1000), MM(1414)),
      PaperSize("B1",        MM(707),  MM(1000)),
      PaperSize("B2",        MM(500),  MM(707)),
      PaperSize("B3",        MM(353),  MM(500)),
      PaperSize("B4",        MM(250),  MM(353)),
      PaperSize("B6",        MM(125),  MM(176)),
      PaperSize("B7",        MM(88),   MM(125)),
      PaperSize("B8",        MM(62),   MM(88)),
      PaperSize("B9",        MM(44),   MM(62)),
      PaperSize("B10",       MM(31),   MM(44)),
      PaperSize("Comm10E",   MM(105),  MM(241)),
      PaperSize("DLE",       MM(110),  MM(220)),
      PaperSize("Folio",     MM(210),  MM(330)),
      PaperSize("Ledger",    MM(432),  MM(279)),
      PaperSize("Tabloid",   MM(279),  MM(432)),
      PaperSize(0,           MM(1),    MM(1))   // mark end of list
      };


//---------------------------------------------------------
//   getPaperSize
//---------------------------------------------------------

static const PaperSize* getPaperSize114(const QString& name)
      {
      for (int i = 0; paperSizes114[i].name; ++i) {
            if (name == paperSizes114[i].name)
                  return &paperSizes114[i];
            }
      qDebug("unknown paper size");
      return &paperSizes114[0];
      }

//---------------------------------------------------------
//   convertFromHtml
//---------------------------------------------------------

QString convertFromHtml(TextBase* t, const QString& ss)
      {
      QTextDocument doc;
      doc.setHtml(ss.trimmed());

      QString s;
      qreal size     = t->size();   // textStyle().size();
      QString family = t->family(); // textStyle().family();

      for (auto b = doc.firstBlock(); b.isValid() ; b = b.next()) {
            if (!s.isEmpty())
                  s += "\n";
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                  QTextFragment f = it.fragment();
                  if (f.isValid()) {
                        QTextCharFormat tf = f.charFormat();
                        QFont font = tf.font();
                        qreal htmlSize = font.pointSizeF();
                        // html font sizes may have spatium adjustments; need to undo this
                        if (t->sizeIsSpatiumDependent())
                              htmlSize *= SPATIUM20 / t->spatium();
                        if (fabs(size - htmlSize) > 0.1) {
                              size = htmlSize;
                              s += QString("<font size=\"%1\"/>").arg(size);
                              }
                        if (family != font.family()) {
                              family = font.family();
                              s += QString("<font face=\"%1\"/>").arg(family);
                              }
                        if (font.bold())
                              s += "<b>";
                        if (font.italic())
                              s += "<i>";
                        if (font.underline())
                              s += "<u>";
                        s += f.text().toHtmlEscaped();
                        if (font.underline())
                              s += "</u>";
                        if (font.italic())
                              s += "</i>";
                        if (font.bold())
                              s += "</b>";
                        }
                  }
            }

      s.replace(QChar(0xe10e), QString("<sym>accidentalNatural</sym>"));  //natural
      s.replace(QChar(0xe10c), QString("<sym>accidentalSharp</sym>"));    // sharp
      s.replace(QChar(0xe10d), QString("<sym>accidentalFlat</sym>"));     // flat
      s.replace(QChar(0xe104), QString("<sym>metNoteHalfUp</sym>")),      // note2_Sym
      s.replace(QChar(0xe105), QString("<sym>metNoteQuarterUp</sym>"));   // note4_Sym
      s.replace(QChar(0xe106), QString("<sym>metNote8thUp</sym>"));       // note8_Sym
      s.replace(QChar(0xe107), QString("<sym>metNote16thUp</sym>"));      // note16_Sym
      s.replace(QChar(0xe108), QString("<sym>metNote32ndUp</sym>"));      // note32_Sym
      s.replace(QChar(0xe109), QString("<sym>metNote64thUp</sym>"));      // note64_Sym
      s.replace(QChar(0xe10a), QString("<sym>metAugmentationDot</sym>")); // dot
      s.replace(QChar(0xe10b), QString("<sym>metAugmentationDot</sym><sym>space</sym><sym>metAugmentationDot</sym>"));    // dotdot
      s.replace(QChar(0xe167), QString("<sym>segno</sym>"));              // segno
      s.replace(QChar(0xe168), QString("<sym>coda</sym>"));               // coda
      s.replace(QChar(0xe169), QString("<sym>codaSquare</sym>"));         // varcoda
      return s;
      }


//---------------------------------------------------------
//   readTextProperties
//---------------------------------------------------------

static bool readTextProperties(XmlReader& e, TextBase* t, Element* be)
      {
      const QStringRef& tag(e.name());
      if (tag == "style") {
            int i = e.readInt();
            SubStyle ss = SubStyle::DEFAULT;
            switch (i) {
                  case 2:  ss = SubStyle::TITLE;     break;
                  case 3:  ss = SubStyle::SUBTITLE;  break;
                  case 4:  ss = SubStyle::COMPOSER;  break;
                  case 5:  ss = SubStyle::POET;      break;
                  case 6:  ss = SubStyle::LYRIC1;    break;
                  case 7:  ss = SubStyle::LYRIC2;    break;
                  case 8:  ss = SubStyle::FINGERING; break;
                  case 9:  ss = SubStyle::INSTRUMENT_LONG;    break;
                  case 10: ss = SubStyle::INSTRUMENT_SHORT;   break;
                  case 11: ss = SubStyle::INSTRUMENT_EXCERPT; break;

                  case 12: ss = SubStyle::DYNAMICS;  break;
                  case 13: ss = SubStyle::EXPRESSION;   break;
                  case 14: ss = SubStyle::TEMPO;     break;
                  case 15: ss = SubStyle::METRONOME; break;
                  case 16: ss = SubStyle::FOOTER;    break;  // TextStyleType::COPYRIGHT
                  case 17: ss = SubStyle::MEASURE_NUMBER; break;
                  case 18: ss = SubStyle::FOOTER; break;    // TextStyleType::PAGE_NUMBER_ODD
                  case 19: ss = SubStyle::FOOTER; break;    // TextStyleType::PAGE_NUMBER_EVEN
                  case 20: ss = SubStyle::TRANSLATOR; break;
                  case 21: ss = SubStyle::TUPLET;     break;

                  case 22: ss = SubStyle::SYSTEM;         break;
                  case 23: ss = SubStyle::STAFF;          break;
                  case 24: ss = SubStyle::HARMONY;        break;
                  case 25: ss = SubStyle::REHEARSAL_MARK; break;
                  case 26: ss = SubStyle::REPEAT_LEFT;         break;
//??                  case 27: ss = SubStyle::VOLTA;          break;
                  case 28: ss = SubStyle::FRAME;          break;
                  case 29: ss = SubStyle::TEXTLINE;       break;
                  case 30: ss = SubStyle::GLISSANDO;      break;
                  case 31: ss = SubStyle::STRING_NUMBER;  break;

                  case 32: ss = SubStyle::OTTAVA;  break;
//??                  case 33: ss = SubStyle::BENCH;   break;
                  case 34: ss = SubStyle::HEADER;  break;
                  case 35: ss = SubStyle::FOOTER;  break;
                  case 0:
                  default:
                        qDebug("style %d invalid", i);
                        ss = SubStyle::DEFAULT;
                        break;
                  }
            be->initSubStyle(ss);
            }
      else if (tag == "subtype")
            e.skipCurrentElement();
      else if (tag == "html-data")
            t->setXmlText(convertFromHtml(t, e.readXml()));
      else if (tag == "foregroundColor")  // same as "color" ?
            e.skipCurrentElement();
      else if (tag == "frame")
            t->setHasFrame(e.readBool());
      else if (tag == "halign") {
            Align align = Align(int(t->align()) & int(~(Align::HCENTER | Align::RIGHT)));
            const QString& val(e.readElementText());
            if (val == "center")
                  align = align | Align::HCENTER;
            else if (val == "right")
                  align = align | Align::RIGHT;
            else if (val == "left")
                  ;
            else
                  qDebug("readText: unknown alignment: <%s>", qPrintable(val));
            t->setAlign(align);
            }
      else if (!t->readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   readText
//---------------------------------------------------------

static void readText(XmlReader& e, Text* t, Element* be)
      {
      while (e.readNextStartElement()) {
            if (!readTextProperties(e, t, be))
                  e.unknown();
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
                  QString text(e.readElementText());
                  bool isInt;
                  int i = text.toInt(&isInt);
                  if (isInt) {
                        a->setBracket(i & 0x8000 ? AccidentalBracket::PARENTHESIS : AccidentalBracket::BRACKET);
                        i &= ~0x8000;
                        AccidentalType at;
                        switch (i) {
                              case 0:
                                    at = AccidentalType::NONE;
                                    break;
                              case 6:
                                    a->setBracket(AccidentalBracket::PARENTHESIS);
                                    // fall through
                              case 1:
                              case 11:
                                    at = AccidentalType::SHARP;
                                    break;
                              case 7:
                                    a->setBracket(AccidentalBracket::PARENTHESIS);
                                    // fall through
                              case 2:
                              case 12:
                                    at = AccidentalType::FLAT;
                                    break;
                              case 8:
                                    a->setBracket(AccidentalBracket::PARENTHESIS);
                                    // fall through
                              case 3:
                              case 13:
                                    at = AccidentalType::SHARP2;
                                    break;
                              case 9:
                                    a->setBracket(AccidentalBracket::PARENTHESIS);
                                    // fall through
                              case 4:
                              case 14:
                                    at = AccidentalType::FLAT2;
                                    break;
                              case 10:
                                    a->setBracket(AccidentalBracket::PARENTHESIS);
                                    // fall through
                              case 5:
                              case 15:
                                    at = AccidentalType::NATURAL;
                                    break;
                              case 16:
                                    at = AccidentalType::FLAT_SLASH;
                                    break;
                              case 17:
                                    at = AccidentalType::FLAT_SLASH2;
                                    break;
                              case 18:
                                    at = AccidentalType::MIRRORED_FLAT2;
                                    break;
                              case 19:
                                    at = AccidentalType::MIRRORED_FLAT;
                                    break;
                              case 20:
                                    at = AccidentalType::NONE;//AccidentalType::MIRRORED_FLAT_SLASH;
                                    break;
                              case 21:
                                    at = AccidentalType::NONE;//AccidentalType::FLAT_FLAT_SLASH;
                                    break;
                              case 22:
                                    at = AccidentalType::SHARP_SLASH;
                                    break;
                              case 23:
                                    at = AccidentalType::SHARP_SLASH2;
                                    break;
                              case 24:
                                    at = AccidentalType::SHARP_SLASH3;
                                    break;
                              case 25:
                                    at = AccidentalType::SHARP_SLASH4;
                                    break;
                              case 26:
                                    at = AccidentalType::SHARP_ARROW_UP;
                                    break;
                              case 27:
                                    at = AccidentalType::SHARP_ARROW_DOWN;
                                    break;
                              case 28:
                                    at = AccidentalType::NONE;//AccidentalType::SHARP_ARROW_BOTH;
                                    break;
                              case 29:
                                    at = AccidentalType::FLAT_ARROW_UP;
                                    break;
                              case 30:
                                    at = AccidentalType::FLAT_ARROW_DOWN;
                                    break;
                              case 31:
                                    at = AccidentalType::NONE;//AccidentalType::FLAT_ARROW_BOTH;
                                    break;
                              case 32:
                                    at = AccidentalType::NATURAL_ARROW_UP;
                                    break;
                              case 33:
                                    at = AccidentalType::NATURAL_ARROW_DOWN;
                                    break;
                              case 34:
                                    at = AccidentalType::NONE;//AccidentalType::NATURAL_ARROW_BOTH;
                                    break;
                              default:
                                    at = AccidentalType::NONE;
                                    break;
                              }
                        a->setAccidentalType(at);
                        }
                  else {
                        const static std::map<QString, AccidentalType> accMap = {
                           {"none", AccidentalType::NONE}, {"sharp", AccidentalType::SHARP},
                           {"flat", AccidentalType::FLAT}, {"natural", AccidentalType::NATURAL},
                           {"double sharp", AccidentalType::SHARP2}, {"double flat", AccidentalType::FLAT2},
                           {"flat-slash", AccidentalType::FLAT_SLASH}, {"flat-slash2", AccidentalType::FLAT_SLASH2},
                           {"mirrored-flat2", AccidentalType::MIRRORED_FLAT2}, {"mirrored-flat", AccidentalType::MIRRORED_FLAT},
                           {"sharp-slash", AccidentalType::SHARP_SLASH}, {"sharp-slash2", AccidentalType::SHARP_SLASH2},
                           {"sharp-slash3", AccidentalType::SHARP_SLASH3}, {"sharp-slash4", AccidentalType::SHARP_SLASH4},
                           {"sharp arrow up", AccidentalType::SHARP_ARROW_UP}, {"sharp arrow down", AccidentalType::SHARP_ARROW_DOWN},
                           {"flat arrow up", AccidentalType::FLAT_ARROW_UP}, {"flat arrow down", AccidentalType::FLAT_ARROW_DOWN},
                           {"natural arrow up", AccidentalType::NATURAL_ARROW_UP}, {"natural arrow down", AccidentalType::NATURAL_ARROW_DOWN},
                           {"sori", AccidentalType::SORI}, {"koron", AccidentalType::KORON}
                        };
                        auto it = accMap.find(text);
                        if (it == accMap.end()) {
                              qDebug("invalid type %s", qPrintable(text));
                              a->setAccidentalType(AccidentalType::NONE);
                              }
                        else
                              a->setAccidentalType(it->second);
                        }
                  }
            else if (tag == "role") {
                  AccidentalRole r = AccidentalRole(e.readInt());
                  if (r == AccidentalRole::AUTO || r == AccidentalRole::USER)
                        a->setRole(r);
                  }
            else if (tag == "small")
                  a->setSmall(e.readInt());
            else if (tag == "offset")
                  e.skipCurrentElement(); // ignore manual layout in older scores
            else if (a->Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   convertHeadGroup
//---------------------------------------------------------

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
            default:
                  val = NoteHead::Group::HEAD_NORMAL;
            }
      return val;
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

static void readNote(Note* note, XmlReader& e)
      {
      e.hasAccidental = false;                     // used for userAccidental backward compatibility

      note->setTpc1(Tpc::TPC_INVALID);
      note->setTpc2(Tpc::TPC_INVALID);

      if (e.hasAttribute("pitch"))                   // obsolete
            note->setPitch(e.intAttribute("pitch"));
      if (e.hasAttribute("tpc"))                     // obsolete
            note->setTpc1(e.intAttribute("tpc"));

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Accidental") {
                  // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
                  // if a userAccidental has some other property set (like for instance offset)
                  Accidental* a;
                  if (e.hasAccidental)            // if the other tag has already been read,
                        a = note->accidental();        // re-use the accidental it constructed
                  else
                        a = new Accidental(note->score());
                  // the accidental needs to know the properties of the
                  // track it belongs to (??)
                  a->setTrack(note->track());
                  readAccidental(a, e);
                  if (!e.hasAccidental)          // only the new accidental, if it has been added previously
                        note->add(a);
                  e.hasAccidental = true;   // we now have an accidental
                  }
            else if (tag == "Text") {
                  Fingering* f = new Fingering(note->score());
                  f->read(e);
                  note->add(f);
                  }
            else if (tag == "onTimeType") {
                  if (e.readElementText() == "offset")
                        note->setOnTimeType(2);
                  else
                        note->setOnTimeType(1);
                  }
            else if (tag == "offTimeType") {
                  if (e.readElementText() == "offset")
                        note->setOffTimeType(2);
                  else
                        note->setOffTimeType(1);
                  }
            else if (tag == "onTimeOffset") {
                  if (note->onTimeType() == 1)
                        note->setOnTimeOffset(e.readInt() * 1000 / note->chord()->actualTicks());
                  else
                        note->setOnTimeOffset(e.readInt() * 10);
                  }
            else if (tag == "offTimeOffset") {
                  if (note->offTimeType() == 1)
                        note->setOffTimeOffset(1000 + (e.readInt() * 1000 / note->chord()->actualTicks()));
                  else
                        note->setOffTimeOffset(1000 + (e.readInt() * 10));
                  }
            else if (tag == "userAccidental") {
                  QString val(e.readElementText());
                  bool ok;
                  int k = val.toInt(&ok);
                  if (ok) {
                        // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
                        // if a userAccidental has some other property set (like for instance offset)
                        // only construct a new accidental, if the other tag has not been read yet
                        // (<userAccidental> tag is only used in older scores: no need to check the score mscVersion)
                        if (!e.hasAccidental) {
                              Accidental* a = new Accidental(note->score());
                              note->add(a);
                              }
                        // TODO: for backward compatibility
                        bool bracket = k & 0x8000;
                        k &= 0xfff;
                        AccidentalType at = AccidentalType::NONE;
                        switch(k) {
                              case 0: at = AccidentalType::NONE; break;
                              case 1: at = AccidentalType::SHARP; break;
                              case 2: at = AccidentalType::FLAT; break;
                              case 3: at = AccidentalType::SHARP2; break;
                              case 4: at = AccidentalType::FLAT2; break;
                              case 5: at = AccidentalType::NATURAL; break;

                              case 6: at = AccidentalType::FLAT_SLASH; break;
                              case 7: at = AccidentalType::FLAT_SLASH2; break;
                              case 8: at = AccidentalType::MIRRORED_FLAT2; break;
                              case 9: at = AccidentalType::MIRRORED_FLAT; break;
                              case 10: at = AccidentalType::NONE; break; // AccidentalType::MIRRORED_FLAT_SLASH
                              case 11: at = AccidentalType::NONE; break; // AccidentalType::FLAT_FLAT_SLASH

                              case 12: at = AccidentalType::SHARP_SLASH; break;
                              case 13: at = AccidentalType::SHARP_SLASH2; break;
                              case 14: at = AccidentalType::SHARP_SLASH3; break;
                              case 15: at = AccidentalType::SHARP_SLASH4; break;

                              case 16: at = AccidentalType::SHARP_ARROW_UP; break;
                              case 17: at = AccidentalType::SHARP_ARROW_DOWN; break;
                              case 18: at = AccidentalType::NONE; break; // AccidentalType::SHARP_ARROW_BOTH
                              case 19: at = AccidentalType::FLAT_ARROW_UP; break;
                              case 20: at = AccidentalType::FLAT_ARROW_DOWN; break;
                              case 21: at = AccidentalType::NONE; break; // AccidentalType::FLAT_ARROW_BOTH
                              case 22: at = AccidentalType::NATURAL_ARROW_UP; break;
                              case 23: at = AccidentalType::NATURAL_ARROW_DOWN; break;
                              case 24: at = AccidentalType::NONE; break; // AccidentalType::NATURAL_ARROW_BOTH
                              case 25: at = AccidentalType::SORI; break;
                              case 26: at = AccidentalType::KORON; break;
                              }
                        note->accidental()->setAccidentalType(at);

                        note->accidental()->setBracket(AccidentalBracket(bracket));

                        note->accidental()->setRole(AccidentalRole::USER);
                        e.hasAccidental = true;   // we now have an accidental
                        }
                  }
            else if (tag == "offset")
                  e.skipCurrentElement(); // ignore manual layout in older scores
            else if (tag == "move")
                  note->chord()->setStaffMove(e.readInt());
            else if (tag == "head") {
                  int i = e.readInt();
                  NoteHead::Group val = convertHeadGroup(i);
                  note->setHeadGroup(val);
                  }
            else if (tag == "headType") {
                  int i = e.readInt();
                  NoteHead::Type val;
                  switch (i) {
                        case 1:
                              val = NoteHead::Type::HEAD_WHOLE;
                              break;
                        case 2:
                              val = NoteHead::Type::HEAD_HALF;
                              break;
                        case 3:
                              val = NoteHead::Type::HEAD_QUARTER;
                              break;
                        case 4:
                              val = NoteHead::Type::HEAD_BREVIS;
                              break;
                        default:
                              val = NoteHead::Type::HEAD_AUTO;
                        }
                  note->setHeadType(val);
                  }
            else if (note->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      // ensure sane values:
      note->setPitch(limit(note->pitch(), 0, 127));

      if (note->concertPitch())
            note->setTpc2(Tpc::TPC_INVALID);
      else {
            note->setPitch(note->pitch() + note->transposition());
            note->setTpc2(note->tpc1());
            note->setTpc1(Tpc::TPC_INVALID);
            }
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
                  v.flip();
                  note->setTpc2(Ms::transposeTpc(note->tpc1(), v, true));
                  }
            }
      }

//---------------------------------------------------------
//   readClefType
//---------------------------------------------------------

static ClefType readClefType(const QString& s)
      {
      ClefType ct = ClefType::G;
      bool ok;
      int i = s.toInt(&ok);
      if (ok) {
            // convert obsolete old coding
            switch (i) {
                  default:
                  case  0: ct = ClefType::G; break;
                  case  1: ct = ClefType::G8_VA; break;
                  case  2: ct = ClefType::G15_MA; break;
                  case  3: ct = ClefType::G8_VB; break;
                  case  4: ct = ClefType::F; break;
                  case  5: ct = ClefType::F8_VB; break;
                  case  6: ct = ClefType::F15_MB; break;
                  case  7: ct = ClefType::F_B; break;
                  case  8: ct = ClefType::F_C; break;
                  case  9: ct = ClefType::C1; break;
                  case 10: ct = ClefType::C2; break;
                  case 11: ct = ClefType::C3; break;
                  case 12: ct = ClefType::C4; break;
                  case 13: ct = ClefType::TAB; break;
                  case 14: ct = ClefType::PERC; break;
                  case 15: ct = ClefType::C5; break;
                  case 16: ct = ClefType::G_1; break;
                  case 17: ct = ClefType::F_8VA; break;
                  case 18: ct = ClefType::F_15MA; break;
                  case 19: ct = ClefType::PERC; break;      // PERC2 no longer supported
                  case 20: ct = ClefType::TAB_SERIF; break;
                  }
            }
      return ct;
      }

//---------------------------------------------------------
//   readClef
//---------------------------------------------------------

static void readClef(Clef* clef, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  clef->setClefType(readClefType(e.readElementText()));
            else if (!clef->readProperties(e))
                  e.unknown();
            }
      if (clef->clefType() == ClefType::INVALID)
            clef->setClefType(ClefType::G);
      }

//---------------------------------------------------------
//   readTuplet
//---------------------------------------------------------

static void readTuplet(Tuplet* tuplet, XmlReader& e)
      {
      int bl = -1;
      tuplet->setId(e.intAttribute("id", 0));

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")    // obsolete
                  e.skipCurrentElement();
            else if (tag == "hasNumber")  // obsolete even in 1.3
                  tuplet->setNumberType(e.readInt() ? Tuplet::NumberType::SHOW_NUMBER : Tuplet::NumberType::NO_TEXT);
            else if (tag == "hasLine") {  // obsolete even in 1.3
                  tuplet->setHasBracket(e.readInt());
                  tuplet->setBracketType(Tuplet::BracketType::AUTO_BRACKET);
                  }
            else if (tag == "baseLen")    // obsolete even in 1.3
                  bl = e.readInt();
            else if (tag == "tick")
                  tuplet->setTick(e.readInt());
            else if (!tuplet->readProperties(e))
                  e.unknown();
            }
      Fraction r = (tuplet->ratio() == 1) ? tuplet->ratio() : tuplet->ratio().reduced();
      // this may be wrong, but at this stage it is kept for compatibility. It will be corrected afterwards
      // during "sanitize" step
      Fraction f(r.denominator(), tuplet->baseLen().fraction().denominator());
      tuplet->setDuration(f.reduced());
      if (bl != -1) {         // obsolete, even in 1.3
            TDuration d;
            d.setVal(bl);
            tuplet->setBaseLen(d);
            d.setVal(bl * tuplet->ratio().denominator());
            tuplet->setDuration(d.fraction());
            }
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
            else if (tag == "Attribute" || tag == "Articulation") {
                  Element* el = readArticulation(chord, e);
                  if (el->isFermata())
                        chord->segment()->add(el);
                  else
                        chord->add(el);
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
            if (tag == "Attribute" || tag == "Articulation") {
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
//   readTempoText
//---------------------------------------------------------

void readTempoText(TempoText* t, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "tempo")
                  t->setTempo(e.readDouble());
            else if (!readTextProperties(e, t, t))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readStaffText
//---------------------------------------------------------

void readStaffText(StaffText* t, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readTextProperties(e, t, t))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

static void readMeasure(Measure* m, int staffIdx, XmlReader& e)
      {
      Segment* segment = 0;
      qreal _spatium = m->spatium();

      QList<Chord*> graceNotes;

      //sort tuplet elements. needed for nested tuplets #22537
      for (Tuplet* t : e.tuplets())
            t->sortElements();
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      m->createStaves(staffIdx);

      // tick is obsolete
      if (e.hasAttribute("tick"))
            e.initTick(m->score()->fileDivision(e.intAttribute("tick")));

      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  m->setLen(Fraction(sl[0].toInt(), sl[1].toInt()));
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            m->score()->sigmap()->add(m->tick(), SigEvent(m->len(), m->timesig()));
            m->score()->sigmap()->add(m->endTick(), SigEvent(m->timesig()));
            }

      Staff* staff = m->score()->staff(staffIdx);
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
                  e.initTick(m->score()->fileDivision(e.readInt()));
                  lastTick = e.tick();
                  }
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(m->score());
                  barLine->setTrack(e.track());
                  barLine->resetProperty(P_ID::BARLINE_SPAN);
                  barLine->resetProperty(P_ID::BARLINE_SPAN_FROM);
                  barLine->resetProperty(P_ID::BARLINE_SPAN_TO);

                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "subtype") {
                              BarLineType t;
                              switch (e.readInt()) {
                                    default:
                                    case 0:
                                          t = BarLineType::NORMAL;
                                          break;
                                    case 1:
                                          t = BarLineType::DOUBLE;
                                          break;
                                    case 2:
                                          t = BarLineType::START_REPEAT;
                                          break;
                                    case 3:
                                          t = BarLineType::END_REPEAT;
                                          break;
                                    case 4:
                                          t = BarLineType::BROKEN;
                                          break;
                                    case 5:
                                          t = BarLineType::END;
                                          break;
                                    case 6:
                                          // TODO t = BarLineType::END_START_REPEAT;
                                          break;
                                    }
                              barLine->setBarLineType(t);
                              }
                        else if (!barLine->Element::readProperties(e))
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
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && e.tick() == m->tick())
                        st = SegmentType::StartRepeatBarLine;
                  else if (e.tick() == m->tick() && segment == 0)
                        st = SegmentType::BeginBarLine;
                  else
                        st = SegmentType::EndBarLine;
                  segment = m->getSegment(st, e.tick());
                  segment->add(barLine);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(m->score());
                  chord->setTrack(e.track());
                  readChord(chord, e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  if (chord->noteType() != NoteType::NORMAL) {
                        graceNotes.push_back(chord);
                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TremoloType::R8) {
                              // old style tremolo found
                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case TremoloType::OLD_R8:  st = TremoloType::R8;  break;
                                    case TremoloType::OLD_R16: st = TremoloType::R16; break;
                                    case TremoloType::OLD_R32: st = TremoloType::R32; break;
                                    case TremoloType::OLD_C8:  st = TremoloType::C8;  break;
                                    case TremoloType::OLD_C16: st = TremoloType::C16; break;
                                    case TremoloType::OLD_C32: st = TremoloType::C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              }
                        }
                  else {
                        segment->add(chord);
                        Q_ASSERT(segment->segmentType() == SegmentType::ChordRest);

                        for (int i = 0; i < graceNotes.size(); ++i) {
                              Chord* gc = graceNotes[i];
                              gc->setGraceIndex(i);
                              chord->add(gc);
                              }
                        graceNotes.clear();
                        int crticks = chord->actualTicks();

                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TremoloType::R8) {
                              // old style tremolo found

                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case TremoloType::OLD_R8:  st = TremoloType::R8;  break;
                                    case TremoloType::OLD_R16: st = TremoloType::R16; break;
                                    case TremoloType::OLD_R32: st = TremoloType::R32; break;
                                    case TremoloType::OLD_C8:  st = TremoloType::C8;  break;
                                    case TremoloType::OLD_C16: st = TremoloType::C16; break;
                                    case TremoloType::OLD_C32: st = TremoloType::C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              if (tremolo->twoNotes()) {
                                    int track = chord->track();
                                    Segment* ss = 0;
                                    for (Segment* ps = m->first(SegmentType::ChordRest); ps; ps = ps->next(SegmentType::ChordRest)) {
                                          if (ps->tick() >= e.tick())
                                                break;
                                          if (ps->element(track))
                                                ss = ps;
                                          }
                                    Chord* pch = 0;       // previous chord
                                    if (ss) {
                                          ChordRest* cr = toChordRest(ss->element(track));
                                          if (cr && cr->type() == ElementType::CHORD)
                                                pch = toChord(cr);
                                          }
                                    if (pch) {
                                          tremolo->setParent(pch);
                                          pch->setTremolo(tremolo);
                                          chord->setTremolo(0);
                                          // force duration to half
                                          Fraction pts(timeStretch * pch->globalDuration());
                                          int pcrticks = pts.ticks();
                                          pch->setDuration(Fraction::fromTicks(pcrticks / 2));
                                          chord->setDuration(Fraction::fromTicks(crticks / 2));
                                          }
                                    else {
                                          qDebug("tremolo: first note not found");
                                          }
                                    crticks /= 2;
                                    }
                              else {
                                    tremolo->setParent(chord);
                                    }
                              }
                        lastTick = e.tick();
                        e.incTick(crticks);
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(m->score());
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(m->timesig()/timeStretch);
                  rest->setTrack(e.track());
                  readRest(rest, e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(rest);

                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(m->timesig()/timeStretch);

                  lastTick = e.tick();
                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(m->score());
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
                  Slur *sl = new Slur(m->score());
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
                  m->score()->addSpanner(sl);
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  Spanner* sp = toSpanner(Element::name2Element(tag, m->score()));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  // ?? sp->setAnchor(Spanner::Anchor::SEGMENT);
                  sp->read(e);
                  m->score()->addSpanner(sp);
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
                  RepeatMeasure* rm = new RepeatMeasure(m->score());
                  rm->setTrack(e.track());
                  rm->read(e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(rm);
                  if (rm->actualDuration().isZero()) { // might happen with 1.3 scores
                        rm->setDuration(m->len());
                        }
                  lastTick = e.tick();
                  e.incTick(m->ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(m->score());
                  clef->setTrack(e.track());
                  readClef(clef, e);
                  if (m->score()->mscVersion() < 113)
                        clef->setUserOff(QPointF());
                  clef->setGenerated(false);
                  // MS3 doesn't support wrong clef for staff type: Default to G
                  bool isDrumStaff = staff->isDrumStaff(e.tick());
                  if (clef->clefType() == ClefType::TAB
                      || (clef->clefType() == ClefType::PERC && !isDrumStaff)
                      || (clef->clefType() != ClefType::PERC && isDrumStaff)) {
                        clef->setClefType(ClefType::G);
                        staff->clefList().erase(e.tick());
                        staff->clefList().insert(std::pair<int,ClefType>(e.tick(), ClefType::G));
                        }

                  // there may be more than one clef segment for same tick position
                  // the first clef may be missing and is added later in layout

                  bool header;
                  if (e.tick() != m->tick())
                        header = false;
                  else if (!segment)
                        header = true;
                  else {
                        header = true;
                        for (Segment* s = m->segments().first(); s && !s->rtick(); s = s->next()) {
                              if (s->isKeySigType() || s->isTimeSigType()) {
                                    // hack: there may be other segment types which should
                                    // generate a clef at current position
                                    header = false;
                                    break;
                                    }
                              }
                        }
                  segment = m->getSegment(header ? SegmentType::HeaderClef : SegmentType::Clef, e.tick());
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(m->score());
                  ts->setTrack(e.track());
                  ts->read(e);
                  // if time sig not at begining of measure => courtesy time sig
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
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(m->score());
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
            else if (tag == "Lyrics") {
                  Element* element = Element::name2Element(tag, m->score());
                  element->setTrack(e.track());
                  element->read(e);
                  segment       = m->getSegment(SegmentType::ChordRest, e.tick());
                  ChordRest* cr = toChordRest(segment->element(element->track()));
                  if (!cr)
                        cr = toChordRest(segment->element(e.track())); // in case lyric itself has bad track info
                  if (!cr)
                        qDebug("Internal error: no chord/rest for lyrics");
                  else
                        cr->add(element);
                  }
            else if (tag == "Text") {
                  StaffText* t = new StaffText(m->score());
                  t->setTrack(e.track());
                  readStaffText(t, e);
                  if (t->empty()) {
                        qDebug("reading empty text: deleted");
                        delete t;
                        }
                  else {
                        segment = m->getSegment(SegmentType::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }
            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(m->score());
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  dyn->setDynamicType(dyn->xmlText());
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Tempo") {
                  TempoText* t = new TempoText(m->score());
                  t->setTrack(e.track());
                  readTempoText(t, e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(t);
                  }
            else if (tag == "StaffText") {
                  StaffText* t = new StaffText(m->score());
                  t->setTrack(e.track());
                  readStaffText(t, e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(t);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "StaffText"
               || tag == "RehearsalMark"
               || tag == "InstrumentChange"
               || tag == "StaffState"
               ) {
                  Element* el = Element::name2Element(tag, m->score());
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  if (el->type() == ElementType::SYMBOL)
                        el->setParent(m);    // this will get reset when adding to segment
                  el->setTrack(e.track());
                  el->read(e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(el);
                  }
            else if (tag == "Marker" || tag == "Jump") {
                  Element* el = Element::name2Element(tag, m->score());
                  el->setTrack(e.track());
                  el->read(e);

                  if (el->isMarker()) {
                        Marker* m = toMarker(el);
                        if (m->markerType() == Marker::Type::SEGNO || m->markerType() == Marker::Type::CODA  ||
                            m->markerType() == Marker::Type::VARCODA || m->markerType() == Marker::Type::CODETTA) {
                              // force the marker type for correct display
                              m->setXmlText("");
                              m->setMarkerType(m->markerType());
                              m->setSubStyle(SubStyle::REPEAT_LEFT);
                              }
                        }
                  m->add(el);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Element* el = Element::name2Element(tag, m->score());
                        el->setTrack(e.track());
                        el->read(e);
                        segment = m->getSegment(SegmentType::ChordRest, e.tick());
                        segment->add(el);
                        }
                  }
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
                  BarLine* barLine = new BarLine(m->score());
                  barLine->setTrack(e.track());
                  barLine->setBarLineType(val);
                  segment = m->getSegment(SegmentType::BeginBarLine, m->tick());
                  segment->add(barLine);
                  }
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(m->score());
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
                        Spacer* spacer = new Spacer(m->score());
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->vspacerDown(staffIdx)->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (!m->vspacerUp(staffIdx)) {
                        Spacer* spacer = new Spacer(m->score());
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
                  Beam* beam = new Beam(m->score());
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  e.addBeam(beam);
                  }
            else if (tag == "Segment") {
                  segment->read(e);
#if 0
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "off1") {
                              qreal o = e.readDouble();
                              qDebug("TODO: off1");
                              }
                        else
                              e.unknown();
                        }
#endif
                  }
            else if (tag == "MeasureNumber") {
                  Text* noText = new Text(SubStyle::MEASURE_NUMBER, m->score());
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
                  // noText->setFlag(ElementFlag::MOVABLE, false); ??
                  noText->setTrack(e.track());
                  noText->setParent(m);
                  m->setNoText(noText->staffIdx(), noText);
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
      // For nested tuplets created with MuseScore 1.3 tuplet dialog (i.e. "Other..." dialog),
      // the parent tuplet was not set. Try to infere if the tuplet was actually a nested tuplet
      for (Tuplet* tuplet : e.tuplets()) {
            int tupletTick = tuplet->tick();
            int tupletDuration = tuplet->actualTicks() - 1;
            std::vector<DurationElement*> tElements = tuplet->elements();
            for (Tuplet* tuplet2 : e.tuplets()) {
                  if ((tuplet2->tuplet()) || (tuplet2->voice() != tuplet->voice())) // already a nested tuplet or in a different voice
                        continue;
                  int possibleDuration = tuplet2->duration().ticks() * tuplet->ratio().denominator() / tuplet->ratio().numerator() - 1;
                  if ((tuplet2 != tuplet) && (tuplet2->tick() >= tupletTick) && (tuplet2->tick() < tupletTick + tupletDuration) && (tuplet2->tick() + possibleDuration < tupletTick + tupletDuration)) {
                        bool found = false;
                        for (DurationElement* de : tElements) {
                              if (de == tuplet2)
                                    found = true;
                              }
                        if (!found) {
                              qDebug("Adding tuplet %p as nested tuplet to tuplet %p",tuplet2,tuplet);
                              tuplet2->setTuplet(tuplet);
                              tuplet->add(tuplet2);
                              }
                        }
                  }
            }
      e.checkTuplets();
      }
//---------------------------------------------------------
//   readMeasureBase
//---------------------------------------------------------

static void readMeasureBase(MeasureBase* mb, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Text") {
                  Text* t = new Text(mb->score());
                  readText(e, t, t);
                  }
            else
                  e.unknown();
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

      Measure* measure = score->firstMeasure();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "Measure") {
                  if (staff == 0) {
                        measure = new Measure(score);
                        measure->setTick(e.tick());
                        const SigEvent& ev = score->sigmap()->timesig(measure->tick());
                        measure->setLen(ev.timesig());
                        measure->setTimesig(ev.nominal());

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
                  else {
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
                  }
            else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                  MeasureBase* mb = toMeasureBase(Element::name2Element(tag, score));
                  readMeasureBase(mb, e);
                  mb->setTick(e.tick());
                  score->measures()->add(mb);
                  }
            else if (tag == "tick")
                  e.initTick(score->fileDivision(e.readInt()));
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

static void readStaff(Staff* staff, XmlReader& e)
      {
      Score* _score = staff->score();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "lines") {
                  int lines = e.readInt();
                  staff->setLines(0, lines);
                  if (lines != 5) {
                        staff->setBarLineFrom(lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                        staff->setBarLineTo(lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (lines - 1) * 2);
                        }
                  }
            else if (tag == "small")
                  staff->setSmall(0, e.readInt());
            else if (tag == "invisible")
                  staff->setInvisible(e.readInt());
            else if (tag == "slashStyle")
                  e.skipCurrentElement();
            else if (tag == "cleflist") {
                  // QList<std::pair<int, ClefType>>& cl = e.clefs(idx());
                  staff->clefList().clear();
                  while (e.readNextStartElement()) {
                        if (e.name() == "clef") {
                              int tick    = e.intAttribute("tick", 0);
                              ClefType ct = readClefType(e.attribute("idx", "0"));
                              staff->clefList().insert(std::pair<int,ClefType>(_score->fileDivision(tick), ct));
                              e.readNext();
                              }
                        else
                              e.unknown();
                        }
                  if (staff->clefList().empty())
                        staff->clefList().insert(std::pair<int,ClefType>(0, ClefType::G));
                  }
            else if (tag == "keylist")
                  staff->keyList()->read(e, _score);
            else if (tag == "bracket") {
                  BracketItem* b = new BracketItem(_score);
                  b->setBracketType( BracketType(e.intAttribute("type", -1)));
                  b->setBracketSpan(e.intAttribute("span", 0));
                  staff->brackets().push_back(b);
                  e.readNext();
                  }
            else if (tag == "barLineSpan")
                  staff->setBarLineSpan(e.readInt());
            else
                  e.unknown();
            }
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
            if (tag == "chorus")
                  chorus = e.readInt();
            else if (tag == "reverb")
                  reverb = e.readInt();
            else if (tag == "midiProgram")
                  program = e.readInt();
            else if (tag == "volume")
                  volume = e.readInt();
            else if (tag == "pan")
                  pan = e.readInt();
            else if (tag == "midiChannel")
                  e.skipCurrentElement();
            else if (tag == "Drum") {
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
//   readPart
//---------------------------------------------------------

static void readPart(Part* part, XmlReader& e)
      {
      Score* _score = part->score();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score);
                  staff->setPart(part);
                  _score->staves().push_back(staff);
                  part->staves()->push_back(staff);
                  readStaff(staff, e);
                  }
            else if (tag == "Instrument") {
                  Instrument* i = part->instrument();
                  readInstrument(i, part, e);
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
                  Staff*   st = part->staff(0);
                  if (d && st && st->lines(0) != 5) {
                        int n = 0;
                        if (st->lines(0) == 1)
                              n = 4;
                        for (int  i = 0; i < DRUM_INSTRUMENTS; ++i)
                              d->drum(i).line -= n;
                        }
                  }
            else if (tag == "name") {
                  Text* t = new Text(_score);
                  readText(e, t, t);
                  part->instrument()->setLongName(t->xmlText());
                  delete t;
                  }
            else if (tag == "shortName") {
                  Text* t = new Text(_score);
                  t->read(e);
                  part->instrument()->setShortName(t->xmlText());
                  delete t;
                  }
            else if (tag == "trackName")
                  part->setPartName(e.readElementText());
            else if (tag == "show")
                  part->setShow(e.readInt());
            else
                  e.unknown();
            }
      if (part->partName().isEmpty())
            part->setPartName(part->instrument()->trackName());

      if (part->instrument()->useDrumset()) {
            for (Staff* staff : *part->staves()) {
                  int lines = staff->lines(0);
                  int bf    = staff->barLineFrom();
                  int bt    = staff->barLineTo();
                  staff->setStaffType(0, StaffType::getDefaultPreset(StaffGroup::PERCUSSION));

                  // this allows 2/3-line percussion staves to keep the double spacing they had in 1.3

                  if (lines == 2 || lines == 3)
                        staff->staffType(0)->setLineDistance(Spatium(2.0));

                  staff->setLines(0, lines);       // this also sets stepOffset
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
      part->instrument()->setArticulation(articulations);
      }

//---------------------------------------------------------
//   readPageFormat
//---------------------------------------------------------

static void readPageFormat(PageFormat* pf, XmlReader& e)
      {
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      bool landscape         = false;
      QString type;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "landscape")
                  landscape = e.readInt();
            else if (tag == "page-margins") {
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
                  pf->setTwosided(type == "odd" || type == "even");
                  if (type == "odd" || type == "both") {
                        pf->setOddLeftMargin(lm);
                        _oddRightMargin = rm;
                        pf->setOddTopMargin(tm);
                        pf->setOddBottomMargin(bm);
                        }
                  if (type == "even" || type == "both") {
                        pf->setEvenLeftMargin(lm);
                        _evenRightMargin = rm;
                        pf->setEvenTopMargin(tm);
                        pf->setEvenBottomMargin(bm);
                        }
                  }
            else if (tag == "page-height")
                  pf->setSize(QSizeF(pf->size().width(), e.readDouble() * 0.5 / PPI));
            else if (tag == "page-width")
                  pf->setSize(QSizeF(e.readDouble() * 0.5 / PPI, pf->size().height()));
            else if (tag == "pageFormat") {
                  const PaperSize* s = getPaperSize114(e.readElementText());
                  pf->setSize(QSizeF(s->w, s->h));
                  }
            else if (tag == "page-offset") {
                  e.readInt();
                  }
            else
                  e.unknown();
            }
      if (landscape)
            pf->setSize(pf->size().transposed());
      qreal w1 = pf->size().width() - pf->oddLeftMargin() - _oddRightMargin;
      qreal w2 = pf->size().width() - pf->evenLeftMargin() - _evenRightMargin;
      pf->setPrintableWidth(qMin(w1, w2));     // silently adjust right margins
      }

//---------------------------------------------------------
//   readStyle
//---------------------------------------------------------

static void readStyle(MStyle* style, XmlReader& e)
      {
      QString oldChordDescriptionFile = style->value(StyleIdx::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();

            if (tag == "lyricsDistance")        // was renamed
                  tag = "lyricsPosBelow";

            if (tag == "TextStyle") {
//                  TextStyle s;
//TODO                  s.read(e);
//                  style->setTextStyle(s);
                  e.skipCurrentElement();
                  }
            else if (tag == "Spatium")
                  style->set(StyleIdx::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout") {
                  PageFormat pf;
                  initPageFormat(style, &pf);
                  pf.read(e);
                  setPageFormat(style, pf);
                  }
            else if (tag == "displayInConcertPitch")
                  style->set(StyleIdx::concertPitch, QVariant(bool(e.readInt())));
            else if (tag == "ChordList") {
                  style->chordList()->clear();
                  style->chordList()->read(e);
                  for (ChordFont f : style->chordList()->fonts) {
                        if (f.family == "MuseJazz") {
                              f.family = "MuseJazz Text";
                              }
                        }
                  style->setCustomChordList(true);
                  chordListTag = true;
                  }
            else if (tag == "pageFillLimit" || tag == "genTimesig" || tag == "FixMeasureNumbers" || tag == "FixMeasureWidth")   // obsolete
                  e.skipCurrentElement();
            else if (tag == "systemDistance")  // obsolete
                  style->set(StyleIdx::minSystemDistance, QVariant(e.readDouble()));
            else {
                  if (tag == "stemDir") {
                        int voice = e.attribute("voice", "1").toInt() - 1;
                        switch(voice) {
                              case 0: tag = "StemDir1"; break;
                              case 1: tag = "StemDir2"; break;
                              case 2: tag = "StemDir3"; break;
                              case 3: tag = "StemDir4"; break;
                              }
                        }
                  // for compatibility:
                  if (tag == "oddHeader" || tag == "evenHeader" || tag == "oddFooter" || tag == "evenFooter")
                        tag += "C";
#if 0 // TODO-ws
                  int idx2;
                  for (idx2 = 0; idx2 < int(ArticulationType::ARTICULATIONS); ++idx2) {
                        ArticulationInfo& ai =  Articulation::articulationList[idx2];
                        // deal with obsolete tags from 1.14 format
                        if (tag == "SforzatoaccentAnchor")
                              tag = "SforzatoAnchor";
                        if (tag == "SnappizzicatorAnchor")
                              tag = "SnappizzicatoAnchor";
                        else if (tag == "EspressivoAnchor")
                              continue;
                        if (QString::compare(tag, QString(ai.name).append("Anchor"),  Qt::CaseInsensitive) == 0
                              || QString::compare(tag, QString("U").append(ai.name).append("Anchor"), Qt::CaseInsensitive) == 0
                              || QString::compare(tag, QString("D").append(ai.name).append("Anchor"), Qt::CaseInsensitive) == 0
                              ) {
                              StyleIdx si = MStyle::styleIdx(QString(ai.name).append("Anchor"));
                              if (si != StyleIdx::NOSTYLE) {
                                    QString val(e.readElementText());
                                    style->set(si, val.toInt());
                                    break;
                                    }
                              }
                        }
                  if (idx2 < int(ArticulationType::ARTICULATIONS))
                        continue;
#endif
                  QString val(e.readElementText());
//TODO                  style->convertToUnit(tag, val);
                  }
            }

      // if we just specified a new chord description file
      // and didn't encounter a ChordList tag
      // then load the chord description file

      QString newChordDescriptionFile = style->value(StyleIdx::chordDescriptionFile).toString();
      if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
            if (!newChordDescriptionFile.startsWith("chords_") && style->value(StyleIdx::chordStyle).toString() == "std") {
                  // should not normally happen,
                  // but treat as "old" (114) score just in case
                  style->set(StyleIdx::chordStyle, QVariant(QString("custom")));
                  style->set(StyleIdx::chordsXmlFile, QVariant(true));
                  qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
                  }
            if (style->value(StyleIdx::chordStyle).toString() == "custom")
                  style->setCustomChordList(true);
            else
                  style->setCustomChordList(false);
            style->chordList()->unload();
            }

      // make sure we have a chordlist
      if (!style->chordList()->loaded() && !chordListTag) {
            if (style->value(StyleIdx::chordsXmlFile).toBool())
                  style->chordList()->read("chords.xml");
            style->chordList()->read(newChordDescriptionFile);
            }
#if 0 // TODO
      //
      //  Compatibility with old scores/styles:
      //  translate old frameWidthMM and paddingWidthMM
      //  into spatium units
      //
      int n = style->textStyles().size();
      qreal _spatium = style->value(StyleIdx::spatium).toDouble();
      qreal spMM = _spatium / DPMM;
      for (int i = 0; i < n; ++i) {
            TextStyle* s = &style->textStyle(StyledPropertyListIdx(i));
            if (s->frameWidthMM() != 0.0)
                  s->setFrameWidth(Spatium(s->frameWidthMM() / spMM));
            if (s->paddingWidthMM() != 0.0)
                  s->setPaddingWidth(Spatium(s->paddingWidthMM() / spMM));
            }
#endif
      }

//---------------------------------------------------------
//   read114
//    import old version <= 1.3 files
//---------------------------------------------------------

Score::FileError MasterScore::read114(XmlReader& e)
      {
      qDebug("==");
#if 0
      for (unsigned int i = 0; i < sizeof(style114)/sizeof(*style114); ++i)
            style().set(style114[i].idx, style114[i].val);

      // old text style defaults
      TextStyle ts = style().textStyle("Chord Symbol");
      ts.setYoff(-4.0);
      style().setTextStyle(ts);
      ts = style().textStyle("Rehearsal Mark");
      ts.setSquare(false);
      ts.setFrameRound(20);
      style().setTextStyle(ts);
      ts = style().textStyle("Dynamics");
      ts.setItalic(false);
      style().setTextStyle(ts);
#endif
      TempoMap tm;
      while (e.readNextStartElement()) {
            e.setTrack(-1);
            const QStringRef& tag(e.name());
            if (tag == "Staff")
                  readStaffContent(this, e);
            else if (tag == "KeySig") {               // not supported
                  KeySig* ks = new KeySig(this);
                  ks->read(e);
                  delete ks;
                  }
            else if (tag == "siglist")
                  _sigmap->read(e, _fileDivision);
            else if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readInt());
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
                              int tick   = e.attribute("tick").toInt();
                              double tmp = e.readElementText().toDouble();
                              tick       = (tick * MScore::division + _fileDivision/2) / _fileDivision;
                              auto pos   = tm.find(tick);
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
                  setPlayMode(PlayMode(e.readInt()));
            else if (tag == "SyntiSettings")
                  _synthesizerState.read(e);
            else if (tag == "Spatium")
                  setSpatium (e.readDouble() * DPMM);
            else if (tag == "Division")
                  _fileDivision = e.readInt();
            else if (tag == "showInvisible")
                  setShowInvisible(e.readInt());
            else if (tag == "showFrames")
                  setShowFrames(e.readInt());
            else if (tag == "showMargins")
                  setShowPageborders(e.readInt());
            else if (tag == "Style") {
                  qreal sp = spatium();
                  readStyle(&style(), e);
                  //style()->load(e);
                  // adjust this now so chords render properly on read
                  // other style adjustments can wait until reading is finished
                  if (styleB(StyleIdx::useGermanNoteNames))
                        style().set(StyleIdx::useStandardNoteNames, false);
                  if (_layoutMode == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        setSpatium(sp);
                        }
                  }
            else if (tag == "TextStyle") {
                  e.skipCurrentElement();
#if 0 // TODO
                  TextStyle s;
                  s.read(e);

                  qreal spMM = spatium() / DPMM;
                  if (s.frameWidthMM() != 0.0)
                        s.setFrameWidth(Spatium(s.frameWidthMM() / spMM));
                  if (s.paddingWidthMM() != 0.0)
                        s.setPaddingWidth(Spatium(s.paddingWidthMM() / spMM));

                  // convert 1.2 text styles
                  s.setName(convertOldTextStyleNames(s.name()));
                  if (s.family() == "MuseJazz")
                        s.setFamily("MuseJazz Text");

                  if (s.name() == "Lyrics Odd Lines" || s.name() == "Lyrics Even Lines")
                        s.setAlign(Align(int(s.align()) & int(~Align::VMASK)) | Align::BASELINE);

                  style().setTextStyle(s);
#endif
                  }
            else if (tag == "page-layout") {
                  PageFormat pf;
                  readPageFormat(&pf, e);
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
                  readPart(part, e);
                  parts().push_back(part);
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
                  Spanner* s = toSpanner(Element::name2Element(tag, this));
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

      if (e.error() != QXmlStreamReader::NoError)
            return FileError::FILE_BAD_FORMAT;

      for (Staff* s : staves()) {
            int idx   = s->idx();
            int track = idx * VOICES;

            // check barLineSpan
            if (s->barLineSpan() > (nstaves() - idx)) {
                  qDebug("read114: invalid barline span %d (max %d)",
                     s->barLineSpan(), nstaves() - idx);
                  s->setBarLineSpan(nstaves() - idx);
                  }
            for (auto i : s->clefList()) {
                  int tick = i.first;
                  ClefType clefId = i.second._concertClef;
                  Measure* m = tick2measure(tick);
                  if (!m)
                        continue;
                  SegmentType st = SegmentType::Clef;
                  if (tick == m->tick()) {
                       if (m->prevMeasure())
                              m = m->prevMeasure();
                        else
                              st = SegmentType::HeaderClef;
                        }
                  Segment* seg = m->getSegment(st, tick);
                  if (seg->element(track))
                        seg->element(track)->setGenerated(false);
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
                  Segment* seg = m->getSegment(SegmentType::KeySig, tick);
                  if (seg->element(track))
                        toKeySig(seg->element(track))->setGenerated(false);
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
            if (!s->isSlur()) {
                  if (s->isVolta()) {
                        Volta* volta = toVolta(s);
                        volta->setAnchor(Spanner::Anchor::MEASURE);
                        }
                  }

            if (s->isOttava() || s->isPedal() || s->isTrill() || s->isTextLine()) {
                  qreal yo = 0;
                  if (s->isOttava()) {
                      // fix ottava position
                      yo = styleP(StyleIdx::ottavaPosAbove);
                      if (s->placeBelow())
                            yo = -yo + s->staff()->height();
                      }
                  else if (s->isPedal()) {
                        yo = styleP(StyleIdx::pedalPosBelow);
                        }
                  else if (s->isTrill()) {
                        yo = styleP(StyleIdx::trillPosAbove);
                        }
                  else if (s->isTextLine()) {
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
                        if (s->segmentType() != SegmentType::ChordRest)
                              continue;
                        ChordRest* cr = toChordRest(s->element(track));
                        if (cr) {
#if 0 // TODO
                              if (cr->isRest()) {
                                    Rest* r = toRest(cr);
                                    if (!r->userOff().isNull()) {
                                          int lineOffset = r->computeLineOffset();
                                          qreal lineDist = r->staff() ? r->staff()->staffType(cr->tick())->lineDistance().val() : 1.0;
                                          r->rUserYoffset() -= (lineOffset * .5 * lineDist * r->spatium());
                                          }
                                    }
#endif
                              if (!first) {
                                    switch (cr->beamMode()) {
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
                                                if (cr->isChord())
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
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->isVBox()) {
                  VBox* b  = toVBox(mb);
                  qreal y = styleP(StyleIdx::staffUpperBorder);
                  b->setBottomGap(y);
                  }
            }

      _fileDivision = MScore::division;

      //
      //    sanity check for barLineSpan and update ottavas
      //
      for (Staff* staff : staves()) {
            int barLineSpan = staff->barLineSpan();
            int idx = staff->idx();
            int n = nstaves();
            if (idx + barLineSpan > n) {
                  qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
                  staff->setBarLineSpan(n - idx);
                  }
            staff->updateOttava();
            }

      // adjust some styles
      Spatium lmbd = styleS(StyleIdx::lyricsMinBottomDistance);
      style().set(StyleIdx::lyricsMinBottomDistance, Spatium(lmbd.val() + 4.0));
      if (styleB(StyleIdx::hideEmptyStaves))        // http://musescore.org/en/node/16228
            style().set(StyleIdx::dontHideStavesInFirstSystem, false);
      if (styleB(StyleIdx::showPageNumberOne)) {    // http://musescore.org/en/node/21207
            style().set(StyleIdx::evenFooterL, QString("$P"));
            style().set(StyleIdx::oddFooterR, QString("$P"));
            }
      if (styleI(StyleIdx::minEmptyMeasures) == 0)
            style().set(StyleIdx::minEmptyMeasures, 1);
      style().set(StyleIdx::frameSystemDistance, styleS(StyleIdx::frameSystemDistance) + Spatium(6.0));
      // hack: net overall effect of layout changes has been for things to take slightly more room
      qreal adjustedSpacing = qMax(styleD(StyleIdx::measureSpacing) * 0.95, 1.0);
      style().set(StyleIdx::measureSpacing, adjustedSpacing);

      _showOmr = false;

      // add invisible tempo text if necessary
      // some 1.3 scores have tempolist but no tempo text
      fixTicks();
      for (auto i : tm) {
            int tick    = i.first;
            qreal tempo = i.second.tempo;
            if (tempomap()->tempo(tick) != tempo) {
                  TempoText* tt = new TempoText(this);
                  tt->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(qRound(tempo*60)));
                  tt->setTempo(tempo);
                  tt->setTrack(0);
                  tt->setVisible(false);
                  Measure* m = tick2measure(tick);
                  if (m) {
                        Segment* seg = m->getSegment(SegmentType::ChordRest, tick);
                        seg->add(tt);
                        setTempo(tick, tempo);
                        }
                  else
                        delete tt;
                  }
            }

      // create excerpts

      for (Excerpt* excerpt : _excerpts) {
            if (excerpt->parts().isEmpty()) {         // ignore empty parts
                  _excerpts.removeOne(excerpt);
                  continue;
                  }
            if (!excerpt->parts().isEmpty()) {
                  Score* nscore = new Score(this);
                  excerpt->setPartScore(nscore);
                  nscore->style().set(StyleIdx::createMultiMeasureRests, true);
                  Excerpt::createExcerpt(excerpt);
                  }
            }

      // volta offsets in older scores are hardcoded to be relative to a voltaY of -2.0sp
      // we'll force this and live with it for the score
      // but we wait until now to do it so parts don't have this issue
      if (styleV(StyleIdx::voltaY) == MScore::baseStyle().value(StyleIdx::voltaY))
            style().set(StyleIdx::voltaY, -2.0f);

      fixTicks();
      rebuildMidiMapping();
      updateChannel();

      // treat reading a 1.14 file as import
      // on save warn if old file will be overwritten
      setCreated(true);
      // don't autosave (as long as there's no change to the score)
      setAutosaveDirty(false);

      return FileError::FILE_NO_ERROR;
      }

}

