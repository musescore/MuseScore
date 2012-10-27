//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: style.cpp 5637 2012-05-16 14:23:09Z wschweer $
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
#include "style_p.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "chordlist.h"
#include "page.h"
#include "mscore.h"

MStyle* style;

//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

//---------------------------------------------------------
//   styleTypes
//---------------------------------------------------------

StyleType styleTypes[] = {
      StyleType("staffUpperBorder",        ST_SPATIUM),
      StyleType("staffLowerBorder",        ST_SPATIUM),
      StyleType("staffDistance",           ST_SPATIUM),
      StyleType("akkoladeDistance",        ST_SPATIUM),

      StyleType("minSystemDistance",       ST_SPATIUM),
      StyleType("maxSystemDistance",       ST_SPATIUM),

      StyleType("lyricsDistance",          ST_SPATIUM),
      StyleType("lyricsMinBottomDistance", ST_SPATIUM),
      StyleType("lyricsLineHeight",        ST_DOUBLE),      // in % of normal height (default: 1.0)
      StyleType("figuredBassFontFamily",   ST_STRING),
      StyleType("figuredBassFontSize",     ST_DOUBLE),      // in pt
      StyleType("figuredBassYOffset",      ST_DOUBLE),      // in sp
      StyleType("figuredBassLineHeight",   ST_DOUBLE),      // in % of normal height
      StyleType("figuredBassStyle" ,       ST_INT),         // 0=modern, 1=historic
      StyleType("systemFrameDistance",     ST_SPATIUM),     // dist. between staff and vertical box
      StyleType("frameSystemDistance",     ST_SPATIUM),     // dist. between vertical box and next system
      StyleType("minMeasureWidth",         ST_SPATIUM),

      StyleType("barWidth",                ST_SPATIUM),
      StyleType("doubleBarWidth",          ST_SPATIUM),
      StyleType("endBarWidth",             ST_SPATIUM),
      StyleType("doubleBarDistance",       ST_SPATIUM),
      StyleType("endBarDistance",          ST_SPATIUM),
      StyleType("repeatBarTips",           ST_BOOL),
      StyleType("startBarlineSingle",      ST_BOOL),
      StyleType("startBarlineMultiple",    ST_BOOL),
      StyleType("bracketWidth",            ST_SPATIUM),     // system bracket line width
      StyleType("bracketDistance",         ST_SPATIUM),     // system bracket distance
      StyleType("akkoladeWidth",           ST_SPATIUM),
      StyleType("akkoladeBarDistance",     ST_SPATIUM),

      StyleType("clefLeftMargin",          ST_SPATIUM),
      StyleType("keysigLeftMargin",        ST_SPATIUM),
      StyleType("timesigLeftMargin",       ST_SPATIUM),
      StyleType("clefKeyRightMargin",      ST_SPATIUM),
      StyleType("clefBarlineDistance",     ST_SPATIUM),
      StyleType("stemWidth",               ST_SPATIUM),
      StyleType("shortenStem",             ST_BOOL),        // ST_shortenStem,
      StyleType("shortStemProgression",    ST_SPATIUM),     // ST_shortStemProgression,
      StyleType("shortestStem",            ST_SPATIUM),
      StyleType("beginRepeatLeftMargin",   ST_SPATIUM),

      StyleType("minNoteDistance",         ST_SPATIUM),
      StyleType("barNoteDistance",         ST_SPATIUM),
      StyleType("noteBarDistance",         ST_SPATIUM),
      StyleType("measureSpacing",          ST_DOUBLE),
      StyleType("staffLineWidth",          ST_SPATIUM),
      StyleType("ledgerLineWidth",         ST_SPATIUM),
      StyleType("ledgerLineLength",        ST_SPATIUM),
      StyleType("accidentalDistance",      ST_SPATIUM),
      StyleType("accidentalNoteDistance",  ST_SPATIUM),
      StyleType("beamWidth",               ST_SPATIUM),

      StyleType("beamDistance",            ST_DOUBLE),      // in beamWidth units
      StyleType("beamMinLen",              ST_SPATIUM),     // len for broken beams
      StyleType("beamMinSlope",            ST_DOUBLE),
      StyleType("beamMaxSlope",            ST_DOUBLE),
      StyleType("maxBeamTicks",            ST_INT),
      StyleType("dotNoteDistance",         ST_SPATIUM),
      StyleType("dotRestDistance",         ST_SPATIUM),
      StyleType("dotDotDistance",          ST_SPATIUM),
      StyleType("propertyDistanceHead",    ST_SPATIUM),     // note property to note head
      StyleType("propertyDistanceStem",    ST_SPATIUM),     // note property to note stem

      StyleType("propertyDistance",        ST_SPATIUM),     // note property to note property
//      StyleType("pageFillLimit",           ST_DOUBLE),      // 0-1.0
      StyleType("lastSystemFillLimit",     ST_DOUBLE),

      StyleType("hairpinY",                ST_SPATIUM),
      StyleType("hairpinHeight",           ST_SPATIUM),
      StyleType("hairpinContHeight",       ST_SPATIUM),
      StyleType("hairpinWidth",            ST_SPATIUM),
      StyleType("showPageNumber",          ST_BOOL),
      StyleType("showPageNumberOne",       ST_BOOL),
      StyleType("pageNumberOddEven",       ST_BOOL),
      StyleType("showMeasureNumber",       ST_BOOL),

      StyleType("showMeasureNumberOne",    ST_BOOL),
      StyleType("measureNumberInterval",   ST_INT),
      StyleType("measureNumberSystem",     ST_BOOL),
      StyleType("measureNumberAllStaffs",  ST_BOOL),
      StyleType("smallNoteMag",            ST_DOUBLE),
      StyleType("graceNoteMag",            ST_DOUBLE),
      StyleType("smallStaffMag",           ST_DOUBLE),
      StyleType("smallClefMag",            ST_DOUBLE),
      StyleType("genClef",                 ST_BOOL),           // create clef for all systems, not only for first
      StyleType("genKeysig",               ST_BOOL),         // create key signature for all systems

      StyleType("genTimesig",              ST_BOOL),
      StyleType("genCourtesyTimesig",      ST_BOOL),
      StyleType("genCourtesyKeysig",       ST_BOOL),
      StyleType("genCourtesyClef",         ST_BOOL),
      StyleType("useGermanNoteNames",      ST_BOOL),
      StyleType("chordDescriptionFile",    ST_STRING),
      StyleType("concertPitch",            ST_BOOL),            // display transposing instruments in concert pitch
      StyleType("createMultiMeasureRests", ST_BOOL),
      StyleType("minEmptyMeasures",        ST_INT),         // minimum number of empty measures for multi measure rest
      StyleType("minMMRestWidth",          ST_SPATIUM),       // minimum width of multi measure rest
      StyleType("hideEmptyStaves",         ST_BOOL),
      StyleType("dontHidStavesInFirstSystm", ST_BOOL),

      StyleType("stemDir1",                ST_DIRECTION),
      StyleType("stemDir2",                ST_DIRECTION),
      StyleType("stemDir3",                ST_DIRECTION),
      StyleType("stemDir4",                ST_DIRECTION),

      //---------------------------------------------------------
      //   PlayStyle
      //---------------------------------------------------------

      StyleType("gateTime",                ST_INT),           // 0-100%
      StyleType("tenutoGateTime",          ST_INT),
      StyleType("staccatoGateTime",        ST_INT),
      StyleType("slurGateTime",            ST_INT),

      StyleType("ArpeggioNoteDistance",    ST_SPATIUM),
      StyleType("ArpeggioLineWidth",       ST_SPATIUM),
      StyleType("ArpeggioHookLen",         ST_SPATIUM),
      StyleType("FixMeasureNumbers",       ST_INT),
      StyleType("FixMeasureWidth",         ST_BOOL),

      StyleType("slurEndWidth",            ST_SPATIUM),
      StyleType("slurMidWidth",            ST_SPATIUM),
      StyleType("slurDottedWidth",         ST_SPATIUM),
      StyleType("slurBow",                 ST_SPATIUM),
      StyleType("sectionPause",            ST_DOUBLE),

      StyleType("musicalSymbolFont",       ST_STRING),

      StyleType("showHeader",              ST_BOOL),
      StyleType("headerStyled",            ST_BOOL),
      StyleType("headerFirstPage",         ST_BOOL),
      StyleType("headerOddEven",           ST_BOOL),
      StyleType("evenHeaderL",             ST_STRING),
      StyleType("evenHeaderC",             ST_STRING),
      StyleType("evenHeaderR",             ST_STRING),
      StyleType("oddHeaderL",              ST_STRING),
      StyleType("oddHeaderC",              ST_STRING),
      StyleType("oddHeaderR",              ST_STRING),

      StyleType("showFooter",              ST_BOOL),
      StyleType("footerStyled",            ST_BOOL),
      StyleType("footerFirstPage",         ST_BOOL),
      StyleType("footerOddEven",           ST_BOOL),
      StyleType("evenFooterL",             ST_STRING),
      StyleType("evenFooterC",             ST_STRING),
      StyleType("evenFooterR",             ST_STRING),
      StyleType("oddFooterL",              ST_STRING),
      StyleType("oddFooterC",              ST_STRING),
      StyleType("oddFooterR",              ST_STRING),

      StyleType("voltaY",                  ST_SPATIUM),
      StyleType("voltaHook",               ST_SPATIUM),
      StyleType("voltaLineWidth",          ST_SPATIUM),

      StyleType("ottavaY",                 ST_SPATIUM),
      StyleType("ottavaHook",              ST_SPATIUM),
      StyleType("ottavaLineWidth",         ST_SPATIUM),

      StyleType("tabClef",                ST_INT),
      };

static const QString ff("FreeSerifMscore");

#define MM(x) ((x)/INCH)
#define OA     OFFSET_ABS
#define OS     OFFSET_SPATIUM
#define TR(x)  QT_TRANSLATE_NOOP("MuseScore", x)
#define AS(x)  s->addTextStyle(x)

//---------------------------------------------------------
//   setDefaultStyle
//    synchronize with TextStyleType
//---------------------------------------------------------

void initStyle(MStyle* s)
      {
      // this is an empty style, no offsets are allowed
      // dont show this style in editor
      AS(TextStyle(
         "", ff, 10, false, false, false, ALIGN_LEFT | ALIGN_BASELINE));

      AS(TextStyle(
         TR("Title"), ff, 24, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, QPointF(), OA, QPointF(50.0, .0)));

      AS(TextStyle(
         TR("Subtitle"), ff, 14, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, QPointF(0, MM(10)), OA, QPointF(50.0, .0)));

      AS(TextStyle(
        TR("Composer"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_BASELINE, QPointF(MM(-1), MM(-2)), OA, QPointF(100.0, 100.0)));

      AS(TextStyle(
         TR("Lyricist"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(MM(1), MM(-2)), OA, QPointF(0.0, 100.0)));

      AS(TextStyle(
         TR("Lyrics odd lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, 7), OS, QPointF(), true));

      AS(TextStyle(
         TR("Lyrics even lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, 7), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Fingering"), ff,  8, false, false, false,
         ALIGN_CENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "InstrumentsExcerpt"), ff, 18, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, QPointF(), OA, QPointF()));

      AS(TextStyle(
         TR( "Dynamics"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.0, 8.0), OS, QPointF(), true));

      AS(TextStyle(           // internal style
         TR( "Dynamics2"), "MScore1", 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.0, 8.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Technik"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.0, -2.0), OS));

#if 0
// (temporarly) switch bold off as Qt cannot show bold glyphs with codepoint > 16bit
// (used for musical symbols)
      AS(TextStyle(
         TR( "Tempo"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0, -4.0), OS, QPointF(),
         true, MMSP(.0), MMSP(.0), 0, Qt::black, false, true));
#else
      AS(TextStyle(
         TR( "Tempo"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0, -4.0), OS, QPointF(),
         true, MMSP(.0), MMSP(.0), 0, Qt::black, false, true));
#endif
      AS(TextStyle(
         TR( "Metronome"), ff, 12, true, false, false, ALIGN_LEFT));

      AS(TextStyle(
         TR( "Measure Number"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM, QPointF(.0, -2.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Translator"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, QPointF(0, 6)));

      AS(TextStyle(
         TR( "Tuplets"), ff,  10, false, true, false,
         ALIGN_CENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "System"), ff,  10, false, false, false,
         ALIGN_LEFT, QPointF(0, -4.0), OS, QPointF(), true,
         Spatium(0.0), Spatium(0.0), 25, Qt::black, false, true));

      AS(TextStyle(
         TR( "Staff"), ff,  10, false, false, false,
         ALIGN_LEFT, QPointF(0, -4.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Chordname"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0, -4.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Rehearsal Mark"), ff,  14, true, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, -3.0), OS, QPointF(), true,
         Spatium(0.2), Spatium(.5), 20, Qt::black, false, true));

      AS(TextStyle(
         TR( "Repeat Text Left"), "MScore",  20, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0, -2.0), OS, QPointF(), true,
         MMSP(0.0), MMSP(0.0), 25, Qt::black, false, true));

      AS(TextStyle(
         TR( "Repeat Text Right"), ff,  12, false, false, false,
         ALIGN_RIGHT | ALIGN_BASELINE, QPointF(0, -2.0), OS, QPointF(100, 0), true,
         MMSP(0.0), MMSP(0.0), 25, Qt::black, false, true));

      AS(TextStyle(
         TR( "Repeat Text"), ff,  12, false, false, false,          // for backward compatibility
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, -2.0), OS, QPointF(100, 0), true,
         MMSP(0.0), MMSP(0.0), 25, Qt::black, false, true));

      // y offset may depend on voltaHook style element
      AS(TextStyle(
         TR( "Volta"), ff, 11, true, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.5, 1.9), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Frame"), ff, 12, false, false, false, ALIGN_LEFT | ALIGN_TOP));

      AS(TextStyle(
         TR( "TextLine"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_VCENTER, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Glissando"), ff, 8, false, true, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "String Number"), ff,  8, false, false, false,
         ALIGN_CENTER, QPointF(0, -5.0), OS, QPointF(100, 0), true,
         Spatium(0.1), Spatium(0.2), 0, Qt::black, true, false));

      AS(TextStyle(
         TR( "Ottava"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_VCENTER, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Bend"), ff, 8, false, false, false,
         ALIGN_CENTER | ALIGN_BOTTOM, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Header"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP));

      AS(TextStyle(
         TR( "Footer"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM, QPointF(0.0, MM(5)), OA));

      AS(TextStyle(
         TR( "Instrument Change"), ff,  12, true, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, QPointF(0, -3.0), OS, QPointF(0, 0), true));

      AS(TextStyle(
         TR("Lyrics Verse"), ff, 11, false, false, false,
         ALIGN_RIGHT | ALIGN_TOP, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
      TR("Figured Bass"), "MScoreBC", 8, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, QPointF(0, 6), OS, QPointF(), true));

#undef MM
#undef OA
#undef OS
#undef TR
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::StyleData()
   : _values(ST_STYLES)
      {
      _customChordList = false;
      static const StyleVal values[ST_STYLES] = {
            StyleVal(ST_staffUpperBorder, Spatium(7.0)),
            StyleVal(ST_staffLowerBorder, Spatium(7.0)),
            StyleVal(ST_staffDistance, Spatium(6.5)),
            StyleVal(ST_akkoladeDistance, Spatium(6.5)),
            StyleVal(ST_minSystemDistance, Spatium(8.5)),
            StyleVal(ST_maxSystemDistance, Spatium(15.0)),
            StyleVal(ST_lyricsDistance, Spatium(3.5)),

            StyleVal(ST_lyricsMinBottomDistance, Spatium(2)),
            StyleVal(ST_lyricsLineHeight, qreal(1.0)),
            StyleVal(ST_figuredBassFontFamily, QString("MuseScore Figured Bass")),
            StyleVal(ST_figuredBassFontSize, qreal(8.0)),
            StyleVal(ST_figuredBassYOffset, Spatium(6.0)),
            StyleVal(ST_figuredBassLineHeight, qreal(1.0)),
            StyleVal(ST_figuredBassStyle, 0),
            StyleVal(ST_systemFrameDistance, Spatium(7.0)),
            StyleVal(ST_frameSystemDistance, Spatium(7.0)),
            StyleVal(ST_minMeasureWidth, Spatium(5.0)),
                                                            // finale european style
            StyleVal(ST_barWidth, Spatium(0.16)),           // 0.1875
            StyleVal(ST_doubleBarWidth, Spatium(0.16)),
            StyleVal(ST_endBarWidth, Spatium(0.3)),         // 0.5
            StyleVal(ST_doubleBarDistance, Spatium(0.30)),
            StyleVal(ST_endBarDistance, Spatium(0.40)),     // 0.3
            StyleVal(ST_repeatBarTips, false),
            StyleVal(ST_startBarlineSingle, false),
            StyleVal(ST_startBarlineMultiple, true),

            StyleVal(ST_bracketWidth, Spatium(0.45)),
            StyleVal(ST_bracketDistance, Spatium(0.8)),
            StyleVal(ST_akkoladeWidth,Spatium(1.6)),
            StyleVal(ST_akkoladeBarDistance,Spatium(.4)),

            StyleVal(ST_clefLeftMargin, Spatium(0.64)),
            StyleVal(ST_keysigLeftMargin, Spatium(0.5)),
            StyleVal(ST_timesigLeftMargin, Spatium(0.5)),

            StyleVal(ST_clefKeyRightMargin, Spatium(1.75)),
            StyleVal(ST_clefBarlineDistance, Spatium(0.18)),      // was 0.5
            StyleVal(ST_stemWidth, Spatium(0.13)),          // 0.09375
            StyleVal(ST_shortenStem, true),
            StyleVal(ST_shortStemProgression, Spatium(0.25)),
            StyleVal(ST_shortestStem, Spatium(2.25)),
            StyleVal(ST_beginRepeatLeftMargin,Spatium(1.0)),
            StyleVal(ST_minNoteDistance,Spatium(0.25)),           // 0.4
            StyleVal(ST_barNoteDistance,Spatium(1.2)),
            StyleVal(ST_noteBarDistance,Spatium(1.0)),

            //38
            StyleVal(ST_measureSpacing, qreal(1.2)),
            StyleVal(ST_staffLineWidth,Spatium(0.08)),      // 0.09375
            StyleVal(ST_ledgerLineWidth,Spatium(0.12)),     // 0.1875
            StyleVal(ST_ledgerLineLength, Spatium(.6)),
            StyleVal(ST_accidentalDistance,Spatium(0.22)),
            StyleVal(ST_accidentalNoteDistance,Spatium(0.22)),
            StyleVal(ST_beamWidth, Spatium(0.5)),           // was 0.48
            StyleVal(ST_beamDistance, qreal(0.5)),          // 0.25sp
            StyleVal(ST_beamMinLen,Spatium(1.316178)),      // exactly note head width
            StyleVal(ST_beamMinSlope, qreal(0.05)),

            StyleVal(ST_beamMaxSlope,         qreal(0.2)),
            StyleVal(ST_maxBeamTicks,         MScore::division),
            StyleVal(ST_dotNoteDistance,      Spatium(0.35)),
            StyleVal(ST_dotRestDistance,      Spatium(0.25)),
            StyleVal(ST_dotDotDistance,       Spatium(0.5)),
            StyleVal(ST_propertyDistanceHead, Spatium(1.0)),
            StyleVal(ST_propertyDistanceStem, Spatium(1.8)),
            StyleVal(ST_propertyDistance,     Spatium(1.0)),
//            StyleVal(ST_pageFillLimit,        qreal(0.7)),
            StyleVal(ST_lastSystemFillLimit,  qreal(0.3)),

            StyleVal(ST_hairpinY, Spatium(8)),
            StyleVal(ST_hairpinHeight, Spatium(1.2)),
            StyleVal(ST_hairpinContHeight, Spatium(0.5)),
            StyleVal(ST_hairpinWidth, Spatium(0.13)),
            StyleVal(ST_showPageNumber, true),
            StyleVal(ST_showPageNumberOne, false),
            StyleVal(ST_pageNumberOddEven, true),
            StyleVal(ST_showMeasureNumber, true),
            StyleVal(ST_showMeasureNumberOne, false),
            StyleVal(ST_measureNumberInterval, 5),
            StyleVal(ST_measureNumberSystem, true),

            //68
            StyleVal(ST_measureNumberAllStaffs,false),
            StyleVal(ST_smallNoteMag, qreal(.7)),
            StyleVal(ST_graceNoteMag, qreal(0.7)),
            StyleVal(ST_smallStaffMag, qreal(0.7)),
            StyleVal(ST_smallClefMag, qreal(0.8)),
            StyleVal(ST_genClef,true),
            StyleVal(ST_genKeysig,true),
            StyleVal(ST_genTimesig,true),
            StyleVal(ST_genCourtesyTimesig, true),
            StyleVal(ST_genCourtesyKeysig, true),
            StyleVal(ST_genCourtesyClef, true),

            StyleVal(ST_useGermanNoteNames, false),
            StyleVal(ST_chordDescriptionFile, QString("stdchords.xml")),
            StyleVal(ST_concertPitch, false),
            StyleVal(ST_createMultiMeasureRests, false),
            StyleVal(ST_minEmptyMeasures, 2),
            StyleVal(ST_minMMRestWidth, Spatium(4)),
            StyleVal(ST_hideEmptyStaves, false),
            StyleVal(ST_dontHideStavesInFirstSystem, true),
            StyleVal(ST_stemDir1, MScore::UP),
            StyleVal(ST_stemDir2, MScore::DOWN),
            StyleVal(ST_stemDir3, MScore::UP),
            StyleVal(ST_stemDir4, MScore::DOWN),

            StyleVal(ST_gateTime, 100),
            StyleVal(ST_tenutoGateTime, 100),
            StyleVal(ST_staccatoGateTime, 50),
            StyleVal(ST_slurGateTime, 100),

            StyleVal(ST_ArpeggioNoteDistance, Spatium(.5)),
            StyleVal(ST_ArpeggioLineWidth, Spatium(.18)),
            StyleVal(ST_ArpeggioHookLen, Spatium(.8)),
            StyleVal(ST_FixMeasureNumbers, 0),
            StyleVal(ST_FixMeasureWidth, false),

            //100
            StyleVal(ST_SlurEndWidth, Spatium(.07)),
            StyleVal(ST_SlurMidWidth, Spatium(.15)),
            StyleVal(ST_SlurDottedWidth, Spatium(.1)),
            StyleVal(ST_SlurBow, Spatium(1.6)),

            StyleVal(ST_SectionPause, qreal(3.0)),

            StyleVal(ST_MusicalSymbolFont, QString("Emmentaler")),

            StyleVal(ST_showHeader,      false),
            StyleVal(ST_headerStyled,    true),
            StyleVal(ST_headerFirstPage, false),
            StyleVal(ST_headerOddEven,   true),

            StyleVal(ST_evenHeaderL, QString()),
            StyleVal(ST_evenHeaderC, QString()),
            StyleVal(ST_evenHeaderR, QString()),
            StyleVal(ST_oddHeaderL,  QString()),
            StyleVal(ST_oddHeaderC,  QString()),
            StyleVal(ST_oddHeaderR,  QString()),

            StyleVal(ST_showFooter,      true),
            StyleVal(ST_footerStyled,    true),
            StyleVal(ST_footerFirstPage, true),
            StyleVal(ST_footerOddEven,   true),

            StyleVal(ST_evenFooterL, QString("$p")),
            StyleVal(ST_evenFooterC, QString("$:copyright:")),
            StyleVal(ST_evenFooterR, QString()),
            StyleVal(ST_oddFooterL,  QString()),
            StyleVal(ST_oddFooterC,  QString("$:copyright:")),
            StyleVal(ST_oddFooterR,  QString("$p")),

            StyleVal(ST_voltaY, Spatium(-3.0)),
            StyleVal(ST_voltaHook, Spatium(1.9)),
            StyleVal(ST_voltaLineWidth, Spatium(.1)),

            StyleVal(ST_ottavaY, Spatium(-3.0)),
            StyleVal(ST_ottavaHook, Spatium(1.9)),
            StyleVal(ST_ottavaLineWidth, Spatium(.1)),
            StyleVal(ST_tabClef, int(CLEF_TAB2))
            };

      for (int idx = 0; idx < ST_STYLES; ++idx)
            _values[idx] = values[idx];

// _textStyles.append(TextStyle(defaultTextStyles[i]));
      _chordList = 0;
      _spatium = SPATIUM20 * MScore::DPI;
      _articulationAnchor[Articulation_Fermata]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_Shortfermata]    = A_TOP_STAFF;
      _articulationAnchor[Articulation_Longfermata]     = A_TOP_STAFF;
      _articulationAnchor[Articulation_Verylongfermata] = A_TOP_STAFF;
      _articulationAnchor[Articulation_Thumb]           = A_CHORD;
      _articulationAnchor[Articulation_Sforzatoaccent]  = A_CHORD;
      _articulationAnchor[Articulation_Espressivo]      = A_CHORD;
      _articulationAnchor[Articulation_Staccato]        = A_CHORD;
      _articulationAnchor[Articulation_Staccatissimo]   = A_CHORD;
      _articulationAnchor[Articulation_Tenuto]          = A_CHORD;
      _articulationAnchor[Articulation_Portato]         = A_CHORD;
      _articulationAnchor[Articulation_Marcato]         = A_CHORD;
      _articulationAnchor[Articulation_Ouvert]          = A_CHORD;
      _articulationAnchor[Articulation_Plusstop]        = A_CHORD;
      _articulationAnchor[Articulation_Upbow]           = A_TOP_STAFF;
      _articulationAnchor[Articulation_Downbow]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_Reverseturn]     = A_TOP_STAFF;
      _articulationAnchor[Articulation_Turn]            = A_TOP_STAFF;
      _articulationAnchor[Articulation_Trill]           = A_TOP_STAFF;
      _articulationAnchor[Articulation_Prall]           = A_TOP_STAFF;
      _articulationAnchor[Articulation_Mordent]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallPrall]      = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallMordent]    = A_TOP_STAFF;
      _articulationAnchor[Articulation_UpPrall]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_DownPrall]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_UpMordent]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_DownMordent]     = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallDown]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallUp]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_LinePrall]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_Schleifer]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_Snappizzicato]   = A_TOP_STAFF;
      _articulationAnchor[Articulation_Tapping]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_Slapping]        = A_TOP_STAFF;
      _articulationAnchor[Articulation_Popping]         = A_TOP_STAFF;
      _spatium = SPATIUM20 * MScore::DPI;
      };

StyleData::StyleData(const StyleData& s)
   : QSharedData(s)
      {
      _values = s._values;
      if (s._chordList)
            _chordList = new ChordList(*(s._chordList));
      else
            _chordList = 0;
      _customChordList = s._customChordList;
      _textStyles      = s._textStyles;
      _pageFormat.copy(s._pageFormat);
      _spatium         = s._spatium;
      for (int i = 0; i < ARTICULATIONS; ++i)
            _articulationAnchor[i] = s._articulationAnchor[i];
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::~StyleData()
      {
      delete _chordList;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle()
      {
      d = new TextStyleData;
      }

TextStyle::TextStyle(
   QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot, const QPointF& _roff,
   bool sd,
   Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg)
      {
      d = new TextStyleData(_name, _family, _size,
         _bold, _italic, _underline, _align, _off, _ot, _roff,
         sd, fw, pw, fr, co, _circle, _systemFlag, fg, bg);
      }

TextStyle::TextStyle(const TextStyle& s)
   : d(s.d)
      {
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
   const QPointF& _off, OffsetType _ot, const QPointF& _roff,
   bool sd,
   Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg)
   :
   ElementLayout(_align, _off, _ot, _roff),
   name(_name), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   sizeIsSpatiumDependent(sd), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag),
   foregroundColor(fg), backgroundColor(bg)
      {
      hasFrame       = (fw.val() != 0.0) || (bg.alpha() != 0);
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
          || s.rxoff()                != rxoff()
          || s.ryoff()                != ryoff()
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
//   read
//---------------------------------------------------------

void TextStyleData::read(const QDomElement& de)
      {
      frameWidth = Spatium(0.0);
      name = de.attribute("name");

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextStyleData::readProperties(const QDomElement& e)
      {
      const QString& tag(e.tagName());
      const QString& val(e.text());
      int i = val.toInt();

      if (tag == "name")
            name = val;
      else if (tag == "family")
            family = val;
      else if (tag == "size")
            size = val.toDouble();
      else if (tag == "bold")
            bold = i;
      else if (tag == "italic")
            italic = i;
      else if (tag == "underline")
            underline = i;
      else if (tag == "align")
            setAlign(Align(i));
      else if (tag == "anchor")     // obsolete
            ;
      else if (ElementLayout::readProperties(e))
            ;
      else if (tag == "sizeIsSpatiumDependent")
            sizeIsSpatiumDependent = val.toDouble();
      else if (tag == "frameWidth") {
            hasFrame = true;
            frameWidthMM = val.toDouble();
            }
      else if (tag == "frameWidthS") {
            hasFrame = true;
            frameWidth = Spatium(val.toDouble());
            }
      else if (tag == "frame")      // obsolete
            hasFrame = i;
      else if (tag == "paddingWidth")          // obsolete
            paddingWidthMM = val.toDouble();
      else if (tag == "paddingWidthS")
            paddingWidth = Spatium(val.toDouble());
      else if (tag == "frameRound")
            frameRound = i;
      else if (tag == "frameColor")
            frameColor = readColor(e);
      else if (tag == "foregroundColor")
            foregroundColor = readColor(e);
      else if (tag == "backgroundColor")
            backgroundColor = readColor(e);
      else if (tag == "circle")
            circle = val.toInt();
      else if (tag == "systemFlag")
            systemFlag = val.toInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void StyleData::load(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            const QString& val(e.text());

            if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(e);
                  setTextStyle(s);
                  }
            else if (tag == "Spatium")
                  setSpatium (val.toDouble() * MScore::DPMM);
            else if (tag == "page-layout")
                  _pageFormat.read(e);
            else if (tag == "displayInConcertPitch")
                  set(StyleVal(ST_concertPitch, bool(val.toInt())));
            else if (tag == "ChordList") {
                  delete _chordList;
                  _chordList = new ChordList;
                  _chordList->read(e);
                  _customChordList = true;
                  }
            else if (tag == "pageFillLimit") { // obsolete
                  }
            else if (tag == "systemDistance")  // obsolete
                  set(StyleVal(ST_minSystemDistance, Spatium(val.toDouble())));
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

                  int idx;
                  for (idx = 0; idx < ST_STYLES; ++idx) {
                        if (styleTypes[idx].name() == tag) {
                              StyleIdx i = StyleIdx(idx);
                              switch(styleTypes[idx].valueType()) {
                                    case ST_SPATIUM:   set(StyleVal(i, Spatium(val.toDouble()))); break;
                                    case ST_DOUBLE:    set(StyleVal(i, qreal(val.toDouble())));          break;
                                    case ST_BOOL:      set(StyleVal(i, bool(val.toInt())));       break;
                                    case ST_INT:       set(StyleVal(i, val.toInt()));             break;
                                    case ST_DIRECTION: set(StyleVal(i, MScore::Direction(val.toInt())));  break;
                                    case ST_STRING:    set(StyleVal(i, val));                     break;
                                    }
                              break;
                              }
                        }
                  if (idx >= ST_STYLES) {
                        if (tag == "oddHeader")
                              ;
                        else if (tag == "evenHeader")
                              ;
                        else if (tag == "oddFooter")
                              ;
                        else if (tag == "evenHeader")
                              ;
                        else {
                              int idx2;
                              for (idx2 = 0; idx2 < ARTICULATIONS; ++idx2) {
                                    ArticulationInfo& ai =  Articulation::articulationList[idx2];
                                    if (tag == ai.name + "Anchor") {
                                          _articulationAnchor[idx2] = ArticulationAnchor(val.toInt());
                                          break;
                                          }
                                    }
                              if (idx2 == ARTICULATIONS)
                                    domError(e);
                              }
                        }
                  }
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
      switch(styleTypes[idx].valueType()) {
            case ST_DOUBLE:
            case ST_SPATIUM:
                  return _values[idx].toDouble() == MScore::baseStyle()->valueD(idx);
            case ST_BOOL:
                  return _values[idx].toBool() == MScore::baseStyle()->valueB(idx);
            case ST_INT:
            case ST_DIRECTION:
                  return _values[idx].toInt() == MScore::baseStyle()->valueI(idx);
            case ST_STRING:
                  return _values[idx].toString() == MScore::baseStyle()->valueSt(idx);
            }
      return false;
      }

//---------------------------------------------------------
//   save
//    if optimize is true, save only if different to default
//    style
//---------------------------------------------------------

void StyleData::save(Xml& xml, bool optimize) const
      {
      xml.stag("Style");

      for (int i = 0; i < ST_STYLES; ++i) {
            StyleIdx idx = StyleIdx(i);
            if (optimize && isDefault(idx))
                  continue;
            switch(styleTypes[idx].valueType()) {
                  case ST_SPATIUM:
                  case ST_DOUBLE:    xml.tag(styleTypes[idx].name(), value(idx).toDouble()); break;
                  case ST_BOOL:      xml.tag(styleTypes[idx].name(), value(idx).toBool()); break;
                  case ST_INT:       xml.tag(styleTypes[idx].name(), value(idx).toInt()); break;
                  case ST_DIRECTION: xml.tag(styleTypes[idx].name(), int(value(idx).toDirection())); break;
                  case ST_STRING:    xml.tag(styleTypes[idx].name(), value(idx).toString()); break;
                  }
            }
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (!optimize || _textStyles[i] != MScore::defaultStyle()->textStyle(i))
                  _textStyles[i].write(xml);
            }
      for (int i = TEXT_STYLES; i < _textStyles.size(); ++i)
            _textStyles[i].write(xml);
      if (_customChordList && _chordList) {
            xml.stag("ChordList");
            _chordList->write(xml);
            xml.etag();
            }
      for (int i = 0; i < ARTICULATIONS; ++i) {
            if (optimize && _articulationAnchor[i] == MScore::defaultStyle()->articulationAnchor(i))
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
      return chordList()->value(id);
      }

//---------------------------------------------------------
//   chordList
//---------------------------------------------------------

ChordList* StyleData::chordList()  const
      {
      if (_chordList == 0) {
            _chordList = new ChordList();
            _chordList->read("chords.xml");
            _chordList->read(value(ST_chordDescriptionFile).toString());
            }
      return _chordList;
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void StyleData::setChordList(ChordList* cl)
      {
      delete _chordList;
      _chordList = cl;
      _customChordList = true;      // TODO: check
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle& StyleData::textStyle(int idx) const
      {
      Q_ASSERT(idx >= 0 && idx < _textStyles.count());
      return _textStyles[idx];
      }

//---------------------------------------------------------
//   StyleVal
//---------------------------------------------------------

StyleVal::StyleVal(StyleIdx t, Spatium val)
      {
      idx  = t;
      v.dbl = val.val();
      }

StyleVal::StyleVal(StyleIdx t, qreal val)
      {
      idx  = t;
      v.dbl = val;
      }

StyleVal::StyleVal(StyleIdx t, bool val)
      {
      idx  = t;
      v.b   = val;
      }

StyleVal::StyleVal(StyleIdx t, int val)
      {
      idx  = t;
      v.i   = val;
      }

StyleVal::StyleVal(StyleIdx t, MScore::Direction val)
      {
      idx  = t;
      v.d   = val;
      }

StyleVal::StyleVal(StyleIdx t, const QString& val)
      {
      idx  = t;
      s    = val;
      }

StyleVal::StyleVal(const StyleVal& val)
      {
      idx = val.idx;
      s   = val.s;
      v   = val.v;
      }

StyleVal& StyleVal::operator=(const StyleVal& val)
      {
      idx = val.idx;
      s   = val.s;
      v   = val.v;
      return *this;
      }

StyleVal::StyleVal(const QString& name, const QString& val)
      {
      for (int i = 0; i < ST_STYLES; ++i) {
            if (styleTypes[i].name() != name)
                  continue;
            idx = StyleIdx(i);
            switch(styleTypes[i].valueType()) {
                  case ST_DOUBLE:
                  case ST_SPATIUM:
                        v.dbl = val.toDouble();
                        break;
                  case ST_BOOL:
                        v.b  = val.toInt();
                        break;
                  case ST_INT:
                        v.i = val.toInt();
                        break;
                  case ST_DIRECTION:
                        v.d = MScore::Direction(val.toInt());
                        break;
                  case ST_STRING:
                        s = val;
                        break;
                  }
            break;
            }
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

StyleVal MStyle::value(StyleIdx idx) const
      {
      return d->_values[idx];
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

ChordList* MStyle::chordList() const
      {
      return d->chordList();
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void MStyle::setChordList(ChordList* cl)
      {
      d->setChordList(cl);
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
      qDebug("TextStyle <%s> not found", qPrintable(name));
      abort();
      return _textStyles[0];
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

int StyleData::textStyleType(const QString& name) const
      {
      for (int i = 0; i < _textStyles.size(); ++i) {
            if (_textStyles[i].name() == name)
                  return i;
            }
      qDebug("TextStyleType <%s> not found", qPrintable(name));
      return TEXT_STYLE_UNKNOWN;
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
void TextStyle::setRxoff(qreal v)                        { d->setRxoff(v); }
void TextStyle::setRyoff(qreal v)                        { d->setRyoff(v); }
void TextStyle::setSizeIsSpatiumDependent(bool v)        { d->sizeIsSpatiumDependent = v; }
void TextStyle::setFrameRound(int v)                     { d->frameRound = v; }
void TextStyle::setFrameColor(const QColor& v)           { d->frameColor = v; }
void TextStyle::setCircle(bool v)                        { d->circle = v;     }
void TextStyle::setSystemFlag(bool v)                    { d->systemFlag = v; }
void TextStyle::setForegroundColor(const QColor& v)      { d->foregroundColor = v; }
void TextStyle::setBackgroundColor(const QColor& v)      { d->backgroundColor = v; }
void TextStyle::write(Xml& xml) const                    { d->write(xml); }
void TextStyle::read(const QDomElement& v)               { d->read(v); }
QFont TextStyle::font(qreal space) const                 { return d->font(space); }
QFont TextStyle::fontPx(qreal spatium) const             { return d->fontPx(spatium); }
QRectF TextStyle::bbox(qreal sp, const QString& s) const { return d->bbox(sp, s); }
QFontMetricsF TextStyle::fontMetrics(qreal space) const  { return d->fontMetrics(space); }
bool TextStyle::operator!=(const TextStyle& s) const     { return d->operator!=(*s.d); }
void TextStyle::layout(Element* e) const                 { d->layout(e); }
void TextStyle::writeProperties(Xml& xml) const          { d->writeProperties(xml); }
const QPointF& TextStyle::reloff() const                 { return d->reloff();      }
void TextStyle::setReloff(const QPointF& p)              { setRxoff(p.x()), setRyoff(p.y()); }
bool TextStyle::readProperties(const QDomElement& v)     { return d->readProperties(v); }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void TextStyle::setFont(const QFont&)
      {
      //TODOxx
      }

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

void MStyle::set(const StyleVal& v)
      {
      d->_values[v.getIdx()] = v;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

const TextStyle& MStyle::textStyle(int idx) const
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

int MStyle::textStyleType(const QString& name) const
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
      set(StyleVal(t, val));
      }

void MStyle::set(StyleIdx t, const QString& val)
      {
      set(StyleVal(t, val));
      }

void MStyle::set(StyleIdx t, bool val)
      {
      set(StyleVal(t, val));
      }

void MStyle::set(StyleIdx t, qreal val)
      {
      set(StyleVal(t, val));
      }

void MStyle::set(StyleIdx t, int val)
      {
      set(StyleVal(t, val));
      }

void MStyle::set(StyleIdx t, MScore::Direction val)
      {
      set(StyleVal(t, val));
      }

//---------------------------------------------------------
//   valueS
//---------------------------------------------------------

Spatium MStyle::valueS(StyleIdx idx) const
      {
      return value(idx).toSpatium();
      }

//---------------------------------------------------------
//   valueSt
//---------------------------------------------------------

QString MStyle::valueSt(StyleIdx idx) const
      {
      return value(idx).toString();
      }

//---------------------------------------------------------
//   valueB
//---------------------------------------------------------

bool MStyle::valueB(StyleIdx idx) const
      {
      return value(idx).toBool();
      }

//---------------------------------------------------------
//   valueD
//---------------------------------------------------------

qreal MStyle::valueD(StyleIdx idx) const
      {
      return value(idx).toDouble();
      }

//---------------------------------------------------------
//   valueI
//---------------------------------------------------------

int MStyle::valueI(StyleIdx idx) const
      {
      return value(idx).toInt();
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool MStyle::load(QFile* qf)
      {
      return d->load(qf);
      }

void MStyle::load(const QDomElement& e)
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
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(qf, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading style file %s at line %d column %d: %s\n",
               qf->fileName().toLatin1().data(), line, column, err.toLatin1().data());
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Style failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return false;
            }
      docName = qf->fileName();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  QString version = e.attribute(QString("version"));
                  QStringList sl  = version.split('.');
                  // _mscVersion  = sl[0].toInt() * 100 + sl[1].toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "Style")
                              load(ee);
                        else
                              domError(ee);
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

