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
      StyleIdx _idx;
      const char* _name;       // xml name for read()/write()
      QVariant _defaultValue;

   public:
      StyleIdx  styleIdx() const            { return _idx;          }
      int idx() const                       { return int(_idx);     }
      const char*  valueType() const        { return _defaultValue.typeName();    }
      const char*      name() const         { return _name;         }
      const QVariant&  defaultValue() const { return _defaultValue; }
      };

#define MM(x) ((x)/INCH)

static const StyleType styleTypes[] {
      { StyleIdx::pageWidth,               "pageWidth",               210.0/INCH },
      { StyleIdx::pageHeight,              "pageHeight",              297.0/INCH }, // A4
      { StyleIdx::pagePrintableWidth,      "pagePrintableWidth",      190.0/INCH },
      { StyleIdx::pageEvenLeftMargin,      "pageEvenLeftMargin",      10.0/INCH  },
      { StyleIdx::pageOddLeftMargin,       "pageOddLeftMargin",       10.0/INCH  },
      { StyleIdx::pageEvenTopMargin,       "pageEvenTopMargin",       10.0/INCH  },
      { StyleIdx::pageEvenBottomMargin,    "pageEvenBottomMargin",    20.0/INCH  },
      { StyleIdx::pageOddTopMargin,        "pageOddTopMargin",        10.0/INCH  },
      { StyleIdx::pageOddBottomMargin,     "pageOddBottomMargin",     20.0/INCH  },
      { StyleIdx::pageTwosided,            "pageTwosided",            true       },

      { StyleIdx::staffUpperBorder,        "staffUpperBorder",        Spatium(7.0)  },
      { StyleIdx::staffLowerBorder,        "staffLowerBorder",        Spatium(7.0)  },
      { StyleIdx::staffDistance,           "staffDistance",           Spatium(6.5)  },
      { StyleIdx::akkoladeDistance,        "akkoladeDistance",        Spatium(6.5)  },
      { StyleIdx::minSystemDistance,       "minSystemDistance",       Spatium(8.5)  },
      { StyleIdx::maxSystemDistance,       "maxSystemDistance",       Spatium(15.0) },

      { StyleIdx::lyricsPlacement,         "lyricsPlacement",         int(Placement::BELOW)  },
      { StyleIdx::lyricsPosAbove,          "lyricsPosAbove",          Spatium(-2.0) },
      { StyleIdx::lyricsPosBelow,          "lyricsPosBelow",          Spatium(2.0) },
      { StyleIdx::lyricsMinTopDistance,    "lyricsMinTopDistance",    Spatium(1.0)  },
      { StyleIdx::lyricsMinBottomDistance, "lyricsMinBottomDistance", Spatium(2.0)  },
      { StyleIdx::lyricsLineHeight,        "lyricsLineHeight",        1.0 },
      { StyleIdx::lyricsDashMinLength,     "lyricsDashMinLength",     Spatium(0.4) },
      { StyleIdx::lyricsDashMaxLength,     "lyricsDashMaxLegth",      Spatium(0.8) },
      { StyleIdx::lyricsDashMaxDistance,   "lyricsDashMaxDistance",   Spatium(16.0) },
      { StyleIdx::lyricsDashForce,         "lyricsDashForce",         QVariant(true) },
      { StyleIdx::lyricsAlignVerseNumber,  "lyricsAlignVerseNumber",  true },
      { StyleIdx::lyricsLineThickness,     "lyricsLineThickness",     Spatium(0.1) },

      { StyleIdx::figuredBassFontFamily,   "figuredBassFontFamily",   QString("MScoreBC") },

//      { StyleIdx::figuredBassFontSize,     "figuredBassFontSize",     QVariant(8.0) },
      { StyleIdx::figuredBassYOffset,      "figuredBassYOffset",      QVariant(6.0) },
      { StyleIdx::figuredBassLineHeight,   "figuredBassLineHeight",   QVariant(1.0) },
      { StyleIdx::figuredBassAlignment,    "figuredBassAlignment",    QVariant(0) },
      { StyleIdx::figuredBassStyle,        "figuredBassStyle" ,       QVariant(0) },
      { StyleIdx::systemFrameDistance,     "systemFrameDistance",     Spatium(7.0) },
      { StyleIdx::frameSystemDistance,     "frameSystemDistance",     Spatium(7.0) },
      { StyleIdx::minMeasureWidth,         "minMeasureWidth",         Spatium(5.0) },
      { StyleIdx::barWidth,                "barWidth",                Spatium(0.16) },      // 0.1875
      { StyleIdx::doubleBarWidth,          "doubleBarWidth",          Spatium(0.16) },

      { StyleIdx::endBarWidth,             "endBarWidth",             Spatium(0.5) },       // 0.5
      { StyleIdx::doubleBarDistance,       "doubleBarDistance",       Spatium(0.46) },      // 0.3
      { StyleIdx::endBarDistance,          "endBarDistance",          Spatium(.40 + (.5) * .5) },     // 0.3
      { StyleIdx::repeatBarlineDotSeparation, "repeatBarlineDotSeparation", Spatium(.40 + .16 * .5) },
      { StyleIdx::repeatBarTips,           "repeatBarTips",           QVariant(false) },
      { StyleIdx::startBarlineSingle,      "startBarlineSingle",      QVariant(false) },
      { StyleIdx::startBarlineMultiple,    "startBarlineMultiple",    QVariant(true) },
      { StyleIdx::bracketWidth,            "bracketWidth",            Spatium(0.45) },
      { StyleIdx::bracketDistance,         "bracketDistance",         Spatium(0.1) },
      { StyleIdx::akkoladeWidth,           "akkoladeWidth",           Spatium(1.6) },
      { StyleIdx::akkoladeBarDistance,     "akkoladeBarDistance",     Spatium(.4) },

      { StyleIdx::dividerLeft,             "dividerLeft",             QVariant(false) },
      { StyleIdx::dividerLeftSym,          "dividerLeftSym",          QVariant(QString("systemDivider")) },
      { StyleIdx::dividerLeftX,            "dividerLeftX",            QVariant(0.0) },
      { StyleIdx::dividerLeftY,            "dividerLeftY",            QVariant(0.0) },
      { StyleIdx::dividerRight,            "dividerRight",            QVariant(false) },
      { StyleIdx::dividerRightSym,         "dividerRightSym",         QVariant(QString("systemDivider")) },
      { StyleIdx::dividerRightX,           "dividerRightX",           QVariant(0.0) },
      { StyleIdx::dividerRightY,           "dividerRightY",           QVariant(0.0) },

      { StyleIdx::clefLeftMargin,          "clefLeftMargin",          Spatium(0.8) },     // 0.64 (gould: <= 1)
      { StyleIdx::keysigLeftMargin,        "keysigLeftMargin",        Spatium(0.5) },
      { StyleIdx::ambitusMargin,           "ambitusMargin",           Spatium(0.5) },

      { StyleIdx::timesigLeftMargin,       "timesigLeftMargin",       Spatium(0.5) },
      { StyleIdx::timesigScale,            "timesigScale",            QVariant(QSizeF(1.0, 1.0)) },
      { StyleIdx::clefKeyRightMargin,      "clefKeyRightMargin",      Spatium(0.8) },
      { StyleIdx::clefKeyDistance,         "clefKeyDistance",         Spatium(1.0) },   // gould: 1 - 1.25
      { StyleIdx::clefTimesigDistance,     "clefTimesigDistance",     Spatium(1.0) },
      { StyleIdx::keyTimesigDistance,      "keyTimesigDistance",      Spatium(1.0) },    // gould: 1 - 1.5
      { StyleIdx::keyBarlineDistance,      "keyTimesigDistance",      Spatium(1.0) },
      { StyleIdx::systemHeaderDistance,    "systemHeaderDistance",    Spatium(2.5) },     // gould: 2.5
      { StyleIdx::systemHeaderTimeSigDistance, "systemHeaderTimeSigDistance", Spatium(2.0) },  // gould: 2.0

      { StyleIdx::clefBarlineDistance,     "clefBarlineDistance",     Spatium(0.5) },
      { StyleIdx::timesigBarlineDistance,  "timesigBarlineDistance",  Spatium(0.5) },
      { StyleIdx::stemWidth,               "stemWidth",               Spatium(0.13) },      // 0.09375
      { StyleIdx::shortenStem,             "shortenStem",             QVariant(true) },
      { StyleIdx::shortStemProgression,    "shortStemProgression",    Spatium(0.25) },
      { StyleIdx::shortestStem,            "shortestStem",            Spatium(2.25) },
      { StyleIdx::beginRepeatLeftMargin,   "beginRepeatLeftMargin",   Spatium(1.0) },
      { StyleIdx::minNoteDistance,         "minNoteDistance",         Spatium(0.25) },      // 0.4
      { StyleIdx::barNoteDistance,         "barNoteDistance",         Spatium(1.0) },     // was 1.2

      { StyleIdx::barAccidentalDistance,   "barAccidentalDistance",   Spatium(.3)   },
      { StyleIdx::multiMeasureRestMargin,  "multiMeasureRestMargin",  Spatium(1.2)  },
      { StyleIdx::noteBarDistance,         "noteBarDistance",         Spatium(1.0)  },
      { StyleIdx::measureSpacing,          "measureSpacing",          QVariant(1.2) },
      { StyleIdx::staffLineWidth,          "staffLineWidth",          Spatium(0.08) },     // 0.09375
      { StyleIdx::ledgerLineWidth,         "ledgerLineWidth",         Spatium(0.16) },     // 0.1875
      { StyleIdx::ledgerLineLength,        "ledgerLineLength",        Spatium(.6)   },     // notehead width + this value
      { StyleIdx::accidentalDistance,      "accidentalDistance",      Spatium(0.22) },
      { StyleIdx::accidentalNoteDistance,  "accidentalNoteDistance",  Spatium(0.22) },

      { StyleIdx::beamWidth,               "beamWidth",               Spatium(0.5)  },     // was 0.48
      { StyleIdx::beamDistance,            "beamDistance",            QVariant(0.5) },     // 0.25sp units
      { StyleIdx::beamMinLen,              "beamMinLen",              Spatium(1.32) },     // 1.316178 exactly notehead widthen beams
      { StyleIdx::beamNoSlope,             "beamNoSlope",             QVariant(false) },

      { StyleIdx::dotMag,                  "dotMag",                  QVariant(1.0) },
      { StyleIdx::dotNoteDistance,         "dotNoteDistance",         Spatium(0.35) },
      { StyleIdx::dotRestDistance,         "dotRestDistance",         Spatium(0.25) },
      { StyleIdx::dotDotDistance,          "dotDotDistance",          Spatium(0.5) },
      { StyleIdx::propertyDistanceHead,    "propertyDistanceHead",    Spatium(1.0) },
      { StyleIdx::propertyDistanceStem,    "propertyDistanceStem",    Spatium(1.8) },
      { StyleIdx::propertyDistance,        "propertyDistance",        Spatium(1.0) },

      { StyleIdx::articulationMag,         "articulationMag",         QVariant(1.0) },
      { StyleIdx::lastSystemFillLimit,     "lastSystemFillLimit",     QVariant(0.3) },

      { StyleIdx::hairpinPlacement,        "hairpinPlacement",        int(Placement::BELOW)  },
      { StyleIdx::hairpinPosAbove,         "hairpinPosAbove",         Spatium(-3.5) },
      { StyleIdx::hairpinPosBelow,         "hairpinPosBelow",         Spatium(3.5) },
      { StyleIdx::hairpinHeight,           "hairpinHeight",           Spatium(1.2) },
      { StyleIdx::hairpinContHeight,       "hairpinContHeight",       Spatium(0.5) },
      { StyleIdx::hairpinLineWidth,        "hairpinWidth",            Spatium(0.13) },

      { StyleIdx::pedalPlacement,          "pedalPlacement",          int(Placement::BELOW)  },
      { StyleIdx::pedalPosAbove,           "pedalPosAbove",           Spatium(-4) },
      { StyleIdx::pedalPosBelow,           "pedalPosBelow",           Spatium(4) },
      { StyleIdx::pedalLineWidth,          "pedalLineWidth",          Spatium(.15) },
      { StyleIdx::pedalLineStyle,          "pedalListStyle",          QVariant(int(Qt::SolidLine)) },
      { StyleIdx::pedalBeginTextOffset,    "pedalBeginTextOffset",    QPointF(0.0, 0.15) },
      { StyleIdx::pedalHookHeight,         "pedalHookHeight",         Spatium(-1.2) },

      { StyleIdx::trillPlacement,          "trillPlacement",          int(Placement::ABOVE)  },
      { StyleIdx::trillPosAbove,           "trillPosAbove",           Spatium(-1) },
      { StyleIdx::trillPosBelow,           "trillPosBelow",           Spatium(1) },

      { StyleIdx::vibratoPlacement,        "vibratoPlacement",        int(Placement::ABOVE)  },
      { StyleIdx::vibratoPosAbove,         "vibratoPosAbove",         Spatium(-1) },
      { StyleIdx::vibratoPosBelow,         "vibratoPosBelow",         Spatium(1) },

      { StyleIdx::harmonyY,                "harmonyY",                Spatium(2.5) },
      { StyleIdx::harmonyFretDist,         "harmonyFretDist",         Spatium(0.5) },
      { StyleIdx::minHarmonyDistance,      "minHarmonyDistance",      Spatium(0.5) },
      { StyleIdx::maxHarmonyBarDistance,   "maxHarmonyBarDistance",   Spatium(3.0) },
      { StyleIdx::capoPosition,            "capoPosition",            QVariant(0) },
      { StyleIdx::fretNumMag,              "fretNumMag",              QVariant(2.0) },
      { StyleIdx::fretNumPos,              "fretNumPos",              QVariant(0) },
      { StyleIdx::fretY,                   "fretY",                   Spatium(2.0) },
      { StyleIdx::fretMinDistance,         "fretMinDistance",         Spatium(0.5) },
      { StyleIdx::showPageNumber,          "showPageNumber",          QVariant(true) },
      { StyleIdx::showPageNumberOne,       "showPageNumberOne",       QVariant(false) },

      { StyleIdx::pageNumberOddEven,       "pageNumberOddEven",       QVariant(true) },
      { StyleIdx::showMeasureNumber,       "showMeasureNumber",       QVariant(true) },
      { StyleIdx::showMeasureNumberOne,    "showMeasureNumberOne",    QVariant(false) },
      { StyleIdx::measureNumberInterval,   "measureNumberInterval",   QVariant(5) },
      { StyleIdx::measureNumberSystem,     "measureNumberSystem",     QVariant(true) },
      { StyleIdx::measureNumberAllStaffs,  "measureNumberAllStaffs",  QVariant(false) },
      { StyleIdx::smallNoteMag,            "smallNoteMag",            QVariant(.7) },
      { StyleIdx::graceNoteMag,            "graceNoteMag",            QVariant(0.7) },
      { StyleIdx::smallStaffMag,           "smallStaffMag",           QVariant(0.7) },
      { StyleIdx::smallClefMag,            "smallClefMag",            QVariant(0.8) },

      { StyleIdx::genClef,                 "genClef",                 QVariant(true) },
      { StyleIdx::genKeysig,               "genKeysig",               QVariant(true) },
      { StyleIdx::genCourtesyTimesig,      "genCourtesyTimesig",      QVariant(true) },
      { StyleIdx::genCourtesyKeysig,       "genCourtesyKeysig",       QVariant(true) },
      { StyleIdx::genCourtesyClef,         "genCourtesyClef",         QVariant(true) },
      { StyleIdx::swingRatio,              "swingRatio",              QVariant(60)   },
      { StyleIdx::swingUnit,               "swingUnit",               QVariant(QString("")) },
      { StyleIdx::useStandardNoteNames,    "useStandardNoteNames",    QVariant(true) },
      { StyleIdx::useGermanNoteNames,      "useGermanNoteNames",      QVariant(false) },
      { StyleIdx::useFullGermanNoteNames,  "useFullGermanNoteNames",  QVariant(false) },

      { StyleIdx::useSolfeggioNoteNames,   "useSolfeggioNoteNames",   QVariant(false) },
      { StyleIdx::useFrenchNoteNames,      "useFrenchNoteNames",      QVariant(false) },
      { StyleIdx::automaticCapitalization, "automaticCapitalization", QVariant(true) },
      { StyleIdx::lowerCaseMinorChords,    "lowerCaseMinorChords",    QVariant(false) },
      { StyleIdx::lowerCaseBassNotes,      "lowerCaseBassNotes",      QVariant(false) },
      { StyleIdx::allCapsNoteNames,        "allCapsNoteNames",        QVariant(false) },
      { StyleIdx::chordStyle,              "chordStyle",              QVariant(QString("std")) },
      { StyleIdx::chordsXmlFile,           "chordsXmlFile",           QVariant(false) },
      { StyleIdx::chordDescriptionFile,    "chordDescriptionFile",    QVariant(QString("chords_std.xml")) },
      { StyleIdx::concertPitch,            "concertPitch",            QVariant(false) },

      { StyleIdx::createMultiMeasureRests, "createMultiMeasureRests", QVariant(false) },
      { StyleIdx::minEmptyMeasures,        "minEmptyMeasures",        QVariant(2) },
      { StyleIdx::minMMRestWidth,          "minMMRestWidth",          Spatium(4) },
      { StyleIdx::hideEmptyStaves,         "hideEmptyStaves",         QVariant(false) },
      { StyleIdx::dontHideStavesInFirstSystem,
                                 "dontHidStavesInFirstSystm",         QVariant(true) },
      { StyleIdx::hideInstrumentNameIfOneInstrument,
                                 "hideInstrumentNameIfOneInstrument", QVariant(true) },
      { StyleIdx::gateTime,                "gateTime",                QVariant(100) },
      { StyleIdx::tenutoGateTime,          "tenutoGateTime",          QVariant(100) },
      { StyleIdx::staccatoGateTime,        "staccatoGateTime",        QVariant(50) },
      { StyleIdx::slurGateTime,            "slurGateTime",            QVariant(100) },

      { StyleIdx::ArpeggioNoteDistance,    "ArpeggioNoteDistance",    Spatium(.5) },
      { StyleIdx::ArpeggioLineWidth,       "ArpeggioLineWidth",       Spatium(.18) },
      { StyleIdx::ArpeggioHookLen,         "ArpeggioHookLen",         Spatium(.8) },
      { StyleIdx::ArpeggioHiddenInStdIfTab,"ArpeggioHiddenInStdIfTab",QVariant(false)},
      { StyleIdx::SlurEndWidth,            "slurEndWidth",            Spatium(.07) },
      { StyleIdx::SlurMidWidth,            "slurMidWidth",            Spatium(.15) },
      { StyleIdx::SlurDottedWidth,         "slurDottedWidth",         Spatium(.10)  },
      { StyleIdx::MinTieLength,            "minTieLength",            Spatium(1.0) },
      { StyleIdx::SlurMinDistance,         "slurMinDistance",         Spatium(0.5) },
      { StyleIdx::SectionPause,            "sectionPause",            QVariant(qreal(3.0)) },
      { StyleIdx::MusicalSymbolFont,       "musicalSymbolFont",       QVariant(QString("Emmentaler")) },
      { StyleIdx::MusicalTextFont,         "musicalTextFont",         QVariant(QString("MScore Text")) },

      { StyleIdx::showHeader,              "showHeader",              QVariant(false) },
      { StyleIdx::headerFirstPage,         "headerFirstPage",         QVariant(false) },
      { StyleIdx::headerOddEven,           "headerOddEven",           QVariant(true) },
      { StyleIdx::evenHeaderL,             "evenHeaderL",             QVariant(QString()) },
      { StyleIdx::evenHeaderC,             "evenHeaderC",             QVariant(QString()) },
      { StyleIdx::evenHeaderR,             "evenHeaderR",             QVariant(QString()) },
      { StyleIdx::oddHeaderL,              "oddHeaderL",              QVariant(QString()) },
      { StyleIdx::oddHeaderC,              "oddHeaderC",              QVariant(QString()) },
      { StyleIdx::oddHeaderR,              "oddHeaderR",              QVariant(QString()) },
      { StyleIdx::showFooter,              "showFooter",              QVariant(true) },

      { StyleIdx::footerFirstPage,         "footerFirstPage",         QVariant(true) },
      { StyleIdx::footerOddEven,           "footerOddEven",           QVariant(true) },
      { StyleIdx::evenFooterL,             "evenFooterL",             QVariant(QString("$p")) },
      { StyleIdx::evenFooterC,             "evenFooterC",             QVariant(QString("$:copyright:")) },
      { StyleIdx::evenFooterR,             "evenFooterR",             QVariant(QString()) },
      { StyleIdx::oddFooterL,              "oddFooterL",              QVariant(QString()) },
      { StyleIdx::oddFooterC,              "oddFooterC",              QVariant(QString("$:copyright:")) },
      { StyleIdx::oddFooterR,              "oddFooterR",              QVariant(QString("$p")) },
      { StyleIdx::voltaY,                  "voltaY",                  Spatium(-3.0) },
      { StyleIdx::voltaHook,               "voltaHook",               Spatium(1.9) },

      { StyleIdx::voltaLineWidth,          "voltaLineWidth",          Spatium(.1) },
      { StyleIdx::voltaLineStyle,          "voltaLineStyle",          QVariant(int(Qt::SolidLine)) },

      { StyleIdx::ottavaPlacement,         "ottavaPlacement",         int(Placement::ABOVE)  },
      { StyleIdx::ottavaPosAbove,          "ottavaPosAbove",          Spatium(-3.0) },
      { StyleIdx::ottavaPosBelow,          "ottavaPosBelow",          Spatium(3.0) },
      { StyleIdx::ottavaHook,              "ottavaHook",              Spatium(1.9) },
      { StyleIdx::ottavaLineWidth,         "ottavaLineWidth",         Spatium(.1) },
      { StyleIdx::ottavaLineStyle,         "ottavaLineStyle",         QVariant(int(Qt::DashLine)) },
      { StyleIdx::ottavaNumbersOnly,       "ottavaNumbersOnly",       true },

      { StyleIdx::tabClef,                 "tabClef",                 QVariant(int(ClefType::TAB)) },
      { StyleIdx::tremoloWidth,            "tremoloWidth",            Spatium(1.2) },  // tremolo stroke width: notehead width
      { StyleIdx::tremoloBoxHeight,        "tremoloBoxHeight",        Spatium(0.65) },

      { StyleIdx::tremoloStrokeWidth,      "tremoloLineWidth",        Spatium(0.5) },  // was 0.35
      { StyleIdx::tremoloDistance,         "tremoloDistance",         Spatium(0.8) },
      { StyleIdx::linearStretch,           "linearStretch",           QVariant(qreal(1.5)) },
      { StyleIdx::crossMeasureValues,      "crossMeasureValues",      QVariant(false) },
      { StyleIdx::keySigNaturals,          "keySigNaturals",          QVariant(int(KeySigNatural::NONE)) },

      { StyleIdx::tupletMaxSlope,          "tupletMaxSlope",          QVariant(qreal(0.5)) },
      { StyleIdx::tupletOufOfStaff,        "tupletOufOfStaff",        QVariant(true) },
      { StyleIdx::tupletVHeadDistance,     "tupletVHeadDistance",     Spatium(.5) },
      { StyleIdx::tupletVStemDistance,     "tupletVStemDistance",     Spatium(.25) },
      { StyleIdx::tupletStemLeftDistance,  "tupletStemLeftDistance",  Spatium(.5) },
      { StyleIdx::tupletStemRightDistance, "tupletStemRightDistance", Spatium(.5) },
      { StyleIdx::tupletNoteLeftDistance,  "tupletNoteLeftDistance",  Spatium(0.0) },
      { StyleIdx::tupletNoteRightDistance, "tupletNoteRightDistance", Spatium(0.0) },
      { StyleIdx::tupletBracketWidth,      "tupletBracketWidth",      Spatium(0.1) },
      { StyleIdx::tupletDirection,         "tupletDirection",         QVariant::fromValue<Direction>(Direction::AUTO) },
      { StyleIdx::tupletNumberType,        "tupletNumberType",        int(TupletNumberType::SHOW_NUMBER) },
      { StyleIdx::tupletBracketType,       "tupletBracketType",       int(TupletBracketType::AUTO_BRACKET) },

      { StyleIdx::barreLineWidth,          "barreLineWidth",          QVariant(1.0) },
      { StyleIdx::fretMag,                 "fretMag",                 QVariant(1.0) },
      { StyleIdx::scaleBarlines,           "scaleBarlines",           QVariant(true) },
      { StyleIdx::barGraceDistance,        "barGraceDistance",        Spatium(.6) },
      { StyleIdx::minVerticalDistance,     "minVerticalDistance",     Spatium(0.5) },
      { StyleIdx::ornamentStyle,           "ornamentStyle",           int(MScore::OrnamentStyle::DEFAULT) },
      { StyleIdx::spatium,                 "Spatium",                 SPATIUM20 },

      { StyleIdx::autoplaceHairpinDynamicsDistance, "autoplaceHairpinDynamicsDistance", Spatium(0.5) },

      { StyleIdx::dynamicsPlacement,       "dynamicsPlacement",       int(Placement::BELOW)  },
      { StyleIdx::dynamicsPosAbove,        "dynamicsPosAbove",        Spatium(-2.0) },
      { StyleIdx::dynamicsPosBelow,        "dynamicsPosBelow",        Spatium(1.0) },

      { StyleIdx::dynamicsMinDistance,         "dynamicsMinDistance",               Spatium(0.5) },
      { StyleIdx::autoplaceVerticalAlignRange, "autoplaceVerticalAlignRange",     int(VerticalAlignRange::SYSTEM) },

      { StyleIdx::textLinePlacement,         "textLinePlacement",         int(Placement::ABOVE)  },
      { StyleIdx::textLinePosAbove,          "textLinePosAbove",          Spatium(-3.5) },
      { StyleIdx::textLinePosBelow,          "textLinePosBelow",          Spatium(3.5) },

      { StyleIdx::tremoloBarLineWidth,       "tremoloBarLineWidth",       Spatium(0.1) },
      { StyleIdx::jumpPosAbove,              "jumpPosAbove",              Spatium(-2.0) },
      { StyleIdx::markerPosAbove,            "markerPosAbove",            Spatium(-2.0) },

//====

      { StyleIdx::defaultFontFace,               "defaultFontFace",               "FreeSerif" },
      { StyleIdx::defaultFontSize,               "defaultFontSize",               10.0  },
      { StyleIdx::defaultFontSpatiumDependent,   "defaultFontSpatiumDependent",   true  },
      { StyleIdx::defaultFontBold,               "defaultFontBold",               false },
      { StyleIdx::defaultFontItalic,             "defaultFontItalic",             false },
      { StyleIdx::defaultFontUnderline,          "defaultFontUnderline",          false },
      { StyleIdx::defaultAlign,                  "defaultAlign",                  QVariant::fromValue(Align::LEFT) },
      { StyleIdx::defaultFrame,                  "defaultFrame",                  false },
      { StyleIdx::defaultFrameSquare,            "defaultFrameSquare",            false },
      { StyleIdx::defaultFrameCircle,            "defaultFrameCircle",            false },
      { StyleIdx::defaultFramePadding,           "defaultFramePadding",           0.2 },
      { StyleIdx::defaultFrameWidth,             "defaultFrameWidth",             0.1 },
      { StyleIdx::defaultFrameRound,             "defaultFrameRound",             0 },
      { StyleIdx::defaultFrameFgColor,           "defaultFrameFgColor",           QColor(0, 0, 0, 255) },
      { StyleIdx::defaultFrameBgColor,           "defaultFrameBgColor",           QColor(255, 255, 255, 0) },
      { StyleIdx::defaultOffset,                 "defaultOffset",                 QPointF() },
      { StyleIdx::defaultOffsetType,             "defaultOffsetType",             int(OffsetType::SPATIUM)   },
      { StyleIdx::defaultSystemFlag,             "defaultSystemFlag",             false },

      { StyleIdx::titleFontFace,                 "titleFontFace",                 "FreeSerif" },
      { StyleIdx::titleFontSize,                 "titleFontSize",                 24.0 },
      { StyleIdx::titleFontSpatiumDependent,     "titleFontSpatiumDependent",     false  },
      { StyleIdx::titleFontBold,                 "titleFontBold",                 false },
      { StyleIdx::titleFontItalic,               "titleFontItalic",               false },
      { StyleIdx::titleFontUnderline,            "titleFontUnderline",            false },
      { StyleIdx::titleAlign,                    "titleAlign",                    QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { StyleIdx::titleOffset,                   "titleOffset",                   QPointF() },
      { StyleIdx::titleOffsetType,               "titleOffsetType",               int(OffsetType::ABS)   },

      { StyleIdx::subTitleFontFace,              "subTitleFontFace",              "FreeSerif" },
      { StyleIdx::subTitleFontSize,              "subTitleFontSize",              14.0 },
      { StyleIdx::subTitleFontSpatiumDependent,  "subTitleFontSpatiumDependent",  false  },
      { StyleIdx::subTitleFontBold,              "subTitleFontBold",              false },
      { StyleIdx::subTitleFontItalic,            "subTtitleFontItalic",           false },
      { StyleIdx::subTitleFontUnderline,         "subTitleFontUnderline",         false },
      { StyleIdx::subTitleAlign,                 "subTitleAlign",                 QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { StyleIdx::subTitleOffset,                "subTitleOffset",                QPointF(0.0, MM(10.0)) },
      { StyleIdx::subTitleOffsetType,            "subTitleOffsetType",            int(OffsetType::ABS)   },

      { StyleIdx::composerFontFace,              "composerFontFace",              "FreeSerif" },
      { StyleIdx::composerFontSize,              "composerFontSize",              12.0 },
      { StyleIdx::composerFontSpatiumDependent,  "composerFontSpatiumDependent",  false  },
      { StyleIdx::composerFontBold,              "composerFontBold",              false },
      { StyleIdx::composerFontItalic,            "composerFontItalic",            false },
      { StyleIdx::composerFontUnderline,         "composerFontUnderline",         false },
      { StyleIdx::composerAlign,                 "composerAlign",                 QVariant::fromValue(Align::RIGHT | Align::BOTTOM) },
      { StyleIdx::composerOffset,                "composerOffset",                QPointF() },
      { StyleIdx::composerOffsetType,            "composerOffsetType",            int(OffsetType::ABS)   },

      { StyleIdx::lyricistFontFace,              "lyricistFontFace",              "FreeSerif" },
      { StyleIdx::lyricistFontSize,              "lyricistFontSize",              12.0 },
      { StyleIdx::lyricistFontSpatiumDependent,  "lyricistFontSpatiumDependent",  false  },
      { StyleIdx::lyricistFontBold,              "lyricistFontBold",              false },
      { StyleIdx::lyricistFontItalic,            "lyricistFontItalic",            false },
      { StyleIdx::lyricistFontUnderline,         "lyricistFontUnderline",         false },
      { StyleIdx::lyricistAlign,                 "lyricistAlign",                 QVariant::fromValue(Align::LEFT | Align::BOTTOM) },
      { StyleIdx::lyricistOffset,                "lyricistOffset",                QPointF() },
      { StyleIdx::lyricistOffsetType,            "lyricistOffsetType",            int(OffsetType::ABS)   },

      { StyleIdx::lyricsOddFontFace,             "lyricsOddFontFace",             "FreeSerif" },
      { StyleIdx::lyricsOddFontSize,             "lyricsOddFontSize",             11.0 },
      { StyleIdx::lyricsOddFontBold,             "lyricsOddFontBold",             false },
      { StyleIdx::lyricsOddFontItalic,           "lyricsOddFontItalic",           false },
      { StyleIdx::lyricsOddFontUnderline,        "lyricsOddFontUnderline",        false },
      { StyleIdx::lyricsOddAlign,                "lyricistOddAlign",              QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { StyleIdx::lyricsOddOffset,               "lyricistOddOffset",             QPointF(0.0, 6.0) },

      { StyleIdx::lyricsEvenFontFace,            "lyricsEvenFontFace",            "FreeSerif" },
      { StyleIdx::lyricsEvenFontSize,            "lyricsEvenFontSize",            11.0 },
      { StyleIdx::lyricsEvenFontBold,            "lyricsEvenFontBold",            false },
      { StyleIdx::lyricsEvenFontItalic,          "lyricsEvenFontItalic",          false },
      { StyleIdx::lyricsEvenFontUnderline,       "lyricsEventFontUnderline",      false },
      { StyleIdx::lyricsEvenAlign,               "lyricistEvenAlign",             QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { StyleIdx::lyricsEvenOffset,              "lyricistEvenOffset",            QPointF(0.0, 6.0) },

      { StyleIdx::fingeringFontFace,             "fingeringFontFace",             "FreeSerif" },
      { StyleIdx::fingeringFontSize,             "fingeringFontSize",             8.0 },
      { StyleIdx::fingeringFontBold,             "fingeringFontBold",             false },
      { StyleIdx::fingeringFontItalic,           "fingeringFontItalic",           false },
      { StyleIdx::fingeringFontUnderline,        "fingeringFontUnderline",        false },
      { StyleIdx::fingeringAlign,                "fingeringAlign",                QVariant::fromValue(Align::CENTER) },
      { StyleIdx::fingeringFrame,                "fingeringFrame",                false },
      { StyleIdx::fingeringFrameSquare,          "fingeringFrameSquare",          false },
      { StyleIdx::fingeringFrameCircle,          "fingeringFrameCircle",          false },
      { StyleIdx::fingeringFramePadding,         "fingeringFramePadding",         0.2 },
      { StyleIdx::fingeringFrameWidth,           "fingeringFrameWidth",           0.1 },
      { StyleIdx::fingeringFrameRound,           "fingeringFrameRound",           0 },
      { StyleIdx::fingeringFrameFgColor,         "fingeringFrameFgColor",         QColor(0, 0, 0, 255) },
      { StyleIdx::fingeringFrameBgColor,         "fingeringFrameBgColor",         QColor(255, 255, 255, 0) },
      { StyleIdx::fingeringOffset,               "fingeringOffset",               QPointF() },

      { StyleIdx::lhGuitarFingeringFontFace,     "lhGuitarFingeringFontFace",     "FreeSerif" },
      { StyleIdx::lhGuitarFingeringFontSize,     "lhGuitarFingeringFontSize",     8.0 },
      { StyleIdx::lhGuitarFingeringFontBold,     "lhGuitarFingeringFontBold",     false },
      { StyleIdx::lhGuitarFingeringFontItalic,   "lhGuitarFingeringFontItalic",   false },
      { StyleIdx::lhGuitarFingeringFontUnderline,"lhGuitarFingeringFontUnderline",false },
      { StyleIdx::lhGuitarFingeringAlign,        "lhGuitarFingeringAlign",        QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { StyleIdx::lhGuitarFingeringFrame,        "lhGuitarFingeringFrame",        false },
      { StyleIdx::lhGuitarFingeringFrameSquare,  "lhGuitarFingeringFrameSquare",  false },
      { StyleIdx::lhGuitarFingeringFrameCircle,  "lhGuitarFingeringFrameCircle",  false },
      { StyleIdx::lhGuitarFingeringFramePadding, "lhGuitarFingeringFramePadding", 0.2 },
      { StyleIdx::lhGuitarFingeringFrameWidth,   "lhGuitarFingeringFrameWidth",   0.1 },
      { StyleIdx::lhGuitarFingeringFrameRound,   "lhGuitarFingeringFrameRound",   0 },
      { StyleIdx::lhGuitarFingeringFrameFgColor, "lhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { StyleIdx::lhGuitarFingeringFrameBgColor, "lhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { StyleIdx::lhGuitarFingeringOffset,       "lhGuitarFingeringOffset",       QPointF(-0.5, 0.0) },

      { StyleIdx::rhGuitarFingeringFontFace,     "rhGuitarFingeringFontFace",     "FreeSerif" },
      { StyleIdx::rhGuitarFingeringFontSize,     "rhGuitarFingeringFontSize",     8.0 },
      { StyleIdx::rhGuitarFingeringFontBold,     "rhGuitarFingeringFontBold",     false },
      { StyleIdx::rhGuitarFingeringFontItalic,   "rhGuitarFingeringFontItalic",   false },
      { StyleIdx::rhGuitarFingeringFontUnderline,"rhGuitarFingeringFontUnderline",false },
      { StyleIdx::rhGuitarFingeringAlign,        "rhGuitarFingeringAlign",        QVariant::fromValue(Align::CENTER) },
      { StyleIdx::rhGuitarFingeringFrame,        "rhGuitarFingeringFrame",        false },
      { StyleIdx::rhGuitarFingeringFrameSquare,  "rhGuitarFingeringFrameSquare",  false },
      { StyleIdx::rhGuitarFingeringFrameCircle,  "rhGuitarFingeringFrameCircle",  false },
      { StyleIdx::rhGuitarFingeringFramePadding, "rhGuitarFingeringFramePadding", 0.2 },
      { StyleIdx::rhGuitarFingeringFrameWidth,   "rhGuitarFingeringFrameWidth",   0.1 },
      { StyleIdx::rhGuitarFingeringFrameRound,   "rhGuitarFingeringFrameRound",   0 },
      { StyleIdx::rhGuitarFingeringFrameFgColor, "rhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { StyleIdx::rhGuitarFingeringFrameBgColor, "rhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { StyleIdx::rhGuitarFingeringOffset,       "rhGuitarFingeringOffset",       QPointF() },

      { StyleIdx::stringNumberFontFace,          "stringNumberFontFace",          "FreeSerif" },
      { StyleIdx::stringNumberFontSize,          "stringNumberFontSize",          8.0 },
      { StyleIdx::stringNumberFontBold,          "stringNumberFontBold",          false },
      { StyleIdx::stringNumberFontItalic,        "stringNumberFontItalic",        false },
      { StyleIdx::stringNumberFontUnderline,     "stringNumberFontUnderline",     false },
      { StyleIdx::stringNumberAlign,             "stringNumberAlign",             QVariant::fromValue(Align::CENTER) },
      { StyleIdx::stringNumberFrame,             "stringNumberFrame",             true },
      { StyleIdx::stringNumberFrameSquare,       "stringNumberFrameSquare",       false },
      { StyleIdx::stringNumberFrameCircle,       "stringNumberFrameCircle",       true },
      { StyleIdx::stringNumberFramePadding,      "stringNumberFramePadding",      0.2 },
      { StyleIdx::stringNumberFrameWidth,        "stringNumberFrameWidth",        0.1 },
      { StyleIdx::stringNumberFrameRound,        "stringNumberFrameRound",        0 },
      { StyleIdx::stringNumberFrameFgColor,      "stringNumberFrameFgColor",      QColor(0, 0, 0, 255) },
      { StyleIdx::stringNumberFrameBgColor,      "stringNumberFrameBgColor",      QColor(255, 255, 255, 0) },
      { StyleIdx::stringNumberOffset,            "stringNumberOffset",            QPointF(0.0, -2.0) },

      { StyleIdx::longInstrumentFontFace,        "longInstrumentFontFace",       "FreeSerif" },
      { StyleIdx::longInstrumentFontSize,        "longInstrumentFontSize",       12.0 },
      { StyleIdx::longInstrumentFontBold,        "longInstrumentFontBold",       false },
      { StyleIdx::longInstrumentFontItalic,      "longInstrumentFontItalic",     false },
      { StyleIdx::longInstrumentFontUnderline,   "longInstrumentFontUnderline",  false },
      { StyleIdx::longInstrumentAlign,           "longInstrumentAlign",          QVariant::fromValue(Align::RIGHT | Align::VCENTER) },

      { StyleIdx::shortInstrumentFontFace,       "shortInstrumentFontFace",      "FreeSerif" },
      { StyleIdx::shortInstrumentFontSize,       "shortInstrumentFontSize",      12.0 },
      { StyleIdx::shortInstrumentFontBold,       "shortInstrumentFontBold",      false },
      { StyleIdx::shortInstrumentFontItalic,     "shortInstrumentFontItalic",    false },
      { StyleIdx::shortInstrumentFontUnderline,  "shortInstrumentFontUnderline", false },
      { StyleIdx::shortInstrumentAlign,          "shortInstrumentAlign",         QVariant::fromValue(Align::RIGHT | Align::VCENTER) },

      { StyleIdx::partInstrumentFontFace,        "partInstrumentFontFace",       "FreeSerif" },
      { StyleIdx::partInstrumentFontSize,        "partInstrumentFontSize",       18.0 },
      { StyleIdx::partInstrumentFontBold,        "partInstrumentFontBold",       false },
      { StyleIdx::partInstrumentFontItalic,      "partInstrumentFontItalic",     false },
      { StyleIdx::partInstrumentFontUnderline,   "partInstrumentFontUnderline",  false },

      { StyleIdx::dynamicsFontFace,              "dynamicsFontFace",             "FreeSerif" },
      { StyleIdx::dynamicsFontSize,              "dynamicsFontSize",             12.0 },
      { StyleIdx::dynamicsFontBold,              "dynamicsFontBold",             false },
      { StyleIdx::dynamicsFontItalic,            "dynamicsFontItalic",           true },
      { StyleIdx::dynamicsFontUnderline,         "dynamicsFontUnderline",        false },
      { StyleIdx::dynamicsAlign,                 "dynamicsAlign",                QVariant::fromValue(Align::HCENTER | Align::BASELINE) },

      { StyleIdx::expressionFontFace,            "expressionFontFace",           "FreeSerif" },
      { StyleIdx::expressionFontSize,            "expressionFontSize",           11.0 },
      { StyleIdx::expressionFontBold,            "expressionFontBold",           false },
      { StyleIdx::expressionFontItalic,          "expressionFontItalic",         true },
      { StyleIdx::expressionFontUnderline,       "expressionFontUnderline",      false },
      { StyleIdx::expressionAlign,               "expressionAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { StyleIdx::tempoFontFace,                 "tempoFontFace",                "FreeSerif" },
      { StyleIdx::tempoFontSize,                 "tempoFontSize",                12.0 },
      { StyleIdx::tempoFontBold,                 "tempoFontBold",                true },
      { StyleIdx::tempoFontItalic,               "tempoFontItalic",              false },
      { StyleIdx::tempoFontUnderline,            "tempoFontUnderline",           false },
      { StyleIdx::tempoAlign,                    "tempoAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { StyleIdx::tempoOffset,                   "tempoOffset",                  QPointF(0.0, -4.0) },
      { StyleIdx::tempoSystemFlag,               "tempoSystemFlag",              true },
      { StyleIdx::tempoPlacement,                "tempoPlacement",               int(Placement::ABOVE)  },
      { StyleIdx::tempoPosAbove,                 "tempoPosAbove",                Spatium(-2.0) },
      { StyleIdx::tempoPosBelow,                 "tempoPosBelow",                Spatium(3.0)  },
      { StyleIdx::tempoMinDistance,              "tempoMinDistance",             Spatium(.5)  },

      { StyleIdx::metronomeFontFace,             "metronomeFontFace",            "FreeSerif" },
      { StyleIdx::metronomeFontSize,             "metronomeFontSize",            12.0 },
      { StyleIdx::metronomeFontBold,             "metronomeFontBold",            true },
      { StyleIdx::metronomeFontItalic,           "metronomeFontItalic",          false },
      { StyleIdx::metronomeFontUnderline,        "metronomeFontUnderline",       false },

      { StyleIdx::measureNumberFontFace,         "measureNumberFontFace",        "FreeSerif" },
      { StyleIdx::measureNumberFontSize,         "measureNumberFontSize",        8.0 },
      { StyleIdx::measureNumberFontBold,         "measureNumberFontBold",        false },
      { StyleIdx::measureNumberFontItalic,       "measureNumberFontItalic",      false },
      { StyleIdx::measureNumberFontUnderline,    "measureNumberFontUnderline",   false },
      { StyleIdx::measureNumberOffset,           "measureNumberOffset",          QPointF(0.0, -2.0) },
      { StyleIdx::measureNumberOffsetType,       "measureNumberOffsetType",      int(OffsetType::SPATIUM)   },

      { StyleIdx::translatorFontFace,            "translatorFontFace",           "FreeSerif" },
      { StyleIdx::translatorFontSize,            "translatorFontSize",           11.0 },
      { StyleIdx::translatorFontBold,            "translatorFontBold",           false },
      { StyleIdx::translatorFontItalic,          "translatorFontItalic",         false },
      { StyleIdx::translatorFontUnderline,       "translatorFontUnderline",      false },

      { StyleIdx::tupletFontFace,                "tupletFontFace",               "FreeSerif" },
      { StyleIdx::tupletFontSize,                "tupletFontSize",               10.0 },
      { StyleIdx::tupletFontBold,                "tupletFontBold",               false },
      { StyleIdx::tupletFontItalic,              "tupletFontItalic",             true },
      { StyleIdx::tupletFontUnderline,           "tupletFontUnderline",          false },
      { StyleIdx::tupletAlign,                   "tupletAlign",                  QVariant::fromValue(Align::CENTER) },

      { StyleIdx::systemFontFace,                "systemFontFace",               "FreeSerif" },
      { StyleIdx::systemFontSize,                "systemFontSize",               10.0 },
      { StyleIdx::systemFontBold,                "systemFontBold",               false },
      { StyleIdx::systemFontItalic,              "systemFontItalic",             false },
      { StyleIdx::systemFontUnderline,           "systemFontUnderline",          false },
      { StyleIdx::systemOffset,                  "systemOffset",                 QPointF(0.0, -4.0) },
      { StyleIdx::systemOffsetType,              "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { StyleIdx::systemAlign,                   "systemAlign",                  QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { StyleIdx::staffTextFontFace,             "staffFontFace",                "FreeSerif" },
      { StyleIdx::staffTextFontSize,             "staffFontSize",                10.0 },
      { StyleIdx::staffTextFontBold,             "staffFontBold",                false },
      { StyleIdx::staffTextFontItalic,           "staffFontItalic",              false },
      { StyleIdx::staffTextFontUnderline,        "staffFontUnderline",           false },
      { StyleIdx::staffTextAlign,                "staffAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { StyleIdx::staffTextOffset,               "staffOffset",                  QPointF(0.0, -4.0) },
      { StyleIdx::staffTextOffsetType,           "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { StyleIdx::staffTextPlacement,            "staffTextPlacement",           int(Placement::ABOVE) },
      { StyleIdx::staffTextPosAbove,             "staffTextPosAbove",            Spatium(-2.0) },
      { StyleIdx::staffTextPosBelow,             "staffTextPosBelow",            Spatium(3.5)  },
      { StyleIdx::staffTextMinDistance,          "staffTextMinDistance",         Spatium(0.5)  },

      { StyleIdx::chordSymbolFontFace,           "chordSymbolFontFace",          "FreeSerif" },
      { StyleIdx::chordSymbolFontSize,           "chordSymbolFontSize",          12.0 },
      { StyleIdx::chordSymbolFontBold,           "chordSymbolFontBold",          false },
      { StyleIdx::chordSymbolFontItalic,         "chordSymbolFontItalic",        false },
      { StyleIdx::chordSymbolFontUnderline,      "chordSymbolFontUnderline",     false },
      { StyleIdx::chordSymbolAlign,              "chordSymbolAlign",             QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { StyleIdx::rehearsalMarkFontFace,         "rehearsalMarkFontFace",        "FreeSerif" },
      { StyleIdx::rehearsalMarkFontSize,         "rehearsalMarkFontSize",        14.0 },
      { StyleIdx::rehearsalMarkFontBold,         "rehearsalMarkFontBold",        true },
      { StyleIdx::rehearsalMarkFontItalic,       "rehearsalMarkFontItalic",      false },
      { StyleIdx::rehearsalMarkFontUnderline,    "rehearsalMarkFontUnderline",   false },
      { StyleIdx::rehearsalMarkAlign,            "rehearsalMarkAlign",           QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { StyleIdx::rehearsalMarkFrame,            "rehearsalMarkFrame",           true  },
      { StyleIdx::rehearsalMarkFrameSquare,      "rehearsalMarkFrameSquare",     false },
      { StyleIdx::rehearsalMarkFrameCircle,      "rehearsalMarkFrameCircle",     false },
      { StyleIdx::rehearsalMarkFramePadding,     "rehearsalMarkFramePadding",    0.5 },
      { StyleIdx::rehearsalMarkFrameWidth,       "rehearsalMarkFrameWidth",      0.2 },
      { StyleIdx::rehearsalMarkFrameRound,       "rehearsalMarkFrameRound",      20 },
      { StyleIdx::rehearsalMarkFrameFgColor,     "rehearsalMarkFrameFgColor",    QColor(0, 0, 0, 255) },
      { StyleIdx::rehearsalMarkFrameBgColor,     "rehearsalMarkFrameBgColor",    QColor(255, 255, 255, 0) },
      { StyleIdx::rehearsalMarkPlacement,        "rehearsalMarkPlacement",       int(Placement::ABOVE) },
      { StyleIdx::rehearsalMarkPosAbove,         "rehearsalMarkPosAbove",        Spatium(-3.0) },
      { StyleIdx::rehearsalMarkPosBelow,         "rehearsalMarkPosBelow",        Spatium(4.0) },
      { StyleIdx::rehearsalMarkMinDistance,      "rehearsalMarkMinDistance",     Spatium(0.5) },

      { StyleIdx::repeatLeftFontFace,            "repeatLeftFontFace",           "FreeSerif" },
      { StyleIdx::repeatLeftFontSize,            "repeatLeftFontSize",           20.0 },
      { StyleIdx::repeatLeftFontBold,            "repeatLeftFontBold",           false },
      { StyleIdx::repeatLeftFontItalic,          "repeatLeftFontItalic",         false },
      { StyleIdx::repeatLeftFontUnderline,       "repeatLeftFontUnderline",      false },
      { StyleIdx::repeatLeftAlign,               "repeatLeftAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { StyleIdx::repeatLeftPlacement,           "repeatLeftPlacement",          int(Placement::ABOVE) },

      { StyleIdx::repeatRightFontFace,           "repeatRightFontFace",          "FreeSerif" },
      { StyleIdx::repeatRightFontSize,           "repeatRightFontSize",          12.0 },
      { StyleIdx::repeatRightFontBold,           "repeatRightFontBold",          false },
      { StyleIdx::repeatRightFontItalic,         "repeatRightFontItalic",        false },
      { StyleIdx::repeatRightFontUnderline,      "repeatRightFontUnderline",     false },
      { StyleIdx::repeatRightAlign,              "repeatRightAlign",             QVariant::fromValue(Align::RIGHT | Align::BASELINE) },
      { StyleIdx::repeatRightPlacement,          "repeatLeftPlacement",          int(Placement::ABOVE) },

      { StyleIdx::voltaFontFace,                 "voltaFontFace",                "FreeSerif" },
      { StyleIdx::voltaFontSize,                 "voltaFontSize",                11.0 },
      { StyleIdx::voltaFontBold,                 "voltaFontBold",                true },
      { StyleIdx::voltaFontItalic,               "voltaFontItalic",              false },
      { StyleIdx::voltaFontUnderline,            "voltaFontUnderline",           false },
      { StyleIdx::voltaAlign,                    "voltaAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { StyleIdx::voltaOffset,                   "voltaOffset",                  QPointF(0.5, 1.9) },

      { StyleIdx::frameFontFace,                 "frameFontFace",                "FreeSerif" },
      { StyleIdx::frameFontSize,                 "frameFontSize",                12.0 },
      { StyleIdx::frameFontBold,                 "frameFontBold",                false },
      { StyleIdx::frameFontItalic,               "frameFontItalic",              false },
      { StyleIdx::frameFontUnderline,            "frameFontUnderline",           false },
      { StyleIdx::frameAlign,                    "frameAlign",                   QVariant::fromValue(Align::LEFT) },

      { StyleIdx::textLineFontFace,              "textLineFontFace",             "FreeSerif" },
      { StyleIdx::textLineFontSize,              "textLineFontSize",             12.0 },
      { StyleIdx::textLineFontBold,              "textLineFontBold",             false },
      { StyleIdx::textLineFontItalic,            "textLineFontItalic",           false },
      { StyleIdx::textLineFontUnderline,         "textLineFontUnderline",        false },

      { StyleIdx::glissandoFontFace,             "glissandoFontFace",            "FreeSerif" },
      { StyleIdx::glissandoFontSize,             "glissandoFontSize",            QVariant(8.0) },
      { StyleIdx::glissandoFontBold,             "glissandoFontBold",            false },
      { StyleIdx::glissandoFontItalic,           "glissandoFontItalic",          true },
      { StyleIdx::glissandoFontUnderline,        "glissandoFontUnderline",       false },
      { StyleIdx::glissandoLineWidth,            "glissandoLineWidth",           Spatium(0.15) },

      { StyleIdx::ottavaFontFace,                "ottavaFontFace",               "FreeSerif" },
      { StyleIdx::ottavaFontSize,                "ottavaFontSize",               12.0 },
      { StyleIdx::ottavaFontBold,                "ottavaFontBold",               false },
      { StyleIdx::ottavaFontItalic,              "ottavaFontItalic",             false },
      { StyleIdx::ottavaFontUnderline,           "ottavaFontUnderline",          false },
      { StyleIdx::ottavaTextAlign,               "ottavaTextAlign",              QVariant::fromValue(Align::LEFT | Align::VCENTER) },

      { StyleIdx::pedalFontFace,                 "pedalFontFace",                "FreeSerif" },
      { StyleIdx::pedalFontSize,                 "pedalFontSize",                12.0 },
      { StyleIdx::pedalFontBold,                 "pedalFontBold",                false },
      { StyleIdx::pedalFontItalic,               "pedalFontItalic",              false },
      { StyleIdx::pedalFontUnderline,            "pedalFontUnderline",           false },
      { StyleIdx::pedalTextAlign,                "pedalTextAlign",               QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { StyleIdx::hairpinFontFace,               "hairpinFontFace",              "FreeSerif" },
      { StyleIdx::hairpinFontSize,               "hairpinFontSize",              12.0 },
      { StyleIdx::hairpinFontBold,               "hairpinFontBold",              false },
      { StyleIdx::hairpinFontItalic,             "hairpinFontItalic",            true },
      { StyleIdx::hairpinFontUnderline,          "hairpinFontUnderline",         false },
      { StyleIdx::hairpinTextAlign,              "hairpinTextAlign",             QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { StyleIdx::bendFontFace,                  "bendFontFace",                 "FreeSerif" },
      { StyleIdx::bendFontSize,                  "bendFontSize",                 8.0 },
      { StyleIdx::bendFontBold,                  "bendFontBold",                 false },
      { StyleIdx::bendFontItalic,                "bendFontItalic",               false },
      { StyleIdx::bendFontUnderline,             "bendFontUnderline",            false },
      { StyleIdx::bendLineWidth,                 "bendLineWidth",                Spatium(0.15) },
      { StyleIdx::bendArrowWidth,                "bendArrowWidth",               Spatium(.5) },

      { StyleIdx::headerFontFace,                "headerFontFace",               "FreeSerif" },
      { StyleIdx::headerFontSize,                "headerFontSize",               8.0 },
      { StyleIdx::headerFontBold,                "headerFontBold",               false },
      { StyleIdx::headerFontItalic,              "headerFontItalic",             false },
      { StyleIdx::headerFontUnderline,           "headerFontUnderline",          false },

      { StyleIdx::footerFontFace,                "footerFontFace",               "FreeSerif" },
      { StyleIdx::footerFontSize,                "footerFontSize",               8.0 },
      { StyleIdx::footerFontBold,                "footerFontBold",               false },
      { StyleIdx::footerFontItalic,              "footerFontItalic",             false },
      { StyleIdx::footerFontUnderline,           "footerFontUnderline",          false },

      { StyleIdx::instrumentChangeFontFace,      "instrumentChangeFontFace",     "FreeSerif" },
      { StyleIdx::instrumentChangeFontSize,      "instrumentChangeFontSize",     12.0 },
      { StyleIdx::instrumentChangeFontBold,      "instrumentChangeFontBold",     true },
      { StyleIdx::instrumentChangeFontItalic,    "instrumentChangeFontItalic",   false },
      { StyleIdx::instrumentChangeFontUnderline, "instrumentChangeFontUnderline",false },
      { StyleIdx::instrumentChangeAlign,         "instrumentChangeAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { StyleIdx::instrumentChangeOffset,        "instrumentChangeOffset",       QPointF(0, -3.0) },

      { StyleIdx::figuredBassFontFace,           "figuredBassFontFace",          "MScoreBC" },
      { StyleIdx::figuredBassFontSize,           "figuredBassFontSize",          8.0 },
      { StyleIdx::figuredBassFontBold,           "figuredBassFontBold",          false },
      { StyleIdx::figuredBassFontItalic,         "figuredBassFontItalic",        false },
      { StyleIdx::figuredBassFontUnderline,      "figuredBassFontUnderline",     false },

      { StyleIdx::user1FontFace,                 "user1FontFace",                "FreeSerif" },
      { StyleIdx::user1FontSize,                 "user1FontSize",                10.0 },
      { StyleIdx::user1FontBold,                 "user1FontBold",                false },
      { StyleIdx::user1FontItalic,               "user1FontItalic",              false },
      { StyleIdx::user1FontUnderline,            "user1FontUnderline",           false },

      { StyleIdx::user2FontFace,                 "user2FontFace",                "FreeSerif" },
      { StyleIdx::user2FontSize,                 "user2FontSize",                10.0 },
      { StyleIdx::user2FontBold,                 "user2FontBold",                false },
      { StyleIdx::user2FontItalic,               "user2FontItalic",              false },
      { StyleIdx::user2FontUnderline,            "user2FontUnderline",           false },

      { StyleIdx::letRingFontFace,               "letRingFontFace",              "FreeSerif" },
      { StyleIdx::letRingFontSize,               "letRingFontSize",              10.0 },
      { StyleIdx::letRingFontBold,               "letRingFontBold",              false },
      { StyleIdx::letRingFontItalic,             "letRingFontItalic",            true },
      { StyleIdx::letRingFontUnderline,          "letRingFontUnderline",         false },
      { StyleIdx::letRingTextAlign,              "letRingTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { StyleIdx::letRingHookHeight,             "letRingHookHeight",            Spatium(0.6) },
      { StyleIdx::letRingPlacement,              "letRingPlacement",             int(Placement::BELOW)  },
      { StyleIdx::letRingPosAbove,               "letRingPosAbove",              Spatium(-4.0) },
      { StyleIdx::letRingPosBelow,               "letRingPosBelow",              Spatium(4.0)  },
      { StyleIdx::letRingLineWidth,              "letRingLineWidth",             Spatium(0.15) },
      { StyleIdx::letRingLineStyle,              "letRingLineStyle",             QVariant(int(Qt::DashLine)) },
      { StyleIdx::letRingBeginTextOffset,        "letRingBeginTextOffset",       QPointF(0.0, 0.15) },
      { StyleIdx::letRingText,                   "letRingText",                  "let ring" },

      { StyleIdx::palmMuteFontFace,              "palmMuteFontFace",              "FreeSerif" },
      { StyleIdx::palmMuteFontSize,              "palmMuteFontSize",              10.0 },
      { StyleIdx::palmMuteFontBold,              "palmMuteFontBold",              false },
      { StyleIdx::palmMuteFontItalic,            "palmMuteFontItalic",            true },
      { StyleIdx::palmMuteFontUnderline,         "palmMuteFontUnderline",         false },
      { StyleIdx::palmMuteTextAlign,             "palmMuteTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { StyleIdx::palmMuteHookHeight,            "palmMuteHookHeight",            Spatium(0.6) },
      { StyleIdx::palmMutePlacement,             "palmMutePlacement",             int(Placement::BELOW)  },
      { StyleIdx::palmMutePosAbove,              "palmMutePosAbove",              Spatium(-4.0) },
      { StyleIdx::palmMutePosBelow,              "palmMutePosBelow",              Spatium(4.0)  },
      { StyleIdx::palmMuteLineWidth,             "palmMuteLineWidth",             Spatium(0.15) },
      { StyleIdx::palmMuteLineStyle,             "palmMuteLineStyle",             QVariant(int(Qt::DashLine)) },
      { StyleIdx::palmMuteBeginTextOffset,       "palmMuteBeginTextOffset",       QPointF(0.0, 0.15) },
      { StyleIdx::palmMuteText,                  "palmMuteText",                  "P.M." },

      { StyleIdx::fermataPosAbove,               "fermataPosAbove",               Spatium(-1.0) },
      { StyleIdx::fermataPosBelow,               "fermataPosBelow",               Spatium(1.0)  },
      { StyleIdx::fermataMinDistance,            "fermataMinDistance",            Spatium(0.4)  },
      };
#undef MM

MStyle  MScore::_baseStyle;
MStyle  MScore::_defaultStyle;

//---------------------------------------------------------
//   sets of styled properties
//---------------------------------------------------------

const std::vector<StyledProperty> emptyStyle {
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> defaultStyle {
      { StyleIdx::defaultFontFace,                    P_ID::FONT_FACE              },
      { StyleIdx::defaultFontSize,                    P_ID::FONT_SIZE              },
      { StyleIdx::defaultFontSpatiumDependent,        P_ID::FONT_SPATIUM_DEPENDENT },
      { StyleIdx::defaultFontBold,                    P_ID::FONT_BOLD              },
      { StyleIdx::defaultFontItalic,                  P_ID::FONT_ITALIC            },
      { StyleIdx::defaultFontUnderline,               P_ID::FONT_UNDERLINE         },
      { StyleIdx::defaultAlign,                       P_ID::ALIGN                  },
      { StyleIdx::defaultFrame,                       P_ID::FRAME                  },
      { StyleIdx::defaultFrameSquare,                 P_ID::FRAME_SQUARE           },
      { StyleIdx::defaultFrameCircle,                 P_ID::FRAME_CIRCLE           },
      { StyleIdx::defaultFramePadding,                P_ID::FRAME_PADDING          },
      { StyleIdx::defaultFrameWidth,                  P_ID::FRAME_WIDTH            },
      { StyleIdx::defaultFrameRound,                  P_ID::FRAME_ROUND            },
      { StyleIdx::defaultFrameFgColor,                P_ID::FRAME_FG_COLOR         },
      { StyleIdx::defaultFrameBgColor,                P_ID::FRAME_BG_COLOR         },
      { StyleIdx::defaultOffset,                      P_ID::OFFSET                 },
      { StyleIdx::defaultOffsetType,                  P_ID::OFFSET_TYPE            },
      { StyleIdx::defaultSystemFlag,                  P_ID::SYSTEM_FLAG            },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> titleStyle {
      { StyleIdx::titleFontFace,                      P_ID::FONT_FACE              },
      { StyleIdx::titleFontSize,                      P_ID::FONT_SIZE              },
      { StyleIdx::titleFontSpatiumDependent,          P_ID::FONT_SPATIUM_DEPENDENT },
      { StyleIdx::titleFontBold,                      P_ID::FONT_BOLD              },
      { StyleIdx::titleFontItalic,                    P_ID::FONT_ITALIC            },
      { StyleIdx::titleFontUnderline,                 P_ID::FONT_UNDERLINE         },
      { StyleIdx::titleAlign,                         P_ID::ALIGN                  },
      { StyleIdx::titleOffset,                        P_ID::OFFSET                 },
      { StyleIdx::titleOffsetType,                    P_ID::OFFSET_TYPE            },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> subTitleStyle {
      { StyleIdx::subTitleFontFace,                   P_ID::FONT_FACE              },
      { StyleIdx::subTitleFontSize,                   P_ID::FONT_SIZE              },
      { StyleIdx::subTitleFontSpatiumDependent,       P_ID::FONT_SPATIUM_DEPENDENT },
      { StyleIdx::subTitleFontBold,                   P_ID::FONT_BOLD              },
      { StyleIdx::subTitleFontItalic,                 P_ID::FONT_ITALIC            },
      { StyleIdx::subTitleFontUnderline,              P_ID::FONT_UNDERLINE         },
      { StyleIdx::subTitleAlign,                      P_ID::ALIGN                  },
      { StyleIdx::subTitleOffset,                     P_ID::OFFSET                 },
      { StyleIdx::subTitleOffsetType,                 P_ID::OFFSET_TYPE            },
      { StyleIdx::subTitleOffset,                     P_ID::OFFSET                 },
      { StyleIdx::subTitleOffsetType,                 P_ID::OFFSET_TYPE            },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> composerStyle {
      { StyleIdx::composerFontFace,                   P_ID::FONT_FACE              },
      { StyleIdx::composerFontSize,                   P_ID::FONT_SIZE              },
      { StyleIdx::composerFontSpatiumDependent,       P_ID::FONT_SPATIUM_DEPENDENT },
      { StyleIdx::composerFontBold,                   P_ID::FONT_BOLD              },
      { StyleIdx::composerFontItalic,                 P_ID::FONT_ITALIC            },
      { StyleIdx::composerFontUnderline,              P_ID::FONT_UNDERLINE         },
      { StyleIdx::composerAlign,                      P_ID::ALIGN                  },
      { StyleIdx::composerOffset,                     P_ID::OFFSET                 },
      { StyleIdx::composerOffsetType,                 P_ID::OFFSET_TYPE            },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lyricistStyle {
      { StyleIdx::lyricistFontFace,                   P_ID::FONT_FACE              },
      { StyleIdx::lyricistFontSize,                   P_ID::FONT_SIZE              },
      { StyleIdx::lyricistFontBold,                   P_ID::FONT_BOLD              },
      { StyleIdx::lyricistFontItalic,                 P_ID::FONT_ITALIC            },
      { StyleIdx::lyricistFontUnderline,              P_ID::FONT_UNDERLINE         },
      { StyleIdx::lyricistAlign,                      P_ID::ALIGN                  },
      { StyleIdx::lyricistOffset,                     P_ID::OFFSET                 },
      { StyleIdx::lyricistOffsetType,                 P_ID::OFFSET_TYPE            },
      { StyleIdx::lyricsPlacement,                    P_ID::PLACEMENT              },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lyricsOddStyle {
      { StyleIdx::lyricsOddFontFace,                  P_ID::FONT_FACE              },
      { StyleIdx::lyricsOddFontSize,                  P_ID::FONT_SIZE              },
      { StyleIdx::lyricsOddFontBold,                  P_ID::FONT_BOLD              },
      { StyleIdx::lyricsOddFontItalic,                P_ID::FONT_ITALIC            },
      { StyleIdx::lyricsOddFontUnderline,             P_ID::FONT_UNDERLINE         },
      { StyleIdx::lyricsOddAlign,                     P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lyricsEvenStyle {
      { StyleIdx::lyricsEvenFontFace,                 P_ID::FONT_FACE              },
      { StyleIdx::lyricsEvenFontSize,                 P_ID::FONT_SIZE              },
      { StyleIdx::lyricsEvenFontBold,                 P_ID::FONT_BOLD              },
      { StyleIdx::lyricsEvenFontItalic,               P_ID::FONT_ITALIC            },
      { StyleIdx::lyricsEvenFontUnderline,            P_ID::FONT_UNDERLINE         },
      { StyleIdx::lyricsEvenAlign,                    P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> fingeringStyle {
      { StyleIdx::fingeringFontFace,                  P_ID::FONT_FACE              },
      { StyleIdx::fingeringFontSize,                  P_ID::FONT_SIZE              },
      { StyleIdx::fingeringFontBold,                  P_ID::FONT_BOLD              },
      { StyleIdx::fingeringFontItalic,                P_ID::FONT_ITALIC            },
      { StyleIdx::fingeringFontUnderline,             P_ID::FONT_UNDERLINE         },
      { StyleIdx::fingeringAlign,                     P_ID::ALIGN                  },
      { StyleIdx::fingeringFrame,                     P_ID::FRAME                  },
      { StyleIdx::fingeringFrameSquare,               P_ID::FRAME_SQUARE           },
      { StyleIdx::fingeringFrameCircle,               P_ID::FRAME_CIRCLE           },
      { StyleIdx::fingeringFramePadding,              P_ID::FRAME_PADDING          },
      { StyleIdx::fingeringFrameWidth,                P_ID::FRAME_WIDTH            },
      { StyleIdx::fingeringFrameRound,                P_ID::FRAME_ROUND            },
      { StyleIdx::fingeringFrameFgColor,              P_ID::FRAME_FG_COLOR         },
      { StyleIdx::fingeringFrameBgColor,              P_ID::FRAME_BG_COLOR         },
      { StyleIdx::fingeringOffset,                    P_ID::OFFSET                 },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> lhGuitarFingeringStyle {
      { StyleIdx::lhGuitarFingeringFontFace,          P_ID::FONT_FACE              },
      { StyleIdx::lhGuitarFingeringFontSize,          P_ID::FONT_SIZE              },
      { StyleIdx::lhGuitarFingeringFontBold,          P_ID::FONT_BOLD              },
      { StyleIdx::lhGuitarFingeringFontItalic,        P_ID::FONT_ITALIC            },
      { StyleIdx::lhGuitarFingeringFontUnderline,     P_ID::FONT_UNDERLINE         },
      { StyleIdx::lhGuitarFingeringAlign,             P_ID::ALIGN                  },
      { StyleIdx::lhGuitarFingeringFrame,             P_ID::FRAME                  },
      { StyleIdx::lhGuitarFingeringFrameSquare,       P_ID::FRAME_SQUARE           },
      { StyleIdx::lhGuitarFingeringFrameCircle,       P_ID::FRAME_CIRCLE           },
      { StyleIdx::lhGuitarFingeringFramePadding,      P_ID::FRAME_PADDING          },
      { StyleIdx::lhGuitarFingeringFrameWidth,        P_ID::FRAME_WIDTH            },
      { StyleIdx::lhGuitarFingeringFrameRound,        P_ID::FRAME_ROUND            },
      { StyleIdx::lhGuitarFingeringFrameFgColor,      P_ID::FRAME_FG_COLOR         },
      { StyleIdx::lhGuitarFingeringFrameBgColor,      P_ID::FRAME_BG_COLOR         },
      { StyleIdx::lhGuitarFingeringOffset,            P_ID::OFFSET                 },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> rhGuitarFingeringStyle {
      { StyleIdx::rhGuitarFingeringFontFace,          P_ID::FONT_FACE              },
      { StyleIdx::rhGuitarFingeringFontSize,          P_ID::FONT_SIZE              },
      { StyleIdx::rhGuitarFingeringFontBold,          P_ID::FONT_BOLD              },
      { StyleIdx::rhGuitarFingeringFontItalic,        P_ID::FONT_ITALIC            },
      { StyleIdx::rhGuitarFingeringFontUnderline,     P_ID::FONT_UNDERLINE         },
      { StyleIdx::rhGuitarFingeringAlign,             P_ID::ALIGN                  },
      { StyleIdx::rhGuitarFingeringFrame,             P_ID::FRAME                  },
      { StyleIdx::rhGuitarFingeringFrameSquare,       P_ID::FRAME_SQUARE           },
      { StyleIdx::rhGuitarFingeringFrameCircle,       P_ID::FRAME_CIRCLE           },
      { StyleIdx::rhGuitarFingeringFramePadding,      P_ID::FRAME_PADDING          },
      { StyleIdx::rhGuitarFingeringFrameWidth,        P_ID::FRAME_WIDTH            },
      { StyleIdx::rhGuitarFingeringFrameRound,        P_ID::FRAME_ROUND            },
      { StyleIdx::rhGuitarFingeringFrameFgColor,      P_ID::FRAME_FG_COLOR         },
      { StyleIdx::rhGuitarFingeringFrameBgColor,      P_ID::FRAME_BG_COLOR         },
      { StyleIdx::rhGuitarFingeringOffset,            P_ID::OFFSET                 },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> stringNumberStyle {
      { StyleIdx::stringNumberFontFace,               P_ID::FONT_FACE              },
      { StyleIdx::stringNumberFontSize,               P_ID::FONT_SIZE              },
      { StyleIdx::stringNumberFontBold,               P_ID::FONT_BOLD              },
      { StyleIdx::stringNumberFontItalic,             P_ID::FONT_ITALIC            },
      { StyleIdx::stringNumberFontUnderline,          P_ID::FONT_UNDERLINE         },
      { StyleIdx::stringNumberAlign,                  P_ID::ALIGN                  },
      { StyleIdx::stringNumberFrame,                  P_ID::FRAME                  },
      { StyleIdx::stringNumberFrameSquare,            P_ID::FRAME_SQUARE           },
      { StyleIdx::stringNumberFrameCircle,            P_ID::FRAME_CIRCLE           },
      { StyleIdx::stringNumberFramePadding,           P_ID::FRAME_PADDING          },
      { StyleIdx::stringNumberFrameWidth,             P_ID::FRAME_WIDTH            },
      { StyleIdx::stringNumberFrameRound,             P_ID::FRAME_ROUND            },
      { StyleIdx::stringNumberFrameFgColor,           P_ID::FRAME_FG_COLOR         },
      { StyleIdx::stringNumberFrameBgColor,           P_ID::FRAME_BG_COLOR         },
      { StyleIdx::stringNumberOffset,                 P_ID::OFFSET                 },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> longInstrumentStyle {
      { StyleIdx::longInstrumentFontFace,             P_ID::FONT_FACE              },
      { StyleIdx::longInstrumentFontSize,             P_ID::FONT_SIZE              },
      { StyleIdx::longInstrumentFontBold,             P_ID::FONT_BOLD              },
      { StyleIdx::longInstrumentFontItalic,           P_ID::FONT_ITALIC            },
      { StyleIdx::longInstrumentFontUnderline,        P_ID::FONT_UNDERLINE         },
      { StyleIdx::longInstrumentAlign,                P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> shortInstrumentStyle {
      { StyleIdx::shortInstrumentFontFace,            P_ID::FONT_FACE              },
      { StyleIdx::shortInstrumentFontSize,            P_ID::FONT_SIZE              },
      { StyleIdx::shortInstrumentFontBold,            P_ID::FONT_BOLD              },
      { StyleIdx::shortInstrumentFontItalic,          P_ID::FONT_ITALIC            },
      { StyleIdx::shortInstrumentFontUnderline,       P_ID::FONT_UNDERLINE         },
      { StyleIdx::shortInstrumentAlign,               P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> partInstrumentStyle {
      { StyleIdx::partInstrumentFontFace,             P_ID::FONT_FACE              },
      { StyleIdx::partInstrumentFontSize,             P_ID::FONT_SIZE              },
      { StyleIdx::partInstrumentFontBold,             P_ID::FONT_BOLD              },
      { StyleIdx::partInstrumentFontItalic,           P_ID::FONT_ITALIC            },
      { StyleIdx::partInstrumentFontUnderline,        P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> dynamicsStyle {
      { StyleIdx::dynamicsFontFace,                   P_ID::FONT_FACE              },
      { StyleIdx::dynamicsFontSize,                   P_ID::FONT_SIZE              },
      { StyleIdx::dynamicsFontBold,                   P_ID::FONT_BOLD              },
      { StyleIdx::dynamicsFontItalic,                 P_ID::FONT_ITALIC            },
      { StyleIdx::dynamicsFontUnderline,              P_ID::FONT_UNDERLINE         },
      { StyleIdx::dynamicsAlign,                      P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> expressionStyle {
      { StyleIdx::expressionFontFace,                 P_ID::FONT_FACE              },
      { StyleIdx::expressionFontSize,                 P_ID::FONT_SIZE              },
      { StyleIdx::expressionFontBold,                 P_ID::FONT_BOLD              },
      { StyleIdx::expressionFontItalic,               P_ID::FONT_ITALIC            },
      { StyleIdx::expressionFontUnderline,            P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> tempoStyle {
      { StyleIdx::tempoFontFace,                      P_ID::FONT_FACE              },
      { StyleIdx::tempoFontSize,                      P_ID::FONT_SIZE              },
      { StyleIdx::tempoFontBold,                      P_ID::FONT_BOLD              },
      { StyleIdx::tempoFontItalic,                    P_ID::FONT_ITALIC            },
      { StyleIdx::tempoFontUnderline,                 P_ID::FONT_UNDERLINE         },
      { StyleIdx::tempoOffset,                        P_ID::OFFSET                 },
      { StyleIdx::tempoSystemFlag,                    P_ID::SYSTEM_FLAG            },
      { StyleIdx::tempoAlign,                         P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> metronomeStyle {
      { StyleIdx::metronomeFontFace,                  P_ID::FONT_FACE              },
      { StyleIdx::metronomeFontSize,                  P_ID::FONT_SIZE              },
      { StyleIdx::metronomeFontBold,                  P_ID::FONT_BOLD              },
      { StyleIdx::metronomeFontItalic,                P_ID::FONT_ITALIC            },
      { StyleIdx::metronomeFontUnderline,             P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> measureNumberStyle {
      { StyleIdx::measureNumberFontFace,              P_ID::FONT_FACE              },
      { StyleIdx::measureNumberFontSize,              P_ID::FONT_SIZE              },
      { StyleIdx::measureNumberFontBold,              P_ID::FONT_BOLD              },
      { StyleIdx::measureNumberFontItalic,            P_ID::FONT_ITALIC            },
      { StyleIdx::measureNumberFontUnderline,         P_ID::FONT_UNDERLINE         },
      { StyleIdx::measureNumberOffset,                P_ID::OFFSET                 },
      { StyleIdx::measureNumberOffsetType,            P_ID::OFFSET_TYPE            },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> translatorStyle {
      { StyleIdx::translatorFontFace,                 P_ID::FONT_FACE              },
      { StyleIdx::translatorFontSize,                 P_ID::FONT_SIZE              },
      { StyleIdx::translatorFontBold,                 P_ID::FONT_BOLD              },
      { StyleIdx::translatorFontItalic,               P_ID::FONT_ITALIC            },
      { StyleIdx::translatorFontUnderline,            P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> tupletStyle {
      { StyleIdx::tupletFontFace,                     P_ID::FONT_FACE              },
      { StyleIdx::tupletFontSize,                     P_ID::FONT_SIZE              },
      { StyleIdx::tupletFontBold,                     P_ID::FONT_BOLD              },
      { StyleIdx::tupletFontItalic,                   P_ID::FONT_ITALIC            },
      { StyleIdx::tupletFontUnderline,                P_ID::FONT_UNDERLINE         },
      { StyleIdx::tupletAlign,                        P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> systemStyle {
      { StyleIdx::systemFontFace,                     P_ID::FONT_FACE              },
      { StyleIdx::systemFontSize,                     P_ID::FONT_SIZE              },
      { StyleIdx::systemFontBold,                     P_ID::FONT_BOLD              },
      { StyleIdx::systemFontItalic,                   P_ID::FONT_ITALIC            },
      { StyleIdx::systemFontUnderline,                P_ID::FONT_UNDERLINE         },
      { StyleIdx::systemOffset,                       P_ID::OFFSET                 },
      { StyleIdx::systemOffsetType,                   P_ID::OFFSET_TYPE            },
      { StyleIdx::systemAlign,                        P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> staffStyle {
      { StyleIdx::staffTextFontFace,                  P_ID::FONT_FACE              },
      { StyleIdx::staffTextFontSize,                  P_ID::FONT_SIZE              },
      { StyleIdx::staffTextFontBold,                  P_ID::FONT_BOLD              },
      { StyleIdx::staffTextFontItalic,                P_ID::FONT_ITALIC            },
      { StyleIdx::staffTextFontUnderline,             P_ID::FONT_UNDERLINE         },
      { StyleIdx::staffTextAlign,                     P_ID::ALIGN                  },
      { StyleIdx::staffTextOffset,                    P_ID::OFFSET                 },
      { StyleIdx::staffTextOffsetType,                P_ID::OFFSET_TYPE            },
      { StyleIdx::staffTextPlacement,                 P_ID::PLACEMENT              },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> chordSymbolStyle {
      { StyleIdx::chordSymbolFontFace,                P_ID::FONT_FACE              },
      { StyleIdx::chordSymbolFontSize,                P_ID::FONT_SIZE              },
      { StyleIdx::chordSymbolFontBold,                P_ID::FONT_BOLD              },
      { StyleIdx::chordSymbolFontItalic,              P_ID::FONT_ITALIC            },
      { StyleIdx::chordSymbolFontUnderline,           P_ID::FONT_UNDERLINE         },
      { StyleIdx::chordSymbolAlign,                   P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> rehearsalMarkStyle {
      { StyleIdx::rehearsalMarkFontFace,              P_ID::FONT_FACE              },
      { StyleIdx::rehearsalMarkFontSize,              P_ID::FONT_SIZE              },
      { StyleIdx::rehearsalMarkFontBold,              P_ID::FONT_BOLD              },
      { StyleIdx::rehearsalMarkFontItalic,            P_ID::FONT_ITALIC            },
      { StyleIdx::rehearsalMarkFontUnderline,         P_ID::FONT_UNDERLINE         },
      { StyleIdx::rehearsalMarkAlign,                 P_ID::ALIGN                  },
      { StyleIdx::rehearsalMarkFrame,                 P_ID::FRAME                  },
      { StyleIdx::rehearsalMarkFrameSquare,           P_ID::FRAME_SQUARE           },
      { StyleIdx::rehearsalMarkFrameCircle,           P_ID::FRAME_CIRCLE           },
      { StyleIdx::rehearsalMarkFramePadding,          P_ID::FRAME_PADDING          },
      { StyleIdx::rehearsalMarkFrameWidth,            P_ID::FRAME_WIDTH            },
      { StyleIdx::rehearsalMarkFrameRound,            P_ID::FRAME_ROUND            },
      { StyleIdx::rehearsalMarkFrameFgColor,          P_ID::FRAME_FG_COLOR         },
      { StyleIdx::rehearsalMarkFrameBgColor,          P_ID::FRAME_BG_COLOR         },
      { StyleIdx::rehearsalMarkPlacement,             P_ID::PLACEMENT              },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> repeatLeftStyle {
      { StyleIdx::repeatLeftFontFace,                 P_ID::FONT_FACE              },
      { StyleIdx::repeatLeftFontSize,                 P_ID::FONT_SIZE              },
      { StyleIdx::repeatLeftFontBold,                 P_ID::FONT_BOLD              },
      { StyleIdx::repeatLeftFontItalic,               P_ID::FONT_ITALIC            },
      { StyleIdx::repeatLeftFontUnderline,            P_ID::FONT_UNDERLINE         },
      { StyleIdx::repeatLeftAlign,                    P_ID::ALIGN                  },
      { StyleIdx::repeatLeftPlacement,                P_ID::PLACEMENT              },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> repeatRightStyle {
      { StyleIdx::repeatRightFontFace,                P_ID::FONT_FACE              },
      { StyleIdx::repeatRightFontSize,                P_ID::FONT_SIZE              },
      { StyleIdx::repeatRightFontBold,                P_ID::FONT_BOLD              },
      { StyleIdx::repeatRightFontItalic,              P_ID::FONT_ITALIC            },
      { StyleIdx::repeatRightFontUnderline,           P_ID::FONT_UNDERLINE         },
      { StyleIdx::repeatRightAlign,                   P_ID::ALIGN                  },
      { StyleIdx::repeatRightPlacement,               P_ID::PLACEMENT              },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> frameStyle {
      { StyleIdx::frameFontFace,                      P_ID::FONT_FACE              },
      { StyleIdx::frameFontSize,                      P_ID::FONT_SIZE              },
      { StyleIdx::frameFontBold,                      P_ID::FONT_BOLD              },
      { StyleIdx::frameFontItalic,                    P_ID::FONT_ITALIC            },
      { StyleIdx::frameFontUnderline,                 P_ID::FONT_UNDERLINE         },
      { StyleIdx::frameAlign,                         P_ID::ALIGN                  },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> textLineStyle {
      { StyleIdx::textLineFontFace,                   P_ID::FONT_FACE              },
      { StyleIdx::textLineFontSize,                   P_ID::FONT_SIZE              },
      { StyleIdx::textLineFontBold,                   P_ID::FONT_BOLD              },
      { StyleIdx::textLineFontItalic,                 P_ID::FONT_ITALIC            },
      { StyleIdx::textLineFontUnderline,              P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> glissandoStyle {
      { StyleIdx::glissandoFontFace,                  P_ID::FONT_FACE              },
      { StyleIdx::glissandoFontSize,                  P_ID::FONT_SIZE              },
      { StyleIdx::glissandoFontBold,                  P_ID::FONT_BOLD              },
      { StyleIdx::glissandoFontItalic,                P_ID::FONT_ITALIC            },
      { StyleIdx::glissandoFontUnderline,             P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> ottavaStyle {
      { StyleIdx::ottavaFontFace,                     P_ID::FONT_FACE              },
      { StyleIdx::ottavaFontSize,                     P_ID::FONT_SIZE              },
      { StyleIdx::ottavaFontBold,                     P_ID::FONT_BOLD              },
      { StyleIdx::ottavaFontItalic,                   P_ID::FONT_ITALIC            },
      { StyleIdx::ottavaFontUnderline,                P_ID::FONT_UNDERLINE         },
      { StyleIdx::ottavaTextAlign,                    P_ID::BEGIN_TEXT_ALIGN       },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> pedalStyle {
      { StyleIdx::pedalFontFace,                      P_ID::BEGIN_FONT_FACE        },
      { StyleIdx::pedalFontFace,                      P_ID::CONTINUE_FONT_FACE     },
      { StyleIdx::pedalFontFace,                      P_ID::END_FONT_FACE          },
      { StyleIdx::pedalFontSize,                      P_ID::BEGIN_FONT_SIZE        },
      { StyleIdx::pedalFontSize,                      P_ID::CONTINUE_FONT_SIZE     },
      { StyleIdx::pedalFontSize,                      P_ID::END_FONT_SIZE          },
      { StyleIdx::pedalFontBold,                      P_ID::BEGIN_FONT_BOLD        },
      { StyleIdx::pedalFontBold,                      P_ID::CONTINUE_FONT_BOLD     },
      { StyleIdx::pedalFontBold,                      P_ID::END_FONT_BOLD          },
      { StyleIdx::pedalFontItalic,                    P_ID::BEGIN_FONT_ITALIC      },
      { StyleIdx::pedalFontItalic,                    P_ID::CONTINUE_FONT_ITALIC   },
      { StyleIdx::pedalFontItalic,                    P_ID::END_FONT_ITALIC        },
      { StyleIdx::pedalFontUnderline,                 P_ID::BEGIN_FONT_UNDERLINE   },
      { StyleIdx::pedalFontUnderline,                 P_ID::CONTINUE_FONT_UNDERLINE},
      { StyleIdx::pedalFontUnderline,                 P_ID::END_FONT_UNDERLINE     },
      { StyleIdx::pedalTextAlign,                     P_ID::BEGIN_TEXT_ALIGN       },
      { StyleIdx::pedalTextAlign,                     P_ID::CONTINUE_TEXT_ALIGN    },
      { StyleIdx::pedalHookHeight,                    P_ID::BEGIN_HOOK_HEIGHT      },
      { StyleIdx::pedalHookHeight,                    P_ID::END_HOOK_HEIGHT        },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> letRingStyle {
      { StyleIdx::letRingFontFace,                      P_ID::BEGIN_FONT_FACE        },
      { StyleIdx::letRingFontFace,                      P_ID::CONTINUE_FONT_FACE     },
      { StyleIdx::letRingFontFace,                      P_ID::END_FONT_FACE          },
      { StyleIdx::letRingFontSize,                      P_ID::BEGIN_FONT_SIZE        },
      { StyleIdx::letRingFontSize,                      P_ID::CONTINUE_FONT_SIZE     },
      { StyleIdx::letRingFontSize,                      P_ID::END_FONT_SIZE          },
      { StyleIdx::letRingFontBold,                      P_ID::BEGIN_FONT_BOLD        },
      { StyleIdx::letRingFontBold,                      P_ID::CONTINUE_FONT_BOLD     },
      { StyleIdx::letRingFontBold,                      P_ID::END_FONT_BOLD          },
      { StyleIdx::letRingFontItalic,                    P_ID::BEGIN_FONT_ITALIC      },
      { StyleIdx::letRingFontItalic,                    P_ID::CONTINUE_FONT_ITALIC   },
      { StyleIdx::letRingFontItalic,                    P_ID::END_FONT_ITALIC        },
      { StyleIdx::letRingFontUnderline,                 P_ID::BEGIN_FONT_UNDERLINE   },
      { StyleIdx::letRingFontUnderline,                 P_ID::CONTINUE_FONT_UNDERLINE},
      { StyleIdx::letRingFontUnderline,                 P_ID::END_FONT_UNDERLINE     },
      { StyleIdx::letRingTextAlign,                     P_ID::BEGIN_TEXT_ALIGN       },
      { StyleIdx::letRingTextAlign,                     P_ID::CONTINUE_TEXT_ALIGN    },
      { StyleIdx::letRingHookHeight,                    P_ID::BEGIN_HOOK_HEIGHT      },
      { StyleIdx::letRingHookHeight,                    P_ID::END_HOOK_HEIGHT        },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> palmMuteStyle {
      { StyleIdx::palmMuteFontFace,                      P_ID::BEGIN_FONT_FACE        },
      { StyleIdx::palmMuteFontFace,                      P_ID::CONTINUE_FONT_FACE     },
      { StyleIdx::palmMuteFontFace,                      P_ID::END_FONT_FACE          },
      { StyleIdx::palmMuteFontSize,                      P_ID::BEGIN_FONT_SIZE        },
      { StyleIdx::palmMuteFontSize,                      P_ID::CONTINUE_FONT_SIZE     },
      { StyleIdx::palmMuteFontSize,                      P_ID::END_FONT_SIZE          },
      { StyleIdx::palmMuteFontBold,                      P_ID::BEGIN_FONT_BOLD        },
      { StyleIdx::palmMuteFontBold,                      P_ID::CONTINUE_FONT_BOLD     },
      { StyleIdx::palmMuteFontBold,                      P_ID::END_FONT_BOLD          },
      { StyleIdx::palmMuteFontItalic,                    P_ID::BEGIN_FONT_ITALIC      },
      { StyleIdx::palmMuteFontItalic,                    P_ID::CONTINUE_FONT_ITALIC   },
      { StyleIdx::palmMuteFontItalic,                    P_ID::END_FONT_ITALIC        },
      { StyleIdx::palmMuteFontUnderline,                 P_ID::BEGIN_FONT_UNDERLINE   },
      { StyleIdx::palmMuteFontUnderline,                 P_ID::CONTINUE_FONT_UNDERLINE},
      { StyleIdx::palmMuteFontUnderline,                 P_ID::END_FONT_UNDERLINE     },
      { StyleIdx::palmMuteTextAlign,                     P_ID::BEGIN_TEXT_ALIGN       },
      { StyleIdx::palmMuteTextAlign,                     P_ID::CONTINUE_TEXT_ALIGN    },
      { StyleIdx::palmMuteHookHeight,                    P_ID::BEGIN_HOOK_HEIGHT      },
      { StyleIdx::palmMuteHookHeight,                    P_ID::END_HOOK_HEIGHT        },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> hairpinStyle {
      { StyleIdx::hairpinFontFace,                    P_ID::BEGIN_FONT_FACE            },
      { StyleIdx::hairpinFontSize,                    P_ID::BEGIN_FONT_SIZE            },
      { StyleIdx::hairpinFontBold,                    P_ID::BEGIN_FONT_BOLD            },
      { StyleIdx::hairpinFontItalic,                  P_ID::BEGIN_FONT_ITALIC          },
      { StyleIdx::hairpinFontUnderline,               P_ID::BEGIN_FONT_UNDERLINE       },
      { StyleIdx::hairpinTextAlign,                   P_ID::BEGIN_TEXT_ALIGN           },
      { StyleIdx::hairpinFontFace,                    P_ID::CONTINUE_FONT_FACE         },
      { StyleIdx::hairpinFontSize,                    P_ID::CONTINUE_FONT_SIZE         },
      { StyleIdx::hairpinFontBold,                    P_ID::CONTINUE_FONT_BOLD         },
      { StyleIdx::hairpinFontItalic,                  P_ID::CONTINUE_FONT_ITALIC       },
      { StyleIdx::hairpinFontUnderline,               P_ID::CONTINUE_FONT_UNDERLINE    },
      { StyleIdx::hairpinTextAlign,                   P_ID::CONTINUE_TEXT_ALIGN        },
      { StyleIdx::hairpinFontFace,                    P_ID::END_FONT_FACE              },
      { StyleIdx::hairpinFontSize,                    P_ID::END_FONT_SIZE              },
      { StyleIdx::hairpinFontBold,                    P_ID::END_FONT_BOLD              },
      { StyleIdx::hairpinFontItalic,                  P_ID::END_FONT_ITALIC            },
      { StyleIdx::hairpinFontUnderline,               P_ID::END_FONT_UNDERLINE         },
      { StyleIdx::hairpinTextAlign,                   P_ID::END_TEXT_ALIGN             },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> bendStyle {
      { StyleIdx::bendFontFace,                       P_ID::FONT_FACE              },
      { StyleIdx::bendFontSize,                       P_ID::FONT_SIZE              },
      { StyleIdx::bendFontBold,                       P_ID::FONT_BOLD              },
      { StyleIdx::bendFontItalic,                     P_ID::FONT_ITALIC            },
      { StyleIdx::bendFontUnderline,                  P_ID::FONT_UNDERLINE         },
      { StyleIdx::bendLineWidth,                      P_ID::LINE_WIDTH             },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> headerStyle {
      { StyleIdx::headerFontFace,                     P_ID::FONT_FACE              },
      { StyleIdx::headerFontSize,                     P_ID::FONT_SIZE              },
      { StyleIdx::headerFontBold,                     P_ID::FONT_BOLD              },
      { StyleIdx::headerFontItalic,                   P_ID::FONT_ITALIC            },
      { StyleIdx::headerFontUnderline,                P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> footerStyle {
      { StyleIdx::footerFontFace,                     P_ID::FONT_FACE              },
      { StyleIdx::footerFontSize,                     P_ID::FONT_SIZE              },
      { StyleIdx::footerFontBold,                     P_ID::FONT_BOLD              },
      { StyleIdx::footerFontItalic,                   P_ID::FONT_ITALIC            },
      { StyleIdx::footerFontUnderline,                P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> instrumentChangeStyle {
      { StyleIdx::instrumentChangeFontFace,           P_ID::FONT_FACE              },
      { StyleIdx::instrumentChangeFontSize,           P_ID::FONT_SIZE              },
      { StyleIdx::instrumentChangeFontBold,           P_ID::FONT_BOLD              },
      { StyleIdx::instrumentChangeFontItalic,         P_ID::FONT_ITALIC            },
      { StyleIdx::instrumentChangeFontUnderline,      P_ID::FONT_UNDERLINE         },
      { StyleIdx::instrumentChangeAlign,              P_ID::ALIGN                  },
      { StyleIdx::instrumentChangeOffset,             P_ID::OFFSET                 },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> figuredBassStyle {
      { StyleIdx::figuredBassFontFace,                P_ID::FONT_FACE              },
      { StyleIdx::figuredBassFontSize,                P_ID::FONT_SIZE              },
      { StyleIdx::figuredBassFontBold,                P_ID::FONT_BOLD              },
      { StyleIdx::figuredBassFontItalic,              P_ID::FONT_ITALIC            },
      { StyleIdx::figuredBassFontUnderline,           P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> user1Style {
      { StyleIdx::user1FontFace,                      P_ID::FONT_FACE              },
      { StyleIdx::user1FontSize,                      P_ID::FONT_SIZE              },
      { StyleIdx::user1FontBold,                      P_ID::FONT_BOLD              },
      { StyleIdx::user1FontItalic,                    P_ID::FONT_ITALIC            },
      { StyleIdx::user1FontUnderline,                 P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
      };

const std::vector<StyledProperty> user2Style {
      { StyleIdx::user2FontFace,                      P_ID::FONT_FACE              },
      { StyleIdx::user2FontSize,                      P_ID::FONT_SIZE              },
      { StyleIdx::user2FontBold,                      P_ID::FONT_BOLD              },
      { StyleIdx::user2FontItalic,                    P_ID::FONT_ITALIC            },
      { StyleIdx::user2FontUnderline,                 P_ID::FONT_UNDERLINE         },
      { StyleIdx::NOSTYLE,                            P_ID::END                    }      // end of list marker
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
      { QT_TRANSLATE_NOOP("TextStyle", "Pedal"),                   &pedalStyle,                     SubStyleId::PEDAL },
      { QT_TRANSLATE_NOOP("TextStyle", "LetRing"),                 &letRingStyle,                   SubStyleId::LET_RING },
      { QT_TRANSLATE_NOOP("TextStyle", "PalmMute"),                &palmMuteStyle,                  SubStyleId::PALM_MUTE },
      { QT_TRANSLATE_NOOP("TextStyle", "Hairpin"),                 &hairpinStyle,                   SubStyleId::HAIRPIN },
      { QT_TRANSLATE_NOOP("TextStyle", "Bend"),                    &bendStyle,                      SubStyleId::BEND },
      { QT_TRANSLATE_NOOP("TextStyle", "Header"),                  &headerStyle,                    SubStyleId::HEADER },
      { QT_TRANSLATE_NOOP("TextStyle", "Footer"),                  &footerStyle,                    SubStyleId::FOOTER },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Change"),       &instrumentChangeStyle,          SubStyleId::INSTRUMENT_CHANGE },
      { QT_TRANSLATE_NOOP("TextStyle", "Figured Bass"),            &figuredBassStyle,               SubStyleId::FIGURED_BASS },
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

const char* MStyle::valueType(const StyleIdx i)
      {
      return styleTypes[int(i)].valueType();
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

QVariant MStyle::value(StyleIdx idx) const
      {
      return _values[int(idx)];
      }

//---------------------------------------------------------
//   valueName
//---------------------------------------------------------

const char* MStyle::valueName(const StyleIdx i)
      {
      return styleTypes[int(i)].name();
      }

//---------------------------------------------------------
//   styleIdx
//---------------------------------------------------------

StyleIdx MStyle::styleIdx(const QString &name)
      {
      for (StyleType st : styleTypes) {
            if (st.name() == name)
                  return st.styleIdx();
            }
      return StyleIdx::NOSTYLE;
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
      qreal _spatium = value(StyleIdx::spatium).toDouble();
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

bool MStyle::isDefault(StyleIdx idx) const
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

void MStyle::set(const StyleIdx t, const QVariant& val)
      {
      const int idx = int(t);
      _values[idx] = val;
      if (t == StyleIdx::spatium)
            precomputeValues();
      else {
            if (!strcmp(styleTypes[idx].valueType(), "Ms::Spatium")) {
                  qreal _spatium = value(StyleIdx::spatium).toDouble();
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
            StyleIdx idx = t.styleIdx();
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
      QString oldChordDescriptionFile = value(StyleIdx::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "TextStyle") {
                  readTextStyle206(this, e);        // obsolete
                  }
            else if (tag == "Spatium")
                  set(StyleIdx::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout") {    // obsolete
                  readPageFormat(this, e);      // from read206.cpp
                  }
            else if (tag == "displayInConcertPitch")
                  set(StyleIdx::concertPitch, QVariant(bool(e.readInt())));
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

      QString newChordDescriptionFile = value(StyleIdx::chordDescriptionFile).toString();
      if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
            if (!newChordDescriptionFile.startsWith("chords_") && value(StyleIdx::chordStyle).toString() == "std") {
                  // should not normally happen,
                  // but treat as "old" (114) score just in case
                  set(StyleIdx::chordStyle, QVariant(QString("custom")));
                  set(StyleIdx::chordsXmlFile, QVariant(true));
                  qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
                  }
            if (value(StyleIdx::chordStyle).toString() == "custom")
                  _customChordList = true;
            else
                  _customChordList = false;
            _chordList.unload();
            }

      // make sure we have a chordlist
      if (!_chordList.loaded() && !chordListTag) {
            if (value(StyleIdx::chordsXmlFile).toBool())
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
            StyleIdx idx = st.styleIdx();
            if (idx == StyleIdx::spatium)       // special handling for spatium
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
      xml.tag("Spatium", value(StyleIdx::spatium).toDouble() / DPMM);
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
