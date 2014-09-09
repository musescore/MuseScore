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

#include <unordered_map>
#include "mscore.h"
#include "style.h"
#include "style_p.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "chordlist.h"
#include "page.h"
#include "mscore.h"
#include "clef.h"

namespace Ms {

MStyle* style;

//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

//---------------------------------------------------------
//   styleTypes
//---------------------------------------------------------

struct StyleTypes2 {
      StyleIdx idx;
      StyleType val;
      };

static const StyleTypes2 styleTypes2[] = {
      { StyleIdx::staffUpperBorder,            StyleType("staffUpperBorder",        StyleValueType::SPATIUM) },
      { StyleIdx::staffLowerBorder,            StyleType("staffLowerBorder",        StyleValueType::SPATIUM) },
      { StyleIdx::staffDistance,               StyleType("staffDistance",           StyleValueType::SPATIUM) },
      { StyleIdx::akkoladeDistance,            StyleType("akkoladeDistance",        StyleValueType::SPATIUM) },
      { StyleIdx::minSystemDistance,           StyleType("minSystemDistance",       StyleValueType::SPATIUM) },
      { StyleIdx::maxSystemDistance,           StyleType("maxSystemDistance",       StyleValueType::SPATIUM) },
      { StyleIdx::lyricsDistance,              StyleType("lyricsDistance",          StyleValueType::SPATIUM) },
      { StyleIdx::lyricsMinBottomDistance,     StyleType("lyricsMinBottomDistance", StyleValueType::SPATIUM) },
      { StyleIdx::lyricsLineHeight,            StyleType("lyricsLineHeight",        StyleValueType::DOUBLE) },      // in % of normal height (default: 1.0)
      { StyleIdx::figuredBassFontFamily,       StyleType("figuredBassFontFamily",   StyleValueType::STRING) },
      { StyleIdx::figuredBassFontSize,         StyleType("figuredBassFontSize",     StyleValueType::DOUBLE) },      // in pt
      { StyleIdx::figuredBassYOffset,          StyleType("figuredBassYOffset",      StyleValueType::DOUBLE) },      // in sp
      { StyleIdx::figuredBassLineHeight,       StyleType("figuredBassLineHeight",   StyleValueType::DOUBLE) },      // in % of normal height
      { StyleIdx::figuredBassAlignment,        StyleType("figuredBassAlignment",    StyleValueType::INT) },         // 0 = top, 1 = bottom
      { StyleIdx::figuredBassStyle,            StyleType("figuredBassStyle" ,       StyleValueType::INT) },         // 0=modern, 1=historic
      { StyleIdx::systemFrameDistance,         StyleType("systemFrameDistance",     StyleValueType::SPATIUM) },     // dist. between staff and vertical box
      { StyleIdx::frameSystemDistance,         StyleType("frameSystemDistance",     StyleValueType::SPATIUM) },     // dist. between vertical box and next system
      { StyleIdx::minMeasureWidth,             StyleType("minMeasureWidth",         StyleValueType::SPATIUM) },
      { StyleIdx::barWidth,                    StyleType("barWidth",                StyleValueType::SPATIUM) },
      { StyleIdx::doubleBarWidth,              StyleType("doubleBarWidth",          StyleValueType::SPATIUM) },
      { StyleIdx::endBarWidth,                 StyleType("endBarWidth",             StyleValueType::SPATIUM) },
      { StyleIdx::doubleBarDistance,           StyleType("doubleBarDistance",       StyleValueType::SPATIUM) },
      { StyleIdx::endBarDistance,              StyleType("endBarDistance",          StyleValueType::SPATIUM) },
      { StyleIdx::repeatBarTips,               StyleType("repeatBarTips",           StyleValueType::BOOL) },
      { StyleIdx::startBarlineSingle,          StyleType("startBarlineSingle",      StyleValueType::BOOL) },
      { StyleIdx::startBarlineMultiple,        StyleType("startBarlineMultiple",    StyleValueType::BOOL) },
      { StyleIdx::bracketWidth,                StyleType("bracketWidth",            StyleValueType::SPATIUM) },     // system bracket line width
      { StyleIdx::bracketDistance,             StyleType("bracketDistance",         StyleValueType::SPATIUM) },     // system bracket distance
      { StyleIdx::akkoladeWidth,               StyleType("akkoladeWidth",           StyleValueType::SPATIUM) },
      { StyleIdx::akkoladeBarDistance,         StyleType("akkoladeBarDistance",     StyleValueType::SPATIUM) },
      { StyleIdx::clefLeftMargin,              StyleType("clefLeftMargin",          StyleValueType::SPATIUM) },
      { StyleIdx::keysigLeftMargin,            StyleType("keysigLeftMargin",        StyleValueType::SPATIUM) },
      { StyleIdx::timesigLeftMargin,           StyleType("timesigLeftMargin",       StyleValueType::SPATIUM) },
      { StyleIdx::clefKeyRightMargin,          StyleType("clefKeyRightMargin",      StyleValueType::SPATIUM) },
      { StyleIdx::clefBarlineDistance,         StyleType("clefBarlineDistance",     StyleValueType::SPATIUM) },
      { StyleIdx::stemWidth,                   StyleType("stemWidth",               StyleValueType::SPATIUM) },
      { StyleIdx::shortenStem,                 StyleType("shortenStem",             StyleValueType::BOOL) },        // StyleIdx::shortenStem,
      { StyleIdx::shortStemProgression,        StyleType("shortStemProgression",    StyleValueType::SPATIUM) },     // StyleIdx::shortStemProgression,
      { StyleIdx::shortestStem,                StyleType("shortestStem",            StyleValueType::SPATIUM) },
      { StyleIdx::beginRepeatLeftMargin,       StyleType("beginRepeatLeftMargin",   StyleValueType::SPATIUM) },
      { StyleIdx::minNoteDistance,             StyleType("minNoteDistance",         StyleValueType::SPATIUM) },
      { StyleIdx::barNoteDistance,             StyleType("barNoteDistance",         StyleValueType::SPATIUM) },
      { StyleIdx::barAccidentalDistance,       StyleType("barAccidentalDistance",   StyleValueType::SPATIUM) },
      { StyleIdx::multiMeasureRestMargin,      StyleType("multiMeasureRestMargin",  StyleValueType::SPATIUM) },
      { StyleIdx::noteBarDistance,             StyleType("noteBarDistance",         StyleValueType::SPATIUM) },
      { StyleIdx::measureSpacing,              StyleType("measureSpacing",          StyleValueType::DOUBLE) },
      { StyleIdx::staffLineWidth,              StyleType("staffLineWidth",          StyleValueType::SPATIUM) },
      { StyleIdx::ledgerLineWidth,             StyleType("ledgerLineWidth",         StyleValueType::SPATIUM) },
      { StyleIdx::ledgerLineLength,            StyleType("ledgerLineLength",        StyleValueType::SPATIUM) },
      { StyleIdx::accidentalDistance,          StyleType("accidentalDistance",      StyleValueType::SPATIUM) },
      { StyleIdx::accidentalNoteDistance,      StyleType("accidentalNoteDistance",  StyleValueType::SPATIUM) },
      { StyleIdx::beamWidth,                   StyleType("beamWidth",               StyleValueType::SPATIUM) },
      { StyleIdx::beamDistance,                StyleType("beamDistance",            StyleValueType::DOUBLE)  },     // in beamWidth units
      { StyleIdx::beamMinLen,                  StyleType("beamMinLen",              StyleValueType::SPATIUM) },     // len for broken beams
      { StyleIdx::beamNoSlope,                 StyleType("beamNoSlope",             StyleValueType::BOOL)    },
      { StyleIdx::dotMag,                      StyleType("dotMag",                  StyleValueType::DOUBLE)  },
      { StyleIdx::dotNoteDistance,             StyleType("dotNoteDistance",         StyleValueType::SPATIUM) },
      { StyleIdx::dotRestDistance,             StyleType("dotRestDistance",         StyleValueType::SPATIUM) },
      { StyleIdx::dotDotDistance,              StyleType("dotDotDistance",          StyleValueType::SPATIUM) },
      { StyleIdx::propertyDistanceHead,        StyleType("propertyDistanceHead",    StyleValueType::SPATIUM) },     // note property to note head
      { StyleIdx::propertyDistanceStem,        StyleType("propertyDistanceStem",    StyleValueType::SPATIUM) },     // note property to note stem
      { StyleIdx::propertyDistance,            StyleType("propertyDistance",        StyleValueType::SPATIUM) },     // note property to note property
      { StyleIdx::articulationMag,             StyleType("articulationMag",         StyleValueType::DOUBLE) },     // note property to note property
      { StyleIdx::lastSystemFillLimit,         StyleType("lastSystemFillLimit",     StyleValueType::DOUBLE) },
      { StyleIdx::hairpinY,                    StyleType("hairpinY",                StyleValueType::SPATIUM) },
      { StyleIdx::hairpinHeight,               StyleType("hairpinHeight",           StyleValueType::SPATIUM) },
      { StyleIdx::hairpinContHeight,           StyleType("hairpinContHeight",       StyleValueType::SPATIUM) },
      { StyleIdx::hairpinLineWidth,            StyleType("hairpinWidth",            StyleValueType::SPATIUM) },
      { StyleIdx::pedalY,                      StyleType("pedalY",                  StyleValueType::SPATIUM) },
      { StyleIdx::pedalLineWidth,              StyleType("pedalLineWidth",          StyleValueType::SPATIUM) },
      { StyleIdx::pedalLineStyle,              StyleType("pedalListStyle",          StyleValueType::INT)     },
      { StyleIdx::trillY,                      StyleType("trillY",                  StyleValueType::SPATIUM) },
      { StyleIdx::harmonyY,                    StyleType("harmonyY",                StyleValueType::SPATIUM) },
      { StyleIdx::harmonyFretDist,             StyleType("harmonyFretDist",         StyleValueType::SPATIUM) },
      { StyleIdx::minHarmonyDistance,          StyleType("minHarmonyDistance",      StyleValueType::SPATIUM) },
      { StyleIdx::maxHarmonyBarDistance,       StyleType("maxHarmonyBarDistance",   StyleValueType::SPATIUM) },
      { StyleIdx::capoPosition,                StyleType("capoPosition",            StyleValueType::INT) },
      { StyleIdx::fretNumMag,                  StyleType("fretNumMag",              StyleValueType::DOUBLE) },
      { StyleIdx::fretNumPos,                  StyleType("fretNumPos",              StyleValueType::INT) },  // 0 = left, 1 = right
      { StyleIdx::fretY,                       StyleType("fretY",                   StyleValueType::SPATIUM) },
      { StyleIdx::showPageNumber,              StyleType("showPageNumber",          StyleValueType::BOOL) },
      { StyleIdx::showPageNumberOne,           StyleType("showPageNumberOne",       StyleValueType::BOOL) },
      { StyleIdx::pageNumberOddEven,           StyleType("pageNumberOddEven",       StyleValueType::BOOL) },
      { StyleIdx::showMeasureNumber,           StyleType("showMeasureNumber",       StyleValueType::BOOL) },
      { StyleIdx::showMeasureNumberOne,        StyleType("showMeasureNumberOne",    StyleValueType::BOOL) },
      { StyleIdx::measureNumberInterval,       StyleType("measureNumberInterval",   StyleValueType::INT) },
      { StyleIdx::measureNumberSystem,         StyleType("measureNumberSystem",     StyleValueType::BOOL) },
      { StyleIdx::measureNumberAllStaffs,      StyleType("measureNumberAllStaffs",  StyleValueType::BOOL) },
      { StyleIdx::smallNoteMag,                StyleType("smallNoteMag",            StyleValueType::DOUBLE) },
      { StyleIdx::graceNoteMag,                StyleType("graceNoteMag",            StyleValueType::DOUBLE) },
      { StyleIdx::smallStaffMag,               StyleType("smallStaffMag",           StyleValueType::DOUBLE) },
      { StyleIdx::smallClefMag,                StyleType("smallClefMag",            StyleValueType::DOUBLE) },
      { StyleIdx::genClef,                     StyleType("genClef",                 StyleValueType::BOOL) },           // create clef for all systems, not only for first
      { StyleIdx::genKeysig,                   StyleType("genKeysig",               StyleValueType::BOOL) },         // create key signature for all systems
      { StyleIdx::genTimesig,                  StyleType("genTimesig",              StyleValueType::BOOL) },
      { StyleIdx::genCourtesyTimesig,          StyleType("genCourtesyTimesig",      StyleValueType::BOOL) },
      { StyleIdx::genCourtesyKeysig,           StyleType("genCourtesyKeysig",       StyleValueType::BOOL) },
      { StyleIdx::genCourtesyClef,             StyleType("genCourtesyClef",         StyleValueType::BOOL) },
      { StyleIdx::swingRatio,                  StyleType("swingRatio",              StyleValueType::INT)  },
      { StyleIdx::swingUnit,                   StyleType("swingUnit",               StyleValueType::STRING)  },
      { StyleIdx::useStandardNoteNames,        StyleType("useStandardNoteNames",    StyleValueType::BOOL) },
      { StyleIdx::useGermanNoteNames,          StyleType("useGermanNoteNames",      StyleValueType::BOOL) },
      { StyleIdx::useSolfeggioNoteNames,       StyleType("useSolfeggioNoteNames",   StyleValueType::BOOL) },
      { StyleIdx::lowerCaseMinorChords,        StyleType("lowerCaseMinorChords",    StyleValueType::BOOL) },
      { StyleIdx::chordStyle,                  StyleType("chordStyle",              StyleValueType::STRING) },
      { StyleIdx::chordsXmlFile,               StyleType("chordsXmlFile",           StyleValueType::BOOL) },
      { StyleIdx::chordDescriptionFile,        StyleType("chordDescriptionFile",    StyleValueType::STRING) },
      { StyleIdx::concertPitch,                StyleType("concertPitch",            StyleValueType::BOOL) },            // display transposing instruments in concert pitch
      { StyleIdx::createMultiMeasureRests,     StyleType("createMultiMeasureRests", StyleValueType::BOOL) },
      { StyleIdx::minEmptyMeasures,            StyleType("minEmptyMeasures",        StyleValueType::INT) },         // minimum number of empty measures for multi measure rest
      { StyleIdx::minMMRestWidth,              StyleType("minMMRestWidth",          StyleValueType::SPATIUM) },       // minimum width of multi measure rest
      { StyleIdx::hideEmptyStaves,             StyleType("hideEmptyStaves",         StyleValueType::BOOL) },
      { StyleIdx::dontHideStavesInFirstSystem, StyleType("dontHidStavesInFirstSystm", StyleValueType::BOOL) },
      { StyleIdx::hideInstrumentNameIfOneInstrument,  StyleType("hideInstrumentNameIfOneInstrument", StyleValueType::BOOL) },
      { StyleIdx::gateTime,                    StyleType("gateTime",                StyleValueType::INT) },           // 0-100%
      { StyleIdx::tenutoGateTime,              StyleType("tenutoGateTime",          StyleValueType::INT) },
      { StyleIdx::staccatoGateTime,            StyleType("staccatoGateTime",        StyleValueType::INT) },
      { StyleIdx::slurGateTime,                StyleType("slurGateTime",            StyleValueType::INT) },
      { StyleIdx::ArpeggioNoteDistance,        StyleType("ArpeggioNoteDistance",    StyleValueType::SPATIUM) },
      { StyleIdx::ArpeggioLineWidth,           StyleType("ArpeggioLineWidth",       StyleValueType::SPATIUM) },
      { StyleIdx::ArpeggioHookLen,             StyleType("ArpeggioHookLen",         StyleValueType::SPATIUM) },
      { StyleIdx::FixMeasureNumbers,           StyleType("FixMeasureNumbers",       StyleValueType::INT) },
      { StyleIdx::FixMeasureWidth,             StyleType("FixMeasureWidth",         StyleValueType::BOOL) },
      { StyleIdx::SlurEndWidth,                StyleType("slurEndWidth",            StyleValueType::SPATIUM) },
      { StyleIdx::SlurMidWidth,                StyleType("slurMidWidth",            StyleValueType::SPATIUM) },
      { StyleIdx::SlurDottedWidth,             StyleType("slurDottedWidth",         StyleValueType::SPATIUM) },
      { StyleIdx::MinTieLength,                StyleType("minTieLength",            StyleValueType::SPATIUM) },
      { StyleIdx::SectionPause,                StyleType("sectionPause",            StyleValueType::DOUBLE) },
      { StyleIdx::MusicalSymbolFont,           StyleType("musicalSymbolFont",       StyleValueType::STRING) },
      { StyleIdx::MusicalTextFont,             StyleType("musicalTextFont",         StyleValueType::STRING) },
      { StyleIdx::showHeader,                  StyleType("showHeader",              StyleValueType::BOOL) },
//      { StyleIdx::headerStyled,                StyleType("headerStyled",            StyleValueType::BOOL) },
      { StyleIdx::headerFirstPage,             StyleType("headerFirstPage",         StyleValueType::BOOL) },
      { StyleIdx::headerOddEven,               StyleType("headerOddEven",           StyleValueType::BOOL) },
      { StyleIdx::evenHeaderL,                 StyleType("evenHeaderL",             StyleValueType::STRING) },
      { StyleIdx::evenHeaderC,                 StyleType("evenHeaderC",             StyleValueType::STRING) },
      { StyleIdx::evenHeaderR,                 StyleType("evenHeaderR",             StyleValueType::STRING) },
      { StyleIdx::oddHeaderL,                  StyleType("oddHeaderL",              StyleValueType::STRING) },
      { StyleIdx::oddHeaderC,                  StyleType("oddHeaderC",              StyleValueType::STRING) },
      { StyleIdx::oddHeaderR,                  StyleType("oddHeaderR",              StyleValueType::STRING) },
      { StyleIdx::showFooter,                  StyleType("showFooter",              StyleValueType::BOOL) },
//      { StyleIdx::footerStyled,                StyleType("footerStyled",            StyleValueType::BOOL) },
      { StyleIdx::footerFirstPage,             StyleType("footerFirstPage",         StyleValueType::BOOL) },
      { StyleIdx::footerOddEven,               StyleType("footerOddEven",           StyleValueType::BOOL) },
      { StyleIdx::evenFooterL,                 StyleType("evenFooterL",             StyleValueType::STRING) },
      { StyleIdx::evenFooterC,                 StyleType("evenFooterC",             StyleValueType::STRING) },
      { StyleIdx::evenFooterR,                 StyleType("evenFooterR",             StyleValueType::STRING) },
      { StyleIdx::oddFooterL,                  StyleType("oddFooterL",              StyleValueType::STRING) },
      { StyleIdx::oddFooterC,                  StyleType("oddFooterC",              StyleValueType::STRING) },
      { StyleIdx::oddFooterR,                  StyleType("oddFooterR",              StyleValueType::STRING) },
      { StyleIdx::voltaY,                      StyleType("voltaY",                  StyleValueType::SPATIUM) },
      { StyleIdx::voltaHook,                   StyleType("voltaHook",               StyleValueType::SPATIUM) },
      { StyleIdx::voltaLineWidth,              StyleType("voltaLineWidth",          StyleValueType::SPATIUM) },
      { StyleIdx::voltaLineStyle,              StyleType("voltaLineStyle",          StyleValueType::INT) },
      { StyleIdx::ottavaY,                     StyleType("ottavaY",                 StyleValueType::SPATIUM) },
      { StyleIdx::ottavaHook,                  StyleType("ottavaHook",              StyleValueType::SPATIUM) },
      { StyleIdx::ottavaLineWidth,             StyleType("ottavaLineWidth",         StyleValueType::SPATIUM) },
      { StyleIdx::ottavaLineStyle,             StyleType("ottavaLineStyle",         StyleValueType::INT) },
      { StyleIdx::ottavaNumbersOnly,           StyleType("ottavaNumbersOnly",       StyleValueType::BOOL) },
      { StyleIdx::tabClef,                     StyleType("tabClef",                 StyleValueType::INT) },
      { StyleIdx::tremoloWidth,                StyleType("tremoloWidth",            StyleValueType::SPATIUM) },
      { StyleIdx::tremoloBoxHeight,            StyleType("tremoloBoxHeight",        StyleValueType::SPATIUM) },
      { StyleIdx::tremoloStrokeWidth,          StyleType("tremoloLineWidth",        StyleValueType::SPATIUM) },
      { StyleIdx::tremoloDistance,             StyleType("tremoloDistance",         StyleValueType::SPATIUM) },
      { StyleIdx::linearStretch,               StyleType("linearStretch",           StyleValueType::DOUBLE) },
      { StyleIdx::crossMeasureValues,          StyleType("crossMeasureValues",      StyleValueType::BOOL) },
      { StyleIdx::keySigNaturals,              StyleType("keySigNaturals",          StyleValueType::INT) },
      { StyleIdx::tupletMaxSlope,              StyleType("tupletMaxSlope",          StyleValueType::DOUBLE) },
      { StyleIdx::tupletOufOfStaff,            StyleType("tupletOufOfStaff",        StyleValueType::BOOL) },
      { StyleIdx::tupletVHeadDistance,         StyleType("tupletVHeadDistance",     StyleValueType::SPATIUM) },
      { StyleIdx::tupletVStemDistance,         StyleType("tupletVStemDistance",     StyleValueType::SPATIUM) },
      { StyleIdx::tupletStemLeftDistance,      StyleType("tupletStemLeftDistance",  StyleValueType::SPATIUM) },
      { StyleIdx::tupletStemRightDistance,     StyleType("tupletStemRightDistance", StyleValueType::SPATIUM) },
      { StyleIdx::tupletNoteLeftDistance,      StyleType("tupletNoteLeftDistance",  StyleValueType::SPATIUM) },
      { StyleIdx::tupletNoteRightDistance,     StyleType("tupletNoteRightDistance", StyleValueType::SPATIUM) }
      };

class StyleTypes {
      StyleType st[int(StyleIdx::STYLES)];

   public:
      StyleTypes()
            {
            for (auto i : styleTypes2)
                  st[int(i.idx)] = i.val;
            };
      const char* name(StyleIdx i) const { return st[int(i)].name(); }
      StyleValueType valueType(StyleIdx i) const { return st[int(i)].valueType(); }
      };

static const StyleTypes styleTypes;

static const QString ff("FreeSerif");

//---------------------------------------------------------
//   setDefaultStyle
//    synchronize with TextStyleType
//---------------------------------------------------------

void initStyle(MStyle* s)
      {
#define MM(x) ((x)/INCH)

      // this is an empty style, no offsets are allowed
      // never show this style
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", ""), ff, 10, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, false,
         false, Spatium(.2), Spatium(.5), 25, QColor(Qt::black), false, false, QColor(Qt::black),
         QColor(255, 255, 255, 0), TextStyleHidden::ALWAYS));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Title"),    ff, 24, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::TOP,    QPointF(), OffsetType::ABS));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Subtitle"), ff, 14, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::TOP,    QPointF(0, MM(10)), OffsetType::ABS));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Composer"), ff, 12, false, false, false,
         AlignmentFlags::RIGHT   | AlignmentFlags::BOTTOM, QPointF(), OffsetType::ABS));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Lyricist"), ff, 12, false, false, false,
         AlignmentFlags::LEFT    | AlignmentFlags::BOTTOM, QPointF(), OffsetType::ABS));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Lyrics Odd Lines"),          ff, 11, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0, 6), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Lyrics Even Lines"),         ff, 11, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0, 6), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Fingering"),                 ff,  8, false, false, false,
         AlignmentFlags::CENTER, QPointF(), OffsetType::ABS, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Instrument Name (Long)"),    ff, 12, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::VCENTER, QPointF(), OffsetType::ABS, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Instrument Name (Short)"),   ff, 12, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::VCENTER, QPointF(), OffsetType::ABS, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Instrument Name (Part)"),    ff, 18, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BOTTOM, QPointF(), OffsetType::ABS));

      // dynamics size is 12pt for bravura-text
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Dynamics"),  ff, 12, false, false,false,
         AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0.0, 8.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Technique"), ff, 12, false, true, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0.0, -2.0), OffsetType::SPATIUM));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Tempo"), ff, 12, true, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0, -4.0), OffsetType::SPATIUM,
         true, false, Spatium(.2), Spatium(.5), 0, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Metronome"),      ff, 12, true, false, false,
         AlignmentFlags::LEFT));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Measure Number"), ff, 8, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BOTTOM, QPointF(.0, -2.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Translator"),     ff, 11, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::TOP, QPointF(0, 6)));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Tuplet"),         ff,  10, false, true, false,
         AlignmentFlags::CENTER, QPointF(), OffsetType::ABS, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "System"), ff,  10, false, false, false,
         AlignmentFlags::LEFT, QPointF(0, -4.0), OffsetType::SPATIUM, true,
         false, Spatium(.2), Spatium(.5), 25, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Staff"),        ff,  10, false, false, false,
         AlignmentFlags::LEFT, QPointF(0, -4.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Chord Symbol"), ff,  12, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(), OffsetType::SPATIUM, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Rehearsal Mark"), ff,  14, true, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BASELINE, QPointF(0, -3.0), OffsetType::SPATIUM, true,
         true, Spatium(.2), Spatium(.5), 20, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Repeat Text Left"), ff,  20, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BASELINE, QPointF(0, -2.0), OffsetType::SPATIUM, true,
         false, Spatium(.2), Spatium(.5), 25, Qt::black, false, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Repeat Text Right"), ff,  12, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::BASELINE, QPointF(0, -2.0), OffsetType::SPATIUM, true,
         false, Spatium(0.2), Spatium(0.5), 25, Qt::black, false, true));

      // for backward compatibility
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Repeat Text"), ff,  12, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::BASELINE, QPointF(0, -2.0), OffsetType::SPATIUM, true,
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

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "String Number"), ff,  8, false, false, false,
         AlignmentFlags::CENTER, QPointF(0, -5.0), OffsetType::SPATIUM, true,
         true, Spatium(.1), Spatium(.2), 0, Qt::black, true, false));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Ottava"),            ff, 12, false, true, false,
         AlignmentFlags::LEFT | AlignmentFlags::VCENTER, QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Bend"),              ff, 8, false, false, false,
         AlignmentFlags::CENTER | AlignmentFlags::BOTTOM, QPointF(), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Header"),            ff, 8, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::TOP));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Footer"),            ff, 8, false, false, false,
         AlignmentFlags::HCENTER | AlignmentFlags::BOTTOM, QPointF(0.0, MM(5)), OffsetType::ABS));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Instrument Change"), ff,  12, true, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::BOTTOM, QPointF(0, -3.0), OffsetType::SPATIUM, true));
      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Lyrics Verse"),      ff, 11, false, false, false,
         AlignmentFlags::RIGHT | AlignmentFlags::TOP, QPointF(), OffsetType::SPATIUM, true));

      s->addTextStyle(TextStyle(QT_TRANSLATE_NOOP ("TextStyle", "Figured Bass"), "MScoreBC", 8, false, false, false,
         AlignmentFlags::LEFT | AlignmentFlags::TOP, QPointF(0, 6), OffsetType::SPATIUM, true,
         false, Spatium(0.0), Spatium(0.0), 25, QColor(Qt::black), false,      // default params
         false, QColor(Qt::black), QColor(255, 255, 255, 0),                   // default params
         TextStyleHidden::IN_EDITOR));                                         // don't show in Style Editor

#undef MM
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::StyleData()
   : _values(int(StyleIdx::STYLES))
      {
      _customChordList = false;

      struct StyleVal2 {
            StyleIdx idx;
            QVariant val;
            };
      static const StyleVal2 values2[] = {
            { StyleIdx::staffUpperBorder,            7.0  },
            { StyleIdx::staffLowerBorder,            7.0  },
            { StyleIdx::staffDistance,               6.5  },
            { StyleIdx::akkoladeDistance,            6.5  },
            { StyleIdx::minSystemDistance,           8.5  },
            { StyleIdx::maxSystemDistance,           15.0 },
            { StyleIdx::lyricsDistance,              2.0  },
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
            { StyleIdx::ledgerLineLength,            QVariant(.6) },     // note head width + this value
            { StyleIdx::accidentalDistance,          QVariant(0.22) },
            { StyleIdx::accidentalNoteDistance,      QVariant(0.22) },
            { StyleIdx::beamWidth,                   QVariant(0.5) },           // was 0.48
            { StyleIdx::beamDistance,                QVariant(0.5) },          // 0.25sp
            { StyleIdx::beamMinLen,                  QVariant(1.32) },      // 1.316178 exactly note head width
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
            { StyleIdx::genTimesig,                  QVariant(true) },
            { StyleIdx::genCourtesyTimesig,          QVariant(true) },
            { StyleIdx::genCourtesyKeysig,           QVariant(true) },
            { StyleIdx::genCourtesyClef,             QVariant(true) },
            { StyleIdx::swingRatio,                  QVariant(60)   },
            { StyleIdx::swingUnit,                   QVariant(QString("")) },
            { StyleIdx::useStandardNoteNames,        QVariant(true) },
            { StyleIdx::useGermanNoteNames,          QVariant(false) },
            { StyleIdx::useSolfeggioNoteNames,       QVariant(false) },
            { StyleIdx::lowerCaseMinorChords,        QVariant(false) },
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
            { StyleIdx::FixMeasureNumbers,           QVariant(0) },
            { StyleIdx::FixMeasureWidth,             QVariant(false) },
            { StyleIdx::SlurEndWidth,                QVariant(.07) },
            { StyleIdx::SlurMidWidth,                QVariant(.15) },
            { StyleIdx::SlurDottedWidth,             QVariant(.1) },
            { StyleIdx::MinTieLength,                QVariant(1.0) },
            { StyleIdx::SectionPause,                QVariant(qreal(3.0)) },
            { StyleIdx::MusicalSymbolFont,           QVariant(QString("Emmentaler")) },
            { StyleIdx::MusicalTextFont,             QVariant(QString("MScore Text")) },
            { StyleIdx::showHeader,                  QVariant(false) },
//            { StyleIdx::headerStyled,                QVariant(true) },
            { StyleIdx::headerFirstPage,             QVariant(false) },
            { StyleIdx::headerOddEven,               QVariant(true) },
            { StyleIdx::evenHeaderL,                 QVariant(QString()) },
            { StyleIdx::evenHeaderC,                 QVariant(QString()) },
            { StyleIdx::evenHeaderR,                 QVariant(QString()) },
            { StyleIdx::oddHeaderL,                  QVariant(QString()) },
            { StyleIdx::oddHeaderC,                  QVariant(QString()) },
            { StyleIdx::oddHeaderR,                  QVariant(QString()) },
            { StyleIdx::showFooter,                  QVariant(true) },
//            { StyleIdx::footerStyled,                QVariant(true) },
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
            { StyleIdx::tremoloWidth,                QVariant(1.2) },  // tremolo stroke width: note head width
            { StyleIdx::tremoloBoxHeight,            QVariant(0.65) },
            { StyleIdx::tremoloStrokeWidth,          QVariant(0.35) },
            { StyleIdx::tremoloDistance,             QVariant(0.8) },
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
            { StyleIdx::tupletNoteRightDistance,     QVariant(0.0)}
            };
      for (unsigned i = 0; i < sizeof(values2)/sizeof(*values2); ++i)
            _values[int(values2[i].idx)] = values2[i].val;

// _textStyles.append(TextStyle(defaultTextStyles[i]));
      _spatium = SPATIUM20 * MScore::DPI;

      _articulationAnchor[int(ArticulationType::Fermata)]         = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Shortfermata)]    = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Longfermata)]     = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Verylongfermata)] = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Sforzatoaccent)]  = ArticulationAnchor::CHORD;
//      _articulationAnchor[int(ArticulationType::Espressivo)]      = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::Staccato)]        = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::Staccatissimo)]   = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::Tenuto)]          = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::Portato)]         = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::Marcato)]         = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::FadeIn)]          = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::FadeOut)]         = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::VolumeSwell)]     = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::WiggleSawtooth)]  = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::WiggleSawtoothWide)]        = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::WiggleVibratoLargeFaster)]  = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::WiggleVibratoLargeSlowest)] = ArticulationAnchor::CHORD;
      _articulationAnchor[int(ArticulationType::Ouvert)]          = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Plusstop)]        = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Upbow)]           = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Downbow)]         = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Reverseturn)]     = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Turn)]            = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Trill)]           = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Prall)]           = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Mordent)]         = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::PrallPrall)]      = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::PrallMordent)]    = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::UpPrall)]         = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::DownPrall)]       = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::UpMordent)]       = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::DownMordent)]     = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::PrallDown)]       = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::PrallUp)]         = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::LinePrall)]       = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Schleifer)]       = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::Snappizzicato)]   = ArticulationAnchor::TOP_STAFF;
//      _articulationAnchor[int(ArticulationType::Tapping)]         = ArticulationAnchor::TOP_STAFF;
//      _articulationAnchor[int(ArticulationType::Slapping)]        = ArticulationAnchor::TOP_STAFF;
//      _articulationAnchor[int(ArticulationType::Popping)]         = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::ThumbPosition)]   = ArticulationAnchor::TOP_STAFF;
      _articulationAnchor[int(ArticulationType::LuteFingThumb)]   = ArticulationAnchor::BOTTOM_CHORD;
      _articulationAnchor[int(ArticulationType::LuteFingFirst)]   = ArticulationAnchor::BOTTOM_CHORD;
      _articulationAnchor[int(ArticulationType::LuteFingSecond)]  = ArticulationAnchor::BOTTOM_CHORD;
      _articulationAnchor[int(ArticulationType::LuteFingThird)]   = ArticulationAnchor::BOTTOM_CHORD;
      };

StyleData::StyleData(const StyleData& s)
   : QSharedData(s)
      {
      _values          = s._values;
      _chordList       = s._chordList;
      _customChordList = s._customChordList;
      _textStyles      = s._textStyles;
      _pageFormat.copy(s._pageFormat);
      _spatium         = s._spatium;
      for (int i = 0; i < int(ArticulationType::ARTICULATIONS); ++i)
            _articulationAnchor[i] = s._articulationAnchor[i];
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::~StyleData()
      {
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle()
      {
      d = new TextStyleData;
      _hidden = TextStyleHidden::NEVER;
      }

TextStyle::TextStyle(QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot,
   bool sd,
   bool hasFrame, Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg, TextStyleHidden hidden)
      {
      d = new TextStyleData(_name, _family, _size,
         _bold, _italic, _underline, _align, _off, _ot,
         sd, hasFrame, fw, pw, fr, co, _circle, _systemFlag, fg, bg);
      _hidden = hidden;
      }

TextStyle::TextStyle(const TextStyle& s)
   : d(s.d)
      {
      _hidden = s._hidden;
      }
TextStyle::~TextStyle()
      {
      }

//---------------------------------------------------------
//   TextStyle::operator=
//---------------------------------------------------------

TextStyle& TextStyle::operator=(const TextStyle& s)
      {
      d = s.d;
//      _hidden = s._hidden;
      return *this;
      }

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

TextStyleData::TextStyleData()
      {
      family                 = "FreeSerif";
      size                   = 10.0;
      bold                   = false;
      italic                 = false;
      underline              = false;
      hasFrame               = false;
      sizeIsSpatiumDependent = false;
      frameWidth             = Spatium(0);
      paddingWidth           = Spatium(0);
      frameWidthMM           = 0.0;
      paddingWidthMM         = 0.0;
      frameRound             = 25;
      frameColor             = MScore::defaultColor;
      circle                 = false;
      systemFlag             = false;
      foregroundColor        = Qt::black;
      backgroundColor        = QColor(255, 255, 255, 0);
      }

TextStyleData::TextStyleData(
   QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot,
   bool sd,
   bool _hasFrame, Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg)
   :
   ElementLayout(_align, _off, _ot),
   name(_name), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   sizeIsSpatiumDependent(sd), hasFrame(_hasFrame), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag),
   foregroundColor(fg), backgroundColor(bg)
      {
      //hasFrame       = (fw.val() != 0.0) || (bg.alpha() != 0);
      family         = _family;
      frameWidthMM   = 0.0;
      paddingWidthMM = 0.0;
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool TextStyleData::operator!=(const TextStyleData& s) const
      {
      return s.name                   != name
          || s.family                 != family
          || s.size                   != size
          || s.bold                   != bold
          || s.italic                 != italic
          || s.underline              != underline
          || s.hasFrame               != hasFrame
          || s.sizeIsSpatiumDependent != sizeIsSpatiumDependent
          || s.frameWidth             != frameWidth
          || s.paddingWidth           != paddingWidth
          || s.frameRound             != frameRound
          || s.frameColor             != frameColor
          || s.circle                 != circle
          || s.systemFlag             != systemFlag
          || s.foregroundColor        != foregroundColor
          || s.backgroundColor        != backgroundColor
          || s.align()                != align()
          || s.offset()               != offset()
          || s.offsetType()           != offsetType()
          ;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::font(qreal _spatium) const
      {
      qreal m = size;
      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);

      if (sizeIsSpatiumDependent)
            m *= _spatium / ( SPATIUM20 * MScore::DPI);

      f.setPointSizeF(m);
      return f;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::fontPx(qreal _spatium) const
      {
      qreal m = size * MScore::DPI / PPI;

      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);
#ifdef USE_GLYPHS
      f.setHintingPreference(QFont::PreferVerticalHinting);
#endif
      if (sizeIsSpatiumDependent)
            m *= _spatium / (SPATIUM20 * MScore::DPI);

      f.setPixelSize(lrint(m));
      return f;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextStyleData::write(Xml& xml) const
      {
      xml.stag("TextStyle");
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextStyleData::writeProperties(Xml& xml) const
      {
      ElementLayout::writeProperties(xml);
      if (!name.isEmpty())
            xml.tag("name", name);
      xml.tag("family", family);
      xml.tag("size", size);
      if (bold)
            xml.tag("bold", bold);
      if (italic)
            xml.tag("italic", italic);
      if (underline)
            xml.tag("underline", underline);
      if (sizeIsSpatiumDependent)
            xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (foregroundColor != Qt::black)
            xml.tag("foregroundColor", foregroundColor);
      if (backgroundColor != QColor(255, 255, 255, 0))
            xml.tag("backgroundColor", backgroundColor);

      if (hasFrame) {
            xml.tag("frameWidthS",   frameWidth.val());
            xml.tag("paddingWidthS", paddingWidth.val());
            xml.tag("frameRound",   frameRound);
            xml.tag("frameColor",   frameColor);
            if (circle)
                  xml.tag("circle", circle);
            }
      if (systemFlag)
            xml.tag("systemFlag", systemFlag);
      }

//---------------------------------------------------------
//   writeProperties
//    write only changes to the reference r
//---------------------------------------------------------

void TextStyleData::writeProperties(Xml& xml, const TextStyleData& r) const
      {
      ElementLayout::writeProperties(xml, r);
      if (!name.isEmpty() && name != r.name)
            xml.tag("name", name);
      if (family != r.family)
            xml.tag("family", family);
      if (size != r.size)
            xml.tag("size", size);
      if (bold != r.bold)
            xml.tag("bold", bold);
      if (italic != r.italic)
            xml.tag("italic", italic);
      if (underline != r.underline)
            xml.tag("underline", underline);
      if (sizeIsSpatiumDependent != r.sizeIsSpatiumDependent)
            xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (foregroundColor != r.foregroundColor)
            xml.tag("foregroundColor", foregroundColor);
      if (backgroundColor != r.backgroundColor)
            xml.tag("backgroundColor", backgroundColor);
      if (hasFrame != r.hasFrame)
            xml.tag("frame", hasFrame);
      if (hasFrame) {
            if (frameWidth.val() != r.frameWidth.val())
                  xml.tag("frameWidthS",   frameWidth.val());
            if (paddingWidth.val() != r.paddingWidth.val())
                  xml.tag("paddingWidthS", paddingWidth.val());
            if (frameRound != r.frameRound)
                  xml.tag("frameRound",   frameRound);
            if (frameColor != r.frameColor)
                  xml.tag("frameColor",   frameColor);
            if (circle != r.circle)
                  xml.tag("circle", circle);
            }
      if (systemFlag != r.systemFlag)
            xml.tag("systemFlag", systemFlag);
      }

//---------------------------------------------------------
//   restyle
//---------------------------------------------------------

void TextStyleData::restyle(const TextStyleData& os, const TextStyleData& ns)
      {
      ElementLayout::restyle(os, ns);
      if (name == os.name)
            name = ns.name;
      if (family == os.family)
            family = ns.family;
      if (size == os.size)
            size = ns.size;
      if (bold == os.bold)
            bold = ns.bold;
      if (italic == os.italic)
            italic = ns.italic;
      if (underline == os.underline)
            underline = ns.underline;
      if (sizeIsSpatiumDependent == os.sizeIsSpatiumDependent)
            sizeIsSpatiumDependent = ns.sizeIsSpatiumDependent;
      if (foregroundColor == os.foregroundColor)
            foregroundColor = ns.foregroundColor;
      if (backgroundColor == os.backgroundColor)
            backgroundColor = ns.backgroundColor;
      if (hasFrame == os.hasFrame)
            hasFrame = ns.hasFrame;
      if (frameWidth.val() == os.frameWidth.val())
            frameWidth = ns.frameWidth;
      if (paddingWidth.val() == os.paddingWidth.val())
            paddingWidth = ns.paddingWidth;
      if (frameRound == os.frameRound)
            frameRound = ns.frameRound;
      if (frameColor == os.frameColor)
            frameColor = ns.frameColor;
      if (circle == os.circle)
            circle = ns.circle;
      if (systemFlag == os.systemFlag)
            systemFlag = ns.systemFlag;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextStyleData::read(XmlReader& e)
      {
      frameWidth = Spatium(0.0);
      name = e.attribute("name");

      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextStyleData::readProperties(XmlReader& e)
      {
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
            setAlign(Align(e.readInt()));
      else if (tag == "anchor")     // obsolete
            e.skipCurrentElement();
      else if (ElementLayout::readProperties(e))
            ;
      else if (tag == "sizeIsSpatiumDependent" || tag == "spatiumSizeDependent")
            sizeIsSpatiumDependent = e.readInt();
      else if (tag == "frameWidth") { // obsolete
            hasFrame = true;
            frameWidthMM = e.readDouble();
            }
      else if (tag == "frameWidthS") {
            hasFrame = true;
            frameWidth = Spatium(e.readDouble());
            }
      else if (tag == "frame")
            hasFrame = e.readInt();
      else if (tag == "paddingWidth")          // obsolete
            paddingWidthMM = e.readDouble();
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
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void StyleData::load(XmlReader& e)
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
                  setSpatium (e.readDouble() * MScore::DPMM);
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
            else if (tag == "pageFillLimit")   // obsolete
                  e.skipCurrentElement();
            else if (tag == "systemDistance")  // obsolete
                  set(StyleIdx::minSystemDistance, QVariant(e.readDouble()));
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
                  if (tag == "oddHeader" || tag == "evenHeader"
                     || tag == "oddFooter" || tag == "evenFooter")
                        tag += "C";

                  QString val(e.readElementText());

                  int i;
                  for (i = 0; i < int(StyleIdx::STYLES); ++i) {
                        StyleIdx idx = static_cast<StyleIdx>(i);
                        if (styleTypes.name(idx) == tag) {
                              switch(styleTypes.valueType(idx)) {
                                    case StyleValueType::SPATIUM:   set(idx, QVariant(val.toDouble()));    break;
                                    case StyleValueType::DOUBLE:    set(idx, QVariant(val.toDouble()));    break;
                                    case StyleValueType::BOOL:      set(idx, QVariant(bool(val.toInt()))); break;
                                    case StyleValueType::INT:       set(idx, QVariant(val.toInt()));       break;
                                    case StyleValueType::DIRECTION: set(idx, QVariant(val.toInt()));       break;
                                    case StyleValueType::STRING:    set(idx, QVariant(val));               break;
                                    }
                              break;
                              }
                        }
                  if (i >= int(StyleIdx::STYLES)) {
                        if (tag == "oddHeader" || tag == "evenHeader"
                           || tag == "oddFooter" || tag == "evenFooter"
                           || tag == "headerStyled" || tag == "footerStyled"
                           || tag == "beamMinSlope" || tag == "beamMaxSlope"
                            || tag == "stemDir1" || tag == "stemDir2" || tag == "stemDir3" || tag == "stemDir4"
                           ) {
                              ;     // obsolete
                              }
                        else {
                              int idx2;
                              for (idx2 = 0; idx2 < int(ArticulationType::ARTICULATIONS); ++idx2) {
                                    ArticulationInfo& ai =  Articulation::articulationList[idx2];
                                    // deal with obsolete tags from 1.14 format
                                    if (tag == "SforzatoaccentAnchor")
                                          tag = "SforzatoAnchor";
                                    if (tag == "SnappizzicatorAnchor")
                                          tag = "SnappizzicatoAnchor";
                                    else if (tag == "EspressivoAnchor")
                                          break;
                                    if (QString::compare(tag, ai.name + "Anchor",  Qt::CaseInsensitive) == 0
                                       || QString::compare(tag, "U" + ai.name + "Anchor", Qt::CaseInsensitive) == 0
                                       || QString::compare(tag, "D" + ai.name + "Anchor", Qt::CaseInsensitive) == 0
                                       ) {
                                          _articulationAnchor[idx2] = ArticulationAnchor(val.toInt());
                                          break;
                                          }

                                    }
                              if (idx2 >= int(ArticulationType::ARTICULATIONS))
                                    e.unknown();
                              }
                        }
                  }
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

      //
      //  Compatibility with old scores/styles:
      //  translate old frameWidthMM and paddingWidthMM
      //  into spatium units
      //
      int n = _textStyles.size();
      qreal spMM = _spatium / MScore::DPMM;
      for (int i = 0; i < n; ++i) {
            TextStyle* s = &_textStyles[i];
            if (s->frameWidthMM() != 0.0)
                  s->setFrameWidth(Spatium(s->frameWidthMM() / spMM));
            if (s->paddingWidthMM() != 0.0)
                  s->setPaddingWidth(Spatium(s->paddingWidthMM() / spMM));
            }
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool StyleData::isDefault(StyleIdx idx) const
      {
      return _values[int(idx)] == MScore::baseStyle()->value(idx);
      }

//---------------------------------------------------------
//   save
//    if optimize is true, save only data which are different
//    from built-in style ( MScore::baseStyle() )
//---------------------------------------------------------

void StyleData::save(Xml& xml, bool optimize) const
      {
      xml.stag("Style");

      for (int i = 0; i < int(StyleIdx::STYLES); ++i) {
            StyleIdx idx = StyleIdx(i);
            if (optimize && isDefault(idx))
                  continue;
            switch(styleTypes.valueType(idx)) {
                  case StyleValueType::SPATIUM:
                  case StyleValueType::DOUBLE:    xml.tag(styleTypes.name(idx), value(idx).toDouble()); break;
                  case StyleValueType::BOOL:      xml.tag(styleTypes.name(idx), value(idx).toBool()); break;
                  case StyleValueType::INT:       xml.tag(styleTypes.name(idx), value(idx).toInt()); break;
                  case StyleValueType::DIRECTION: xml.tag(styleTypes.name(idx), value(idx).toInt()); break;
                  case StyleValueType::STRING:    xml.tag(styleTypes.name(idx), value(idx).toString()); break;
                  }
            }
      for (int i = 0; i < int(TextStyleType::TEXT_STYLES); ++i) {
            if (!optimize || _textStyles[i] != MScore::baseStyle()->textStyle(TextStyleType(i)))
                  _textStyles[i].write(xml);
            }
      for (int i = int(TextStyleType::TEXT_STYLES); i < _textStyles.size(); ++i)
            _textStyles[i].write(xml);
      if (_customChordList && !_chordList.isEmpty()) {
            xml.stag("ChordList");
            _chordList.write(xml);
            xml.etag();
            }
      for (int i = 0; i < int(ArticulationType::ARTICULATIONS); ++i) {
            if (optimize && _articulationAnchor[i] == MScore::baseStyle()->articulationAnchor(i))
                  continue;
            const ArticulationInfo& ai = Articulation::articulationList[i];
            xml.tag(ai.name + "Anchor", int(_articulationAnchor[i]));
            }
      _pageFormat.write(xml);
      xml.tag("Spatium", _spatium / MScore::DPMM);
      xml.etag();
      }

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* StyleData::chordDescription(int id) const
      {
      if (!_chordList.contains(id))
            return 0;
      return &*_chordList.find(id);
      }

//---------------------------------------------------------
//   chordList
//---------------------------------------------------------

ChordList* StyleData::chordList()
      {
      return &_chordList;
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void StyleData::setChordList(ChordList* cl, bool custom)
      {
      _chordList = *cl;
      _customChordList = custom;
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle& StyleData::textStyle(TextStyleType idx) const
      {
      Q_ASSERT(int(idx) >= 0 && int(idx) < _textStyles.count());
      return _textStyles[int(idx)];
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

QVariant MStyle::value(StyleIdx idx) const
      {
      return d->_values[int(idx)];
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool MStyle::isDefault(StyleIdx idx) const
      {
      return d->isDefault(idx);
      }

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* MStyle::chordDescription(int id) const
      {
      return d->chordDescription(id);
      }

//---------------------------------------------------------
//   chordList
//---------------------------------------------------------

ChordList* MStyle::chordList()
      {
      return d->chordList();
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void MStyle::setChordList(ChordList* cl, bool custom)
      {
      d->setChordList(cl, custom);
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle& StyleData::textStyle(const QString& name) const
      {
      foreach(const TextStyle& s, _textStyles) {
            if (s.name() == name)
                  return s;
            }
      qFatal("TextStyle <%s> not found", qPrintable(name));
      return _textStyles[0];
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

TextStyleType StyleData::textStyleType(const QString& name) const
      {
      for (int i = 0; i < _textStyles.size(); ++i) {
            if (_textStyles[i].name() == name)
                  return TextStyleType(i);
            }
      if (name == "Dynamics2")
            return TextStyleType::DYNAMICS;
      qDebug("TextStyleType <%s> not found", qPrintable(name));
      return TextStyleType::DEFAULT;
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void StyleData::setTextStyle(const TextStyle& ts)
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
//   TextStyle method wrappers
//---------------------------------------------------------

QString TextStyle::name() const                          {  return d->name;    }
QString TextStyle::family() const                        {  return d->family;  }
qreal TextStyle::size() const                            {  return d->size;    }
bool TextStyle::bold() const                             {  return d->bold;    }
bool TextStyle::italic() const                           { return d->italic; }
bool TextStyle::underline() const                        { return d->underline; }
bool TextStyle::hasFrame() const                         { return d->hasFrame; }
Align TextStyle::align() const                           { return d->align(); }
const QPointF& TextStyle::offset() const                 { return d->offset(); }
QPointF TextStyle::offset(qreal spatium) const           { return d->offset(spatium); }
OffsetType TextStyle::offsetType() const                 { return d->offsetType(); }
bool TextStyle::sizeIsSpatiumDependent() const           { return d->sizeIsSpatiumDependent; }

Spatium TextStyle::frameWidth()  const                   { return d->frameWidth; }
Spatium TextStyle::paddingWidth() const                  { return d->paddingWidth; }
qreal TextStyle::frameWidthMM()  const                   { return d->frameWidthMM; }
qreal TextStyle::paddingWidthMM() const                  { return d->paddingWidthMM; }
void TextStyle::setFrameWidth(Spatium v)                 { d->frameWidth = v; }
void TextStyle::setPaddingWidth(Spatium v)               { d->paddingWidth = v; }

int TextStyle::frameRound() const                        { return d->frameRound; }
QColor TextStyle::frameColor() const                     { return d->frameColor; }
bool TextStyle::circle() const                           { return d->circle;     }
bool TextStyle::systemFlag() const                       { return d->systemFlag; }
QColor TextStyle::foregroundColor() const                { return d->foregroundColor; }
QColor TextStyle::backgroundColor() const                { return d->backgroundColor; }
void TextStyle::setName(const QString& s)                { d->name = s; }
void TextStyle::setFamily(const QString& s)              { d->family = s; }
void TextStyle::setSize(qreal v)                         { d->size = v; }
void TextStyle::setBold(bool v)                          { d->bold = v; }
void TextStyle::setItalic(bool v)                        { d->italic = v; }
void TextStyle::setUnderline(bool v)                     { d->underline = v; }
void TextStyle::setHasFrame(bool v)                      { d->hasFrame = v; }
void TextStyle::setAlign(Align v)                        { d->setAlign(v); }
void TextStyle::setXoff(qreal v)                         { d->setXoff(v); }
void TextStyle::setYoff(qreal v)                         { d->setYoff(v); }
void TextStyle::setOffsetType(OffsetType v)              { d->setOffsetType(v); }
void TextStyle::setSizeIsSpatiumDependent(bool v)        { d->sizeIsSpatiumDependent = v; }
void TextStyle::setFrameRound(int v)                     { d->frameRound = v; }
void TextStyle::setFrameColor(const QColor& v)           { d->frameColor = v; }
void TextStyle::setCircle(bool v)                        { d->circle = v;     }
void TextStyle::setSystemFlag(bool v)                    { d->systemFlag = v; }
void TextStyle::setForegroundColor(const QColor& v)      { d->foregroundColor = v; }
void TextStyle::setBackgroundColor(const QColor& v)      { d->backgroundColor = v; }
void TextStyle::write(Xml& xml) const                    { d->write(xml); }
void TextStyle::read(XmlReader& v)               { d->read(v); }
QFont TextStyle::font(qreal space) const                 { return d->font(space); }
QFont TextStyle::fontPx(qreal spatium) const             { return d->fontPx(spatium); }
QRectF TextStyle::bbox(qreal sp, const QString& s) const { return d->bbox(sp, s); }
QFontMetricsF TextStyle::fontMetrics(qreal space) const  { return d->fontMetrics(space); }
bool TextStyle::operator!=(const TextStyle& s) const     { return d->operator!=(*s.d); }
void TextStyle::layout(Element* e) const                 { d->layout(e); }
void TextStyle::writeProperties(Xml& xml) const          { d->writeProperties(xml); }
void TextStyle::writeProperties(Xml& xml, const TextStyle& r) const { d->writeProperties(xml, *r.d); }
void TextStyle::restyle(const TextStyle& os, const TextStyle& ns) { d->restyle(*os.d, *ns.d); }
bool TextStyle::readProperties(XmlReader& v)     { return d->readProperties(v); }

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

MStyle::MStyle()
      {
      d = new StyleData;
      }

MStyle::MStyle(const MStyle& s)
   : d(s.d)
      {
      }

MStyle::~MStyle()
      {
      }

MStyle& MStyle::operator=(const MStyle& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void MStyle::set(StyleIdx id, const QVariant& v)
      {
      d->_values[int(id)] = v;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

const TextStyle& MStyle::textStyle(TextStyleType idx) const
      {
      return d->textStyle(idx);
      }

const TextStyle& MStyle::textStyle(const QString& name) const
      {
      return d->textStyle(name);
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

TextStyleType MStyle::textStyleType(const QString& name) const
      {
      return d->textStyleType(name);
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void MStyle::setTextStyle(const TextStyle& ts)
      {
      d->setTextStyle(ts);
      }

//---------------------------------------------------------
//   addTextStyle
//---------------------------------------------------------

void MStyle::addTextStyle(const TextStyle& ts)
      {
      d->_textStyles.append(ts);
      }

//---------------------------------------------------------
//   removeTextStyle
//---------------------------------------------------------

void MStyle::removeTextStyle(const TextStyle& /*ts*/)
      {
      // TODO: d->_textStyles.append(ts);
      }

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

const QList<TextStyle>& MStyle::textStyles() const
      {
      return d->_textStyles;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void MStyle::set(StyleIdx t, Spatium val)
      {
      set(t, QVariant(val.val()));
      }

void MStyle::set(StyleIdx t, const QString& val)
      {
      set(t, QVariant(val));
      }

void MStyle::set(StyleIdx t, bool val)
      {
      set(t, QVariant(val));
      }

void MStyle::set(StyleIdx t, qreal val)
      {
      set(t, QVariant(val));
      }

void MStyle::set(StyleIdx t, int val)
      {
      set(t, QVariant(val));
      }

void MStyle::set(StyleIdx t, MScore::Direction val)
      {
      set(t, QVariant(int(val)));
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool MStyle::load(QFile* qf)
      {
      return d->load(qf);
      }

void MStyle::load(XmlReader& e)
      {
      d->load(e);
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void MStyle::save(Xml& xml, bool optimize)
      {
      d->save(xml, optimize);
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool StyleData::load(QFile* qf)
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

//---------------------------------------------------------
//   pageFormat
//---------------------------------------------------------

const PageFormat* MStyle::pageFormat() const
      {
      return d->pageFormat();
      }

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

void MStyle::setPageFormat(const PageFormat& pf)
      {
      d->setPageFormat(pf);
      }

void StyleData::setPageFormat(const PageFormat& pf)
      {
      _pageFormat.copy(pf);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal MStyle::spatium() const
      {
      return d->spatium();
      }

//---------------------------------------------------------
//   setSpatium
//---------------------------------------------------------

void MStyle::setSpatium(qreal v)
      {
      d->setSpatium(v);
      }

//---------------------------------------------------------
//   articulationAnchor
//---------------------------------------------------------

ArticulationAnchor MStyle::articulationAnchor(int id) const
      {
      return d->articulationAnchor(id);
      }

//---------------------------------------------------------
//   setArticulationAnchor
//---------------------------------------------------------

void MStyle::setArticulationAnchor(int id, ArticulationAnchor val)
      {
      return d->setArticulationAnchor(id, val);
      }

}

