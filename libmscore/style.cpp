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

namespace Ms {

//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

static const StyleType styleTypes[] {
      { StyleIdx::staffUpperBorder,        "staffUpperBorder",        Spatium(7.0)  },
      { StyleIdx::staffLowerBorder,        "staffLowerBorder",        Spatium(7.0)  },
      { StyleIdx::staffDistance,           "staffDistance",           Spatium(6.5)  },
      { StyleIdx::akkoladeDistance,        "akkoladeDistance",        Spatium(6.5)  },
      { StyleIdx::minSystemDistance,       "minSystemDistance",       Spatium(8.5)  },
      { StyleIdx::maxSystemDistance,       "maxSystemDistance",       Spatium(15.0) },

      { StyleIdx::lyricsPlacement,         "lyricsPlacement",         int(Element::Placement::BELOW)  },
      { StyleIdx::lyricsPosAbove,          "lyricsPosAbove",          Spatium(-2.0) },
      { StyleIdx::lyricsPosBelow,          "lyricsPosBelow",          Spatium(2.0) },
      { StyleIdx::lyricsMinTopDistance,    "lyricsMinTopDistance",    Spatium(1.0)  },
      { StyleIdx::lyricsMinBottomDistance, "lyricsMinBottomDistance", Spatium(3.0)  },
      { StyleIdx::lyricsLineHeight,        "lyricsLineHeight",        1.0 },
      { StyleIdx::lyricsDashMinLength,     "lyricsDashMinLength",     Spatium(0.4) },
      { StyleIdx::lyricsDashMaxLength,     "lyricsDashMaxLegth",      Spatium(0.8) },
      { StyleIdx::lyricsDashMaxDistance,   "lyricsDashMaxDistance",   Spatium(16.0) },
      { StyleIdx::lyricsDashForce,         "lyricsDashForce",         QVariant(true) },
      { StyleIdx::lyricsAlignVerseNumber,  "lyricsAlignVerseNumber",  true },
      { StyleIdx::lyricsLineThickness,     "lyricsLineThickness",     Spatium(0.1) },

      { StyleIdx::figuredBassFontFamily,   "figuredBassFontFamily",   QString("MScoreBC") },

      { StyleIdx::figuredBassFontSize,     "figuredBassFontSize",     QVariant(8.0) },
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
      { StyleIdx::doubleBarDistance,       "doubleBarDistance",       Spatium(0.30) },
      { StyleIdx::endBarDistance,          "endBarDistance",          Spatium(0.40) },     // 0.3
      { StyleIdx::repeatBarlineDotSeparation, "repeatBarlineDotSeparation", Spatium(0.40) },
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
      { StyleIdx::clefKeyRightMargin,      "clefKeyRightMargin",      Spatium(0.8) },
      { StyleIdx::clefKeyDistance,         "clefKeyDistance",         Spatium(1.0) },   // gould: 1 - 1.25
      { StyleIdx::clefTimesigDistance,     "clefTimesigDistance",     Spatium(1.0) },
      { StyleIdx::keyTimesigDistance,      "keyTimesigDistance",      Spatium(1.0) },    // gould: 1 - 1.5
      { StyleIdx::keyBarlineDistance,      "keyTimesigDistance",      Spatium(1.0) },
      { StyleIdx::systemHeaderDistance,    "systemHeaderDistance",    Spatium(2.5) },     // gould: 2.5
      { StyleIdx::systemHeaderTimeSigDistance, "systemHeaderTimeSigDistance", Spatium(2.0) },  // gould: 2.0

      { StyleIdx::clefBarlineDistance,     "clefBarlineDistance",     Spatium(0.18) },      // was 0.5
      { StyleIdx::timesigBarlineDistance,  "timesigBarlineDistance",  Spatium(0.5) },
      { StyleIdx::stemWidth,               "stemWidth",               Spatium(0.13) },      // 0.09375
      { StyleIdx::shortenStem,             "shortenStem",             QVariant(true) },
      { StyleIdx::shortStemProgression,    "shortStemProgression",    Spatium(0.25) },
      { StyleIdx::shortestStem,            "shortestStem",            Spatium(2.25) },
      { StyleIdx::beginRepeatLeftMargin,   "beginRepeatLeftMargin",   Spatium(1.0) },
      { StyleIdx::minNoteDistance,         "minNoteDistance",         Spatium(0.25) },      // 0.4
      { StyleIdx::barNoteDistance,         "barNoteDistance",         Spatium(1.2) },

      { StyleIdx::barAccidentalDistance,   "barAccidentalDistance",   Spatium(.3) },
      { StyleIdx::multiMeasureRestMargin,  "multiMeasureRestMargin",  Spatium(1.2) },
      { StyleIdx::noteBarDistance,         "noteBarDistance",         Spatium(1.0) },
      { StyleIdx::measureSpacing,          "measureSpacing",          QVariant(1.2) },
      { StyleIdx::staffLineWidth,          "staffLineWidth",          Spatium(0.08) },      // 0.09375
      { StyleIdx::ledgerLineWidth,         "ledgerLineWidth",         Spatium(0.16) },     // 0.1875
      { StyleIdx::ledgerLineLength,        "ledgerLineLength",        Spatium(.6) },     // notehead width + this value
      { StyleIdx::accidentalDistance,      "accidentalDistance",      Spatium(0.22) },
      { StyleIdx::accidentalNoteDistance,  "accidentalNoteDistance",  Spatium(0.22) },
      { StyleIdx::beamWidth,               "beamWidth",               Spatium(0.5) },           // was 0.48

      { StyleIdx::beamDistance,            "beamDistance",            QVariant(0.5) },          // 0.25sp units
      { StyleIdx::beamMinLen,              "beamMinLen",              Spatium(1.32) },      // 1.316178 exactly notehead widthen beams
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
      { StyleIdx::hairpinY,                "hairpinY",                Spatium(7.5) },
      { StyleIdx::hairpinHeight,           "hairpinHeight",           Spatium(1.2) },
      { StyleIdx::hairpinContHeight,       "hairpinContHeight",       Spatium(0.5) },
      { StyleIdx::hairpinLineWidth,        "hairpinWidth",            Spatium(0.13) },
      { StyleIdx::pedalY,                  "pedalY",                  Spatium(8) },
      { StyleIdx::pedalLineWidth,          "pedalLineWidth",          Spatium(.15) },
      { StyleIdx::pedalLineStyle,          "pedalListStyle",          QVariant(int(Qt::SolidLine)) },
      { StyleIdx::trillY,                  "trillY",                  Spatium(-1) },

      { StyleIdx::harmonyY,                "harmonyY",                Spatium(2.5) },
      { StyleIdx::harmonyFretDist,         "harmonyFretDist",         Spatium(0.5) },
      { StyleIdx::minHarmonyDistance,      "minHarmonyDistance",      Spatium(0.5) },
      { StyleIdx::maxHarmonyBarDistance,   "maxHarmonyBarDistance",   Spatium(3.0) },
      { StyleIdx::capoPosition,            "capoPosition",            QVariant(0) },
      { StyleIdx::fretNumMag,              "fretNumMag",              QVariant(2.0) },
      { StyleIdx::fretNumPos,              "fretNumPos",              QVariant(0) },
      { StyleIdx::fretY,                   "fretY",                   Spatium(2.0) },
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
      { StyleIdx::SlurEndWidth,            "slurEndWidth",            Spatium(.07) },
      { StyleIdx::SlurMidWidth,            "slurMidWidth",            Spatium(.15) },
      { StyleIdx::SlurDottedWidth,         "slurDottedWidth",         Spatium(.1) },
      { StyleIdx::MinTieLength,            "minTieLength",            Spatium(1.0) },
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
      { StyleIdx::ottavaY,                 "ottavaY",                 Spatium(-3.0) },
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
      { StyleIdx::tupletDirection,         "tupletDirection",         Direction(Direction::AUTO) },
      { StyleIdx::tupletNumberType,        "tupletNumberType",        int(Tuplet::NumberType::SHOW_NUMBER) },
      { StyleIdx::tupletBracketType,       "tupletBracketType",       int(Tuplet::BracketType::AUTO_BRACKET) },

      { StyleIdx::barreLineWidth,          "barreLineWidth",          QVariant(1.0) },
      { StyleIdx::fretMag,                 "fretMag",                 QVariant(1.0) },
      { StyleIdx::scaleBarlines,           "scaleBarlines",           QVariant(true) },
      { StyleIdx::barGraceDistance,        "barGraceDistance",        Spatium(.6) },
      { StyleIdx::minVerticalDistance,     "minVerticalDistance",     Spatium(0.5) },
      { StyleIdx::ornamentStyle,           "ornamentStyle",           int(MScore::OrnamentStyle::DEFAULT) },
      { StyleIdx::spatium,                 "Spatium",                 SPATIUM20 },

      { StyleIdx::autoplaceHairpinDynamicsDistance, "autoplaceHairpinDynamicsDistance", Spatium(0.5) },
      { StyleIdx::dynamicsMinDistance,              "dynamicsMinDistance",               Spatium(0.5) },
      { StyleIdx::autoplaceVerticalAlignRange,      "autoplaceVerticalAlignRange",     int(VerticalAlignRange::SYSTEM) },

      { StyleIdx::textLinePlacement,         "textLinePlacement",         int(Element::Placement::ABOVE)  },
      { StyleIdx::textLinePosAbove,          "textLinePosAbove",          Spatium(-3.5) },
      { StyleIdx::textLinePosBelow,          "textLinePosBelow",          Spatium(3.5) },

      { StyleIdx::tremoloBarLineWidth,       "tremoloBarLineWidth",       Spatium(0.1) },
      };

//---------------------------------------------------------
//   valueType
//---------------------------------------------------------

const char* MStyle::valueType(const StyleIdx i)
      {
      return styleTypes[int(i)].valueType();
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

static const QString ff("FreeSerif");

//---------------------------------------------------------
//   setDefaultStyle
//    synchronize with TextStyleType
//---------------------------------------------------------

void initStyle(MStyle* s)
      {
#define MM(x) ((x)/INCH)
#define N(x)  QT_TRANSLATE_NOOP ("TextStyle", x)
#define LT    AlignmentFlags::LEFT | AlignmentFlags::TOP

      // this is an empty style, no offsets are allowed
      // never show this style
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", ""), ff, 10, false, false, false, AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, false, false, false, Spatium(.2), Spatium(.5), 25, QColor(Qt::black), false, false, QColor(Qt::black), QColor(255, 255, 255, 0), TextStyleHidden::ALWAYS));

      s->addTextStyle(TextStyle(N("Title"),                   ff, 24, false, false, false, AlignmentFlags::HCENTER | AlignmentFlags::TOP,      QPointF(), OffsetType::ABS));
      s->addTextStyle(TextStyle(N("Subtitle"),                ff, 14, false, false, false, AlignmentFlags::HCENTER | AlignmentFlags::TOP,      QPointF(0, MM(10)), OffsetType::ABS));
      s->addTextStyle(TextStyle(N("Composer"),                ff, 12, false, false, false, AlignmentFlags::RIGHT   | AlignmentFlags::BOTTOM,   QPointF(), OffsetType::ABS));
      s->addTextStyle(TextStyle(N("Lyricist"),                ff, 12, false, false, false, AlignmentFlags::LEFT    | AlignmentFlags::BOTTOM,   QPointF(), OffsetType::ABS));
      s->addTextStyle(TextStyle(N("Lyrics Odd Lines"),        ff, 11, false, false, false, AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0, 6), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(N("Lyrics Even Lines"),       ff, 11, false, false, false, AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0, 6), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(N("Fingering"),               ff,  8, false, false, false, AlignmentFlags::CENTER,                             QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(N("LH Guitar Fingering"),     ff,  8, false, false, false, AlignmentFlags::RIGHT   | AlignmentFlags::VCENTER,  QPointF(-0.5, 0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(N("RH Guitar Fingering"),     ff,  8, false, false, false, AlignmentFlags::CENTER,                             QPointF(),        OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(N("String Number"),           ff,  8, false, false, false, AlignmentFlags::CENTER,                             QPointF(0, -2.0), OffsetType::SPATIUM, true, true, true, Spatium(.1), Spatium(.2), 0, Qt::black, true, false));
      s->addTextStyle(TextStyle(N("Instrument Name (Long)"),  ff, 12, false, false, false, AlignmentFlags::RIGHT | AlignmentFlags::VCENTER,    QPointF(),        OffsetType::ABS, true));
      s->addTextStyle(TextStyle(N("Instrument Name (Short)"), ff, 12, false, false, false, AlignmentFlags::RIGHT | AlignmentFlags::VCENTER,    QPointF(),        OffsetType::ABS, true));
      s->addTextStyle(TextStyle(N("Instrument Name (Part)"),  ff, 18, false, false, false, LT, QPointF(),        OffsetType::ABS));

      // dynamics size is 12pt for bravura-text
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Dynamics"),  ff,  12, false, true, false, AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0.0, 8.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Expression"), ff, 11, false, true, false, AlignmentFlags::LEFT    | AlignmentFlags::BASELINE, QPointF(0, -6.0),   OffsetType::SPATIUM, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Tempo"), ff, 12, true, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0, -4.0), OffsetType::SPATIUM,
         true, false, false, Spatium(.2), Spatium(.5), 0, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Metronome"),      ff, 12, true, false, false,
         AlignmentFlags::LEFT));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Measure Number"), ff, 8, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BOTTOM, QPointF(.0, -2.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Translator"),     ff, 11, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::TOP, QPointF(0, 6)));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Tuplet"),         ff, 10, false, true, false,
         AlignmentFlags::CENTER, QPointF(), OffsetType::ABS, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "System"), ff,  10, false, false, false,
         AlignmentFlags::LEFT, QPointF(0, -4.0), OffsetType::SPATIUM, true, false,
         false, Spatium(.2), Spatium(.5), 25, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Staff"),        ff,  10, false, false, false,
         AlignmentFlags::LEFT, QPointF(0, -4.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Chord Symbol"), ff,  12, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Rehearsal Mark"), ff,  14,
         true,
         false,
         false,
         AlignmentFlags::HCENTER | AlignmentFlags::BASELINE,
         QPointF(0, -3.0), OffsetType::SPATIUM,
         true,
         true,    // hasFrame
         true,    // square
         Spatium(.2), Spatium(.5), 0, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Repeat Text Left"), ff,  20, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0, -2.0), OffsetType::SPATIUM, true, false,
         false, Spatium(.2), Spatium(.5), 25, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Repeat Text Right"), ff,  12, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::BASELINE, QPointF(0, -2.0), OffsetType::SPATIUM, true, false,
         false, Spatium(0.2), Spatium(0.5), 25, Qt::black, false, true));

      // y offset may depend on voltaHook style element
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Volta"),     ff, 11, true, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0.5, 1.9), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Frame"),     ff, 12, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::TOP));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Text Line"), ff, 12, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::VCENTER, QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Glissando"), ff, 8, false, true, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Ottava"),            ff, 12, false, true, false,
         AlignmentFlags::LEFT | AlignmentFlags::VCENTER, QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Pedal"),             ff, 12, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0.0, 0.15), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Hairpin"),           ff, 12, false, true, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Bend"),              ff, 8, false, false, false,
         AlignmentFlags::CENTER | AlignmentFlags::BOTTOM, QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Header"),            ff, 8, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::TOP));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Footer"),            ff, 8, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BOTTOM, QPointF(0.0, MM(5)), OffsetType::ABS));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Instrument Change"), ff,  12, true, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BOTTOM, QPointF(0, -3.0), OffsetType::SPATIUM, true));

/*      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Lyrics Verse Number"),ff, 11, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, true));
*/
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Figured Bass"), "MScoreBC", 8, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::TOP, QPointF(0, 6), OffsetType::SPATIUM, true, false,
         false, Spatium(0.0), Spatium(0.0), 25, QColor(Qt::black), false,      // default params
         false, QColor(Qt::black), QColor(255, 255, 255, 0),                   // default params
         TextStyleHidden::IN_EDITOR));                                         // don't show in Style Editor

#undef MM
#undef N

#ifndef NDEBUG
      int i = 0;
      for (const StyleType& t : styleTypes) {
            Q_ASSERT(t.idx() == i);
            ++i;
            }
#endif
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

MStyle::MStyle()
   : _values(int(StyleIdx::STYLES)), _precomputedValues(int(StyleIdx::STYLES))
      {
      _customChordList = false;
      for (const StyleType& t : styleTypes)
            _values[t.idx()] = t.defaultValue();
      precomputeValues();
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
      return _values[int(idx)] == MScore::baseStyle()->value(idx);
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
//   MStyle
//---------------------------------------------------------

MStyle::MStyle(const MStyle& s)
      {
      _values            = s._values;
      _precomputedValues = s._precomputedValues;
      _chordList         = s._chordList;
      _textStyles        = s._textStyles;
      _pageFormat.copy(s._pageFormat);
      _customChordList   = s._customChordList;
      }

MStyle& MStyle::operator=(const MStyle& s)
      {
      _values            = s._values;
      _precomputedValues = s._precomputedValues;
      _chordList         = s._chordList;
      _textStyles        = s._textStyles;
      _pageFormat.copy(s._pageFormat);
      _customChordList   = s._customChordList;
      return *this;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

const TextStyle& MStyle::textStyle(TextStyleType idx) const
      {
      Q_ASSERT(int(idx) >= 0 && int(idx) < _textStyles.count());
      return _textStyles[int(idx)];
      }

TextStyle& MStyle::textStyle(TextStyleType idx)
      {
      Q_ASSERT(int(idx) >= 0 && int(idx) < _textStyles.count());
      return _textStyles[int(idx)];
      }

const TextStyle& MStyle::textStyle(const QString& name) const
      {
      for (const TextStyle& s : _textStyles) {
            if (s.name() == name)
                  return s;
            }
      qFatal("TextStyle <%s> not found", qPrintable(name));
      return _textStyles[0];
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

TextStyleType MStyle::textStyleType(const QString& name) const
      {
      for (int i = 0; i < _textStyles.size(); ++i) {
            if (_textStyles[i].name() == name)
                  return TextStyleType(i);
            }
      qDebug("TextStyleType <%s> not found", qPrintable(name));
      return TextStyleType::DEFAULT;
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void MStyle::setTextStyle(const TextStyle& ts)
      {
      for (int i = 0; i < _textStyles.size(); ++i) {
            if (_textStyles[i].name() == ts.name()) {
                  _textStyles[i] = ts;
                  return;
                  }
            }
      _textStyles.append(ts);
      }

//---------------------------------------------------------
//   addTextStyle
//---------------------------------------------------------

void MStyle::addTextStyle(const TextStyle& ts)
      {
      _textStyles.append(ts);
      }

//---------------------------------------------------------
//   removeTextStyle
//---------------------------------------------------------

void MStyle::removeTextStyle(const TextStyle& /*ts*/)
      {
      // TODO
      }

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

const QList<TextStyle>& MStyle::textStyles() const
      {
      return _textStyles;
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
//   convertToUnit
//---------------------------------------------------------

void MStyle::convertToUnit(const QString& tag, const QString& val)
      {
      for (const StyleType& t : styleTypes) {
            StyleIdx idx = t.styleIdx();
            if (t.name() == tag) {
                  const char* type = t.valueType();
                  if (!strcmp("Ms::Spatium", type))
                        set(idx, Spatium(val.toDouble()));
                  else if (!strcmp("double", type))
                        set(idx, QVariant(val.toDouble()));
                  else if (!strcmp("bool", type))
                        set(idx, QVariant(bool(val.toInt())));
                  else if (!strcmp("int", type))
                        set(idx, QVariant(val.toInt()));
                  else if (!strcmp("Ms::Direction", type))
                        set(idx, QVariant::fromValue(Direction(val.toInt())));
                  else if (!strcmp("QString", type))
                        set(idx, QVariant(val));
                  else
                        qFatal("MStyle::load: unhandled type %s", type);
                  return;
                  }
            }
      qDebug("MStyle::load: unhandled style value %s", qPrintable(tag));
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

void MStyle::load(XmlReader& e)
      {
      QString oldChordDescriptionFile = value(StyleIdx::chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();

            if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(e);
                  setTextStyle(s);
                  }
            else if (tag == "Spatium")
                  set(StyleIdx::spatium, e.readDouble() * DPMM);
            else if (tag == "page-layout")
                  _pageFormat.read(e);
            else if (tag == "displayInConcertPitch")
                  set(StyleIdx::concertPitch, QVariant(bool(e.readInt())));
            else if (tag == "ChordList") {
                  _chordList.clear();
                  _chordList.read(e);
                  _customChordList = true;
                  chordListTag = true;
                  }
            else
                  convertToUnit(tag, e.readElementText());
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

void MStyle::save(Xml& xml, bool optimize)
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
            else if (!strcmp("double", type))
                  xml.tag(st.name(), value(idx).toDouble());
            else if (!strcmp("bool", type))
                  xml.tag(st.name(), value(idx).toInt());
            else if (!strcmp("int", type))
                  xml.tag(st.name(), value(idx).toInt());
            else if (!strcmp("Ms::Direction", type))
                  xml.tag(st.name(), value(idx).toInt());
            else if (!strcmp("QString", type))
                  xml.tag(st.name(), value(idx).toString());
            else
                  qFatal("bad style type");
            }
      for (int i = 0; i < int(TextStyleType::TEXT_STYLES); ++i) {
            if (!optimize || _textStyles[i] != MScore::baseStyle()->textStyle(TextStyleType(i)))
                  _textStyles[i].write(xml);
            }
      for (int i = int(TextStyleType::TEXT_STYLES); i < _textStyles.size(); ++i)
            _textStyles[i].write(xml);
      if (_customChordList && !_chordList.empty()) {
            xml.stag("ChordList");
            _chordList.write(xml);
            xml.etag();
            }
      if (!MScore::saveTemplateMode || (_pageFormat.name() != "A4" && _pageFormat.name() != "Letter"))
            _pageFormat.write(xml);
      xml.tag("Spatium", value(StyleIdx::spatium).toDouble() / DPMM);
      xml.etag();
      }

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

void MStyle::setPageFormat(const PageFormat& pf)
      {
      _pageFormat.copy(pf);
      }

}
