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
#include "timesig.h"
#include "slur.h"
#include "chord.h"
#include "rest.h"
#include "breath.h"
#include "repeat.h"
#include "utils.h"
#include "read206.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif


namespace Ms {

//---------------------------------------------------------
//   StyleVal206
//---------------------------------------------------------

struct StyleVal2 {
            StyleIdx idx;
            QVariant val;
            };
      static const StyleVal2 style206[] = {
      { StyleIdx::staffUpperBorder,            7.0  },
      { StyleIdx::staffLowerBorder,            7.0  },
      { StyleIdx::staffDistance,               6.5  },
      { StyleIdx::akkoladeDistance,            6.5  },
      { StyleIdx::minSystemDistance,           8.5  },
      { StyleIdx::maxSystemDistance,           15.0 },
//      { StyleIdx::lyricsDistance,              2.0  }, //renamed to lyricsPosBelow
      { StyleIdx::lyricsMinBottomDistance,     4.0  },
      { StyleIdx::lyricsLineHeight,            QVariant(1.0) },
      { StyleIdx::figuredBassFontFamily,       QVariant(QString("MScoreBC")) },
      { StyleIdx::figuredBassFontSize,         QVariant(8.0) },
      { StyleIdx::figuredBassYOffset,          QVariant(6.0) },
      { StyleIdx::figuredBassLineHeight,       QVariant(1.0) },
      { StyleIdx::figuredBassAlignment,        QVariant(0) },
      { StyleIdx::figuredBassStyle,            QVariant(0) },
      { StyleIdx::systemFrameDistance,         QVariant(7.0) },
      { StyleIdx::frameSystemDistance,         QVariant(7.0) },
      { StyleIdx::minMeasureWidth,             QVariant(5.0) },
      { StyleIdx::barWidth,                    QVariant(0.16) },      // 0.1875
      { StyleIdx::doubleBarWidth,              QVariant(0.16) },
      { StyleIdx::endBarWidth,                 QVariant(0.5) },       // 0.5
      { StyleIdx::doubleBarDistance,           QVariant(0.30) },
      { StyleIdx::endBarDistance,              QVariant(0.40) },     // 0.3
      { StyleIdx::repeatBarTips,               QVariant(false) },
      { StyleIdx::startBarlineSingle,          QVariant(false) },
      { StyleIdx::startBarlineMultiple,        QVariant(true) },
      { StyleIdx::bracketWidth,                QVariant(0.45) },
      { StyleIdx::bracketDistance,             QVariant(0.1) },
      { StyleIdx::akkoladeWidth,               QVariant(1.6) },
      { StyleIdx::akkoladeBarDistance,         QVariant(.4) },
      { StyleIdx::clefLeftMargin,              QVariant(0.64) },
      { StyleIdx::keysigLeftMargin,            QVariant(0.5) },
      { StyleIdx::timesigLeftMargin,           QVariant(0.5) },
      { StyleIdx::clefKeyRightMargin,          QVariant(1.75) },
      { StyleIdx::clefBarlineDistance,         QVariant(0.18) },      // was 0.5
      { StyleIdx::stemWidth,                   QVariant(0.13) },      // 0.09375
      { StyleIdx::shortenStem,                 QVariant(true) },
      { StyleIdx::shortStemProgression,        QVariant(0.25) },
      { StyleIdx::shortestStem,                QVariant(2.25) },
      { StyleIdx::beginRepeatLeftMargin,       QVariant(1.0) },
      { StyleIdx::minNoteDistance,             QVariant(0.25) },      // 0.4
      { StyleIdx::barNoteDistance,             QVariant(1.2) },
      { StyleIdx::barAccidentalDistance,       QVariant(.3) },
      { StyleIdx::multiMeasureRestMargin,      QVariant(1.2) },
      { StyleIdx::noteBarDistance,             QVariant(1.0) },
      { StyleIdx::measureSpacing,              QVariant(1.2) },
      { StyleIdx::staffLineWidth,              QVariant(0.08) },      // 0.09375
      { StyleIdx::ledgerLineWidth,             QVariant(0.16) },     // 0.1875
      { StyleIdx::ledgerLineLength,            QVariant(.6) },     // notehead width + this value
      { StyleIdx::accidentalDistance,          QVariant(0.22) },
      { StyleIdx::accidentalNoteDistance,      QVariant(0.22) },
      { StyleIdx::beamWidth,                   QVariant(0.5) },           // was 0.48
      { StyleIdx::beamDistance,                QVariant(0.5) },          // 0.25sp
      { StyleIdx::beamMinLen,                  QVariant(1.32) },      // 1.316178 exactly notehead width
      { StyleIdx::beamNoSlope,                 QVariant(false) },
      { StyleIdx::dotMag,                      QVariant(1.0) },
      { StyleIdx::dotNoteDistance,             QVariant(0.35) },
      { StyleIdx::dotRestDistance,             QVariant(0.25) },
      { StyleIdx::dotDotDistance,              QVariant(0.5) },
      { StyleIdx::propertyDistanceHead,        QVariant(1.0) },
      { StyleIdx::propertyDistanceStem,        QVariant(1.8) },
      { StyleIdx::propertyDistance,            QVariant(1.0) },
      { StyleIdx::articulationMag,             QVariant(1.0) },
      { StyleIdx::lastSystemFillLimit,         QVariant(0.3) },
      { StyleIdx::hairpinY,                    QVariant(7.5) },
      { StyleIdx::hairpinHeight,               QVariant(1.2) },
      { StyleIdx::hairpinContHeight,           QVariant(0.5) },
      { StyleIdx::hairpinLineWidth,            QVariant(0.13) },
      { StyleIdx::pedalY,                      QVariant(8) },
      { StyleIdx::pedalLineWidth,              QVariant(.15) },
      { StyleIdx::pedalLineStyle,              QVariant(int(Qt::SolidLine)) },
      { StyleIdx::trillY,                      QVariant(-1) },
      { StyleIdx::harmonyY,                    QVariant(2.5) },
      { StyleIdx::harmonyFretDist,             QVariant(0.5) },
      { StyleIdx::minHarmonyDistance,          QVariant(0.5) },
      { StyleIdx::maxHarmonyBarDistance,       QVariant(3.0) },
      { StyleIdx::capoPosition,                QVariant(0) },
      { StyleIdx::fretNumMag,                  QVariant(2.0) },
      { StyleIdx::fretNumPos,                  QVariant(0) },
      { StyleIdx::fretY,                       QVariant(2.0) },
      { StyleIdx::showPageNumber,              QVariant(true) },
      { StyleIdx::showPageNumberOne,           QVariant(false) },
      { StyleIdx::pageNumberOddEven,           QVariant(true) },
      { StyleIdx::showMeasureNumber,           QVariant(true) },
      { StyleIdx::showMeasureNumberOne,        QVariant(false) },
      { StyleIdx::measureNumberInterval,       QVariant(5) },
      { StyleIdx::measureNumberSystem,         QVariant(true) },
      { StyleIdx::measureNumberAllStaffs,      QVariant(false) },
      { StyleIdx::smallNoteMag,                QVariant(.7) },
      { StyleIdx::graceNoteMag,                QVariant(0.7) },
      { StyleIdx::smallStaffMag,               QVariant(0.7) },
      { StyleIdx::smallClefMag,                QVariant(0.8) },
      { StyleIdx::genClef,                     QVariant(true) },
      { StyleIdx::genKeysig,                   QVariant(true) },
//      { StyleIdx::genTimesig,                  QVariant(true) },
      { StyleIdx::genCourtesyTimesig,          QVariant(true) },
      { StyleIdx::genCourtesyKeysig,           QVariant(true) },
      { StyleIdx::genCourtesyClef,             QVariant(true) },
      { StyleIdx::swingRatio,                  QVariant(60)   },
      { StyleIdx::swingUnit,                   QVariant(QString("")) },
      { StyleIdx::useStandardNoteNames,        QVariant(true) },
      { StyleIdx::useGermanNoteNames,          QVariant(false) },
      { StyleIdx::useFullGermanNoteNames,      QVariant(false) },
      { StyleIdx::useSolfeggioNoteNames,       QVariant(false) },
      { StyleIdx::useFrenchNoteNames,          QVariant(false) },
      { StyleIdx::automaticCapitalization,     QVariant(true) },
      { StyleIdx::lowerCaseMinorChords,        QVariant(false) },
      { StyleIdx::lowerCaseBassNotes,          QVariant(false) },
      { StyleIdx::allCapsNoteNames,            QVariant(false) },
      { StyleIdx::chordStyle,                  QVariant(QString("std")) },
      { StyleIdx::chordsXmlFile,               QVariant(false) },
      { StyleIdx::chordDescriptionFile,        QVariant(QString("chords_std.xml")) },
      { StyleIdx::concertPitch,                QVariant(false) },
      { StyleIdx::createMultiMeasureRests,     QVariant(false) },
      { StyleIdx::minEmptyMeasures,            QVariant(2) },
      { StyleIdx::minMMRestWidth,              QVariant(4) },
      { StyleIdx::hideEmptyStaves,             QVariant(false) },
      { StyleIdx::dontHideStavesInFirstSystem, QVariant(true) },
      { StyleIdx::hideInstrumentNameIfOneInstrument, QVariant(true) },
      { StyleIdx::gateTime,                    QVariant(100) },
      { StyleIdx::tenutoGateTime,              QVariant(100) },
      { StyleIdx::staccatoGateTime,            QVariant(50) },
      { StyleIdx::slurGateTime,                QVariant(100) },
      { StyleIdx::ArpeggioNoteDistance,        QVariant(.5) },
      { StyleIdx::ArpeggioLineWidth,           QVariant(.18) },
      { StyleIdx::ArpeggioHookLen,             QVariant(.8) },
      { StyleIdx::SlurEndWidth,                QVariant(.07) },
      { StyleIdx::SlurMidWidth,                QVariant(.15) },
      { StyleIdx::SlurDottedWidth,             QVariant(.1) },
      { StyleIdx::MinTieLength,                QVariant(1.0) },
      { StyleIdx::SectionPause,                QVariant(qreal(3.0)) },
      { StyleIdx::MusicalSymbolFont,           QVariant(QString("Emmentaler")) },
      { StyleIdx::MusicalTextFont,             QVariant(QString("MScore Text")) },
      { StyleIdx::showHeader,                  QVariant(false) },
//      { StyleIdx::headerStyled,                QVariant(true) },
      { StyleIdx::headerFirstPage,             QVariant(false) },
      { StyleIdx::headerOddEven,               QVariant(true) },
      { StyleIdx::evenHeaderL,                 QVariant(QString()) },
      { StyleIdx::evenHeaderC,                 QVariant(QString()) },
      { StyleIdx::evenHeaderR,                 QVariant(QString()) },
      { StyleIdx::oddHeaderL,                  QVariant(QString()) },
      { StyleIdx::oddHeaderC,                  QVariant(QString()) },
      { StyleIdx::oddHeaderR,                  QVariant(QString()) },
      { StyleIdx::showFooter,                  QVariant(true) },
//      { StyleIdx::footerStyled,                QVariant(true) },
      { StyleIdx::footerFirstPage,             QVariant(true) },
      { StyleIdx::footerOddEven,               QVariant(true) },
      { StyleIdx::evenFooterL,                 QVariant(QString("$p")) },
      { StyleIdx::evenFooterC,                 QVariant(QString("$:copyright:")) },
      { StyleIdx::evenFooterR,                 QVariant(QString()) },
      { StyleIdx::oddFooterL,                  QVariant(QString()) },
      { StyleIdx::oddFooterC,                  QVariant(QString("$:copyright:")) },
      { StyleIdx::oddFooterR,                  QVariant(QString("$p")) },
      { StyleIdx::voltaY,                      QVariant(-3.0) },
      { StyleIdx::voltaHook,                   QVariant(1.9) },
      { StyleIdx::voltaLineWidth,              QVariant(.1) },
      { StyleIdx::voltaLineStyle,              QVariant(int(Qt::SolidLine)) },
      { StyleIdx::ottavaY,                     QVariant(-3.0) },
      { StyleIdx::ottavaHook,                  QVariant(1.9) },
      { StyleIdx::ottavaLineWidth,             QVariant(.1) },
      { StyleIdx::ottavaLineStyle,             QVariant(int(Qt::DashLine)) },
      { StyleIdx::ottavaNumbersOnly,           true },
      { StyleIdx::tabClef,                     QVariant(int(ClefType::TAB)) },
      { StyleIdx::tremoloWidth,                QVariant(1.2) },  // tremolo stroke width: notehead width
      { StyleIdx::tremoloBoxHeight,            QVariant(0.65) },
      { StyleIdx::tremoloStrokeWidth,          QVariant(0.5) },  // was 0.35
      { StyleIdx::tremoloDistance,             QVariant(0.8) },
      // TODO { StyleIdx::tremoloBeamLengthMultiplier, QVariant(0.62) },
      // TODO { StyleIdx::tremoloMaxBeamLength,        QVariant(12.0) },
      { StyleIdx::linearStretch,               QVariant(qreal(1.5)) },
      { StyleIdx::crossMeasureValues,          QVariant(false) },
      { StyleIdx::keySigNaturals,              QVariant(int(KeySigNatural::NONE)) },
      { StyleIdx::tupletMaxSlope,              QVariant(qreal(0.5)) },
      { StyleIdx::tupletOufOfStaff,            QVariant(true) },
      { StyleIdx::tupletVHeadDistance,         QVariant(.5) },
      { StyleIdx::tupletVStemDistance,         QVariant(.25) },
      { StyleIdx::tupletStemLeftDistance,      QVariant(.5) },
      { StyleIdx::tupletStemRightDistance,     QVariant(.5) },
      { StyleIdx::tupletNoteLeftDistance,      QVariant(0.0) },
      { StyleIdx::tupletNoteRightDistance,     QVariant(0.0) },
      { StyleIdx::barreLineWidth,              QVariant(1.0) },
      { StyleIdx::fretMag,                     QVariant(1.0) },
      { StyleIdx::scaleBarlines,               QVariant(true) },
      { StyleIdx::barGraceDistance,            QVariant(.6) },
      };

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
                        a->setHasBracket(i);
                  }
            else if (tag == "subtype") {
                  QString text = e.readElementText();
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
                  Articulation* atr = new Articulation(chord->score());
                  atr->setTrack(chord->track());
                  atr->read(e);
                  chord->add(atr);
                  }
            else if (chord->readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   convertOldTextStyleNames
//---------------------------------------------------------

static QString convertOldTextStyleNames(const QString& s)
      {
      QString rs(s);
      // convert 2.0 text styles
      if (s == "Repeat Text")
            rs = "Repeat Text Right";
      return rs;
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
      { SymId::ornamentDownPrall,         "downprall",                 },
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
      { SymId::luteFingeringRHThird,      "lutefingering3rd",          }
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
//   readArticulation
//---------------------------------------------------------

void readArticulation(Articulation* a, XmlReader& e)
      {
      a->setSymId(SymId::fermataAbove);      // default -- backward compatibility (no type = ufermata in 1.2)
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype") {
                  QString s = e.readElementText();
                  if (s[0].isDigit()) {
                        int oldType = s.toInt();
                        a->setSymId(articulationNames[oldType].id);
                        }
                  else {
                        a->setSymId(oldArticulationNames2SymId(s));
                        if (a->symId() == SymId::noSym) {
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
                                          a->setSymId(al[i].id);
                                          a->setUp(al[i].up);
                                          a->setDirection(a->up() ? Direction_UP : Direction_DOWN);
                                          break;
                                          }
                                    }
                              if (i == n)
                                    qDebug("Articulation: unknown type <%s>", qPrintable(s));
                              }
                        }
                  }
             else if (a->readProperties(e))
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
                  readArticulation(atr, e);
                  rest->add(atr);
                  }
            else if (rest->readProperties(e))
                  ;
            else
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
      Score* score = m->score();

      QList<Chord*> graceNotes;
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      for (int n = m->mstaves().size(); n <= staffIdx; ++n) {
            Staff* staff = score->staff(n);
            MStaff* s    = new MStaff;
            s->lines     = new StaffLines(score);
            s->lines->setParent(m);
            s->lines->setTrack(n * VOICES);
            s->lines->setVisible(!staff->invisible());
            m->mstaves().push_back(s);
            }

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
                  BarLine* barLine = new BarLine(score);
                  barLine->setTrack(e.track());
                  barLine->read(e);

                  //
                  //  StartRepeatBarLine: always at the beginning tick of a measure, always BarLineType::START_REPEAT
                  //  BarLine:            in the middle of a measure, has no semantic
                  //  EndBarLine:         at the end tick of a measure
                  //  BeginBarLine:       first segment of a measure

                  Segment::Type st;
                  if ((e.tick() != m->tick()) && (e.tick() != m->endTick()))
                        st = Segment::Type::BarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && e.tick() == m->tick())
                        st = Segment::Type::StartRepeatBarLine;
                  else if (e.tick() == m->tick() && segment == 0)
                        st = Segment::Type::BeginBarLine;
                  else
                        st = Segment::Type::EndBarLine;
                  segment = m->getSegment(st, e.tick());
                  segment->add(barLine);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score);
                  chord->setTrack(e.track());
                  readChord(chord, e);
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
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
                  readRest(rest, e);
                  segment = m->getSegment(rest, e.tick());
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
                  Segment* prev = m->findSegment(Segment::Type::ChordRest, prevTick);
                  if (prev) {
                        // find chordrest
                        ChordRest* lastCR = static_cast<ChordRest*>(prev->element(e.track()));
                        if (lastCR)
                              tick = prevTick + lastCR->actualTicks();
                        }
                  segment = m->getSegment(Segment::Type::Breath, tick);
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
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  // ?? sp->setAnchor(Spanner::Anchor::SEGMENT);
                  sp->read(e);
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
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(rm);
                  lastTick = e.tick();
                  e.incTick(m->ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score);
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);

                  // there may be more than one clef segment for same tick position
                  if (!segment) {
                        // this is the first segment of measure
                        segment = m->getSegment(Segment::Type::Clef, e.tick());
                        }
                  else {
                        bool firstSegment = false;
                        // the first clef may be missing and is added later in layout
                        for (Segment* s = m->segments().first(); s && s->tick() == e.tick(); s = s->next()) {
                              if (s->segmentType() == Segment::Type::Clef
                                    // hack: there may be other segment types which should
                                    // generate a clef at current position
                                 || s->segmentType() == Segment::Type::StartRepeatBarLine
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
                                    if (s->segmentType() == Segment::Type::Clef) {
                                          segment = s;
                                          break;
                                          }
                                    }
                              if (!segment) {
                                    segment = new Segment(m, Segment::Type::Clef, e.tick());
                                    m->segments().insert(segment, ns);
                                    }
                              }
                        else {
                              // this is the first clef: move to left
                              segment = m->getSegment(Segment::Type::Clef, e.tick());
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
                  // if time sig not at begining of measure => courtesy time sig
                  int currTick = e.tick();
                  bool courtesySig = (currTick > m->tick());
                  if (courtesySig) {
                        // if courtesy sig., just add it without map processing
                        segment = m->getSegment(Segment::Type::TimeSigAnnounce, currTick);
                        segment->add(ts);
                        }
                  else {
                        // if 'real' time sig., do full process
                        segment = m->getSegment(Segment::Type::TimeSig, currTick);
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
                        segment = m->getSegment(courtesySig ? Segment::Type::KeySigAnnounce : Segment::Type::KeySig, curTick);
                        segment->add(ks);
                        if (!courtesySig)
                              staff->setKey(curTick, ks->keySigEvent());
                        }
                  }
            else if (tag == "Text") {
                  Text* t = new StaffText(score);
                  t->setTrack(e.track());
                  t->read(e);
                  if (t->empty()) {
                        qDebug("reading empty text: deleted");
                        delete t;
                        }
                  else {
                        segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score);
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "RehearsalMark"
               || tag == "InstrumentChange"
               || tag == "StaffState"
               || tag == "FiguredBass"
               ) {
                  Element* el = Element::name2Element(tag, score);
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  el->setTrack(e.track());
                  el->read(e);
                  segment = m->getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(el);
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
                        segment = m->getSegment(Segment::Type::ChordRest, e.tick());
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
                  segment = m->getSegment(Segment::Type::BeginBarLine, m->tick());
                  segment->add(barLine);
                  }
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score);
                  tuplet->setTrack(e.track());
                  tuplet->setTick(e.tick());
                  tuplet->setParent(m);
                  tuplet->read(e);
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
                  if (m->mstaves()[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score);
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->mstaves()[staffIdx]->_vspacerDown->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (m->mstaves()[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(score);
                        spacer->setSpacerType(SpacerType::UP);
                        spacer->setTrack(staffIdx * VOICES);
                        m->add(spacer);
                        }
                  m->mstaves()[staffIdx]->_vspacerUp->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  m->mstaves()[staffIdx]->_visible = e.readInt();
            else if (tag == "slashStyle")
                  m->mstaves()[staffIdx]->_slashStyle = e.readInt();
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
                  Text* noText = new Text(score);
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
                  // noText->setFlag(ElementFlag::MOVABLE, false); ??
                  noText->setTrack(e.track());
                  noText->setParent(m);
                  m->mstaves()[noText->staffIdx()]->setNoText(noText);
                  }
            else if (tag == "SystemDivider") {
                  SystemDivider* sd = new SystemDivider(score);
                  sd->read(e);
                  m->add(sd);
                  }
            else if (tag == "Ambitus") {
                  Ambitus* range = new Ambitus(score);
                  range->read(e);
                  segment = m->getSegment(Segment::Type::Ambitus, e.tick());
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
                        MeasureBase* mb = static_cast<MeasureBase*>(Element::name2Element(tag, score));
                        mb->read(e);
                        mb->setTick(e.tick());
                        score->measures()->add(mb);
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
static void readStyle(MStyle* style, XmlReader& e)
      {
      QString oldChordDescriptionFile = style->value(StyleIdx::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();

            if (tag == "lyricsDistance")        // was renamed
                  tag = "lyricsPosBelow";

            if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(e);
                  // convert old 2.0 text styles
                  s.setName(convertOldTextStyleNames(s.name()));
                  if (s.family() == "MuseJazz")
                        s.setFamily("MuseJazz Text");
                  style->setTextStyle(s);
                  }
            else if (tag == "Spatium")
                  style->set(StyleIdx::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout") {
                  PageFormat pf;
                  pf.copy(*style->pageFormat());
                  pf.read(e);
                  style->setPageFormat(pf);
                  }
            else if (tag == "displayInConcertPitch")
                  style->set(StyleIdx::concertPitch, QVariant(bool(e.readInt())));
            else if (tag == "ChordList") {
                  style->chordList()->clear();
                  style->chordList()->read(e);
                  style->setCustomChordList(true);
                  chordListTag = true;
                  }
            else {
                  QString val(e.readElementText());
                  style->convertToUnit(tag, val);
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
                  qreal sp = score->style()->value(StyleIdx::spatium).toDouble();
                  readStyle(score->style(), e);
                  if (score->style()->value(StyleIdx::MusicalTextFont).toString() == "MuseJazz")
                        score->style()->set(StyleIdx::MusicalTextFont, "MuseJazz Text");
                  // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
                  if (score->layoutMode() == LayoutMode::FLOAT) {
                        // style should not change spatium in
                        // float mode
                        score->style()->set(StyleIdx::spatium, sp);
                        }
                  score->setScoreFont(ScoreFont::fontFactory(score->style()->value(StyleIdx::MusicalSymbolFont).toString()));
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
                  part->read(e);
                  score->parts().push_back(part);
                  }
            else if ((tag == "HairPin")
                || (tag == "Ottava")
                || (tag == "TextLine")
                || (tag == "Volta")
                || (tag == "Trill")
                || (tag == "Slur")
                || (tag == "Pedal")) {
                  Spanner* s = static_cast<Spanner*>(Element::name2Element(tag, score));
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
                        Score* s = new Score(m, MScore::baseStyle());
                        Excerpt* ex = new Excerpt(m);

                        ex->setPartScore(s);
                        ex->setTracks(e.tracks());
                        s->read(e);
                        m->addExcerpt(ex);
                        }
                  }
            else if (tag == "PageList") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Page") {
                              Page* page = new Page(score);
                              score->pages().append(page);
                              page->read(e);
                              }
                        else
                              e.unknown();
                        }
                  }
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
            int minBarLineFrom = st->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
            if (st->barLineFrom() < minBarLineFrom)
                  st->setBarLineFrom(minBarLineFrom);
            if (st->barLineFrom() > st->lines() * 2)
                  st->setBarLineFrom(st->lines() * 2);
            // check spanTo
            Staff* stTo = st->barLineSpan() <= 1 ? st : score->staff(idx + st->barLineSpan() - 1);
            // 1-line staves have special bar line spans
            int maxBarLineTo        = stTo->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines()*2;
            int defaultBarLineTo    = stTo->lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (stTo->lines() - 1) * 2;
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

      if (!score->masterScore()->omr())
            score->masterScore()->setShowOmr(false);

      score->fixTicks();
      score->masterScore()->rebuildMidiMapping();
      score->masterScore()->updateChannel();
      score->createPlayEvents();

      return true;
      }

//---------------------------------------------------------
//   read206
//    import old version > 1.3  and < 2.x files
//---------------------------------------------------------

Score::FileError MasterScore::read206(XmlReader& e)
      {
      for (unsigned int i = 0; i < sizeof(style206)/sizeof(*style206); ++i)
            style()->set(style206[i].idx, style206[i].val);
      // old text style default
      TextStyle ts = style()->textStyle("Rehearsal Mark");
      ts.setSquare(false);
      ts.setFrameRound(20);
      style()->setTextStyle(ts);
      ts = style()->textStyle("Dynamics");
      ts.setItalic(false);
      style()->setTextStyle(ts);

      qDebug("read206");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                  setMscoreVersion(e.readElementText());
                  parseVersion(mscoreVersion());
                  }
            else if (tag == "programRevision")
                  setMscoreRevision(e.readInt());
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

      setCreated(false);
      return FileError::FILE_NO_ERROR;
      }

}

