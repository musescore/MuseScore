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
#include "read206.h"
#include "undo.h"

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
      Sid  styleIdx() const                 { return _idx;          }
      int idx() const                       { return int(_idx);     }
      const char*  valueType() const        { return _defaultValue.typeName();    }
      const char*      name() const         { return _name;         }
      const QVariant&  defaultValue() const { return _defaultValue; }
      };

//---------------------------------------------------------
//   styleTypes
//
//    Keep in sync with sid in style.h
//---------------------------------------------------------


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
      { Sid::lyricsPosAbove,          "lyricsPosAbove",          QPointF(0.0, -2.0) },
      { Sid::lyricsPosBelow,          "lyricsPosBelow",          QPointF(.0, 3.0) },
      { Sid::lyricsMinTopDistance,    "lyricsMinTopDistance",    Spatium(1.0)  },
      { Sid::lyricsMinBottomDistance, "lyricsMinBottomDistance", Spatium(2.0)  },
      { Sid::lyricsLineHeight,        "lyricsLineHeight",        1.0 },
      { Sid::lyricsDashMinLength,     "lyricsDashMinLength",     Spatium(0.4) },
      { Sid::lyricsDashMaxLength,     "lyricsDashMaxLegth",      Spatium(0.8) },
      { Sid::lyricsDashMaxDistance,   "lyricsDashMaxDistance",   Spatium(16.0) },
      { Sid::lyricsDashForce,         "lyricsDashForce",         QVariant(true) },
      { Sid::lyricsAlignVerseNumber,  "lyricsAlignVerseNumber",  true },
      { Sid::lyricsLineThickness,     "lyricsLineThickness",     Spatium(0.1) },
      { Sid::lyricsMelismaAlign,      "lyricsMelismaAlign",      QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::lyricsMelismaPad,        "lyricsMelismaPad",        Spatium(0.10) }, // the empty space before a melisma line
      { Sid::lyricsDashPad,           "lyricsDashPad",           Spatium(0.05) }, // the min. empty space before and after a dash
      { Sid::lyricsDashLineThickness, "lyricsDashLineThickness", Spatium(0.15) }, // in sp. units
      { Sid::lyricsDashYposRatio,     "lyricsDashYposRatio",     0.67          }, // the fraction of lyrics font x-height to raise the dashes above text base line

      { Sid::lyricsOddFontFace,       "lyricsOddFontFace",       "FreeSerif" },
      { Sid::lyricsOddFontSize,       "lyricsOddFontSize",       11.0 },
      { Sid::lyricsOddFontStyle,      "lyricsOddFontStyle",      int(FontStyle::Normal) },
      { Sid::lyricsOddAlign,          "lyricsOddAlign",          QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::lyricsOddFrameType,      "lyricsOddFrameType",      int(FrameType::NO_FRAME) },
      { Sid::lyricsOddFramePadding,   "lyricsOddFramePadding",   0.2 },
      { Sid::lyricsOddFrameWidth,     "lyricsOddFrameWidth",     0.1 },
      { Sid::lyricsOddFrameRound,     "lyricsOddFrameRound",     0 },
      { Sid::lyricsOddFrameFgColor,   "lyricsOddFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::lyricsOddFrameBgColor,   "lyricsOddFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::lyricsEvenFontFace,      "lyricsEvenFontFace",      "FreeSerif" },
      { Sid::lyricsEvenFontSize,      "lyricsEvenFontSize",      11.0 },
      { Sid::lyricsEvenFontStyle,     "lyricsEvenFontStyle",     int(FontStyle::Normal) },
      { Sid::lyricsEvenAlign,         "lyricsEvenAlign",         QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::lyricsEvenFrameType,     "lyricsOddFrameType",      int(FrameType::NO_FRAME) },
      { Sid::lyricsEvenFramePadding,  "lyricsOddFramePadding",   0.2 },
      { Sid::lyricsEvenFrameWidth,    "lyricsOddFrameWidth",     0.1 },
      { Sid::lyricsEvenFrameRound,    "lyricsOddFrameRound",     0 },
      { Sid::lyricsEvenFrameFgColor,  "lyricsOddFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::lyricsEvenFrameBgColor,  "lyricsOddFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::figuredBassFontFamily,   "figuredBassFontFamily",   QString("MScoreBC") },

//      { Sid::figuredBassFontSize,     "figuredBassFontSize",     QVariant(8.0) },
      { Sid::figuredBassYOffset,      "figuredBassYOffset",      QVariant(6.0) },
      { Sid::figuredBassLineHeight,   "figuredBassLineHeight",   QVariant(1.0) },
      { Sid::figuredBassAlignment,    "figuredBassAlignment",    QVariant(0) },
      { Sid::figuredBassStyle,        "figuredBassStyle" ,       QVariant(0) },
      { Sid::systemFrameDistance,     "systemFrameDistance",     Spatium(7.0) },
      { Sid::frameSystemDistance,     "frameSystemDistance",     Spatium(7.0) },
      { Sid::minMeasureWidth,         "minMeasureWidth",         Spatium(5.0) },
      { Sid::barWidth,                "barWidth",                Spatium(0.16) },
      { Sid::doubleBarWidth,          "doubleBarWidth",          Spatium(0.16) },

      { Sid::endBarWidth,             "endBarWidth",             Spatium(0.5) },
      { Sid::doubleBarDistance,       "doubleBarDistance",       Spatium(.40 + .16) },
      { Sid::endBarDistance,          "endBarDistance",          Spatium(.40 + (.16 + .50) * .5) },
      { Sid::repeatBarlineDotSeparation, "repeatBarlineDotSeparation", Spatium(.40 + .46 * .5) },
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
      { Sid::articulationPosAbove,    "articulationPosAbove",    QPointF(0.0, 0.0) },
      { Sid::lastSystemFillLimit,     "lastSystemFillLimit",     QVariant(0.3) },

      { Sid::hairpinPlacement,        "hairpinPlacement",        int(Placement::BELOW)  },
      { Sid::hairpinPosAbove,         "hairpinPosAbove",         QPointF(0.0, -3.5) },
      { Sid::hairpinPosBelow,         "hairpinPosBelow",         QPointF(.0, 3.5) },
      { Sid::hairpinHeight,           "hairpinHeight",           Spatium(1.2) },
      { Sid::hairpinContHeight,       "hairpinContHeight",       Spatium(0.5) },
      { Sid::hairpinLineWidth,        "hairpinWidth",            Spatium(0.13) },
      { Sid::hairpinFontFace,         "hairpinFontFace",         "FreeSerif" },
      { Sid::hairpinFontSize,         "hairpinFontSize",         12.0 },
      { Sid::hairpinFontStyle,        "hairpinFontStyle",        int(FontStyle::Italic) },
      { Sid::hairpinTextAlign,        "hairpinTextAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::hairpinFrameType,        "hairpinFrameType",        int(FrameType::NO_FRAME) },
      { Sid::hairpinFramePadding,     "hairpinFramePadding",     0.2 },
      { Sid::hairpinFrameWidth,       "hairpinFrameWidth",       0.1 },
      { Sid::hairpinFrameRound,       "hairpinFrameRound",       0 },
      { Sid::hairpinFrameFgColor,     "hairpinFrameFgColor",     QColor(0, 0, 0, 255) },
      { Sid::hairpinFrameBgColor,     "hairpinFrameBgColor",     QColor(255, 255, 255, 0) },

      { Sid::pedalPlacement,          "pedalPlacement",          int(Placement::BELOW)  },
      { Sid::pedalPosAbove,           "pedalPosAbove",           QPointF(.0, -4) },
      { Sid::pedalPosBelow,           "pedalPosBelow",           QPointF(.0, 4) },
      { Sid::pedalLineWidth,          "pedalLineWidth",          Spatium(.15) },
      { Sid::pedalLineStyle,          "pedalListStyle",          QVariant(int(Qt::SolidLine)) },
      { Sid::pedalBeginTextOffset,    "pedalBeginTextOffset",    QPointF(0.0, 0.15) },
      { Sid::pedalHookHeight,         "pedalHookHeight",         Spatium(-1.2) },
      { Sid::pedalFontFace,           "pedalFontFace",           "FreeSerif" },
      { Sid::pedalFontSize,           "pedalFontSize",           12.0 },
      { Sid::pedalFontStyle,          "pedalFontStyle",          int(FontStyle::Normal) },
      { Sid::pedalTextAlign,          "pedalTextAlign",          QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::pedalFrameType,          "pedalFrameType",          int(FrameType::NO_FRAME) },
      { Sid::pedalFramePadding,       "pedalFramePadding",       0.2 },
      { Sid::pedalFrameWidth,         "pedalFrameWidth",         0.1 },
      { Sid::pedalFrameRound,         "pedalFrameRound",         0 },
      { Sid::pedalFrameFgColor,       "pedalFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::pedalFrameBgColor,       "pedalFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::trillPlacement,          "trillPlacement",          int(Placement::ABOVE)  },
      { Sid::trillPosAbove,           "trillPosAbove",           QPointF(.0, -1) },
      { Sid::trillPosBelow,           "trillPosBelow",           QPointF(.0, 1) },

      { Sid::vibratoPlacement,        "vibratoPlacement",        int(Placement::ABOVE)  },
      { Sid::vibratoPosAbove,         "vibratoPosAbove",         QPointF(.0, -1) },
      { Sid::vibratoPosBelow,         "vibratoPosBelow",         QPointF(.0, 1) },

      { Sid::harmonyFretDist,          "harmonyFretDist",        Spatium(0.5) },
      { Sid::minHarmonyDistance,       "minHarmonyDistance",     Spatium(0.5) },
      { Sid::maxHarmonyBarDistance,    "maxHarmonyBarDistance",  Spatium(3.0) },
      { Sid::harmonyPlacement,         "harmonyPlacement",       int(Placement::ABOVE) },

      { Sid::chordSymbolAPosAbove,      "chordSymbolPosAbove",       QPointF(.0, -2.5) },
      { Sid::chordSymbolAPosBelow,      "chordSymbolPosBelow",       QPointF(.0, 3.5) },

      { Sid::chordSymbolBPosAbove,      "chordSymbolBPosAbove",      QPointF(.0, -5.0) },
      { Sid::chordSymbolBPosBelow,      "chordSymbolBPosBelow",      QPointF(.0, 3.5) },

      { Sid::chordSymbolAFontFace,      "chordSymbolAFontFace",      "FreeSerif" },
      { Sid::chordSymbolAFontSize,      "chordSymbolAFontSize",      12.0 },
      { Sid::chordSymbolAFontStyle,     "chordSymbolAFontStyle",     int(FontStyle::Normal) },
      { Sid::chordSymbolAAlign,         "chordSymbolAAlign",         QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::chordSymbolAFrameType,     "chordSymbolAFrameType",     int(FrameType::NO_FRAME) },
      { Sid::chordSymbolAFramePadding,  "chordSymbolAFramePadding",  0.2 },
      { Sid::chordSymbolAFrameWidth,    "chordSymbolAFrameWidth",    0.1 },
      { Sid::chordSymbolAFrameRound,    "chordSymbolAFrameRound",    0 },
      { Sid::chordSymbolAFrameFgColor,  "chordSymbolAFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::chordSymbolAFrameBgColor,  "chordSymbolAFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::chordSymbolBFontFace,      "chordSymbolBFontFace",      "FreeSerif" },
      { Sid::chordSymbolBFontSize,      "chordSymbolBFontSize",      12.0 },
      { Sid::chordSymbolBFontStyle,     "chordSymbolBFontStyle",     int(FontStyle::Italic) },
      { Sid::chordSymbolBAlign,         "chordSymbolBAlign",         QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::chordSymbolBFrameType,     "chordSymbolBFrameType",     int(FrameType::NO_FRAME) },
      { Sid::chordSymbolBFramePadding,  "chordSymbolBFramePadding",  0.2 },
      { Sid::chordSymbolBFrameWidth,    "chordSymbolBFrameWidth",    0.1 },
      { Sid::chordSymbolBFrameRound,    "chordSymbolBFrameRound",    0 },
      { Sid::chordSymbolBFrameFgColor,  "chordSymbolBFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::chordSymbolBFrameBgColor,  "chordSymbolBFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::capoPosition,            "capoPosition",            QVariant(0) },
      { Sid::fretNumMag,              "fretNumMag",              QVariant(2.0) },
      { Sid::fretNumPos,              "fretNumPos",              QVariant(0) },
      { Sid::fretY,                   "fretY",                   Spatium(2.0) },
      { Sid::fretMinDistance,         "fretMinDistance",         Spatium(0.5) },
      { Sid::fretMag,                 "fretMag",                 QVariant(1.0) },
      { Sid::fretPlacement,           "fretPlacement",           int(Placement::ABOVE) },
      { Sid::fretStrings,             "fretStrings",             6 },
      { Sid::fretFrets,               "fretFrets",               5 },
      { Sid::fretOffset,              "fretOffset",              0 },
      { Sid::fretBarre,               "fretBarre",               0 },

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

      { Sid::voltaPosAbove,           "voltaPosAbove",           QPointF(0.0, -3.0) },
      { Sid::voltaHook,               "voltaHook",               Spatium(1.9) },
      { Sid::voltaLineWidth,          "voltaLineWidth",          Spatium(.1) },
      { Sid::voltaLineStyle,          "voltaLineStyle",          QVariant(int(Qt::SolidLine)) },
      { Sid::voltaFontFace,           "voltaFontFace",           "FreeSerif" },
      { Sid::voltaFontSize,           "voltaFontSize",           11.0 },
      { Sid::voltaFontStyle,          "voltaFontStyle",          int(FontStyle::Bold) },
      { Sid::voltaAlign,              "voltaAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::voltaOffset,             "voltaOffset",             QPointF(0.5, 1.9) },
      { Sid::voltaFrameType,          "voltaFrameType",          int(FrameType::NO_FRAME) },
      { Sid::voltaFramePadding,       "voltaFramePadding",       0.2 },
      { Sid::voltaFrameWidth,         "voltaFrameWidth",         0.1 },
      { Sid::voltaFrameRound,         "voltaFrameRound",         0 },
      { Sid::voltaFrameFgColor,       "voltaFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::voltaFrameBgColor,       "voltaFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::ottava8VAPlacement,      "ottava8VAPlacement",      int(Placement::ABOVE)  },
      { Sid::ottava8VBPlacement,      "ottava8VBPlacement",      int(Placement::BELOW)  },
      { Sid::ottava15MAPlacement,     "ottava15MAPlacement",     int(Placement::ABOVE)  },
      { Sid::ottava15MBPlacement,     "ottava15MBPlacement",     int(Placement::BELOW)  },
      { Sid::ottava22MAPlacement,     "ottava22MAPlacement",     int(Placement::ABOVE)  },
      { Sid::ottava22MBPlacement,     "ottava22MBPlacement",     int(Placement::BELOW)  },

      { Sid::ottava8VAText,           "ottava8VAText",           QString("<sym>ottavaAlta</sym>") },
      { Sid::ottava8VBText,           "ottava8VBText",           QString("<sym>ottavaBassaBa</sym>") },
      { Sid::ottava15MAText,          "ottava15MAText",          QString("<sym>quindicesimaAlta</sym>") },
      { Sid::ottava15MBText,          "ottava15MBText",          QString("<sym>quindicesimaBassa</sym>") },
      { Sid::ottava22MAText,          "ottava22MAText",          QString("<sym>ventiduesimaAlta</sym>") },
      { Sid::ottava22MBText,          "ottava22MBText",          QString("<sym>ventiduesimaBassa</sym>") },

      { Sid::ottava8VAnoText,         "ottava8VAnoText",         QString("<sym>ottava</sym>") },
      { Sid::ottava8VBnoText,         "ottava8VBnoText",         QString("<sym>ottava</sym>") },
      { Sid::ottava15MAnoText,        "ottava15MAnoText",        QString("<sym>quindicesima</sym>") },
      { Sid::ottava15MBnoText,        "ottava15MBnoText",        QString("<sym>quindicesima</sym>") },
      { Sid::ottava22MAnoText,        "ottava22MAnoText",        QString("<sym>ventiduesima</sym>") },
      { Sid::ottava22MBnoText,        "ottava22MBnoText",        QString("<sym>ventiduesima</sym>") },

      { Sid::ottavaPosAbove,          "ottavaPosAbove",          QPointF(.0, -3.0) },
      { Sid::ottavaPosBelow,          "ottavaPosBelow",          QPointF(.0, 3.0) },
      { Sid::ottavaHookAbove,         "ottavaHookAbove",         Spatium(1.9) },
      { Sid::ottavaHookBelow,         "ottavaHookBelow",         Spatium(-1.9) },
      { Sid::ottavaLineWidth,         "ottavaLineWidth",         Spatium(.1) },
      { Sid::ottavaLineStyle,         "ottavaLineStyle",         QVariant(int(Qt::DashLine)) },
      { Sid::ottavaNumbersOnly,       "ottavaNumbersOnly",       true },
      { Sid::ottavaFontFace,          "ottavaFontFace",          "FreeSerif" },
      { Sid::ottavaFontSize,          "ottavaFontSize",          12.0 },
      { Sid::ottavaFontStyle,         "ottavaFontStyle",         int(FontStyle::Normal) },
      { Sid::ottavaTextAlign,         "ottavaTextAlign",         QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::ottavaFrameType,         "ottavaFrameType",         int(FrameType::NO_FRAME) },
      { Sid::ottavaFramePadding,      "ottavaFramePadding",      0.2 },
      { Sid::ottavaFrameWidth,        "ottavaFrameWidth",        0.1 },
      { Sid::ottavaFrameRound,        "ottavaFrameRound",        0 },
      { Sid::ottavaFrameFgColor,      "ottavaFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::ottavaFrameBgColor,      "ottavaFrameBgColor",      QColor(255, 255, 255, 0) },

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
      { Sid::tupletFontStyle,         "tupletFontStyle",         int(FontStyle::Italic) },
      { Sid::tupletAlign,             "tupletAlign",             QVariant::fromValue(Align::CENTER) },
      { Sid::tupletBracketHookHeight, "tupletBracketHookHeight", Spatium(1.0) },
      { Sid::tupletOffset,            "tupletOffset",            QPointF()  },
      { Sid::tupletFrameType,         "tupletFrameType",         int(FrameType::NO_FRAME) },
      { Sid::tupletFramePadding,      "tupletFramePadding",      0.2 },
      { Sid::tupletFrameWidth,        "tupletFrameWidth",        0.1 },
      { Sid::tupletFrameRound,        "tupletFrameRound",        0 },
      { Sid::tupletFrameFgColor,      "tupletFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::tupletFrameBgColor,      "tupletFrameBgColor",      QColor(255, 255, 255, 0) },

      { Sid::barreLineWidth,          "barreLineWidth",          QVariant(1.0) },
      { Sid::scaleBarlines,           "scaleBarlines",           QVariant(true) },
      { Sid::barGraceDistance,        "barGraceDistance",        Spatium(.6) },
      { Sid::minVerticalDistance,     "minVerticalDistance",     Spatium(0.5) },
      { Sid::ornamentStyle,           "ornamentStyle",           int(MScore::OrnamentStyle::DEFAULT) },
      { Sid::spatium,                 "Spatium",                 SPATIUM20 },

      { Sid::autoplaceHairpinDynamicsDistance, "autoplaceHairpinDynamicsDistance", Spatium(0.5) },

      { Sid::dynamicsPlacement,       "dynamicsPlacement",       int(Placement::BELOW)  },
      { Sid::dynamicsPosAbove,        "dynamicsPosAbove",        QPointF(.0, -2.0) },
      { Sid::dynamicsPosBelow,        "dynamicsPosBelow",        QPointF(.0, 4.0) },

      { Sid::dynamicsMinDistance,         "dynamicsMinDistance",               Spatium(0.5) },
      { Sid::autoplaceVerticalAlignRange, "autoplaceVerticalAlignRange",     int(VerticalAlignRange::SYSTEM) },

      { Sid::textLinePlacement,         "textLinePlacement",         int(Placement::ABOVE)  },
      { Sid::textLinePosAbove,          "textLinePosAbove",          QPointF(.0, -3.5) },
      { Sid::textLinePosBelow,          "textLinePosBelow",          QPointF(.0, 3.5) },
      { Sid::textLineFrameType,         "textLineFrameType",          int(FrameType::NO_FRAME) },
      { Sid::textLineFramePadding,      "textLineFramePadding",       0.2 },
      { Sid::textLineFrameWidth,        "textLineFrameWidth",         0.1 },
      { Sid::textLineFrameRound,        "textLineFrameRound",         0 },
      { Sid::textLineFrameFgColor,      "textLineFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::textLineFrameBgColor,      "textLineFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::tremoloBarLineWidth,       "tremoloBarLineWidth",       Spatium(0.12) },
      { Sid::jumpPosAbove,              "jumpPosAbove",              QPointF(.0, -2.0) },
      { Sid::markerPosAbove,            "markerPosAbove",            QPointF(.0, -2.0) },

      { Sid::defaultFontFace,               "defaultFontFace",               "FreeSerif" },
      { Sid::defaultFontSize,               "defaultFontSize",               10.0  },
      { Sid::defaultFontSpatiumDependent,   "defaultFontSpatiumDependent",   true  },
      { Sid::defaultFontStyle,              "defaultFontStyle",              int(FontStyle::Normal) },
      { Sid::defaultAlign,                  "defaultAlign",                  QVariant::fromValue(Align::LEFT) },
      { Sid::defaultFrameType,              "defaultFrameType",              int(FrameType::NO_FRAME) },
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
      { Sid::titleFontStyle,                "titleFontStyle",                int(FontStyle::Normal) },
      { Sid::titleAlign,                    "titleAlign",                    QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { Sid::titleOffset,                   "titleOffset",                   QPointF() },
      { Sid::titleOffsetType,               "titleOffsetType",               int(OffsetType::ABS)   },
      { Sid::titleFrameType,                "titleFrameType",                int(FrameType::NO_FRAME) },
      { Sid::titleFramePadding,             "titleFramePadding",             0.2 },
      { Sid::titleFrameWidth,               "titleFrameWidth",               0.1 },
      { Sid::titleFrameRound,               "titleFrameRound",               0 },
      { Sid::titleFrameFgColor,             "titleFrameFgColor",             QColor(0, 0, 0, 255) },
      { Sid::titleFrameBgColor,             "titleFrameBgColor",             QColor(255, 255, 255, 0) },

      { Sid::subTitleFontFace,              "subTitleFontFace",              "FreeSerif" },
      { Sid::subTitleFontSize,              "subTitleFontSize",              14.0 },
      { Sid::subTitleFontSpatiumDependent,  "subTitleFontSpatiumDependent",  false  },
      { Sid::subTitleFontStyle,             "subTitleFontStyle",             int(FontStyle::Normal) },
      { Sid::subTitleAlign,                 "subTitleAlign",                 QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { Sid::subTitleOffset,                "subTitleOffset",                QPointF(0.0, 10.0) },
      { Sid::subTitleOffsetType,            "subTitleOffsetType",            int(OffsetType::ABS)   },
      { Sid::subTitleFrameType,             "subTitleFrameType",             int(FrameType::NO_FRAME) },
      { Sid::subTitleFramePadding,          "subTitleFramePadding",          0.2 },
      { Sid::subTitleFrameWidth,            "subTitleFrameWidth",            0.1 },
      { Sid::subTitleFrameRound,            "subTitleFrameRound",            0 },
      { Sid::subTitleFrameFgColor,          "subTitleFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::subTitleFrameBgColor,          "subTitleFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::composerFontFace,              "composerFontFace",              "FreeSerif" },
      { Sid::composerFontSize,              "composerFontSize",              12.0 },
      { Sid::composerFontSpatiumDependent,  "composerFontSpatiumDependent",  false  },
      { Sid::composerFontStyle,             "composerFontStyle",             int(FontStyle::Normal) },
      { Sid::composerAlign,                 "composerAlign",                 QVariant::fromValue(Align::RIGHT | Align::BOTTOM) },
      { Sid::composerOffset,                "composerOffset",                QPointF() },
      { Sid::composerOffsetType,            "composerOffsetType",            int(OffsetType::ABS)   },
      { Sid::composerFrameType,             "composerFrameType",             int(FrameType::NO_FRAME) },
      { Sid::composerFramePadding,          "composerFramePadding",          0.2 },
      { Sid::composerFrameWidth,            "composerFrameWidth",            0.1 },
      { Sid::composerFrameRound,            "composerFrameRound",            0 },
      { Sid::composerFrameFgColor,          "composerFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::composerFrameBgColor,          "composerFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::lyricistFontFace,              "lyricistFontFace",              "FreeSerif" },
      { Sid::lyricistFontSize,              "lyricistFontSize",              12.0 },
      { Sid::lyricistFontSpatiumDependent,  "lyricistFontSpatiumDependent",  false  },
      { Sid::lyricistFontStyle,             "lyricistFontStyle",             int(FontStyle::Normal) },
      { Sid::lyricistAlign,                 "lyricistAlign",                 QVariant::fromValue(Align::LEFT | Align::BOTTOM) },
      { Sid::lyricistOffset,                "lyricistOffset",                QPointF() },
      { Sid::lyricistOffsetType,            "lyricistOffsetType",            int(OffsetType::ABS)   },
      { Sid::lyricistFrameType,             "lyricistFrameType",             int(FrameType::NO_FRAME) },
      { Sid::lyricistFramePadding,          "lyricistFramePadding",          0.2 },
      { Sid::lyricistFrameWidth,            "lyricistFrameWidth",            0.1 },
      { Sid::lyricistFrameRound,            "lyricistFrameRound",            0 },
      { Sid::lyricistFrameFgColor,          "lyricistFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::lyricistFrameBgColor,          "lyricistFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::fingeringFontFace,             "fingeringFontFace",             "FreeSerif" },
      { Sid::fingeringFontSize,             "fingeringFontSize",             8.0 },
      { Sid::fingeringFontStyle,            "fingeringFontStyle",             int(FontStyle::Normal) },
      { Sid::fingeringAlign,                "fingeringAlign",                QVariant::fromValue(Align::CENTER) },
      { Sid::fingeringFrameType,            "fingeringFrameType",            int(FrameType::NO_FRAME) },
      { Sid::fingeringFramePadding,         "fingeringFramePadding",         0.2 },
      { Sid::fingeringFrameWidth,           "fingeringFrameWidth",           0.1 },
      { Sid::fingeringFrameRound,           "fingeringFrameRound",           0 },
      { Sid::fingeringFrameFgColor,         "fingeringFrameFgColor",         QColor(0, 0, 0, 255) },
      { Sid::fingeringFrameBgColor,         "fingeringFrameBgColor",         QColor(255, 255, 255, 0) },
      { Sid::fingeringOffset,               "fingeringOffset",               QPointF() },

      { Sid::lhGuitarFingeringFontFace,     "lhGuitarFingeringFontFace",     "FreeSerif" },
      { Sid::lhGuitarFingeringFontSize,     "lhGuitarFingeringFontSize",     8.0 },
      { Sid::lhGuitarFingeringFontStyle,    "lhGuitarFingeringFontStyle",    int(FontStyle::Normal) },
      { Sid::lhGuitarFingeringAlign,        "lhGuitarFingeringAlign",        QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::lhGuitarFingeringFrameType,    "lhGuitarFingeringFrameType",    int(FrameType::NO_FRAME) },
      { Sid::lhGuitarFingeringFramePadding, "lhGuitarFingeringFramePadding", 0.2 },
      { Sid::lhGuitarFingeringFrameWidth,   "lhGuitarFingeringFrameWidth",   0.1 },
      { Sid::lhGuitarFingeringFrameRound,   "lhGuitarFingeringFrameRound",   0 },
      { Sid::lhGuitarFingeringFrameFgColor, "lhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::lhGuitarFingeringFrameBgColor, "lhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { Sid::lhGuitarFingeringOffset,       "lhGuitarFingeringOffset",       QPointF(-0.5, 0.0) },

      { Sid::rhGuitarFingeringFontFace,     "rhGuitarFingeringFontFace",     "FreeSerif" },
      { Sid::rhGuitarFingeringFontSize,     "rhGuitarFingeringFontSize",     8.0 },
      { Sid::rhGuitarFingeringFontStyle,    "rhGuitarFingeringFontStyle",    int(FontStyle::Normal) },
      { Sid::rhGuitarFingeringAlign,        "rhGuitarFingeringAlign",        QVariant::fromValue(Align::CENTER) },
      { Sid::rhGuitarFingeringFrameType,    "rhGuitarFingeringFrameType",    int(FrameType::NO_FRAME) },
      { Sid::rhGuitarFingeringFramePadding, "rhGuitarFingeringFramePadding", 0.2 },
      { Sid::rhGuitarFingeringFrameWidth,   "rhGuitarFingeringFrameWidth",   0.1 },
      { Sid::rhGuitarFingeringFrameRound,   "rhGuitarFingeringFrameRound",   0 },
      { Sid::rhGuitarFingeringFrameFgColor, "rhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::rhGuitarFingeringFrameBgColor, "rhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { Sid::rhGuitarFingeringOffset,       "rhGuitarFingeringOffset",       QPointF() },

      { Sid::stringNumberFontFace,          "stringNumberFontFace",          "FreeSerif" },
      { Sid::stringNumberFontSize,          "stringNumberFontSize",          8.0 },
      { Sid::stringNumberFontStyle,         "stringNumberFontStyle",         int(FontStyle::Normal) },
      { Sid::stringNumberAlign,             "stringNumberAlign",             QVariant::fromValue(Align::CENTER) },
      { Sid::stringNumberFrameType,         "stringNumberFrameType",         int(FrameType::CIRCLE) },
      { Sid::stringNumberFramePadding,      "stringNumberFramePadding",      0.2 },
      { Sid::stringNumberFrameWidth,        "stringNumberFrameWidth",        0.1 },
      { Sid::stringNumberFrameRound,        "stringNumberFrameRound",        0 },
      { Sid::stringNumberFrameFgColor,      "stringNumberFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::stringNumberFrameBgColor,      "stringNumberFrameBgColor",      QColor(255, 255, 255, 0) },
      { Sid::stringNumberOffset,            "stringNumberOffset",            QPointF(0.0, -2.0) },

      { Sid::longInstrumentFontFace,        "longInstrumentFontFace",       "FreeSerif" },
      { Sid::longInstrumentFontSize,        "longInstrumentFontSize",       12.0 },
      { Sid::longInstrumentFontStyle,       "longInstrumentFontStyle",      int(FontStyle::Normal) },
      { Sid::longInstrumentAlign,           "longInstrumentAlign",          QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::longInstrumentOffset,          "longInstrumentOffset",         QPointF(.0, .0) },
      { Sid::longInstrumentFrameType,       "longInstrumentFrameType",      int(FrameType::NO_FRAME) },
      { Sid::longInstrumentFramePadding,    "longInstrumentFramePadding",   0.2 },
      { Sid::longInstrumentFrameWidth,      "longInstrumentFrameWidth",     0.1 },
      { Sid::longInstrumentFrameRound,      "longInstrumentFrameRound",     0 },
      { Sid::longInstrumentFrameFgColor,    "longInstrumentFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::longInstrumentFrameBgColor,    "longInstrumentFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::shortInstrumentFontFace,       "shortInstrumentFontFace",      "FreeSerif" },
      { Sid::shortInstrumentFontSize,       "shortInstrumentFontSize",      12.0 },
      { Sid::shortInstrumentFontStyle,      "shortInstrumentFontStyle",     int(FontStyle::Normal) },
      { Sid::shortInstrumentAlign,          "shortInstrumentAlign",         QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::shortInstrumentOffset,         "shortInstrumentOffset",        QPointF(.0, .0) },
      { Sid::shortInstrumentFrameType,      "shortInstrumentFrameType",     int(FrameType::NO_FRAME) },
      { Sid::shortInstrumentFramePadding,   "shortInstrumentFramePadding",  0.2 },
      { Sid::shortInstrumentFrameWidth,     "shortInstrumentFrameWidth",    0.1 },
      { Sid::shortInstrumentFrameRound,     "shortInstrumentFrameRound",    0 },
      { Sid::shortInstrumentFrameFgColor,   "shortInstrumentFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::shortInstrumentFrameBgColor,   "shortInstrumentFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::partInstrumentFontFace,        "partInstrumentFontFace",       "FreeSerif" },
      { Sid::partInstrumentFontSize,        "partInstrumentFontSize",       18.0 },
      { Sid::partInstrumentFontStyle,       "partInstrumentFontStyle",      int(FontStyle::Normal) },
      { Sid::partInstrumentAlign,           "partInstrumentAlign",          QVariant::fromValue(Align::LEFT) },
      { Sid::partInstrumentOffset,          "partInstrumentOffset",         QPointF() },
      { Sid::partInstrumentFrameType,       "partInstrumentFrameType",      int(FrameType::NO_FRAME) },
      { Sid::partInstrumentFramePadding,    "partInstrumentFramePadding",   0.2 },
      { Sid::partInstrumentFrameWidth,      "partInstrumentFrameWidth",     0.1 },
      { Sid::partInstrumentFrameRound,      "partInstrumentFrameRound",     0 },
      { Sid::partInstrumentFrameFgColor,    "partInstrumentFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::partInstrumentFrameBgColor,    "partInstrumentFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::dynamicsFontFace,              "dynamicsFontFace",             "FreeSerif" },
      { Sid::dynamicsFontSize,              "dynamicsFontSize",             12.0 },
      { Sid::dynamicsFontStyle,             "dynamicsFontStyle",            int(FontStyle::Italic) },
      { Sid::dynamicsAlign,                 "dynamicsAlign",                QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::dynamicsFrameType,             "dynamicsFrameType",            int(FrameType::NO_FRAME) },
      { Sid::dynamicsFramePadding,          "dynamicsFramePadding",         0.2 },
      { Sid::dynamicsFrameWidth,            "dynamicsFrameWidth",           0.1 },
      { Sid::dynamicsFrameRound,            "dynamicsFrameRound",           0 },
      { Sid::dynamicsFrameFgColor,          "dynamicsFrameFgColor",         QColor(0, 0, 0, 255) },
      { Sid::dynamicsFrameBgColor,          "dynamicsFrameBgColor",         QColor(255, 255, 255, 0) },

      { Sid::expressionFontFace,            "expressionFontFace",           "FreeSerif" },
      { Sid::expressionFontSize,            "expressionFontSize",           11.0 },
      { Sid::expressionFontStyle,           "expressionFontStyle",          int(FontStyle::Italic) },
      { Sid::expressionAlign,               "expressionAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::expressionPlacement,           "expressionPlacement",          int(Placement::ABOVE)  },
      { Sid::expressionOffset,              "expressionOffset",             QPointF() },
      { Sid::expressionFrameType,           "expressionFrameType",          int(FrameType::NO_FRAME) },
      { Sid::expressionFramePadding,        "expressionFramePadding",       0.2 },
      { Sid::expressionFrameWidth,          "expressionFrameWidth",         0.1 },
      { Sid::expressionFrameRound,          "expressionFrameRound",         0 },
      { Sid::expressionFrameFgColor,        "expressionFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::expressionFrameBgColor,        "expressionFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::tempoFontFace,                 "tempoFontFace",                "FreeSerif" },
      { Sid::tempoFontSize,                 "tempoFontSize",                12.0 },
      { Sid::tempoFontStyle,                "tempoFontStyle",               int(FontStyle::Bold) },
      { Sid::tempoAlign,                    "tempoAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::tempoSystemFlag,               "tempoSystemFlag",              true },
      { Sid::tempoPlacement,                "tempoPlacement",               int(Placement::ABOVE)  },
      { Sid::tempoPosAbove,                 "tempoPosAbove",                QPointF(.0, -2.0) },
      { Sid::tempoPosBelow,                 "tempoPosBelow",                QPointF(.0, 3.0)  },
      { Sid::tempoMinDistance,              "tempoMinDistance",             Spatium(.5)  },
      { Sid::tempoFrameType,                "tempoFrameType",               int(FrameType::NO_FRAME) },
      { Sid::tempoFramePadding,             "tempoFramePadding",            0.2 },
      { Sid::tempoFrameWidth,               "tempoFrameWidth",              0.1 },
      { Sid::tempoFrameRound,               "tempoFrameRound",              0 },
      { Sid::tempoFrameFgColor,             "tempoFrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::tempoFrameBgColor,             "tempoFrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::metronomeFontFace,             "metronomeFontFace",            "FreeSerif" },
      { Sid::metronomeFontSize,             "metronomeFontSize",            12.0 },
      { Sid::metronomeFontStyle,            "metronomeFontStyle",           int(FontStyle::Bold) },
      { Sid::metronomePlacement,            "metronomePlacement",           int(Placement::ABOVE) },
      { Sid::metronomeAlign,                "metronomeAlign",               QVariant::fromValue(Align::LEFT) },
      { Sid::metronomeOffset,               "metronomeOffset",              QPointF() },
      { Sid::metronomeFrameType,            "metronomeFrameType",           int(FrameType::NO_FRAME) },
      { Sid::metronomeFramePadding,         "metronomeFramePadding",        0.2 },
      { Sid::metronomeFrameWidth,           "metronomeFrameWidth",          0.1 },
      { Sid::metronomeFrameRound,           "metronomeFrameRound",          0 },
      { Sid::metronomeFrameFgColor,         "metronomeFrameFgColor",        QColor(0, 0, 0, 255) },
      { Sid::metronomeFrameBgColor,         "metronomeFrameBgColor",        QColor(255, 255, 255, 0) },

      { Sid::measureNumberFontFace,         "measureNumberFontFace",        "FreeSerif" },
      { Sid::measureNumberFontSize,         "measureNumberFontSize",        8.0 },
      { Sid::measureNumberFontStyle,        "measureNumberFontStyle",       int(FontStyle::Normal) },
      { Sid::measureNumberOffset,           "measureNumberOffset",          QPointF(0.0, -2.0) },
      { Sid::measureNumberOffsetType,       "measureNumberOffsetType",      int(OffsetType::SPATIUM)   },
      { Sid::measureNumberAlign,            "measureNumberAlign",           QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::measureNumberFrameType,        "measureNumberFrameType",       int(FrameType::NO_FRAME) },
      { Sid::measureNumberFramePadding,     "measureNumberFramePadding",    0.2 },
      { Sid::measureNumberFrameWidth,       "measureNumberFrameWidth",      0.1 },
      { Sid::measureNumberFrameRound,       "measureNumberFrameRound",      0 },
      { Sid::measureNumberFrameFgColor,     "measureNumberFrameFgColor",    QColor(0, 0, 0, 255) },
      { Sid::measureNumberFrameBgColor,     "measureNumberFrameBgColor",    QColor(255, 255, 255, 0) },

      { Sid::translatorFontFace,            "translatorFontFace",           "FreeSerif" },
      { Sid::translatorFontSize,            "translatorFontSize",           11.0 },
      { Sid::translatorFontStyle,           "translatorFontStyle",          int(FontStyle::Normal) },
      { Sid::translatorAlign,               "translatorAlign",              QVariant::fromValue(Align::LEFT) },
      { Sid::translatorOffset,              "translatorOffset",             QPointF() },
      { Sid::translatorFrameType,           "translatorFrameType",          int(FrameType::NO_FRAME) },
      { Sid::translatorFramePadding,        "translatorFramePadding",       0.2 },
      { Sid::translatorFrameWidth,          "translatorFrameWidth",         0.1 },
      { Sid::translatorFrameRound,          "translatorFrameRound",         0 },
      { Sid::translatorFrameFgColor,        "translatorFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::translatorFrameBgColor,        "translatorFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::systemTextFontFace,            "systemFontFace",               "FreeSerif" },
      { Sid::systemTextFontSize,            "systemFontSize",               10.0 },
      { Sid::systemTextFontStyle,           "systemFontStyle",              int(FontStyle::Normal) },
      { Sid::systemTextAlign,               "systemAlign",                  QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::systemTextOffsetType,          "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { Sid::systemTextPlacement,           "systemPlacement",              int(Placement::ABOVE) },
      { Sid::systemTextPosAbove,            "systemPosAbove",               QPointF(.0, -2.0) },
      { Sid::systemTextPosBelow,            "systemPosBelow",               QPointF(.0, 3.5)  },
      { Sid::systemTextMinDistance,         "systemMinDistance",            Spatium(0.5)  },
      { Sid::systemTextFrameType,           "systemFrameType",              int(FrameType::NO_FRAME) },
      { Sid::systemTextFramePadding,        "systemFramePadding",           0.2 },
      { Sid::systemTextFrameWidth,          "systemFrameWidth",             0.1 },
      { Sid::systemTextFrameRound,          "systemFrameRound",             0  },
      { Sid::systemTextFrameFgColor,        "systemFrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::systemTextFrameBgColor,        "systemFrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::staffTextFontFace,             "staffFontFace",                "FreeSerif" },
      { Sid::staffTextFontSize,             "staffFontSize",                10.0 },
      { Sid::staffTextFontStyle,            "staffFontStyle",               int(FontStyle::Normal) },
      { Sid::staffTextAlign,                "staffAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::staffTextOffsetType,           "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { Sid::staffTextPlacement,            "staffPlacement",               int(Placement::ABOVE) },
      { Sid::staffTextPosAbove,             "staffPosAbove",                QPointF(.0, -2.0) },
      { Sid::staffTextPosBelow,             "staffPosBelow",                QPointF(.0, 3.5)  },
      { Sid::staffTextMinDistance,          "staffMinDistance",             Spatium(0.5)  },
      { Sid::staffTextFrameType,            "staffFrameType",               int(FrameType::NO_FRAME) },
      { Sid::staffTextFramePadding,         "staffFramePadding",            0.2 },
      { Sid::staffTextFrameWidth,           "staffFrameWidth",              0.1 },
      { Sid::staffTextFrameRound,           "staffFrameRound",              0  },
      { Sid::staffTextFrameFgColor,         "staffFrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::staffTextFrameBgColor,         "staffFrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::rehearsalMarkFontFace,         "rehearsalMarkFontFace",        "FreeSerif" },
      { Sid::rehearsalMarkFontSize,         "rehearsalMarkFontSize",        14.0 },
      { Sid::rehearsalMarkFontStyle,        "rehearsalMarkFontStyle",       int(FontStyle::Bold) },
      { Sid::rehearsalMarkAlign,            "rehearsalMarkAlign",           QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::rehearsalMarkFrameType,        "rehearsalMarkFrameType",       int(FrameType::SQUARE)  },
      { Sid::rehearsalMarkFramePadding,     "rehearsalMarkFramePadding",    0.5 },
      { Sid::rehearsalMarkFrameWidth,       "rehearsalMarkFrameWidth",      0.2 },
      { Sid::rehearsalMarkFrameRound,       "rehearsalMarkFrameRound",      20 },
      { Sid::rehearsalMarkFrameFgColor,     "rehearsalMarkFrameFgColor",    QColor(0, 0, 0, 255) },
      { Sid::rehearsalMarkFrameBgColor,     "rehearsalMarkFrameBgColor",    QColor(255, 255, 255, 0) },
      { Sid::rehearsalMarkPlacement,        "rehearsalMarkPlacement",       int(Placement::ABOVE) },
      { Sid::rehearsalMarkPosAbove,         "rehearsalMarkPosAbove",        QPointF(.0, -3.0) },
      { Sid::rehearsalMarkPosBelow,         "rehearsalMarkPosBelow",        QPointF(.0, 4.0) },
      { Sid::rehearsalMarkMinDistance,      "rehearsalMarkMinDistance",     Spatium(0.5) },

      { Sid::repeatLeftFontFace,            "repeatLeftFontFace",           "FreeSerif" },
      { Sid::repeatLeftFontSize,            "repeatLeftFontSize",           20.0 },
      { Sid::repeatLeftFontStyle,           "repeatLeftFontStyle",          int(FontStyle::Normal) },
      { Sid::repeatLeftAlign,               "repeatLeftAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::repeatLeftPlacement,           "repeatLeftPlacement",          int(Placement::ABOVE) },
      { Sid::repeatLeftFrameType,           "repeatLeftFrameType",          int(FrameType::NO_FRAME) },
      { Sid::repeatLeftFramePadding,        "repeatLeftFramePadding",       0.2 },
      { Sid::repeatLeftFrameWidth,          "repeatLeftFrameWidth",         0.1 },
      { Sid::repeatLeftFrameRound,          "repeatLeftFrameRound",         0  },
      { Sid::repeatLeftFrameFgColor,        "repeatLeftFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::repeatLeftFrameBgColor,        "repeatLeftFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::repeatRightFontFace,           "repeatRightFontFace",          "FreeSerif" },
      { Sid::repeatRightFontSize,           "repeatRightFontSize",          12.0 },
      { Sid::repeatRightFontStyle,          "repeatRightFontStyle",         int(FontStyle::Normal) },
      { Sid::repeatRightAlign,              "repeatRightAlign",             QVariant::fromValue(Align::RIGHT | Align::BASELINE) },
      { Sid::repeatRightPlacement,          "repeatRightPlacement",         int(Placement::ABOVE) },
      { Sid::repeatRightFrameType,          "repeatRightFrameType",         int(FrameType::NO_FRAME) },
      { Sid::repeatRightFramePadding,       "repeatRightFramePadding",      0.2 },
      { Sid::repeatRightFrameWidth,         "repeatRightFrameWidth",        0.1 },
      { Sid::repeatRightFrameRound,         "repeatRightFrameRound",        0  },
      { Sid::repeatRightFrameFgColor,       "repeatRightFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::repeatRightFrameBgColor,       "repeatRightFrameBgColor",      QColor(255, 255, 255, 0) },

      { Sid::frameFontFace,                 "frameFontFace",                "FreeSerif" },
      { Sid::frameFontSize,                 "frameFontSize",                12.0 },
      { Sid::frameFontStyle,                "frameFontStyle",               int(FontStyle::Normal) },
      { Sid::frameAlign,                    "frameAlign",                   QVariant::fromValue(Align::LEFT) },
      { Sid::frameOffset,                   "frameOffset",                  QPointF() },
      { Sid::frameFrameType,                "frameFrameType",               int(FrameType::NO_FRAME) },
      { Sid::frameFramePadding,             "frameFramePadding",            0.2 },
      { Sid::frameFrameWidth,               "frameFrameWidth",              0.1 },
      { Sid::frameFrameRound,               "frameFrameRound",              0  },
      { Sid::frameFrameFgColor,             "frameFrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::frameFrameBgColor,             "frameFrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::textLineFontFace,              "textLineFontFace",             "FreeSerif" },
      { Sid::textLineFontSize,              "textLineFontSize",             12.0 },
      { Sid::textLineFontStyle,             "textLineFontStyle",            int(FontStyle::Normal) },
      { Sid::textLineTextAlign,             "textLineTextAlign",            QVariant::fromValue(Align::LEFT | Align::VCENTER) },

      { Sid::glissandoFontFace,             "glissandoFontFace",            "FreeSerif" },
      { Sid::glissandoFontSize,             "glissandoFontSize",            QVariant(8.0) },
      { Sid::glissandoFontStyle,            "glissandoFontStyle",           int(FontStyle::Italic) },
      { Sid::glissandoAlign,                "glissandoAlign",               QVariant::fromValue(Align::LEFT) },
      { Sid::glissandoOffset,               "glissandoOffset",              QPointF() },
      { Sid::glissandoFrameType,            "glissandoFrameType",           int(FrameType::NO_FRAME) },
      { Sid::glissandoFramePadding,         "glissandoFramePadding",        0.2 },
      { Sid::glissandoFrameWidth,           "glissandoFrameWidth",          0.1 },
      { Sid::glissandoFrameRound,           "glissandoFrameRound",          0 },
      { Sid::glissandoFrameFgColor,         "glissandoFrameFgColor",        QColor(0, 0, 0, 255) },
      { Sid::glissandoFrameBgColor,         "glissandoFrameBgColor",        QColor(255, 255, 255, 0) },
      { Sid::glissandoLineWidth,            "glissandoLineWidth",           Spatium(0.15) },
      { Sid::glissandoText,                 "glissandoText",                QString("gliss.") },

      { Sid::bendFontFace,                  "bendFontFace",                 "FreeSerif" },
      { Sid::bendFontSize,                  "bendFontSize",                 8.0 },
      { Sid::bendFontStyle,                 "bendFontStyle",                int(FontStyle::Normal) },
      { Sid::bendAlign,                     "bendAlign",                    QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::bendOffset,                    "bendOffset",                   QPointF() },
      { Sid::bendFrameType,                 "bendFrameType",                int(FrameType::NO_FRAME) },
      { Sid::bendFramePadding,              "bendFramePadding",             0.2 },
      { Sid::bendFrameWidth,                "bendFrameWidth",               0.1 },
      { Sid::bendFrameRound,                "bendFrameRound",               0 },
      { Sid::bendFrameFgColor,              "bendFrameFgColor",             QColor(0, 0, 0, 255) },
      { Sid::bendFrameBgColor,              "bendFrameBgColor",             QColor(255, 255, 255, 0) },
      { Sid::bendLineWidth,                 "bendLineWidth",                Spatium(0.15) },
      { Sid::bendArrowWidth,                "bendArrowWidth",               Spatium(.5) },

      { Sid::headerFontFace,                "headerFontFace",               "FreeSerif" },
      { Sid::headerFontSize,                "headerFontSize",               8.0 },
      { Sid::headerFontStyle,               "headerFontStyle",              int(FontStyle::Normal) },
      { Sid::headerAlign,                   "headerAlign",                  QVariant::fromValue(Align::LEFT) },
      { Sid::headerOffset,                  "headerOffset",                 QPointF() },
      { Sid::headerFrameType,               "headerFrameType",              int(FrameType::NO_FRAME) },
      { Sid::headerFramePadding,            "headerFramePadding",           0.2 },
      { Sid::headerFrameWidth,              "headerFrameWidth",             0.1 },
      { Sid::headerFrameRound,              "headerFrameRound",             0  },
      { Sid::headerFrameFgColor,            "headerFrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::headerFrameBgColor,            "headerFrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::footerFontFace,                "footerFontFace",               "FreeSerif" },
      { Sid::footerFontSize,                "footerFontSize",               8.0 },
      { Sid::footerFontStyle,               "footerFontStyle",              int(FontStyle::Normal) },
      { Sid::footerAlign,                   "footerAlign",                  QVariant::fromValue(Align::LEFT) },
      { Sid::footerOffset,                  "footerOffset",                 QPointF() },
      { Sid::footerFrameType,               "footerFrameType",              int(FrameType::NO_FRAME) },
      { Sid::footerFramePadding,            "footerFramePadding",           0.2 },
      { Sid::footerFrameWidth,              "footerFrameWidth",             0.1 },
      { Sid::footerFrameRound,              "footerFrameRound",             0  },
      { Sid::footerFrameFgColor,            "footerFrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::footerFrameBgColor,            "footerFrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::instrumentChangeFontFace,      "instrumentChangeFontFace",     "FreeSerif" },
      { Sid::instrumentChangeFontSize,      "instrumentChangeFontSize",     12.0 },
      { Sid::instrumentChangeFontStyle,     "instrumentChangeFontStyle",    int(FontStyle::Bold) },
      { Sid::instrumentChangeAlign,         "instrumentChangeAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::instrumentChangeOffset,        "instrumentChangeOffset",       QPointF() },
      { Sid::instrumentChangePlacement,     "instrumentChangePlacement",    int(Placement::ABOVE)  },
      { Sid::instrumentChangePosAbove,      "instrumentChangePosAbove",     QPointF(.0, -2.0) },
      { Sid::instrumentChangePosBelow,      "instrumentChangePosBelow",     QPointF(.0, 3.5)  },
      { Sid::instrumentChangeMinDistance,   "instrumentChangeMinDistance",  Spatium(0.5)  },
      { Sid::instrumentChangeFrameType,     "instrumentChangeFrameType",    int(FrameType::NO_FRAME) },
      { Sid::instrumentChangeFramePadding,  "instrumentChangeFramePadding", 0.2 },
      { Sid::instrumentChangeFrameWidth,    "instrumentChangeFrameWidth",   0.1 },
      { Sid::instrumentChangeFrameRound,    "instrumentChangeFrameRound",   0 },
      { Sid::instrumentChangeFrameFgColor,  "instrumentChangeFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::instrumentChangeFrameBgColor,  "instrumentChangeFrameBgColor", QColor(255, 255, 255, 0) },

      { Sid::figuredBassFontFace,           "figuredBassFontFace",          "MScoreBC" },
      { Sid::figuredBassFontSize,           "figuredBassFontSize",          8.0 },
      { Sid::figuredBassFontStyle,          "figuredBassFontStyle",         int(FontStyle::Normal) },

      { Sid::user1FontFace,                 "user1FontFace",                "FreeSerif" },
      { Sid::user1FontSize,                 "user1FontSize",                10.0 },
      { Sid::user1FontStyle,                "user1FontStyle",               int(FontStyle::Normal) },
      { Sid::user1Align,                    "user1Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user1Offset,                   "user1Offset",                  0.0 },
      { Sid::user1OffsetType,               "user1OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user1FrameType,                "user1FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user1FramePadding,             "user1FramePadding",            0.2 },
      { Sid::user1FrameWidth,               "user1FrameWidth",              0.1 },
      { Sid::user1FrameRound,               "user1FrameRound",              0 },
      { Sid::user1FrameFgColor,             "user1FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user1FrameBgColor,             "user1FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user2FontFace,                 "user2FontFace",                "FreeSerif" },
      { Sid::user2FontSize,                 "user2FontSize",                10.0 },
      { Sid::user2FontStyle,                "user2FontStyle",               int(FontStyle::Normal) },
      { Sid::user2Align,                    "user2Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user2Offset,                   "user2Offset",                  0.0 },
      { Sid::user2OffsetType,               "user2OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user2FrameType,                "user2FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user2FramePadding,             "user2FramePadding",            0.2 },
      { Sid::user2FrameWidth,               "user2FrameWidth",              0.1 },
      { Sid::user2FrameRound,               "user2FrameRound",              0 },
      { Sid::user2FrameFgColor,             "user2FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user2FrameBgColor,             "user2FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user3FontFace,                 "user3FontFace",                "FreeSerif" },
      { Sid::user3FontSize,                 "user3FontSize",                10.0 },
      { Sid::user3FontStyle,                "user3FontStyle",               int(FontStyle::Normal) },
      { Sid::user3Align,                    "user3Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user3Offset,                   "user3Offset",                  0.0 },
      { Sid::user3OffsetType,               "user3OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user3FrameType,                "user3FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user3FramePadding,             "user3FramePadding",            0.2 },
      { Sid::user3FrameWidth,               "user3FrameWidth",              0.1 },
      { Sid::user3FrameRound,               "user3FrameRound",              0 },
      { Sid::user3FrameFgColor,             "user3FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user3FrameBgColor,             "user3FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user4FontFace,                 "user4FontFace",                "FreeSerif" },
      { Sid::user4FontSize,                 "user4FontSize",                10.0 },
      { Sid::user4FontStyle,                "user4FontStyle",               int(FontStyle::Normal) },
      { Sid::user4Align,                    "user4Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user4Offset,                   "user4Offset",                  0.0 },
      { Sid::user4OffsetType,               "user4OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user4FrameType,                "user4FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user4FramePadding,             "user4FramePadding",            0.2 },
      { Sid::user4FrameWidth,               "user4FrameWidth",              0.1 },
      { Sid::user4FrameRound,               "user4FrameRound",              0 },
      { Sid::user4FrameFgColor,             "user4FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user4FrameBgColor,             "user4FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user5FontFace,                 "user5FontFace",                "FreeSerif" },
      { Sid::user5FontSize,                 "user5FontSize",                10.0 },
      { Sid::user5FontStyle,                "user5FontStyle",               int(FontStyle::Normal) },
      { Sid::user5Align,                    "user5Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user5Offset,                   "user5Offset",                  0.0 },
      { Sid::user5OffsetType,               "user5OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user5FrameType,                "user5FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user5FramePadding,             "user5FramePadding",            0.2 },
      { Sid::user5FrameWidth,               "user5FrameWidth",              0.1 },
      { Sid::user5FrameRound,               "user5FrameRound",              0 },
      { Sid::user5FrameFgColor,             "user5FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user5FrameBgColor,             "user5FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user6FontFace,                 "user6FontFace",                "FreeSerif" },
      { Sid::user6FontSize,                 "user6FontSize",                10.0 },
      { Sid::user6FontStyle,                "user6FontStyle",               int(FontStyle::Normal) },
      { Sid::user6Align,                    "user6Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user6Offset,                   "user6Offset",                  0.0 },
      { Sid::user6OffsetType,               "user6OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user6FrameType,                "user6FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user6FramePadding,             "user6FramePadding",            0.2 },
      { Sid::user6FrameWidth,               "user6FrameWidth",              0.1 },
      { Sid::user6FrameRound,               "user6FrameRound",              0 },
      { Sid::user6FrameFgColor,             "user6FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user6FrameBgColor,             "user6FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::letRingFontFace,               "letRingFontFace",              "FreeSerif" },
      { Sid::letRingFontSize,               "letRingFontSize",              10.0 },
      { Sid::letRingFontStyle,              "letRingFontStyle",             false },
      { Sid::letRingTextAlign,              "letRingTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::letRingHookHeight,             "letRingHookHeight",            Spatium(0.6) },
      { Sid::letRingPlacement,              "letRingPlacement",             int(Placement::BELOW)  },
      { Sid::letRingPosAbove,               "letRingPosAbove",              QPointF(.0, -4.0) },
      { Sid::letRingPosBelow,               "letRingPosBelow",              QPointF(.0, 4.0)  },
      { Sid::letRingLineWidth,              "letRingLineWidth",             Spatium(0.15) },
      { Sid::letRingLineStyle,              "letRingLineStyle",             QVariant(int(Qt::DashLine)) },
      { Sid::letRingBeginTextOffset,        "letRingBeginTextOffset",       QPointF(0.0, 0.15) },
      { Sid::letRingText,                   "letRingText",                  "let ring" },
      { Sid::letRingFrameType,              "letRingFrameType",          int(FrameType::NO_FRAME) },
      { Sid::letRingFramePadding,           "letRingFramePadding",       0.2 },
      { Sid::letRingFrameWidth,             "letRingFrameWidth",         0.1 },
      { Sid::letRingFrameRound,             "letRingFrameRound",         0 },
      { Sid::letRingFrameFgColor,           "letRingFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::letRingFrameBgColor,           "letRingFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::palmMuteFontFace,              "palmMuteFontFace",              "FreeSerif" },
      { Sid::palmMuteFontSize,              "palmMuteFontSize",              10.0 },
      { Sid::palmMuteFontStyle,             "palmMuteFontStyle",             false },
      { Sid::palmMuteTextAlign,             "palmMuteTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::palmMuteHookHeight,            "palmMuteHookHeight",            Spatium(0.6) },
      { Sid::palmMutePlacement,             "palmMutePlacement",             int(Placement::BELOW)  },
      { Sid::palmMutePosAbove,              "palmMutePosAbove",              QPointF(.0, -4.0) },
      { Sid::palmMutePosBelow,              "palmMutePosBelow",              QPointF(.0, 4.0)  },
      { Sid::palmMuteLineWidth,             "palmMuteLineWidth",             Spatium(0.15) },
      { Sid::palmMuteLineStyle,             "palmMuteLineStyle",             QVariant(int(Qt::DashLine)) },
      { Sid::palmMuteBeginTextOffset,       "palmMuteBeginTextOffset",       QPointF(0.0, 0.15) },
      { Sid::palmMuteText,                  "palmMuteText",                  "P.M." },
      { Sid::palmMuteFrameType,             "palmMuteFrameType",             int(FrameType::NO_FRAME) },
      { Sid::palmMuteFramePadding,          "palmMuteFramePadding",          0.2 },
      { Sid::palmMuteFrameWidth,            "palmMuteFrameWidth",            0.1 },
      { Sid::palmMuteFrameRound,            "palmMuteFrameRound",            0 },
      { Sid::palmMuteFrameFgColor,          "palmMuteFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::palmMuteFrameBgColor,          "palmMuteFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::fermataPosAbove,               "fermataPosAbove",               QPointF(.0, -1.0) },
      { Sid::fermataPosBelow,               "fermataPosBelow",               QPointF(.0, 1.0)  },
      { Sid::fermataMinDistance,            "fermataMinDistance",            Spatium(0.4)  },
      };

MStyle  MScore::_baseStyle;
MStyle  MScore::_defaultStyle;

//---------------------------------------------------------
//   text styles
//---------------------------------------------------------

const TextStyle defaultTextStyle {{
      { Sid::defaultFontFace,                    Pid::FONT_FACE              },
      { Sid::defaultFontSize,                    Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::defaultFontStyle,                   Pid::FONT_STYLE             },
      { Sid::defaultAlign,                       Pid::ALIGN                  },
      { Sid::defaultOffset,                      Pid::OFFSET                 },
      { Sid::defaultFrameType,                   Pid::FRAME_TYPE             },
      { Sid::defaultFramePadding,                Pid::FRAME_PADDING          },
      { Sid::defaultFrameWidth,                  Pid::FRAME_WIDTH            },
      { Sid::defaultFrameRound,                  Pid::FRAME_ROUND            },
      { Sid::defaultFrameFgColor,                Pid::FRAME_FG_COLOR         },
      { Sid::defaultFrameBgColor,                Pid::FRAME_BG_COLOR         },
      }};

const TextStyle titleTextStyle {{
      { Sid::titleFontFace,                      Pid::FONT_FACE              },
      { Sid::titleFontSize,                      Pid::FONT_SIZE              },
      { Sid::titleFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::titleFontStyle,                     Pid::FONT_STYLE             },
      { Sid::titleAlign,                         Pid::ALIGN                  },
      { Sid::titleOffset,                        Pid::OFFSET                 },
      { Sid::titleFrameType,                     Pid::FRAME_TYPE             },
      { Sid::titleFramePadding,                  Pid::FRAME_PADDING          },
      { Sid::titleFrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::titleFrameRound,                    Pid::FRAME_ROUND            },
      { Sid::titleFrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::titleFrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle subTitleTextStyle {{
      { Sid::subTitleFontFace,                   Pid::FONT_FACE              },
      { Sid::subTitleFontSize,                   Pid::FONT_SIZE              },
      { Sid::subTitleFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::subTitleFontStyle,                  Pid::FONT_STYLE             },
      { Sid::subTitleAlign,                      Pid::ALIGN                  },
      { Sid::subTitleOffset,                     Pid::OFFSET                 },
      { Sid::subTitleFrameType,                  Pid::FRAME_TYPE             },
      { Sid::subTitleFramePadding,               Pid::FRAME_PADDING          },
      { Sid::subTitleFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::subTitleFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::subTitleFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::subTitleFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle composerTextStyle {{
      { Sid::composerFontFace,                   Pid::FONT_FACE              },
      { Sid::composerFontSize,                   Pid::FONT_SIZE              },
      { Sid::composerFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::composerFontStyle,                  Pid::FONT_STYLE             },
      { Sid::composerAlign,                      Pid::ALIGN                  },
      { Sid::composerOffset,                     Pid::OFFSET                 },
      { Sid::composerFrameType,                  Pid::FRAME_TYPE             },
      { Sid::composerFramePadding,               Pid::FRAME_PADDING          },
      { Sid::composerFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::composerFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::composerFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::composerFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle lyricistTextStyle {{
      { Sid::lyricistFontFace,                   Pid::FONT_FACE              },
      { Sid::lyricistFontSize,                   Pid::FONT_SIZE              },
      { Sid::lyricistFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lyricistFontStyle,                  Pid::FONT_STYLE             },
      { Sid::lyricistAlign,                      Pid::ALIGN                  },
      { Sid::lyricistOffset,                     Pid::OFFSET                 },
      { Sid::lyricistFrameType,                  Pid::FRAME_TYPE             },
      { Sid::lyricistFramePadding,               Pid::FRAME_PADDING          },
      { Sid::lyricistFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::lyricistFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::lyricistFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::lyricistFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle lyricsEvenTextStyle {{
      { Sid::lyricsEvenFontFace,                 Pid::FONT_FACE              },
      { Sid::lyricsEvenFontSize,                 Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lyricsEvenFontStyle,                Pid::FONT_STYLE             },
      { Sid::lyricsEvenAlign,                    Pid::ALIGN                  },
      { Sid::lyricsPosBelow,                     Pid::OFFSET                 },
      { Sid::lyricsEvenFrameType,                Pid::FRAME_TYPE             },
      { Sid::lyricsEvenFramePadding,             Pid::FRAME_PADDING          },
      { Sid::lyricsEvenFrameWidth,               Pid::FRAME_WIDTH            },
      { Sid::lyricsEvenFrameRound,               Pid::FRAME_ROUND            },
      { Sid::lyricsEvenFrameFgColor,             Pid::FRAME_FG_COLOR         },
      { Sid::lyricsEvenFrameBgColor,             Pid::FRAME_BG_COLOR         },
      }};

const TextStyle lyricsOddTextStyle {{
      { Sid::lyricsOddFontFace,                  Pid::FONT_FACE              },
      { Sid::lyricsOddFontSize,                  Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lyricsOddFontStyle,                 Pid::FONT_STYLE             },
      { Sid::lyricsOddAlign,                     Pid::ALIGN                  },
      { Sid::lyricsPosBelow,                     Pid::OFFSET                 },
      { Sid::lyricsOddFrameType,                 Pid::FRAME_TYPE             },
      { Sid::lyricsOddFramePadding,              Pid::FRAME_PADDING          },
      { Sid::lyricsOddFrameWidth,                Pid::FRAME_WIDTH            },
      { Sid::lyricsOddFrameRound,                Pid::FRAME_ROUND            },
      { Sid::lyricsOddFrameFgColor,              Pid::FRAME_FG_COLOR         },
      { Sid::lyricsOddFrameBgColor,              Pid::FRAME_BG_COLOR         },
      }};

const TextStyle fingeringTextStyle {{
      { Sid::fingeringFontFace,                  Pid::FONT_FACE              },
      { Sid::fingeringFontSize,                  Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::fingeringFontStyle,                 Pid::FONT_STYLE             },
      { Sid::fingeringAlign,                     Pid::ALIGN                  },
      { Sid::fingeringOffset,                    Pid::OFFSET                 },
      { Sid::fingeringFrameType,                 Pid::FRAME_TYPE             },
      { Sid::fingeringFramePadding,              Pid::FRAME_PADDING          },
      { Sid::fingeringFrameWidth,                Pid::FRAME_WIDTH            },
      { Sid::fingeringFrameRound,                Pid::FRAME_ROUND            },
      { Sid::fingeringFrameFgColor,              Pid::FRAME_FG_COLOR         },
      { Sid::fingeringFrameBgColor,              Pid::FRAME_BG_COLOR         },
      }};

const TextStyle lhGuitarFingeringTextStyle {{
      { Sid::lhGuitarFingeringFontFace,          Pid::FONT_FACE              },
      { Sid::lhGuitarFingeringFontSize,          Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lhGuitarFingeringFontStyle,         Pid::FONT_STYLE             },
      { Sid::lhGuitarFingeringAlign,             Pid::ALIGN                  },
      { Sid::lhGuitarFingeringOffset,            Pid::OFFSET                 },
      { Sid::lhGuitarFingeringFrameType,         Pid::FRAME_TYPE             },
      { Sid::lhGuitarFingeringFramePadding,      Pid::FRAME_PADDING          },
      { Sid::lhGuitarFingeringFrameWidth,        Pid::FRAME_WIDTH            },
      { Sid::lhGuitarFingeringFrameRound,        Pid::FRAME_ROUND            },
      { Sid::lhGuitarFingeringFrameFgColor,      Pid::FRAME_FG_COLOR         },
      { Sid::lhGuitarFingeringFrameBgColor,      Pid::FRAME_BG_COLOR         },
      }};

const TextStyle rhGuitarFingeringTextStyle {{
      { Sid::rhGuitarFingeringFontFace,          Pid::FONT_FACE              },
      { Sid::rhGuitarFingeringFontSize,          Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::rhGuitarFingeringFontStyle,         Pid::FONT_STYLE             },
      { Sid::rhGuitarFingeringAlign,             Pid::ALIGN                  },
      { Sid::rhGuitarFingeringOffset,            Pid::OFFSET                 },
      { Sid::rhGuitarFingeringFrameType,         Pid::FRAME_TYPE             },
      { Sid::rhGuitarFingeringFramePadding,      Pid::FRAME_PADDING          },
      { Sid::rhGuitarFingeringFrameWidth,        Pid::FRAME_WIDTH            },
      { Sid::rhGuitarFingeringFrameRound,        Pid::FRAME_ROUND            },
      { Sid::rhGuitarFingeringFrameFgColor,      Pid::FRAME_FG_COLOR         },
      { Sid::rhGuitarFingeringFrameBgColor,      Pid::FRAME_BG_COLOR         },
      }};

const TextStyle stringNumberTextStyle {{
      { Sid::stringNumberFontFace,               Pid::FONT_FACE              },
      { Sid::stringNumberFontSize,               Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::stringNumberFontStyle,              Pid::FONT_STYLE             },
      { Sid::stringNumberAlign,                  Pid::ALIGN                  },
      { Sid::stringNumberOffset,                 Pid::OFFSET                 },
      { Sid::stringNumberFrameType,              Pid::FRAME_TYPE             },
      { Sid::stringNumberFramePadding,           Pid::FRAME_PADDING          },
      { Sid::stringNumberFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::stringNumberFrameRound,             Pid::FRAME_ROUND            },
      { Sid::stringNumberFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::stringNumberFrameBgColor,           Pid::FRAME_BG_COLOR         },
      }};

const TextStyle longInstrumentTextStyle {{
      { Sid::longInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::longInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::longInstrumentFontStyle,            Pid::FONT_STYLE             },
      { Sid::longInstrumentAlign,                Pid::ALIGN                  },
      { Sid::longInstrumentOffset,               Pid::OFFSET                 },
      { Sid::longInstrumentFrameType,            Pid::FRAME_TYPE             },
      { Sid::longInstrumentFramePadding,         Pid::FRAME_PADDING          },
      { Sid::longInstrumentFrameWidth,           Pid::FRAME_WIDTH            },
      { Sid::longInstrumentFrameRound,           Pid::FRAME_ROUND            },
      { Sid::longInstrumentFrameFgColor,         Pid::FRAME_FG_COLOR         },
      { Sid::longInstrumentFrameBgColor,         Pid::FRAME_BG_COLOR         },
      }};

const TextStyle shortInstrumentTextStyle {{
      { Sid::shortInstrumentFontFace,            Pid::FONT_FACE              },
      { Sid::shortInstrumentFontSize,            Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::shortInstrumentFontStyle,           Pid::FONT_STYLE             },
      { Sid::shortInstrumentAlign,               Pid::ALIGN                  },
      { Sid::shortInstrumentOffset,              Pid::OFFSET                 },
      { Sid::shortInstrumentFrameType,           Pid::FRAME_TYPE             },
      { Sid::shortInstrumentFramePadding,        Pid::FRAME_PADDING          },
      { Sid::shortInstrumentFrameWidth,          Pid::FRAME_WIDTH            },
      { Sid::shortInstrumentFrameRound,          Pid::FRAME_ROUND            },
      { Sid::shortInstrumentFrameFgColor,        Pid::FRAME_FG_COLOR         },
      { Sid::shortInstrumentFrameBgColor,        Pid::FRAME_BG_COLOR         },
      }};

const TextStyle partInstrumentTextStyle {{
      { Sid::partInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::partInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::partInstrumentFontStyle,            Pid::FONT_STYLE             },
      { Sid::partInstrumentAlign,                Pid::ALIGN                  },
      { Sid::partInstrumentOffset,               Pid::OFFSET                 },
      { Sid::partInstrumentFrameType,            Pid::FRAME_TYPE             },
      { Sid::partInstrumentFramePadding,         Pid::FRAME_PADDING          },
      { Sid::partInstrumentFrameWidth,           Pid::FRAME_WIDTH            },
      { Sid::partInstrumentFrameRound,           Pid::FRAME_ROUND            },
      { Sid::partInstrumentFrameFgColor,         Pid::FRAME_FG_COLOR         },
      { Sid::partInstrumentFrameBgColor,         Pid::FRAME_BG_COLOR         },
      }};

const TextStyle dynamicsTextStyle {{
      { Sid::dynamicsFontFace,                   Pid::FONT_FACE              },
      { Sid::dynamicsFontSize,                   Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::dynamicsFontStyle,                  Pid::FONT_STYLE             },
      { Sid::dynamicsAlign,                      Pid::ALIGN                  },
      { Sid::dynamicsPosBelow,                   Pid::OFFSET                 },
      { Sid::dynamicsFrameType,                  Pid::FRAME_TYPE             },
      { Sid::dynamicsFramePadding,               Pid::FRAME_PADDING          },
      { Sid::dynamicsFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::dynamicsFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::dynamicsFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::dynamicsFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle expressionTextStyle {{
      { Sid::expressionFontFace,                 Pid::FONT_FACE              },
      { Sid::expressionFontSize,                 Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::expressionFontStyle,                Pid::FONT_STYLE             },
      { Sid::expressionAlign,                    Pid::ALIGN                  },
      { Sid::expressionOffset,                   Pid::OFFSET                 },
      { Sid::expressionFrameType,                Pid::FRAME_TYPE             },
      { Sid::expressionFramePadding,             Pid::FRAME_PADDING          },
      { Sid::expressionFrameWidth,               Pid::FRAME_WIDTH            },
      { Sid::expressionFrameRound,               Pid::FRAME_ROUND            },
      { Sid::expressionFrameFgColor,             Pid::FRAME_FG_COLOR         },
      { Sid::expressionFrameBgColor,             Pid::FRAME_BG_COLOR         },
      }};

const TextStyle tempoTextStyle {{
      { Sid::tempoFontFace,                      Pid::FONT_FACE              },
      { Sid::tempoFontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::tempoFontStyle,                     Pid::FONT_STYLE             },
      { Sid::tempoAlign,                         Pid::ALIGN                  },
      { Sid::tempoPosAbove,                      Pid::OFFSET                 },
      { Sid::tempoFrameType,                     Pid::FRAME_TYPE             },
      { Sid::tempoFramePadding,                  Pid::FRAME_PADDING          },
      { Sid::tempoFrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::tempoFrameRound,                    Pid::FRAME_ROUND            },
      { Sid::tempoFrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::tempoFrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle metronomeTextStyle {{
      { Sid::metronomeFontFace,                  Pid::FONT_FACE              },
      { Sid::metronomeFontSize,                  Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::metronomeFontStyle,                 Pid::FONT_STYLE             },
      { Sid::metronomeAlign,                     Pid::ALIGN                  },
      { Sid::metronomeOffset,                    Pid::OFFSET                 },
      { Sid::metronomeFrameType,                 Pid::FRAME_TYPE             },
      { Sid::metronomeFramePadding,              Pid::FRAME_PADDING          },
      { Sid::metronomeFrameWidth,                Pid::FRAME_WIDTH            },
      { Sid::metronomeFrameRound,                Pid::FRAME_ROUND            },
      { Sid::metronomeFrameFgColor,              Pid::FRAME_FG_COLOR         },
      { Sid::metronomeFrameBgColor,              Pid::FRAME_BG_COLOR         },
      }};

const TextStyle measureNumberTextStyle {{
      { Sid::measureNumberFontFace,              Pid::FONT_FACE              },
      { Sid::measureNumberFontSize,              Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::measureNumberFontStyle,             Pid::FONT_STYLE             },
      { Sid::measureNumberAlign,                 Pid::ALIGN                  },
      { Sid::measureNumberOffset,                Pid::OFFSET                 },
      { Sid::measureNumberFrameType,             Pid::FRAME_TYPE             },
      { Sid::measureNumberFramePadding,          Pid::FRAME_PADDING          },
      { Sid::measureNumberFrameWidth,            Pid::FRAME_WIDTH            },
      { Sid::measureNumberFrameRound,            Pid::FRAME_ROUND            },
      { Sid::measureNumberFrameFgColor,          Pid::FRAME_FG_COLOR         },
      { Sid::measureNumberFrameBgColor,          Pid::FRAME_BG_COLOR         },
      }};

const TextStyle translatorTextStyle {{
      { Sid::translatorFontFace,                 Pid::FONT_FACE              },
      { Sid::translatorFontSize,                 Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::translatorFontStyle,                Pid::FONT_STYLE             },
      { Sid::translatorAlign,                    Pid::ALIGN                  },
      { Sid::translatorOffset,                   Pid::OFFSET                 },
      { Sid::translatorFrameType,                Pid::FRAME_TYPE             },
      { Sid::translatorFramePadding,             Pid::FRAME_PADDING          },
      { Sid::translatorFrameWidth,               Pid::FRAME_WIDTH            },
      { Sid::translatorFrameRound,               Pid::FRAME_ROUND            },
      { Sid::translatorFrameFgColor,             Pid::FRAME_FG_COLOR         },
      { Sid::translatorFrameBgColor,             Pid::FRAME_BG_COLOR         },
      }};

const TextStyle tupletTextStyle {{
      { Sid::tupletFontFace,                     Pid::FONT_FACE               },
      { Sid::tupletFontSize,                     Pid::FONT_SIZE               },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::tupletFontStyle,                    Pid::FONT_STYLE              },
      { Sid::tupletAlign,                        Pid::ALIGN                   },
      { Sid::tupletOffset,                       Pid::OFFSET                 },
      { Sid::tupletFrameType,                    Pid::FRAME_TYPE             },
      { Sid::tupletFramePadding,                 Pid::FRAME_PADDING          },
      { Sid::tupletFrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::tupletFrameRound,                   Pid::FRAME_ROUND            },
      { Sid::tupletFrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::tupletFrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle systemTextStyle {{
      { Sid::systemTextFontFace,                 Pid::FONT_FACE              },
      { Sid::systemTextFontSize,                 Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::systemTextFontStyle,                Pid::FONT_STYLE             },
      { Sid::systemTextAlign,                    Pid::ALIGN                  },
      { Sid::systemTextPosAbove,                 Pid::OFFSET                 },
      { Sid::systemTextFrameType,                Pid::FRAME_TYPE             },
      { Sid::systemTextFramePadding,             Pid::FRAME_PADDING          },
      { Sid::systemTextFrameWidth,               Pid::FRAME_WIDTH            },
      { Sid::systemTextFrameRound,               Pid::FRAME_ROUND            },
      { Sid::systemTextFrameFgColor,             Pid::FRAME_FG_COLOR         },
      { Sid::systemTextFrameBgColor,             Pid::FRAME_BG_COLOR         },
      }};

const TextStyle staffTextStyle {{
      { Sid::staffTextFontFace,                  Pid::FONT_FACE              },
      { Sid::staffTextFontSize,                  Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::staffTextFontStyle,                 Pid::FONT_STYLE             },
      { Sid::staffTextAlign,                     Pid::ALIGN                  },
      { Sid::staffTextPosAbove,                  Pid::OFFSET                 },
      { Sid::staffTextFrameType,                 Pid::FRAME_TYPE             },
      { Sid::staffTextFramePadding,              Pid::FRAME_PADDING          },
      { Sid::staffTextFrameWidth,                Pid::FRAME_WIDTH            },
      { Sid::staffTextFrameRound,                Pid::FRAME_ROUND            },
      { Sid::staffTextFrameFgColor,              Pid::FRAME_FG_COLOR         },
      { Sid::staffTextFrameBgColor,              Pid::FRAME_BG_COLOR         },
      }};

const TextStyle chordSymbolTextStyleA {{
      { Sid::chordSymbolAFontFace,               Pid::FONT_FACE              },
      { Sid::chordSymbolAFontSize,               Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::chordSymbolAFontStyle,              Pid::FONT_STYLE             },
      { Sid::chordSymbolAAlign,                  Pid::ALIGN                  },
      { Sid::chordSymbolAPosAbove,               Pid::OFFSET                 },
      { Sid::chordSymbolAFrameType,              Pid::FRAME_TYPE             },
      { Sid::chordSymbolAFramePadding,           Pid::FRAME_PADDING          },
      { Sid::chordSymbolAFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::chordSymbolAFrameRound,             Pid::FRAME_ROUND            },
      { Sid::chordSymbolAFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::chordSymbolAFrameBgColor,           Pid::FRAME_BG_COLOR         },
      }};

const TextStyle chordSymbolTextStyleB {{
      { Sid::chordSymbolBFontFace,               Pid::FONT_FACE              },
      { Sid::chordSymbolBFontSize,               Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::chordSymbolBFontStyle,              Pid::FONT_STYLE             },
      { Sid::chordSymbolBAlign,                  Pid::ALIGN                  },
      { Sid::chordSymbolBPosAbove,               Pid::OFFSET                 },
      { Sid::chordSymbolBFrameType,              Pid::FRAME_TYPE             },
      { Sid::chordSymbolBFramePadding,           Pid::FRAME_PADDING          },
      { Sid::chordSymbolBFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::chordSymbolBFrameRound,             Pid::FRAME_ROUND            },
      { Sid::chordSymbolBFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::chordSymbolBFrameBgColor,           Pid::FRAME_BG_COLOR         },
      }};

const TextStyle rehearsalMarkTextStyle {{
      { Sid::rehearsalMarkFontFace,              Pid::FONT_FACE              },
      { Sid::rehearsalMarkFontSize,              Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::rehearsalMarkFontStyle,             Pid::FONT_STYLE             },
      { Sid::rehearsalMarkAlign,                 Pid::ALIGN                  },
      { Sid::rehearsalMarkPosAbove,              Pid::OFFSET                 },
      { Sid::rehearsalMarkFrameType,             Pid::FRAME_TYPE             },
      { Sid::rehearsalMarkFramePadding,          Pid::FRAME_PADDING          },
      { Sid::rehearsalMarkFrameWidth,            Pid::FRAME_WIDTH            },
      { Sid::rehearsalMarkFrameRound,            Pid::FRAME_ROUND            },
      { Sid::rehearsalMarkFrameFgColor,          Pid::FRAME_FG_COLOR         },
      { Sid::rehearsalMarkFrameBgColor,          Pid::FRAME_BG_COLOR         },
      }};

const TextStyle repeatLeftTextStyle {{
      { Sid::repeatLeftFontFace,                 Pid::FONT_FACE              },
      { Sid::repeatLeftFontSize,                 Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::repeatLeftFontStyle,                Pid::FONT_STYLE             },
      { Sid::repeatLeftAlign,                    Pid::ALIGN                  },
      { Sid::markerPosAbove,                     Pid::OFFSET                 },
      { Sid::repeatLeftFrameType,                Pid::FRAME_TYPE             },
      { Sid::repeatLeftFramePadding,             Pid::FRAME_PADDING          },
      { Sid::repeatLeftFrameWidth,               Pid::FRAME_WIDTH            },
      { Sid::repeatLeftFrameRound,               Pid::FRAME_ROUND            },
      { Sid::repeatLeftFrameFgColor,             Pid::FRAME_FG_COLOR         },
      { Sid::repeatLeftFrameBgColor,             Pid::FRAME_BG_COLOR         },
      }};

const TextStyle repeatRightTextStyle {{
      { Sid::repeatRightFontFace,                Pid::FONT_FACE              },
      { Sid::repeatRightFontSize,                Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::repeatRightFontStyle,               Pid::FONT_STYLE             },
      { Sid::repeatRightAlign,                   Pid::ALIGN                  },
      { Sid::jumpPosAbove,                       Pid::OFFSET                 },
      { Sid::repeatRightFrameType,               Pid::FRAME_TYPE             },
      { Sid::repeatRightFramePadding,            Pid::FRAME_PADDING          },
      { Sid::repeatRightFrameWidth,              Pid::FRAME_WIDTH            },
      { Sid::repeatRightFrameRound,              Pid::FRAME_ROUND            },
      { Sid::repeatRightFrameFgColor,            Pid::FRAME_FG_COLOR         },
      { Sid::repeatRightFrameBgColor,            Pid::FRAME_BG_COLOR         },
      }};

const TextStyle frameTextStyle {{
      { Sid::frameFontFace,                      Pid::FONT_FACE              },
      { Sid::frameFontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::frameFontStyle,                     Pid::FONT_STYLE             },
      { Sid::frameAlign,                         Pid::ALIGN                  },
      { Sid::frameOffset,                        Pid::OFFSET                 },
      { Sid::frameFrameType,                     Pid::FRAME_TYPE             },
      { Sid::frameFramePadding,                  Pid::FRAME_PADDING          },
      { Sid::frameFrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::frameFrameRound,                    Pid::FRAME_ROUND            },
      { Sid::frameFrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::frameFrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle textLineTextStyle {{
      { Sid::textLineFontFace,                   Pid::BEGIN_FONT_FACE        },
      { Sid::textLineFontSize,                   Pid::BEGIN_FONT_SIZE        },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::textLineFontStyle,                  Pid::BEGIN_FONT_STYLE       },
      { Sid::textLineTextAlign,                  Pid::ALIGN                  },
      { Sid::textLinePosAbove,                   Pid::OFFSET                 },
      { Sid::textLineFrameType,                  Pid::FRAME_TYPE             },
      { Sid::textLineFramePadding,               Pid::FRAME_PADDING          },
      { Sid::textLineFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::textLineFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::textLineFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::textLineFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle glissandoTextStyle {{
      { Sid::glissandoFontFace,                  Pid::FONT_FACE              },
      { Sid::glissandoFontSize,                  Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::glissandoFontStyle,                 Pid::FONT_STYLE             },
      { Sid::glissandoAlign,                     Pid::ALIGN                  },
      { Sid::glissandoOffset,                    Pid::OFFSET                 },
      { Sid::glissandoFrameType,                 Pid::FRAME_TYPE             },
      { Sid::glissandoFramePadding,              Pid::FRAME_PADDING          },
      { Sid::glissandoFrameWidth,                Pid::FRAME_WIDTH            },
      { Sid::glissandoFrameRound,                Pid::FRAME_ROUND            },
      { Sid::glissandoFrameFgColor,              Pid::FRAME_FG_COLOR         },
      { Sid::glissandoFrameBgColor,              Pid::FRAME_BG_COLOR         },
      }};

const TextStyle ottavaTextStyle {{
      { Sid::ottavaFontFace,                     Pid::BEGIN_FONT_FACE        },
      { Sid::ottavaFontSize,                     Pid::BEGIN_FONT_SIZE        },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::ottavaFontStyle,                    Pid::BEGIN_FONT_STYLE       },
      { Sid::ottavaTextAlign,                    Pid::BEGIN_TEXT_ALIGN       },
      { Sid::ottavaPosAbove,                     Pid::OFFSET                 },
      { Sid::ottavaFrameType,                    Pid::FRAME_TYPE             },
      { Sid::ottavaFramePadding,                 Pid::FRAME_PADDING          },
      { Sid::ottavaFrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::ottavaFrameRound,                   Pid::FRAME_ROUND            },
      { Sid::ottavaFrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::ottavaFrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle voltaTextStyle {{
      { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE         },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::voltaFontStyle,                     Pid::BEGIN_FONT_STYLE        },
      { Sid::voltaAlign,                         Pid::BEGIN_TEXT_ALIGN        },
      { Sid::voltaOffset,                        Pid::BEGIN_TEXT_OFFSET       },
      { Sid::voltaFrameType,                     Pid::FRAME_TYPE             },
      { Sid::voltaFramePadding,                  Pid::FRAME_PADDING          },
      { Sid::voltaFrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::voltaFrameRound,                    Pid::FRAME_ROUND            },
      { Sid::voltaFrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::voltaFrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle pedalTextStyle {{
      { Sid::pedalFontFace,                      Pid::BEGIN_FONT_FACE         },
      { Sid::pedalFontSize,                      Pid::BEGIN_FONT_SIZE         },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::pedalFontStyle,                     Pid::BEGIN_FONT_STYLE        },
      { Sid::pedalTextAlign,                     Pid::BEGIN_TEXT_ALIGN        },
      { Sid::pedalPosAbove,                      Pid::BEGIN_TEXT_OFFSET       },
      { Sid::pedalFrameType,                     Pid::FRAME_TYPE             },
      { Sid::pedalFramePadding,                  Pid::FRAME_PADDING          },
      { Sid::pedalFrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::pedalFrameRound,                    Pid::FRAME_ROUND            },
      { Sid::pedalFrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::pedalFrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle letRingTextStyle {{
      { Sid::letRingFontFace,                    Pid::BEGIN_FONT_FACE        },
      { Sid::letRingFontSize,                    Pid::BEGIN_FONT_SIZE        },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::letRingFontStyle,                   Pid::BEGIN_FONT_STYLE       },
      { Sid::letRingTextAlign,                   Pid::BEGIN_TEXT_ALIGN       },
      { Sid::letRingPosAbove,                    Pid::BEGIN_TEXT_OFFSET       },
      { Sid::letRingFrameType,                   Pid::FRAME_TYPE             },
      { Sid::letRingFramePadding,                Pid::FRAME_PADDING          },
      { Sid::letRingFrameWidth,                  Pid::FRAME_WIDTH            },
      { Sid::letRingFrameRound,                  Pid::FRAME_ROUND            },
      { Sid::letRingFrameFgColor,                Pid::FRAME_FG_COLOR         },
      { Sid::letRingFrameBgColor,                Pid::FRAME_BG_COLOR         },
      }};

const TextStyle palmMuteTextStyle {{
      { Sid::palmMuteFontFace,                   Pid::BEGIN_FONT_FACE        },
      { Sid::palmMuteFontSize,                   Pid::BEGIN_FONT_SIZE        },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::palmMuteFontStyle,                  Pid::BEGIN_FONT_STYLE       },
      { Sid::palmMuteTextAlign,                  Pid::BEGIN_TEXT_ALIGN       },
      { Sid::palmMutePosAbove,                   Pid::BEGIN_TEXT_OFFSET       },
      { Sid::palmMuteFrameType,                  Pid::FRAME_TYPE             },
      { Sid::palmMuteFramePadding,               Pid::FRAME_PADDING          },
      { Sid::palmMuteFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::palmMuteFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::palmMuteFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::palmMuteFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle hairpinTextStyle {{
      { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE            },
      { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE            },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::hairpinFontStyle,                   Pid::BEGIN_FONT_STYLE           },
      { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN           },
      { Sid::hairpinPosAbove,                    Pid::BEGIN_TEXT_OFFSET       },
      { Sid::hairpinFrameType,                   Pid::FRAME_TYPE             },
      { Sid::hairpinFramePadding,                Pid::FRAME_PADDING          },
      { Sid::hairpinFrameWidth,                  Pid::FRAME_WIDTH            },
      { Sid::hairpinFrameRound,                  Pid::FRAME_ROUND            },
      { Sid::hairpinFrameFgColor,                Pid::FRAME_FG_COLOR         },
      { Sid::hairpinFrameBgColor,                Pid::FRAME_BG_COLOR         },
      }};

const TextStyle bendTextStyle {{
      { Sid::bendFontFace,                       Pid::FONT_FACE              },
      { Sid::bendFontSize,                       Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::bendFontStyle,                      Pid::FONT_STYLE             },
      { Sid::bendAlign,                          Pid::BEGIN_TEXT_ALIGN       },
      { Sid::bendOffset,                         Pid::BEGIN_TEXT_OFFSET      },
      { Sid::bendFrameType,                      Pid::FRAME_TYPE             },
      { Sid::bendFramePadding,                   Pid::FRAME_PADDING          },
      { Sid::bendFrameWidth,                     Pid::FRAME_WIDTH            },
      { Sid::bendFrameRound,                     Pid::FRAME_ROUND            },
      { Sid::bendFrameFgColor,                   Pid::FRAME_FG_COLOR         },
      { Sid::bendFrameBgColor,                   Pid::FRAME_BG_COLOR         },
      }};

const TextStyle headerTextStyle {{
      { Sid::headerFontFace,                     Pid::FONT_FACE              },
      { Sid::headerFontSize,                     Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::headerFontStyle,                    Pid::FONT_STYLE             },
      { Sid::headerAlign,                        Pid::BEGIN_TEXT_ALIGN       },
      { Sid::headerOffset,                       Pid::BEGIN_TEXT_OFFSET      },
      { Sid::headerFrameType,                    Pid::FRAME_TYPE             },
      { Sid::headerFramePadding,                 Pid::FRAME_PADDING          },
      { Sid::headerFrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::headerFrameRound,                   Pid::FRAME_ROUND            },
      { Sid::headerFrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::headerFrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle footerTextStyle {{
      { Sid::footerFontFace,                     Pid::FONT_FACE              },
      { Sid::footerFontSize,                     Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::footerFontStyle,                    Pid::FONT_STYLE             },
      { Sid::footerAlign,                        Pid::BEGIN_TEXT_ALIGN       },
      { Sid::footerOffset,                       Pid::BEGIN_TEXT_OFFSET      },
      { Sid::footerFrameType,                    Pid::FRAME_TYPE             },
      { Sid::footerFramePadding,                 Pid::FRAME_PADDING          },
      { Sid::footerFrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::footerFrameRound,                   Pid::FRAME_ROUND            },
      { Sid::footerFrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::footerFrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle instrumentChangeTextStyle {{
      { Sid::instrumentChangeFontFace,           Pid::FONT_FACE              },
      { Sid::instrumentChangeFontSize,           Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::instrumentChangeFontStyle,          Pid::FONT_STYLE             },
      { Sid::instrumentChangeAlign,              Pid::ALIGN                  },
      { Sid::instrumentChangeOffset,             Pid::OFFSET                 },
      { Sid::instrumentChangeFrameType,          Pid::FRAME_TYPE             },
      { Sid::instrumentChangeFramePadding,       Pid::FRAME_PADDING          },
      { Sid::instrumentChangeFrameWidth,         Pid::FRAME_WIDTH            },
      { Sid::instrumentChangeFrameRound,         Pid::FRAME_ROUND            },
      { Sid::instrumentChangeFrameFgColor,       Pid::FRAME_FG_COLOR         },
      { Sid::instrumentChangeFrameBgColor,       Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user1TextStyle {{
      { Sid::user1FontFace,                      Pid::FONT_FACE              },
      { Sid::user1FontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user1FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user1Align,                         Pid::ALIGN                  },
      { Sid::user1Offset,                        Pid::OFFSET                 },
      { Sid::user1FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user1FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user1FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user1FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user1FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user1FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user2TextStyle {{
      { Sid::user2FontFace,                      Pid::FONT_FACE              },
      { Sid::user2FontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user2FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user2Align,                         Pid::ALIGN                  },
      { Sid::user2Offset,                        Pid::OFFSET                 },
      { Sid::user2FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user2FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user2FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user2FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user2FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user2FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user3TextStyle {{
      { Sid::user3FontFace,                      Pid::FONT_FACE              },
      { Sid::user3FontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user3FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user3Align,                         Pid::ALIGN                  },
      { Sid::user3Offset,                        Pid::OFFSET                 },
      { Sid::user3FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user3FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user3FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user3FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user3FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user3FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user4TextStyle {{
      { Sid::user4FontFace,                      Pid::FONT_FACE              },
      { Sid::user4FontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user4FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user4Align,                         Pid::ALIGN                  },
      { Sid::user4Offset,                        Pid::OFFSET                 },
      { Sid::user4FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user4FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user4FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user4FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user4FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user4FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user5TextStyle {{
      { Sid::user5FontFace,                      Pid::FONT_FACE              },
      { Sid::user5FontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user5FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user5Align,                         Pid::ALIGN                  },
      { Sid::user5Offset,                        Pid::OFFSET                 },
      { Sid::user5FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user5FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user5FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user5FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user5FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user5FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user6TextStyle {{
      { Sid::user6FontFace,                      Pid::FONT_FACE              },
      { Sid::user6FontSize,                      Pid::FONT_SIZE              },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user6FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user6Align,                         Pid::ALIGN                  },
      { Sid::user6Offset,                        Pid::OFFSET                 },
      { Sid::user6FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user6FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user6FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user6FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user6FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user6FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

//---------------------------------------------------------
//   TextStyleName
//---------------------------------------------------------

struct TextStyleName {
      const char* name;
      const TextStyle* ts;
      Tid tid;
      };

static constexpr std::array<TextStyleName, int(Tid::TEXT_STYLES)> textStyles { {
      { QT_TRANSLATE_NOOP("TextStyle", "Default"),                 &defaultTextStyle,           Tid::DEFAULT },
      { QT_TRANSLATE_NOOP("TextStyle", "Title"),                   &titleTextStyle,             Tid::TITLE },
      { QT_TRANSLATE_NOOP("TextStyle", "Subtitle"),                &subTitleTextStyle,          Tid::SUBTITLE },
      { QT_TRANSLATE_NOOP("TextStyle", "Composer"),                &composerTextStyle,          Tid::COMPOSER },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyricist"),                &lyricistTextStyle,          Tid::POET },

      { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Odd Lines"),        &lyricsOddTextStyle,         Tid::LYRICS_ODD },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Even Lines"),       &lyricsEvenTextStyle,        Tid::LYRICS_EVEN },
      { QT_TRANSLATE_NOOP("TextStyle", "Fingering"),               &fingeringTextStyle,         Tid::FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "LH Guitar Fingering"),     &lhGuitarFingeringTextStyle, Tid::LH_GUITAR_FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "RH Guitar Fingering"),     &rhGuitarFingeringTextStyle, Tid::RH_GUITAR_FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "String Number"),           &stringNumberTextStyle,      Tid::STRING_NUMBER },

      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Long)"),  &longInstrumentTextStyle,    Tid::INSTRUMENT_LONG },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Short)"), &shortInstrumentTextStyle,   Tid::INSTRUMENT_SHORT },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Part)"),  &partInstrumentTextStyle,    Tid::INSTRUMENT_EXCERPT },
      { QT_TRANSLATE_NOOP("TextStyle", "Dynamics"),                &dynamicsTextStyle,          Tid::DYNAMICS },
      { QT_TRANSLATE_NOOP("TextStyle", "Expression"),              &expressionTextStyle,        Tid::EXPRESSION },

      { QT_TRANSLATE_NOOP("TextStyle", "Tempo"),                   &tempoTextStyle,             Tid::TEMPO },
      { QT_TRANSLATE_NOOP("TextStyle", "Metronome"),               &metronomeTextStyle,         Tid::METRONOME },
      { QT_TRANSLATE_NOOP("TextStyle", "Measure Number"),          &measureNumberTextStyle,     Tid::MEASURE_NUMBER },
      { QT_TRANSLATE_NOOP("TextStyle", "Translator"),              &translatorTextStyle,        Tid::TRANSLATOR },
      { QT_TRANSLATE_NOOP("TextStyle", "Tuplet"),                  &tupletTextStyle,            Tid::TUPLET },

      { QT_TRANSLATE_NOOP("TextStyle", "System"),                  &systemTextStyle,            Tid::SYSTEM },
      { QT_TRANSLATE_NOOP("TextStyle", "Staff"),                   &staffTextStyle,             Tid::STAFF },
      { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol"),            &chordSymbolTextStyleA,      Tid::HARMONY_A },
      { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol (Alternate)"),&chordSymbolTextStyleB,      Tid::HARMONY_B },
      { QT_TRANSLATE_NOOP("TextStyle", "Rehearsal Mark"),          &rehearsalMarkTextStyle,     Tid::REHEARSAL_MARK },

      { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Left"),        &repeatLeftTextStyle,        Tid::REPEAT_LEFT },
      { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Right"),       &repeatRightTextStyle,       Tid::REPEAT_RIGHT },
      { QT_TRANSLATE_NOOP("TextStyle", "Frame"),                   &frameTextStyle,             Tid::FRAME },
      { QT_TRANSLATE_NOOP("TextStyle", "Text Line"),               &textLineTextStyle,          Tid::TEXTLINE },
      { QT_TRANSLATE_NOOP("TextStyle", "Glissando"),               &glissandoTextStyle,         Tid::GLISSANDO },

      { QT_TRANSLATE_NOOP("TextStyle", "Ottava"),                  &ottavaTextStyle,            Tid::OTTAVA },
      { QT_TRANSLATE_NOOP("TextStyle", "Volta"),                   &voltaTextStyle,             Tid::VOLTA },
      { QT_TRANSLATE_NOOP("TextStyle", "Pedal"),                   &pedalTextStyle,             Tid::PEDAL },
      { QT_TRANSLATE_NOOP("TextStyle", "Let Ring"),                &letRingTextStyle,           Tid::LET_RING },
      { QT_TRANSLATE_NOOP("TextStyle", "Palm Mute"),               &palmMuteTextStyle,          Tid::PALM_MUTE },

      { QT_TRANSLATE_NOOP("TextStyle", "Hairpin"),                 &hairpinTextStyle,           Tid::HAIRPIN },
      { QT_TRANSLATE_NOOP("TextStyle", "Bend"),                    &bendTextStyle,              Tid::BEND },
      { QT_TRANSLATE_NOOP("TextStyle", "Header"),                  &headerTextStyle,            Tid::HEADER },
      { QT_TRANSLATE_NOOP("TextStyle", "Footer"),                  &footerTextStyle,            Tid::FOOTER },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Change"),       &instrumentChangeTextStyle,  Tid::INSTRUMENT_CHANGE },

      { QT_TRANSLATE_NOOP("TextStyle", "User-1"),                  &user1TextStyle,             Tid::USER1 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-2"),                  &user2TextStyle,             Tid::USER2 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-3"),                  &user3TextStyle,             Tid::USER3 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-4"),                  &user4TextStyle,             Tid::USER4 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-5"),                  &user5TextStyle,             Tid::USER5 },

      { QT_TRANSLATE_NOOP("TextStyle", "User-6"),                  &user6TextStyle,             Tid::USER6 },
      } };

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle* textStyle(const char* name)
      {
      for (const auto& s : textStyles) {
            if (strcmp(s.name, name) == 0)
                  return s.ts;
            }
      qDebug("textStyle <%s> not known", name);
      return textStyles[0].ts;
      }

const TextStyle* textStyle(Tid idx)
      {
      Q_ASSERT(idx == textStyles[int(idx)].tid);
      return textStyles[int(idx)].ts;
      }

//---------------------------------------------------------
//   TextStyleFromName
//---------------------------------------------------------

Tid textStyleFromName(const QString& name)
      {
      for (const auto& s : textStyles) {
            if (s.name == name)
                  return s.tid;
            }
      if (name == "Technique")                  // compatibility
            return Tid::EXPRESSION;

      qWarning("text style <%s> not known", qPrintable(name));
      return Tid::DEFAULT;
      }

//---------------------------------------------------------
//   textStyleName
//---------------------------------------------------------

const char* textStyleName(Tid idx)
      {
      Q_ASSERT(idx == textStyles[int(idx)].tid);
      return textStyles[int(idx)].name;
      }

//---------------------------------------------------------
//   textStyleUserName
//---------------------------------------------------------

QString textStyleUserName(Tid idx)
      {
      Q_ASSERT(idx == textStyles[int(idx)].tid);
      return qApp->translate("TextStyle", textStyleName(idx));
      }

static std::vector<Tid> _allTextStyles;

static const std::vector<Tid> _primaryTextStyles = {
      Tid::TITLE,
      Tid::SUBTITLE,
      Tid::COMPOSER,
      Tid::POET,
      Tid::TRANSLATOR,
      Tid::FRAME,
      Tid::HEADER,
      Tid::FOOTER,
      Tid::MEASURE_NUMBER,
      Tid::INSTRUMENT_EXCERPT,
      Tid::INSTRUMENT_CHANGE,
      Tid::STAFF,
      Tid::SYSTEM,
      Tid::EXPRESSION,
      Tid::DYNAMICS,
      Tid::HAIRPIN,
      Tid::TEMPO,
      Tid::REHEARSAL_MARK,
      Tid::REPEAT_LEFT,
      Tid::REPEAT_RIGHT,
      Tid::LYRICS_ODD,
      Tid::LYRICS_EVEN,
      Tid::HARMONY_A,
      Tid::HARMONY_B,
      Tid::USER1,
      Tid::USER2,
      Tid::USER3,
      Tid::USER4,
      Tid::USER5,
      Tid::USER6
      };

//---------------------------------------------------------
//   allTextStyles
//---------------------------------------------------------

const std::vector<Tid>& allTextStyles()
      {
      if (_allTextStyles.empty()) {
            _allTextStyles.reserve(int(Tid::TEXT_STYLES));
            for (const auto& s : textStyles) {
                  if (s.tid == Tid::DEFAULT)
                        continue;
                  _allTextStyles.push_back(s.tid);
                  }
            }
      return _allTextStyles;
      }

//---------------------------------------------------------
//   primaryTextStyles
//---------------------------------------------------------

const std::vector<Tid>& primaryTextStyles()
      {
      return _primaryTextStyles;
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
      if (i == Sid::NOSTYLE)
            return "no style";
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
      if (readStyleValCompat(e))
            return true;
      return false;
      }

//---------------------------------------------------------
//   readStyleValCompat
//    Read obsolete style values which may appear in files
//    produced by older versions of MuseScore.
//---------------------------------------------------------

bool MStyle::readStyleValCompat(XmlReader& e)
      {
      const QStringRef tag(e.name());
      if (tag == "tempoOffset") { // pre-3.0-beta
            const qreal x = e.doubleAttribute("x", 0.0);
            const qreal y = e.doubleAttribute("y", 0.0);
            const QPointF val(x, y);
            set(Sid::tempoPosAbove, val);
            set(Sid::tempoPosBelow, val);
            e.readElementText();
            return true;
            }
      if (readTextStyleValCompat(e))
            return true;
      return false;
      }

//---------------------------------------------------------
//   readTextStyleValCompat
//    Handle transition from separate bold, underline and
//    italic style properties to the single *FontStyle
//    property set.
//---------------------------------------------------------

bool MStyle::readTextStyleValCompat(XmlReader& e)
      {
      static const std::array<std::pair<const char*, FontStyle>, 3> styleNamesEndings {{
            { "FontBold",      FontStyle::Bold      },
            { "FontItalic",    FontStyle::Italic    },
            { "FontUnderline", FontStyle::Underline }
            }};

      const QStringRef tag(e.name());
      FontStyle readFontStyle = FontStyle::Normal;
      QStringRef typeName;
      for (auto& fontStyle : styleNamesEndings) {
            if (tag.endsWith(fontStyle.first)) {
                  readFontStyle = fontStyle.second;
                  typeName = tag.mid(0, tag.length() - strlen(fontStyle.first));
                  break;
                  }
            }
      if (readFontStyle == FontStyle::Normal)
            return false;

      const QString newFontStyleName = typeName.toString() + "FontStyle";
      const Sid sid = MStyle::styleIdx(newFontStyleName);
      if (sid == Sid::NOSTYLE) {
            qWarning() << "readFontStyleValCompat: couldn't read text readFontStyle value:" << tag;
            return false;
            }

      const bool readVal = bool(e.readElementText().toInt());
      const QVariant val = value(sid);
      FontStyle newFontStyle = (val == QVariant()) ? FontStyle::Normal : FontStyle(val.toInt());
      if (readVal)
            newFontStyle = newFontStyle + readFontStyle;
      else
            newFontStyle = newFontStyle - readFontStyle;

      set(sid, int(newFontStyle));
      return true;
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

            if (tag == "TextStyle")
                  readTextStyle206(this, e);        // obsolete
            else if (tag == "ottavaHook") {           // obsolete, for 3.0dev bw. compatibility, should be removed in final release
                  qreal y = qAbs(e.readDouble());
                  set(Sid::ottavaHookAbove, y);
                  set(Sid::ottavaHookBelow, -y);
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
            else if (!strcmp("Ms::Align", type)) {
                  Align a = Align(value(idx).toInt());
                  // Don't write if it's the default value
                  if (a == Align(st.defaultValue().toInt()))
                        continue;
                  QString horizontal = "left";
                  QString vertical = "top";
                  if (a & Align::HCENTER)
                        horizontal = "center";
                  else if (a & Align::RIGHT)
                        horizontal = "right";

                  if (a & Align::VCENTER)
                        vertical = "center";
                  else if (a & Align::BOTTOM)
                        vertical = "bottom";
                  else if (a & Align::BASELINE)
                        vertical = "baseline";

                  xml.tag(st.name(), horizontal+","+vertical);
                  }
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

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void MStyle::reset(Score* score)
      {
      for (const StyleType& st : styleTypes)
            score->undo(new ChangeStyleVal(score, st.styleIdx(), MScore::defaultStyle().value(st.styleIdx())));
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
      for (auto a : textStyles) {
            Q_ASSERT(int(a.tid) == idx);
            ++idx;
            }
      }
#endif

}
