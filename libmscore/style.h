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

#ifndef __STYLE_H__
#define __STYLE_H__

#include "chordlist.h"
#include "types.h"

namespace Ms {

enum class Pid : int;
class XmlWriter;
struct ChordDescription;
class Element;
class Score;

//---------------------------------------------------------
//   Sid
//
//    Keep in sync with styleTypes[] in style.cpp
//---------------------------------------------------------

enum class Sid {
      NOSTYLE = -1,

      pageWidth,
      pageHeight,
      pagePrintableWidth,
      pageEvenLeftMargin,
      pageOddLeftMargin,
      pageEvenTopMargin,
      pageEvenBottomMargin,
      pageOddTopMargin,
      pageOddBottomMargin,
      pageTwosided,

      pageSize,
      pageUnits,
      pageOrientation,
      pageFullWidth,
      pageFullHeight,
      marginOddLeft,
      marginOddRight,
      marginOddTop,
      marginOddBottom,
      marginEvenTop,
      marginEvenBottom,

      staffUpperBorder,
      staffLowerBorder,
      staffDistance,
      akkoladeDistance,

      minSystemDistance,
      maxSystemDistance,

      lyricsPlacement,
      lyricsPosAbove,
      lyricsPosBelow,
      lyricsMinTopDistance,
      lyricsMinBottomDistance,
      lyricsMinDistance,
      lyricsLineHeight,
      lyricsDashMinLength,
      lyricsDashMaxLength,
      lyricsDashMaxDistance,
      lyricsDashForce,
      lyricsAlignVerseNumber,
      lyricsLineThickness,
      lyricsMelismaAlign,
      lyricsMelismaPad,
      lyricsDashPad,
      lyricsDashLineThickness,
      lyricsDashYposRatio,

      lyricsOddFontFace,
      lyricsOddFontSize,
      lyricsOddFontSpatiumDependent,
      lyricsOddFontStyle,
      lyricsOddColor,
      lyricsOddAlign,
      lyricsOddFrameType,
      lyricsOddFramePadding,
      lyricsOddFrameWidth,
      lyricsOddFrameRound,
      lyricsOddFrameFgColor,
      lyricsOddFrameBgColor,

      lyricsEvenFontFace,
      lyricsEvenFontSize,
      lyricsEvenFontSpatiumDependent,
      lyricsEvenFontStyle,
      lyricsEvenColor,
      lyricsEvenAlign,
      lyricsEvenFrameType,
      lyricsEvenFramePadding,
      lyricsEvenFrameWidth,
      lyricsEvenFrameRound,
      lyricsEvenFrameFgColor,
      lyricsEvenFrameBgColor,

      figuredBassFontFamily,
//      figuredBassFontSize,
      figuredBassYOffset,
      figuredBassLineHeight,
      figuredBassAlignment,
      figuredBassStyle,
      systemFrameDistance,
      frameSystemDistance,
      minMeasureWidth,

      barWidth,
      doubleBarWidth,
      endBarWidth,
      doubleBarDistance,
      endBarDistance,
      repeatBarlineDotSeparation,
      repeatBarTips,
      startBarlineSingle,
      startBarlineMultiple,

      bracketWidth,
      bracketDistance,
      akkoladeWidth,
      akkoladeBarDistance,
      dividerLeft,
      dividerLeftSym,
      dividerLeftX,
      dividerLeftY,
      dividerRight,
      dividerRightSym,
      dividerRightX,
      dividerRightY,

      clefLeftMargin,
      keysigLeftMargin,
      ambitusMargin,
      timesigLeftMargin,
      timesigScale,

      clefKeyRightMargin,
      clefKeyDistance,
      clefTimesigDistance,
      keyTimesigDistance,
      keyBarlineDistance,
      systemHeaderDistance,
      systemHeaderTimeSigDistance,

      clefBarlineDistance,
      timesigBarlineDistance,

      stemWidth,
      shortenStem,
      shortStemProgression,
      shortestStem,
      beginRepeatLeftMargin,
      minNoteDistance,
      barNoteDistance,
      barAccidentalDistance,
      multiMeasureRestMargin,
      noteBarDistance,

      measureSpacing,
      staffLineWidth,
      ledgerLineWidth,
      ledgerLineLength,
      accidentalDistance,
      accidentalNoteDistance,
      beamWidth,
      beamDistance,
      beamMinLen,
      beamNoSlope,

      dotMag,
      dotNoteDistance,
      dotRestDistance,
      dotDotDistance,
      propertyDistanceHead,
      propertyDistanceStem,
      propertyDistance,
      articulationMag,
      articulationPosAbove,
      lastSystemFillLimit,

      hairpinPlacement,
      hairpinPosAbove,
      hairpinPosBelow,
      hairpinLinePosAbove,
      hairpinLinePosBelow,
      hairpinHeight,
      hairpinContHeight,
      hairpinLineWidth,
      hairpinFontFace,
      hairpinFontSize,
      hairpinFontSpatiumDependent,
      hairpinFontStyle,
      hairpinColor,
      hairpinTextAlign,
      hairpinFrameType,
      hairpinFramePadding,
      hairpinFrameWidth,
      hairpinFrameRound,
      hairpinFrameFgColor,
      hairpinFrameBgColor,
      hairpinText,
      hairpinCrescText,
      hairpinDecrescText,
      hairpinCrescContText,
      hairpinDecrescContText,
      hairpinLineStyle,
      hairpinLineLineStyle,

      pedalPlacement,
      pedalPosAbove,
      pedalPosBelow,
      pedalLineWidth,
      pedalLineStyle,
      pedalBeginTextOffset,
      pedalHookHeight,
      pedalFontFace,
      pedalFontSize,
      pedalFontSpatiumDependent,
      pedalFontStyle,
      pedalColor,
      pedalTextAlign,
      pedalFrameType,
      pedalFramePadding,
      pedalFrameWidth,
      pedalFrameRound,
      pedalFrameFgColor,
      pedalFrameBgColor,

      trillPlacement,
      trillPosAbove,
      trillPosBelow,

      vibratoPlacement,
      vibratoPosAbove,
      vibratoPosBelow,

      harmonyFretDist,
      minHarmonyDistance,
      maxHarmonyBarDistance,
      harmonyPlacement,

      chordSymbolAPosAbove,
      chordSymbolAPosBelow,

      chordSymbolBPosAbove,
      chordSymbolBPosBelow,

      chordSymbolAFontFace,
      chordSymbolAFontSize,
      chordSymbolAFontSpatiumDependent,
      chordSymbolAFontStyle,
      chordSymbolAColor,
      chordSymbolAAlign,
      chordSymbolAFrameType,
      chordSymbolAFramePadding,
      chordSymbolAFrameWidth,
      chordSymbolAFrameRound,
      chordSymbolAFrameFgColor,
      chordSymbolAFrameBgColor,

      chordSymbolBFontFace,
      chordSymbolBFontSize,
      chordSymbolBFontSpatiumDependent,
      chordSymbolBFontStyle,
      chordSymbolBColor,
      chordSymbolBAlign,
      chordSymbolBFrameType,
      chordSymbolBFramePadding,
      chordSymbolBFrameWidth,
      chordSymbolBFrameRound,
      chordSymbolBFrameFgColor,
      chordSymbolBFrameBgColor,

      capoPosition,
      fretNumMag,
      fretNumPos,
      fretY,
      fretMinDistance,
      fretMag,
      fretPlacement,
      fretStrings,
      fretFrets,
      fretOffset,
      fretBarre,

      showPageNumber,
      showPageNumberOne,
      pageNumberOddEven,
      showMeasureNumber,
      showMeasureNumberOne,
      measureNumberInterval,
      measureNumberSystem,

      measureNumberAllStaffs,
      smallNoteMag,
      graceNoteMag,
      smallStaffMag,
      smallClefMag,
      genClef,
      genKeysig,
      genCourtesyTimesig,
      genCourtesyKeysig,
      genCourtesyClef,

      swingRatio,
      swingUnit,

      useStandardNoteNames,
      useGermanNoteNames,
      useFullGermanNoteNames,
      useSolfeggioNoteNames,
      useFrenchNoteNames,
      automaticCapitalization,
      lowerCaseMinorChords,
      lowerCaseBassNotes,
      allCapsNoteNames,
      chordStyle,
      chordsXmlFile,
      chordDescriptionFile,
      concertPitch,
      createMultiMeasureRests,
      minEmptyMeasures,
      minMMRestWidth,
      hideEmptyStaves,
      dontHideStavesInFirstSystem,
      hideInstrumentNameIfOneInstrument,
      gateTime,
      tenutoGateTime,
      staccatoGateTime,
      slurGateTime,

      ArpeggioNoteDistance,
      ArpeggioLineWidth,
      ArpeggioHookLen,
      ArpeggioHiddenInStdIfTab,

      SlurEndWidth,
      SlurMidWidth,
      SlurDottedWidth,
      MinTieLength,
      SlurMinDistance,

      SectionPause,
      MusicalSymbolFont,
      MusicalTextFont,

      showHeader,
      headerFirstPage,
      headerOddEven,
      evenHeaderL,
      evenHeaderC,
      evenHeaderR,
      oddHeaderL,
      oddHeaderC,
      oddHeaderR,

      showFooter,
      footerFirstPage,
      footerOddEven,
      evenFooterL,
      evenFooterC,
      evenFooterR,
      oddFooterL,
      oddFooterC,
      oddFooterR,

      voltaPosAbove,
      voltaHook,
      voltaLineWidth,
      voltaLineStyle,
      voltaFontFace,
      voltaFontSize,
      voltaFontSpatiumDependent,
      voltaFontStyle,
      voltaColor,
      voltaAlign,
      voltaOffset,
      voltaFrameType,
      voltaFramePadding,
      voltaFrameWidth,
      voltaFrameRound,
      voltaFrameFgColor,
      voltaFrameBgColor,

      ottava8VAPlacement,
      ottava8VBPlacement,
      ottava15MAPlacement,
      ottava15MBPlacement,
      ottava22MAPlacement,
      ottava22MBPlacement,

      ottava8VAText,
      ottava8VBText,
      ottava15MAText,
      ottava15MBText,
      ottava22MAText,
      ottava22MBText,

      ottava8VAnoText,
      ottava8VBnoText,
      ottava15MAnoText,
      ottava15MBnoText,
      ottava22MAnoText,
      ottava22MBnoText,

      ottavaPosAbove,
      ottavaPosBelow,
      ottavaHookAbove,
      ottavaHookBelow,
      ottavaLineWidth,
      ottavaLineStyle,
      ottavaNumbersOnly,
      ottavaFontFace,
      ottavaFontSize,
      ottavaFontSpatiumDependent,
      ottavaFontStyle,
      ottavaColor,
      ottavaTextAlign,
      ottavaFrameType,
      ottavaFramePadding,
      ottavaFrameWidth,
      ottavaFrameRound,
      ottavaFrameFgColor,
      ottavaFrameBgColor,

      tabClef,

      tremoloWidth,
      tremoloBoxHeight,
      tremoloStrokeWidth,
      tremoloDistance,
      // TODO tremoloBeamLengthMultiplier,
      // TODO tremoloMaxBeamLength,

      linearStretch,
      crossMeasureValues,
      keySigNaturals,

      tupletMaxSlope,
      tupletOufOfStaff,
      tupletVHeadDistance,
      tupletVStemDistance,
      tupletStemLeftDistance,
      tupletStemRightDistance,
      tupletNoteLeftDistance,
      tupletNoteRightDistance,
      tupletBracketWidth,
      tupletDirection,
      tupletNumberType,
      tupletBracketType,
      tupletFontFace,
      tupletFontSize,
      tupletFontSpatiumDependent,
      tupletFontStyle,
      tupletColor,
      tupletAlign,
      tupletBracketHookHeight,
      tupletOffset,
      tupletFrameType,
      tupletFramePadding,
      tupletFrameWidth,
      tupletFrameRound,
      tupletFrameFgColor,
      tupletFrameBgColor,

      barreLineWidth,
      scaleBarlines,
      barGraceDistance,

      minVerticalDistance,
      ornamentStyle,
      spatium,

      autoplaceHairpinDynamicsDistance,

      dynamicsPlacement,
      dynamicsPosAbove,
      dynamicsPosBelow,

      dynamicsMinDistance,
      autoplaceVerticalAlignRange,

      textLinePlacement,
      textLinePosAbove,
      textLinePosBelow,
      textLineFrameType,
      textLineFramePadding,
      textLineFrameWidth,
      textLineFrameRound,
      textLineFrameFgColor,
      textLineFrameBgColor,

      tremoloBarLineWidth,
      jumpPosAbove,
      markerPosAbove,

      defaultFontFace,
      defaultFontSize,
      defaultFontSpatiumDependent,
      defaultFontStyle,
      defaultColor,
      defaultAlign,
      defaultFrameType,
      defaultFramePadding,
      defaultFrameWidth,
      defaultFrameRound,
      defaultFrameFgColor,
      defaultFrameBgColor,
      defaultOffset,
      defaultOffsetType,
      defaultSystemFlag,
      defaultText,

      titleFontFace,
      titleFontSize,
      titleFontSpatiumDependent,
      titleFontStyle,
      titleColor,
      titleAlign,
      titleOffset,
      titleOffsetType,
      titleFrameType,
      titleFramePadding,
      titleFrameWidth,
      titleFrameRound,
      titleFrameFgColor,
      titleFrameBgColor,

      subTitleFontFace,
      subTitleFontSize,
      subTitleFontSpatiumDependent,
      subTitleFontStyle,
      subTitleColor,
      subTitleAlign,
      subTitleOffset,
      subTitleOffsetType,
      subTitleFrameType,
      subTitleFramePadding,
      subTitleFrameWidth,
      subTitleFrameRound,
      subTitleFrameFgColor,
      subTitleFrameBgColor,

      composerFontFace,
      composerFontSize,
      composerFontSpatiumDependent,
      composerFontStyle,
      composerColor,
      composerAlign,
      composerOffset,
      composerOffsetType,
      composerFrameType,
      composerFramePadding,
      composerFrameWidth,
      composerFrameRound,
      composerFrameFgColor,
      composerFrameBgColor,

      lyricistFontFace,
      lyricistFontSize,
      lyricistFontSpatiumDependent,
      lyricistFontStyle,
      lyricistColor,
      lyricistAlign,
      lyricistOffset,
      lyricistOffsetType,
      lyricistFrameType,
      lyricistFramePadding,
      lyricistFrameWidth,
      lyricistFrameRound,
      lyricistFrameFgColor,
      lyricistFrameBgColor,

      fingeringFontFace,
      fingeringFontSize,
      fingeringFontSpatiumDependent,
      fingeringFontStyle,
      fingeringColor,
      fingeringAlign,
      fingeringFrameType,
      fingeringFramePadding,
      fingeringFrameWidth,
      fingeringFrameRound,
      fingeringFrameFgColor,
      fingeringFrameBgColor,
      fingeringOffset,

      lhGuitarFingeringFontFace,
      lhGuitarFingeringFontSize,
      lhGuitarFingeringFontSpatiumDependent,
      lhGuitarFingeringFontStyle,
      lhGuitarFingeringColor,
      lhGuitarFingeringAlign,
      lhGuitarFingeringFrameType,
      lhGuitarFingeringFramePadding,
      lhGuitarFingeringFrameWidth,
      lhGuitarFingeringFrameRound,
      lhGuitarFingeringFrameFgColor,
      lhGuitarFingeringFrameBgColor,
      lhGuitarFingeringOffset,

      rhGuitarFingeringFontFace,
      rhGuitarFingeringFontSize,
      rhGuitarFingeringFontSpatiumDependent,
      rhGuitarFingeringFontStyle,
      rhGuitarFingeringColor,
      rhGuitarFingeringAlign,
      rhGuitarFingeringFrameType,
      rhGuitarFingeringFramePadding,
      rhGuitarFingeringFrameWidth,
      rhGuitarFingeringFrameRound,
      rhGuitarFingeringFrameFgColor,
      rhGuitarFingeringFrameBgColor,
      rhGuitarFingeringOffset,

      stringNumberFontFace,
      stringNumberFontSize,
      stringNumberFontSpatiumDependent,
      stringNumberFontStyle,
      stringNumberColor,
      stringNumberAlign,
      stringNumberFrameType,
      stringNumberFramePadding,
      stringNumberFrameWidth,
      stringNumberFrameRound,
      stringNumberFrameFgColor,
      stringNumberFrameBgColor,
      stringNumberOffset,

      longInstrumentFontFace,
      longInstrumentFontSize,
      longInstrumentFontSpatiumDependent,
      longInstrumentFontStyle,
      longInstrumentColor,
      longInstrumentAlign,
      longInstrumentOffset,
      longInstrumentFrameType,
      longInstrumentFramePadding,
      longInstrumentFrameWidth,
      longInstrumentFrameRound,
      longInstrumentFrameFgColor,
      longInstrumentFrameBgColor,

      shortInstrumentFontFace,
      shortInstrumentFontSize,
      shortInstrumentFontSpatiumDependent,
      shortInstrumentFontStyle,
      shortInstrumentColor,
      shortInstrumentAlign,
      shortInstrumentOffset,
      shortInstrumentFrameType,
      shortInstrumentFramePadding,
      shortInstrumentFrameWidth,
      shortInstrumentFrameRound,
      shortInstrumentFrameFgColor,
      shortInstrumentFrameBgColor,

      partInstrumentFontFace,
      partInstrumentFontSize,
      partInstrumentFontSpatiumDependent,
      partInstrumentFontStyle,
      partInstrumentColor,
      partInstrumentAlign,
      partInstrumentOffset,
      partInstrumentFrameType,
      partInstrumentFramePadding,
      partInstrumentFrameWidth,
      partInstrumentFrameRound,
      partInstrumentFrameFgColor,
      partInstrumentFrameBgColor,

      dynamicsFontFace,
      dynamicsFontSize,
      dynamicsFontSpatiumDependent,
      dynamicsFontStyle,
      dynamicsColor,
      dynamicsAlign,
      dynamicsFrameType,
      dynamicsFramePadding,
      dynamicsFrameWidth,
      dynamicsFrameRound,
      dynamicsFrameFgColor,
      dynamicsFrameBgColor,

      expressionFontFace,
      expressionFontSize,
      expressionFontSpatiumDependent,
      expressionFontStyle,
      expressionColor,
      expressionAlign,
      expressionPlacement,
      expressionOffset,
      expressionFrameType,
      expressionFramePadding,
      expressionFrameWidth,
      expressionFrameRound,
      expressionFrameFgColor,
      expressionFrameBgColor,

      tempoFontFace,
      tempoFontSize,
      tempoFontSpatiumDependent,
      tempoFontStyle,
      tempoColor,
      tempoAlign,
      tempoSystemFlag,
      tempoPlacement,
      tempoPosAbove,
      tempoPosBelow,
      tempoMinDistance,
      tempoFrameType,
      tempoFramePadding,
      tempoFrameWidth,
      tempoFrameRound,
      tempoFrameFgColor,
      tempoFrameBgColor,

      metronomeFontFace,
      metronomeFontSize,
      metronomeFontSpatiumDependent,
      metronomeFontStyle,
      metronomeColor,
      metronomePlacement,
      metronomeAlign,
      metronomeOffset,
      metronomeFrameType,
      metronomeFramePadding,
      metronomeFrameWidth,
      metronomeFrameRound,
      metronomeFrameFgColor,
      metronomeFrameBgColor,

      measureNumberFontFace,
      measureNumberFontSize,
      measureNumberFontSpatiumDependent,
      measureNumberFontStyle,
      measureNumberColor,
      measureNumberOffset,
      measureNumberOffsetType,
      measureNumberAlign,
      measureNumberFrameType,
      measureNumberFramePadding,
      measureNumberFrameWidth,
      measureNumberFrameRound,
      measureNumberFrameFgColor,
      measureNumberFrameBgColor,

      translatorFontFace,
      translatorFontSize,
      translatorFontSpatiumDependent,
      translatorFontStyle,
      translatorColor,
      translatorAlign,
      translatorOffset,
      translatorFrameType,
      translatorFramePadding,
      translatorFrameWidth,
      translatorFrameRound,
      translatorFrameFgColor,
      translatorFrameBgColor,

      systemTextFontFace,
      systemTextFontSize,
      systemTextFontSpatiumDependent,
      systemTextFontStyle,
      systemTextColor,
      systemTextAlign,
      systemTextOffsetType,
      systemTextPlacement,
      systemTextPosAbove,
      systemTextPosBelow,
      systemTextMinDistance,
      systemTextFrameType,
      systemTextFramePadding,
      systemTextFrameWidth,
      systemTextFrameRound,
      systemTextFrameFgColor,
      systemTextFrameBgColor,

      staffTextFontFace,
      staffTextFontSize,
      staffTextFontSpatiumDependent,
      staffTextFontStyle,
      staffTextColor,
      staffTextAlign,
      staffTextOffsetType,
      staffTextPlacement,
      staffTextPosAbove,
      staffTextPosBelow,
      staffTextMinDistance,
      staffTextFrameType,
      staffTextFramePadding,
      staffTextFrameWidth,
      staffTextFrameRound,
      staffTextFrameFgColor,
      staffTextFrameBgColor,

      rehearsalMarkFontFace,
      rehearsalMarkFontSize,
      rehearsalMarkFontSpatiumDependent,
      rehearsalMarkFontStyle,
      rehearsalMarkColor,
      rehearsalMarkAlign,
      rehearsalMarkFrameType,
      rehearsalMarkFramePadding,
      rehearsalMarkFrameWidth,
      rehearsalMarkFrameRound,
      rehearsalMarkFrameFgColor,
      rehearsalMarkFrameBgColor,
      rehearsalMarkPlacement,
      rehearsalMarkPosAbove,
      rehearsalMarkPosBelow,
      rehearsalMarkMinDistance,

      repeatLeftFontFace,
      repeatLeftFontSize,
      repeatLeftFontSpatiumDependent,
      repeatLeftFontStyle,
      repeatLeftColor,
      repeatLeftAlign,
      repeatLeftPlacement,
      repeatLeftFrameType,
      repeatLeftFramePadding,
      repeatLeftFrameWidth,
      repeatLeftFrameRound,
      repeatLeftFrameFgColor,
      repeatLeftFrameBgColor,

      repeatRightFontFace,
      repeatRightFontSize,
      repeatRightFontSpatiumDependent,
      repeatRightFontStyle,
      repeatRightColor,
      repeatRightAlign,
      repeatRightPlacement,
      repeatRightFrameType,
      repeatRightFramePadding,
      repeatRightFrameWidth,
      repeatRightFrameRound,
      repeatRightFrameFgColor,
      repeatRightFrameBgColor,

      frameFontFace,
      frameFontSize,
      frameFontSpatiumDependent,
      frameFontStyle,
      frameColor,
      frameAlign,
      frameOffset,
      frameFrameType,
      frameFramePadding,
      frameFrameWidth,
      frameFrameRound,
      frameFrameFgColor,
      frameFrameBgColor,

      textLineFontFace,
      textLineFontSize,
      textLineFontSpatiumDependent,
      textLineFontStyle,
      textLineColor,
      textLineTextAlign,

      glissandoFontFace,
      glissandoFontSize,
      glissandoFontSpatiumDependent,
      glissandoFontStyle,
      glissandoColor,
      glissandoAlign,
      glissandoOffset,
      glissandoFrameType,
      glissandoFramePadding,
      glissandoFrameWidth,
      glissandoFrameRound,
      glissandoFrameFgColor,
      glissandoFrameBgColor,
      glissandoLineWidth,
      glissandoText,

      bendFontFace,
      bendFontSize,
      bendFontSpatiumDependent,
      bendFontStyle,
      bendColor,
      bendAlign,
      bendOffset,
      bendFrameType,
      bendFramePadding,
      bendFrameWidth,
      bendFrameRound,
      bendFrameFgColor,
      bendFrameBgColor,
      bendLineWidth,
      bendArrowWidth,

      headerFontFace,
      headerFontSize,
      headerFontSpatiumDependent,
      headerFontStyle,
      headerColor,
      headerAlign,
      headerOffset,
      headerFrameType,
      headerFramePadding,
      headerFrameWidth,
      headerFrameRound,
      headerFrameFgColor,
      headerFrameBgColor,

      footerFontFace,
      footerFontSize,
      footerFontSpatiumDependent,
      footerFontStyle,
      footerColor,
      footerAlign,
      footerOffset,
      footerFrameType,
      footerFramePadding,
      footerFrameWidth,
      footerFrameRound,
      footerFrameFgColor,
      footerFrameBgColor,

      instrumentChangeFontFace,
      instrumentChangeFontSize,
      instrumentChangeFontSpatiumDependent,
      instrumentChangeFontStyle,
      instrumentChangeColor,
      instrumentChangeAlign,
      instrumentChangeOffset,
      instrumentChangePlacement,
      instrumentChangePosAbove,
      instrumentChangePosBelow,
      instrumentChangeMinDistance,
      instrumentChangeFrameType,
      instrumentChangeFramePadding,
      instrumentChangeFrameWidth,
      instrumentChangeFrameRound,
      instrumentChangeFrameFgColor,
      instrumentChangeFrameBgColor,

      figuredBassFontFace,
      figuredBassFontSize,
      figuredBassFontSpatiumDependent,
      figuredBassFontStyle,
      figuredBassColor,

      user1Name,
      user1FontFace,
      user1FontSize,
      user1FontSpatiumDependent,
      user1FontStyle,
      user1Color,
      user1Align,
      user1Offset,
      user1OffsetType,
      user1FrameType,
      user1FramePadding,
      user1FrameWidth,
      user1FrameRound,
      user1FrameFgColor,
      user1FrameBgColor,

      user2Name,
      user2FontFace,
      user2FontSize,
      user2FontSpatiumDependent,
      user2FontStyle,
      user2Color,
      user2Align,
      user2Offset,
      user2OffsetType,
      user2FrameType,
      user2FramePadding,
      user2FrameWidth,
      user2FrameRound,
      user2FrameFgColor,
      user2FrameBgColor,

      user3Name,
      user3FontFace,
      user3FontSize,
      user3FontSpatiumDependent,
      user3FontStyle,
      user3Color,
      user3Align,
      user3Offset,
      user3OffsetType,
      user3FrameType,
      user3FramePadding,
      user3FrameWidth,
      user3FrameRound,
      user3FrameFgColor,
      user3FrameBgColor,

      user4Name,
      user4FontFace,
      user4FontSize,
      user4FontSpatiumDependent,
      user4FontStyle,
      user4Color,
      user4Align,
      user4Offset,
      user4OffsetType,
      user4FrameType,
      user4FramePadding,
      user4FrameWidth,
      user4FrameRound,
      user4FrameFgColor,
      user4FrameBgColor,

      user5Name,
      user5FontFace,
      user5FontSize,
      user5FontSpatiumDependent,
      user5FontStyle,
      user5Color,
      user5Align,
      user5Offset,
      user5OffsetType,
      user5FrameType,
      user5FramePadding,
      user5FrameWidth,
      user5FrameRound,
      user5FrameFgColor,
      user5FrameBgColor,

      user6Name,
      user6FontFace,
      user6FontSize,
      user6FontSpatiumDependent,
      user6FontStyle,
      user6Color,
      user6Align,
      user6Offset,
      user6OffsetType,
      user6FrameType,
      user6FramePadding,
      user6FrameWidth,
      user6FrameRound,
      user6FrameFgColor,
      user6FrameBgColor,

      letRingFontFace,
      letRingFontSize,
      letRingFontSpatiumDependent,
      letRingFontStyle,
      letRingColor,
      letRingTextAlign,
      letRingHookHeight,
      letRingPlacement,
      letRingPosAbove,
      letRingPosBelow,
      letRingLineWidth,
      letRingLineStyle,
      letRingBeginTextOffset,
      letRingText,
      letRingFrameType,
      letRingFramePadding,
      letRingFrameWidth,
      letRingFrameRound,
      letRingFrameFgColor,
      letRingFrameBgColor,

      palmMuteFontFace,
      palmMuteFontSize,
      palmMuteFontSpatiumDependent,
      palmMuteFontStyle,
      palmMuteColor,
      palmMuteTextAlign,
      palmMuteHookHeight,
      palmMutePlacement,
      palmMutePosAbove,
      palmMutePosBelow,
      palmMuteLineWidth,
      palmMuteLineStyle,
      palmMuteBeginTextOffset,
      palmMuteText,
      palmMuteFrameType,
      palmMuteFramePadding,
      palmMuteFrameWidth,
      palmMuteFrameRound,
      palmMuteFrameFgColor,
      palmMuteFrameBgColor,

      fermataPosAbove,
      fermataPosBelow,
      fermataMinDistance,

      fingeringPlacement,

      STYLES
      };

static constexpr qreal INCH      = 25.4;                // millimeters per inch
static constexpr qreal PPI       = 72.0;                // PostScript points per inch
static constexpr qreal DPI_F     = 5;                   // internal resolution factor
static constexpr qreal DPI       = DPI_F * PPI;
static constexpr qreal SPATIUM20 = DPI_F * (DPI / PPI); // == DPI_F * DPI_F
static constexpr qreal DPMM      = DPI / INCH;
static constexpr qreal DIDOT     = 1.06574601373228;    // didot per point
static constexpr qreal CICERO    = 12.0;                // didot per cicero and points per pica
static constexpr qreal MSCX_F    = 10000;               // page settings stored in points * MSCX_F

//---------------------------------------------------------
//   PageUnit
//    Units require a custom structure for text suffixes and
//    widget steps because Qt does not provide them.
//---------------------------------------------------------
struct PageUnit {
      const char* _key;   // never translated
      const char* _name;
      const char* _suffix;
      int _decimals;      // number of decimal places for QDoubleSpinBox
      qreal _step;        // for spin widgets in specific units
      qreal _stepSpatium; // ditto
      qreal _max;         // max width and height value
      qreal _factor;      // conversion-to-points factor

public:
      const char* key()    const { return _key;             }
      const char* name()   const { return _name;            }
      const char* suffix() const { return _suffix;          }
      int decimals()       const { return _decimals;        }
      qreal step()         const { return _step;            }
      qreal stepSpatium()  const { return _stepSpatium;     }
      qreal max()          const { return _max;             }
      qreal factor()       const { return _factor * MSCX_F; }
      qreal paintFactor()  const { return _factor * DPI_F;  }
      };

//---------------------------------------------------------
//   pageUnits - an array of PageUnit instances
//---------------------------------------------------------
const PageUnit pageUnits[] = { 
      { "Millimeters",  QT_TRANSLATE_NOOP("unitName", "Millimeters"),  QT_TRANSLATE_NOOP("unitSuffix", "mm"), 2, 1.0,  0.2,    9999.99, PPI / INCH     },
      { "Points",       QT_TRANSLATE_NOOP("unitName", "Points"),       QT_TRANSLATE_NOOP("unitSuffix", "pt"), 0, 1.0,  0.2,   99999.99, 1.0            },
      { "Inches",       QT_TRANSLATE_NOOP("unitName", "Inches"),       QT_TRANSLATE_NOOP("unitSuffix", "in"), 2, 0.05, 0.005,  2000.99, PPI            },
      { "Picas",        QT_TRANSLATE_NOOP("unitName", "Picas"),        QT_TRANSLATE_NOOP("unitSuffix", "p" ), 2, 1.0,  0.1,    9999.99, CICERO         },
      { "Didot",        QT_TRANSLATE_NOOP("unitName", "Didot"),        QT_TRANSLATE_NOOP("unitSuffix", "dd"), 0, 1.0,  0.005, 99999.99, DIDOT          },
      { "Cicero",       QT_TRANSLATE_NOOP("unitName", "Cicero"),       QT_TRANSLATE_NOOP("unitSuffix", "c" ), 2, 1.0,  0.1,    9999.99, DIDOT * CICERO },
      { "Staff Spaces", QT_TRANSLATE_NOOP("unitName", "Staff Spaces"), QT_TRANSLATE_NOOP("unitSuffix", "sp"), 2, 0.1,  0.01,   9999.99, 0              }, // factor must be set, somehow, if ever used
      { "Pixels",       QT_TRANSLATE_NOOP("unitName", "Pixels"),       QT_TRANSLATE_NOOP("unitSuffix", "px"), 2, 1.0,  1.0,   99999.99, 1.0 / DPI_F    }  // if ever used...
      };

// For human readable .mscx files
const QString pageOrient[] = { QString("Portrait"), QString("Landscape") };

class MPageLayout : public QPageLayout {
   private:
      double factor()      const{ return pageUnits[int(units())].factor();      }
      double paintFactor() const{ return pageUnits[int(units())].paintFactor(); }
   public:
      double width()        const { return fullRect().width()  * paintFactor(); }
      double height()       const { return fullRect().height() * paintFactor(); }
      double paintWidth()   const { return paintRect().width() * paintFactor(); }
      double leftMargin()   const { return margins().left()    * paintFactor(); }
      double rightMargin()  const { return margins().right()   * paintFactor(); }
      double topMargin()    const { return margins().top()     * paintFactor(); }
      double bottomMargin() const { return margins().bottom()  * paintFactor(); }

      // "Points" is really QPageSize::Point / MSCX_F == 0.0001pt
      int widthPoints()        const { return lrint(fullRect().width()  * factor()); }
      int heightPoints()       const { return lrint(fullRect().height() * factor()); }
      int leftMarginPoints()   const { return lrint(margins().left()    * factor()); }
      int rightMarginPoints()  const { return lrint(margins().right()   * factor()); }
      int topMarginPoints()    const { return lrint(margins().top()     * factor()); }
      int bottomMarginPoints() const { return lrint(margins().bottom()  * factor()); }
      };

//---------------------------------------------------------
//   MStyle
//    the name "Style" gives problems with some microsoft
//    header files...
//---------------------------------------------------------

class MStyle {
      std::array<QVariant, int(Sid::STYLES)> _values;
      std::array<qreal, int(Sid::STYLES)> _precomputedValues;

      QPageSize _pageSize;
      MPageLayout _pageOdd;
      MPageLayout _pageEven;
      bool _isMMInch;       // for 3.01 to 3.01+ conversion

      ChordList _chordList;
      bool _customChordList;        // if true, chordlist will be saved as part of score

   public:
      MStyle();

      void precomputeValues();
      void initPageLayout();
      QVariant value(Sid idx) const;
      qreal pvalue(Sid idx) const    { return _precomputedValues[int(idx)]; }
      void set(Sid idx, const QVariant& v);

      QPageSize& pageSize()   { return _pageSize; }
      MPageLayout& pageOdd()  { return _pageOdd;  }
      MPageLayout& pageEven() { return _pageEven; }
      const QPageSize& pageSize() const   { return _pageSize; }
      const MPageLayout& pageOdd() const  { return _pageOdd;  }
      const MPageLayout& pageEven() const { return _pageEven; }
      void setPageSize(QPageSize& val)    { _pageSize = val; }
      void setPageOdd(MPageLayout& val)   { _pageOdd  = val; }
      void setPageEven(MPageLayout& val)  { _pageEven = val; }
      bool isDefault(Sid idx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList()  { return &_chordList; }
      void setChordList(ChordList*, bool custom = true);    // Style gets ownership of ChordList
      void setCustomChordList(bool t) { _customChordList = t; }

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(XmlWriter& xml, bool optimize);
      bool readProperties(XmlReader&);
      bool readStyleValCompat(XmlReader&);
      bool readTextStyleValCompat(XmlReader&);

      void fromPageLayout();
      void toPageLayout();
      void spatium301(XmlReader& e);

      bool fuzzyCompare(Sid sid, double pageValue, double epsilon = 0.01);

      static bool isMetric(QPageLayout::Unit unit);
      static const char* valueType(const Sid);
      static const char* valueName(const Sid);
      static Sid styleIdx(const QString& name);
      };

//---------------------------------------------------------
//   StyledProperty
//---------------------------------------------------------

struct StyledProperty {
      Sid sid;
      Pid pid;
      };

typedef std::vector<StyledProperty> ElementStyle;

#define TEXT_STYLE_SIZE 13

typedef std::array<StyledProperty, TEXT_STYLE_SIZE> TextStyle;


const TextStyle* textStyle(Tid);
const TextStyle* textStyle(const char*);

const char* textStyleName(Tid);
QString textStyleUserName(Tid);
Tid textStyleFromName(const QString&);

const std::vector<Tid>& allTextStyles();
const std::vector<Tid>& primaryTextStyles();

#ifndef NDEBUG
extern void checkStyles();
#endif

}     // namespace Ms


#endif
