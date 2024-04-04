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
#include "arpeggio.h"
#include "audio.h"
#include "sig.h"
#include "barline.h"
#include "measure.h"
#include "ambitus.h"
#include "bend.h"
#include "chordline.h"
#include "hook.h"
#include "tuplet.h"
#include "systemdivider.h"
#include "spacer.h"
#include "keysig.h"
#include "stafftext.h"
#include "dynamic.h"
#include "drumset.h"
#include "timesig.h"
#include "slur.h"
#include "tie.h"
#include "chord.h"
#include "rest.h"
#include "breath.h"
#include "repeat.h"
#include "utils.h"
#include "read206.h"
#include "excerpt.h"
#include "articulation.h"
#include "volta.h"
#include "pedal.h"
#include "hairpin.h"
#include "glissando.h"
#include "ottava.h"
#include "trill.h"
#include "rehearsalmark.h"
#include "box.h"
#include "textframe.h"
#include "textline.h"
#include "fingering.h"
#include "fermata.h"
#include "image.h"
#include "stem.h"
#include "stemslash.h"
#include "undo.h"
#include "lyrics.h"
#include "tempotext.h"
#include "measurenumber.h"
#include "marker.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif


namespace Ms {

static void readText206(XmlReader& e, TextBase* t, Element* be);

//---------------------------------------------------------
//   excessTextStyles206
//    The first map has the name of the style as the string
//    The second map has the mapping of each Sid that the style identifies
//    to the default value for that sid.
//---------------------------------------------------------

static std::map<QString, std::map<Sid, QVariant>> excessTextStyles206;

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
//   readTextStyle206
//---------------------------------------------------------

void readTextStyle206(MStyle* style, XmlReader& e, std::map<QString, std::map<Sid, QVariant>>& excessStyles)
      {
      QString family = "FreeSerif";
      double size = 10;
      bool sizeIsSpatiumDependent = false;
      FontStyle fontStyle = FontStyle::Normal;
      Align align = Align::LEFT;
      QPointF offset;
      OffsetType offsetType = OffsetType::SPATIUM;

      FrameType frameType = FrameType::NO_FRAME;
      Spatium paddingWidth(0.0);
      Spatium frameWidth(0.0);
      QColor foregroundColor = QColor(0, 0, 0, 255);
      QColor backgroundColor = QColor(255, 255, 255, 0);

      Placement placement = Placement::ABOVE;
      bool placementValid = false;

      QString name = e.attribute("name");
      QColor frameColor = QColor(0, 0, 0, 255);

      bool systemFlag = false;
      qreal lineWidth = -1.0;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "name")
                  name = e.readElementText();
            else if (tag == "family")
                  family = e.readElementText();
            else if (tag == "size")
                  size = e.readDouble();
            else if (tag == "bold") {
                  if (e.readInt())
                        fontStyle = fontStyle + FontStyle::Bold;
                  }
            else if (tag == "italic") {
                  if (e.readInt())
                        fontStyle = fontStyle + FontStyle::Italic;
                  }
            else if (tag == "underline") {
                  if (e.readInt())
                        fontStyle = fontStyle + FontStyle::Underline;
                  }
#if 0 // should not happen, but won't harm either
            else if (tag == "strike") {
                  if (e.readInt())
                        fontStyle = fontStyle + FontStyle::Strike;
                  }
#endif
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
                  }
            else if (tag == "yoffset") {
                  qreal yo = e.readDouble();
                  if (offsetType == OffsetType::ABS)
                        yo /= INCH;
                  offset.setY(yo);
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
                  frameType = FrameType::SQUARE;
                  /*frameWidthMM =*/ e.readDouble();
                  }
            else if (tag == "frameWidthS") {
                  frameType = FrameType::SQUARE;
                  frameWidth = Spatium(e.readDouble());
                  }
            else if (tag == "frame")
                  frameType = e.readInt() ? FrameType::SQUARE : FrameType::NO_FRAME;
            else if (tag == "paddingWidth")          // obsolete
                  /*paddingWidthMM =*/ e.readDouble();
            else if (tag == "paddingWidthS")
                  paddingWidth = Spatium(e.readDouble());
            else if (tag == "frameRound")
                  e.readInt();
            else if (tag == "frameColor")
                  frameColor = e.readColor();
            else if (tag == "foregroundColor")
                  foregroundColor = e.readColor();
            else if (tag == "backgroundColor")
                  backgroundColor = e.readColor();
            else if (tag == "circle")
                  frameType = e.readInt() ? FrameType::CIRCLE : FrameType::NO_FRAME;
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
            Tid ss;
            } styleTable[] = {
            { "",                        Tid::DEFAULT },
            { "Title",                   Tid::TITLE },
            { "Subtitle",                Tid::SUBTITLE },
            { "Composer",                Tid::COMPOSER },
            { "Lyricist",                Tid::POET },
            { "Lyrics Odd Lines",        Tid::LYRICS_ODD },
            { "Lyrics Even Lines",       Tid::LYRICS_EVEN },
            { "Fingering",               Tid::FINGERING },
            { "LH Guitar Fingering",     Tid::LH_GUITAR_FINGERING },
            { "RH Guitar Fingering",     Tid::RH_GUITAR_FINGERING },
            { "String Number",           Tid::STRING_NUMBER },
            { "Instrument Name (Long)",  Tid::INSTRUMENT_LONG },
            { "Instrument Name (Short)", Tid::INSTRUMENT_SHORT },
            { "Instrument Name (Part)",  Tid::INSTRUMENT_EXCERPT },
            { "Dynamics",                Tid::DYNAMICS },
            { "Technique",               Tid::EXPRESSION },
            { "Tempo",                   Tid::TEMPO },
            { "Metronome",               Tid::METRONOME },
            { "Measure Number",          Tid::MEASURE_NUMBER },
            { "Translator",              Tid::TRANSLATOR },
            { "Tuplet",                  Tid::TUPLET },
            { "System",                  Tid::SYSTEM },
            { "Staff",                   Tid::STAFF },
            { "Chord Symbol",            Tid::HARMONY_A },
            { "Rehearsal Mark",          Tid::REHEARSAL_MARK },
            { "Repeat Text Left",        Tid::REPEAT_LEFT },
            { "Repeat Text Right",       Tid::REPEAT_RIGHT },
            { "Frame",                   Tid::FRAME },
            { "Text Line",               Tid::TEXTLINE },
            { "Glissando",               Tid::GLISSANDO },
            { "Ottava",                  Tid::OTTAVA },
            { "Pedal",                   Tid::PEDAL },
            { "Hairpin",                 Tid::HAIRPIN },
            { "Bend",                    Tid::BEND },
            { "Header",                  Tid::HEADER },
            { "Footer",                  Tid::FOOTER },
            { "Instrument Change",       Tid::INSTRUMENT_CHANGE },
            { "Repeat Text",             Tid::IGNORED_STYLES, },     // Repeat Text style no longer exists
            { "Figured Bass",            Tid::IGNORED_STYLES, },     // F.B. data are in style properties
            { "Volta",                   Tid::VOLTA },
            };
      Tid ss = Tid::TEXT_STYLES;
      for (const auto& i : styleTable) {
            if (name == i.name) {
                  ss = i.ss;
                  break;
                  }
            }

      if (ss == Tid::IGNORED_STYLES)
            return;

      bool isExcessStyle = false;
      if (ss == Tid::TEXT_STYLES) {
            ss = e.addUserTextStyle(name);
            if (ss == Tid::TEXT_STYLES) {
                  qDebug("unhandled substyle <%s>", qPrintable(name));
                  isExcessStyle = true;
                  }
            else {
                  int idx = int(ss) - int(Tid::USER1);
                  if ((int(ss) < int(Tid::USER1)) || (int(ss) > int(Tid::USER12))) {
                        qDebug("User style index %d outside of range.", idx);
                        return;
                        }
                  Sid sid[] = { Sid::user1Name, Sid::user2Name, Sid::user3Name, Sid::user4Name, Sid::user5Name, Sid::user6Name,
                                Sid::user7Name, Sid::user8Name, Sid::user9Name, Sid::user10Name, Sid::user11Name, Sid::user12Name};
                  style->set(sid[idx], name);
                  }
            }

      std::map<Sid, QVariant> excessPairs;
      const TextStyle* ts;
      if (isExcessStyle)
            ts = textStyle("User-1");
      else
            ts = textStyle(ss);
      for (const auto& i : *ts) {
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
                  case Pid::BEGIN_FONT_STYLE:
                  case Pid::CONTINUE_FONT_STYLE:
                  case Pid::END_FONT_STYLE:
                  case Pid::FONT_STYLE:
                        value = int(fontStyle);
                        break;
                  case Pid::FRAME_TYPE:
                        value = int(frameType);
                        break;
                  case Pid::FRAME_WIDTH:
                        value = frameWidth;
                        break;
                  case Pid::FRAME_PADDING:
                        value = paddingWidth;
                        break;
                  case Pid::FRAME_FG_COLOR:
                        value = frameColor;
                        break;
                  case Pid::FRAME_BG_COLOR:
                        value = backgroundColor;
                        break;
                  case Pid::SIZE_SPATIUM_DEPENDENT:
                        value = sizeIsSpatiumDependent;
                        break;
                  case Pid::BEGIN_TEXT_ALIGN:
                  case Pid::CONTINUE_TEXT_ALIGN:
                  case Pid::END_TEXT_ALIGN:
                  case Pid::ALIGN:
                        value = QVariant::fromValue(align);
                        break;
#if 0  //TODO-offset
                  case Pid::OFFSET:
                        if (offsetValid) {
                              if (ss == Tid::TEMPO) {
                                    style->set(Sid::tempoPosAbove, Spatium(offset.y()));
                                    offset = QPointF();
                                    }
                              else if (ss == Tid::STAFF) {
                                    style->set(Sid::staffTextPosAbove, Spatium(offset.y()));
                                    offset = QPointF();
                                    }
                              else if (ss == Tid::REHEARSAL_MARK) {
                                    style->set(Sid::rehearsalMarkPosAbove, Spatium(offset.y()));
                                    offset = QPointF();
                                    }
                              value = offset;
                              }
                        break;
                  case Pid::OFFSET_TYPE:
                        value = int(offsetType);
                        break;
#endif
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
            if (value.isValid()) {
                  if (isExcessStyle)
                        excessPairs[i.sid] = value;
                  else
                        style->set(i.sid, value);
                  }
//            else
//                  qDebug("invalid style value <%s> pid<%s>", MStyle::valueName(i.sid), propertyName(i.pid));
            }

      if (isExcessStyle && excessPairs.size() > 0)
            excessStyles[name] = excessPairs;
      }

//---------------------------------------------------------
//   readAccidental206
//---------------------------------------------------------

void readAccidental206(Accidental* a, XmlReader& e)
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
      { SymId::ornamentShortTrill,        "prall",                     },
      { SymId::ornamentMordent,           "mordent",                   },
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
      int bank    = 0;
      int volume  = 100;
      int pan     = 60;
      int chorus  = 30;
      int reverb  = 30;
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

      if (i->instrumentId().isEmpty())
            i->setInstrumentId(i->recognizeInstrumentId());

      // Read single-note dynamics from template
      i->setSingleNoteDynamicsFromTemplate();

      if (i->channel().empty()) {      // for backward compatibility
            Channel* a = new Channel;
            a->setName(Channel::DEFAULT_NAME);
            a->setProgram(i->recognizeMidiProgram());
            a->setBank(bank);
            a->setVolume(volume);
            a->setPan(pan);
            a->setReverb(reverb);
            a->setChorus(chorus);
            i->appendChannel(a);
            }
      else if (i->channel(0)->program() < 0){
            i->channel(0)->setProgram(i->recognizeMidiProgram());
            }
      if (i->useDrumset()) {
            if (i->channel()[0]->bank() == 0)
                  i->channel()[0]->setBank(128);
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

void readPart206(Part* part, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument") {
                  Instrument* i = part->_instruments.instrument(/* tick */ -1);
                  readInstrument(i, part, e);
                  Drumset* ds = i->drumset();
                  Staff*   s = part->staff(0);
                  int lld = s ? qRound(s->lineDistance(Fraction(0,1))) : 1;
                  if (ds && s && lld > 1) {
                        for (int j = 0; j < DRUM_INSTRUMENTS; ++j)
                              ds->drum(j).line /= lld;
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
                  readAccidental206(a, e);
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
            else if (readNoteProperties206(note, e))
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
            Fraction tick = note->chord() ? note->chord()->tick() : Fraction(-1,1);
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
#if 0
      // TODO - adapt this code

      // check consistency of pitch, tpc1, tpc2, and transposition
      // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
      // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
      // including perhaps some we don't know about yet,
      // we will attempt to fix some problems here regardless of version

      if (staff() && !staff()->isDrumStaff(e.tick()) && !e.pasteMode() && !MScore::testMode) {
            int tpc1Pitch = (tpc2pitch(_tpc[0]) + 12) % 12;
            int tpc2Pitch = (tpc2pitch(_tpc[1]) + 12) % 12;
            int soundingPitch = _pitch % 12;
            if (tpc1Pitch != soundingPitch) {
                  qDebug("bad tpc1 - soundingPitch = %d, tpc1 = %d", soundingPitch, tpc1Pitch);
                  _pitch += tpc1Pitch - soundingPitch;
                  }
            if (staff()) {
                  Interval v = staff()->part()->instrument(e.tick())->transpose();
                  int writtenPitch = (_pitch - v.chromatic) % 12;
                  if (tpc2Pitch != writtenPitch) {
                        qDebug("bad tpc2 - writtenPitch = %d, tpc2 = %d", writtenPitch, tpc2Pitch);
                        if (concertPitch()) {
                              // assume we want to keep sounding pitch
                              // so fix written pitch (tpc only)
                              v.flip();
                              _tpc[1] = Ms::transposeTpc(_tpc[0], v, true);
                              }
                        else {
                              // assume we want to keep written pitch
                              // so fix sounding pitch (both tpc and pitch)
                              _tpc[0] = Ms::transposeTpc(_tpc[1], v, true);
                              _pitch += tpc2Pitch - writtenPitch;
                              }
                        }
                  }
            }
#endif
      }

//---------------------------------------------------------
//   adjustPlacement
//---------------------------------------------------------

static void adjustPlacement(Element* e)
      {
      if (!e || !e->staff())
            return;

      // element to use to determine placement
      // for spanners, choose first segment
      Element* ee;
      Spanner* spanner;
      if (e->isSpanner()) {
            spanner = toSpanner(e);
            if (spanner->spannerSegments().empty())
                  return;
            ee = spanner->spannerSegments().front();
            if (!ee)
                  return;
            }
      else {
            spanner = nullptr;
            ee = e;
            }

      // determine placement based on offset
      // anything below staff will be set to below
      qreal staffHeight = e->staff()->height();
      qreal threshold = staffHeight;
      qreal offsetAdjust = 0.0;
      Placement defaultPlacement = Placement(e->propertyDefault(Pid::PLACEMENT).toInt());
      Placement newPlacement;
      // most offsets will be recorded as relative to top staff line
      // exceptions are styled offsets on elements with default placement below
      qreal normalize;
      if (defaultPlacement == Placement::BELOW && ee->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED)
            normalize = staffHeight;
      else
            normalize = 0.0;
      qreal ypos = ee->offset().y() + normalize;
      if (ypos >= threshold) {
            newPlacement = Placement::BELOW;
            offsetAdjust -= staffHeight;
            }
      else {
            newPlacement = Placement::ABOVE;
            }

      // set placement
      e->setProperty(Pid::PLACEMENT, int(newPlacement));
      if (newPlacement != defaultPlacement)
            e->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);

      // adjust offset
      if (spanner) {
            // adjust segments individually
            for (auto a : spanner->spannerSegments()) {
                  // spanner segments share the placement setting of the spanner
                  // just adjust offset
                  if (defaultPlacement == Placement::BELOW && a->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED)
                        normalize = staffHeight;
                  else
                        normalize = 0.0;
                  qreal yp = a->offset().y() + normalize;
                  a->ryoffset() += normalize + offsetAdjust;

                  // if any segments are offset to opposite side of staff from placement,
                  // or if they are within staff,
                  // disable autoplace
                  bool disableAutoplace;
                  if (yp + a->height() <= 0.0)
                        disableAutoplace = (newPlacement == Placement::BELOW);
                  else if (yp > staffHeight)
                        disableAutoplace = (newPlacement == Placement::ABOVE);
                  else
                        disableAutoplace = true;
                  if (disableAutoplace)
                        a->setAutoplace(false);
                  // needed for https://musescore.org/en/node/281312
                  // ideally we would rebase and calculate new offset
                  // but this may not be possible
                  // since original offset is relative to system
                  a->rxoffset() = 0;
                  }
            }
      else {
            e->ryoffset() += normalize + offsetAdjust;
            // if within staff, disable autoplace
            if (ypos + e->height() > 0.0 && ypos <= staffHeight)
                  e->setAutoplace(false);
            }
      }

//---------------------------------------------------------
//   readNoteProperties206
//---------------------------------------------------------

bool readNoteProperties206(Note* note, XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "pitch")
            note->setPitch(e.readInt());
      else if (tag == "tpc") {
            const int tpc = e.readInt();
            note->setTpc1(tpc);
            note->setTpc2(tpc);
            }
      else if (tag == "track")            // for performance
            note->setTrack(e.readInt());
      else if (tag == "Accidental") {
            Accidental* a = new Accidental(note->score());
            a->setTrack(note->track());
            a->read(e);
            note->add(a);
            }
      else if (tag == "Tie") {
            Tie* tie = new Tie(note->score());
            tie->setParent(note);
            tie->setTrack(note->track());
            readTie206(e, tie);
            tie->setStartNote(note);
            note->setTieFor(tie);
            }
      else if (tag == "tpc2")
            note->setTpc2(e.readInt());
      else if (tag == "small")
            note->setSmall(e.readInt());
      else if (tag == "mirror")
            note->readProperty(e, Pid::MIRROR_HEAD);
      else if (tag == "dotPosition")
            note->readProperty(e, Pid::DOT_POSITION);
      else if (tag == "fixed")
            note->setFixed(e.readBool());
      else if (tag == "fixedLine")
            note->setFixedLine(e.readInt());
      else if (tag == "head")
            note->readProperty(e, Pid::HEAD_GROUP);
      else if (tag == "velocity")
            note->setVeloOffset(e.readInt());
      else if (tag == "play")
            note->setPlay(e.readInt());
      else if (tag == "tuning")
            note->setTuning(e.readDouble());
      else if (tag == "fret")
            note->setFret(e.readInt());
      else if (tag == "string")
            note->setString(e.readInt());
      else if (tag == "ghost")
            note->setGhost(e.readInt());
      else if (tag == "headType")
            note->readProperty(e, Pid::HEAD_TYPE);
      else if (tag == "veloType")
            note->readProperty(e, Pid::VELO_TYPE);
      else if (tag == "line")
            note->setLine(e.readInt());
      else if (tag == "Fingering") {
            Fingering* f = new Fingering(note->score());
            f->setTrack(note->track());
            readText206(e, f, note);
            note->add(f);
            }
      else if (tag == "Symbol") {
            Symbol* s = new Symbol(note->score());
            s->setTrack(note->track());
            s->read(e);
            note->add(s);
            }
      else if (tag == "Image") {
            if (MScore::noImages)
                  e.skipCurrentElement();
            else {
                  Image* image = new Image(note->score());
                  image->setTrack(note->track());
                  image->read(e);
                  note->add(image);
                  }
            }
      else if (tag == "Bend") {
            Bend* b = new Bend(note->score());
            b->setTrack(note->track());
            b->read(e);
            note->add(b);
            }
      else if (tag == "NoteDot") {
            NoteDot* dot = new NoteDot(note->score());
            dot->read(e);
            note->add(dot);
            }
      else if (tag == "Events") {
            note->playEvents().clear();    // remove default event
            while (e.readNextStartElement()) {
                  const QStringRef& etag(e.name());
                  if (etag == "Event") {
                        NoteEvent ne;
                        ne.read(e);
                        note->playEvents().append(ne);
                        }
                  else
                        e.unknown();
                  }
            if (Chord* ch = note->chord())
                  ch->setPlayEventType(PlayEventType::User);
            }
      else if (tag == "endSpanner") {
            int id = e.intAttribute("id");
            Spanner* sp = e.findSpanner(id);
            if (sp) {
                  sp->setEndElement(note);
                  if (sp->isTie())
                        note->setTieBack(toTie(sp));
                  else {
                        if (sp->isGlissando() && note->parent() && note->parent()->isChord())
                              toChord(note->parent())->setEndsGlissando(true);
                        note->addSpannerBack(sp);
                        }
                  e.removeSpanner(sp);
                  }
            else {
                  // End of a spanner whose start element will appear later;
                  // may happen for cross-staff spanner from a lower to a higher staff
                  // (for instance a glissando from bass to treble staff of piano).
                  // Create a place-holder spanner with end data
                  // (a TextLine is used only because both Spanner or SLine are abstract,
                  // the actual class does not matter, as long as it is derived from Spanner)
                  int id1 = e.intAttribute("id", -1);
                  Staff* staff = note->staff();
                  if (id1 != -1 &&
                              // DISABLE if pasting into a staff with linked staves
                              // because the glissando is not properly cloned into the linked staves
                              staff && (!e.pasteMode() || !staff->links() || staff->links()->empty())) {
                        Spanner* placeholder = new TextLine(note->score());
                        placeholder->setAnchor(Spanner::Anchor::NOTE);
                        placeholder->setEndElement(note);
                        placeholder->setTrack2(note->track());
                        placeholder->setTick(Fraction(0,1));
                        placeholder->setTick2(e.tick());
                        e.addSpanner(id1, placeholder);
                        }
                  }
            e.readNext();
            }
      else if (tag == "TextLine"
            || tag == "Glissando") {
            Spanner* sp = toSpanner(Element::name2Element(tag, note->score()));
            // check this is not a lower-to-higher cross-staff spanner we already got
            int id = e.intAttribute("id");
            Spanner* placeholder = e.findSpanner(id);
            if (placeholder && placeholder->endElement()) {
                  // if it is, fill end data from place-holder
                  sp->setAnchor(Spanner::Anchor::NOTE);           // make sure we can set a Note as end element
                  sp->setEndElement(placeholder->endElement());
                  sp->setTrack2(placeholder->track2());
                  sp->setTick(e.tick());                          // make sure tick2 will be correct
                  sp->setTick2(placeholder->tick2());
                  toNote(placeholder->endElement())->addSpannerBack(sp);
                  // remove no longer needed place-holder before reading the new spanner,
                  // as reading it also adds it to XML reader list of spanners,
                  // which would overwrite the place-holder
                  e.removeSpanner(placeholder);
                  delete placeholder;
                  }
            sp->setTrack(note->track());
            sp->read(e);
            Staff* staff = note->staff();
            // DISABLE pasting of glissandi into staves with other lionked staves
            // because the glissando is not properly cloned into the linked staves
            if (e.pasteMode() && staff && staff->links() && !staff->links()->empty()) {
                  e.removeSpanner(sp);    // read() added the element to the XMLReader: remove it
                  delete sp;
                  }
            else {
                  sp->setAnchor(Spanner::Anchor::NOTE);
                  sp->setStartElement(note);
                  sp->setTick(e.tick());
                  note->addSpannerFor(sp);
                  sp->setParent(note);
                  adjustPlacement(sp);
                  }
            }
      else if (tag == "offset")
            note->Element::readProperties(e);
      else if (note->Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   ReadStyleName206
//    Retrieve the content of the "style" tag from the
//    QString with the content of the whole text tag
//---------------------------------------------------------

static QString ReadStyleName206(QString xmlTag)
      {
      QString s;
      if (xmlTag.contains("<style>")) {
            QRegExp re("<style>([^<]+)</style>");
            if (re.indexIn(xmlTag) > -1)
                  s = re.cap(1);
            }
      return s;
      }

//---------------------------------------------------------
//   readTextPropertyStyle206
//    This reads only the 'style' tag, so that it can be read
//    before setting anything else.
//---------------------------------------------------------

static bool readTextPropertyStyle206(QString xmlTag, const XmlReader& e, TextBase* t, Element* be)
      {
      QString s = ReadStyleName206(xmlTag);

      if (s.isEmpty())
            return false;

      if (!be->isTuplet()) {      // Hack
            if (excessTextStyles206.find(s) != excessTextStyles206.end()) {
                  // Init the text with a style that can't be stored as a user style
                  // due to the limit on the number of user styles possible.
                  // Use User-1, since it has all the possible user style pids
                  t->initTid(Tid::DEFAULT);
                  std::map<Sid, QVariant> styleVals = excessTextStyles206[s];
                  for (const StyledProperty& p : *textStyle("User-1")) {
                        if (t->getProperty(p.pid) == t->propertyDefault(p.pid) && styleVals.find(p.sid) != styleVals.end())
                              t->setProperty(p.pid, styleVals[p.sid]);
                        }
                  }
            else {
                  Tid ss;
                  ss = e.lookupUserTextStyle(s);
                  if (ss == Tid::TEXT_STYLES)
                        ss = textStyleFromName(s);
                  if (ss != Tid::TEXT_STYLES)
                        t->initTid(ss);
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   readTextProperties206
//---------------------------------------------------------

static bool readTextProperties206(XmlReader& e, TextBase* t)
      {
      const QStringRef& tag(e.name());
      if (tag == "style") {
            e.skipCurrentElement(); // read in readTextPropertyStyle206
            }
      else if (tag == "foregroundColor")  // same as "color" ?
            e.skipCurrentElement();
      else if (tag == "frame") {
            t->setFrameType(e.readBool() ? FrameType::SQUARE : FrameType::NO_FRAME);
            t->setPropertyFlags(Pid::FRAME_TYPE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "frameRound")
            t->readProperty(e, Pid::FRAME_ROUND);
      else if (tag == "circle") {
            if (e.readBool())
                  t->setFrameType(FrameType::CIRCLE);
            else {
                  if (t->circle())
                        t->setFrameType(FrameType::SQUARE);
                  }
            t->setPropertyFlags(Pid::FRAME_TYPE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "paddingWidthS")
            t->readProperty(e, Pid::FRAME_PADDING);
      else if (tag == "frameWidthS")
            t->readProperty(e, Pid::FRAME_WIDTH);
      else if (tag == "frameColor")
            t->readProperty(e, Pid::FRAME_FG_COLOR);
      else if (tag == "backgroundColor")
            t->readProperty(e, Pid::FRAME_BG_COLOR);
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
            t->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
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
            t->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
            }
      else if (tag == "pos") {
            t->readProperty(e, Pid::OFFSET);
            if ((char(t->align()) & char(Align::VMASK)) == char(Align::TOP))
                  t->ryoffset() += .5 * t->score()->spatium();     // HACK: bbox is different in 2.x
            adjustPlacement(t);
            }
      else if (!t->readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   TextReaderContext206
//    For 2.x files, the style tag could be in a different
//    position with respect to 3.x files. Since seek
//    position is not reliable for readline in QIODevices (for
//    example because of non-single-byte characters in at least
//    one of the fields; some two-byte characters are counted as
//    two single-byte characters and thus the reading could
//    start at the wrong position), a copy of the text tag
//    is created and read in a separate XmlReader, while
//    the text style is extracted from a QString containing
//    the whole text xml tag.
//    TextReaderContext206 takes care of this process
//---------------------------------------------------------

class TextReaderContext206 {
      XmlReader& origReader;
      XmlReader tagReader;
      QString xmlTag;

   public:
      TextReaderContext206(XmlReader& e)
         : origReader(e), tagReader(QString())
            {
            // Create a new xml document containing only the (text) xml chunk
            QString name = origReader.name().toString();
            qint64 additionalLines = origReader.lineNumber() - 2; // Subtracting the 2 new lines that will be added
            xmlTag = origReader.readXml();
            xmlTag.prepend("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<" + name + ">");
            xmlTag.append("</" + name + ">\n");
            tagReader.addData(xmlTag); // Add the xml data to the XmlReader
            // the additional lines are needed to output the correct line number
            // of the original file in case of error
            tagReader.setOffsetLines(additionalLines);
            copyProperties(origReader, tagReader);
            tagReader.readNextStartElement(); // read up to the first "name" tag
            }

      // Disable copying the TextReaderContext206
      TextReaderContext206(const TextReaderContext206&) = delete;
      TextReaderContext206& operator=(const TextReaderContext206&) = delete;

      ~TextReaderContext206()
            {
            // Ensure to copy back the potentially changed properties
            // to the original XmlReader before destruction
            copyProperties(tagReader, origReader);
            }

      XmlReader& reader() { return tagReader; }
      const QString& tag() { return xmlTag; }
   private:
      void copyProperties(XmlReader& original, XmlReader& derived);
      };

//---------------------------------------------------------
//   copyProperties
//    Copy some of the XmlReader properties of an original
//    XmlReader object to a "derived" XmlReader object.
//    This is used for example when using the derived XmlReader
//    to read the element properties of a 2.x text, or to
//    update the base XmlReader object (for example, its
//    link list) after reading the properties of a 2.x text
//---------------------------------------------------------

void TextReaderContext206::copyProperties(XmlReader& original, XmlReader& derived)
      {
      derived.setDocName(original.getDocName());
      derived.setTrackOffset(original.trackOffset());
      derived.setTrack(original.track() - original.trackOffset());

      derived.setTickOffset(original.tickOffset());
      derived.setTick(original.tick() - original.tickOffset());

      derived.setLastMeasure(original.lastMeasure());
      derived.setCurrentMeasure(original.currentMeasure());
      derived.setCurrentMeasureIndex(original.currentMeasureIndex());

      derived.linkIds() = original.linkIds();
      derived.staffLinkedElements() = original.staffLinkedElements();
      }

//---------------------------------------------------------
//   readText206
//---------------------------------------------------------

static void readText206(XmlReader& e, TextBase* t, Element* be)
      {
      TextReaderContext206 ctx(e);
      readTextPropertyStyle206(ctx.tag(), e, t, be);
      while (ctx.reader().readNextStartElement()) {
            if (!readTextProperties206(ctx.reader(), t))
                  ctx.reader().unknown();
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

static void readTempoText(TempoText* t, XmlReader& e)
      {
      TextReaderContext206 ctx(e);
      readTextPropertyStyle206(ctx.tag(), e, t, t);
      while (ctx.reader().readNextStartElement()) {
            const QStringRef& tag(ctx.reader().name());
            if (tag == "tempo")
                  t->setTempo(ctx.reader().readDouble());
            else if (tag == "followText")
                  t->setFollowText(ctx.reader().readInt());
            else if (!readTextProperties206(ctx.reader(), t))
                  ctx.reader().unknown();
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
//   readMarker
//---------------------------------------------------------

static void readMarker(Marker* m, XmlReader& e)
      {
      TextReaderContext206 ctx(e);
      readTextPropertyStyle206(ctx.tag(), e, m, m);
      Marker::Type mt = Marker::Type::SEGNO;

      while (ctx.reader().readNextStartElement()) {
            const QStringRef& tag(ctx.reader().name());
            if (tag == "label") {
                  QString s(ctx.reader().readElementText());
                  m->setLabel(s);
                  mt = m->markerType(s);
                  }
            else if (!readTextProperties206(ctx.reader(), m))
                  ctx.reader().unknown();
            }
      m->setMarkerType(mt);
      }

//---------------------------------------------------------
//   readDynamic
//---------------------------------------------------------

static void readDynamic(Dynamic* d, XmlReader& e)
      {
      TextReaderContext206 ctx(e);
      readTextPropertyStyle206(ctx.tag(), e, d, d);
      while (ctx.reader().readNextStartElement()) {
            const QStringRef& tag = ctx.reader().name();
            if (tag == "subtype")
                  d->setDynamicType(ctx.reader().readElementText());
            else if (tag == "velocity")
                  d->setVelocity(ctx.reader().readInt());
            else if (tag == "dynType")
                  d->setDynRange(Dynamic::Range(ctx.reader().readInt()));
            else if (!readTextProperties206(ctx.reader(), d))
                  ctx.reader().unknown();
            }
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
                  tuplet->resetNumberProperty();
                  readText206(e, _number, tuplet);
                  _number->setVisible(tuplet->visible());     //?? override saved property
                  _number->setTrack(tuplet->track());
                  // move property flags from _number
                  for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN, Pid::SIZE_SPATIUM_DEPENDENT })
                        tuplet->setPropertyFlags(p, _number->propertyFlags(p));
                  }
            else if (!readTupletProperties206(e, tuplet))
                  e.unknown();
            }
      Fraction r = (tuplet->ratio() == Fraction(1,1)) ? tuplet->ratio() : tuplet->ratio().reduced();
      Fraction f(r.denominator(), tuplet->baseLen().fraction().denominator());
      tuplet->setTicks(f.reduced());
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
                  readText206(e, _verseNumber, lyrics);
                  _verseNumber->setParent(lyrics);
                  }
            else if (tag == "style")
                  e.readElementText();    // ignore style
            else if (!lyrics->readProperties(e))
                  e.unknown();
            }

      // if any endTick, make it relative to current tick
      if (iEndTick)
            lyrics->setTicks(Fraction::fromTicks(iEndTick) - e.tick());
      if (_verseNumber) {
            // TODO: add text to main text
            delete _verseNumber;
            }
      lyrics->setAutoplace(true);
      if (!lyrics->isStyled(Pid::OFFSET) && !e.pasteMode()) {
            // fix offset for pre-3.1 scores
            // 2.x and earlier: y offset was relative to staff; x offset was relative to center of notehead
            lyrics->rxoffset() -= lyrics->symWidth(SymId::noteheadBlack) * 0.5;
            //lyrics->ryoffset() -= lyrics->placeBelow() && lyrics->staff() ? lyrics->staff()->height() : 0.0;
            // temporarily set placement to above, since the original offset is relative to top of staff
            // depend on adjustPlacement() to change the placement if appropriate
            lyrics->setPlacement(Placement::ABOVE);
            adjustPlacement(lyrics);
            }
      }

//---------------------------------------------------------
//   readDurationProperties206
//---------------------------------------------------------

bool readDurationProperties206(XmlReader& e, DurationElement* de)
      {
      if (e.name() == "Tuplet") {
            int i = e.readInt();
            Tuplet* t = e.findTuplet(i);
            if (!t) {
                  qDebug("readDurationProperties206(): Tuplet id %d not found", i);
                  t = de->score()->searchTuplet(e, i);
                  if (t) {
                        qDebug("   ...found outside measure, input file corrupted?");
                        e.addTuplet(t);
                        }
                  }
            if (t) {
                  de->setTuplet(t);
                  if (!de->score()->undoStack()->active())     // HACK, also added in Undo::AddElement()
                        t->add(de);
                  }
            return true;
            }
      else if (de->Element::readProperties(e))
            return true;
      return false;
      }

//---------------------------------------------------------
//   readTupletProperties206
//---------------------------------------------------------

bool readTupletProperties206(XmlReader& e, Tuplet* de)
      {
      const QStringRef& tag(e.name());

      if (de->readStyledProperty(e, tag))
            ;
      else if (tag == "normalNotes")
            de->setProperty(Pid::NORMAL_NOTES, e.readInt());
      else if (tag == "actualNotes")
            de->setProperty(Pid::ACTUAL_NOTES, e.readInt());
      else if (tag == "p1")
            de->setProperty(Pid::P1, e.readPoint() * de->score()->spatium());
      else if (tag == "p2")
            de->setProperty(Pid::P2, e.readPoint() * de->score()->spatium());
      else if (tag == "baseNote")
            de->setBaseLen(TDuration(e.readElementText()));
      else if (tag == "Number") {
            Text* _number = new Text(de->score());
            de->setNumber(_number);
            _number->setComposition(true);
            _number->setParent(de);
//            _number->setSubStyleId(SubStyleId::TUPLET);
//            initSubStyle(SubStyleId::TUPLET);   // hack: initialize number
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN })
                  _number->resetProperty(p);
            readText206(e, _number, de);
            _number->setVisible(de->visible());     //?? override saved property
            _number->setTrack(de->track());
            // move property flags from _number
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN })
                  de->setPropertyFlags(p, _number->propertyFlags(p));
            }
      else if (!readDurationProperties206(e, de))
            return false;
      return true;
      }

//---------------------------------------------------------
//   readChordRestProperties206
//---------------------------------------------------------

bool readChordRestProperties206(XmlReader& e, ChordRest* ch)
      {
      const QStringRef& tag(e.name());

      if (tag == "durationType") {
            ch->setDurationType(e.readElementText());
            if (ch->actualDurationType().type() != TDuration::DurationType::V_MEASURE) {
                  if (ch->score()->mscVersion() < 112 && (ch->type() == ElementType::REST) &&
                              // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                              ch->ticks().numerator() != 0 &&
                              // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                              (ch->actualDurationType()==TDuration::DurationType::V_WHOLE && ch->ticks() <= Fraction(4, 4)) ) {
                        // old pre 2.0 scores: convert
                        ch->setDurationType(TDuration::DurationType::V_MEASURE);
                        }
                  else  // not from old score: set duration fraction from duration type
                        ch->setTicks(ch->actualDurationType().fraction());
                  }
            }
      else if (tag == "BeamMode") {
            QString val(e.readElementText());
            Beam::Mode bm = Beam::Mode::AUTO;
            if (val == "auto")
                  bm = Beam::Mode::AUTO;
            else if (val == "begin")
                  bm = Beam::Mode::BEGIN;
            else if (val == "mid")
                  bm = Beam::Mode::MID;
            else if (val == "end")
                  bm = Beam::Mode::END;
            else if (val == "no")
                  bm = Beam::Mode::NONE;
            else if (val == "begin32")
                  bm = Beam::Mode::BEGIN32;
            else if (val == "begin64")
                  bm = Beam::Mode::BEGIN64;
            else
                  bm = Beam::Mode(val.toInt());
            ch->setBeamMode(bm);
            }
      else if (tag == "Articulation") {
            Element* el = readArticulation(ch, e);
            if (el->isFermata())
                  ch->segment()->add(el);
            else
                  ch->add(el);
            }
      else if (tag == "leadingSpace" || tag == "trailingSpace") {
            qDebug("ChordRest: %s obsolete", tag.toLocal8Bit().data());
            e.skipCurrentElement();
            }
      else if (tag == "Beam") {
            int id = e.readInt();
            Beam* beam = e.findBeam(id);
            if (beam)
                  beam->add(ch);        // also calls ch->setBeam(beam)
            else
                  qDebug("Beam id %d not found", id);
            }
      else if (tag == "small")
            ch->setSmall(e.readInt());
      else if (tag == "duration")
            ch->setTicks(e.readFraction());
      else if (tag == "ticklen") {      // obsolete (version < 1.12)
            int mticks = ch->score()->sigmap()->timesig(e.tick()).timesig().ticks();
            int i = e.readInt();
            if (i == 0)
                  i = mticks;
            if ((ch->type() == ElementType::REST) && (mticks == i)) {
                  ch->setDurationType(TDuration::DurationType::V_MEASURE);
                  ch->setTicks(Fraction::fromTicks(i));
                  }
            else {
                  Fraction f = Fraction::fromTicks(i);
                  ch->setTicks(f);
                  ch->setDurationType(TDuration(f));
                  }
            }
      else if (tag == "dots")
            ch->setDots(e.readInt());
      else if (tag == "move")
            ch->setStaffMove(e.readInt());
      else if (tag == "Slur") {
            int id = e.intAttribute("id");
            if (id == 0)
                  id = e.intAttribute("number");                  // obsolete
            Spanner* spanner = e.findSpanner(id);
            QString atype(e.attribute("type"));

            if (!spanner) {
                  if (atype == "stop") {
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = ch->track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  else if (atype == "start")
                        qDebug("spanner: start without spanner");
                  }
            else {
                  if (atype == "start") {
                        if (spanner->ticks() > Fraction(0,1) && spanner->tick() == Fraction(-1,1)) // stop has been read first
                              spanner->setTicks(spanner->ticks() - e.tick() - Fraction::fromTicks(1));
                        spanner->setTick(e.tick());
                        spanner->setTrack(ch->track());
                        if (spanner->type() == ElementType::SLUR)
                              spanner->setStartElement(ch);
                        if (e.pasteMode()) {
                              for (ScoreElement* el : spanner->linkList()) {
                                    if (el == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(el);
                                    ls->setTick(spanner->tick());
                                    for (ScoreElement* ee : ch->linkList()) {
                                          ChordRest* cr = toChordRest(ee);
                                          if (cr->score() == ee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack(cr->track());
                                                if (ls->type() == ElementType::SLUR)
                                                      ls->setStartElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  else if (atype == "stop") {
                        spanner->setTick2(e.tick());
                        spanner->setTrack2(ch->track());
                        if (spanner->isSlur())
                              spanner->setEndElement(ch);
                        ChordRest* start = toChordRest(spanner->startElement());
                        if (start)
                              spanner->setTrack(start->track());
                        if (e.pasteMode()) {
                              for (ScoreElement* el : spanner->linkList()) {
                                    if (el == spanner)
                                          continue;
                                    Spanner* ls = static_cast<Spanner*>(el);
                                    ls->setTick2(spanner->tick2());
                                    for (ScoreElement* ee : ch->linkList()) {
                                          ChordRest* cr = toChordRest(ee);
                                          if (cr->score() == ee->score() && cr->staffIdx() == ls->staffIdx()) {
                                                ls->setTrack2(cr->track());
                                                if (ls->type() == ElementType::SLUR)
                                                      ls->setEndElement(cr);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  else
                        qDebug("readChordRestProperties206(): unknown Slur type <%s>", qPrintable(atype));
                  }
            e.readNext();
            }
      else if (tag == "Lyrics") {
            Lyrics* l = new Lyrics(ch->score());
            l->setTrack(e.track());
            readLyrics(l, e);
            ch->add(l);
            }
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            ch->setOffset(pt * ch->spatium());
            }
      else if (ch->isRest() && tag == "Image"){
            if (MScore::noImages)
                  e.skipCurrentElement();
            else {
                  Image *image = new Image(ch->score());
                  image->setTrack(e.track());
                  image->read(e);
                  ch->add(image);
                  }
            }
      else if (!readDurationProperties206(e, ch))
            return false;
      return true;
      }

//---------------------------------------------------------
//   readChordProperties206
//---------------------------------------------------------

bool readChordProperties206(XmlReader& e, Chord* ch)
      {
      const QStringRef& tag(e.name());

      if (tag == "Note") {
            Note* note = new Note(ch->score());
            // the note needs to know the properties of the track it belongs to
            note->setTrack(ch->track());
            note->setChord(ch);
            readNote(note, e);
            ch->add(note);
            }
      else if (readChordRestProperties206(e, ch))
            ;
      else if (tag == "Stem") {
            Stem* s = new Stem(ch->score());
            s->read(e);
            ch->add(s);
            }
      else if (tag == "Hook") {
            Hook* hook = new Hook(ch->score());
            hook->read(e);
            ch->add(hook);
            }
      else if (tag == "appoggiatura") {
            ch->setNoteType(NoteType::APPOGGIATURA);
            e.readNext();
            }
      else if (tag == "acciaccatura") {
            ch->setNoteType(NoteType::ACCIACCATURA);
            e.readNext();
            }
      else if (tag == "grace4") {
            ch->setNoteType(NoteType::GRACE4);
            e.readNext();
            }
      else if (tag == "grace16") {
            ch->setNoteType(NoteType::GRACE16);
            e.readNext();
            }
      else if (tag == "grace32") {
            ch->setNoteType(NoteType::GRACE32);
            e.readNext();
            }
      else if (tag == "grace8after") {
            ch->setNoteType(NoteType::GRACE8_AFTER);
            e.readNext();
            }
      else if (tag == "grace16after") {
            ch->setNoteType(NoteType::GRACE16_AFTER);
            e.readNext();
            }
      else if (tag == "grace32after") {
            ch->setNoteType(NoteType::GRACE32_AFTER);
            e.readNext();
            }
      else if (tag == "StemSlash") {
            StemSlash* ss = new StemSlash(ch->score());
            ss->read(e);
            ch->add(ss);
            }
      else if (ch->readProperty(tag, e, Pid::STEM_DIRECTION))
            ;
      else if (tag == "noStem")
            ch->setNoStem(e.readInt());
      else if (tag == "Arpeggio") {
            Arpeggio* arpeggio = new Arpeggio(ch->score());
            arpeggio->setTrack(ch->track());
            arpeggio->read(e);
            arpeggio->setParent(ch);
            ch->add(arpeggio);
            }
      // old glissando format, chord-to-chord, attached to its final chord
      else if (tag == "Glissando") {
            // the measure we are reading is not inserted in the score yet
            // as well as, possibly, the glissando intended initial chord;
            // then we cannot fully link the glissando right now;
            // temporarily attach the glissando to its final note as a back spanner;
            // after the whole score is read, Score::connectTies() will look for
            // the suitable initial note
            Note* finalNote = ch->upNote();
            Glissando* gliss = new Glissando(ch->score());
            gliss->read(e);
            gliss->setAnchor(Spanner::Anchor::NOTE);
            gliss->setStartElement(nullptr);
            gliss->setEndElement(nullptr);
            // in TAB, use straight line with no text
            if (ch->score()->staff(e.track() >> 2)->isTabStaff(ch->tick())) {
                  gliss->setGlissandoType(GlissandoType::STRAIGHT);
                  gliss->setShowText(false);
                  }
            finalNote->addSpannerBack(gliss);
            }
      else if (tag == "Tremolo") {
            Tremolo* tremolo = new Tremolo(ch->score());
            tremolo->setTrack(ch->track());
            tremolo->read(e);
            tremolo->setParent(ch);
            tremolo->setDurationType(ch->durationType());
            ch->setTremolo(tremolo);
            }
      else if (tag == "tickOffset")       // obsolete
            ;
      else if (tag == "ChordLine") {
            ChordLine* cl = new ChordLine(ch->score());
            cl->read(e);
            QPointF o = cl->offset();
            cl->setOffset(0.0, 0.0);
            ch->add(cl);
            e.fixOffsets().append({cl, o});
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   convertDoubleArticulations
//    Replace double articulations with proper SMuFL
//    symbols which were not available for use prior to 3.0
//---------------------------------------------------------

static void convertDoubleArticulations(Chord* chord, XmlReader& e)
      {
      std::vector<Articulation*> pairableArticulations;
      for (Articulation* a : chord->articulations()) {
            if (a->isStaccato() || a->isTenuto()
               || a->isAccent() || a->isMarcato()) {
                  pairableArticulations.push_back(a);
                  };
            }
      if (pairableArticulations.size() != 2)
            // Do not replace triple articulation if this happens
            return;

      SymId newSymId = SymId::noSym;
      for (int i = 0; i < 2; ++i) {
            if (newSymId != SymId::noSym)
                  break;
            Articulation* ai = pairableArticulations[i];
            Articulation* aj = pairableArticulations[(i == 0) ? 1 : 0];
            if (ai->isStaccato()) {
                  if (aj->isAccent())
                        newSymId = SymId::articAccentStaccatoAbove;
                  else if (aj->isMarcato())
                        newSymId = SymId::articMarcatoStaccatoAbove;
                  else if (aj->isTenuto())
                        newSymId = SymId::articTenutoStaccatoAbove;
                  }
            else if (ai->isTenuto()) {
                  if (aj->isAccent())
                        newSymId = SymId::articTenutoAccentAbove;
                  else if (aj->isMarcato())
                        newSymId = SymId::articMarcatoTenutoAbove;
                  }
            }

      if (newSymId != SymId::noSym) {
            // We reuse old articulation and change symbol ID
            // rather than constructing a new articulation
            // in order to preserve its other properties.
            Articulation* newArtic = pairableArticulations[0];
            for (Articulation* a : pairableArticulations) {
                  chord->remove(a);
                  if (a != newArtic) {
                        if (LinkedElements* link = a->links())
                              e.linkIds().remove(link->lid());
                        delete a;
                        }
                  }

            ArticulationAnchor anchor = newArtic->anchor();
            newArtic->setSymId(newSymId);
            newArtic->setAnchor(anchor);
            chord->add(newArtic);
            }
      }

//---------------------------------------------------------
//   fixTies
//---------------------------------------------------------

static void fixTies(Chord* chord)
      {
      std::vector<Note*> notes;
      for (Note* note : chord->notes()) {
            Tie* tie = note->tieBack();
            if (tie && tie->startNote()->pitch() != note->pitch()) {
                  notes.push_back(tie->startNote());
                  }
            }
      for (Note* note : notes) {
            Note* endNote = chord->findNote(note->pitch());
            note->tieFor()->setEndNote(endNote);
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
            else if (tag == "Stem") {
                  Stem* stem = new Stem(chord->score());
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "subtype")        // obsolete
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
            else if (readChordProperties206(e, chord))
                  ;
            else
                  e.unknown();
            }
      convertDoubleArticulations(chord, e);
      fixTies(chord);
      }

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

static void readRest(Rest* rest, XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readChordRestProperties206(e, rest))
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
      else if (!tl->readProperties(e))
            return false;
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
                  for (const QString& l : qAsConst(sl)) {
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
      adjustPlacement(volta);
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
      adjustPlacement(pedal);
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
      ottava->styleChanged();
      adjustPlacement(ottava);
      }

//---------------------------------------------------------
//   readHairpin206
//---------------------------------------------------------

void readHairpin206(XmlReader& e, Hairpin* h)
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
      adjustPlacement(h);
      }

//---------------------------------------------------------
//   readTrill206
//---------------------------------------------------------

void readTrill206(XmlReader& e, Trill* t)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  t->setTrillType(e.readElementText());
            else if (tag == "Accidental") {
                  Accidental* _accidental = new Accidental(t->score());
                  readAccidental206(_accidental, e);
                  _accidental->setParent(t);
                  t->setAccidental(_accidental);
                  }
            else if (tag == "ornamentStyle")
                  t->readProperty(e, Pid::ORNAMENT_STYLE);
            else if (tag == "play")
                  t->setPlayArticulation(e.readBool());
            else if (!t->SLine::readProperties(e))
                  e.unknown();
            }
      adjustPlacement(t);
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
      adjustPlacement(tlb);
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

Element* readArticulation(Element* parent, XmlReader& e)
      {
      Element* el = 0;
      SymId sym = SymId::fermataAbove;          // default -- backward compatibility (no type = ufermata in 1.2)
      ArticulationAnchor anchor  = ArticulationAnchor::TOP_STAFF;
      Direction direction = Direction::AUTO;
      Score* score = parent->score();
      int track = parent->track();
      double timeStretch = 0.0;
      bool useDefaultPlacement = true;

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
                                          if ((direction == Direction::DOWN) != (track & 1))
                                                useDefaultPlacement = false;
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
                              el = new Fermata(sym, score);
                              break;
                        default:
                              el = new Articulation(sym, score);
                              toArticulation(el)->setDirection(direction);
                              break;
                        };
                  }
            else if (tag == "anchor") {
                  useDefaultPlacement = false;
                  if (!el || el->isFermata())
                        anchor = ArticulationAnchor(e.readInt());
                  else
                        el->readProperties(e);
                  }
            else  if (tag == "direction") {
                  useDefaultPlacement = false;
                  if (!el || el->isFermata())
                        direction = toDirection(e.readElementText());
                  else
                        el->readProperties(e);
                  }
            else if (tag == "timeStretch") {
                  timeStretch = e.readDouble();
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
      if (!el)
            el = new Fermata(sym, score);
      if (el->isFermata()) {
            if (!qFuzzyIsNull(timeStretch))
                  el->setProperty(Pid::TIME_STRETCH, timeStretch);
            if (useDefaultPlacement)
                  el->setPlacement(track & 1 ? Placement::BELOW : Placement::ABOVE);
            else
                  setFermataPlacement(el, anchor, direction);
            }
      el->setTrack(track);
      return el;
      }

//---------------------------------------------------------
//   readSlurTieProperties
//---------------------------------------------------------

static bool readSlurTieProperties(XmlReader& e, SlurTie* st)
      {
      const QStringRef& tag(e.name());

      if (st->readProperty(tag, e, Pid::SLUR_DIRECTION))
            ;
      else if (tag == "lineType")
            st->setLineType(e.readInt());
      else if (tag == "SlurSegment") {
            SlurTieSegment* s = st->newSlurTieSegment();
            s->read(e);
            st->add(s);
            }
      else if (!st->Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   readSlur206
//---------------------------------------------------------

void readSlur206(XmlReader& e, Slur* s)
      {
      s->setTrack(e.track());      // set staff
      e.addSpanner(e.intAttribute("id"), s);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "track2")
                  s->setTrack2(e.readInt());
            else if (tag == "startTrack")       // obsolete
                  s->setTrack(e.readInt());
            else if (tag == "endTrack")         // obsolete
                  e.readInt();
            else if (!readSlurTieProperties(e, s))
                  e.unknown();
            }
      if (s->track2() == -1)
            s->setTrack2(s->track());
      }

//---------------------------------------------------------
//   readTie206
//---------------------------------------------------------

void readTie206(XmlReader& e, Tie* t)
      {
      e.addSpanner(e.intAttribute("id"), t);
      while (e.readNextStartElement()) {
            if (readSlurTieProperties(e, t))
                  ;
            else
                  e.unknown();
            }
      if (t->score()->mscVersion() <= 114 && t->spannerSegments().size() == 1) {
            // ignore manual adjustments to single-segment ties in older scores
            TieSegment* ss = t->frontSegment();
            QPointF zeroP;
            ss->ups(Grip::START).off     = zeroP;
            ss->ups(Grip::BEZIER1).off   = zeroP;
            ss->ups(Grip::BEZIER2).off   = zeroP;
            ss->ups(Grip::END).off       = zeroP;
            ss->setOffset(zeroP);
            ss->setUserOff2(zeroP);
            }
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
            e.setTick(Fraction::fromTicks(score->fileDivision(e.intAttribute("tick"))));

      bool irregular;
      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  m->setTicks(Fraction(sl[0].toInt(), sl[1].toInt()));
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            irregular = true;
            score->sigmap()->add(m->tick().ticks(), SigEvent(m->ticks(), m->timesig()));
            score->sigmap()->add(m->endTick().ticks(), SigEvent(m->timesig()));
            }
      else
            irregular = false;

      Staff* staff = score->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(m->tick()));

      // keep track of tick of previous element
      // this allows markings that need to apply to previous element to do so
      // even though we may have already advanced to next tick position
      Fraction lastTick = e.tick();

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "move")
                  e.setTick(e.readFraction() + m->tick());
            else if (tag == "tick") {
                  e.setTick(Fraction::fromTicks(score->fileDivision(e.readInt())));
                  lastTick = e.tick();
                  }
            else if (tag == "BarLine") {
                  Fermata* fermataAbove = nullptr;
                  Fermata* fermataBelow = nullptr;
                  BarLine* bl = new BarLine(score);
                  bl->setTrack(e.track());
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "subtype")
                              bl->setBarLineType(e.readElementText());
                        else if (t == "customSubtype")                      // obsolete
                              e.readInt();
                        else if (t == "span") {
                              //TODO bl->setSpanFrom(e.intAttribute("from", bl->spanFrom()));  // obsolete
                              // bl->setSpanTo(e.intAttribute("to", bl->spanTo()));            // obsolete
                              int span = e.readInt();
                              if (span)
                                    span--;
                              bl->setSpanStaff(span);
                              }
                        else if (t == "spanFromOffset")
                              bl->setSpanFrom(e.readInt());
                        else if (t == "spanToOffset")
                              bl->setSpanTo(e.readInt());
                        else if (t == "Articulation") {
                              Element* el = readArticulation(bl, e);
                              if (el->isFermata()) {
                                    if (el->placement() == Placement::ABOVE)
                                          fermataAbove = toFermata(el);
                                    else {
                                          fermataBelow = toFermata(el);
                                          fermataBelow->setTrack((bl->staffIdx() + bl->spanStaff()) * VOICES);
                                          }
                                    }
                              else
                                    bl->add(el);
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
                  bl->layout();
                  if (fermataAbove)
                        segment->add(fermataAbove);
                  if (fermataBelow)
                        segment->add(fermataBelow);
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
                        Fraction crticks = chord->actualTicks();
                        lastTick         = e.tick();
                        e.incTick(crticks);
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score);
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setTicks(m->timesig()/timeStretch);
                  rest->setTrack(e.track());
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  rest->setParent(segment);
                  readRest(rest, e);
                  segment->add(rest);

                  if (!rest->ticks().isValid())     // hack
                        rest->setTicks(m->timesig()/timeStretch);

                  lastTick = e.tick();
                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score);
                  breath->setTrack(e.track());
                  breath->setPlacement(Placement::ABOVE);
                  Fraction tick = e.tick();
                  breath->read(e);
                  // older scores placed the breath segment right after the chord to which it applies
                  // rather than before the next chordrest segment with an element for the staff
                  // result would be layout too far left if there are other segments due to notes in other staves
                  // we need to find tick of chord to which this applies, and add its duration
                  Fraction prevTick;
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
                  readSlur206(e, sl);
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
                  sp->eraseSpannerSegments();
                  e.addSpanner(e.intAttribute("id", -1), sp);

                  if (tag == "Volta")
                        readVolta206(e, toVolta(sp));
                  else if (tag == "Pedal")
                        readPedal(e, toPedal(sp));
                  else if (tag == "Ottava")
                        readOttava(e, toOttava(sp));
                  else if (tag == "HairPin")
                        readHairpin206(e, toHairpin(sp));
                  else if (tag == "Trill")
                        readTrill206(e, toTrill(sp));
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
                  readRest(rm, e);
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
                  if (e.tick().isZero()) {
                        if (score->staff(staffIdx)->clef(Fraction(0,1)) != clef->clefType())
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
                  Fraction currTick = e.tick();
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
                              score->sigmap()->add(m->tick().ticks(), SigEvent(m->ticks(), m->timesig()));
                              score->sigmap()->add(m->endTick().ticks(), SigEvent(m->timesig()));
                              }
                        else {
                              m->setTicks(m->timesig());
                              score->sigmap()->add(m->tick().ticks(), SigEvent(m->timesig()));
                              }
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score);
                  ks->setTrack(e.track());
                  ks->read(e);
                  Fraction curTick = e.tick();
                  if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick.isZero()) {
                        // ignore empty key signature
                        qDebug("remove keysig c at tick 0");
                        if (ks->links()) {
                              if (ks->links()->size() == 1)
                                    e.linkIds().remove(ks->links()->lid());
                              }
                        delete ks;
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
                  // MuseScore 3 has different types for system text and
                  // staff text while MuseScore 2 didn't.
                  // We need to decide first which one we should create.
                  TextReaderContext206 ctx(e);
                  QString styleName = ReadStyleName206(ctx.tag());
                  StaffTextBase* t;
                  if (styleName == "System"   || styleName == "Tempo"
                     || styleName == "Marker" || styleName == "Jump"
                     || styleName == "Volta") // TODO: is it possible to get it from style?
                        t = new SystemText(score);
                  else
                        t = new StaffText(score);
                  t->setTrack(e.track());
                  readTextPropertyStyle206(ctx.tag(), e, t, t);
                  while (ctx.reader().readNextStartElement()) {
                        if (!readTextProperties206(ctx.reader(), t))
                              ctx.reader().unknown();
                        }
                  if (t->empty()) {
                        if (t->links()) {
                              if (t->links()->size() == 1) {
                                    qDebug("reading empty text: deleted lid = %d", t->links()->lid());
                                    ctx.reader().linkIds().remove(t->links()->lid());
                                    delete t;
                                    }
                              }
                        }
                  else {
#if 0
                        // This code was added at commit ed5b615
                        // but it seems to no longer be appropriate.
                        // autoplace is usually true,
                        // exception is text within staff,
                        // and in this case offset is already correct without further adjustment.
                        if (!t->autoplace()) {
                              // adjust position
                              qreal userY = t->offset().y() / t->spatium();
                              qreal yo = -(-2.0 - userY) * t->spatium();
                              t->layout();
                              t->setAlign(Align::LEFT | Align::TOP);
                              t->ryoffset() = yo;
                              }
#endif
                        segment = m->getSegment(SegmentType::ChordRest, ctx.reader().tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score);
                  dyn->setTrack(e.track());
                  readDynamic(dyn, e);
                  segment = m->getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "RehearsalMark") {
                  RehearsalMark* el = new RehearsalMark(score);
                  el->setTrack(e.track());
                  readText206(e, el, el);
//                  el->setOffset(el->offset() - el->score()->styleValue(Pid::OFFSET, Sid::rehearsalMarkPosAbove).toPointF());
//                  if (el->offset().isNull())
//                        el->setAutoplace(true);
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
                  if (el->staff() && (el->isHarmony() || el->isFretDiagram() || el->isInstrumentChange()))
                        adjustPlacement(el);
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
            else if (tag == "Marker" || tag == "Jump") {
                  Element* el = Element::name2Element(tag, score);
                  el->setTrack(e.track());
                  if (tag == "Marker") {
                        Marker* ma = toMarker(el);
                        readMarker(ma, e);
                        Element* markerEl = toElement(ma);
                        m->add(markerEl);
                        }
                  else {
                        el->read(e);
                        m->add(el);
                        }
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
                  m->setStaffStemless(staffIdx, e.readInt());
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score);
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  e.addBeam(beam);
                  }
            else if (tag == "Segment") {
                  if (segment) segment->read(e);
                  else e.unknown();
                  }
            else if (tag == "MeasureNumber") {
                  MeasureNumber* noText = new MeasureNumber(score);
                  readText206(e, noText, m);
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
                  e.setTick(e.lastMeasure()->tick());
                  }
            else if (m->MeasureBase::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      e.checkTuplets();
      m->connectTremolo();
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
      b->setTopGap(0.0);
      b->setBottomGap(0.0);
      b->setAutoSizeEnabled(false);
      b->setPropertyFlags(Pid::TOP_GAP, PropertyFlags::UNSTYLED);
      b->setPropertyFlags(Pid::BOTTOM_GAP, PropertyFlags::UNSTYLED);

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
      e.setTick(Fraction(0,1));
      e.setTrack(staff * VOICES);
      Box* lastReadBox = nullptr;
      bool readMeasureLast = false;

      if (staff == 0) {
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());

                  if (tag == "Measure") {
                        if (lastReadBox) {
                              lastReadBox->setBottomGap(lastReadBox->bottomGap() + lastReadBox->propertyDefault(Pid::BOTTOM_GAP).toReal());
                              lastReadBox = nullptr;
                              }
                        readMeasureLast = true;

                        Measure* measure = 0;
                        measure = new Measure(score);
                        measure->setTick(e.tick());
                        //
                        // inherit timesig from previous measure
                        //
                        Measure* m = e.lastMeasure(); // measure->prevMeasure();
                        Fraction f(m ? m->timesig() : Fraction(4,4));
                        measure->setTicks(f);
                        measure->setTimesig(f);

                        readMeasure(measure, staff, e);
                        measure->checkMeasure(staff);
                        if (!measure->isMMRest()) {
                              score->measures()->add(measure);
                              e.setLastMeasure(measure);
                              e.setTick(measure->endTick());
                              }
                        else {
                              // this is a multi measure rest
                              // always preceded by the first measure it replaces
                              Measure* lm = e.lastMeasure();

                              if (lm) {
                                    lm->setMMRest(measure);
                                    measure->setTick(lm->tick());
                                    }
                              }
                        }
                  else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                        Box* b = toBox(Element::name2Element(tag, score));
                        readBox(b, e);
                        b->setTick(e.tick());
                        score->measures()->add(b);

                        // If it's the first box, and comes before any measures, reset to
                        // 301 default.
                        if (!readMeasureLast && !lastReadBox) {
                              b->setTopGap(b->propertyDefault(Pid::TOP_GAP).toReal());
                              b->setPropertyFlags(Pid::TOP_GAP, PropertyFlags::STYLED);
                              }
                        else if (readMeasureLast)
                              b->setTopGap(b->topGap() + b->propertyDefault(Pid::TOP_GAP).toReal());

                        lastReadBox = b;
                        readMeasureLast = false;
                        }
                  else if (tag == "tick")
                        e.setTick(Fraction::fromTicks(score->fileDivision(e.readInt())));
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
                        e.setTick(measure->tick());
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
                        e.setTick(Fraction::fromTicks(score->fileDivision(e.readInt())));
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
      excessTextStyles206.clear();
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();
            if (tag == "TextStyle")
                  readTextStyle206(style, e, excessTextStyles206);
            else if (tag == "Spatium")
                  style->set(Sid::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout")
                  readPageFormat(style, e);
            else if (tag == "displayInConcertPitch")
                  style->set(Sid::concertPitch, QVariant(bool(e.readInt())));
            else if (tag == "pedalY") {
                  qreal y = e.readDouble();
                  style->set(Sid::pedalPosBelow, QPointF(0.0, y));
                  }
            else if (tag == "lyricsDistance") {
                  qreal y = e.readDouble();
                  style->set(Sid::lyricsPosBelow, QPointF(0.0, y));
                  }
            else if (tag == "lyricsMinBottomDistance") {
                  // no longer meaningful since it is now measured from skyline rather than staff
                  //style->set(Sid::lyricsMinBottomDistance, QPointF(0.0, y));
                  e.skipCurrentElement();
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
            else if (tag == "harmonyY") {
                  qreal val = -e.readDouble();
                  if (val > 0.0) {
                        style->set(Sid::harmonyPlacement, int(Placement::BELOW));
                        style->set(Sid::chordSymbolAPosBelow,  QPointF(.0, val));
                        }
                  else {
                        style->set(Sid::harmonyPlacement, int(Placement::ABOVE));
                        style->set(Sid::chordSymbolAPosBelow,  QPointF(.0, val));
                        }
                  }
            else {
                  if (!style->readProperties(e)) {
                        e.skipCurrentElement();
                        }
                  }
            }

      bool disableHarmonyPlay = MScore::harmonyPlayDisableCompatibility && !MScore::testMode;
      if (disableHarmonyPlay) {
            style->set(Sid::harmonyPlay, false);
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
      if (!chordListTag)
            style->checkChordList();
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
                  const QString& t = e.attribute("tag");
                  QString val(e.readElementText());
                  if (id >= 0 && id < 32) {
                        score->layerTags()[id] = t;
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
            else if (tag == "page-offset")
                  score->setPageNumberOffset(e.readInt());
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
                  readText206(e, text, text);
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
                  readPart206(part, e);
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
                  if (tag == "HairPin")
                        readHairpin206(e, toHairpin(s));
                  else if (tag == "Ottava")
                        readOttava(e, toOttava(s));
                  else if (tag == "TextLine")
                        readTextLine206(e, toTextLine(s));
                  else if (tag == "Volta")
                        readVolta206(e, toVolta(s));
                  else if (tag == "Trill")
                        readTrill206(e, toTrill(s));
                  else if (tag == "Slur")
                        readSlur206(e, toSlur(s));
                  else {
                        Q_ASSERT(tag == "Pedal");
                        readPedal(e, toPedal(s));
                        }
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
                              qDebug("read206: readScore(): part cannot have parts");
                              e.skipCurrentElement();
                              }
                        }
                  }
            else if (tag == "Score") {          // recursion
                  if (MScore::noExcerpts)
                        e.skipCurrentElement();
                  else {
                        e.tracks().clear();
                        e.clearUserTextStyles();
                        MasterScore* m = score->masterScore();
                        Score* s       = new Score(m, MScore::baseStyle());
                        int defaultsVersion = m->style().defaultStyleVersion();
                        s->setStyle(*MStyle::resolveStyleDefaults(defaultsVersion));
                        s->style().setDefaultStyleVersion(defaultsVersion);
                        s->setEnableVerticalSpread(false);
                        Excerpt* ex = new Excerpt(m);

                        ex->setPartScore(s);
                        e.setLastMeasure(nullptr);
                        readScore(s, e);
                        ex->setTracks(e.tracks());
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
            MScore::lastError = QObject::tr("XML read error at line %1, column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(e.name().toString());
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

      for (Part* p : score->parts()) {
            p->updateHarmonyChannels(false);
            }

      if (score->isMaster()) {
            MasterScore* ms = static_cast<MasterScore*>(score);
            if (!ms->omr())
                  ms->setShowOmr(false);
            ms->rebuildMidiMapping();
            ms->updateChannel();
 //           ms->createPlayEvents();
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
                        const QStringRef& t(e.name());
                        qreal val = e.readDouble() * 0.5 / PPI;
                        if (t == "left-margin")
                              lm = val;
                        else if (t == "right-margin")
                              rm = val;
                        else if (t == "top-margin")
                              tm = val;
                        else if (t == "bottom-margin")
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

      setEnableVerticalSpread(false);

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
                  if (sp <= 0)
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
            if (sp <= 0)
                  continue;
            for (int span = 1; span <= sp; ++span) {
                  Staff* ns = staff(staffIdx + span);
                  ns->setBarLineSpan(sp - span);
                  }
            staffIdx += sp;
            }

      // fix positions
      //    offset = saved offset - layout position
      doLayout();
      for (auto i : e.fixOffsets()) {
            i.first->setOffset(i.second - i.first->pos());
            }

      // treat reading a 2.06 file as import
      // on save warn if old file will be overwritten
      setCreated(true);
      // don't autosave (as long as there's no change to the score)
      setAutosaveDirty(false);

      return FileError::FILE_NO_ERROR;
      }

MStyle* styleDefaults206()
      {
      static MStyle* result = nullptr;

      if (result)
            return result;

      result = new MStyle();
      QFile baseDefaults(":/styles/legacy-style-defaults-v2.mss");

      if (!baseDefaults.open(QIODevice::ReadOnly))
            return result;

      result->load(&baseDefaults);

      return result;
      }

}

