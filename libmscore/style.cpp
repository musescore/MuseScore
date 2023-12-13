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
#include "textlinebase.h"
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

static const int LEGACY_MSC_VERSION_V3 = 301;
static const int LEGACY_MSC_VERSION_V2 = 206;
static const int LEGACY_MSC_VERSION_V1 = 114;

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
      { Sid::pagePrintableWidth,      "pagePrintableWidth",      180.0/INCH },
      { Sid::pageEvenLeftMargin,      "pageEvenLeftMargin",      15.0/INCH  },
      { Sid::pageOddLeftMargin,       "pageOddLeftMargin",       15.0/INCH  },
      { Sid::pageEvenTopMargin,       "pageEvenTopMargin",       15.0/INCH  },
      { Sid::pageEvenBottomMargin,    "pageEvenBottomMargin",    15.0/INCH  },
      { Sid::pageOddTopMargin,        "pageOddTopMargin",        15.0/INCH  },
      { Sid::pageOddBottomMargin,     "pageOddBottomMargin",     15.0/INCH  },
      { Sid::pageTwosided,            "pageTwosided",            true       },

      { Sid::staffUpperBorder,        "staffUpperBorder",        Spatium(7.0)  },
      { Sid::staffLowerBorder,        "staffLowerBorder",        Spatium(7.0)  },
      { Sid::staffHeaderFooterPadding, "staffHeaderFooterPadding", Spatium(1.0) },
      { Sid::staffDistance,           "staffDistance",           Spatium(6.5)  },
      { Sid::akkoladeDistance,        "akkoladeDistance",        Spatium(6.5)  },
      { Sid::minSystemDistance,       "minSystemDistance",       Spatium(8.5)  },
      { Sid::maxSystemDistance,       "maxSystemDistance",       Spatium(15.0) },

      { Sid::enableVerticalSpread,    "enableVerticalSpread",    false         },
      { Sid::spreadSystem,            "spreadSystem",            2.5           },
      { Sid::spreadSquareBracket,     "spreadSquareBracket",     1.2           },
      { Sid::spreadCurlyBracket,      "spreadCurlyBracket",      1.1           },
      { Sid::minSystemSpread,         "minSystemSpread",         Spatium(8.5)  },
      { Sid::maxSystemSpread,         "maxSystemSpread",         Spatium(32.0) },
      { Sid::minStaffSpread,          "minSpreadSpread",         Spatium(3.5)  },
      { Sid::maxStaffSpread,          "maxSpreadSpread",         Spatium(20.0) },
      { Sid::maxAkkoladeDistance,     "maxAkkoladeDistance",     Spatium(6.5)  },
      { Sid::maxPageFillSpread,       "maxPageFillSpread",       Spatium(6.0)  },

      { Sid::lyricsPlacement,         "lyricsPlacement",         int(Placement::BELOW)  },
      { Sid::lyricsPosAbove,          "lyricsPosAbove",          QPointF(0.0, -2.0) },
      { Sid::lyricsPosBelow,          "lyricsPosBelow",          QPointF(.0, 3.0) },
      { Sid::lyricsMinTopDistance,    "lyricsMinTopDistance",    Spatium(1.0)  },
      { Sid::lyricsMinBottomDistance, "lyricsMinBottomDistance", Spatium(2.0)  },
      { Sid::lyricsMinDistance,       "lyricsMinDistance",       Spatium(0.25)  },
      { Sid::lyricsLineHeight,        "lyricsLineHeight",        1.0 },
      { Sid::lyricsDashMinLength,     "lyricsDashMinLength",     Spatium(0.4) },
      { Sid::lyricsDashMaxLength,     "lyricsDashMaxLength",     Spatium(0.8) },
      { Sid::lyricsDashMaxDistance,   "lyricsDashMaxDistance",   Spatium(16.0) },
      { Sid::lyricsDashForce,         "lyricsDashForce",         QVariant(true) },
      { Sid::lyricsAlignVerseNumber,  "lyricsAlignVerseNumber",  true },
      { Sid::lyricsLineThickness,     "lyricsLineThickness",     Spatium(0.1) },
      { Sid::lyricsMelismaAlign,      "lyricsMelismaAlign",      QVariant::fromValue(Align::LEFT | Align::BASELINE) },

      { Sid::lyricsMelismaPad,        "lyricsMelismaPad",        Spatium(0.10) }, // the empty space before a melisma line
      { Sid::lyricsDashPad,           "lyricsDashPad",           Spatium(0.05) }, // the min. empty space before and after a dash
      { Sid::lyricsDashLineThickness, "lyricsDashLineThickness", Spatium(0.15) }, // in sp. units
      { Sid::lyricsDashYposRatio,     "lyricsDashYposRatio",     0.60          }, // the fraction of lyrics font x-height to raise the dashes above text base line

      { Sid::lyricsOddFontFace,       "lyricsOddFontFace",       "Edwin" },
      { Sid::lyricsOddFontSize,       "lyricsOddFontSize",       10.0 },
      { Sid::lyricsOddLineSpacing,    "lyricsOddLineSpacing",    1.0 },
      { Sid::lyricsOddFontSpatiumDependent, "lyricsOddFontSpatiumDependent", true },
      { Sid::lyricsOddFontStyle,      "lyricsOddFontStyle",      int(FontStyle::Normal) },
      { Sid::lyricsOddColor,          "lyricsOddColor",          QColor(0, 0, 0, 255) },
      { Sid::lyricsOddAlign,          "lyricsOddAlign",          QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::lyricsOddFrameType,      "lyricsOddFrameType",      int(FrameType::NO_FRAME) },
      { Sid::lyricsOddFramePadding,   "lyricsOddFramePadding",   0.2 },
      { Sid::lyricsOddFrameWidth,     "lyricsOddFrameWidth",     0.1 },
      { Sid::lyricsOddFrameRound,     "lyricsOddFrameRound",     0 },
      { Sid::lyricsOddFrameFgColor,   "lyricsOddFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::lyricsOddFrameBgColor,   "lyricsOddFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::lyricsEvenFontFace,      "lyricsEvenFontFace",      "Edwin" },
      { Sid::lyricsEvenFontSize,      "lyricsEvenFontSize",      10.0 },
      { Sid::lyricsEvenLineSpacing,   "lyricsEvenLineSpacing",   1.0 },
      { Sid::lyricsEvenFontSpatiumDependent, "lyricsEvenFontSpatiumDependent", true },
      { Sid::lyricsEvenFontStyle,     "lyricsEvenFontStyle",     int(FontStyle::Normal) },
      { Sid::lyricsEvenColor,         "lyricsEvenColor",         QColor(0, 0, 0, 255) },
      { Sid::lyricsEvenAlign,         "lyricsEvenAlign",         QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::lyricsEvenFrameType,     "lyricsEvenFrameType",     int(FrameType::NO_FRAME) },
      { Sid::lyricsEvenFramePadding,  "lyricsEvenFramePadding",  0.2 },
      { Sid::lyricsEvenFrameWidth,    "lyricsEvenFrameWidth",    0.1 },
      { Sid::lyricsEvenFrameRound,    "lyricsEvenFrameRound",    0 },
      { Sid::lyricsEvenFrameFgColor,  "lyricsEvenFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::lyricsEvenFrameBgColor,  "lyricsEvenFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::figuredBassFontFamily,   "figuredBassFontFamily",   QString("MScoreBC") },

//      { Sid::figuredBassFontSize,     "figuredBassFontSize",     QVariant(8.0) },
      { Sid::figuredBassYOffset,      "figuredBassYOffset",      QVariant(6.0) },
      { Sid::figuredBassLineHeight,   "figuredBassLineHeight",   QVariant(1.0) },
      { Sid::figuredBassAlignment,    "figuredBassAlignment",    QVariant(0) },
      { Sid::figuredBassStyle,        "figuredBassStyle" ,       QVariant(0) },
      { Sid::systemFrameDistance,     "systemFrameDistance",     Spatium(7.0) },
      { Sid::frameSystemDistance,     "frameSystemDistance",     Spatium(7.0) },
      { Sid::minMeasureWidth,         "minMeasureWidth",         Spatium(5.0) },
      { Sid::barWidth,                "barWidth",                Spatium(0.18) },
      { Sid::doubleBarWidth,          "doubleBarWidth",          Spatium(0.18) },

      { Sid::endBarWidth,             "endBarWidth",             Spatium(0.55) },
      { Sid::doubleBarDistance,       "doubleBarDistance",       Spatium(0.55) },
      { Sid::endBarDistance,          "endBarDistance",          Spatium(0.7) },
      { Sid::repeatBarlineDotSeparation, "repeatBarlineDotSeparation", Spatium(0.7) },
      { Sid::repeatBarTips,           "repeatBarTips",           QVariant(false) },
      { Sid::startBarlineSingle,      "startBarlineSingle",      QVariant(false) },
      { Sid::startBarlineMultiple,    "startBarlineMultiple",    QVariant(true) },

      { Sid::bracketWidth,            "bracketWidth",            Spatium(0.44) },
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

      { Sid::timesigLeftMargin,       "timesigLeftMargin",       Spatium(0.63) },
      { Sid::timesigScale,            "timesigScale",            QVariant(QSizeF(1.0, 1.0)) },
      { Sid::midClefKeyRightMargin,   "midClefKeyRightMargin",   Spatium(1.0) },
      { Sid::clefKeyRightMargin,      "clefKeyRightMargin",      Spatium(0.8) },
      { Sid::clefKeyDistance,         "clefKeyDistance",         Spatium(1.0) },   // gould: 1 - 1.25
      { Sid::clefTimesigDistance,     "clefTimesigDistance",     Spatium(1.0) },
      { Sid::keyTimesigDistance,      "keyTimesigDistance",      Spatium(1.0) },    // gould: 1 - 1.5
      { Sid::keyBarlineDistance,      "keyBarlineDistance",      Spatium(1.0) },
      { Sid::systemHeaderDistance,    "systemHeaderDistance",    Spatium(2.5) },     // gould: 2.5
      { Sid::systemHeaderTimeSigDistance, "systemHeaderTimeSigDistance", Spatium(2.0) },  // gould: 2.0
      { Sid::systemTrailerRightMargin, "systemTrailerRightMargin", Spatium(0.5) },

      { Sid::clefBarlineDistance,     "clefBarlineDistance",     Spatium(0.5) },
      { Sid::timesigBarlineDistance,  "timesigBarlineDistance",  Spatium(0.5) },
      { Sid::stemWidth,               "stemWidth",               Spatium(0.11) },
      { Sid::shortenStem,             "shortenStem",             QVariant(true) },
      { Sid::shortStemProgression,    "shortStemProgression",    Spatium(0.25) },
      { Sid::shortestStem,            "shortestStem",            Spatium(2.25) },
      { Sid::beginRepeatLeftMargin,   "beginRepeatLeftMargin",   Spatium(1.0) },
      { Sid::minNoteDistance,         "minNoteDistance",         Spatium(0.2) },
      { Sid::barNoteDistance,         "barNoteDistance",         Spatium(1.3) },     // was 1.2

      { Sid::barAccidentalDistance,   "barAccidentalDistance",   Spatium(0.65)   },
      { Sid::multiMeasureRestMargin,  "multiMeasureRestMargin",  Spatium(1.2)  },
      { Sid::noteBarDistance,         "noteBarDistance",         Spatium(1.5)  },
      { Sid::measureSpacing,          "measureSpacing",          QVariant(1.2) },
      { Sid::staffLineWidth,          "staffLineWidth",          Spatium(0.11) },
      { Sid::ledgerLineWidth,         "ledgerLineWidth",         Spatium(0.16) },     // 0.1875
      { Sid::ledgerLineLength,        "ledgerLineLength",        Spatium(0.35) },     // notehead width + this value
      { Sid::accidentalDistance,      "accidentalDistance",      Spatium(0.22) },
      { Sid::accidentalNoteDistance,  "accidentalNoteDistance",  Spatium(0.25) },
      { Sid::bracketedAccidentalPadding,  "bracketedAccidentalPadding",  Spatium(0.175) }, // Padding inside parentheses for bracketed accidentals
      { Sid::alignAccidentalsLeft,    "alignAccidentalsLeft",    QVariant(false) },   // When laid out in columns, whether accidentals align left or right. Musescore <= 3.5 uses left alignment.

      { Sid::beamWidth,               "beamWidth",               Spatium(0.5)  },     // was 0.48
      { Sid::beamDistance,            "beamDistance",            QVariant(0.5) },
      { Sid::beamMinLen,              "beamMinLen",              Spatium(1.3) },     // 1.316178 exactly notehead widthen beams
      { Sid::beamNoSlope,             "beamNoSlope",             QVariant(false) },

      { Sid::dotMag,                  "dotMag",                  QVariant(1.0) },
      { Sid::dotNoteDistance,         "dotNoteDistance",         Spatium(0.5) },
      { Sid::dotRestDistance,         "dotRestDistance",         Spatium(0.25) },
      { Sid::dotDotDistance,          "dotDotDistance",          Spatium(0.65) },
      { Sid::propertyDistanceHead,    "propertyDistanceHead",    Spatium(1.0) },
      { Sid::propertyDistanceStem,    "propertyDistanceStem",    Spatium(1.8) },
      { Sid::propertyDistance,        "propertyDistance",        Spatium(1.0) },

      { Sid::articulationMag,         "articulationMag",         QVariant(1.0) },
      { Sid::articulationPosAbove,    "articulationPosAbove",    QPointF(0.0, 0.0) },
      { Sid::articulationAnchorDefault, "articulationAnchorDefault", int(ArticulationAnchor::CHORD) },
      { Sid::articulationAnchorLuteFingering, "articulationAnchorLuteFingering", int(ArticulationAnchor::BOTTOM_CHORD) },
      { Sid::articulationAnchorOther, "articulationAnchorOther", int(ArticulationAnchor::TOP_STAFF) },
      { Sid::lastSystemFillLimit,     "lastSystemFillLimit",     QVariant(0.3) },

      { Sid::hairpinPlacement,        "hairpinPlacement",        int(Placement::BELOW)  },
      { Sid::hairpinPosAbove,         "hairpinPosAbove",         QPointF(0.0, -2.0) },
      { Sid::hairpinPosBelow,         "hairpinPosBelow",         QPointF(.0, 2) },
      { Sid::hairpinLinePosAbove,     "hairpinLinePosAbove",     QPointF(0.0, -3.0) },
      { Sid::hairpinLinePosBelow,     "hairpinLinePosBelow",     QPointF(.0, 4.0) },
      { Sid::hairpinHeight,           "hairpinHeight",           Spatium(1.15) },
      { Sid::hairpinContHeight,       "hairpinContHeight",       Spatium(0.5) },
      { Sid::hairpinLineWidth,        "hairpinWidth",            Spatium(0.12) },
      { Sid::hairpinFontFace,         "hairpinFontFace",         "Edwin" },
      { Sid::hairpinFontSize,         "hairpinFontSize",         10.0 },
      { Sid::hairpinLineSpacing,      "hairpinLineSpacing",      1.0 },
      { Sid::hairpinFontSpatiumDependent, "hairpinFontSpatiumDependent", true },
      { Sid::hairpinFontStyle,        "hairpinFontStyle",        int(FontStyle::Italic) },
      { Sid::hairpinColor,            "hairpinColor",            QColor(0, 0, 0, 255) },
      { Sid::hairpinTextAlign,        "hairpinTextAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::hairpinFrameType,        "hairpinFrameType",        int(FrameType::NO_FRAME) },
      { Sid::hairpinFramePadding,     "hairpinFramePadding",     0.2 },
      { Sid::hairpinFrameWidth,       "hairpinFrameWidth",       0.1 },
      { Sid::hairpinFrameRound,       "hairpinFrameRound",       0 },
      { Sid::hairpinFrameFgColor,     "hairpinFrameFgColor",     QColor(0, 0, 0, 255) },
      { Sid::hairpinFrameBgColor,     "hairpinFrameBgColor",     QColor(255, 255, 255, 0) },
      { Sid::hairpinText,             "hairpinText",             QString() },
      { Sid::hairpinCrescText,        "hairpinCrescText",        QString("cresc.") },
      { Sid::hairpinDecrescText,      "hairpinDecrescText",      QString("dim.") },
      { Sid::hairpinCrescContText,    "hairpinCrescContText",    QString("(cresc.)") },
      { Sid::hairpinDecrescContText,  "hairpinDecrescContText",  QString("(dim.)") },
      { Sid::hairpinLineStyle,        "hairpinLineStyle",        QVariant(int(Qt::SolidLine)) },
      { Sid::hairpinLineLineStyle,    "hairpinLineLineStyle",    QVariant(int(Qt::CustomDashLine)) },

      { Sid::pedalPlacement,          "pedalPlacement",          int(Placement::BELOW)  },
      { Sid::pedalPosAbove,           "pedalPosAbove",           QPointF(.0, -1) },
      { Sid::pedalPosBelow,           "pedalPosBelow",           QPointF(.0, 2.5) },
      { Sid::pedalLineWidth,          "pedalLineWidth",          Spatium(0.11) },
      { Sid::pedalLineStyle,          "pedalListStyle",          QVariant(int(Qt::SolidLine)) },
      { Sid::pedalBeginTextOffset,    "pedalBeginTextOffset",    QPointF(0.0, 0.15) },
      { Sid::pedalHookHeight,         "pedalHookHeight",         Spatium(-1.2) },
      { Sid::pedalFontFace,           "pedalFontFace",           "Edwin" },
      { Sid::pedalFontSize,           "pedalFontSize",           12.0 },
      { Sid::pedalLineSpacing,        "pedalLineSpacing",        1.0 },
      { Sid::pedalFontSpatiumDependent, "pedalFontSpatiumDependent", true },
      { Sid::pedalFontStyle,          "pedalFontStyle",          int(FontStyle::Normal) },
      { Sid::pedalColor,              "pedalColor",              QColor(0, 0, 0, 255) },
      { Sid::pedalTextAlign,          "pedalTextAlign",          QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::pedalFrameType,          "pedalFrameType",          int(FrameType::NO_FRAME) },
      { Sid::pedalFramePadding,       "pedalFramePadding",       0.2 },
      { Sid::pedalFrameWidth,         "pedalFrameWidth",         0.1 },
      { Sid::pedalFrameRound,         "pedalFrameRound",         0 },
      { Sid::pedalFrameFgColor,       "pedalFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::pedalFrameBgColor,       "pedalFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::trillPlacement,          "trillPlacement",          int(Placement::ABOVE)  },
      { Sid::trillPosAbove,           "trillPosAbove",           QPointF(.0, -0.5) },
      { Sid::trillPosBelow,           "trillPosBelow",           QPointF(.0, 2) },

      { Sid::vibratoPlacement,        "vibratoPlacement",        int(Placement::ABOVE)  },
      { Sid::vibratoPosAbove,         "vibratoPosAbove",         QPointF(.0, -1) },
      { Sid::vibratoPosBelow,         "vibratoPosBelow",         QPointF(.0, 1) },

      { Sid::harmonyFretDist,          "harmonyFretDist",        Spatium(1.0) },
      { Sid::minHarmonyDistance,       "minHarmonyDistance",     Spatium(0.5) },
      { Sid::maxHarmonyBarDistance,    "maxHarmonyBarDistance",  Spatium(3.0) },
      { Sid::maxChordShiftAbove,       "maxChordShiftAbove",     Spatium(0.0) },
      { Sid::maxChordShiftBelow,       "maxChordShiftBelow",     Spatium(0.0) },

      { Sid::harmonyPlacement,         "harmonyPlacement",           int(Placement::ABOVE) },
      { Sid::romanNumeralPlacement,    "romanNumeralPlacement",      int(Placement::BELOW) },
      { Sid::nashvilleNumberPlacement, "nashvilleNumberPlacement",   int(Placement::ABOVE) },
      { Sid::harmonyPlay,              "harmonyPlay",                true },
      { Sid::harmonyVoiceLiteral,      "harmonyVoiceLiteral",        true },
      { Sid::harmonyVoicing,           "harmonyVoicing",             int(Voicing::AUTO) },
      { Sid::harmonyDuration,          "harmonyDuration",            int(HDuration::UNTIL_NEXT_CHORD_SYMBOL) },

      { Sid::chordSymbolAPosAbove,      "chordSymbolPosAbove",       QPointF(.0, -2.5) },
      { Sid::chordSymbolAPosBelow,      "chordSymbolPosBelow",       QPointF(.0, 3.5) },

      { Sid::chordSymbolBPosAbove,      "chordSymbolBPosAbove",      QPointF(.0, -5.0) },
      { Sid::chordSymbolBPosBelow,      "chordSymbolBPosBelow",      QPointF(.0, 3.5) },

      { Sid::romanNumeralPosAbove,      "romanNumeralPosAbove",      QPointF(.0, -2.5) },
      { Sid::romanNumeralPosBelow,      "romanNumeralPosBelow",      QPointF(.0, 3.5) },

      { Sid::nashvilleNumberPosAbove,   "nashvilleNumberPosAbove",   QPointF(.0, -2.5) },
      { Sid::nashvilleNumberPosBelow,   "nashvilleNumberPosBelow",   QPointF(.0, 3.5) },

      { Sid::chordSymbolAFontFace,      "chordSymbolAFontFace",      "Edwin" },
      { Sid::chordSymbolAFontSize,      "chordSymbolAFontSize",      11.0 },
      { Sid::chordSymbolALineSpacing,   "chordSymbolALineSpacing",   1.0 },
      { Sid::chordSymbolAFontSpatiumDependent, "chordSymbolAFontSpatiumDependent", true },
      { Sid::chordSymbolAFontStyle,     "chordSymbolAFontStyle",     int(FontStyle::Normal) },
      { Sid::chordSymbolAColor,         "chordSymbolAColor",         QColor(0, 0, 0, 255) },
      { Sid::chordSymbolAAlign,         "chordSymbolAAlign",         QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::chordSymbolAFrameType,     "chordSymbolAFrameType",     int(FrameType::NO_FRAME) },
      { Sid::chordSymbolAFramePadding,  "chordSymbolAFramePadding",  0.2 },
      { Sid::chordSymbolAFrameWidth,    "chordSymbolAFrameWidth",    0.1 },
      { Sid::chordSymbolAFrameRound,    "chordSymbolAFrameRound",    0 },
      { Sid::chordSymbolAFrameFgColor,  "chordSymbolAFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::chordSymbolAFrameBgColor,  "chordSymbolAFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::chordSymbolBFontFace,      "chordSymbolBFontFace",      "Edwin" },
      { Sid::chordSymbolBFontSize,      "chordSymbolBFontSize",      11.0 },
      { Sid::chordSymbolBLineSpacing,   "chordSymbolBLineSpacing",   1.0 },
      { Sid::chordSymbolBFontSpatiumDependent, "chordSymbolBFontSpatiumDependent", true },
      { Sid::chordSymbolBFontStyle,     "chordSymbolBFontStyle",     int(FontStyle::Italic) },
      { Sid::chordSymbolBColor,         "chordSymbolBColor",         QColor(0, 0, 0, 255) },
      { Sid::chordSymbolBAlign,         "chordSymbolBAlign",         QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::chordSymbolBFrameType,     "chordSymbolBFrameType",     int(FrameType::NO_FRAME) },
      { Sid::chordSymbolBFramePadding,  "chordSymbolBFramePadding",  0.2 },
      { Sid::chordSymbolBFrameWidth,    "chordSymbolBFrameWidth",    0.1 },
      { Sid::chordSymbolBFrameRound,    "chordSymbolBFrameRound",    0 },
      { Sid::chordSymbolBFrameFgColor,  "chordSymbolBFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::chordSymbolBFrameBgColor,  "chordSymbolBFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::romanNumeralFontFace,      "romanNumeralFontFace",      "Campania" },
      { Sid::romanNumeralFontSize,      "romanNumeralFontSize",      12.0 },
      { Sid::romanNumeralLineSpacing,   "romanNumeralLineSpacing",   1.0 },
      { Sid::romanNumeralFontSpatiumDependent, "romanNumeralFontSpatiumDependent", true },
      { Sid::romanNumeralFontStyle,     "romanNumeralFontStyle",     int(FontStyle::Normal) },
      { Sid::romanNumeralColor,         "romanNumeralColor",         QColor(0, 0, 0, 255) },
      { Sid::romanNumeralAlign,         "romanNumeralAlign",         QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::romanNumeralFrameType,     "romanNumeralFrameType",     int(FrameType::NO_FRAME) },
      { Sid::romanNumeralFramePadding,  "romanNumeralFramePadding",  0.2 },
      { Sid::romanNumeralFrameWidth,    "romanNumeralFrameWidth",    0.1 },
      { Sid::romanNumeralFrameRound,    "romanNumeralFrameRound",    0 },
      { Sid::romanNumeralFrameFgColor,  "romanNumeralFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::romanNumeralFrameBgColor,  "romanNumeralFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::nashvilleNumberFontFace,      "nashvilleNumberFontFace",      "Edwin" },
      { Sid::nashvilleNumberFontSize,      "nashvilleNumberFontSize",      12.0 },
      { Sid::nashvilleNumberLineSpacing,   "nashvilleNumberLineSpacing",   1.0 },
      { Sid::nashvilleNumberFontSpatiumDependent, "nashvilleNumberFontSpatiumDependent", true },
      { Sid::nashvilleNumberFontStyle,     "nashvilleNumberFontStyle",     int(FontStyle::Normal) },
      { Sid::nashvilleNumberColor,         "nashvilleNumberColor",         QColor(0, 0, 0, 255) },
      { Sid::nashvilleNumberAlign,         "nashvilleNumberAlign",         QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::nashvilleNumberFrameType,     "nashvilleNumberFrameType",     int(FrameType::NO_FRAME) },
      { Sid::nashvilleNumberFramePadding,  "nashvilleNumberFramePadding",  0.2 },
      { Sid::nashvilleNumberFrameWidth,    "nashvilleNumberFrameWidth",    0.1 },
      { Sid::nashvilleNumberFrameRound,    "nashvilleNumberFrameRound",    0 },
      { Sid::nashvilleNumberFrameFgColor,  "nashvilleNumberFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::nashvilleNumberFrameBgColor,  "nashvilleNumberFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::capoPosition,            "capoPosition",            QVariant(0) },
      { Sid::fretNumMag,              "fretNumMag",              QVariant(2.0) },
      { Sid::fretNumPos,              "fretNumPos",              QVariant(0) },
      { Sid::fretY,                   "fretY",                   Spatium(1.0) },
      { Sid::fretMinDistance,         "fretMinDistance",         Spatium(0.5) },
      { Sid::fretMag,                 "fretMag",                 QVariant(1.0) },
      { Sid::fretPlacement,           "fretPlacement",           int(Placement::ABOVE) },
      { Sid::fretStrings,             "fretStrings",             6 },
      { Sid::fretFrets,               "fretFrets",               5 },
      { Sid::fretNut,                 "fretNut",                 QVariant(true) },
      { Sid::fretDotSize,             "fretDotSize",             QVariant(1.0) },
      { Sid::fretStringSpacing,       "fretStringSpacing",       Spatium(0.7) },
      { Sid::fretFretSpacing,         "fretFretSpacing",         Spatium(0.8) },
      { Sid::fretOrientation,         "fretOrientation",         int(Orientation::VERTICAL) },
      { Sid::maxFretShiftAbove,       "maxFretShiftAbove",       Spatium(0.0) },
      { Sid::maxFretShiftBelow,       "maxFretShiftBelow",       Spatium(0.0) },

      { Sid::showPageNumber,          "showPageNumber",          QVariant(true) },
      { Sid::showPageNumberOne,       "showPageNumberOne",       QVariant(false) },
      { Sid::pageNumberOddEven,       "pageNumberOddEven",       QVariant(true) },
      { Sid::showMeasureNumber,       "showMeasureNumber",       QVariant(true) },
      { Sid::showMeasureNumberOne,    "showMeasureNumberOne",    QVariant(false) },
      { Sid::measureNumberInterval,   "measureNumberInterval",   QVariant(5) },
      { Sid::measureNumberSystem,     "measureNumberSystem",     QVariant(true) },
      { Sid::measureNumberAllStaves,  "measureNumberAllStaffs",  QVariant(false) }, // need to keep staffs and not staves here for backward compatibility
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
      { Sid::chordExtensionMag,       "chordExtensionMag",       QVariant(1.0)   },
      { Sid::chordExtensionAdjust,    "chordExtensionAdjust",    QVariant(0.0)   },
      { Sid::chordModifierMag,        "chordModifierMag",        QVariant(1.0)   },
      { Sid::chordModifierAdjust,     "chordModifierAdjust",     QVariant(0.0)   },
      { Sid::concertPitch,            "concertPitch",            QVariant(false) },

      { Sid::createMultiMeasureRests, "createMultiMeasureRests", QVariant(false) },
      { Sid::minEmptyMeasures,        "minEmptyMeasures",        QVariant(2) },
      { Sid::minMMRestWidth,          "minMMRestWidth",          Spatium(4) },
      { Sid::mmRestNumberPos,         "mmRestNumberPos",         Spatium(-1.5) },
      { Sid::hideEmptyStaves,         "hideEmptyStaves",         QVariant(false) },
      { Sid::dontHideStavesInFirstSystem,
                                 "dontHidStavesInFirstSystm",    QVariant(true) },
      { Sid::enableIndentationOnFirstSystem,
                                 "enableIndentationOnFirstSystem", QVariant(true) },
      { Sid::firstSystemIndentationValue, "firstSystemIndentationValue", Spatium(5.0) },
      { Sid::alwaysShowBracketsWhenEmptyStavesAreHidden,
                                 "alwaysShowBracketsWhenEmptyStavesAreHidden", QVariant(false) },
      { Sid::hideInstrumentNameIfOneInstrument,
                                 "hideInstrumentNameIfOneInstrument", QVariant(true) },
      { Sid::gateTime,                "gateTime",                QVariant(100) },
      { Sid::tenutoGateTime,          "tenutoGateTime",          QVariant(100) },
      { Sid::staccatoGateTime,        "staccatoGateTime",        QVariant(50) },
      { Sid::slurGateTime,            "slurGateTime",            QVariant(100) },

      { Sid::ArpeggioNoteDistance,    "ArpeggioNoteDistance",    Spatium(.5) },
      { Sid::ArpeggioAccidentalDistance,    "ArpeggioAccidentalDistance",    Spatium(.5) },
      { Sid::ArpeggioAccidentalDistanceMin,    "ArpeggioAccidentalDistanceMin",    Spatium(.33) },
      { Sid::ArpeggioLineWidth,       "ArpeggioLineWidth",       Spatium(.18) },
      { Sid::ArpeggioHookLen,         "ArpeggioHookLen",         Spatium(.8) },
      { Sid::ArpeggioHiddenInStdIfTab,"ArpeggioHiddenInStdIfTab",QVariant(false)},
      { Sid::SlurEndWidth,            "slurEndWidth",            Spatium(.07) },
      { Sid::SlurMidWidth,            "slurMidWidth",            Spatium(.21) },
      { Sid::SlurDottedWidth,         "slurDottedWidth",         Spatium(.10)  },
      { Sid::MinTieLength,            "minTieLength",            Spatium(1.0) },
      { Sid::SlurMinDistance,         "slurMinDistance",         Spatium(0.5) },
      { Sid::SectionPause,            "sectionPause",            QVariant(qreal(3.0)) },
      { Sid::MusicalSymbolFont,       "musicalSymbolFont",       QVariant(QString("Leland")) },
      { Sid::MusicalTextFont,         "musicalTextFont",         QVariant(QString("Leland Text")) },

      { Sid::showHeader,              "showHeader",              QVariant(true) },
      { Sid::headerFirstPage,         "headerFirstPage",         QVariant(false) },
      { Sid::headerOddEven,           "headerOddEven",           QVariant(true) },
      { Sid::evenHeaderL,             "evenHeaderL",             QVariant(QString("$p")) },
      { Sid::evenHeaderC,             "evenHeaderC",             QVariant(QString()) },
      { Sid::evenHeaderR,             "evenHeaderR",             QVariant(QString()) },
      { Sid::oddHeaderL,              "oddHeaderL",              QVariant(QString()) },
      { Sid::oddHeaderC,              "oddHeaderC",              QVariant(QString()) },
      { Sid::oddHeaderR,              "oddHeaderR",              QVariant(QString("$p")) },
      { Sid::showFooter,              "showFooter",              QVariant(true) },

      { Sid::footerFirstPage,         "footerFirstPage",         QVariant(true) },
      { Sid::footerOddEven,           "footerOddEven",           QVariant(true) },
      { Sid::footerInsideMargins,     "footerInsideMargins",     QVariant(false) },
      { Sid::evenFooterL,             "evenFooterL",             QVariant(QString()) },
      { Sid::evenFooterC,             "evenFooterC",             QVariant(QString("$:copyright:")) },
      { Sid::evenFooterR,             "evenFooterR",             QVariant(QString()) },
      { Sid::oddFooterL,              "oddFooterL",              QVariant(QString()) },
      { Sid::oddFooterC,              "oddFooterC",              QVariant(QString("$:copyright:")) },
      { Sid::oddFooterR,              "oddFooterR",              QVariant(QString()) },

      { Sid::voltaPosAbove,           "voltaPosAbove",           QPointF(0.0, -3.0) },
      { Sid::voltaHook,               "voltaHook",               Spatium(2.2) },
      { Sid::voltaLineWidth,          "voltaLineWidth",          Spatium(0.11) },
      { Sid::voltaLineStyle,          "voltaLineStyle",          QVariant(int(Qt::SolidLine)) },
      { Sid::voltaFontFace,           "voltaFontFace",           "Edwin" },
      { Sid::voltaFontSize,           "voltaFontSize",           11.0 },
      { Sid::voltaLineSpacing,        "voltaLineSpacing",        1.0 },
      { Sid::voltaFontSpatiumDependent, "voltaFontSpatiumDependent", true },
      { Sid::voltaFontStyle,          "voltaFontStyle",          int(FontStyle::Bold) },
      { Sid::voltaColor,              "voltaColor",              QColor(0, 0, 0, 255) },
      { Sid::voltaAlign,              "voltaAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::voltaOffset,             "voltaOffset",             QPointF(0.6, 2.2) },
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
      { Sid::ottava8VAContinueText,   "ottava8VAContinueText",   QString("<sym>ottavaAlta</sym>") },
      { Sid::ottava8VBText,           "ottava8VBText",           QString("<sym>ottavaBassa</sym>") },
      { Sid::ottava8VBContinueText,   "ottava8VBContinueText",   QString("<sym>ottavaBassa</sym>") },
      { Sid::ottava15MAText,          "ottava15MAText",          QString("<sym>quindicesimaAlta</sym>") },
      { Sid::ottava15MAContinueText,  "ottava15MAContinueText",  QString("<sym>quindicesimaAlta</sym>") },
      { Sid::ottava15MBText,          "ottava15MBText",          QString("<sym>quindicesimaBassa</sym>") },
      { Sid::ottava15MBContinueText,  "ottava15MBContinueText",  QString("<sym>quindicesimaBassa</sym>") },
      { Sid::ottava22MAText,          "ottava22MAText",          QString("<sym>ventiduesimaAlta</sym>") },
      { Sid::ottava22MAContinueText,  "ottava22MAContinueText",  QString("<sym>ventiduesimaAlta</sym>") },
      { Sid::ottava22MBText,          "ottava22MBText",          QString("<sym>ventiduesimaBassa</sym>") },
      { Sid::ottava22MBContinueText, "ottava22MBContinueText",   QString("<sym>ventiduesimaBassa</sym>") },

      { Sid::ottava8VAnoText,         "ottava8VAnoText",         QString("<sym>ottava</sym>") },
      { Sid::ottava8VAnoContinueText, "ottava8VAnoContinueText", QString("<sym>ottava</sym>") },
      { Sid::ottava8VBnoText,         "ottava8VBnoText",         QString("<sym>ottava</sym>") },
      { Sid::ottava8VBnoContinueText, "ottava8VBnoContinueText", QString("<sym>ottava</sym>") },
      { Sid::ottava15MAnoText,        "ottava15MAnoText",        QString("<sym>quindicesima</sym>") },
      { Sid::ottava15MAnoContinueText,"ottava15MAnoContinueText",QString("<sym>quindicesima</sym>") },
      { Sid::ottava15MBnoText,        "ottava15MBnoText",        QString("<sym>quindicesima</sym>") },
      { Sid::ottava15MBnoContinueText,"ottava15MBnoContinueText",QString("<sym>quindicesima</sym>") },
      { Sid::ottava22MAnoText,        "ottava22MAnoText",        QString("<sym>ventiduesima</sym>") },
      { Sid::ottava22MAnoContinueText,"ottava22MAnoContinueText",QString("<sym>ventiduesima</sym>") },
      { Sid::ottava22MBnoText,        "ottava22MBnoText",        QString("<sym>ventiduesima</sym>") },
      { Sid::ottava22MBnoContinueText,"ottava22MBnoContinueText",QString("<sym>ventiduesima</sym>") },

      { Sid::ottavaPosAbove,          "ottavaPosAbove",          QPointF(.0, -2.0) },
      { Sid::ottavaPosBelow,          "ottavaPosBelow",          QPointF(.0, 2.0) },
      { Sid::ottavaHookAbove,         "ottavaHookAbove",         Spatium(1) },
      { Sid::ottavaHookBelow,         "ottavaHookBelow",         Spatium(-1) },
      { Sid::ottavaLineWidth,         "ottavaLineWidth",         Spatium(0.11) },
      { Sid::ottavaLineStyle,         "ottavaLineStyle",         QVariant(int(Qt::DashLine)) },
      { Sid::ottavaNumbersOnly,       "ottavaNumbersOnly",       true },
      { Sid::ottavaFontFace,          "ottavaFontFace",          "Edwin" },
      { Sid::ottavaFontSize,          "ottavaFontSize",          10.0 },
      { Sid::ottavaLineSpacing,       "ottavaLineSpacing",       1.0 },
      { Sid::ottavaFontSpatiumDependent, "ottavaFontSpatiumDependent", true },
      { Sid::ottavaFontStyle,         "ottavaFontStyle",         int(FontStyle::Normal) },
      { Sid::ottavaColor,             "ottavaColor",             QColor(0, 0, 0, 255) },
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
      { Sid::tremoloStyle,            "tremoloStrokeStyle",      int(TremoloStyle::DEFAULT) },
      { Sid::tremoloStrokeLengthMultiplier, "tremoloStrokeLengthMultiplier", 0.62 },

      { Sid::linearStretch,           "linearStretch",           QVariant(qreal(1.5)) },
      { Sid::crossMeasureValues,      "crossMeasureValues",      QVariant(false) },
      { Sid::keySigNaturals,          "keySigNaturals",          QVariant(int(KeySigNatural::NONE)) },

      { Sid::tupletMaxSlope,          "tupletMaxSlope",          QVariant(qreal(0.5)) },
      { Sid::tupletOufOfStaff,        "tupletOufOfStaff",        QVariant(true) },
      { Sid::tupletVHeadDistance,     "tupletVHeadDistance",     Spatium(.5) },
      { Sid::tupletVStemDistance,     "tupletVStemDistance",     Spatium(.5) },
      { Sid::tupletStemLeftDistance,  "tupletStemLeftDistance",  Spatium(0.0) },
      { Sid::tupletStemRightDistance, "tupletStemRightDistance", Spatium(.5) },
      { Sid::tupletNoteLeftDistance,  "tupletNoteLeftDistance",  Spatium(-0.5) },
      { Sid::tupletNoteRightDistance, "tupletNoteRightDistance", Spatium(0.0) },
      { Sid::tupletBracketWidth,      "tupletBracketWidth",      Spatium(0.1) },
      { Sid::tupletDirection,         "tupletDirection",         QVariant::fromValue<Direction>(Direction::AUTO) },
      { Sid::tupletNumberType,        "tupletNumberType",        int(TupletNumberType::SHOW_NUMBER) },
      { Sid::tupletBracketType,       "tupletBracketType",       int(TupletBracketType::AUTO_BRACKET) },
      { Sid::tupletFontFace,          "tupletFontFace",          "Edwin" },
      { Sid::tupletFontSize,          "tupletFontSize",          9.0 },
      { Sid::tupletLineSpacing,       "tupletLineSpacing",       1.0 },
      { Sid::tupletFontSpatiumDependent, "tupletFontSpatiumDependent", true },
      { Sid::tupletFontStyle,         "tupletFontStyle",         int(FontStyle::Italic) },
      { Sid::tupletColor,             "tupletColor",             QColor(0, 0, 0, 255) },
      { Sid::tupletAlign,             "tupletAlign",             QVariant::fromValue(Align::CENTER) },
      { Sid::tupletBracketHookHeight, "tupletBracketHookHeight", Spatium(0.75) },
      { Sid::tupletOffset,            "tupletOffset",            QPointF()  },
      { Sid::tupletFrameType,         "tupletFrameType",         int(FrameType::NO_FRAME) },
      { Sid::tupletFramePadding,      "tupletFramePadding",      0.2 },
      { Sid::tupletFrameWidth,        "tupletFrameWidth",        0.1 },
      { Sid::tupletFrameRound,        "tupletFrameRound",        0 },
      { Sid::tupletFrameFgColor,      "tupletFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::tupletFrameBgColor,      "tupletFrameBgColor",      QColor(255, 255, 255, 0) },

      { Sid::barreLineWidth,          "barreLineWidth",          QVariant(1.0) },
      { Sid::scaleBarlines,           "scaleBarlines",           QVariant(true) },
      { Sid::barGraceDistance,        "barGraceDistance",        Spatium(1.0) },
      { Sid::minVerticalDistance,     "minVerticalDistance",     Spatium(0.5) },
      { Sid::ornamentStyle,           "ornamentStyle",           int(MScore::OrnamentStyle::DEFAULT) },
      { Sid::spatium,                 "Spatium",                 24.8 }, // 1.75 * DPMM

      { Sid::autoplaceHairpinDynamicsDistance, "autoplaceHairpinDynamicsDistance", Spatium(0.5) },

      { Sid::dynamicsPlacement,       "dynamicsPlacement",       int(Placement::BELOW)  },
      { Sid::dynamicsPosAbove,        "dynamicsPosAbove",        QPointF(.0, -1.5) },
      { Sid::dynamicsPosBelow,        "dynamicsPosBelow",        QPointF(.0, 2.5) },

      { Sid::dynamicsMinDistance,         "dynamicsMinDistance",               Spatium(0.5) },
      { Sid::autoplaceVerticalAlignRange, "autoplaceVerticalAlignRange",     int(VerticalAlignRange::SYSTEM) },

      { Sid::textLinePlacement,         "textLinePlacement",         int(Placement::ABOVE)  },
      { Sid::textLinePosAbove,          "textLinePosAbove",          QPointF(.0, -1.0) },
      { Sid::textLinePosBelow,          "textLinePosBelow",          QPointF(.0, 1.0) },
      { Sid::textLineFrameType,         "textLineFrameType",          int(FrameType::NO_FRAME) },
      { Sid::textLineFramePadding,      "textLineFramePadding",       0.2 },
      { Sid::textLineFrameWidth,        "textLineFrameWidth",         0.1 },
      { Sid::textLineFrameRound,        "textLineFrameRound",         0 },
      { Sid::textLineFrameFgColor,      "textLineFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::textLineFrameBgColor,      "textLineFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::systemTextLinePlacement,         "systemTextLinePlacement",         int(Placement::ABOVE)  },
      { Sid::systemTextLinePosAbove,          "systemTextLinePosAbove",          QPointF(.0, -1.0) },
      { Sid::systemTextLinePosBelow,          "systemTextLinePosBelow",          QPointF(.0, 1.0) },
      { Sid::systemTextLineFrameType,         "systemTextLineFrameType",          int(FrameType::NO_FRAME) },
      { Sid::systemTextLineFramePadding,      "systemTextLineFramePadding",       0.2 },
      { Sid::systemTextLineFrameWidth,        "systemTextLineFrameWidth",         0.1 },
      { Sid::systemTextLineFrameRound,        "systemTextLineFrameRound",         0 },
      { Sid::systemTextLineFrameFgColor,      "systemTextLineFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::systemTextLineFrameBgColor,      "systemTextLineFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::tremoloBarLineWidth,       "tremoloBarLineWidth",       Spatium(0.12) },
      { Sid::jumpPosAbove,              "jumpPosAbove",              QPointF(.0, -2.0) },
      { Sid::markerPosAbove,            "markerPosAbove",            QPointF(.0, -2.0) },

      { Sid::defaultFontFace,               "defaultFontFace",               "Edwin" },
      { Sid::defaultFontSize,               "defaultFontSize",               10.0  },
      { Sid::defaultLineSpacing,            "defaultLineSpacing",            1.0 },
      { Sid::defaultFontSpatiumDependent,   "defaultFontSpatiumDependent",   true  },
      { Sid::defaultFontStyle,              "defaultFontStyle",              int(FontStyle::Normal) },
      { Sid::defaultColor,                  "defaultColor",                  QColor(0, 0, 0, 255) },
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

      { Sid::titleFontFace,                 "titleFontFace",                 "Edwin" },
      { Sid::titleFontSize,                 "titleFontSize",                 22.0 },
      { Sid::titleLineSpacing,              "titleLineSpacing",              1.0 },
      { Sid::titleFontSpatiumDependent,     "titleFontSpatiumDependent",     false  },
      { Sid::titleFontStyle,                "titleFontStyle",                int(FontStyle::Normal) },
      { Sid::titleColor,                    "titleColor",                    QColor(0, 0, 0, 255) },
      { Sid::titleAlign,                    "titleAlign",                    QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { Sid::titleOffset,                   "titleOffset",                   QPointF() },
      { Sid::titleOffsetType,               "titleOffsetType",               int(OffsetType::ABS)   },
      { Sid::titleFrameType,                "titleFrameType",                int(FrameType::NO_FRAME) },
      { Sid::titleFramePadding,             "titleFramePadding",             0.2 },
      { Sid::titleFrameWidth,               "titleFrameWidth",               0.1 },
      { Sid::titleFrameRound,               "titleFrameRound",               0 },
      { Sid::titleFrameFgColor,             "titleFrameFgColor",             QColor(0, 0, 0, 255) },
      { Sid::titleFrameBgColor,             "titleFrameBgColor",             QColor(255, 255, 255, 0) },

      { Sid::subTitleFontFace,              "subTitleFontFace",              "Edwin" },
      { Sid::subTitleFontSize,              "subTitleFontSize",              16.0 },
      { Sid::subTitleLineSpacing,           "subTitleLineSpacing",           1.0 },
      { Sid::subTitleFontSpatiumDependent,  "subTitleFontSpatiumDependent",  false  },
      { Sid::subTitleFontStyle,             "subTitleFontStyle",             int(FontStyle::Normal) },
      { Sid::subTitleColor,                 "subTitleColor",                 QColor(0, 0, 0, 255) },
      { Sid::subTitleAlign,                 "subTitleAlign",                 QVariant::fromValue(Align::HCENTER | Align::TOP) },
      { Sid::subTitleOffset,                "subTitleOffset",                QPointF(0.0, 10.0) },
      { Sid::subTitleOffsetType,            "subTitleOffsetType",            int(OffsetType::ABS)   },
      { Sid::subTitleFrameType,             "subTitleFrameType",             int(FrameType::NO_FRAME) },
      { Sid::subTitleFramePadding,          "subTitleFramePadding",          0.2 },
      { Sid::subTitleFrameWidth,            "subTitleFrameWidth",            0.1 },
      { Sid::subTitleFrameRound,            "subTitleFrameRound",            0 },
      { Sid::subTitleFrameFgColor,          "subTitleFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::subTitleFrameBgColor,          "subTitleFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::composerFontFace,              "composerFontFace",              "Edwin" },
      { Sid::composerFontSize,              "composerFontSize",              10.0 },
      { Sid::composerLineSpacing,           "composerLineSpacing",           1.0 },
      { Sid::composerFontSpatiumDependent,  "composerFontSpatiumDependent",  false  },
      { Sid::composerFontStyle,             "composerFontStyle",             int(FontStyle::Normal) },
      { Sid::composerColor,                 "composerColor",                 QColor(0, 0, 0, 255) },
      { Sid::composerAlign,                 "composerAlign",                 QVariant::fromValue(Align::RIGHT | Align::BOTTOM) },
      { Sid::composerOffset,                "composerOffset",                QPointF() },
      { Sid::composerOffsetType,            "composerOffsetType",            int(OffsetType::ABS)   },
      { Sid::composerFrameType,             "composerFrameType",             int(FrameType::NO_FRAME) },
      { Sid::composerFramePadding,          "composerFramePadding",          0.2 },
      { Sid::composerFrameWidth,            "composerFrameWidth",            0.1 },
      { Sid::composerFrameRound,            "composerFrameRound",            0 },
      { Sid::composerFrameFgColor,          "composerFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::composerFrameBgColor,          "composerFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::lyricistFontFace,              "lyricistFontFace",              "Edwin" },
      { Sid::lyricistFontSize,              "lyricistFontSize",              10.0 },
      { Sid::lyricistLineSpacing,           "lyricistLineSpacing",           1.0 },
      { Sid::lyricistFontSpatiumDependent,  "lyricistFontSpatiumDependent",  false  },
      { Sid::lyricistFontStyle,             "lyricistFontStyle",             int(FontStyle::Normal) },
      { Sid::lyricistColor,                 "lyricistColor",                 QColor(0, 0, 0, 255) },
      { Sid::lyricistAlign,                 "lyricistAlign",                 QVariant::fromValue(Align::LEFT | Align::BOTTOM) },
      { Sid::lyricistOffset,                "lyricistOffset",                QPointF() },
      { Sid::lyricistOffsetType,            "lyricistOffsetType",            int(OffsetType::ABS)   },
      { Sid::lyricistFrameType,             "lyricistFrameType",             int(FrameType::NO_FRAME) },
      { Sid::lyricistFramePadding,          "lyricistFramePadding",          0.2 },
      { Sid::lyricistFrameWidth,            "lyricistFrameWidth",            0.1 },
      { Sid::lyricistFrameRound,            "lyricistFrameRound",            0 },
      { Sid::lyricistFrameFgColor,          "lyricistFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::lyricistFrameBgColor,          "lyricistFrameBgColor",          QColor(255, 255, 255, 0) },

      { Sid::fingeringFontFace,             "fingeringFontFace",             "Edwin" },
      { Sid::fingeringFontSize,             "fingeringFontSize",             8.0 },
      { Sid::fingeringLineSpacing,          "fingeringLineSpacing",          1.0 },
      { Sid::fingeringFontSpatiumDependent, "fingeringFontSpatiumDependent", true },
      { Sid::fingeringFontStyle,            "fingeringFontStyle",             int(FontStyle::Normal) },
      { Sid::fingeringColor,                "fingeringColor",                QColor(0, 0, 0, 255) },
      { Sid::fingeringAlign,                "fingeringAlign",                QVariant::fromValue(Align::CENTER) },
      { Sid::fingeringFrameType,            "fingeringFrameType",            int(FrameType::NO_FRAME) },
      { Sid::fingeringFramePadding,         "fingeringFramePadding",         0.2 },
      { Sid::fingeringFrameWidth,           "fingeringFrameWidth",           0.1 },
      { Sid::fingeringFrameRound,           "fingeringFrameRound",           0 },
      { Sid::fingeringFrameFgColor,         "fingeringFrameFgColor",         QColor(0, 0, 0, 255) },
      { Sid::fingeringFrameBgColor,         "fingeringFrameBgColor",         QColor(255, 255, 255, 0) },
      { Sid::fingeringOffset,               "fingeringOffset",               QPointF() },

      { Sid::lhGuitarFingeringFontFace,     "lhGuitarFingeringFontFace",     "Edwin" },
      { Sid::lhGuitarFingeringFontSize,     "lhGuitarFingeringFontSize",     8.0 },
      { Sid::lhGuitarFingeringLineSpacing,  "lhGuitarFingeringLineSpacing",  1.0 },
      { Sid::lhGuitarFingeringFontSpatiumDependent, "lhGuitarFingeringFontSpatiumDependent", true },
      { Sid::lhGuitarFingeringFontStyle,    "lhGuitarFingeringFontStyle",    int(FontStyle::Normal) },
      { Sid::lhGuitarFingeringColor,        "lhGuitarFingeringColor",        QColor(0, 0, 0, 255) },
      { Sid::lhGuitarFingeringAlign,        "lhGuitarFingeringAlign",        QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::lhGuitarFingeringFrameType,    "lhGuitarFingeringFrameType",    int(FrameType::NO_FRAME) },
      { Sid::lhGuitarFingeringFramePadding, "lhGuitarFingeringFramePadding", 0.2 },
      { Sid::lhGuitarFingeringFrameWidth,   "lhGuitarFingeringFrameWidth",   0.1 },
      { Sid::lhGuitarFingeringFrameRound,   "lhGuitarFingeringFrameRound",   0 },
      { Sid::lhGuitarFingeringFrameFgColor, "lhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::lhGuitarFingeringFrameBgColor, "lhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { Sid::lhGuitarFingeringOffset,       "lhGuitarFingeringOffset",       QPointF(-0.5, 0.0) },

      { Sid::rhGuitarFingeringFontFace,     "rhGuitarFingeringFontFace",     "Edwin" },
      { Sid::rhGuitarFingeringFontSize,     "rhGuitarFingeringFontSize",     8.0 },
      { Sid::rhGuitarFingeringLineSpacing,  "rhGuitarFingeringLineSpacing",  1.0 },
      { Sid::rhGuitarFingeringFontSpatiumDependent, "rhGuitarFingeringFontSpatiumDependent", true },
      { Sid::rhGuitarFingeringFontStyle,    "rhGuitarFingeringFontStyle",    int(FontStyle::Italic) },
      { Sid::rhGuitarFingeringColor,        "rhGuitarFingeringColor",        QColor(0, 0, 0, 255) },
      { Sid::rhGuitarFingeringAlign,        "rhGuitarFingeringAlign",        QVariant::fromValue(Align::CENTER) },
      { Sid::rhGuitarFingeringFrameType,    "rhGuitarFingeringFrameType",    int(FrameType::NO_FRAME) },
      { Sid::rhGuitarFingeringFramePadding, "rhGuitarFingeringFramePadding", 0.2 },
      { Sid::rhGuitarFingeringFrameWidth,   "rhGuitarFingeringFrameWidth",   0.1 },
      { Sid::rhGuitarFingeringFrameRound,   "rhGuitarFingeringFrameRound",   0 },
      { Sid::rhGuitarFingeringFrameFgColor, "rhGuitarFingeringFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::rhGuitarFingeringFrameBgColor, "rhGuitarFingeringFrameBgColor", QColor(255, 255, 255, 0) },
      { Sid::rhGuitarFingeringOffset,       "rhGuitarFingeringOffset",       QPointF() },

      { Sid::stringNumberFontFace,          "stringNumberFontFace",          "Edwin" },
      { Sid::stringNumberFontSize,          "stringNumberFontSize",          8.0 },
      { Sid::stringNumberLineSpacing,       "stringNumberLineSpacing",       1.0 },
      { Sid::stringNumberFontSpatiumDependent, "stringNumberFontSpatiumDependent", true },
      { Sid::stringNumberFontStyle,         "stringNumberFontStyle",         int(FontStyle::Normal) },
      { Sid::stringNumberColor,             "stringNumberColor",             QColor(0, 0, 0, 255) },
      { Sid::stringNumberAlign,             "stringNumberAlign",             QVariant::fromValue(Align::CENTER) },
      { Sid::stringNumberFrameType,         "stringNumberFrameType",         int(FrameType::CIRCLE) },
      { Sid::stringNumberFramePadding,      "stringNumberFramePadding",      0.2 },
      { Sid::stringNumberFrameWidth,        "stringNumberFrameWidth",        0.1 },
      { Sid::stringNumberFrameRound,        "stringNumberFrameRound",        0 },
      { Sid::stringNumberFrameFgColor,      "stringNumberFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::stringNumberFrameBgColor,      "stringNumberFrameBgColor",      QColor(255, 255, 255, 0) },
      { Sid::stringNumberOffset,            "stringNumberOffset",            QPointF(0.0, 0.0) },

      { Sid::longInstrumentFontFace,        "longInstrumentFontFace",       "Edwin" },
      { Sid::longInstrumentFontSize,        "longInstrumentFontSize",       10.0 },
      { Sid::longInstrumentLineSpacing,     "longInstrumentLineSpacing",    1.0 },
      { Sid::longInstrumentFontSpatiumDependent, "longInstrumentFontSpatiumDependent", true },
      { Sid::longInstrumentFontStyle,       "longInstrumentFontStyle",      int(FontStyle::Normal) },
      { Sid::longInstrumentColor,           "longInstrumentColor",          QColor(0, 0, 0, 255) },
      { Sid::longInstrumentAlign,           "longInstrumentAlign",          QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::longInstrumentOffset,          "longInstrumentOffset",         QPointF(.0, .0) },
      { Sid::longInstrumentFrameType,       "longInstrumentFrameType",      int(FrameType::NO_FRAME) },
      { Sid::longInstrumentFramePadding,    "longInstrumentFramePadding",   0.2 },
      { Sid::longInstrumentFrameWidth,      "longInstrumentFrameWidth",     0.1 },
      { Sid::longInstrumentFrameRound,      "longInstrumentFrameRound",     0 },
      { Sid::longInstrumentFrameFgColor,    "longInstrumentFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::longInstrumentFrameBgColor,    "longInstrumentFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::shortInstrumentFontFace,       "shortInstrumentFontFace",      "Edwin" },
      { Sid::shortInstrumentFontSize,       "shortInstrumentFontSize",      10.0 },
      { Sid::shortInstrumentLineSpacing,    "shortInstrumentLineSpacing",   1.0 },
      { Sid::shortInstrumentFontSpatiumDependent, "shortInstrumentFontSpatiumDependent", true },
      { Sid::shortInstrumentFontStyle,      "shortInstrumentFontStyle",     int(FontStyle::Normal) },
      { Sid::shortInstrumentColor,          "shortInstrumentColor",         QColor(0, 0, 0, 255) },
      { Sid::shortInstrumentAlign,          "shortInstrumentAlign",         QVariant::fromValue(Align::RIGHT | Align::VCENTER) },
      { Sid::shortInstrumentOffset,         "shortInstrumentOffset",        QPointF(.0, .0) },
      { Sid::shortInstrumentFrameType,      "shortInstrumentFrameType",     int(FrameType::NO_FRAME) },
      { Sid::shortInstrumentFramePadding,   "shortInstrumentFramePadding",  0.2 },
      { Sid::shortInstrumentFrameWidth,     "shortInstrumentFrameWidth",    0.1 },
      { Sid::shortInstrumentFrameRound,     "shortInstrumentFrameRound",    0 },
      { Sid::shortInstrumentFrameFgColor,   "shortInstrumentFrameFgColor",  QColor(0, 0, 0, 255) },
      { Sid::shortInstrumentFrameBgColor,   "shortInstrumentFrameBgColor",  QColor(255, 255, 255, 0) },

      { Sid::partInstrumentFontFace,        "partInstrumentFontFace",       "Edwin" },
      { Sid::partInstrumentFontSize,        "partInstrumentFontSize",       14.0 },
      { Sid::partInstrumentLineSpacing,     "partInstrumentLineSpacing",    1.0 },
      { Sid::partInstrumentFontSpatiumDependent, "partInstrumentFontSpatiumDependent", false },
      { Sid::partInstrumentFontStyle,       "partInstrumentFontStyle",      int(FontStyle::Normal) },
      { Sid::partInstrumentColor,           "partInstrumentColor",          QColor(0, 0, 0, 255) },
      { Sid::partInstrumentAlign,           "partInstrumentAlign",          QVariant::fromValue(Align::LEFT) },
      { Sid::partInstrumentOffset,          "partInstrumentOffset",         QPointF() },
      { Sid::partInstrumentFrameType,       "partInstrumentFrameType",      int(FrameType::NO_FRAME) },
      { Sid::partInstrumentFramePadding,    "partInstrumentFramePadding",   0.2 },
      { Sid::partInstrumentFrameWidth,      "partInstrumentFrameWidth",     0.1 },
      { Sid::partInstrumentFrameRound,      "partInstrumentFrameRound",     0 },
      { Sid::partInstrumentFrameFgColor,    "partInstrumentFrameFgColor",   QColor(0, 0, 0, 255) },
      { Sid::partInstrumentFrameBgColor,    "partInstrumentFrameBgColor",   QColor(255, 255, 255, 0) },

      { Sid::dynamicsFontFace,              "dynamicsFontFace",             "Edwin" },
      { Sid::dynamicsFontSize,              "dynamicsFontSize",             11.0 },
      { Sid::dynamicsLineSpacing,           "dynamicsLineSpacing",          1.0 },
      { Sid::dynamicsFontSpatiumDependent,  "dynamicsFontSpatiumDependent", true },
      { Sid::dynamicsFontStyle,             "dynamicsFontStyle",            int(FontStyle::Italic) },
      { Sid::dynamicsColor,                 "dynamicsColor",                QColor(0, 0, 0, 255) },
      { Sid::dynamicsAlign,                 "dynamicsAlign",                QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::dynamicsFrameType,             "dynamicsFrameType",            int(FrameType::NO_FRAME) },
      { Sid::dynamicsFramePadding,          "dynamicsFramePadding",         0.2 },
      { Sid::dynamicsFrameWidth,            "dynamicsFrameWidth",           0.1 },
      { Sid::dynamicsFrameRound,            "dynamicsFrameRound",           0 },
      { Sid::dynamicsFrameFgColor,          "dynamicsFrameFgColor",         QColor(0, 0, 0, 255) },
      { Sid::dynamicsFrameBgColor,          "dynamicsFrameBgColor",         QColor(255, 255, 255, 0) },

      { Sid::expressionFontFace,            "expressionFontFace",           "Edwin" },
      { Sid::expressionFontSize,            "expressionFontSize",           10.0 },
      { Sid::expressionLineSpacing,         "expressionLineSpacing",        1.0 },
      { Sid::expressionFontSpatiumDependent, "expressionFontSpatiumDependent", true },
      { Sid::expressionFontStyle,           "expressionFontStyle",          int(FontStyle::Italic) },
      { Sid::expressionColor,               "expressionColor",              QColor(0, 0, 0, 255) },
      { Sid::expressionAlign,               "expressionAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::expressionPlacement,           "expressionPlacement",          int(Placement::BELOW)  },
      { Sid::expressionOffset,              "expressionOffset",             QPointF(.0, 2.5) },
      { Sid::expressionFrameType,           "expressionFrameType",          int(FrameType::NO_FRAME) },
      { Sid::expressionFramePadding,        "expressionFramePadding",       0.2 },
      { Sid::expressionFrameWidth,          "expressionFrameWidth",         0.1 },
      { Sid::expressionFrameRound,          "expressionFrameRound",         0 },
      { Sid::expressionFrameFgColor,        "expressionFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::expressionFrameBgColor,        "expressionFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::tempoFontFace,                 "tempoFontFace",                "Edwin" },
      { Sid::tempoFontSize,                 "tempoFontSize",                12.0 },
      { Sid::tempoLineSpacing,              "tempoLineSpacing",             1.0 },
      { Sid::tempoFontSpatiumDependent,     "tempoFontSpatiumDependent",    true },
      { Sid::tempoFontStyle,                "tempoFontStyle",               int(FontStyle::Bold) },
      { Sid::tempoColor,                    "tempoColor",                   QColor(0, 0, 0, 255) },
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

      { Sid::metronomeFontFace,             "metronomeFontFace",            "Edwin" },
      { Sid::metronomeFontSize,             "metronomeFontSize",            12.0 },
      { Sid::metronomeLineSpacing,          "metronomeLineSpacing",         1.0 },
      { Sid::metronomeFontSpatiumDependent, "metronomeFontSpatiumDependent", false },
      { Sid::metronomeFontStyle,            "metronomeFontStyle",           int(FontStyle::Normal) },
      { Sid::metronomeColor,                "metronomeColor",               QColor(0, 0, 0, 255) },
      { Sid::metronomePlacement,            "metronomePlacement",           int(Placement::ABOVE) },
      { Sid::metronomeAlign,                "metronomeAlign",               QVariant::fromValue(Align::LEFT) },
      { Sid::metronomeOffset,               "metronomeOffset",              QPointF() },
      { Sid::metronomeFrameType,            "metronomeFrameType",           int(FrameType::NO_FRAME) },
      { Sid::metronomeFramePadding,         "metronomeFramePadding",        0.2 },
      { Sid::metronomeFrameWidth,           "metronomeFrameWidth",          0.1 },
      { Sid::metronomeFrameRound,           "metronomeFrameRound",          0 },
      { Sid::metronomeFrameFgColor,         "metronomeFrameFgColor",        QColor(0, 0, 0, 255) },
      { Sid::metronomeFrameBgColor,         "metronomeFrameBgColor",        QColor(255, 255, 255, 0) },

      { Sid::measureNumberFontFace,         "measureNumberFontFace",        "Edwin" },
      { Sid::measureNumberFontSize,         "measureNumberFontSize",        8.0 },
      { Sid::measureNumberLineSpacing,      "measureNumberLineSpacing",     1.0 },
      { Sid::measureNumberFontSpatiumDependent, "measureNumberFontSpatiumDependent", false },
      { Sid::measureNumberFontStyle,        "measureNumberFontStyle",       2 },
      { Sid::measureNumberColor,            "measureNumberColor",           QColor(0, 0, 0, 255) },
      { Sid::measureNumberPosAbove,         "measureNumberOffset",          QPointF(0.0, -2.0) }, // This measureNumberOffset cannot be renamed to measureNumberPosAbove for backward compatibility
      { Sid::measureNumberPosBelow,         "measureNumberPosBelow",        QPointF(0.0, 2.0) },
      { Sid::measureNumberOffsetType,       "measureNumberOffsetType",      int(OffsetType::SPATIUM)   },
      { Sid::measureNumberVPlacement,       "measureNumberVPlacement",      int(Placement::ABOVE) },
      { Sid::measureNumberHPlacement,       "measureNumberHPlacement",      int(HPlacement::LEFT) },
      { Sid::measureNumberAlign,            "measureNumberAlign",           QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::measureNumberFrameType,        "measureNumberFrameType",       int(FrameType::NO_FRAME) },
      { Sid::measureNumberFramePadding,     "measureNumberFramePadding",    0.2 },
      { Sid::measureNumberFrameWidth,       "measureNumberFrameWidth",      0.1 },
      { Sid::measureNumberFrameRound,       "measureNumberFrameRound",      0 },
      { Sid::measureNumberFrameFgColor,     "measureNumberFrameFgColor",    QColor(0, 0, 0, 255) },
      { Sid::measureNumberFrameBgColor,     "measureNumberFrameBgColor",    QColor(255, 255, 255, 0) },

      { Sid::mmRestShowMeasureNumberRange,  "mmRestShowMeasureNumberRange", false },
      { Sid::mmRestRangeBracketType,        "mmRestRangeBracketType",       int(MMRestRangeBracketType::BRACKETS) },

      { Sid::mmRestRangeFontFace,           "mmRestRangeFontFace",          "Edwin" },
      { Sid::mmRestRangeFontSize,           "mmRestRangeFontSize",          8.0 },
      { Sid::mmRestRangeFontSpatiumDependent, "mmRestRangeFontSpatiumDependent", false },
      { Sid::mmRestRangeFontStyle,          "mmRestRangeFontStyle",         int(FontStyle::Italic)          },
      { Sid::mmRestRangeColor,              "mmRestRangeColor",             QColor(0, 0, 0, 255)            },
      { Sid::mmRestRangePosAbove,           "measureNumberPosAbove",        QPointF(0.0, -3.0)              },
      { Sid::mmRestRangePosBelow,           "measureNumberPosBelow",        QPointF(0.0, 1.0)               },
      { Sid::mmRestRangeOffsetType,         "mmRestRangeOffsetType",        int(OffsetType::SPATIUM)        },
      { Sid::mmRestRangeVPlacement,         "mmRestRangeVPlacement",        int(Placement::BELOW)           },
      { Sid::mmRestRangeHPlacement,         "mmRestRangeHPlacement",        int(HPlacement::CENTER)         },
      { Sid::mmRestRangeAlign,              "mmRestRangeAlign",             QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::mmRestRangeFrameType,          "mmRestRangeFrameType",         int(FrameType::NO_FRAME)        },
      { Sid::mmRestRangeFramePadding,       "mmRestRangeFramePadding",      0.2                             },
      { Sid::mmRestRangeFrameWidth,         "mmRestRangeFrameWidth",        0.1                             },
      { Sid::mmRestRangeFrameRound,         "mmRestRangeFrameRound",        0                               },
      { Sid::mmRestRangeFrameFgColor,       "mmRestRangeFrameFgColor",      QColor(0, 0, 0, 255)            },
      { Sid::mmRestRangeFrameBgColor,       "mmRestRangeFrameBgColor",      QColor(255, 255, 255, 0)        },

      { Sid::translatorFontFace,            "translatorFontFace",           "Edwin" },
      { Sid::translatorFontSize,            "translatorFontSize",           10.0 },
      { Sid::translatorLineSpacing,         "translatorLineSpacing",        1.0 },
      { Sid::translatorFontSpatiumDependent, "translatorFontSpatiumDependent", false },
      { Sid::translatorFontStyle,           "translatorFontStyle",          int(FontStyle::Normal) },
      { Sid::translatorColor,               "translatorColor",              QColor(0, 0, 0, 255) },
      { Sid::translatorAlign,               "translatorAlign",              QVariant::fromValue(Align::LEFT) },
      { Sid::translatorOffset,              "translatorOffset",             QPointF() },
      { Sid::translatorFrameType,           "translatorFrameType",          int(FrameType::NO_FRAME) },
      { Sid::translatorFramePadding,        "translatorFramePadding",       0.2 },
      { Sid::translatorFrameWidth,          "translatorFrameWidth",         0.1 },
      { Sid::translatorFrameRound,          "translatorFrameRound",         0 },
      { Sid::translatorFrameFgColor,        "translatorFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::translatorFrameBgColor,        "translatorFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::systemTextFontFace,            "systemFontFace",               "Edwin" },
      { Sid::systemTextFontSize,            "systemFontSize",               10.0 },
      { Sid::systemTextLineSpacing,         "systemTextLineSpacing",        1.0 },
      { Sid::systemTextFontSpatiumDependent, "systemFontSpatiumDependent",  true },
      { Sid::systemTextFontStyle,           "systemFontStyle",              int(FontStyle::Normal) },
      { Sid::systemTextColor,               "systemTextColor",              QColor(0, 0, 0, 255) },
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

      { Sid::staffTextFontFace,             "staffFontFace",                "Edwin" },
      { Sid::staffTextFontSize,             "staffFontSize",                10.0 },
      { Sid::staffTextLineSpacing,          "staffTextLineSpacing",         1.0 },
      { Sid::staffTextFontSpatiumDependent, "staffFontSpatiumDependent",    true },
      { Sid::staffTextFontStyle,            "staffFontStyle",               int(FontStyle::Normal) },
      { Sid::staffTextColor,                "staffTextColor",               QColor(0, 0, 0, 255) },
      { Sid::staffTextAlign,                "staffAlign",                   QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::staffTextOffsetType,           "systemOffsetType",             int(OffsetType::SPATIUM)   },
      { Sid::staffTextPlacement,            "staffPlacement",               int(Placement::ABOVE) },
      { Sid::staffTextPosAbove,             "staffPosAbove",                QPointF(.0, -1.0) },
      { Sid::staffTextPosBelow,             "staffPosBelow",                QPointF(.0, 2.5)  },
      { Sid::staffTextMinDistance,          "staffMinDistance",             Spatium(0.5)  },
      { Sid::staffTextFrameType,            "staffFrameType",               int(FrameType::NO_FRAME) },
      { Sid::staffTextFramePadding,         "staffFramePadding",            0.2 },
      { Sid::staffTextFrameWidth,           "staffFrameWidth",              0.1 },
      { Sid::staffTextFrameRound,           "staffFrameRound",              0  },
      { Sid::staffTextFrameFgColor,         "staffFrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::staffTextFrameBgColor,         "staffFrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::rehearsalMarkFontFace,         "rehearsalMarkFontFace",        "Edwin" },
      { Sid::rehearsalMarkFontSize,         "rehearsalMarkFontSize",        14.0 },
      { Sid::rehearsalMarkLineSpacing,      "rehearsalMarkLineSpacing",     1.0 },
      { Sid::rehearsalMarkFontSpatiumDependent, "rehearsalMarkFontSpatiumDependent", true },
      { Sid::rehearsalMarkFontStyle,        "rehearsalMarkFontStyle",       int(FontStyle::Bold) },
      { Sid::rehearsalMarkColor,            "rehearsalMarkColor",           QColor(0, 0, 0, 255) },
      { Sid::rehearsalMarkAlign,            "rehearsalMarkAlign",           QVariant::fromValue(Align::HCENTER | Align::BASELINE) },
      { Sid::rehearsalMarkFrameType,        "rehearsalMarkFrameType",       int(FrameType::SQUARE)  },
      { Sid::rehearsalMarkFramePadding,     "rehearsalMarkFramePadding",    0.5 },
      { Sid::rehearsalMarkFrameWidth,       "rehearsalMarkFrameWidth",      0.16 },
      { Sid::rehearsalMarkFrameRound,       "rehearsalMarkFrameRound",      0 },
      { Sid::rehearsalMarkFrameFgColor,     "rehearsalMarkFrameFgColor",    QColor(0, 0, 0, 255) },
      { Sid::rehearsalMarkFrameBgColor,     "rehearsalMarkFrameBgColor",    QColor(255, 255, 255, 0) },
      { Sid::rehearsalMarkPlacement,        "rehearsalMarkPlacement",       int(Placement::ABOVE) },
      { Sid::rehearsalMarkPosAbove,         "rehearsalMarkPosAbove",        QPointF(.0, -3.0) },
      { Sid::rehearsalMarkPosBelow,         "rehearsalMarkPosBelow",        QPointF(.0, 4.0) },
      { Sid::rehearsalMarkMinDistance,      "rehearsalMarkMinDistance",     Spatium(0.5) },

      { Sid::repeatLeftFontFace,            "repeatLeftFontFace",           "Edwin" },
      { Sid::repeatLeftFontSize,            "repeatLeftFontSize",           18.0 },
      { Sid::repeatLeftLineSpacing,         "repeatLeftLineSpacing",        1.0 },
      { Sid::repeatLeftFontSpatiumDependent, "repeatLeftFontSpatiumDependent", true },
      { Sid::repeatLeftFontStyle,           "repeatLeftFontStyle",          int(FontStyle::Normal) },
      { Sid::repeatLeftColor,               "repeatLeftColor",              QColor(0, 0, 0, 255) },
      { Sid::repeatLeftAlign,               "repeatLeftAlign",              QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::repeatLeftPlacement,           "repeatLeftPlacement",          int(Placement::ABOVE) },
      { Sid::repeatLeftFrameType,           "repeatLeftFrameType",          int(FrameType::NO_FRAME) },
      { Sid::repeatLeftFramePadding,        "repeatLeftFramePadding",       0.2 },
      { Sid::repeatLeftFrameWidth,          "repeatLeftFrameWidth",         0.1 },
      { Sid::repeatLeftFrameRound,          "repeatLeftFrameRound",         0  },
      { Sid::repeatLeftFrameFgColor,        "repeatLeftFrameFgColor",       QColor(0, 0, 0, 255) },
      { Sid::repeatLeftFrameBgColor,        "repeatLeftFrameBgColor",       QColor(255, 255, 255, 0) },

      { Sid::repeatRightFontFace,           "repeatRightFontFace",          "Edwin" },
      { Sid::repeatRightFontSize,           "repeatRightFontSize",          11.0 },
      { Sid::repeatRightLineSpacing,        "repeatRightLineSpacing",       1.0 },
      { Sid::repeatRightFontSpatiumDependent, "repeatRightFontSpatiumDependent", true },
      { Sid::repeatRightFontStyle,          "repeatRightFontStyle",         int(FontStyle::Normal) },
      { Sid::repeatRightColor,              "repeatRightColor",             QColor(0, 0, 0, 255) },
      { Sid::repeatRightAlign,              "repeatRightAlign",             QVariant::fromValue(Align::RIGHT | Align::BASELINE) },
      { Sid::repeatRightPlacement,          "repeatRightPlacement",         int(Placement::ABOVE) },
      { Sid::repeatRightFrameType,          "repeatRightFrameType",         int(FrameType::NO_FRAME) },
      { Sid::repeatRightFramePadding,       "repeatRightFramePadding",      0.2 },
      { Sid::repeatRightFrameWidth,         "repeatRightFrameWidth",        0.1 },
      { Sid::repeatRightFrameRound,         "repeatRightFrameRound",        0  },
      { Sid::repeatRightFrameFgColor,       "repeatRightFrameFgColor",      QColor(0, 0, 0, 255) },
      { Sid::repeatRightFrameBgColor,       "repeatRightFrameBgColor",      QColor(255, 255, 255, 0) },

      { Sid::frameFontFace,                 "frameFontFace",                "Edwin" },
      { Sid::frameFontSize,                 "frameFontSize",                10.0 },
      { Sid::frameLineSpacing,              "frameLineSpacing",             1.0 },
      { Sid::frameFontSpatiumDependent,     "frameFontSpatiumDependent",    false },
      { Sid::frameFontStyle,                "frameFontStyle",               int(FontStyle::Normal) },
      { Sid::frameColor,                    "frameColor",                   QColor(0, 0, 0, 255) },
      { Sid::frameAlign,                    "frameAlign",                   QVariant::fromValue(Align::LEFT) },
      { Sid::frameOffset,                   "frameOffset",                  QPointF() },
      { Sid::frameFrameType,                "frameFrameType",               int(FrameType::NO_FRAME) },
      { Sid::frameFramePadding,             "frameFramePadding",            0.2 },
      { Sid::frameFrameWidth,               "frameFrameWidth",              0.1 },
      { Sid::frameFrameRound,               "frameFrameRound",              0  },
      { Sid::frameFrameFgColor,             "frameFrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::frameFrameBgColor,             "frameFrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::textLineFontFace,              "textLineFontFace",             "Edwin" },
      { Sid::textLineFontSize,              "textLineFontSize",             10.0 },
      { Sid::textLineLineSpacing,           "textLineLineSpacing",          1.0 },
      { Sid::textLineFontSpatiumDependent,  "textLineFontSpatiumDependent", true },
      { Sid::textLineFontStyle,             "textLineFontStyle",            int(FontStyle::Normal) },
      { Sid::textLineColor,                 "textLineColor",                QColor(0, 0, 0, 255) },
      { Sid::textLineTextAlign,             "textLineTextAlign",            QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::textLineSystemFlag,            "textLineSystemFlag",           false },

      { Sid::systemTextLineFontFace,              "systemTextLineFontFace",             "Edwin" },
      { Sid::systemTextLineFontSize,              "systemTextLineFontSize",             12.0 },
      { Sid::systemTextLineFontSpatiumDependent,  "systemTextLineFontSpatiumDependent", true },
      { Sid::systemTextLineFontStyle,             "systemTextLineFontStyle",            int(FontStyle::Normal) },
      { Sid::systemTextLineColor,                 "systemTextLineColor",                QColor(0, 0, 0, 255) },
      { Sid::systemTextLineTextAlign,             "systemTextLineTextAlign",            QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::systemTextLineSystemFlag,            "systemTextLineSystemFlag",           true },

      { Sid::glissandoFontFace,             "glissandoFontFace",            "Edwin" },
      { Sid::glissandoFontSize,             "glissandoFontSize",            QVariant(8.0) },
      { Sid::glissandoLineSpacing,         "glissandoLineSpacing",          1.0 },
      { Sid::glissandoFontSpatiumDependent, "glissandoFontSpatiumDependent", true },
      { Sid::glissandoFontStyle,            "glissandoFontStyle",           int(FontStyle::Italic) },
      { Sid::glissandoColor,                "glissandoColor",               QColor(0, 0, 0, 255) },
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

      { Sid::bendFontFace,                  "bendFontFace",                 "Edwin" },
      { Sid::bendFontSize,                  "bendFontSize",                 8.0 },
      { Sid::bendLineSpacing,               "bendLineSpacing",              1.0 },
      { Sid::bendFontSpatiumDependent,      "bendFontSpatiumDependent",     true },
      { Sid::bendFontStyle,                 "bendFontStyle",                int(FontStyle::Normal) },
      { Sid::bendColor,                     "bendColor",                    QColor(0, 0, 0, 255) },
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

      { Sid::headerFontFace,                "headerFontFace",               "Edwin" },
      { Sid::headerFontSize,                "headerFontSize",               11.0 },
      { Sid::headerLineSpacing,             "headerLineSpacing",            1.0 },
      { Sid::headerFontSpatiumDependent,    "headerFontSpatiumDependent",   false },
      { Sid::headerFontStyle,               "headerFontStyle",              1 },
      { Sid::headerColor,                   "headerColor",                  QColor(0, 0, 0, 255) },
      { Sid::headerAlign,                   "headerAlign",                  QVariant::fromValue(Align::CENTER | Align::TOP) },
      { Sid::headerOffset,                  "headerOffset",                 QPointF() },
      { Sid::headerFrameType,               "headerFrameType",              int(FrameType::NO_FRAME) },
      { Sid::headerFramePadding,            "headerFramePadding",           0.2 },
      { Sid::headerFrameWidth,              "headerFrameWidth",             0.1 },
      { Sid::headerFrameRound,              "headerFrameRound",             0  },
      { Sid::headerFrameFgColor,            "headerFrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::headerFrameBgColor,            "headerFrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::footerFontFace,                "footerFontFace",               "Edwin" },
      { Sid::footerFontSize,                "footerFontSize",               9.0 },
      { Sid::footerLineSpacing,             "footerLineSpacing",            1.0 },
      { Sid::footerFontSpatiumDependent,    "footerFontSpatiumDependent",   false },
      { Sid::footerFontStyle,               "footerFontStyle",              int(FontStyle::Normal) },
      { Sid::footerColor,                   "footerColor",                  QColor(0, 0, 0, 255) },
      { Sid::footerAlign,                   "footerAlign",                  QVariant::fromValue(Align::CENTER | Align::BOTTOM) },
      { Sid::footerOffset,                  "footerOffset",                 QPointF(0.0, 0.0) },
      { Sid::footerFrameType,               "footerFrameType",              int(FrameType::NO_FRAME) },
      { Sid::footerFramePadding,            "footerFramePadding",           0.2 },
      { Sid::footerFrameWidth,              "footerFrameWidth",             0.1 },
      { Sid::footerFrameRound,              "footerFrameRound",             0  },
      { Sid::footerFrameFgColor,            "footerFrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::footerFrameBgColor,            "footerFrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::instrumentChangeFontFace,      "instrumentChangeFontFace",     "Edwin" },
      { Sid::instrumentChangeFontSize,      "instrumentChangeFontSize",     10.0 },
      { Sid::instrumentChangeLineSpacing,   "instrumentChangeLineSpacing",  1.0 },
      { Sid::instrumentChangeFontSpatiumDependent, "instrumentChangeFontSpatiumDependent", true },
      { Sid::instrumentChangeFontStyle,     "instrumentChangeFontStyle",    int(FontStyle::Bold) },
      { Sid::instrumentChangeColor,         "instrumentChangeColor",        QColor(0, 0, 0, 255) },
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

      { Sid::stickingFontFace,              "stickingFontFace",     "Edwin" },
      { Sid::stickingFontSize,              "stickingFontSize",     10.0 },
      { Sid::stickingLineSpacing,           "stickingLineSpacing",  1.0 },
      { Sid::stickingFontSpatiumDependent,  "stickingFontSpatiumDependent", true },
      { Sid::stickingFontStyle,             "stickingFontStyle",    int(FontStyle::Normal) },
      { Sid::stickingColor,                 "stickingColor",        QColor(0, 0, 0, 255) },
      { Sid::stickingAlign,                 "stickingAlign",        QVariant::fromValue(Align::LEFT | Align::BASELINE) },
      { Sid::stickingOffset,                "stickingOffset",       QPointF() },
      { Sid::stickingPlacement,             "stickingPlacement",    int(Placement::BELOW)  },
      { Sid::stickingPosAbove,              "stickingPosAbove",     QPointF(.0, -2.0) },
      { Sid::stickingPosBelow,              "stickingPosBelow",     QPointF(.0, 2.0)  },
      { Sid::stickingMinDistance,           "stickingMinDistance",  Spatium(0.5)  },
      { Sid::stickingFrameType,             "stickingFrameType",    int(FrameType::NO_FRAME) },
      { Sid::stickingFramePadding,          "stickingFramePadding", 0.2 },
      { Sid::stickingFrameWidth,            "stickingFrameWidth",   0.1 },
      { Sid::stickingFrameRound,            "stickingFrameRound",   0 },
      { Sid::stickingFrameFgColor,          "stickingFrameFgColor", QColor(0, 0, 0, 255) },
      { Sid::stickingFrameBgColor,          "stickingFrameBgColor", QColor(255, 255, 255, 0) },

      { Sid::figuredBassFontFace,           "figuredBassFontFace",          "MScoreBC" },
      { Sid::figuredBassFontSize,           "figuredBassFontSize",          8.0 },
      { Sid::figuredBassLineSpacing,        "figuredBassLineSpacing",       1.0 },
      { Sid::figuredBassFontSpatiumDependent, "figuredBassFontSpatiumDependent", true },
      { Sid::figuredBassFontStyle,          "figuredBassFontStyle",         int(FontStyle::Normal) },
      { Sid::figuredBassColor,              "figuredBassColor",             QColor(0, 0, 0, 255) },

      { Sid::user1Name,                     "user1Name",                    "" },
      { Sid::user1FontFace,                 "user1FontFace",                "Edwin" },
      { Sid::user1FontSize,                 "user1FontSize",                10.0 },
      { Sid::user1LineSpacing,              "user1LineSpacing",             1.0 },
      { Sid::user1FontSpatiumDependent,     "user1FontSpatiumDependent",    true },
      { Sid::user1FontStyle,                "user1FontStyle",               int(FontStyle::Normal) },
      { Sid::user1Color,                    "user1Color",                   QColor(0, 0, 0, 255) },
      { Sid::user1Align,                    "user1Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user1Offset,                   "user1Offset",                  QPointF() },
      { Sid::user1OffsetType,               "user1OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user1FrameType,                "user1FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user1FramePadding,             "user1FramePadding",            0.2 },
      { Sid::user1FrameWidth,               "user1FrameWidth",              0.1 },
      { Sid::user1FrameRound,               "user1FrameRound",              0 },
      { Sid::user1FrameFgColor,             "user1FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user1FrameBgColor,             "user1FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user2Name,                     "user2Name",                    "" },
      { Sid::user2FontFace,                 "user2FontFace",                "Edwin" },
      { Sid::user2FontSize,                 "user2FontSize",                10.0 },
      { Sid::user2LineSpacing,              "user2LineSpacing",             1.0 },
      { Sid::user2FontSpatiumDependent,     "user2FontSpatiumDependent",    true },
      { Sid::user2FontStyle,                "user2FontStyle",               int(FontStyle::Normal) },
      { Sid::user2Color,                    "user2Color",                   QColor(0, 0, 0, 255) },
      { Sid::user2Align,                    "user2Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user2Offset,                   "user2Offset",                  QPointF() },
      { Sid::user2OffsetType,               "user2OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user2FrameType,                "user2FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user2FramePadding,             "user2FramePadding",            0.2 },
      { Sid::user2FrameWidth,               "user2FrameWidth",              0.1 },
      { Sid::user2FrameRound,               "user2FrameRound",              0 },
      { Sid::user2FrameFgColor,             "user2FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user2FrameBgColor,             "user2FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user3Name,                     "user3Name",                    "" },
      { Sid::user3FontFace,                 "user3FontFace",                "Edwin" },
      { Sid::user3FontSize,                 "user3FontSize",                10.0 },
      { Sid::user3LineSpacing,              "user3LineSpacing",             1.0 },
      { Sid::user3FontSpatiumDependent,     "user3FontSpatiumDependent",    true },
      { Sid::user3FontStyle,                "user3FontStyle",               int(FontStyle::Normal) },
      { Sid::user3Color,                    "user3Color",                   QColor(0, 0, 0, 255) },
      { Sid::user3Align,                    "user3Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user3Offset,                   "user3Offset",                  QPointF() },
      { Sid::user3OffsetType,               "user3OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user3FrameType,                "user3FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user3FramePadding,             "user3FramePadding",            0.2 },
      { Sid::user3FrameWidth,               "user3FrameWidth",              0.1 },
      { Sid::user3FrameRound,               "user3FrameRound",              0 },
      { Sid::user3FrameFgColor,             "user3FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user3FrameBgColor,             "user3FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user4Name,                     "user4Name",                    "" },
      { Sid::user4FontFace,                 "user4FontFace",                "Edwin" },
      { Sid::user4FontSize,                 "user4FontSize",                10.0 },
      { Sid::user4LineSpacing,              "user4LineSpacing",             1.0 },
      { Sid::user4FontSpatiumDependent,     "user4FontSpatiumDependent",    true },
      { Sid::user4FontStyle,                "user4FontStyle",               int(FontStyle::Normal) },
      { Sid::user4Color,                    "user4Color",                   QColor(0, 0, 0, 255) },
      { Sid::user4Align,                    "user4Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user4Offset,                   "user4Offset",                  QPointF() },
      { Sid::user4OffsetType,               "user4OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user4FrameType,                "user4FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user4FramePadding,             "user4FramePadding",            0.2 },
      { Sid::user4FrameWidth,               "user4FrameWidth",              0.1 },
      { Sid::user4FrameRound,               "user4FrameRound",              0 },
      { Sid::user4FrameFgColor,             "user4FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user4FrameBgColor,             "user4FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user5Name,                     "user5Name",                    "" },
      { Sid::user5FontFace,                 "user5FontFace",                "Edwin" },
      { Sid::user5FontSize,                 "user5FontSize",                10.0 },
      { Sid::user5LineSpacing,              "user5LineSpacing",             1.0 },
      { Sid::user5FontSpatiumDependent,     "user5FontSpatiumDependent",    true },
      { Sid::user5FontStyle,                "user5FontStyle",               int(FontStyle::Normal) },
      { Sid::user5Color,                    "user5Color",                   QColor(0, 0, 0, 255) },
      { Sid::user5Align,                    "user5Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user5Offset,                   "user5Offset",                  QPointF() },
      { Sid::user5OffsetType,               "user5OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user5FrameType,                "user5FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user5FramePadding,             "user5FramePadding",            0.2 },
      { Sid::user5FrameWidth,               "user5FrameWidth",              0.1 },
      { Sid::user5FrameRound,               "user5FrameRound",              0 },
      { Sid::user5FrameFgColor,             "user5FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user5FrameBgColor,             "user5FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user6Name,                     "user6Name",                    "" },
      { Sid::user6FontFace,                 "user6FontFace",                "Edwin" },
      { Sid::user6FontSize,                 "user6FontSize",                10.0 },
      { Sid::user6LineSpacing,              "user6LineSpacing",             1.0 },
      { Sid::user6FontSpatiumDependent,     "user6FontSpatiumDependent",    true },
      { Sid::user6FontStyle,                "user6FontStyle",               int(FontStyle::Normal) },
      { Sid::user6Color,                    "user6Color",                   QColor(0, 0, 0, 255) },
      { Sid::user6Align,                    "user6Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user6Offset,                   "user6Offset",                  QPointF() },
      { Sid::user6OffsetType,               "user6OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user6FrameType,                "user6FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user6FramePadding,             "user6FramePadding",            0.2 },
      { Sid::user6FrameWidth,               "user6FrameWidth",              0.1 },
      { Sid::user6FrameRound,               "user6FrameRound",              0 },
      { Sid::user6FrameFgColor,             "user6FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user6FrameBgColor,             "user6FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user7Name,                     "user7Name",                    "" },
      { Sid::user7FontFace,                 "user7FontFace",                "Edwin" },
      { Sid::user7FontSize,                 "user7FontSize",                10.0 },
      { Sid::user7LineSpacing,              "user7LineSpacing",             1.0 },
      { Sid::user7FontSpatiumDependent,     "user7FontSpatiumDependent",    true },
      { Sid::user7FontStyle,                "user7FontStyle",               int(FontStyle::Normal) },
      { Sid::user7Color,                    "user7Color",                   QColor(0, 0, 0, 255) },
      { Sid::user7Align,                    "user7Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user7Offset,                   "user7Offset",                  QPointF() },
      { Sid::user7OffsetType,               "user7OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user7FrameType,                "user7FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user7FramePadding,             "user7FramePadding",            0.2 },
      { Sid::user7FrameWidth,               "user7FrameWidth",              0.1 },
      { Sid::user7FrameRound,               "user7FrameRound",              0 },
      { Sid::user7FrameFgColor,             "user7FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user7FrameBgColor,             "user7FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user8Name,                     "user8Name",                    "" },
      { Sid::user8FontFace,                 "user8FontFace",                "Edwin" },
      { Sid::user8FontSize,                 "user8FontSize",                10.0 },
      { Sid::user8LineSpacing,              "user8LineSpacing",             1.0 },
      { Sid::user8FontSpatiumDependent,     "user8FontSpatiumDependent",    true },
      { Sid::user8FontStyle,                "user8FontStyle",               int(FontStyle::Normal) },
      { Sid::user8Color,                    "user8Color",                   QColor(0, 0, 0, 255) },
      { Sid::user8Align,                    "user8Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user8Offset,                   "user8Offset",                  QPointF() },
      { Sid::user8OffsetType,               "user8OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user8FrameType,                "user8FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user8FramePadding,             "user8FramePadding",            0.2 },
      { Sid::user8FrameWidth,               "user8FrameWidth",              0.1 },
      { Sid::user8FrameRound,               "user8FrameRound",              0 },
      { Sid::user8FrameFgColor,             "user8FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user8FrameBgColor,             "user8FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user9Name,                     "user9Name",                    "" },
      { Sid::user9FontFace,                 "user9FontFace",                "Edwin" },
      { Sid::user9FontSize,                 "user9FontSize",                10.0 },
      { Sid::user9LineSpacing,              "user9LineSpacing",             1.0 },
      { Sid::user9FontSpatiumDependent,     "user9FontSpatiumDependent",    true },
      { Sid::user9FontStyle,                "user9FontStyle",               int(FontStyle::Normal) },
      { Sid::user9Color,                    "user9Color",                   QColor(0, 0, 0, 255) },
      { Sid::user9Align,                    "user9Align",                   QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user9Offset,                   "user9Offset",                  QPointF() },
      { Sid::user9OffsetType,               "user9OffsetType",              int(OffsetType::SPATIUM)    },
      { Sid::user9FrameType,                "user9FrameType",               int(FrameType::NO_FRAME) },
      { Sid::user9FramePadding,             "user9FramePadding",            0.2 },
      { Sid::user9FrameWidth,               "user9FrameWidth",              0.1 },
      { Sid::user9FrameRound,               "user9FrameRound",              0 },
      { Sid::user9FrameFgColor,             "user9FrameFgColor",            QColor(0, 0, 0, 255) },
      { Sid::user9FrameBgColor,             "user9FrameBgColor",            QColor(255, 255, 255, 0) },

      { Sid::user10Name,                    "user10Name",                   "" },
      { Sid::user10FontFace,                "user10FontFace",               "Edwin" },
      { Sid::user10FontSize,                "user10FontSize",               10.0 },
      { Sid::user10LineSpacing,             "user10LineSpacing",            1.0 },
      { Sid::user10FontSpatiumDependent,    "user10FontSpatiumDependent",   true },
      { Sid::user10FontStyle,               "user10FontStyle",              int(FontStyle::Normal) },
      { Sid::user10Color,                   "user10Color",                  QColor(0, 0, 0, 255) },
      { Sid::user10Align,                   "user10Align",                  QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user10Offset,                  "user10Offset",                 QPointF() },
      { Sid::user10OffsetType,              "user10OffsetType",             int(OffsetType::SPATIUM)    },
      { Sid::user10FrameType,               "user10FrameType",              int(FrameType::NO_FRAME) },
      { Sid::user10FramePadding,            "user10FramePadding",           0.2 },
      { Sid::user10FrameWidth,              "user10FrameWidth",             0.1 },
      { Sid::user10FrameRound,              "user10FrameRound",             0 },
      { Sid::user10FrameFgColor,            "user10FrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::user10FrameBgColor,            "user10FrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::user11Name,                    "user11Name",                   "" },
      { Sid::user11FontFace,                "user11FontFace",               "Edwin" },
      { Sid::user11FontSize,                "user11FontSize",               10.0 },
      { Sid::user11LineSpacing,             "user11LineSpacing",            1.0 },
      { Sid::user11FontSpatiumDependent,    "user11FontSpatiumDependent",   true },
      { Sid::user11FontStyle,               "user11FontStyle",              int(FontStyle::Normal) },
      { Sid::user11Color,                   "user11Color",                  QColor(0, 0, 0, 255) },
      { Sid::user11Align,                   "user11Align",                  QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user11Offset,                  "user11Offset",                 QPointF() },
      { Sid::user11OffsetType,              "user11OffsetType",             int(OffsetType::SPATIUM)    },
      { Sid::user11FrameType,               "user11FrameType",              int(FrameType::NO_FRAME) },
      { Sid::user11FramePadding,            "user11FramePadding",           0.2 },
      { Sid::user11FrameWidth,              "user11FrameWidth",             0.1 },
      { Sid::user11FrameRound,              "user11FrameRound",             0 },
      { Sid::user11FrameFgColor,            "user11FrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::user11FrameBgColor,            "user11FrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::user12Name,                    "user12Name",                   "" },
      { Sid::user12FontFace,                "user12FontFace",               "Edwin" },
      { Sid::user12FontSize,                "user12FontSize",               10.0 },
      { Sid::user12LineSpacing,             "user12LineSpacing",            1.0 },
      { Sid::user12FontSpatiumDependent,    "user12FontSpatiumDependent",   true },
      { Sid::user12FontStyle,               "user12FontStyle",              int(FontStyle::Normal) },
      { Sid::user12Color,                   "user12Color",                  QColor(0, 0, 0, 255) },
      { Sid::user12Align,                   "user12Align",                  QVariant::fromValue(Align::LEFT | Align::TOP) },
      { Sid::user12Offset,                  "user12Offset",                 QPointF() },
      { Sid::user12OffsetType,              "user12OffsetType",             int(OffsetType::SPATIUM)    },
      { Sid::user12FrameType,               "user12FrameType",              int(FrameType::NO_FRAME) },
      { Sid::user12FramePadding,            "user12FramePadding",           0.2 },
      { Sid::user12FrameWidth,              "user12FrameWidth",             0.1 },
      { Sid::user12FrameRound,              "user12FrameRound",             0 },
      { Sid::user12FrameFgColor,            "user12FrameFgColor",           QColor(0, 0, 0, 255) },
      { Sid::user12FrameBgColor,            "user12FrameBgColor",           QColor(255, 255, 255, 0) },

      { Sid::letRingFontFace,               "letRingFontFace",              "Edwin" },
      { Sid::letRingFontSize,               "letRingFontSize",              10.0 },
      { Sid::letRingLineSpacing,            "letRingLineSpacing",           1.0 },
      { Sid::letRingFontSpatiumDependent,   "letRingFontSpatiumDependent",  true },
      { Sid::letRingFontStyle,              "letRingFontStyle",             int(FontStyle::Normal) },
      { Sid::letRingColor,                  "letRingColor",                 QColor(0, 0, 0, 255) },
      { Sid::letRingTextAlign,              "letRingTextAlign",             QVariant::fromValue(Align::LEFT | Align::VCENTER) },
      { Sid::letRingHookHeight,             "letRingHookHeight",            Spatium(0.6) },
      { Sid::letRingPlacement,              "letRingPlacement",             int(Placement::BELOW)  },
      { Sid::letRingPosAbove,               "letRingPosAbove",              QPointF(.0, 0.0) },
      { Sid::letRingPosBelow,               "letRingPosBelow",              QPointF(.0, 0.0)  },
      { Sid::letRingLineWidth,              "letRingLineWidth",             Spatium(0.15) },
      { Sid::letRingLineStyle,              "letRingLineStyle",             QVariant(int(Qt::DashLine)) },
      { Sid::letRingBeginTextOffset,        "letRingBeginTextOffset",       QPointF(0.0, 0.15) },
      { Sid::letRingText,                   "letRingText",                  "let ring" },
      { Sid::letRingFrameType,              "letRingFrameType",             int(FrameType::NO_FRAME) },
      { Sid::letRingFramePadding,           "letRingFramePadding",          0.2 },
      { Sid::letRingFrameWidth,             "letRingFrameWidth",            0.1 },
      { Sid::letRingFrameRound,             "letRingFrameRound",            0 },
      { Sid::letRingFrameFgColor,           "letRingFrameFgColor",          QColor(0, 0, 0, 255) },
      { Sid::letRingFrameBgColor,           "letRingFrameBgColor",          QColor(255, 255, 255, 0) },
      { Sid::letRingEndHookType,            "letRingEndHookType",           int(HookType::HOOK_90T) },

      { Sid::palmMuteFontFace,              "palmMuteFontFace",              "Edwin" },
      { Sid::palmMuteFontSize,              "palmMuteFontSize",              10.0 },
      { Sid::palmMuteLineSpacing,           "palmMuteLineSpacing",           1.0 },
      { Sid::palmMuteFontSpatiumDependent,  "palmMuteFontSpatiumDependent",  true },
      { Sid::palmMuteFontStyle,             "palmMuteFontStyle",             int(FontStyle::Normal) },
      { Sid::palmMuteColor,                 "palmMuteColor",                 QColor(0, 0, 0, 255) },
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
      { Sid::palmMuteEndHookType,           "palmMuteEndHookType",           int(HookType::HOOK_90T) },

      { Sid::fermataPosAbove,               "fermataPosAbove",               QPointF(.0, -0.5) },
      { Sid::fermataPosBelow,               "fermataPosBelow",               QPointF(.0, 0.5)  },
      { Sid::fermataMinDistance,            "fermataMinDistance",            Spatium(0.4)  },

      { Sid::fingeringPlacement,            "fingeringPlacement",            int(Placement::ABOVE) },

      { Sid::articulationMinDistance,       "articulationMinDistance",       Spatium(0.5)  },
      { Sid::fingeringMinDistance,          "fingeringMinDistance",          Spatium(0.5)  },
      { Sid::hairpinMinDistance,            "hairpinMinDistance",            Spatium(0.7)  },
      { Sid::letRingMinDistance,            "letRingMinDistance",            Spatium(0.7)  },
      { Sid::ottavaMinDistance,             "ottavaMinDistance",             Spatium(0.7)  },
      { Sid::palmMuteMinDistance,           "palmMuteMinDistance",           Spatium(0.7)  },
      { Sid::pedalMinDistance,              "pedalMinDistance",              Spatium(0.7)  },
      { Sid::repeatMinDistance,             "repeatMinDistance",             Spatium(0.5)  },
      { Sid::textLineMinDistance,           "textLineMinDistance",           Spatium(0.7)  },
      { Sid::systemTextLineMinDistance,     "systemTextLineMinDistance",     Spatium(0.7)  },
      { Sid::trillMinDistance,              "trillMinDistance",              Spatium(0.5)  },
      { Sid::vibratoMinDistance,            "vibratoMinDistance",            Spatium(1.0)  },
      { Sid::voltaMinDistance,              "voltaMinDistance",              Spatium(1.0)  },
      { Sid::figuredBassMinDistance,        "figuredBassMinDistance",        Spatium(0.5)  },
      { Sid::tupletMinDistance,             "tupletMinDistance",             Spatium(0.5)  },

      { Sid::autoplaceEnabled,              "autoplaceEnabled",              true },
      { Sid::usePre_3_6_defaults,           "usePre_3_6_defaults",           false},
      { Sid::defaultsVersion,               "defaultsVersion",               Ms::MSCVERSION}
      };

MStyle  MScore::_baseStyle;
MStyle  MScore::_defaultStyle;

//---------------------------------------------------------
//   text styles
//---------------------------------------------------------

const TextStyle defaultTextStyle {{
      { Sid::defaultFontFace,                    Pid::FONT_FACE              },
      { Sid::defaultFontSize,                    Pid::FONT_SIZE              },
      { Sid::defaultLineSpacing,                 Pid::TEXT_LINE_SPACING      },
      { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::defaultFontStyle,                   Pid::FONT_STYLE             },
      { Sid::defaultColor,                       Pid::COLOR                  },
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
      { Sid::titleLineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::titleFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::titleFontStyle,                     Pid::FONT_STYLE             },
      { Sid::titleColor,                         Pid::COLOR                  },
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
      { Sid::subTitleLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::subTitleFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::subTitleFontStyle,                  Pid::FONT_STYLE             },
      { Sid::subTitleColor,                      Pid::COLOR                  },
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
      { Sid::composerLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::composerFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::composerFontStyle,                  Pid::FONT_STYLE             },
      { Sid::composerColor,                      Pid::COLOR                  },
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
      { Sid::lyricistLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::lyricistFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lyricistFontStyle,                  Pid::FONT_STYLE             },
      { Sid::lyricistColor,                      Pid::COLOR                  },
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
      { Sid::lyricsEvenLineSpacing,              Pid::TEXT_LINE_SPACING      },
      { Sid::lyricsEvenFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lyricsEvenFontStyle,                Pid::FONT_STYLE             },
      { Sid::lyricsEvenColor,                    Pid::COLOR                  },
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
      { Sid::lyricsOddLineSpacing,               Pid::TEXT_LINE_SPACING      },
      { Sid::lyricsOddFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lyricsOddFontStyle,                 Pid::FONT_STYLE             },
      { Sid::lyricsOddColor,                     Pid::COLOR                  },
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
      { Sid::fingeringLineSpacing,               Pid::TEXT_LINE_SPACING      },
      { Sid::fingeringFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::fingeringFontStyle,                 Pid::FONT_STYLE             },
      { Sid::fingeringColor,                     Pid::COLOR                  },
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
      { Sid::lhGuitarFingeringFontFace,             Pid::FONT_FACE              },
      { Sid::lhGuitarFingeringFontSize,             Pid::FONT_SIZE              },
      { Sid::lhGuitarFingeringLineSpacing,          Pid::TEXT_LINE_SPACING      },
      { Sid::lhGuitarFingeringFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::lhGuitarFingeringFontStyle,            Pid::FONT_STYLE             },
      { Sid::lhGuitarFingeringColor,                Pid::COLOR                  },
      { Sid::lhGuitarFingeringAlign,                Pid::ALIGN                  },
      { Sid::lhGuitarFingeringOffset,               Pid::OFFSET                 },
      { Sid::lhGuitarFingeringFrameType,            Pid::FRAME_TYPE             },
      { Sid::lhGuitarFingeringFramePadding,         Pid::FRAME_PADDING          },
      { Sid::lhGuitarFingeringFrameWidth,           Pid::FRAME_WIDTH            },
      { Sid::lhGuitarFingeringFrameRound,           Pid::FRAME_ROUND            },
      { Sid::lhGuitarFingeringFrameFgColor,         Pid::FRAME_FG_COLOR         },
      { Sid::lhGuitarFingeringFrameBgColor,         Pid::FRAME_BG_COLOR         },
      }};

const TextStyle rhGuitarFingeringTextStyle {{
      { Sid::rhGuitarFingeringFontFace,             Pid::FONT_FACE              },
      { Sid::rhGuitarFingeringFontSize,             Pid::FONT_SIZE              },
      { Sid::rhGuitarFingeringLineSpacing,          Pid::TEXT_LINE_SPACING      },
      { Sid::rhGuitarFingeringFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::rhGuitarFingeringFontStyle,            Pid::FONT_STYLE             },
      { Sid::rhGuitarFingeringColor,                Pid::COLOR                  },
      { Sid::rhGuitarFingeringAlign,                Pid::ALIGN                  },
      { Sid::rhGuitarFingeringOffset,               Pid::OFFSET                 },
      { Sid::rhGuitarFingeringFrameType,            Pid::FRAME_TYPE             },
      { Sid::rhGuitarFingeringFramePadding,         Pid::FRAME_PADDING          },
      { Sid::rhGuitarFingeringFrameWidth,           Pid::FRAME_WIDTH            },
      { Sid::rhGuitarFingeringFrameRound,           Pid::FRAME_ROUND            },
      { Sid::rhGuitarFingeringFrameFgColor,         Pid::FRAME_FG_COLOR         },
      { Sid::rhGuitarFingeringFrameBgColor,         Pid::FRAME_BG_COLOR         },
      }};

const TextStyle stringNumberTextStyle {{
      { Sid::stringNumberFontFace,               Pid::FONT_FACE              },
      { Sid::stringNumberFontSize,               Pid::FONT_SIZE              },
      { Sid::stringNumberLineSpacing,            Pid::TEXT_LINE_SPACING      },
      { Sid::stringNumberFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::stringNumberFontStyle,              Pid::FONT_STYLE             },
      { Sid::stringNumberColor,                  Pid::COLOR                  },
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
      { Sid::longInstrumentLineSpacing,          Pid::TEXT_LINE_SPACING      },
      { Sid::longInstrumentFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::longInstrumentFontStyle,            Pid::FONT_STYLE             },
      { Sid::longInstrumentColor,                Pid::COLOR                  },
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
      { Sid::shortInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::shortInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::shortInstrumentLineSpacing,          Pid::TEXT_LINE_SPACING      },
      { Sid::shortInstrumentFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::shortInstrumentFontStyle,            Pid::FONT_STYLE             },
      { Sid::shortInstrumentColor,                Pid::COLOR                  },
      { Sid::shortInstrumentAlign,                Pid::ALIGN                  },
      { Sid::shortInstrumentOffset,               Pid::OFFSET                 },
      { Sid::shortInstrumentFrameType,            Pid::FRAME_TYPE             },
      { Sid::shortInstrumentFramePadding,         Pid::FRAME_PADDING          },
      { Sid::shortInstrumentFrameWidth,           Pid::FRAME_WIDTH            },
      { Sid::shortInstrumentFrameRound,           Pid::FRAME_ROUND            },
      { Sid::shortInstrumentFrameFgColor,         Pid::FRAME_FG_COLOR         },
      { Sid::shortInstrumentFrameBgColor,         Pid::FRAME_BG_COLOR         },
      }};

const TextStyle partInstrumentTextStyle {{
      { Sid::partInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::partInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::partInstrumentLineSpacing,          Pid::TEXT_LINE_SPACING      },
      { Sid::partInstrumentFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::partInstrumentFontStyle,            Pid::FONT_STYLE             },
      { Sid::partInstrumentColor,                Pid::COLOR                  },
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
      { Sid::dynamicsLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::dynamicsFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::dynamicsFontStyle,                  Pid::FONT_STYLE             },
      { Sid::dynamicsColor,                      Pid::COLOR                  },
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
      { Sid::expressionLineSpacing,              Pid::TEXT_LINE_SPACING      },
      { Sid::expressionFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::expressionFontStyle,                Pid::FONT_STYLE             },
      { Sid::expressionColor,                    Pid::COLOR                  },
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
      { Sid::tempoLineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::tempoFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::tempoFontStyle,                     Pid::FONT_STYLE             },
      { Sid::tempoColor,                         Pid::COLOR                  },
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
      { Sid::metronomeLineSpacing,               Pid::TEXT_LINE_SPACING      },
      { Sid::metronomeFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::metronomeFontStyle,                 Pid::FONT_STYLE             },
      { Sid::metronomeColor,                     Pid::COLOR                  },
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
      { Sid::measureNumberLineSpacing,           Pid::TEXT_LINE_SPACING      },
      { Sid::measureNumberFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::measureNumberFontStyle,             Pid::FONT_STYLE             },
      { Sid::measureNumberColor,                 Pid::COLOR                  },
      { Sid::measureNumberAlign,                 Pid::ALIGN                  },
      { Sid::measureNumberPosAbove,              Pid::OFFSET                 },
      { Sid::measureNumberFrameType,             Pid::FRAME_TYPE             },
      { Sid::measureNumberFramePadding,          Pid::FRAME_PADDING          },
      { Sid::measureNumberFrameWidth,            Pid::FRAME_WIDTH            },
      { Sid::measureNumberFrameRound,            Pid::FRAME_ROUND            },
      { Sid::measureNumberFrameFgColor,          Pid::FRAME_FG_COLOR         },
      { Sid::measureNumberFrameBgColor,          Pid::FRAME_BG_COLOR         },
      }};

const TextStyle mmRestRangeTextStyle {{
      { Sid::mmRestRangeFontFace,              Pid::FONT_FACE              },
      { Sid::mmRestRangeFontSize,              Pid::FONT_SIZE              },
      { Sid::mmRestRangeFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::mmRestRangeFontStyle,             Pid::FONT_STYLE             },
      { Sid::mmRestRangeColor,                 Pid::COLOR                  },
      { Sid::mmRestRangeAlign,                 Pid::ALIGN                  },
      { Sid::mmRestRangePosAbove,              Pid::OFFSET                 },
      { Sid::mmRestRangeFrameType,             Pid::FRAME_TYPE             },
      { Sid::mmRestRangeFramePadding,          Pid::FRAME_PADDING          },
      { Sid::mmRestRangeFrameWidth,            Pid::FRAME_WIDTH            },
      { Sid::mmRestRangeFrameRound,            Pid::FRAME_ROUND            },
      { Sid::mmRestRangeFrameFgColor,          Pid::FRAME_FG_COLOR         },
      { Sid::mmRestRangeFrameBgColor,          Pid::FRAME_BG_COLOR         },
      }};

const TextStyle translatorTextStyle {{
      { Sid::translatorFontFace,                 Pid::FONT_FACE              },
      { Sid::translatorFontSize,                 Pid::FONT_SIZE              },
      { Sid::translatorLineSpacing,              Pid::TEXT_LINE_SPACING      },
      { Sid::translatorFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::translatorFontStyle,                Pid::FONT_STYLE             },
      { Sid::translatorColor,                    Pid::COLOR                  },
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
      { Sid::tupletFontFace,                     Pid::FONT_FACE              },
      { Sid::tupletFontSize,                     Pid::FONT_SIZE              },
      { Sid::tupletLineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::tupletFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::tupletFontStyle,                    Pid::FONT_STYLE             },
      { Sid::tupletColor,                        Pid::COLOR                  },
      { Sid::tupletAlign,                        Pid::ALIGN                  },
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
      { Sid::systemTextLineSpacing,              Pid::TEXT_LINE_SPACING      },
      { Sid::systemTextFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::systemTextFontStyle,                Pid::FONT_STYLE             },
      { Sid::systemTextColor,                    Pid::COLOR                  },
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
      { Sid::staffTextLineSpacing,               Pid::TEXT_LINE_SPACING      },
      { Sid::staffTextFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::staffTextFontStyle,                 Pid::FONT_STYLE             },
      { Sid::staffTextColor,                     Pid::COLOR                  },
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
      { Sid::chordSymbolALineSpacing,            Pid::TEXT_LINE_SPACING      },
      { Sid::chordSymbolAFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::chordSymbolAFontStyle,              Pid::FONT_STYLE             },
      { Sid::chordSymbolAColor,                  Pid::COLOR                  },
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
      { Sid::chordSymbolBLineSpacing,            Pid::TEXT_LINE_SPACING      },
      { Sid::chordSymbolBFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::chordSymbolBFontStyle,              Pid::FONT_STYLE             },
      { Sid::chordSymbolBColor,                  Pid::COLOR                  },
      { Sid::chordSymbolBAlign,                  Pid::ALIGN                  },
      { Sid::chordSymbolBPosAbove,               Pid::OFFSET                 },
      { Sid::chordSymbolBFrameType,              Pid::FRAME_TYPE             },
      { Sid::chordSymbolBFramePadding,           Pid::FRAME_PADDING          },
      { Sid::chordSymbolBFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::chordSymbolBFrameRound,             Pid::FRAME_ROUND            },
      { Sid::chordSymbolBFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::chordSymbolBFrameBgColor,           Pid::FRAME_BG_COLOR         },
      }};

const TextStyle romanNumeralTextStyle {{
      { Sid::romanNumeralFontFace,               Pid::FONT_FACE              },
      { Sid::romanNumeralFontSize,               Pid::FONT_SIZE              },
      { Sid::romanNumeralLineSpacing,            Pid::TEXT_LINE_SPACING      },
      { Sid::romanNumeralFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::romanNumeralFontStyle,              Pid::FONT_STYLE             },
      { Sid::romanNumeralColor,                  Pid::COLOR                  },
      { Sid::romanNumeralAlign,                  Pid::ALIGN                  },
      { Sid::romanNumeralPosAbove,               Pid::OFFSET                 },
      { Sid::romanNumeralFrameType,              Pid::FRAME_TYPE             },
      { Sid::romanNumeralFramePadding,           Pid::FRAME_PADDING          },
      { Sid::romanNumeralFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::romanNumeralFrameRound,             Pid::FRAME_ROUND            },
      { Sid::romanNumeralFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::romanNumeralFrameBgColor,           Pid::FRAME_BG_COLOR         },
      }};

const TextStyle nashvilleNumberTextStyle {{
      { Sid::nashvilleNumberFontFace,               Pid::FONT_FACE              },
      { Sid::nashvilleNumberFontSize,               Pid::FONT_SIZE              },
      { Sid::nashvilleNumberLineSpacing,            Pid::TEXT_LINE_SPACING      },
      { Sid::nashvilleNumberFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::nashvilleNumberFontStyle,              Pid::FONT_STYLE             },
      { Sid::nashvilleNumberColor,                  Pid::COLOR                  },
      { Sid::nashvilleNumberAlign,                  Pid::ALIGN                  },
      { Sid::nashvilleNumberPosAbove,               Pid::OFFSET                 },
      { Sid::nashvilleNumberFrameType,              Pid::FRAME_TYPE             },
      { Sid::nashvilleNumberFramePadding,           Pid::FRAME_PADDING          },
      { Sid::nashvilleNumberFrameWidth,             Pid::FRAME_WIDTH            },
      { Sid::nashvilleNumberFrameRound,             Pid::FRAME_ROUND            },
      { Sid::nashvilleNumberFrameFgColor,           Pid::FRAME_FG_COLOR         },
      { Sid::nashvilleNumberFrameBgColor,           Pid::FRAME_BG_COLOR         },
      }};

const TextStyle rehearsalMarkTextStyle {{
      { Sid::rehearsalMarkFontFace,              Pid::FONT_FACE              },
      { Sid::rehearsalMarkFontSize,              Pid::FONT_SIZE              },
      { Sid::rehearsalMarkLineSpacing,           Pid::TEXT_LINE_SPACING      },
      { Sid::rehearsalMarkFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::rehearsalMarkFontStyle,             Pid::FONT_STYLE             },
      { Sid::rehearsalMarkColor,                 Pid::COLOR                  },
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
      { Sid::repeatLeftLineSpacing,              Pid::TEXT_LINE_SPACING      },
      { Sid::repeatLeftFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::repeatLeftFontStyle,                Pid::FONT_STYLE             },
      { Sid::repeatLeftColor,                    Pid::COLOR                  },
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
      { Sid::repeatRightLineSpacing,             Pid::TEXT_LINE_SPACING      },
      { Sid::repeatRightFontSpatiumDependent,    Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::repeatRightFontStyle,               Pid::FONT_STYLE             },
      { Sid::repeatRightColor,                   Pid::COLOR                  },
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
      { Sid::frameLineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::frameFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::frameFontStyle,                     Pid::FONT_STYLE             },
      { Sid::frameColor,                         Pid::COLOR                  },
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
      { Sid::textLineLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::textLineFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::textLineFontStyle,                  Pid::BEGIN_FONT_STYLE       },
      { Sid::textLineColor,                      Pid::COLOR                  },
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
      { Sid::glissandoLineSpacing,               Pid::TEXT_LINE_SPACING      },
      { Sid::glissandoFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::glissandoFontStyle,                 Pid::FONT_STYLE             },
      { Sid::glissandoColor,                     Pid::COLOR                  },
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
      { Sid::ottavaLineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::ottavaFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::ottavaFontStyle,                    Pid::BEGIN_FONT_STYLE       },
      { Sid::ottavaColor,                        Pid::COLOR                  },
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
      { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE        },
      { Sid::voltaFontSize,                      Pid::BEGIN_FONT_SIZE        },
      { Sid::voltaLineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::voltaFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::voltaFontStyle,                     Pid::BEGIN_FONT_STYLE       },
      { Sid::voltaColor,                         Pid::COLOR                  },
      { Sid::voltaAlign,                         Pid::BEGIN_TEXT_ALIGN       },
      { Sid::voltaOffset,                        Pid::BEGIN_TEXT_OFFSET      },
      { Sid::voltaFrameType,                     Pid::FRAME_TYPE             },
      { Sid::voltaFramePadding,                  Pid::FRAME_PADDING          },
      { Sid::voltaFrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::voltaFrameRound,                    Pid::FRAME_ROUND            },
      { Sid::voltaFrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::voltaFrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle pedalTextStyle {{
      { Sid::pedalFontFace,                      Pid::BEGIN_FONT_FACE        },
      { Sid::pedalFontSize,                      Pid::BEGIN_FONT_SIZE        },
      { Sid::pedalLineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::pedalFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::pedalFontStyle,                     Pid::BEGIN_FONT_STYLE       },
      { Sid::pedalColor,                         Pid::COLOR                  },
      { Sid::pedalTextAlign,                     Pid::BEGIN_TEXT_ALIGN       },
      { Sid::pedalPosAbove,                      Pid::BEGIN_TEXT_OFFSET      },
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
      { Sid::letRingLineSpacing,                 Pid::TEXT_LINE_SPACING      },
      { Sid::letRingFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::letRingFontStyle,                   Pid::BEGIN_FONT_STYLE       },
      { Sid::letRingColor,                       Pid::COLOR                  },
      { Sid::letRingTextAlign,                   Pid::BEGIN_TEXT_ALIGN       },
      { Sid::letRingPosAbove,                    Pid::BEGIN_TEXT_OFFSET      },
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
      { Sid::palmMuteLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::palmMuteFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::palmMuteFontStyle,                  Pid::BEGIN_FONT_STYLE       },
      { Sid::palmMuteColor,                      Pid::COLOR                  },
      { Sid::palmMuteTextAlign,                  Pid::BEGIN_TEXT_ALIGN       },
      { Sid::palmMutePosAbove,                   Pid::BEGIN_TEXT_OFFSET      },
      { Sid::palmMuteFrameType,                  Pid::FRAME_TYPE             },
      { Sid::palmMuteFramePadding,               Pid::FRAME_PADDING          },
      { Sid::palmMuteFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::palmMuteFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::palmMuteFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::palmMuteFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle hairpinTextStyle {{
      { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE        },
      { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE        },
      { Sid::hairpinLineSpacing,                 Pid::TEXT_LINE_SPACING      },
      { Sid::hairpinFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::hairpinFontStyle,                   Pid::BEGIN_FONT_STYLE       },
      { Sid::hairpinColor,                       Pid::COLOR                  },
      { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN       },
      { Sid::hairpinPosAbove,                    Pid::BEGIN_TEXT_OFFSET      },
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
      { Sid::bendLineSpacing,                    Pid::TEXT_LINE_SPACING      },
      { Sid::bendFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::bendFontStyle,                      Pid::FONT_STYLE             },
      { Sid::bendColor,                          Pid::COLOR                  },
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
      { Sid::headerLineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::headerFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::headerFontStyle,                    Pid::FONT_STYLE             },
      { Sid::headerColor,                        Pid::COLOR                  },
      { Sid::headerAlign,                        Pid::ALIGN                  },
      { Sid::headerOffset,                       Pid::OFFSET                 },
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
      { Sid::footerLineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::footerFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::footerFontStyle,                    Pid::FONT_STYLE             },
      { Sid::footerColor,                        Pid::COLOR                  },
      { Sid::footerAlign,                        Pid::ALIGN                  },
      { Sid::footerOffset,                       Pid::OFFSET                 },
      { Sid::footerFrameType,                    Pid::FRAME_TYPE             },
      { Sid::footerFramePadding,                 Pid::FRAME_PADDING          },
      { Sid::footerFrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::footerFrameRound,                   Pid::FRAME_ROUND            },
      { Sid::footerFrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::footerFrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle instrumentChangeTextStyle {{
      { Sid::instrumentChangeFontFace,             Pid::FONT_FACE              },
      { Sid::instrumentChangeFontSize,             Pid::FONT_SIZE              },
      { Sid::instrumentChangeLineSpacing,          Pid::TEXT_LINE_SPACING      },
      { Sid::instrumentChangeFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::instrumentChangeFontStyle,            Pid::FONT_STYLE             },
      { Sid::instrumentChangeColor,                Pid::COLOR                  },
      { Sid::instrumentChangeAlign,                Pid::ALIGN                  },
      { Sid::instrumentChangeOffset,               Pid::OFFSET                 },
      { Sid::instrumentChangeFrameType,            Pid::FRAME_TYPE             },
      { Sid::instrumentChangeFramePadding,         Pid::FRAME_PADDING          },
      { Sid::instrumentChangeFrameWidth,           Pid::FRAME_WIDTH            },
      { Sid::instrumentChangeFrameRound,           Pid::FRAME_ROUND            },
      { Sid::instrumentChangeFrameFgColor,         Pid::FRAME_FG_COLOR         },
      { Sid::instrumentChangeFrameBgColor,         Pid::FRAME_BG_COLOR         },
      }};

const TextStyle stickingTextStyle {{
      { Sid::stickingFontFace,                   Pid::FONT_FACE              },
      { Sid::stickingFontSize,                   Pid::FONT_SIZE              },
      { Sid::stickingLineSpacing,                Pid::TEXT_LINE_SPACING      },
      { Sid::stickingFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::stickingFontStyle,                  Pid::FONT_STYLE             },
      { Sid::stickingColor,                      Pid::COLOR                  },
      { Sid::stickingAlign,                      Pid::ALIGN                  },
      { Sid::stickingOffset,                     Pid::OFFSET                 },
      { Sid::stickingFrameType,                  Pid::FRAME_TYPE             },
      { Sid::stickingFramePadding,               Pid::FRAME_PADDING          },
      { Sid::stickingFrameWidth,                 Pid::FRAME_WIDTH            },
      { Sid::stickingFrameRound,                 Pid::FRAME_ROUND            },
      { Sid::stickingFrameFgColor,               Pid::FRAME_FG_COLOR         },
      { Sid::stickingFrameBgColor,               Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user1TextStyle {{
      { Sid::user1FontFace,                      Pid::FONT_FACE              },
      { Sid::user1FontSize,                      Pid::FONT_SIZE              },
      { Sid::user1LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user1FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user1FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user1Color,                         Pid::COLOR                  },
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
      { Sid::user2LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user2FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user2FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user2Color,                         Pid::COLOR                  },
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
      { Sid::user3LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user3FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user3FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user3Color,                         Pid::COLOR                  },
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
      { Sid::user4LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user4FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user4FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user4Color,                         Pid::COLOR                  },
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
      { Sid::user5LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user5FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user5FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user5Color,                         Pid::COLOR                  },
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
      { Sid::user6LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user6FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user6FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user6Color,                         Pid::COLOR                  },
      { Sid::user6Align,                         Pid::ALIGN                  },
      { Sid::user6Offset,                        Pid::OFFSET                 },
      { Sid::user6FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user6FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user6FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user6FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user6FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user6FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user7TextStyle {{
      { Sid::user7FontFace,                      Pid::FONT_FACE              },
      { Sid::user7FontSize,                      Pid::FONT_SIZE              },
      { Sid::user7LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user7FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user7FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user7Color,                         Pid::COLOR                  },
      { Sid::user7Align,                         Pid::ALIGN                  },
      { Sid::user7Offset,                        Pid::OFFSET                 },
      { Sid::user7FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user7FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user7FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user7FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user7FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user7FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user8TextStyle {{
      { Sid::user8FontFace,                      Pid::FONT_FACE              },
      { Sid::user8FontSize,                      Pid::FONT_SIZE              },
      { Sid::user8LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user8FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user8FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user8Color,                         Pid::COLOR                  },
      { Sid::user8Align,                         Pid::ALIGN                  },
      { Sid::user8Offset,                        Pid::OFFSET                 },
      { Sid::user8FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user8FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user8FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user8FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user8FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user8FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user9TextStyle {{
      { Sid::user9FontFace,                      Pid::FONT_FACE              },
      { Sid::user9FontSize,                      Pid::FONT_SIZE              },
      { Sid::user9LineSpacing,                   Pid::TEXT_LINE_SPACING      },
      { Sid::user9FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user9FontStyle,                     Pid::FONT_STYLE             },
      { Sid::user9Color,                         Pid::COLOR                  },
      { Sid::user9Align,                         Pid::ALIGN                  },
      { Sid::user9Offset,                        Pid::OFFSET                 },
      { Sid::user9FrameType,                     Pid::FRAME_TYPE             },
      { Sid::user9FramePadding,                  Pid::FRAME_PADDING          },
      { Sid::user9FrameWidth,                    Pid::FRAME_WIDTH            },
      { Sid::user9FrameRound,                    Pid::FRAME_ROUND            },
      { Sid::user9FrameFgColor,                  Pid::FRAME_FG_COLOR         },
      { Sid::user9FrameBgColor,                  Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user10TextStyle {{
      { Sid::user10FontFace,                     Pid::FONT_FACE              },
      { Sid::user10FontSize,                     Pid::FONT_SIZE              },
      { Sid::user10LineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::user10FontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user10FontStyle,                    Pid::FONT_STYLE             },
      { Sid::user10Color,                        Pid::COLOR                  },
      { Sid::user10Align,                        Pid::ALIGN                  },
      { Sid::user10Offset,                       Pid::OFFSET                 },
      { Sid::user10FrameType,                    Pid::FRAME_TYPE             },
      { Sid::user10FramePadding,                 Pid::FRAME_PADDING          },
      { Sid::user10FrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::user10FrameRound,                   Pid::FRAME_ROUND            },
      { Sid::user10FrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::user10FrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user11TextStyle {{
      { Sid::user11FontFace,                     Pid::FONT_FACE              },
      { Sid::user11FontSize,                     Pid::FONT_SIZE              },
      { Sid::user11LineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::user11FontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user11FontStyle,                    Pid::FONT_STYLE             },
      { Sid::user11Color,                        Pid::COLOR                  },
      { Sid::user11Align,                        Pid::ALIGN                  },
      { Sid::user11Offset,                       Pid::OFFSET                 },
      { Sid::user11FrameType,                    Pid::FRAME_TYPE             },
      { Sid::user11FramePadding,                 Pid::FRAME_PADDING          },
      { Sid::user11FrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::user11FrameRound,                   Pid::FRAME_ROUND            },
      { Sid::user11FrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::user11FrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

const TextStyle user12TextStyle {{
      { Sid::user12FontFace,                     Pid::FONT_FACE              },
      { Sid::user12FontSize,                     Pid::FONT_SIZE              },
      { Sid::user12LineSpacing,                  Pid::TEXT_LINE_SPACING      },
      { Sid::user12FontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
      { Sid::user12FontStyle,                    Pid::FONT_STYLE             },
      { Sid::user12Color,                        Pid::COLOR                  },
      { Sid::user12Align,                        Pid::ALIGN                  },
      { Sid::user12Offset,                       Pid::OFFSET                 },
      { Sid::user12FrameType,                    Pid::FRAME_TYPE             },
      { Sid::user12FramePadding,                 Pid::FRAME_PADDING          },
      { Sid::user12FrameWidth,                   Pid::FRAME_WIDTH            },
      { Sid::user12FrameRound,                   Pid::FRAME_ROUND            },
      { Sid::user12FrameFgColor,                 Pid::FRAME_FG_COLOR         },
      { Sid::user12FrameBgColor,                 Pid::FRAME_BG_COLOR         },
      }};

//---------------------------------------------------------
//   TextStyleName
//---------------------------------------------------------

struct TextStyleName {
      const char* name;
      const char* name4; // Mu4 compatibility
      const TextStyle* ts;
      Tid tid;
      };

// Must be in sync with Tid enum (in types.h)

static constexpr std::array<TextStyleName, int(Tid::TEXT_STYLES)> textStyles { {
      { QT_TRANSLATE_NOOP("TextStyle", "Default"),                 "default",             &defaultTextStyle,           Tid::DEFAULT },
// Page-orientde styles
      { QT_TRANSLATE_NOOP("TextStyle", "Title"),                   "title",               &titleTextStyle,             Tid::TITLE },
      { QT_TRANSLATE_NOOP("TextStyle", "Subtitle"),                "subtitle",            &subTitleTextStyle,          Tid::SUBTITLE },
      { QT_TRANSLATE_NOOP("TextStyle", "Composer"),                "composer",            &composerTextStyle,          Tid::COMPOSER },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyricist"),                "lyricist",            &lyricistTextStyle,          Tid::POET },
      { QT_TRANSLATE_NOOP("TextStyle", "Translator"),              "translator",          &translatorTextStyle,        Tid::TRANSLATOR },
      { QT_TRANSLATE_NOOP("TextStyle", "Frame"),                   "frame",               &frameTextStyle,             Tid::FRAME },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Part)"),  "instrument_excerpt",  &partInstrumentTextStyle,    Tid::INSTRUMENT_EXCERPT },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Long)"),  "instrument_long",     &longInstrumentTextStyle,    Tid::INSTRUMENT_LONG },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Short)"), "instrument_short",    &shortInstrumentTextStyle,   Tid::INSTRUMENT_SHORT },
      { QT_TRANSLATE_NOOP("TextStyle", "Instrument Change"),       "instrument_chgange",  &instrumentChangeTextStyle,  Tid::INSTRUMENT_CHANGE },
      { QT_TRANSLATE_NOOP("TextStyle", "Header"),                  "header",              &headerTextStyle,            Tid::HEADER },
      { QT_TRANSLATE_NOOP("TextStyle", "Footer"),                  "footer",              &footerTextStyle,            Tid::FOOTER },
// Measure-oriented styles
      { QT_TRANSLATE_NOOP("TextStyle", "Measure Number"),          "measure_number",      &measureNumberTextStyle,     Tid::MEASURE_NUMBER },
      { QT_TRANSLATE_NOOP("TextStyle", "Multimeasure Rest Range"), "mmrest_range",        &mmRestRangeTextStyle,       Tid::MMREST_RANGE },
// Sytem-level styles
      { QT_TRANSLATE_NOOP("TextStyle", "Tempo"),                   "tempo",               &tempoTextStyle,             Tid::TEMPO },
      { QT_TRANSLATE_NOOP("TextStyle", "Metronome"),               "metronome",           &metronomeTextStyle,         Tid::METRONOME },
      { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Left"),        "repeat_left",         &repeatLeftTextStyle,        Tid::REPEAT_LEFT },
      { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Right"),       "repeat_right",        &repeatRightTextStyle,       Tid::REPEAT_RIGHT },
      { QT_TRANSLATE_NOOP("TextStyle", "Rehearsal Mark"),          "rehersal_mark",       &rehearsalMarkTextStyle,     Tid::REHEARSAL_MARK },
      { QT_TRANSLATE_NOOP("TextStyle", "System"),                  "system",              &systemTextStyle,            Tid::SYSTEM },
// Staff oriented styles
      { QT_TRANSLATE_NOOP("TextStyle", "Staff"),                   "staff",               &staffTextStyle,             Tid::STAFF },
      { QT_TRANSLATE_NOOP("TextStyle", "Expression"),              "expression",          &expressionTextStyle,        Tid::EXPRESSION },
      { QT_TRANSLATE_NOOP("TextStyle", "Dynamics"),                "dynamics",            &dynamicsTextStyle,          Tid::DYNAMICS },
      { QT_TRANSLATE_NOOP("TextStyle", "Hairpin"),                 "hairpin",             &hairpinTextStyle,           Tid::HAIRPIN },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Odd Lines"),        "lyrics_odd",          &lyricsOddTextStyle,         Tid::LYRICS_ODD },
      { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Even Lines"),       "lyrics_even",         &lyricsEvenTextStyle,        Tid::LYRICS_EVEN },
      { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol"),            "harmony_a",           &chordSymbolTextStyleA,      Tid::HARMONY_A },
      { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol (Alternate)"),"harmony_b",           &chordSymbolTextStyleB,      Tid::HARMONY_B },
      { QT_TRANSLATE_NOOP("TextStyle", "Roman Numeral Analysis"),  "harmone_roman",       &romanNumeralTextStyle,      Tid::HARMONY_ROMAN },
      { QT_TRANSLATE_NOOP("TextStyle", "Nashville Number"),        "harmony_nashville",   &nashvilleNumberTextStyle,   Tid::HARMONY_NASHVILLE },
// Note oriented styles
      { QT_TRANSLATE_NOOP("TextStyle", "Tuplet"),                  "tuplet",              &tupletTextStyle,            Tid::TUPLET },
      { QT_TRANSLATE_NOOP("TextStyle", "Sticking"),                "sticking",            &stickingTextStyle,          Tid::STICKING },
      { QT_TRANSLATE_NOOP("TextStyle", "Fingering"),               "fingering",           &fingeringTextStyle,         Tid::FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "LH Guitar Fingering"),     "guitar_fingering_lh", &lhGuitarFingeringTextStyle, Tid::LH_GUITAR_FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "RH Guitar Fingering"),     "guitar_fingering_rh", &rhGuitarFingeringTextStyle, Tid::RH_GUITAR_FINGERING },
      { QT_TRANSLATE_NOOP("TextStyle", "String Number"),           "string_number",       &stringNumberTextStyle,      Tid::STRING_NUMBER },
// Line-oriented styles
      { QT_TRANSLATE_NOOP("TextStyle", "Text Line"),               "textline",            &textLineTextStyle,          Tid::TEXTLINE },
      { QT_TRANSLATE_NOOP("TextStyle", "Volta"),                   "volta",               &voltaTextStyle,             Tid::VOLTA },
      { QT_TRANSLATE_NOOP("TextStyle", "Ottava"),                  "ottava",              &ottavaTextStyle,            Tid::OTTAVA },
      { QT_TRANSLATE_NOOP("TextStyle", "Glissando"),               "glissando",           &glissandoTextStyle,         Tid::GLISSANDO },
      { QT_TRANSLATE_NOOP("TextStyle", "Pedal"),                   "pedal",               &pedalTextStyle,             Tid::PEDAL },
      { QT_TRANSLATE_NOOP("TextStyle", "Bend"),                    "bend",                &bendTextStyle,              Tid::BEND },
      { QT_TRANSLATE_NOOP("TextStyle", "Let Ring"),                "let_ring",            &letRingTextStyle,           Tid::LET_RING },
      { QT_TRANSLATE_NOOP("TextStyle", "Palm Mute"),               "palm_mute",           &palmMuteTextStyle,          Tid::PALM_MUTE },
// User styles
      { QT_TRANSLATE_NOOP("TextStyle", "User-1"),                  "user_1",              &user1TextStyle,             Tid::USER1 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-2"),                  "user_2",              &user2TextStyle,             Tid::USER2 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-3"),                  "user_3",              &user3TextStyle,             Tid::USER3 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-4"),                  "user_4",              &user4TextStyle,             Tid::USER4 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-5"),                  "user_5",              &user5TextStyle,             Tid::USER5 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-6"),                  "user_6",              &user6TextStyle,             Tid::USER6 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-7"),                  "user_7",              &user7TextStyle,             Tid::USER7 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-8"),                  "user_8",              &user8TextStyle,             Tid::USER8 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-9"),                  "user_9",              &user9TextStyle,             Tid::USER9 },
      { QT_TRANSLATE_NOOP("TextStyle", "User-10"),                 "user_10",             &user10TextStyle,            Tid::USER10},
      { QT_TRANSLATE_NOOP("TextStyle", "User-11"),                 "user_11",             &user11TextStyle,            Tid::USER11},
      { QT_TRANSLATE_NOOP("TextStyle", "User-12"),                 "user_12",             &user12TextStyle,            Tid::USER12},
      } };

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle* textStyle(const char* name)
      {
      for (const auto& s : textStyles) {
            if (!strcmp(s.name, name)
                || !strcmp(s.name4, name)) // Mu4 compatibility
                  return s.ts;
            }
      if (!strcmp(name, "poet"))           // Mu4 compatibility
            return &lyricistTextStyle;

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
            if (s.name == name
                || s.name4 == name) // Mu4 compatibility
                  return s.tid;
            }
      if (name == "Technique")      // compatibility
            return Tid::EXPRESSION;
      else if (name == "poet")      // Mu4 compatibility
            return Tid::POET;

      qDebug("text style <%s> not known", qPrintable(name));
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
      Tid::INSTRUMENT_EXCERPT,
      Tid::INSTRUMENT_CHANGE,
      Tid::HEADER,
      Tid::FOOTER,
      Tid::MEASURE_NUMBER,
      Tid::MMREST_RANGE,
      Tid::TEMPO,
      Tid::REPEAT_LEFT,
      Tid::REPEAT_RIGHT,
      Tid::REHEARSAL_MARK,
      Tid::SYSTEM,
      Tid::STAFF,
      Tid::EXPRESSION,
      Tid::DYNAMICS,
      Tid::HAIRPIN,
      Tid::LYRICS_ODD,
      Tid::LYRICS_EVEN,
      Tid::HARMONY_A,
      Tid::HARMONY_B,
      Tid::HARMONY_ROMAN,
      Tid::HARMONY_NASHVILLE,
      Tid::STICKING,
      Tid::USER1,
      Tid::USER2,
      Tid::USER3,
      Tid::USER4,
      Tid::USER5,
      Tid::USER6,
      Tid::USER7,
      Tid::USER8,
      Tid::USER9,
      Tid::USER10,
      Tid::USER11,
      Tid::USER12,
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
//   pageStyles
//---------------------------------------------------------

QSet<Sid> pageStyles()
{
    static const QSet<Sid> styles {
        Sid::pageWidth,
        Sid::pageHeight,
        Sid::pagePrintableWidth,
        Sid::pageEvenTopMargin,
        Sid::pageEvenBottomMargin,
        Sid::pageEvenLeftMargin,
        Sid::pageOddTopMargin,
        Sid::pageOddBottomMargin,
        Sid::pageOddLeftMargin,
        Sid::pageTwosided,
        Sid::spatium
    };

    return styles;
}

//---------------------------------------------------------
//   fretStyles
//---------------------------------------------------------

QSet<Sid> fretStyles()
{
      static const QSet<Sid> styles {
            Sid::fretPlacement,
            Sid::fretStrings,
            Sid::fretFrets,
            Sid::fretOrientation,
    };

    return styles;
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

const QVariant& MStyle::value(Sid idx) const
      {
      const QVariant& val = _values[int(idx)];
      if (!val.isValid()) {
            qDebug("invalid style value %d %s", int(idx), MStyle::valueName(idx));
            static QVariant emptyVal;
            return emptyVal;
            }
      return val;
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

MStyle* MStyle::resolveStyleDefaults(const int defaultsVersion)
      {
      switch (defaultsVersion) {
            case LEGACY_MSC_VERSION_V3:
                 return styleDefaults301();
            case LEGACY_MSC_VERSION_V2:
                 return styleDefaults206();
            case LEGACY_MSC_VERSION_V1:
                 return styleDefaults114();
            default:
                 return &MScore::baseStyle();
          }
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

MStyle::MStyle()
      {
      _defaultStyleVersion = MSCVERSION;
      _customChordList = false;
      for (const StyleType& t : styleTypes)
            _values[t.idx()] = t.defaultValue();
      }

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
      return value(idx) == resolveStyleDefaults(_defaultStyleVersion)->value(idx);
      }

void MStyle::setDefaultStyleVersion(const int defaultsVersion)
      {
      _defaultStyleVersion = defaultsVersion;
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
//   checkChordList
//---------------------------------------------------------

void MStyle::checkChordList()
      {
      // make sure we have a chordlist
      if (!_chordList.loaded()) {
            qreal emag = value(Sid::chordExtensionMag).toDouble();
            qreal eadjust = value(Sid::chordExtensionAdjust).toDouble();
            qreal mmag = value(Sid::chordModifierMag).toDouble();
            qreal madjust = value(Sid::chordModifierAdjust).toDouble();
            _chordList.configureAutoAdjust(emag, eadjust, mmag, madjust);
            if (value(Sid::chordsXmlFile).toBool())
                  _chordList.read("chords.xml");
            _chordList.read(value(Sid::chordDescriptionFile).toString());
            }
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
                        qreal x = e.doubleAttribute("w", 0.0);
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
//    Handle transition from separate bold, underline, strike
//    and italic style properties to the single *FontStyle
//    property set.
//---------------------------------------------------------

bool MStyle::readTextStyleValCompat(XmlReader& e)
      {
      static const std::array<std::pair<const char*, FontStyle>, 4> styleNamesEndings {{
            { "FontBold",      FontStyle::Bold      },
            { "FontItalic",    FontStyle::Italic    },
            { "FontUnderline", FontStyle::Underline },
            { "FontStrike",    FontStyle::Strike    }
            }};

      const QStringRef tag(e.name());
      FontStyle readFontStyle = FontStyle::Normal;
      QStringRef typeName;
      for (auto& fontStyle : styleNamesEndings) {
            if (tag.endsWith(fontStyle.first)) {
                  readFontStyle = fontStyle.second;
                  typeName = tag.mid(0, tag.length() - int(strlen(fontStyle.first)));
                  break;
                  }
            }
      if (readFontStyle == FontStyle::Normal)
            return false;

      const QString newFontStyleName = typeName.toString() + "FontStyle";
      const Sid sid = MStyle::styleIdx(newFontStyleName);
      if (sid == Sid::NOSTYLE) {
            qDebug() << "readFontStyleValCompat: couldn't read text readFontStyle value:" << tag;
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

bool MStyle::load(QFile* qf, bool ign)
      {
      XmlReader e(qf);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl  = version.split('.');
                  int mscVersion  = sl[0].toInt() * 100 + sl[1].toInt();
                  if (mscVersion != MSCVERSION && !ign)
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

void MStyle::load(XmlReader& e, bool isMu4)
      {
      QString oldChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "TextStyle")
                  //readTextStyle206(this, e);        // obsolete
                  e.readElementText();
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
                  _chordList.unload();
                  _chordList.read(e);
                  _customChordList = true;
                  chordListTag = true;
                  }
            else if (tag == "lyricsDashMaxLegth") // pre-3.6 typo, now: "lyricsDashMaxLength"
                  set(Sid::lyricsDashMaxLength, Spatium(e.readDouble()));
// start 4.x compat, in oder of appearance in the style file
// fixing some Mu4 defaults to their Mu3.6 counterparts, skipping/ignoring others or letting them pass
            //else if (isMu4 && tag == "pageWidth") // 8.27 -> 8.27662, rounding issue, and depends on locale, printer setup, so let's pass
            //else if (isMu4 && tag == "pageHeight") // 11.69 -> 11.6929, rounding issue, and depends on locale, printer setup, so let's pass
            //else if (isMu4 && tag == "pagePrintableWidth") // 7.0889 -> 7.08661, rounding issue, and depends on locale, printer setup, so let's pass
            else if (tag == "staffHeaderFooterPadding" // Mu4 only, let's skip
                     || tag == "instrumentNameOffset" // Mu4 only, let's skip
                     || tag == "alignSystemToMargin") // Mu4 only, let's skip
                  e.skipCurrentElement();
            //else if (isMu4 && tag == "lyricsMinBottomDistance") // 1.5 -> 2, Mu4's default seems better, so let's pass
            else if (isMu4 && tag == "lyricsDashLineThickness") { // 0.1 -> 0.15
                  qreal lyricsDashLineThickness = e.readDouble();
                  if (qFuzzyCompare(lyricsDashLineThickness, 0.1)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::lyricsDashLineThickness, Spatium(lyricsDashLineThickness));
                  }
            else if (isMu4 && tag == "minMeasureWidth") { // 8 -> 5
                  qreal minMeasureWidth = e.readDouble();
                  if (qFuzzyCompare(minMeasureWidth, 8.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::minMeasureWidth, Spatium(minMeasureWidth));
                  }
            else if (isMu4 && (tag == "doubleBarDistance" // 0.37 -> 0.55, depends on font's `engravingDefaults`! Let's skip, i.e. reset back
                               || tag == "endBarDistance" // 0.37 -> 0.7, depends on font's `engravingDefaults`! Let's skip, i.e. reset back
                               || tag == "repeatBarlineDotSeparation")) // 0.37 -> 0.7, depends on font's `engravingDefaults`! Let's skip, i.e. reset back
                  e.skipCurrentElement();
            else if (isMu4 && tag == "bracketWidth") { // 0.45 -> 0.44
                  qreal bracketWidth = e.readDouble();
                  if (qFuzzyCompare(bracketWidth, 0.45)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::bracketWidth, Spatium(bracketWidth));
                  }
            else if (isMu4 && tag == "bracketDistance") // 0.45 -> 0.1, let's skip, i.e. reset back
                  e.skipCurrentElement();
            else if (isMu4 && tag == "akkoladeWidth") { // 1.5 -> 1.6
                  qreal akkoladeWidth = e.readDouble();
                  if (qFuzzyCompare(akkoladeWidth, 1.5)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::akkoladeWidth, Spatium(akkoladeWidth));
                  }
            else if (isMu4 && tag == "akkoladeBarDistance") { // 0.35 -> 0.4
                  qreal akkoladeBarDistance = e.readDouble();
                  if (qFuzzyCompare(akkoladeBarDistance, 0.35)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::akkoladeBarDistance, Spatium(akkoladeBarDistance));
                  }
            else if (isMu4 && tag == "clefLeftMargin") { // 0.75 -> 0.8
                  qreal clefLeftMargin = e.readDouble();
                  if (qFuzzyCompare(clefLeftMargin, 0.75)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::clefLeftMargin, Spatium(clefLeftMargin));
                  }
            else if (tag == "systemTrailerRightMargin" // Mu4 only, let's skip
                     || tag == "useStraightNoteFlags") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "stemWidth") // 0.1 -> 0.11, depends on font's `engravingDefaults`! Let's skip.
                  e.skipCurrentElement();
            else if (tag == "stemLength" // Mu4 only, let's skip
                     || tag == "stemLengthSmall" // Mu4 only, let's skip
                     || tag == "shortStemStartLocation") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "shortestStem") { // 2.5 -> 2.25
                  qreal shortestStem = e.readDouble();
                  if (qFuzzyCompare(shortestStem, 2.5)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::shortestStem, Spatium(shortestStem));
                  }
            else if (tag == "minStaffSizeForAutoStems" // Mu4.0 only, let's skip
                     || tag == "smallStaffStemDirection") // Mu4.0 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "minNoteDistance") { // 0.5 -> 0.2
                  qreal minNoteDistance = e.readDouble();
                  if (qFuzzyCompare(minNoteDistance, 0.5)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::minNoteDistance, Spatium(minNoteDistance));
                  }
            else if (isMu4 && tag == "measureSpacing") // 1.5 -> 1.2, let's skip, i.e. reset back
                  e.skipCurrentElement();
            else if (tag == "measureRepeatNumberPos" // Mu4 only, let's skip
                     || tag == "mrNumberSeries" // Mu4 only, let's skip
                     || tag == "mrNumberEveryXMeasures" // Mu4 only, let's skip
                     || tag == "mrNumberSeriesWithParentheses" // Mu4 only, let's skip
                     || tag == "oneMeasureRepeatShow1" // Mu4 only, let's skip
                     || tag == "fourMeasureRepeatShowExtenders") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "ledgerLineLength") // 0.33 -> 0.35, depends on font's `engravingDefaults`! Let's skip.
                  e.skipCurrentElement();
            else if (tag == "stemSlashPosition" // Mu4 only, let's skip
                     || tag == "stemSlashAngle" // Mu4 only, let's skip
                     || tag == "stemSlashThickness" // Mu4 only, let's skip
                     || tag == "keysigAccidentalDistance" // Mu4 only, let's skip
                     || tag == "keysigNaturalDistance") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (tag == "useWideBeams") // Mu4 for beamDistance, default depends on font's `engravingDefaults`! Maybe better skip?
                  e.skipCurrentElement();
            else if (isMu4 && tag == "beamMinLen") { // 1.1 -> 1.3
                  qreal beamMinLen = e.readDouble();
                  if (qFuzzyCompare(beamMinLen, 1.1)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::beamMinLen, Spatium(beamMinLen));
                  }
            else if (tag == "snapCustomBeamsToGrid") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && (tag == "propertyDistanceHead" // 0.4 -> 1, articulation/ornaments distance, let's skip, i.e. reset back
                               || tag == "propertyDistanceStem" // 0.4 -> 1.8, articulation/ornaments distance, let's skip, i.e. reset back
                               || tag == "propertyDistance")) // 0.4 -> 1, articulation/ornaments distance, let's skip, i.e. reset back
                  e.skipCurrentElement();
            //else if (isMu4 && tag == "articulationAnchorLuteFingering") // 1 -> 4 ? ToDo
            else if (isMu4 && (tag == "articulationStemHAlign" // Mu4.1+ only let's skip
                               ||tag == "articulationKeepTogether")) // Mu4.1+ only let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "hairpinLinePosAbove") { // y: -1.5 -> -3
                  QPointF hairpinLinePosAbove = e.readPoint();
                  if (qFuzzyCompare(hairpinLinePosAbove.y(), -1.5)) // 4.2+ default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::hairpinLinePosAbove, QPointF(hairpinLinePosAbove));
                  }
            else if (isMu4 && tag == "hairpinLinePosBelow") { // y: 4 (Mu4.0-4.1), 2.5 (Mu4.2+) -> 4
                  QPointF hairpinLinePosBelow = e.readPoint();
                  if (qFuzzyCompare(hairpinLinePosBelow.y(), 2.5)) // 4.2+ default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::hairpinLinePosBelow, QPointF(hairpinLinePosBelow));
                  }
            else if (isMu4 && tag == "hairpinLineStyle") {
                  int _lineStyle = Qt::SolidLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "dashed")
                        _lineStyle = Qt::DashLine;
                  set(Sid::hairpinLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "hairpinDashLineLen" // Mu4 only, let's skip
                     ||tag == "hairpinDashGapLen") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "hairpinLineLineStyle") {
                  int _lineStyle = Qt::CustomDashLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "solid")
                        _lineStyle = Qt::SolidLine;
                  set(Sid::hairpinLineLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "hairpinLineDashLineLen" // Mu4 only, let's skip
                     ||tag == "hairpinLineDashGapLen") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (tag == "pedalLineStyle" || (isMu4 && tag == "pedalListStyle")) { // pre-4.1 typo: "pedalListStyle"
                  int _lineStyle = Qt::SolidLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "dashed")
                        _lineStyle = Qt::DashLine;
                  set(Sid::pedalLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "pedalDashLineLen" // Mu4 only, let's skip
                     || tag == "pedalDashGapLen" // Mu4 only, let's skip
                     || tag == "pedalText" // Mu4.1+ only, let's skip
                     || tag == "pedalHookText" // Mu4.2+ only, let's skip
                     || tag == "pedalContinueText" // Mu4.1+ only, let's skip
                     || tag == "pedalContinueHookText" // Mu4.2+ only, let's skip
                     || tag == "pedalEndText" // Mu4.1+ only, let's skip
                     || tag == "pedalRosetteEndText") // Mu4.2+ only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "chordSymbolAFontSize") { // 10 -> 11
                  qreal chordSymbolAFontSize = e.readDouble();
                  if (qFuzzyCompare(chordSymbolAFontSize, 10.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::chordSymbolAFontSize, QVariant(chordSymbolAFontSize));
                  }
            else if (isMu4 && tag == "chordSymbolBFontSize") { // 10 -> 11
                  qreal chordSymbolBFontSize = e.readDouble();
                  if (qFuzzyCompare(chordSymbolBFontSize, 10.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::chordSymbolBFontSize, QVariant(chordSymbolBFontSize));
                  }
            else if (tag == "graceToMainNoteDist" // Mu4 only, let's skip
                     || tag == "graceToGraceNoteDist") // Mu4 only, let's skip
                  e.skipCurrentElement();
            //else if (isMu4 && tag == "useStandardNoteNames") // 1 -> 0, why??? Seems a mistake, let's pass
            else if (tag == "multiVoiceRestTwoSpaceOffset") // Mu4.1+ only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "minMMRestWidth") { // 4. resp. 6 -> 4
                  qreal minMMRestWidth = e.readDouble();
                  if (qFuzzyCompare(minMMRestWidth, 6.0)) // 4.1 default (4.0 same as 3.6), let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::minMMRestWidth, Spatium(minMMRestWidth));
                  }
            else if (isMu4 && tag == "mmRestNumberPos") { // -0.5 -> -1.5, maybe keep the Mu4 default?
                  qreal mmRestNumberPos = e.readDouble();
                  if (qFuzzyCompare(mmRestNumberPos, -0.5)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::mmRestNumberPos, Spatium(mmRestNumberPos));
                  }
            else if (tag == "mmRestNumberMaskHBar" // Mu4 only, let's skip
                     || tag == "multiMeasureRestMargin" // Mu4 only, let's skip
                     || tag == "mmRestHBarThickness" // Mu4 only, let's skip
                     || tag == "mmRestHBarVStrokeThickness" // Mu4 only, let's skip
                     || tag == "mmRestHBarVStrokeHeight" // Mu4 only, let's skip
                     || tag == "oldStyleMultiMeasureRests" // Mu4 only, let's skip
                     || tag == "mmRestOldStyleMaxMeasures" // Mu4 only, let's skip
                     || tag == "mmRestOldStyleSpacing") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (tag == "dontHideStavesInFirstSystem") // pre-4.0 typo: "dontHidStavesInFirstSystm"
                  set(Sid::dontHideStavesInFirstSystem, QVariant(e.readBool()));
            else if (tag == "alwaysShowSquareBracketsWhenEmptyStavesAreHidden" // Mu4 only, let's skip
                     || tag == "ArpeggioAccidentalDistance" // Mu4 only, let's skip
                     || tag == "ArpeggioAccidentalDistanceMin") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "ArpeggioNoteDistance") { // 0.4 (Mu4.2+) -> 0.5 (Mu3.7)
                  qreal ArpeggioNoteDistance = e.readDouble();
                  if (qFuzzyCompare(ArpeggioNoteDistance, 0.4)) // 4.2+ default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::ArpeggioNoteDistance, Spatium(ArpeggioNoteDistance));
                  }
            else if (isMu4 && tag == "ArpeggioAccidentalDistance") { // 0.3 (Mu4.2+) -> 0.5 (Mu3.7)
                  qreal ArpeggioAccidentalDistance = e.readDouble();
                  if (qFuzzyCompare(ArpeggioAccidentalDistance, 0.3)) // 4.2+ default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::ArpeggioAccidentalDistance, Spatium(ArpeggioAccidentalDistance));
                  }
            else if (isMu4 && tag == "slurEndWidth") // 0.05 -> 0.07, depends on font's `engravingDefaults`! Let's skip.
                  e.skipCurrentElement();
            else if (tag == "minStraightGlissandoLength" // Mu4.1+ only, let's skip
                     || tag == "minWigglyGlissandoLength" // Mu4.1+ only, let's skip
                     || tag == "headerSlurTieDistance") // Mu4 only, let's skip
                  e.skipCurrentElement();
            //else if (isMu4 && tag == "evenFooterC") // $C -> $:copyright:, hard to tell whether set on purpose, however: I do like the Mu4 default better, so let's pass
            //else if (isMu4 && tag == "oddFooterC") // $C -> $:copyright:, hard to tell whether set on purpose however: I do like the Mu4 default better, so let's pass
            else if (isMu4 && tag == "voltaLineStyle") {
                  int _lineStyle = Qt::SolidLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "dashed")
                        _lineStyle = Qt::DashLine;
                  set(Sid::ottavaLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "voltaDashLineLen" // Mu4 only, let's skip
                     || tag == "voltaDashGapLen") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "ottavaLineStyle") {
                  int _lineStyle = Qt::DashLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "solid")
                        _lineStyle = Qt::SolidLine;
                  set(Sid::ottavaLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "ottavaDashLineLen" // Mu4 only, let's skip
                     || tag == "ottavaDashGapLen" // Mu4 only, let's skip
                     || tag == "ottavaTextAlignAbove" // Mu4 only, let's skip
                     || tag == "ottavaTextAlignBelow" // Mu4 only, let's skip
                     || tag == "tremoloNoteSidePadding" // Mu4 only, let's skip
                     || tag == "tremoloOutSidePadding") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "tupletStemLeftDistance") { // 0.5 -> 0
                  qreal tupletStemLeftDistance = e.readDouble();
                  if (qFuzzyCompare(tupletStemLeftDistance, 0.5)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::tupletStemLeftDistance, Spatium(tupletStemLeftDistance));
                  }
            else if (isMu4 && tag == "tupletNoteLeftDistance") { // 0 -> -0.5
                  qreal tupletNoteLeftDistance = e.readDouble();
                  if (qFuzzyCompare(tupletNoteLeftDistance, 0.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::tupletNoteLeftDistance, Spatium(tupletNoteLeftDistance));
                  }
            else if (isMu4 && tag == "scaleBarlines") { // 0 -> 1, why???
                  bool scaleBarlines = e.readInt();
                  if (!scaleBarlines) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::scaleBarlines, QVariant(scaleBarlines));
                  }
            else if (tag == "dynamicsOverrideFont" // Mu4.1+ only, let's skip
                     || tag == "dynamicsFont" // Mu4.1+ only, let's skip
                     || tag == "dynamicsSize" // Mu4.1+ only, let's skip
                     || tag == "avoidBarLines"// Mu4.1+ only, let's skip
                     || tag == "snapToDynamics" // Mu4.1+ only, let's skip
                     || tag == "centerOnNotehead") // Mu4.1+ only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "subTitleFontSize") { // 14 -> 16
                  qreal subTitleFontSize = e.readDouble();
                  if (qFuzzyCompare(subTitleFontSize, 14.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::subTitleFontSize, QVariant(subTitleFontSize));
                  }
            else if (tag == "preferSameStringForTranspose" // Mu4.1+ only, let's skip
                     || tag.startsWith("harpPedal")) // Mu4.1+ only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "dynamicsFontSize") // 10 -> 11, Mu4 uses the musical font's ones, maybe that's the reason, so let's skip, i.e. reset back
                  e.skipCurrentElement();
            else if (tag.startsWith("tempoChange")) // Mu4.1+ only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "measureNumberPosBelow") { // y: 1 -> 2
                  QPointF measureNumberPosBelow = e.readPoint();
                  if (qFuzzyCompare(measureNumberPosBelow.y(), 1.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::measureNumberPosBelow, QPointF(measureNumberPosBelow));
                  }
            else if (tag == "expressionPosAbove" // Mu4.1+ only, let's skip
                     || tag == "expressionPosBelow" // Mu4.1+ only, let's skip
                     || tag == "expressionMinDistance" // Mu4.1+ only, let's skip
                     || tag == "glissandoStyle" // Mu4.2+ only, let's skip
                     || tag == "glissandoStyleHarp" // Mu4.2+ only, let's skip
                     || tag.startsWith("guitarBend") // Mu4.2+ only, let's skip
                     || tag == "useCueSizeFretForGraceBends") // Mu4.2+ only, let's skip
                  e.skipCurrentElement();
            //else if (tag == "headerAlign") // center,top -> center,center ToDo
            //else if (tag == "footerAlign") // center,bottom -> center,center Todo
            else if (isMu4 && tag == "footerOffset") { // y: 0 -> 5, better use Mu3's default to prevent collisions.
                  QPointF footerOffset = e.readPoint();
                  if (qFuzzyCompare(footerOffset.y(), 0.0)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::footerOffset, QPointF(footerOffset));
                  }
            else if (isMu4 && tag == "letRingLineWidth") { // 0.11 -> 0.15
                  qreal letRingLineWidth = e.readDouble();
                  if (qFuzzyCompare(letRingLineWidth, 0.11)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::letRingLineWidth, Spatium(letRingLineWidth));
                  }
            else if (isMu4 && tag == "letRingLineStyle") {
                  int _lineStyle = Qt::DashLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "solid")
                        _lineStyle = Qt::SolidLine;
                  set(Sid::letRingLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "letRingDashLineLen" // Mu4 only, let's skip
                     || tag == "letRingDashGapLen") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "palmMutePosAbove") { // y: -4 resp. 0 -> -4
                  QPointF palmMutePosAbove = e.readPoint();
                  if (qFuzzyCompare(palmMutePosAbove.y(), 0.0)) // 4.1 default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::palmMutePosAbove, QPointF(palmMutePosAbove));
                  }
            else if (isMu4 && tag == "palmMutePosBelow") { // y: 4 resp. 0 -> 4
                  QPointF palmMutePosBelow = e.readPoint();
                  if (qFuzzyCompare(palmMutePosBelow.y(), 0.0)) // 4.1 default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::palmMutePosBelow, QPointF(palmMutePosBelow));
                  }
            else if (isMu4 && tag == "palmMuteLineWidth") { // 0.11 -> 0.15
                  qreal palmMuteLineWidth = e.readDouble();
                  if (qFuzzyCompare(palmMuteLineWidth, 0.11)) // 4.x default, let's skip, i.e. reset back
                        e.skipCurrentElement();
                  else
                        set(Sid::palmMuteLineWidth, Spatium(palmMuteLineWidth));
                  }
            else if (isMu4 && tag == "palmMuteLineStyle") {
                  int _lineStyle = Qt::DashLine;
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "solid")
                        _lineStyle = Qt::SolidLine;
                  set(Sid::palmMuteLineStyle, QVariant(_lineStyle));
                  }
            else if (tag == "palmMuteDashLineLen" // Mu4 only, let's skip
                     || tag == "palmMuteDashGapLen") // Mu4 only, let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "articulationMinDistance") // 0.4 -> 0.5, reset back
                  set(Sid::articulationMinDistance, styleTypes[int(Sid::articulationMinDistance)].defaultValue()); // 3.x default
            else if (tag.contains("ShowTab") // Mu4 only, let's skip
                     || tag == "chordlineThickness") // doesn't exist in Mu3 (and was wrong in Mu4.0), let's skip
                  e.skipCurrentElement();
            else if (isMu4 && tag == "defaultsVersion") // 400/420 -> 302, let's sip, i.e. reset to Mu3
                  e.skipCurrentElement();
            //else if (isMu4 && tag == "Spatium") { // 1.74978 -> 1.75, rounding issue, has been read further up already
// end 4.x compat: WARNING: we're reaching MSVC's limit for nesting :-(
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

      if (!chordListTag)
            checkChordList();
      }

void MStyle::applyNewDefaults(const MStyle& other, const int defaultsVersion)
      {
      _defaultStyleVersion = defaultsVersion;

      for (auto st : qAsConst(styleTypes))
            if (isDefault(st.styleIdx())) {
                  st._defaultValue = other.value(st.styleIdx());
                  _values.at(st.idx()) = other.value(st.styleIdx());
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
                  if (optimize && a == Align(st.defaultValue().toInt()))
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

void MStyle::resetAllStyles(Score* score, const QSet<Sid>& ignoredStyles)
      {
      for (const StyleType& st : styleTypes) {
           if (ignoredStyles.isEmpty() || !ignoredStyles.contains(st.styleIdx())) {
                score->undo(new ChangeStyleVal(score, st.styleIdx(), MScore::defaultStyle().value(st.styleIdx())));
           }
      }
      }

void MStyle::resetStyles(Score* score, const QSet<Sid>& stylesToReset)
      {
      for (const StyleType& st : styleTypes) {
           if (stylesToReset.contains(st.styleIdx())) {
                score->undo(new ChangeStyleVal(score, st.styleIdx(), MScore::defaultStyle().value(st.styleIdx())));
           }
      }
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
