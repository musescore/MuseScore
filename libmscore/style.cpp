//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mscore.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "chordlist.h"
#include "page.h"
#include "mscore.h"
#include "clef.h"
#include "tuplet.h"
#include "layout.h"
#include "property.h"
#include "elementlayout.h"
#include "read206.h"

namespace Ms {

//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

//---------------------------------------------------------
//   StyleType
//---------------------------------------------------------

struct StyleType {
      Sid _idx;
      const char* _name;       // xml name for read()/write()
      QVariant _defaultValue;

   public:
      Sid  styleIdx() const            { return _idx;          }
      int idx() const                       { return int(_idx);     }
      const char*  valueType() const        { return _defaultValue.typeName();    }
      const char*      name() const         { return _name;         }
      const QVariant&  defaultValue() const { return _defaultValue; }
      };

#define MM(x) ((x)/INCH)

static const StyleType styleTypes[] {
      { Sid::pageWidth,               "pageWidth",               210.0/INCH },
      { Sid::pageHeight,              "pageHeight",              297.0/INCH }, // A4
      { Sid::pagePrintableWidth,      "pagePrintableWidth",      190.0/INCH },
      { Sid::pageEvenLeftMargin,      "pageEvenLeftMargin",      10.0/INCH  },
      { Sid::pageOddLeftMargin,       "pageOddLeftMargin",       10.0/INCH  },
      { Sid::pageEvenTopMargin,       "pageEvenTopMargin",       10.0/INCH  },
      { Sid::pageEvenBottomMargin,    "pageEvenBottomMargin",    20.0/INCH  },
      { Sid::pageOddTopMargin,        "pageOddTopMargin",        10.0/INCH  },
      { Sid::pageOddBottomMargin,     "pageOddBottomMargin",     20.0/INCH  },
      { Sid::pageTwosided,            "pageTwosided",            true       },

      { Sid::staffUpperBorder,        "staffUpperBorder",        Spatium(7.0)  },
      { Sid::staffLowerBorder,        "staffLowerBorder",        Spatium(7.0)  },
      { Sid::staffDistance,           "staffDistance",           Spatium(6.5)  },
      { Sid::akkoladeDistance,        "akkoladeDistance",        Spatium(6.5)  },
      { Sid::minSystemDistance,       "minSystemDistance",       Spatium(8.5)  },
      { Sid::maxSystemDistance,       "maxSystemDistance",       Spatium(15.0) },

      { Sid::lyricsPlacement,         "lyricsPlacement",         int(Placement::BELOW)  },
      { Sid::lyricsPosAbove,          "lyricsPosAbove",          Spatium(-2.0) },
      { Sid::lyricsPosBelow,          "lyricsPosBelow",          Spatium(2.0) },
      { Sid::lyricsMinTopDistance,    "lyricsMinTopDistance",    Spatium(1.0)  },
      { Sid::lyricsMinBottomDistance, "lyricsMinBottomDistance", Spatium(2.0)  },
      { Sid::lyricsLineHeight,        "lyricsLineHeight",        1.0 },
      { Sid::lyricsDashMinLength,     "lyricsDashMinLength",     Spatium(0.4) },
      { Sid::lyricsDashMaxLength,     "lyricsDashMaxLegth",      Spatium(0.8) },
      { Sid::lyricsDashMaxDistance,   "lyricsDashMaxDistance",   Spatium(16.0) },
      { Sid::lyricsDashForce,         "lyricsDashForce",         QVariant(true) },
      { Sid::lyricsAlignVerseNumber,  "lyricsAlignVerseNumber",  true },
      { Sid::lyricsLineThickness,     "lyricsLineThickness",     Spatium(0.1) },

      { Sid::figuredBassFontFamily,   "figuredBassFontFamily",   QString("MScoreBC") },

//      { Sid::figuredBassFontSize,     "figuredBassFontSize",     QVariant(8.0) },
      { Sid::figuredBassYOffset,      "figuredBassYOffset",      QVariant(6.0) },
      { Sid::figuredBassLineHeight,   "figuredBassLineHeight",   QVariant(1.0) },
      { Sid::figuredBassAlignment,    "figuredBassAlignment",    QVariant(0) },
      { Sid::figuredBassStyle,        "figuredBassStyle" ,       QVariant(0) },
      { Sid::systemFrameDistance,     "systemFrameDistance",     Spatium(7.0) },
      { Sid::frameSystemDistance,     "frameSystemDistance",     Spatium(7.0) },
      { Sid::minMeasureWidth,         "minMeasureWidth",         Spatium(5.0) },
      { Sid::barWidth,                "barWidth",                Spatium(0.16) },      // 0.1875
      { Sid::doubleBarWidth,          "doubleBarWidth",          Spatium(0.16) },

      { Sid::endBarWidth,             "endBarWidth",             Spatium(0.5) },       // 0.5
      { Sid::doubleBarDistance,       "doubleBarDistance",       Spatium(0.46) },      // 0.3
      { Sid::endBarDistance,          "endBarDistance",          Spatium(.40 + (.5) * .5) },     // 0.3
      { Sid::repeatBarlineDotSeparation, "repeatBarlineDotSeparation", Spatium(.40 + .16 * .5) },
      { Sid::repeatBarTips,           "repeatBarTips",           QVariant(false) },
      { Sid::startBarlineSingle,      "startBarlineSingle",      QVariant(false) },
      { Sid::startBarlineMultiple,    "startBarlineMultiple",    QVariant(true) },
      { Sid::bracketWidth,            "bracketWidth",            Spatium(0.45) },
      { Sid::bracketDistance,         "bracketDistance",         Spatium(0.1) },
      { Sid::akkoladeWidth,           "akkoladeWidth",           Spatium(1.6) },
      { Sid::akkoladeBarDistance,     "akkoladeBarDistance",     Spatium(.4) },

      { Sid::dividerLeft,             "dividerLeft",             QVariant(false) },
      { Sid::dividerLeftSym,          "dividerLeftSym",          QVariant(QString("systemDivider")) },
      { Sid::dividerLeftX,            "dividerLeftX",            QVariant(0.0) },
      { Sid::dividerLeftY,            "dividerLeftY",            QVariant(0.0) },
      { Sid::dividerRight,            "dividerRight",            QVariant(false) },
      { Sid::dividerRightSym,         "dividerRightSym",         QVariant(QString("systemDivider")) },
      { Sid::dividerRightX,           "dividerRightX",           QVariant(0.0) },
      { Sid::dividerRightY,           "dividerRightY",           QVariant(0.0) },

      { Sid::clefLeftMargin,          "clefLeftMargin",          Spatium(0.8) },     // 0.64 (gould: <= 1)
      { Sid::keysigLeftMargin,        "keysigLeftMargin",        Spatium(0.5) },
      { Sid::ambitusMargin,           "ambitusMargin",           Spatium(0.5) },

      { Sid::timesigLeftMargin,       "timesigLeftMargin",       Spatium(0.5) },
      { Sid::timesigScale,            "timesigScale",            QVariant(QSizeF(1.0, 1.0)) },
      { Sid::clefKeyRightMargin,      "clefKeyRightMargin",      Spatium(0.8) },
      { Sid::clefKeyDistance,         "clefKeyDistance",         Spatium(1.0) },   // gould: 1 - 1.25
      { Sid::clefTimesigDistance,     "clefTimesigDistance",     Spatium(1.0) },
      { Sid::keyTimesigDistance,      "keyTimesigDistance",      Spatium(1.0) },    // gould: 1 - 1.5
      { Sid::keyBarlineDistance,      "keyTimesigDistance",      Spatium(1.0) },
      { Sid::systemHeaderDistance,    "systemHeaderDistance",    Spatium(2.5) },     // gould: 2.5
      { Sid::systemHeaderTimeSigDistance, "systemHeaderTimeSigDistance", Spatium(2.0) },  // gould: 2.0

      { Sid::clefBarlineDistance,     "clefBarlineDistance",     Spatium(0.5) },
      { Sid::timesigBarlineDistance,  "timesigBarlineDistance",  Spatium(0.5) },
      { Sid::stemWidth,               "stemWidth",               Spatium(0.13) },      // 0.09375
      { Sid::shortenStem,             "shortenStem",             QVariant(true) },
      { Sid::shortStemProgression,    "shortStemProgression",    Spatium(0.25) },
      { Sid::shortestStem,            "shortestStem",            Spatium(2.25) },
      { Sid::beginRepeatLeftMargin,   "beginRepeatLeftMargin",   Spatium(1.0) },
      { Sid::minNoteDistance,         "minNoteDistance",         Spatium(0.25) },      // 0.4
      { Sid::barNoteDistance,         "barNoteDistance",         Spatium(1.0) },     // was 1.2

      { Sid::barAccidentalDistance,   "barAccidentalDistance",   Spatium(.3)   },
      { Sid::multiMeasureRestMargin,  "multiMeasureRestMargin",  Spatium(1.2)  },
      { Sid::noteBarDistance,         "noteBarDistance",         Spatium(1.0)  },
      { Sid::measureSpacing,          "measureSpacing",          QVariant(1.2) },
      { Sid::staffLineWidth,          "staffLineWidth",          Spatium(0.08) },     // 0.09375
      { Sid::ledgerLineWidth,         "ledgerLineWidth",         Spatium(0.16) },     // 0.1875
      { Sid::ledgerLineLength,        "ledgerLineLength",        Spatium(.6)   },     // notehead width + this value
      { Sid::accidentalDistance,      "accidentalDistance",      Spatium(0.22) },
      { Sid::accidentalNoteDistance,  "accidentalNoteDistance",  Spatium(0.22) },

      { Sid::beamWidth,               "beamWidth",               Spatium(0.5)  },     // was 0.48
      { Sid::beamDistance,            "beamDistance",            QVariant(0.5) },     // 0.25sp units
      { Sid::beamMinLen,              "beamMinLen",              Spatium(1.32) },     // 1.316178 exactly notehead widthen beams
      { Sid::beamNoSlope,             "beamNoSlope",             QVariant(false) },

      { Sid::dotMag,                  "dotMag",                  QVariant(1.0) },
      { Sid::dotNoteDistance,         "dotNoteDistance",         Spatium(0.35) },
      { Sid::dotRestDistance,         "dotRestDistance",         Spatium(0.25) },
      { Sid::dotDotDistance,          "dotDotDistance",          Spatium(0.5) },
      { Sid::propertyDistanceHead,    "propertyDistanceHead",    Spatium(1.0) },
      { Sid::propertyDistanceStem,    "propertyDistanceStem",    Spatium(1.8) },
      { Sid::propertyDistance,        "propertyDistance",        Spatium(1.0) },

      { Sid::articulationMag,         "articulationMag",         QVariant(1.0) },
      { Sid::lastSystemFillLimit,     "lastSystemFillLimit",     QVariant(0.3) },

      { Sid::hairpinPlacement,        "hairpinPlacement",        int(Placement::BELOW)  },
      { Sid::hairpinPosAbove,         "hairpinPosAbove",         Spatium(-3.5) },
      { Sid::hairpinPosBelow,         "hairpinPosBelow",         Spatium(3.5) },
      { Sid::hairpinHeight,           "hairpinHeight",           Spatium(1.2) },
      { Sid::hairpinContHeight,       "hairpinContHeight",       Spatium(0.5) },
      { Sid::hairpinLineWidth,        "hairpinWidth",            Spatium(0.13) },
      { Sid::hairpinFontFace,         "hairpinFontFace",         "FreeSerif" },
      { Sid::hairpinFontSize,         "hairpinFontSize",         12.0 },
      { Sid::hairpinFontBold,         "hairpinFontBold",         false },
      { Sid::hairpinFontItalic,       "hairpinFontItalic",       true },
      { Sid::hairpinFontUnderline,    "hairpinFontUnderline",    false },
      { Sid::hairpinTextAlign,        "hairpinTextAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::pedalPlacement,          "pedalPlacement",          int(Placement::BELOW)  },
      { Sid::pedalPosAbove,           "pedalPosAbove",           Spatium(-4) },
      { Sid::pedalPosBelow,           "pedalPosBelow",           Spatium(4) },
      { Sid::pedalLineWidth,          "pedalLineWidth",          Spatium(.15) },
      { Sid::pedalLineStyle,          "pedalListStyle",          QVariant(int(Qt::SolidLine)) },
      { Sid::pedalBeginTextOffset,    "pedalBeginTextOffset",    QPointF(0.0, 0.15) },
      { Sid::pedalHookHeight,         "pedalHookHeight",         Spatium(-1.2) },
      { Sid::pedalFontFace,           "pedalFontFace",           "FreeSerif" },
      { Sid::pedalFontSize,           "pedalFontSize",           12.0 },
      { Sid::pedalFontBold,           "pedalFontBold",           false },
      { Sid::pedalFontItalic,         "pedalFontItalic",         false },
      { Sid::pedalFontUnderline,      "pedalFontUnderline",      false },
      { Sid::pedalTextAlign,          "pedalTextAlign",          QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::trillPlacement,          "trillPlacement",          int(Placement::ABOVE)  },
      { Sid::trillPosAbove,           "trillPosAbove",           Spatium(-1) },
      { Sid::trillPosBelow,           "trillPosBelow",           Spatium(1) },

      { Sid::vibratoPlacement,        "vibratoPlacement",        int(Placement::ABOVE)  },
      { Sid::vibratoPosAbove,         "vibratoPosAbove",         Spatium(-1) },
      { Sid::vibratoPosBelow,         "vibratoPosBelow",         Spatium(1) },

      { Sid::harmonyY,                "harmonyY",                Spatium(2.5) },
      { Sid::harmonyFretDist,         "harmonyFretDist",         Spatium(0.5) },
      { Sid::minHarmonyDistance,      "minHarmonyDistance",      Spatium(0.5) },
      { Sid::maxHarmonyBarDistance,   "maxHarmonyBarDistance",   Spatium(3.0) },
      { Sid::capoPosition,            "capoPosition",            QVariant(0) },
      { Sid::fretNumMag,              "fretNumMag",              QVariant(2.0) },
      { Sid::fretNumPos,              "fretNumPos",              QVariant(0) },
      { Sid::fretY,                   "fretY",                   Spatium(2.0) },
      { Sid::fretMinDistance,         "fretMinDistance",         Spatium(0.5) },
      { Sid::showPageNumber,          "showPageNumber",          QVariant(true) },
      { Sid::showPageNumberOne,       "showPageNumberOne",       QVariant(false) },

      { Sid::pageNumberOddEven,       "pageNumberOddEven",       QVariant(true) },
      { Sid::showMeasureNumber,       "showMeasureNumber",       QVariant(true) },
      { Sid::showMeasureNumberOne,    "showMeasureNumberOne",    QVariant(false) },
      { Sid::measureNumberInterval,   "measureNumberInterval",   QVariant(5) },
      { Sid::measureNumberSystem,     "measureNumberSystem",     QVariant(true) },
      { Sid::measureNumberAllStaffs,  "measureNumberAllStaffs",  QVariant(false) },
      { Sid::smallNoteMag,            "smallNoteMag",            QVariant(.7) },
      { Sid::graceNoteMag,            "graceNoteMag",            QVariant(0.7) },
      { Sid::smallStaffMag,           "smallStaffMag",           QVariant(0.7) },
      { Sid::smallClefMag,            "smallClefMag",            QVariant(0.8) },

      { Sid::genClef,                 "genClef",                 QVariant(true) },
      { Sid::genKeysig,               "genKeysig",               QVariant(true) },
      { Sid::genCourtesyTimesig,      "genCourtesyTimesig",      QVariant(true) },
      { Sid::genCourtesyKeysig,       "genCourtesyKeysig",       QVariant(true) },
      { Sid::genCourtesyClef,         "genCourtesyClef",         QVariant(true) },
      { Sid::swingRatio,              "swingRatio",              QVariant(60)   },
      { Sid::swingUnit,               "swingUnit",               QVariant(QString("")) },
      { Sid::useStandardNoteNames,    "useStandardNoteNames",    QVariant(true) },
      { Sid::useGermanNoteNames,      "useGermanNoteNames",      QVariant(false) },
      { Sid::useFullGermanNoteNames,  "useFullGermanNoteNames",  QVariant(false) },

      { Sid::useSolfeggioNoteNames,   "useSolfeggioNoteNames",   QVariant(false) },
      { Sid::useFrenchNoteNames,      "useFrenchNoteNames",      QVariant(false) },
      { Sid::automaticCapitalization, "automaticCapitalization", QVariant(true) },
      { Sid::lowerCaseMinorChords,    "lowerCaseMinorChords",    QVariant(false) },
      { Sid::lowerCaseBassNotes,      "lowerCaseBassNotes",      QVariant(false) },
      { Sid::allCapsNoteNames,        "allCapsNoteNames",        QVariant(false) },
      { Sid::chordStyle,              "chordStyle",              QVariant(QString("std")) },
      { Sid::chordsXmlFile,           "chordsXmlFile",           QVariant(false) },
      { Sid::chordDescriptionFile,    "chordDescriptionFile",    QVariant(QString("chords_std.xml")) },
      { Sid::concertPitch,            "concertPitch",            QVariant(false) },

      { Sid::createMultiMeasureRests, "createMultiMeasureRests", QVariant(false) },
      { Sid::minEmptyMeasures,        "minEmptyMeasures",        QVariant(2) },
      { Sid::minMMRestWidth,          "minMMRestWidth",          Spatium(4) },
      { Sid::hideEmptyStaves,         "hideEmptyStaves",         QVariant(false) },
      { Sid::dontHideStavesInFirstSystem,
                                 "dontHidStavesInFirstSystm",         QVariant(true) },
      { Sid::hideInstrumentNameIfOneInstrument,
                                 "hideInstrumentNameIfOneInstrument", QVariant(true) },
      { Sid::gateTime,                "gateTime",                QVariant(100) },
      { Sid::tenutoGateTime,          "tenutoGateTime",          QVariant(100) },
      { Sid::staccatoGateTime,        "staccatoGateTime",        QVariant(50) },
      { Sid::slurGateTime,            "slurGateTime",            QVariant(100) },

      { Sid::ArpeggioNoteDistance,    "ArpeggioNoteDistance",    Spatium(.5) },
      { Sid::ArpeggioLineWidth,       "ArpeggioLineWidth",       Spatium(.18) },
      { Sid::ArpeggioHookLen,         "ArpeggioHookLen",         Spatium(.8) },
      { Sid::ArpeggioHiddenInStdIfTab,"ArpeggioHiddenInStdIfTab",QVariant(false)},
      { Sid::SlurEndWidth,            "slurEndWidth",            Spatium(.07) },
      { Sid::SlurMidWidth,            "slurMidWidth",            Spatium(.15) },
      { Sid::SlurDottedWidth,         "slurDottedWidth",         Spatium(.10)  },
      { Sid::MinTieLength,            "minTieLength",            Spatium(1.0) },
      { Sid::SlurMinDistance,         "slurMinDistance",         Spatium(0.5) },
      { Sid::SectionPause,            "sectionPause",            QVariant(qreal(3.0)) },
      { Sid::MusicalSymbolFont,       "musicalSymbolFont",       QVariant(QString("Emmentaler")) },
      { Sid::MusicalTextFont,         "musicalTextFont",         QVariant(QString("MScore Text")) },

      { Sid::showHeader,              "showHeader",              QVariant(false) },
      { Sid::headerFirstPage,         "headerFirstPage",         QVariant(false) },
      { Sid::headerOddEven,           "headerOddEven",           QVariant(true) },
      { Sid::evenHeaderL,             "evenHeaderL",             QVariant(QString()) },
      { Sid::evenHeaderC,             "evenHeaderC",             QVariant(QString()) },
      { Sid::evenHeaderR,             "evenHeaderR",             QVariant(QString()) },
      { Sid::oddHeaderL,              "oddHeaderL",              QVariant(QString()) },
      { Sid::oddHeaderC,              "oddHeaderC",              QVariant(QString()) },
      { Sid::oddHeaderR,              "oddHeaderR",              QVariant(QString()) },
      { Sid::showFooter,              "showFooter",              QVariant(true) },

      { Sid::footerFirstPage,         "footerFirstPage",         QVariant(true) },
      { Sid::footerOddEven,           "footerOddEven",           QVariant(true) },
      { Sid::evenFooterL,             "evenFooterL",             QVariant(QString("$p")) },
      { Sid::evenFooterC,             "evenFooterC",             QVariant(QString("$:copyright:")) },
      { Sid::evenFooterR,             "evenFooterR",             QVariant(QString()) },
      { Sid::oddFooterL,              "oddFooterL",              QVariant(QString()) },
      { Sid::oddFooterC,              "oddFooterC",              QVariant(QString("$:copyright:")) },
      { Sid::oddFooterR,              "oddFooterR",              QVariant(QString("$p")) },

      { Sid::voltaY,                  "voltaY",                  Spatium(-3.0) },
      { Sid::voltaHook,               "voltaHook",               Spatium(1.9) },
      { Sid::voltaLineWidth,          "voltaLineWidth",          Spatium(.1) },
      { Sid::voltaLineStyle,          "voltaLineStyle",          QVariant(int(Qt::SolidLine)) },
      { Sid::voltaFontFace,           "voltaFontFace",           "FreeSerif" },
      { Sid::voltaFontSize,           "voltaFontSize",           11.0 },
      { Sid::voltaFontBold,           "voltaFontBold",           true },
      { Sid::voltaFontItalic,         "voltaFontItalic",         false },
      { Sid::voltaFontUnderline,      "voltaFontUnderline",      false },
      { Sid::voltaAlign,              "voltaAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::voltaOffset,             "voltaOffset",             QPointF(0.5, 1.9) },

      { Sid::ottavaPlacement,         "ottavaPlacement",         int(Placement::ABOVE)  },
      { Sid::ottavaPosAbove,          "ottavaPosAbove",          Spatium(-3.0) },
      { Sid::ottavaPosBelow,          "ottavaPosBelow",          Spatium(3.0) },
      { Sid::ottavaHook,              "ottavaHook",              Spatium(1.9) },
      { Sid::ottavaLineWidth,         "ottavaLineWidth",         Spatium(.1) },
      { Sid::ottavaLineStyle,         "ottavaLineStyle",         QVariant(int(Qt::DashLine)) },
      { Sid::ottavaNumbersOnly,       "ottavaNumbersOnly",       true },
      { Sid::ottavaFontFace,          "ottavaFontFace",          "FreeSerif" },
      { Sid::ottavaFontSize,          "ottavaFontSize",          12.0 },
      { Sid::ottavaFontBold,          "ottavaFontBold",          false },
      { Sid::ottavaFontItalic,        "ottavaFontItalic",        false },
      { Sid::ottavaFontUnderline,     "ottavaFontUnderline",     false },
      { Sid::ottavaTextAlign,         "ottavaTextAlign",         QVariant::fromValue(Align::LEFT | Align::VCENTER) },

      { Sid::tabClef,                 "tabClef",                 QVariant(int(ClefType::TAB)) },

      { Sid::tremoloWidth,            "tremoloWidth",            Spatium(1.2) },  // tremolo stroke width: notehead width
      { Sid::tremoloBoxHeight,        "tremoloBoxHeight",        Spatium(0.65) },
      { Sid::tremoloStrokeWidth,      "tremoloLineWidth",        Spatium(0.5) },  // was 0.35
      { Sid::tremoloDistance,         "tremoloDistance",         Spatium(0.8) },

      { Sid::linearStretch,           "linearStretch",           QVariant(qreal(1.5)) },
      { Sid::crossMeasureValues,      "crossMeasureValues",      QVariant(false) },
      { Sid::keySigNaturals,          "keySigNaturals",          QVariant(int(KeySigNatural::NONE)) },

      { Sid::tupletMaxSlope,          "tupletMaxSlope",          QVariant(qreal(0.5)) },
      { Sid::tupletOufOfStaff,        "tupletOufOfStaff",        QVariant(true) },
      { Sid::tupletVHeadDistance,     "tupletVHeadDistance",     Spatium(.5) },
      { Sid::tupletVStemDistance,     "tupletVStemDistance",     Spatium(.25) },
      { Sid::tupletStemLeftDistance,  "tupletStemLeftDistance",  Spatium(.5) },
      { Sid::tupletStemRightDistance, "tupletStemRightDistance", Spatium(.5) },
      { Sid::tupletNoteLeftDistance,  "tupletNoteLeftDistance",  Spatium(0.0) },
      { Sid::tupletNoteRightDistance, "tupletNoteRightDistance", Spatium(0.0) },
      { Sid::tupletBracketWidth,      "tupletBracketWidth",      Spatium(0.1) },
      { Sid::tupletDirection,         "tupletDirection",         QVariant::fromValue<Direction>(Direction::AUTO) },
      { Sid::tupletNumberType,        "tupletNumberType",        int(TupletNumberType::SHOW_NUMBER) },
      { Sid::tupletBracketType,       "tupletBracketType",       int(TupletBracketType::AUTO_BRACKET) },
      { Sid::tupletFontFace,          "tupletFontFace",          "FreeSerif" },
      { Sid::tupletFontSize,          "tupletFontSize",          10.0 },
      { Sid::tupletFontBold,          "tupletFontBold",          false },
      { Sid::tupletFontItalic,        "tupletFontItalic",        true },
      { Sid::tupletFontUnderline,     "tupletFontUnderline",     false },
      { Sid::tupletAlign,             "tupletAlign",             QVariant::fromValue(Align::CENTER) },

      { Sid::barreLineWidth,          "barreLineWidth",          QVariant(1.0) },
      { Sid::fretMag,                 "fretMag",                 QVariant(1.0) },
      { Sid::scaleBarlines,           "scaleBarlines",           QVariant(true) },
      { Sid::barGraceDistance,        "barGraceDistance",        Spatium(.6) },
      { Sid::minVerticalDistance,     "minVerticalDistance",     Spatium(0.5) },
      { Sid::ornamentStyle,           "ornamentStyle",           int(MScore::OrnamentStyle::DEFAULT) },
      { Sid::spatium,                 "Spatium",                 SPATIUM20 },

      { Sid::autoplaceHairpinDynamicsDistance, "autoplaceHairpinDynamicsDistance", Spatium(0.5) },

      { Sid::dynamicsPlacement,       "dynamicsPlacement",       int(Placement::BELOW)  },
      { Sid::dynamicsPosAbove,        "dynamicsPosAbove",        Spatium(-2.0) },
      { Sid::dynamicsPosBelow,        "dynamicsPosBelow",        Spatium(1.0) },

      { Sid::dynamicsMinDistance,         "dynamicsMinDistance",               Spatium(0.5) },
      { Sid::autoplaceVerticalAlignRange, "autoplaceVerticalAlignRange",     int(VerticalAlignRange::SYSTEM) },

      { Sid::textLinePlacement,         "textLinePlacement",         int(Placement::ABOVE)  },
      { Sid::textLinePosAbove,          "textLinePosAbove",          Spatium(-3.5) },
      { Sid::textLinePosBelow,          "textLinePosBelow",          Spatium(3.5) },

      { Sid::tremoloBarLineWidth,       "tremoloBarLineWidth",       Spatium(0.1) },
      { Sid::jumpPosAbove,              "jumpPosAbove",              Spatium(-2.0) },
      { Sid::markerPosAbove,            "markerPosAbove",            Spatium(-2.0) },

//====

      { Sid::defaultFontFace,               "defaultFontFace",               "FreeSerif" },
      { Sid::defaultFontSize,               "defaultFontSize",               10.0  },
      { Sid::defaultFontSpatiumDependent,   "defaultFontSpatiumDependent",   true  },
      { Sid::defaultFontBold,               "defaultFontBold",               false },
      { Sid::defaultFontItalic,             "defaultFontItalic",             false },
      { Sid::defaultFontUnderline,          "defaultFontUnderline",          false },
      { Sid::defaultAlign,                  "defaultAlign",                  QVariant::fromValue(Align::LEFT) },
      { Sid::defaultFrame,                  "defaultFrame",                  false },
      { Sid::defaultFrameSquare,            "defaultFrameSquare",            false },
      { Sid::defaultFrameCircle,            "defaultFrameCircle",            false },
      { Sid::defaultFramePadding,           "defaultFramePadding",           0.2 },
      { Sid::defaultFrameWidth,             "defaultFrameWidth",             0.1 },
      { Sid::defaultFrameRound,             "defaultFrameRound",             0 },
      { Sid::defaultFrameFgColor,           "defaultFrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::defaultFrameBgColor,           "defaultFrameBgColor",           QColor(255, 255, 255, 0) },
      { Sid::defaultOffset,                 "defaultOffset",                 QPointF() },
      { Sid::defaultOffsetType,             "defaultOffsetType",             int(OffsetType::SPATIUM)   },
      { Sid::defaultSystemFlag,             "defaultSystemFlag",             false },
      { Sid::defaultText,                   "defaultText",                   QString("")    },

      { Sid::titleFontFace,                 "titleFontFace",                 "FreeSerif" },
      { Sid::titleFontSize,                 "titleFontSize",                 24.0 },
      { Sid::titleFontSpatiumDependent,     "titleFontSpatiumDependent",     false  },
      { Sid::titleFontBold,                 "titleFontBold",                 false },
      { Sid::titleFontItalic,               "titleFontItalic",               false },
      { Sid::titleFontUnderline,            "titleFontUnderline",            false },
      { Sid::titleAlign,                    "titleAlign",                    QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { Sid::titleOffset,                   "titleOffset",                   QPointF() },
      { Sid::titleOffsetType,               "titleOffsetType",               int(OffsetType::ABS)   },

      { Sid::subTitleFontFace,              "subTitleFontFace",              "FreeSerif" },
      { Sid::subTitleFontSize,              "subTitleFontSize",              14.0 },
      { Sid::subTitleFontSpatiumDependent,  "subTitleFontSpatiumDependent",  false  },
      { Sid::subTitleFontBold,              "subTitleFontBold",              false },
      { Sid::subTitleFontItalic,            "subTtitleFontItalic",           false },
      { Sid::subTitleFontUnderline,         "subTitleFontUnderline",         false },
      { Sid::subTitleAlign,                 "subTitleAlign",                 QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { Sid::subTitleOffset,                "subTitleOffset",                QPointF(0.0, MM(10.0)) },
      { Sid::subTitleOffsetType,            "subTitleOffsetType",            int(OffsetType::ABS)   },

      { Sid::composerFontFace,              "composerFontFace",              "FreeSerif" },
      { Sid::composerFontSize,              "composerFontSize",              12.0 },
      { Sid::composerFontSpatiumDependent,  "composerFontSpatiumDependent",  false  },
      { Sid::composerFontBold,              "composerFontBold",              false },
      { Sid::composerFontItalic,            "composerFontItalic",            false },
      { Sid::composerFontUnderline,         "composerFontUnderline",         false },
      { Sid::composerAlign,                 "composerAlign",                 QVariant::fromValue(Align::RIGHT | Align::BOTTOM) },
      { Sid::composerOffset,                "composerOffset",                QPointF() },
      { Sid::composerOffsetType,            "composerOffsetType",            int(OffsetType::ABS)   },

      { Sid::lyricistFontFace,              "lyricistFontFace",              "FreeSerif" },
      { Sid::lyricistFontSize,              "lyricistFontSize",              12.0 },
      { Sid::lyricistFontSpatiumDependent,  "lyricistFontSpatiumDependent",  false  },
      { Sid::lyricistFontBold,              "lyricistFontBold",              false },
      { Sid::lyricistFontItalic,            "lyricistFontItalic",            false },
      { Sid::lyricistFontUnderline,         "lyricistFontUnderline",         false },
      { Sid::lyricistAlign,                 "lyricistAlign",                 QVariant::fromValue(Align::LEFT | Align::BOTTOM) },
      { Sid::lyricistOffset,                "lyricistOffset",                QPointF() },
      { Sid::lyricistOffsetType,            "lyricistOffsetType",            int(OffsetType::ABS)   },

      { Sid::lyricsOddFontFace,             "lyricsOddFontFace",             "FreeSerif" },
      { Sid::lyricsOddFontSize,             "lyricsOddFontSize",             11.0 },
      { Sid::lyricsOddFontBold,             "lyricsOddFontBold",             false },
      { Sid::lyricsOddFontItalic,           "lyricsOddFontItalic",           false },
      { Sid::lyricsOddFontUnderline,        "lyricsOddFontUnderline",        false },
      { Sid::lyricsOddAlign,                "lyricistOddAlign",              QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::lyricsOddOffset,               "lyricistOddOffset",             QPointF(0.0, 6.0) },

      { Sid::lyricsEvenFontFace,            "lyricsEvenFontFace",            "FreeSerif" },
      { Sid::lyricsEvenFontSize,            "lyricsEvenFontSize",            11.0 },
      { Sid::lyricsEvenFontBold,            "lyricsEvenFontBold",            false },
      { Sid::lyricsEvenFontItalic,          "lyricsEvenFontItalic",          false },
      { Sid::lyricsEvenFontUnderline,       "lyricsEventFontUnderline",      false },
      { Sid::lyricsEvenAlign,               "lyricistEvenAlign",             QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::lyricsEvenOffset,              "lyricistEvenOffset",            QPointF(0.0, 6.0) },

      { Sid::fingeringFontFace,             "fingeringFontFace",             "FreeSerif" },
      { Sid::fingeringFontSize,             "fingeringFontSize",             8.0 },
      { Sid::fingeringFontBold,             "fingeringFontBold",             false },
      { Sid::fingeringFontItalic,           "fingeringFontItalic",           false },
      { Sid::fingeringFontUnderline,        "fingeringFontUnderline",        false },
      { Sid::fingeringAlign,                "fingeringAlign",                QVariant::fromValue(Align::CENTER) },
      { Sid::fingeringFrame,                "fingeringFrame",                false },
      { Sid::fingeringFrameSquare,          "fingeringFrameSquare",          false },
      { Sid::fingeringFrameCircle,          "fingeringFrameCircle",          false },
      { Sid::fingeringFramePadding,         "fingeringFramePadding",         0.2 },
      { Sid::fingeringFrameWidth,           "fingeringFrameWidth",           0.1 },
      { Sid::fingeringFrameRound,           "fingeringFrameRound",           0 },
      { Sid::fingeringFrameFgColor,         "fingeringFrameFgColor",         QColor(0, 0, 0, 255) },
      { Sid::fingeringFrameBgColor,         "fingeringFrameBgColor",         QColor(255, 255, 255, 0) },
      { Sid::fingeringOffset,               "fingeringOffset",               QPointF() },

      { Sid::lhGuitarFingeringFontFace,     "lhGuitarFingeringFontFace",     "FreeSerif" },
      { Sid::lhGuitarFingeringFontSize,     "lhGuitarFingeringFontSize",     8.0 },
      { Sid::lhGuitarFingeringFontBold,     "lhGuitarFingeringFontBold",     false },
      { Sid::lhGuitarFingeringFontItalic,   "lhGuitarFingeringFontItalic",   false },
      { Sid::lhGuitarFingeringFontUnderline,"lhGuitarFingeringFontUnderline",false },
      { Sid::lhGuitarFingeringAlign,        "lhGuitarFingeringAlign",        QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::lhGuitarFingeringFrame,        "lhGuitarFingeringFrame",        false },
      { Sid::lhGuitarFingeringFrameSquare,  "lhGuitarFingeringFrameSquare",  false },
      { Sid::lhGuitarFingeringFrameCircle,  "lhGuitarFingeringFrameCircle",  false },
      { Sid::lhGuitarFingeringFramePadding, "lhGuitarFingeringFramePadding", 0.2 },
      { Sid::lhGuitarFingeringFrameWidth,   "lhGuitarFingeringFrameWidth",   0.1 },
      { Sid::lhGuitarFingeringFrameRound,   "lhGuitarFingeringFrameRound",   0 },
      { Sid::lhGuitarFingeringFrameFgColor, "lhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::lhGuitarFingeringFrameBgColor, "lhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { Sid::lhGuitarFingeringOffset,       "lhGuitarFingeringOffset",       QPointF(-0.5, 0.0) },

      { Sid::rhGuitarFingeringFontFace,     "rhGuitarFingeringFontFace",     "FreeSerif" },
      { Sid::rhGuitarFingeringFontSize,     "rhGuitarFingeringFontSize",     8.0 },
      { Sid::rhGuitarFingeringFontBold,     "rhGuitarFingeringFontBold",     false },
      { Sid::rhGuitarFingeringFontItalic,   "rhGuitarFingeringFontItalic",   false },
      { Sid::rhGuitarFingeringFontUnderline,"rhGuitarFingeringFontUnderline",false },
      { Sid::rhGuitarFingeringAlign,        "rhGuitarFingeringAlign",        QVariant::fromValue(Align::CENTER) },
      { Sid::rhGuitarFingeringFrame,        "rhGuitarFingeringFrame",        false },
      { Sid::rhGuitarFingeringFrameSquare,  "rhGuitarFingeringFrameSquare",  false },
      { Sid::rhGuitarFingeringFrameCircle,  "rhGuitarFingeringFrameCircle",  false },
      { Sid::rhGuitarFingeringFramePadding, "rhGuitarFingeringFramePadding", 0.2 },
      { Sid::rhGuitarFingeringFrameWidth,   "rhGuitarFingeringFrameWidth",   0.1 },
      { Sid::rhGuitarFingeringFrameRound,   "rhGuitarFingeringFrameRound",   0 },
      { Sid::rhGuitarFingeringFrameFgColor, "rhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::rhGuitarFingeringFrameBgColor, "rhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { Sid::rhGuitarFingeringOffset,       "rhGuitarFingeringOffset",       QPointF() },

      { Sid::stringNumberFontFace,          "stringNumberFontFace",          "FreeSerif" },
      { Sid::stringNumberFontSize,          "stringNumberFontSize",          8.0 },
      { Sid::stringNumberFontBold,          "stringNumberFontBold",          false },
      { Sid::stringNumberFontItalic,        "stringNumberFontItalic",        false },
      { Sid::stringNumberFontUnderline,     "stringNumberFontUnderline",     false },
      { Sid::stringNumberAlign,             "stringNumberAlign",             QVariant::fromValue(Align::CENTER) },
      { Sid::stringNumberFrame,             "stringNumberFrame",             true },
      { Sid::stringNumberFrameSquare,       "stringNumberFrameSquare",       false },
      { Sid::stringNumberFrameCircle,       "stringNumberFrameCircle",       true },
      { Sid::stringNumberFramePadding,      "stringNumberFramePadding",      0.2 },
      { Sid::stringNumberFrameWidth,        "stringNumberFrameWidth",        0.1 },
      { Sid::stringNumberFrameRound,        "stringNumberFrameRound",        0 },
      { Sid::stringNumberFrameFgColor,      "stringNumberFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::stringNumberFrameBgColor,      "stringNumberFrameBgColor",      QColor(255, 255, 255, 0) },
      { Sid::stringNumberOffset,            "stringNumberOffset",            QPointF(0.0, -2.0) },

      { Sid::longInstrumentFontFace,        "longInstrumentFontFace",       "FreeSerif" },
      { Sid::longInstrumentFontSize,        "longInstrumentFontSize",       12.0 },
      { Sid::longInstrumentFontBold,        "longInstrumentFontBold",       false },
      { Sid::longInstrumentFontItalic,      "longInstrumentFontItalic",     false },
      { Sid::longInstrumentFontUnderline,   "longInstrumentFontUnderline",  false },
      { Sid::longInstrumentAlign,           "longInstrumentAlign",          QVariant::fromValue(Align::RIGHT | Align::VCENTER) },

      { Sid::shortInstrumentFontFace,       "shortInstrumentFontFace",      "FreeSerif" },
      { Sid::shortInstrumentFontSize,       "shortInstrumentFontSize",      12.0 },
      { Sid::shortInstrumentFontBold,       "shortInstrumentFontBold",      false },
      { Sid::shortInstrumentFontItalic,     "shortInstrumentFontItalic",    false },
      { Sid::shortInstrumentFontUnderline,  "shortInstrumentFontUnderline", false },
      { Sid::shortInstrumentAlign,          "shortInstrumentAlign",         QVariant::fromValue(Align::RIGHT | Align::VCENTER) },

      { Sid::partInstrumentFontFace,        "partInstrumentFontFace",       "FreeSerif" },
      { Sid::partInstrumentFontSize,        "partInstrumentFontSize",       18.0 },
      { Sid::partInstrumentFontBold,        "partInstrumentFontBold",       false },
      { Sid::partInstrumentFontItalic,      "partInstrumentFontItalic",     false },
      { Sid::partInstrumentFontUnderline,   "partInstrumentFontUnderline",  false },

      { Sid::dynamicsFontFace,              "dynamicsFontFace",             "FreeSerif" },
      { Sid::dynamicsFontSize,              "dynamicsFontSize",             12.0 },
      { Sid::dynamicsFontBold,              "dynamicsFontBold",             false },
      { Sid::dynamicsFontItalic,            "dynamicsFontItalic",           true },
      { Sid::dynamicsFontUnderline,         "dynamicsFontUnderline",        false },
      { Sid::dynamicsAlign,                 "dynamicsAlign",                QVariant::fromValue(Align::HCENTER | Align::BASELINE) },

      { Sid::expressionFontFace,            "expressionFontFace",           "FreeSerif" },
      { Sid::expressionFontSize,            "expressionFontSize",           11.0 },
      { Sid::expressionFontBold,            "expressionFontBold",           false },
      { Sid::expressionFontItalic,          "expressionFontItalic",         true },
      { Sid::expressionFontUnderline,       "expressionFontUnderline",      false },
      { Sid::expressionAlign,               "expressionAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::tempoFontFace,                 "tempoFontFace",                "FreeSerif" },
      { Sid::tempoFontSize,                 "tempoFontSize",                12.0 },
      { Sid::tempoFontBold,                 "tempoFontBold",                true },
      { Sid::tempoFontItalic,               "tempoFontItalic",              false },
      { Sid::tempoFontUnderline,            "tempoFontUnderline",           false },
      { Sid::tempoAlign,                    "tempoAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::tempoOffset,                   "tempoOffset",                  QPointF(0.0, -4.0) },
      { Sid::tempoSystemFlag,               "tempoSystemFlag",              true },
      { Sid::tempoPlacement,                "tempoPlacement",               int(Placement::ABOVE)  },
      { Sid::tempoPosAbove,                 "tempoPosAbove",                Spatium(-2.0) },
      { Sid::tempoPosBelow,                 "tempoPosBelow",                Spatium(3.0)  },
      { Sid::tempoMinDistance,              "tempoMinDistance",             Spatium(.5)  },

      { Sid::metronomeFontFace,             "metronomeFontFace",            "FreeSerif" },
      { Sid::metronomeFontSize,             "metronomeFontSize",            12.0 },
      { Sid::metronomeFontBold,             "metronomeFontBold",            true },
      { Sid::metronomeFontItalic,           "metronomeFontItalic",          false },
      { Sid::metronomeFontUnderline,        "metronomeFontUnderline",       false },

      { Sid::measureNumberFontFace,         "measureNumberFontFace",        "FreeSerif" },
      { Sid::measureNumberFontSize,         "measureNumberFontSize",        8.0 },
      { Sid::measureNumberFontBold,         "measureNumberFontBold",        false },
      { Sid::measureNumberFontItalic,       "measureNumberFontItalic",      false },
      { Sid::measureNumberFontUnderline,    "measureNumberFontUnderline",   false },
      { Sid::measureNumberOffset,           "measureNumberOffset",          QPointF(0.0, -2.0) },
      { Sid::measureNumberOffsetType,       "measureNumberOffsetType",      int(OffsetType::SPATIUM)   },

      { Sid::translatorFontFace,            "translatorFontFace",           "FreeSerif" },
      { Sid::translatorFontSize,            "translatorFontSize",           11.0 },
      { Sid::translatorFontBold,            "translatorFontBold",           false },
      { Sid::translatorFontItalic,          "translatorFontItalic",         false },
      { Sid::translatorFontUnderline,       "translatorFontUnderline",      false },

      { Sid::systemFontFace,                "systemFontFace",               "FreeSerif" },
      { Sid::systemFontSize,                "systemFontSize",               10.0 },
      { Sid::systemFontBold,                "systemFontBold",               false },
      { Sid::systemFontItalic,              "systemFontItalic",             false },
      { Sid::systemFontUnderline,           "systemFontUnderline",          false },
      { Sid::systemOffset,                  "systemOffset",                 QPointF(0.0, -4.0) },
      { Sid::systemOffsetType,              "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { Sid::systemAlign,                   "systemAlign",                  QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::staffTextFontFace,             "staffFontFace",                "FreeSerif" },
      { Sid::staffTextFontSize,             "staffFontSize",                10.0 },
      { Sid::staffTextFontBold,             "staffFontBold",                false },
      { Sid::staffTextFontItalic,           "staffFontItalic",              false },
      { Sid::staffTextFontUnderline,        "staffFontUnderline",           false },
      { Sid::staffTextAlign,                "staffAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::staffTextOffset,               "staffOffset",                  QPointF(0.0, -4.0) },
      { Sid::staffTextOffsetType,           "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { Sid::staffTextPlacement,            "staffTextPlacement",           int(Placement::ABOVE) },
      { Sid::staffTextPosAbove,             "staffTextPosAbove",            Spatium(-2.0) },
      { Sid::staffTextPosBelow,             "staffTextPosBelow",            Spatium(3.5)  },
      { Sid::staffTextMinDistance,          "staffTextMinDistance",         Spatium(0.5)  },

      { Sid::chordSymbolFontFace,           "chordSymbolFontFace",          "FreeSerif" },
      { Sid::chordSymbolFontSize,           "chordSymbolFontSize",          12.0 },
      { Sid::chordSymbolFontBold,           "chordSymbolFontBold",          false },
      { Sid::chordSymbolFontItalic,         "chordSymbolFontItalic",        false },
      { Sid::chordSymbolFontUnderline,      "chordSymbolFontUnderline",     false },
      { Sid::chordSymbolAlign,              "chordSymbolAlign",             QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::rehearsalMarkFontFace,         "rehearsalMarkFontFace",        "FreeSerif" },
      { Sid::rehearsalMarkFontSize,         "rehearsalMarkFontSize",        14.0 },
      { Sid::rehearsalMarkFontBold,         "rehearsalMarkFontBold",        true },
      { Sid::rehearsalMarkFontItalic,       "rehearsalMarkFontItalic",      false },
      { Sid::rehearsalMarkFontUnderline,    "rehearsalMarkFontUnderline",   false },
      { Sid::rehearsalMarkAlign,            "rehearsalMarkAlign",           QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::rehearsalMarkFrame,            "rehearsalMarkFrame",           true  },
      { Sid::rehearsalMarkFrameSquare,      "rehearsalMarkFrameSquare",     false },
      { Sid::rehearsalMarkFrameCircle,      "rehearsalMarkFrameCircle",     false },
      { Sid::rehearsalMarkFramePadding,     "rehearsalMarkFramePadding",    0.5 },
      { Sid::rehearsalMarkFrameWidth,       "rehearsalMarkFrameWidth",      0.2 },
      { Sid::rehearsalMarkFrameRound,       "rehearsalMarkFrameRound",      20 },
      { Sid::rehearsalMarkFrameFgColor,     "rehearsalMarkFrameFgColor",    QColor(0, 0, 0, 255) },
      { Sid::rehearsalMarkFrameBgColor,     "rehearsalMarkFrameBgColor",    QColor(255, 255, 255, 0) },
      { Sid::rehearsalMarkPlacement,        "rehearsalMarkPlacement",       int(Placement::ABOVE) },
      { Sid::rehearsalMarkPosAbove,         "rehearsalMarkPosAbove",        Spatium(-3.0) },
      { Sid::rehearsalMarkPosBelow,         "rehearsalMarkPosBelow",        Spatium(4.0) },
      { Sid::rehearsalMarkMinDistance,      "rehearsalMarkMinDistance",     Spatium(0.5) },

      { Sid::repeatLeftFontFace,            "repeatLeftFontFace",           "FreeSerif" },
      { Sid::repeatLeftFontSize,            "repeatLeftFontSize",           20.0 },
      { Sid::repeatLeftFontBold,            "repeatLeftFontBold",           false },
      { Sid::repeatLeftFontItalic,          "repeatLeftFontItalic",         false },
      { Sid::repeatLeftFontUnderline,       "repeatLeftFontUnderline",      false },
      { Sid::repeatLeftAlign,               "repeatLeftAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::repeatLeftPlacement,           "repeatLeftPlacement",          int(Placement::ABOVE) },

      { Sid::repeatRightFontFace,           "repeatRightFontFace",          "FreeSerif" },
      { Sid::repeatRightFontSize,           "repeatRightFontSize",          12.0 },
      { Sid::repeatRightFontBold,           "repeatRightFontBold",          false },
      { Sid::repeatRightFontItalic,         "repeatRightFontItalic",        false },
      { Sid::repeatRightFontUnderline,      "repeatRightFontUnderline",     false },
      { Sid::repeatRightAlign,              "repeatRightAlign",             QVariant::fromValue(Align::RIGHT | Align::BASELINE) },
      { Sid::repeatRightPlacement,          "repeatLeftPlacement",          int(Placement::ABOVE) },

      { Sid::frameFontFace,                 "frameFontFace",                "FreeSerif" },
      { Sid::frameFontSize,                 "frameFontSize",                12.0 },
      { Sid::frameFontBold,                 "frameFontBold",                false },
      { Sid::frameFontItalic,               "frameFontItalic",              false },
      { Sid::frameFontUnderline,            "frameFontUnderline",           false },
      { Sid::frameAlign,                    "frameAlign",                   QVariant::fromValue(Align::LEFT) },

      { Sid::textLineFontFace,              "textLineFontFace",             "FreeSerif" },
      { Sid::textLineFontSize,              "textLineFontSize",             12.0 },
      { Sid::textLineFontBold,              "textLineFontBold",             false },
      { Sid::textLineFontItalic,            "textLineFontItalic",           false },
      { Sid::textLineFontUnderline,         "textLineFontUnderline",        false },

      { Sid::glissandoFontFace,             "glissandoFontFace",            "FreeSerif" },
      { Sid::glissandoFontSize,             "glissandoFontSize",            QVariant(8.0) },
      { Sid::glissandoFontBold,             "glissandoFontBold",            false },
      { Sid::glissandoFontItalic,           "glissandoFontItalic",          true },
      { Sid::glissandoFontUnderline,        "glissandoFontUnderline",       false },
      { Sid::glissandoLineWidth,            "glissandoLineWidth",           Spatium(0.15) },
      { Sid::glissandoText,                 "glissandoText",                QString("gliss.") },

      { Sid::bendFontFace,                  "bendFontFace",                 "FreeSerif" },
      { Sid::bendFontSize,                  "bendFontSize",                 8.0 },
      { Sid::bendFontBold,                  "bendFontBold",                 false },
      { Sid::bendFontItalic,                "bendFontItalic",               false },
      { Sid::bendFontUnderline,             "bendFontUnderline",            false },
      { Sid::bendLineWidth,                 "bendLineWidth",                Spatium(0.15) },
      { Sid::bendArrowWidth,                "bendArrowWidth",               Spatium(.5) },

      { Sid::headerFontFace,                "headerFontFace",               "FreeSerif" },
      { Sid::headerFontSize,                "headerFontSize",               8.0 },
      { Sid::headerFontBold,                "headerFontBold",               false },
      { Sid::headerFontItalic,              "headerFontItalic",             false },
      { Sid::headerFontUnderline,           "headerFontUnderline",          false },

      { Sid::footerFontFace,                "footerFontFace",               "FreeSerif" },
      { Sid::footerFontSize,                "footerFontSize",               8.0 },
      { Sid::footerFontBold,                "footerFontBold",               false },
      { Sid::footerFontItalic,              "footerFontItalic",             false },
      { Sid::footerFontUnderline,           "footerFontUnderline",          false },

      { Sid::instrumentChangeFontFace,      "instrumentChangeFontFace",     "FreeSerif" },
      { Sid::instrumentChangeFontSize,      "instrumentChangeFontSize",     12.0 },
      { Sid::instrumentChangeFontBold,      "instrumentChangeFontBold",     true },
      { Sid::instrumentChangeFontItalic,    "instrumentChangeFontItalic",   false },
      { Sid::instrumentChangeFontUnderline, "instrumentChangeFontUnderline",false },
      { Sid::instrumentChangeAlign,         "instrumentChangeAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::instrumentChangeOffset,        "instrumentChangeOffset",       QPointF(0, -3.0) },

      { Sid::figuredBassFontFace,           "figuredBassFontFace",          "MScoreBC" },
      { Sid::figuredBassFontSize,           "figuredBassFontSize",          8.0 },
      { Sid::figuredBassFontBold,           "figuredBassFontBold",          false },
      { Sid::figuredBassFontItalic,         "figuredBassFontItalic",        false },
      { Sid::figuredBassFontUnderline,      "figuredBassFontUnderline",     false },

      { Sid::user1FontFace,                 "user1FontFace",                "FreeSerif" },
      { Sid::user1FontSize,                 "user1FontSize",                10.0 },
      { Sid::user1FontBold,                 "user1FontBold",                false },
      { Sid::user1FontItalic,               "user1FontItalic",              false },
      { Sid::user1FontUnderline,            "user1FontUnderline",           false },

      { Sid::user2FontFace,                 "user2FontFace",                "FreeSerif" },
      { Sid::user2FontSize,                 "user2FontSize",                10.0 },
      { Sid::user2FontBold,                 "user2FontBold",                false },
      { Sid::user2FontItalic,               "user2FontItalic",              false },
      { Sid::user2FontUnderline,            "user2FontUnderline",           false },

      { Sid::letRingFontFace,               "letRingFontFace",              "FreeSerif" },
      { Sid::letRingFontSize,               "letRingFontSize",              10.0 },
      { Sid::letRingFontBold,               "letRingFontBold",              false },
      { Sid::letRingFontItalic,             "letRingFontItalic",            true },
      { Sid::letRingFontUnderline,          "letRingFontUnderline",         false },
      { Sid::letRingTextAlign,              "letRingTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::letRingHookHeight,             "letRingHookHeight",            Spatium(0.6) },
      { Sid::letRingPlacement,              "letRingPlacement",             int(Placement::BELOW)  },
      { Sid::letRingPosAbove,               "letRingPosAbove",              Spatium(-4.0) },
      { Sid::letRingPosBelow,               "letRingPosBelow",              Spatium(4.0)  },
      { Sid::letRingLineWidth,              "letRingLineWidth",             Spatium(0.15) },
      { Sid::letRingLineStyle,              "letRingLineStyle",             QVariant(int(Qt::DashLine)) },
      { Sid::letRingBeginTextOffset,        "letRingBeginTextOffset",       QPointF(0.0, 0.15) },
      { Sid::letRingText,                   "letRingText",                  "let ring" },

      { Sid::palmMuteFontFace,              "palmMuteFontFace",              "FreeSerif" },
      { Sid::palmMuteFontSize,              "palmMuteFontSize",              10.0 },
      { Sid::palmMuteFontBold,              "palmMuteFontBold",              false },
      { Sid::palmMuteFontItalic,            "palmMuteFontItalic",            true },
      { Sid::palmMuteFontUnderline,         "palmMuteFontUnderline",         false },
      { Sid::palmMuteTextAlign,             "palmMuteTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::palmMuteHookHeight,            "palmMuteHookHeight",            Spatium(0.6) },
      { Sid::palmMutePlacement,             "palmMutePlacement",             int(Placement::BELOW)  },
      { Sid::palmMutePosAbove,              "palmMutePosAbove",              Spatium(-4.0) },
      { Sid::palmMutePosBelow,              "palmMutePosBelow",              Spatium(4.0)  },
      { Sid::palmMuteLineWidth,             "palmMuteLineWidth",             Spatium(0.15) },
      { Sid::palmMuteLineStyle,             "palmMuteLineStyle",             QVariant(int(Qt::DashLine)) },
      { Sid::palmMuteBeginTextOffset,       "palmMuteBeginTextOffset",       QPointF(0.0, 0.15) },
      { Sid::palmMuteText,                  "palmMuteText",                  "P.M." },

      { Sid::fermataPosAbove,               "fermataPosAbove",               Spatium(-1.0) },
      { Sid::fermataPosBelow,               "fermataPosBelow",               Spatium(1.0)  },
      { Sid::fermataMinDistance,            "fermataMinDistance",            Spatium(0.4)  },
      };
#undef MM

MStyle  MScore::_baseStyle;
MStyle  MScore::_defaultStyle;

//---------------------------------------------------------
//   sets of styled properties
//---------------------------------------------------------

const std::vector<StyledProperty> emptyStyle {
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> defaultStyle {
      { Sid::defaultFontFace,                    Pid::FONT_FACE              },
      { Sid::defaultFontSize,                    Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::FONT_SPATIUM_DEPENDENT },
      { Sid::defaultFontBold,                    Pid::FONT_BOLD              },
      { Sid::defaultFontItalic,                  Pid::FONT_ITALIC            },
      { Sid::defaultFontUnderline,               Pid::FONT_UNDERLINE         },
      { Sid::defaultAlign,                       Pid::ALIGN                  },
      { Sid::defaultFrame,                       Pid::FRAME                  },
      { Sid::defaultFrameSquare,                 Pid::FRAME_SQUARE           },
      { Sid::defaultFrameCircle,                 Pid::FRAME_CIRCLE           },
      { Sid::defaultFramePadding,                Pid::FRAME_PADDING          },
      { Sid::defaultFrameWidth,                  Pid::FRAME_WIDTH            },
      { Sid::defaultFrameRound,                  Pid::FRAME_ROUND            },
      { Sid::defaultFrameFgColor,                Pid::FRAME_FG_COLOR         },
      { Sid::defaultFrameBgColor,                Pid::FRAME_BG_COLOR         },
      { Sid::defaultOffset,                      Pid::OFFSET                 },
      { Sid::defaultOffsetType,                  Pid::OFFSET_TYPE            },
      { Sid::defaultSystemFlag,                  Pid::SYSTEM_FLAG            },
      { Sid::defaultText,                        Pid::TEXT                   },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> titleStyle {
      { Sid::titleFontFace,                      Pid::FONT_FACE              },
      { Sid::titleFontSize,                      Pid::FONT_SIZE              },
      { Sid::titleFontSpatiumDependent,          Pid::FONT_SPATIUM_DEPENDENT },
      { Sid::titleFontBold,                      Pid::FONT_BOLD              },
      { Sid::titleFontItalic,                    Pid::FONT_ITALIC            },
      { Sid::titleFontUnderline,                 Pid::FONT_UNDERLINE         },
      { Sid::titleAlign,                         Pid::ALIGN                  },
      { Sid::titleOffset,                        Pid::OFFSET                 },
      { Sid::titleOffsetType,                    Pid::OFFSET_TYPE            },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> subTitleStyle {
      { Sid::subTitleFontFace,                   Pid::FONT_FACE              },
      { Sid::subTitleFontSize,                   Pid::FONT_SIZE              },
      { Sid::subTitleFontSpatiumDependent,       Pid::FONT_SPATIUM_DEPENDENT },
      { Sid::subTitleFontBold,                   Pid::FONT_BOLD              },
      { Sid::subTitleFontItalic,                 Pid::FONT_ITALIC            },
      { Sid::subTitleFontUnderline,              Pid::FONT_UNDERLINE         },
      { Sid::subTitleAlign,                      Pid::ALIGN                  },
      { Sid::subTitleOffset,                     Pid::OFFSET                 },
      { Sid::subTitleOffsetType,                 Pid::OFFSET_TYPE            },
      { Sid::subTitleOffset,                     Pid::OFFSET                 },
      { Sid::subTitleOffsetType,                 Pid::OFFSET_TYPE            },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> composerStyle {
      { Sid::composerFontFace,                   Pid::FONT_FACE              },
      { Sid::composerFontSize,                   Pid::FONT_SIZE              },
      { Sid::composerFontSpatiumDependent,       Pid::FONT_SPATIUM_DEPENDENT },
      { Sid::composerFontBold,                   Pid::FONT_BOLD              },
      { Sid::composerFontItalic,                 Pid::FONT_ITALIC            },
      { Sid::composerFontUnderline,              Pid::FONT_UNDERLINE         },
      { Sid::composerAlign,                      Pid::ALIGN                  },
      { Sid::composerOffset,                     Pid::OFFSET                 },
      { Sid::composerOffsetType,                 Pid::OFFSET_TYPE            },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lyricistStyle {
      { Sid::lyricistFontFace,                   Pid::FONT_FACE              },
      { Sid::lyricistFontSize,                   Pid::FONT_SIZE              },
      { Sid::lyricistFontBold,                   Pid::FONT_BOLD              },
      { Sid::lyricistFontItalic,                 Pid::FONT_ITALIC            },
      { Sid::lyricistFontUnderline,              Pid::FONT_UNDERLINE         },
      { Sid::lyricistAlign,                      Pid::ALIGN                  },
      { Sid::lyricistOffset,                     Pid::OFFSET                 },
      { Sid::lyricistOffsetType,                 Pid::OFFSET_TYPE            },
      { Sid::lyricsPlacement,                    Pid::PLACEMENT              },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lyricsOddStyle {
      { Sid::lyricsOddFontFace,                  Pid::FONT_FACE              },
      { Sid::lyricsOddFontSize,                  Pid::FONT_SIZE              },
      { Sid::lyricsOddFontBold,                  Pid::FONT_BOLD              },
      { Sid::lyricsOddFontItalic,                Pid::FONT_ITALIC            },
      { Sid::lyricsOddFontUnderline,             Pid::FONT_UNDERLINE         },
      { Sid::lyricsOddAlign,                     Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lyricsEvenStyle {
      { Sid::lyricsEvenFontFace,                 Pid::FONT_FACE              },
      { Sid::lyricsEvenFontSize,                 Pid::FONT_SIZE              },
      { Sid::lyricsEvenFontBold,                 Pid::FONT_BOLD              },
      { Sid::lyricsEvenFontItalic,               Pid::FONT_ITALIC            },
      { Sid::lyricsEvenFontUnderline,            Pid::FONT_UNDERLINE         },
      { Sid::lyricsEvenAlign,                    Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> fingeringStyle {
      { Sid::fingeringFontFace,                  Pid::FONT_FACE              },
      { Sid::fingeringFontSize,                  Pid::FONT_SIZE              },
      { Sid::fingeringFontBold,                  Pid::FONT_BOLD              },
      { Sid::fingeringFontItalic,                Pid::FONT_ITALIC            },
      { Sid::fingeringFontUnderline,             Pid::FONT_UNDERLINE         },
      { Sid::fingeringAlign,                     Pid::ALIGN                  },
      { Sid::fingeringFrame,                     Pid::FRAME                  },
      { Sid::fingeringFrameSquare,               Pid::FRAME_SQUARE           },
      { Sid::fingeringFrameCircle,               Pid::FRAME_CIRCLE           },
      { Sid::fingeringFramePadding,              Pid::FRAME_PADDING          },
      { Sid::fingeringFrameWidth,                Pid::FRAME_WIDTH            },
      { Sid::fingeringFrameRound,                Pid::FRAME_ROUND            },
      { Sid::fingeringFrameFgColor,              Pid::FRAME_FG_COLOR         },
      { Sid::fingeringFrameBgColor,              Pid::FRAME_BG_COLOR         },
      { Sid::fingeringOffset,                    Pid::OFFSET                 },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lhGuitarFingeringStyle {
      { Sid::lhGuitarFingeringFontFace,          Pid::FONT_FACE              },
      { Sid::lhGuitarFingeringFontSize,          Pid::FONT_SIZE              },
      { Sid::lhGuitarFingeringFontBold,          Pid::FONT_BOLD              },
      { Sid::lhGuitarFingeringFontItalic,        Pid::FONT_ITALIC            },
      { Sid::lhGuitarFingeringFontUnderline,     Pid::FONT_UNDERLINE         },
      { Sid::lhGuitarFingeringAlign,             Pid::ALIGN                  },
      { Sid::lhGuitarFingeringFrame,             Pid::FRAME                  },
      { Sid::lhGuitarFingeringFrameSquare,       Pid::FRAME_SQUARE           },
      { Sid::lhGuitarFingeringFrameCircle,       Pid::FRAME_CIRCLE           },
      { Sid::lhGuitarFingeringFramePadding,      Pid::FRAME_PADDING          },
      { Sid::lhGuitarFingeringFrameWidth,        Pid::FRAME_WIDTH            },
      { Sid::lhGuitarFingeringFrameRound,        Pid::FRAME_ROUND            },
      { Sid::lhGuitarFingeringFrameFgColor,      Pid::FRAME_FG_COLOR         },
      { Sid::lhGuitarFingeringFrameBgColor,      Pid::FRAME_BG_COLOR         },
      { Sid::lhGuitarFingeringOffset,            Pid::OFFSET                 },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> rhGuitarFingeringStyle {
      { Sid::rhGuitarFingeringFontFace,          Pid::FONT_FACE              },
      { Sid::rhGuitarFingeringFontSize,          Pid::FONT_SIZE              },
      { Sid::rhGuitarFingeringFontBold,          Pid::FONT_BOLD              },
      { Sid::rhGuitarFingeringFontItalic,        Pid::FONT_ITALIC            },
      { Sid::rhGuitarFingeringFontUnderline,     Pid::FONT_UNDERLINE         },
      { Sid::rhGuitarFingeringAlign,             Pid::ALIGN                  },
      { Sid::rhGuitarFingeringFrame,             Pid::FRAME                  },
      { Sid::rhGuitarFingeringFrameSquare,       Pid::FRAME_SQUARE           },
      { Sid::rhGuitarFingeringFrameCircle,       Pid::FRAME_CIRCLE           },
      { Sid::rhGuitarFingeringFramePadding,      Pid::FRAME_PADDING          },
      { Sid::rhGuitarFingeringFrameWidth,        Pid::FRAME_WIDTH            },
      { Sid::rhGuitarFingeringFrameRound,        Pid::FRAME_ROUND            },
      { Sid::rhGuitarFingeringFrameFgColor,      Pid::FRAME_FG_COLOR         },
      { Sid::rhGuitarFingeringFrameBgColor,      Pid::FRAME_BG_COLOR         },
      { Sid::rhGuitarFingeringOffset,            Pid::OFFSET                 },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> stringNumberStyle {
      { Sid::stringNumberFontFace,               Pid::FONT_FACE              },
      { Sid::stringNumberFontSize,               Pid::FONT_SIZE              },
      { Sid::stringNumberFontBold,               Pid::FONT_BOLD              },
      { Sid::stringNumberFontItalic,             Pid::FONT_ITALIC            },
      { Sid::stringNumberFontUnderline,          Pid::FONT_UNDERLINE         },
      { Sid::stringNumberAlign,                  Pid::ALIGN                  },
      { Sid::stringNumberFrame,                  Pid::FRAME                  },
      { Sid::stringNumberFrameSquare,            Pid::FRAME_SQUARE           },
      { Sid::stringNumberFrameCircle,            Pid::FRAME_CIRCLE           },
      { Sid::stringNumberFramePadding,           Pid::FRAME_PADDING          },
      { Sid::stringNumberFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::stringNumberFrameRound,             Pid::FRAME_ROUND            },
      { Sid::stringNumberFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::stringNumberFrameBgColor,           Pid::FRAME_BG_COLOR         },
      { Sid::stringNumberOffset,                 Pid::OFFSET                 },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> longInstrumentStyle {
      { Sid::longInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::longInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::longInstrumentFontBold,             Pid::FONT_BOLD              },
      { Sid::longInstrumentFontItalic,           Pid::FONT_ITALIC            },
      { Sid::longInstrumentFontUnderline,        Pid::FONT_UNDERLINE         },
      { Sid::longInstrumentAlign,                Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> shortInstrumentStyle {
      { Sid::shortInstrumentFontFace,            Pid::FONT_FACE              },
      { Sid::shortInstrumentFontSize,            Pid::FONT_SIZE              },
      { Sid::shortInstrumentFontBold,            Pid::FONT_BOLD              },
      { Sid::shortInstrumentFontItalic,          Pid::FONT_ITALIC            },
      { Sid::shortInstrumentFontUnderline,       Pid::FONT_UNDERLINE         },
      { Sid::shortInstrumentAlign,               Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> partInstrumentStyle {
      { Sid::partInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::partInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::partInstrumentFontBold,             Pid::FONT_BOLD              },
      { Sid::partInstrumentFontItalic,           Pid::FONT_ITALIC            },
      { Sid::partInstrumentFontUnderline,        Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> dynamicsStyle {
      { Sid::dynamicsFontFace,                   Pid::FONT_FACE              },
      { Sid::dynamicsFontSize,                   Pid::FONT_SIZE              },
      { Sid::dynamicsFontBold,                   Pid::FONT_BOLD              },
      { Sid::dynamicsFontItalic,                 Pid::FONT_ITALIC            },
      { Sid::dynamicsFontUnderline,              Pid::FONT_UNDERLINE         },
      { Sid::dynamicsAlign,                      Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> expressionStyle {
      { Sid::expressionFontFace,                 Pid::FONT_FACE              },
      { Sid::expressionFontSize,                 Pid::FONT_SIZE              },
      { Sid::expressionFontBold,                 Pid::FONT_BOLD              },
      { Sid::expressionFontItalic,               Pid::FONT_ITALIC            },
      { Sid::expressionFontUnderline,            Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> tempoStyle {
      { Sid::tempoFontFace,                      Pid::FONT_FACE              },
      { Sid::tempoFontSize,                      Pid::FONT_SIZE              },
      { Sid::tempoFontBold,                      Pid::FONT_BOLD              },
      { Sid::tempoFontItalic,                    Pid::FONT_ITALIC            },
      { Sid::tempoFontUnderline,                 Pid::FONT_UNDERLINE         },
      { Sid::tempoOffset,                        Pid::OFFSET                 },
      { Sid::tempoSystemFlag,                    Pid::SYSTEM_FLAG            },
      { Sid::tempoAlign,                         Pid::ALIGN                  },
      { Sid::tempoPlacement,                     Pid::PLACEMENT              },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> metronomeStyle {
      { Sid::metronomeFontFace,                  Pid::FONT_FACE              },
      { Sid::metronomeFontSize,                  Pid::FONT_SIZE              },
      { Sid::metronomeFontBold,                  Pid::FONT_BOLD              },
      { Sid::metronomeFontItalic,                Pid::FONT_ITALIC            },
      { Sid::metronomeFontUnderline,             Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> measureNumberStyle {
      { Sid::measureNumberFontFace,              Pid::FONT_FACE              },
      { Sid::measureNumberFontSize,              Pid::FONT_SIZE              },
      { Sid::measureNumberFontBold,              Pid::FONT_BOLD              },
      { Sid::measureNumberFontItalic,            Pid::FONT_ITALIC            },
      { Sid::measureNumberFontUnderline,         Pid::FONT_UNDERLINE         },
      { Sid::measureNumberOffset,                Pid::OFFSET                 },
      { Sid::measureNumberOffsetType,            Pid::OFFSET_TYPE            },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> translatorStyle {
      { Sid::translatorFontFace,                 Pid::FONT_FACE              },
      { Sid::translatorFontSize,                 Pid::FONT_SIZE              },
      { Sid::translatorFontBold,                 Pid::FONT_BOLD              },
      { Sid::translatorFontItalic,               Pid::FONT_ITALIC            },
      { Sid::translatorFontUnderline,            Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> tupletStyle {
      { Sid::tupletDirection,                    Pid::DIRECTION               },
      { Sid::tupletNumberType,                   Pid::NUMBER_TYPE             },
      { Sid::tupletBracketType,                  Pid::BRACKET_TYPE            },
      { Sid::tupletBracketWidth,                 Pid::LINE_WIDTH              },
      { Sid::tupletFontFace,                     Pid::FONT_FACE               },
      { Sid::tupletFontSize,                     Pid::FONT_SIZE               },
      { Sid::tupletFontBold,                     Pid::FONT_BOLD               },
      { Sid::tupletFontItalic,                   Pid::FONT_ITALIC             },
      { Sid::tupletFontUnderline,                Pid::FONT_UNDERLINE          },
      { Sid::tupletAlign,                        Pid::ALIGN                   },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> systemStyle {
      { Sid::systemFontFace,                     Pid::FONT_FACE              },
      { Sid::systemFontSize,                     Pid::FONT_SIZE              },
      { Sid::systemFontBold,                     Pid::FONT_BOLD              },
      { Sid::systemFontItalic,                   Pid::FONT_ITALIC            },
      { Sid::systemFontUnderline,                Pid::FONT_UNDERLINE         },
      { Sid::systemOffset,                       Pid::OFFSET                 },
      { Sid::systemOffsetType,                   Pid::OFFSET_TYPE            },
      { Sid::systemAlign,                        Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> staffStyle {
      { Sid::staffTextFontFace,                  Pid::FONT_FACE              },
      { Sid::staffTextFontSize,                  Pid::FONT_SIZE              },
      { Sid::staffTextFontBold,                  Pid::FONT_BOLD              },
      { Sid::staffTextFontItalic,                Pid::FONT_ITALIC            },
      { Sid::staffTextFontUnderline,             Pid::FONT_UNDERLINE         },
      { Sid::staffTextAlign,                     Pid::ALIGN                  },
      { Sid::staffTextOffset,                    Pid::OFFSET                 },
      { Sid::staffTextOffsetType,                Pid::OFFSET_TYPE            },
      { Sid::staffTextPlacement,                 Pid::PLACEMENT              },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> chordSymbolStyle {
      { Sid::chordSymbolFontFace,                Pid::FONT_FACE              },
      { Sid::chordSymbolFontSize,                Pid::FONT_SIZE              },
      { Sid::chordSymbolFontBold,                Pid::FONT_BOLD              },
      { Sid::chordSymbolFontItalic,              Pid::FONT_ITALIC            },
      { Sid::chordSymbolFontUnderline,           Pid::FONT_UNDERLINE         },
      { Sid::chordSymbolAlign,                   Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> rehearsalMarkStyle {
      { Sid::rehearsalMarkFontFace,              Pid::FONT_FACE              },
      { Sid::rehearsalMarkFontSize,              Pid::FONT_SIZE              },
      { Sid::rehearsalMarkFontBold,              Pid::FONT_BOLD              },
      { Sid::rehearsalMarkFontItalic,            Pid::FONT_ITALIC            },
      { Sid::rehearsalMarkFontUnderline,         Pid::FONT_UNDERLINE         },
      { Sid::rehearsalMarkAlign,                 Pid::ALIGN                  },
      { Sid::rehearsalMarkFrame,                 Pid::FRAME                  },
      { Sid::rehearsalMarkFrameSquare,           Pid::FRAME_SQUARE           },
      { Sid::rehearsalMarkFrameCircle,           Pid::FRAME_CIRCLE           },
      { Sid::rehearsalMarkFramePadding,          Pid::FRAME_PADDING          },
      { Sid::rehearsalMarkFrameWidth,            Pid::FRAME_WIDTH            },
      { Sid::rehearsalMarkFrameRound,            Pid::FRAME_ROUND            },
      { Sid::rehearsalMarkFrameFgColor,          Pid::FRAME_FG_COLOR         },
      { Sid::rehearsalMarkFrameBgColor,          Pid::FRAME_BG_COLOR         },
      { Sid::rehearsalMarkPlacement,             Pid::PLACEMENT              },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> repeatLeftStyle {
      { Sid::repeatLeftFontFace,                 Pid::FONT_FACE              },
      { Sid::repeatLeftFontSize,                 Pid::FONT_SIZE              },
      { Sid::repeatLeftFontBold,                 Pid::FONT_BOLD              },
      { Sid::repeatLeftFontItalic,               Pid::FONT_ITALIC            },
      { Sid::repeatLeftFontUnderline,            Pid::FONT_UNDERLINE         },
      { Sid::repeatLeftAlign,                    Pid::ALIGN                  },
      { Sid::repeatLeftPlacement,                Pid::PLACEMENT              },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> repeatRightStyle {
      { Sid::repeatRightFontFace,                Pid::FONT_FACE              },
      { Sid::repeatRightFontSize,                Pid::FONT_SIZE              },
      { Sid::repeatRightFontBold,                Pid::FONT_BOLD              },
      { Sid::repeatRightFontItalic,              Pid::FONT_ITALIC            },
      { Sid::repeatRightFontUnderline,           Pid::FONT_UNDERLINE         },
      { Sid::repeatRightAlign,                   Pid::ALIGN                  },
      { Sid::repeatRightPlacement,               Pid::PLACEMENT              },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> frameStyle {
      { Sid::frameFontFace,                      Pid::FONT_FACE              },
      { Sid::frameFontSize,                      Pid::FONT_SIZE              },
      { Sid::frameFontBold,                      Pid::FONT_BOLD              },
      { Sid::frameFontItalic,                    Pid::FONT_ITALIC            },
      { Sid::frameFontUnderline,                 Pid::FONT_UNDERLINE         },
      { Sid::frameAlign,                         Pid::ALIGN                  },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> textLineStyle {
      { Sid::textLineFontFace,                   Pid::BEGIN_FONT_FACE         },
      { Sid::textLineFontFace,                   Pid::CONTINUE_FONT_FACE      },
      { Sid::textLineFontFace,                   Pid::END_FONT_FACE           },
      { Sid::textLineFontSize,                   Pid::BEGIN_FONT_SIZE         },
      { Sid::textLineFontSize,                   Pid::CONTINUE_FONT_SIZE      },
      { Sid::textLineFontSize,                   Pid::END_FONT_SIZE           },
      { Sid::textLineFontBold,                   Pid::BEGIN_FONT_BOLD         },
      { Sid::textLineFontBold,                   Pid::CONTINUE_FONT_BOLD      },
      { Sid::textLineFontBold,                   Pid::END_FONT_BOLD           },
      { Sid::textLineFontItalic,                 Pid::BEGIN_FONT_ITALIC       },
      { Sid::textLineFontItalic,                 Pid::CONTINUE_FONT_ITALIC    },
      { Sid::textLineFontItalic,                 Pid::END_FONT_ITALIC         },
      { Sid::textLineFontUnderline,              Pid::BEGIN_FONT_UNDERLINE    },
      { Sid::textLineFontUnderline,              Pid::CONTINUE_FONT_UNDERLINE },
      { Sid::textLineFontUnderline,              Pid::END_FONT_UNDERLINE      },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> glissandoStyle {
      { Sid::glissandoFontFace,                  Pid::FONT_FACE               },
      { Sid::glissandoFontSize,                  Pid::FONT_SIZE               },
      { Sid::glissandoFontBold,                  Pid::FONT_BOLD               },
      { Sid::glissandoFontItalic,                Pid::FONT_ITALIC             },
      { Sid::glissandoFontUnderline,             Pid::FONT_UNDERLINE          },
      { Sid::glissandoLineWidth,                 Pid::LINE_WIDTH              },
      { Sid::glissandoText,                      Pid::GLISS_TEXT              },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> ottavaStyle {
      { Sid::ottavaNumbersOnly,                  Pid::NUMBERS_ONLY            },
      { Sid::ottavaFontFace,                     Pid::BEGIN_FONT_FACE         },
      { Sid::ottavaFontFace,                     Pid::CONTINUE_FONT_FACE      },
      { Sid::ottavaFontFace,                     Pid::END_FONT_FACE           },
      { Sid::ottavaFontSize,                     Pid::BEGIN_FONT_SIZE         },
      { Sid::ottavaFontSize,                     Pid::CONTINUE_FONT_SIZE      },
      { Sid::ottavaFontSize,                     Pid::END_FONT_SIZE           },
      { Sid::ottavaFontBold,                     Pid::BEGIN_FONT_BOLD         },
      { Sid::ottavaFontBold,                     Pid::CONTINUE_FONT_BOLD      },
      { Sid::ottavaFontBold,                     Pid::END_FONT_BOLD           },
      { Sid::ottavaFontItalic,                   Pid::BEGIN_FONT_ITALIC       },
      { Sid::ottavaFontItalic,                   Pid::CONTINUE_FONT_ITALIC    },
      { Sid::ottavaFontItalic,                   Pid::END_FONT_ITALIC         },
      { Sid::ottavaFontUnderline,                Pid::BEGIN_FONT_UNDERLINE    },
      { Sid::ottavaFontUnderline,                Pid::CONTINUE_FONT_UNDERLINE },
      { Sid::ottavaFontUnderline,                Pid::END_FONT_UNDERLINE      },
      { Sid::ottavaTextAlign,                    Pid::BEGIN_TEXT_ALIGN        },
      { Sid::ottavaTextAlign,                    Pid::CONTINUE_TEXT_ALIGN     },
      { Sid::ottavaTextAlign,                    Pid::END_TEXT_ALIGN          },
      { Sid::ottavaLineWidth,                    Pid::LINE_WIDTH              },
      { Sid::ottavaLineStyle,                    Pid::LINE_STYLE              },
      { Sid::ottavaPlacement,                    Pid::PLACEMENT               },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> voltaStyle {
      { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE         },
      { Sid::voltaFontFace,                      Pid::CONTINUE_FONT_FACE      },
      { Sid::voltaFontFace,                      Pid::END_FONT_FACE           },
      { Sid::voltaFontSize,                      Pid::BEGIN_FONT_SIZE         },
      { Sid::voltaFontSize,                      Pid::CONTINUE_FONT_SIZE      },
      { Sid::voltaFontSize,                      Pid::END_FONT_SIZE           },
      { Sid::voltaFontBold,                      Pid::BEGIN_FONT_BOLD         },
      { Sid::voltaFontBold,                      Pid::CONTINUE_FONT_BOLD      },
      { Sid::voltaFontBold,                      Pid::END_FONT_BOLD           },
      { Sid::voltaFontItalic,                    Pid::BEGIN_FONT_ITALIC       },
      { Sid::voltaFontItalic,                    Pid::CONTINUE_FONT_ITALIC    },
      { Sid::voltaFontItalic,                    Pid::END_FONT_ITALIC         },
      { Sid::voltaFontUnderline,                 Pid::BEGIN_FONT_UNDERLINE    },
      { Sid::voltaFontUnderline,                 Pid::CONTINUE_FONT_UNDERLINE },
      { Sid::voltaFontUnderline,                 Pid::END_FONT_UNDERLINE      },
      { Sid::voltaAlign,                         Pid::BEGIN_TEXT_ALIGN        },
      { Sid::voltaAlign,                         Pid::CONTINUE_TEXT_ALIGN     },
      { Sid::voltaAlign,                         Pid::END_TEXT_ALIGN          },
      { Sid::voltaOffset,                        Pid::BEGIN_TEXT_OFFSET       },
      { Sid::voltaOffset,                        Pid::CONTINUE_TEXT_OFFSET    },
      { Sid::voltaOffset,                        Pid::END_TEXT_OFFSET         },
      { Sid::voltaLineWidth,                     Pid::LINE_WIDTH              },
      { Sid::voltaLineStyle,                     Pid::LINE_STYLE              },
      { Sid::voltaHook,                          Pid::BEGIN_HOOK_HEIGHT       },
      { Sid::voltaHook,                          Pid::END_HOOK_HEIGHT         },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> pedalStyle {
      { Sid::pedalFontFace,                      Pid::BEGIN_FONT_FACE         },
      { Sid::pedalFontFace,                      Pid::CONTINUE_FONT_FACE      },
      { Sid::pedalFontFace,                      Pid::END_FONT_FACE           },
      { Sid::pedalFontSize,                      Pid::BEGIN_FONT_SIZE         },
      { Sid::pedalFontSize,                      Pid::CONTINUE_FONT_SIZE      },
      { Sid::pedalFontSize,                      Pid::END_FONT_SIZE           },
      { Sid::pedalFontBold,                      Pid::BEGIN_FONT_BOLD         },
      { Sid::pedalFontBold,                      Pid::CONTINUE_FONT_BOLD      },
      { Sid::pedalFontBold,                      Pid::END_FONT_BOLD           },
      { Sid::pedalFontItalic,                    Pid::BEGIN_FONT_ITALIC       },
      { Sid::pedalFontItalic,                    Pid::CONTINUE_FONT_ITALIC    },
      { Sid::pedalFontItalic,                    Pid::END_FONT_ITALIC         },
      { Sid::pedalFontUnderline,                 Pid::BEGIN_FONT_UNDERLINE    },
      { Sid::pedalFontUnderline,                 Pid::CONTINUE_FONT_UNDERLINE },
      { Sid::pedalFontUnderline,                 Pid::END_FONT_UNDERLINE      },
      { Sid::pedalTextAlign,                     Pid::BEGIN_TEXT_ALIGN        },
      { Sid::pedalTextAlign,                     Pid::CONTINUE_TEXT_ALIGN     },
      { Sid::pedalTextAlign,                     Pid::END_TEXT_ALIGN          },
      { Sid::pedalHookHeight,                    Pid::BEGIN_HOOK_HEIGHT       },
      { Sid::pedalHookHeight,                    Pid::END_HOOK_HEIGHT         },
      { Sid::pedalBeginTextOffset,               Pid::BEGIN_TEXT_OFFSET       },
      { Sid::pedalPlacement,                     Pid::PLACEMENT               },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> letRingStyle {
      { Sid::letRingFontFace,                      Pid::BEGIN_FONT_FACE        },
      { Sid::letRingFontFace,                      Pid::CONTINUE_FONT_FACE     },
      { Sid::letRingFontFace,                      Pid::END_FONT_FACE          },
      { Sid::letRingFontSize,                      Pid::BEGIN_FONT_SIZE        },
      { Sid::letRingFontSize,                      Pid::CONTINUE_FONT_SIZE     },
      { Sid::letRingFontSize,                      Pid::END_FONT_SIZE          },
      { Sid::letRingFontBold,                      Pid::BEGIN_FONT_BOLD        },
      { Sid::letRingFontBold,                      Pid::CONTINUE_FONT_BOLD     },
      { Sid::letRingFontBold,                      Pid::END_FONT_BOLD          },
      { Sid::letRingFontItalic,                    Pid::BEGIN_FONT_ITALIC      },
      { Sid::letRingFontItalic,                    Pid::CONTINUE_FONT_ITALIC   },
      { Sid::letRingFontItalic,                    Pid::END_FONT_ITALIC        },
      { Sid::letRingFontUnderline,                 Pid::BEGIN_FONT_UNDERLINE   },
      { Sid::letRingFontUnderline,                 Pid::CONTINUE_FONT_UNDERLINE},
      { Sid::letRingFontUnderline,                 Pid::END_FONT_UNDERLINE     },
      { Sid::letRingTextAlign,                     Pid::BEGIN_TEXT_ALIGN       },
      { Sid::letRingTextAlign,                     Pid::CONTINUE_TEXT_ALIGN    },
      { Sid::letRingTextAlign,                     Pid::END_TEXT_ALIGN         },
      { Sid::letRingHookHeight,                    Pid::BEGIN_HOOK_HEIGHT      },
      { Sid::letRingHookHeight,                    Pid::END_HOOK_HEIGHT        },
      { Sid::NOSTYLE,                              Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> palmMuteStyle {
      { Sid::palmMuteFontFace,                      Pid::BEGIN_FONT_FACE        },
      { Sid::palmMuteFontFace,                      Pid::CONTINUE_FONT_FACE     },
      { Sid::palmMuteFontFace,                      Pid::END_FONT_FACE          },
      { Sid::palmMuteFontSize,                      Pid::BEGIN_FONT_SIZE        },
      { Sid::palmMuteFontSize,                      Pid::CONTINUE_FONT_SIZE     },
      { Sid::palmMuteFontSize,                      Pid::END_FONT_SIZE          },
      { Sid::palmMuteFontBold,                      Pid::BEGIN_FONT_BOLD        },
      { Sid::palmMuteFontBold,                      Pid::CONTINUE_FONT_BOLD     },
      { Sid::palmMuteFontBold,                      Pid::END_FONT_BOLD          },
      { Sid::palmMuteFontItalic,                    Pid::BEGIN_FONT_ITALIC      },
      { Sid::palmMuteFontItalic,                    Pid::CONTINUE_FONT_ITALIC   },
      { Sid::palmMuteFontItalic,                    Pid::END_FONT_ITALIC        },
      { Sid::palmMuteFontUnderline,                 Pid::BEGIN_FONT_UNDERLINE   },
      { Sid::palmMuteFontUnderline,                 Pid::CONTINUE_FONT_UNDERLINE},
      { Sid::palmMuteFontUnderline,                 Pid::END_FONT_UNDERLINE     },
      { Sid::palmMuteTextAlign,                     Pid::BEGIN_TEXT_ALIGN       },
      { Sid::palmMuteTextAlign,                     Pid::CONTINUE_TEXT_ALIGN    },
      { Sid::palmMuteTextAlign,                     Pid::END_TEXT_ALIGN         },
      { Sid::palmMuteHookHeight,                    Pid::BEGIN_HOOK_HEIGHT      },
      { Sid::palmMuteHookHeight,                    Pid::END_HOOK_HEIGHT        },
      { Sid::NOSTYLE,                               Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> hairpinStyle {
      { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE            },
      { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE            },
      { Sid::hairpinFontBold,                    Pid::BEGIN_FONT_BOLD            },
      { Sid::hairpinFontItalic,                  Pid::BEGIN_FONT_ITALIC          },
      { Sid::hairpinFontUnderline,               Pid::BEGIN_FONT_UNDERLINE       },
      { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN           },
      { Sid::hairpinFontFace,                    Pid::CONTINUE_FONT_FACE         },
      { Sid::hairpinFontSize,                    Pid::CONTINUE_FONT_SIZE         },
      { Sid::hairpinFontBold,                    Pid::CONTINUE_FONT_BOLD         },
      { Sid::hairpinFontItalic,                  Pid::CONTINUE_FONT_ITALIC       },
      { Sid::hairpinFontUnderline,               Pid::CONTINUE_FONT_UNDERLINE    },
      { Sid::hairpinTextAlign,                   Pid::CONTINUE_TEXT_ALIGN        },
      { Sid::hairpinFontFace,                    Pid::END_FONT_FACE              },
      { Sid::hairpinFontSize,                    Pid::END_FONT_SIZE              },
      { Sid::hairpinFontBold,                    Pid::END_FONT_BOLD              },
      { Sid::hairpinFontItalic,                  Pid::END_FONT_ITALIC            },
      { Sid::hairpinFontUnderline,               Pid::END_FONT_UNDERLINE         },
      { Sid::hairpinTextAlign,                   Pid::END_TEXT_ALIGN             },
      { Sid::hairpinLineWidth,                   Pid::LINE_WIDTH                 },
      { Sid::hairpinHeight,                      Pid::HAIRPIN_HEIGHT             },
      { Sid::hairpinContHeight,                  Pid::HAIRPIN_CONT_HEIGHT        },
      { Sid::hairpinPlacement,                   Pid::PLACEMENT                  },
      { Sid::NOSTYLE,                            Pid::END                        }      // end of list marker
      };

const std::vector<StyledProperty> bendStyle {
      { Sid::bendFontFace,                       Pid::FONT_FACE              },
      { Sid::bendFontSize,                       Pid::FONT_SIZE              },
      { Sid::bendFontBold,                       Pid::FONT_BOLD              },
      { Sid::bendFontItalic,                     Pid::FONT_ITALIC            },
      { Sid::bendFontUnderline,                  Pid::FONT_UNDERLINE         },
      { Sid::bendLineWidth,                      Pid::LINE_WIDTH             },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> headerStyle {
      { Sid::headerFontFace,                     Pid::FONT_FACE              },
      { Sid::headerFontSize,                     Pid::FONT_SIZE              },
      { Sid::headerFontBold,                     Pid::FONT_BOLD              },
      { Sid::headerFontItalic,                   Pid::FONT_ITALIC            },
      { Sid::headerFontUnderline,                Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> footerStyle {
      { Sid::footerFontFace,                     Pid::FONT_FACE              },
      { Sid::footerFontSize,                     Pid::FONT_SIZE              },
      { Sid::footerFontBold,                     Pid::FONT_BOLD              },
      { Sid::footerFontItalic,                   Pid::FONT_ITALIC            },
      { Sid::footerFontUnderline,                Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> instrumentChangeStyle {
      { Sid::instrumentChangeFontFace,           Pid::FONT_FACE              },
      { Sid::instrumentChangeFontSize,           Pid::FONT_SIZE              },
      { Sid::instrumentChangeFontBold,           Pid::FONT_BOLD              },
      { Sid::instrumentChangeFontItalic,         Pid::FONT_ITALIC            },
      { Sid::instrumentChangeFontUnderline,      Pid::FONT_UNDERLINE         },
      { Sid::instrumentChangeAlign,              Pid::ALIGN                  },
      { Sid::instrumentChangeOffset,             Pid::OFFSET                 },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> figuredBassStyle {
      { Sid::figuredBassFontFace,                Pid::FONT_FACE              },
      { Sid::figuredBassFontSize,                Pid::FONT_SIZE              },
      { Sid::figuredBassFontBold,                Pid::FONT_BOLD              },
      { Sid::figuredBassFontItalic,              Pid::FONT_ITALIC            },
      { Sid::figuredBassFontUnderline,           Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> beamStyle {
      { Sid::beamNoSlope,                        Pid::BEAM_NO_SLOPE           },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> boxStyle {
      { Sid::systemFrameDistance,                Pid::TOP_GAP                 },
      { Sid::frameSystemDistance,                Pid::BOTTOM_GAP              },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> fretStyle {
      { Sid::fretNumPos,                         Pid::FRET_NUM_POS            },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> tremoloBarStyle {
      { Sid::tremoloBarLineWidth,                Pid::LINE_WIDTH              },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> timesigStyle {
      { Sid::timesigScale,                       Pid::SCALE                   },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> stemStyle {
      { Sid::stemWidth,                          Pid::LINE_WIDTH              },
      { Sid::NOSTYLE,                            Pid::END                     }      // end of list marker
      };

const std::vector<StyledProperty> user1Style {
      { Sid::user1FontFace,                      Pid::FONT_FACE              },
      { Sid::user1FontSize,                      Pid::FONT_SIZE              },
      { Sid::user1FontBold,                      Pid::FONT_BOLD              },
      { Sid::user1FontItalic,                    Pid::FONT_ITALIC            },
      { Sid::user1FontUnderline,                 Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

const std::vector<StyledProperty> user2Style {
      { Sid::user2FontFace,                      Pid::FONT_FACE              },
      { Sid::user2FontSize,                      Pid::FONT_SIZE              },
      { Sid::user2FontBold,                      Pid::FONT_BOLD              },
      { Sid::user2FontItalic,                    Pid::FONT_ITALIC            },
      { Sid::user2FontUnderline,                 Pid::FONT_UNDERLINE         },
      { Sid::NOSTYLE,                            Pid::END                    }      // end of list marker
      };

//---------------------------------------------------------
//   StyledPropertyListName
//---------------------------------------------------------

struct StyledPropertyListName {
      const char* name;
      const SubStyle* spl;
      SubStyleId ss;
      };

//---------------------------------------------------------
//   namedStyles
//    must be in sync with SubStyle enumeration
//---------------------------------------------------------

static constexpr std::array<StyledPropertyListName, int(SubStyleId::SUBSTYLES)> namedStyles { {
      { QT_TRANSLATE_NOOP("TextStyle", "empty"),                   &emptyStyle,                     SubStyleId::EMPTY   },
      { QT_TRANSLATE_NOOP("TextStyle", "default"),                 &defaultStyle,                   SubStyleId::DEFAULT },
      { QT_TRANSLATE_NOOP("TextStyle", "Title"),                   &titleStyle,                     SubStyleId::TITLE },
      { QT_TRANSLATE_NOOP("TextStyle", "Subtitle"),                &subTitleStyle,                  SubStyleId::SUBTITLE },
      { QT_TRANSLATE_NOOP("TextStyle", "Composer"),                &composerStyle,                  SubStyleId::COMPOSER },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyricist"),                &lyricistStyle,                  SubStyleId::POET },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Odd Lines"),        &lyricsOddStyle,                 SubStyleId::LYRIC_ODD },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Even Lines"),       &lyricsEvenStyle,                SubStyleId::LYRIC_EVEN },
      { QT_TRANSLATE_NOOP("TextStyle", "Fingering"),               &fingeringStyle,                 SubStyleId::FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "LH Guitar Fingering"),     &lhGuitarFingeringStyle,         SubStyleId::LH_GUITAR_FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "RH Guitar Fingering"),     &rhGuitarFingeringStyle,         SubStyleId::RH_GUITAR_FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "String Number"),           &stringNumberStyle,              SubStyleId::STRING_NUMBER },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Long)"),  &longInstrumentStyle,            SubStyleId::INSTRUMENT_LONG },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Short)"), &shortInstrumentStyle,           SubStyleId::INSTRUMENT_SHORT },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Part)"),  &partInstrumentStyle,            SubStyleId::INSTRUMENT_EXCERPT },
      { QT_TRANSLATE_NOOP("TextStyle", "Dynamics"),                &dynamicsStyle,                  SubStyleId::DYNAMICS },
      { QT_TRANSLATE_NOOP("TextStyle", "Expression"),              &expressionStyle,                SubStyleId::EXPRESSION },
      { QT_TRANSLATE_NOOP("TextStyle", "Tempo"),                   &tempoStyle,                     SubStyleId::TEMPO },
      { QT_TRANSLATE_NOOP("TextStyle", "Metronome"),               &metronomeStyle,                 SubStyleId::METRONOME },
      { QT_TRANSLATE_NOOP("TextStyle", "Measure Number"),          &measureNumberStyle,             SubStyleId::MEASURE_NUMBER },
      { QT_TRANSLATE_NOOP("TextStyle", "Translator"),              &translatorStyle,                SubStyleId::TRANSLATOR },
      { QT_TRANSLATE_NOOP("TextStyle", "Tuplet"),                  &tupletStyle,                    SubStyleId::TUPLET },
      { QT_TRANSLATE_NOOP("TextStyle", "System"),                  &systemStyle,                    SubStyleId::SYSTEM },
      { QT_TRANSLATE_NOOP("TextStyle", "Staff"),                   &staffStyle,                     SubStyleId::STAFF },
      { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol"),            &chordSymbolStyle,               SubStyleId::HARMONY },
      { QT_TRANSLATE_NOOP("TextStyle", "Rehearsal Mark"),          &rehearsalMarkStyle,             SubStyleId::REHEARSAL_MARK },
      { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Left"),        &repeatLeftStyle,                SubStyleId::REPEAT_LEFT },
      { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Right"),       &repeatRightStyle,               SubStyleId::REPEAT_RIGHT },
      { QT_TRANSLATE_NOOP("TextStyle", "Frame"),                   &frameStyle,                     SubStyleId::FRAME },
      { QT_TRANSLATE_NOOP("TextStyle", "Text Line"),               &textLineStyle,                  SubStyleId::TEXTLINE },
      { QT_TRANSLATE_NOOP("TextStyle", "Glissando"),               &glissandoStyle,                 SubStyleId::GLISSANDO },
      { QT_TRANSLATE_NOOP("TextStyle", "Ottava"),                  &ottavaStyle,                    SubStyleId::OTTAVA },
      { QT_TRANSLATE_NOOP("TextStyle", "Volta"),                   &voltaStyle,                     SubStyleId::VOLTA },
      { QT_TRANSLATE_NOOP("TextStyle", "Pedal"),                   &pedalStyle,                     SubStyleId::PEDAL },
      { QT_TRANSLATE_NOOP("TextStyle", "LetRing"),                 &letRingStyle,                   SubStyleId::LET_RING },
      { QT_TRANSLATE_NOOP("TextStyle", "PalmMute"),                &palmMuteStyle,                  SubStyleId::PALM_MUTE },
      { QT_TRANSLATE_NOOP("TextStyle", "Hairpin"),                 &hairpinStyle,                   SubStyleId::HAIRPIN },
      { QT_TRANSLATE_NOOP("TextStyle", "Bend"),                    &bendStyle,                      SubStyleId::BEND },
      { QT_TRANSLATE_NOOP("TextStyle", "Header"),                  &headerStyle,                    SubStyleId::HEADER },
      { QT_TRANSLATE_NOOP("TextStyle", "Footer"),                  &footerStyle,                    SubStyleId::FOOTER },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Change"),       &instrumentChangeStyle,          SubStyleId::INSTRUMENT_CHANGE },
      { QT_TRANSLATE_NOOP("TextStyle", "Figured Bass"),            &figuredBassStyle,               SubStyleId::FIGURED_BASS },
      { QT_TRANSLATE_NOOP("TextStyle", "Beam"),                    &beamStyle,                      SubStyleId::BEAM  },
      { QT_TRANSLATE_NOOP("TextStyle", "Box"),                     &boxStyle,                       SubStyleId::BOX   },
      { QT_TRANSLATE_NOOP("TextStyle", "FretDiagram"),             &fretStyle,                      SubStyleId::FRET  },
      { "TremoloBar",                                              &tremoloBarStyle,                SubStyleId::TREMOLO_BAR },
      { "TimeSig",                                                 &timesigStyle,                   SubStyleId::TIMESIG },
      { "Stem",                                                    &stemStyle,                      SubStyleId::STEM },
      { QT_TRANSLATE_NOOP("TextStyle", "User-1"),                  &user1Style,                     SubStyleId::USER1 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-2"),                  &user2Style,                     SubStyleId::USER2 },
      } };

//---------------------------------------------------------
//   subStyle
//---------------------------------------------------------

const SubStyle& subStyle(const char* name)
      {
      for (const StyledPropertyListName& s : namedStyles) {
            if (strcmp(s.name, name) == 0)
                  return *s.spl;
            }
      qDebug("substyle <%s> not known", name);
      return *namedStyles[0].spl;
      }

const SubStyle& subStyle(SubStyleId idx)
      {
      return *namedStyles[int(idx)].spl;
      }

//---------------------------------------------------------
//   subStyleFromName
//---------------------------------------------------------

SubStyleId subStyleFromName(const QString& name)
      {
      for (const StyledPropertyListName& s : namedStyles) {
            if (s.name == name)
                  return SubStyleId(s.ss);
            }
      if (name == "Technique")                  // compatibility
            return SubStyleId::EXPRESSION;

      qDebug("substyle <%s> not known", qPrintable(name));
      return SubStyleId::DEFAULT;
      }

//---------------------------------------------------------
//   subStyleName
//---------------------------------------------------------

const char* subStyleName(SubStyleId idx)
      {
      return namedStyles[int(idx)].name;
      }

//---------------------------------------------------------
//   subStyleUserName
//---------------------------------------------------------

QString subStyleUserName(SubStyleId idx)
      {
      return qApp->translate("TextStyle", subStyleName(idx));
      }

//---------------------------------------------------------
//   valueType
//---------------------------------------------------------

const char* MStyle::valueType(const Sid i)
      {
      return styleTypes[int(i)].valueType();
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

QVariant MStyle::value(Sid idx) const
      {
      if (!_values[int(idx)].isValid()) {
            qDebug("invalid style value %d %s", int(idx), MStyle::valueName(idx));
            return QVariant();
            }
      return _values[int(idx)];
      }

//---------------------------------------------------------
//   valueName
//---------------------------------------------------------

const char* MStyle::valueName(const Sid i)
      {
      return styleTypes[int(i)].name();
      }

//---------------------------------------------------------
//   styleIdx
//---------------------------------------------------------

Sid MStyle::styleIdx(const QString &name)
      {
      for (StyleType st : styleTypes) {
            if (st.name() == name)
                  return st.styleIdx();
            }
      return Sid::NOSTYLE;
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

MStyle::MStyle()
      {
      _customChordList = false;
      for (const StyleType& t : styleTypes)
            _values[t.idx()] = t.defaultValue();
      };

//---------------------------------------------------------
//   precomputeValues
//---------------------------------------------------------

void MStyle::precomputeValues()
      {
      qreal _spatium = value(Sid::spatium).toDouble();
      for (const StyleType& t : styleTypes) {
            if (!strcmp(t.valueType(), "Ms::Spatium"))
                  _precomputedValues[t.idx()] = _values[t.idx()].value<Spatium>().val() * _spatium;
            }
      }

//---------------------------------------------------------
//   isDefault
//    caution: custom types need to register comparison operator
//          to make this work
//---------------------------------------------------------

bool MStyle::isDefault(Sid idx) const
      {
      return value(idx) == MScore::baseStyle().value(idx);
      }

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* MStyle::chordDescription(int id) const
      {
      if (!_chordList.contains(id))
            return 0;
      return &*_chordList.find(id);
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void MStyle::setChordList(ChordList* cl, bool custom)
      {
      _chordList       = *cl;
      _customChordList = custom;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void MStyle::set(const Sid t, const QVariant& val)
      {
      const int idx = int(t);
      _values[idx] = val;
      if (t == Sid::spatium)
            precomputeValues();
      else {
            if (!strcmp(styleTypes[idx].valueType(), "Ms::Spatium")) {
                  qreal _spatium = value(Sid::spatium).toDouble();
                  _precomputedValues[idx] = _values[idx].value<Spatium>().val() * _spatium;
                  }
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool MStyle::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      for (const StyleType& t : styleTypes) {
            Sid idx = t.styleIdx();
            if (t.name() == tag) {
                  const char* type = t.valueType();
                  if (!strcmp("Ms::Spatium", type))
                        set(idx, Spatium(e.readElementText().toDouble()));
                  else if (!strcmp("double", type))
                        set(idx, QVariant(e.readElementText().toDouble()));
                  else if (!strcmp("bool", type))
                        set(idx, QVariant(bool(e.readElementText().toInt())));
                  else if (!strcmp("int", type))
                        set(idx, QVariant(e.readElementText().toInt()));
                  else if (!strcmp("Ms::Direction", type))
                        set(idx, QVariant::fromValue(Direction(e.readElementText().toInt())));
                  else if (!strcmp("QString", type))
                        set(idx, QVariant(e.readElementText()));
                  else if (!strcmp("Ms::Align", type)) {
                        QStringList sl = e.readElementText().split(',');
                        if (sl.size() != 2) {
                              qDebug("bad align text <%s>", qPrintable(e.readElementText()));
                              return true;
                              }
                        Align align = Align::LEFT;
                        if (sl[0] == "center")
                              align = align | Align::HCENTER;
                        else if (sl[0] == "right")
                              align = align | Align::RIGHT;
                        else if (sl[0] == "left")
                              ;
                        else {
                              qDebug("bad align text <%s>", qPrintable(sl[0]));
                              return true;
                              }
                        if (sl[1] == "center")
                              align = align | Align::VCENTER;
                        else if (sl[1] == "bottom")
                              align = align | Align::BOTTOM;
                        else if (sl[1] == "baseline")
                              align = align | Align::BASELINE;
                        else if (sl[1] == "top")
                              ;
                        else {
                              qDebug("bad align text <%s>", qPrintable(sl[1]));
                              return true;
                              }
                        set(idx, QVariant::fromValue(align));
                        }
                  else if (!strcmp("QPointF", type)) {
                        qreal x = e.doubleAttribute("x", 0.0);
                        qreal y = e.doubleAttribute("y", 0.0);
                        set(idx, QPointF(x, y));
                        e.readElementText();
                        }
                  else if (!strcmp("QSizeF", type)) {
                        qreal x = e.intAttribute("w", 0);
                        qreal y = e.doubleAttribute("h", 0.0);
                        set(idx, QSizeF(x, y));
                        e.readElementText();
                        }
                  else if (!strcmp("QColor", type)) {
                        QColor c;
                        c.setRed(e.intAttribute("r"));
                        c.setGreen(e.intAttribute("g"));
                        c.setBlue(e.intAttribute("b"));
                        c.setAlpha(e.intAttribute("a", 255));
                        set(idx, c);
                        e.readElementText();
                        }
                  else {
                        qFatal("unhandled type %s", type);
                        }
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool MStyle::load(QFile* qf)
      {
      XmlReader e(qf);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl  = version.split('.');
                  int mscVersion  = sl[0].toInt() * 100 + sl[1].toInt();
                  if (mscVersion != MSCVERSION)
                        return false;
                  while (e.readNextStartElement()) {
                        if (e.name() == "Style")
                              load(e);
                        else
                              e.unknown();
                        }
                  }
            }
      return true;
      }

extern void readPageFormat(MStyle* style, XmlReader& e);

void MStyle::load(XmlReader& e)
      {
      QString oldChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "TextStyle") {
                  readTextStyle206(this, e);        // obsolete
                  }
            else if (tag == "Spatium")
                  set(Sid::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout") {    // obsolete
                  readPageFormat(this, e);      // from read206.cpp
                  }
            else if (tag == "displayInConcertPitch")
                  set(Sid::concertPitch, QVariant(bool(e.readInt())));
            else if (tag == "ChordList") {
                  _chordList.clear();
                  _chordList.read(e);
                  _customChordList = true;
                  chordListTag = true;
                  }
            else if (!readProperties(e))
                  e.unknown();
            }

      // if we just specified a new chord description file
      // and didn't encounter a ChordList tag
      // then load the chord description file

      QString newChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
      if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
            if (!newChordDescriptionFile.startsWith("chords_") && value(Sid::chordStyle).toString() == "std") {
                  // should not normally happen,
                  // but treat as "old" (114) score just in case
                  set(Sid::chordStyle, QVariant(QString("custom")));
                  set(Sid::chordsXmlFile, QVariant(true));
                  qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
                  }
            if (value(Sid::chordStyle).toString() == "custom")
                  _customChordList = true;
            else
                  _customChordList = false;
            _chordList.unload();
            }

      // make sure we have a chordlist
      if (!_chordList.loaded() && !chordListTag) {
            if (value(Sid::chordsXmlFile).toBool())
                  _chordList.read("chords.xml");
            _chordList.read(newChordDescriptionFile);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void MStyle::save(XmlWriter& xml, bool optimize)
      {
      xml.stag("Style");

      for (const StyleType& st : styleTypes) {
            Sid idx = st.styleIdx();
            if (idx == Sid::spatium)       // special handling for spatium
                  continue;
            if (optimize && isDefault(idx))
                  continue;
            const char* type = st.valueType();
            if (!strcmp("Ms::Spatium", type))
                  xml.tag(st.name(), value(idx).value<Spatium>().val());
            else if (!strcmp("Ms::Direction", type))
                  xml.tag(st.name(), value(idx).toInt());
            else
                  xml.tag(st.name(), value(idx));
            }
      if (_customChordList && !_chordList.empty()) {
            xml.stag("ChordList");
            _chordList.write(xml);
            xml.etag();
            }
      xml.tag("Spatium", value(Sid::spatium).toDouble() / DPMM);
      xml.etag();
      }

#ifndef NDEBUG
//---------------------------------------------------------
//   checkStyles
//---------------------------------------------------------

void checkStyles()
      {
      int idx = 0;
      for (const StyleType& t : styleTypes) {
            Q_ASSERT(t.idx() == idx);
            ++idx;
            }
      idx = 0;
      for (auto a : namedStyles) {
            Q_ASSERT(int(a.ss) == idx);
            ++idx;
            }
      }
#endif

}
