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

namespace Ms {

enum class P_ID : int;
class XmlWriter;
struct ChordDescription;
class Element;

//---------------------------------------------------------
//   StyleIdx
//
//    Keep in sync with styleTypes[] in style.cpp
//---------------------------------------------------------

enum class StyleIdx {
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
      lyricsLineHeight,
      lyricsDashMinLength,
      lyricsDashMaxLength,
      lyricsDashMaxDistance,
      lyricsDashForce,
      lyricsAlignVerseNumber,
      lyricsLineThickness,

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
      lastSystemFillLimit,

      hairpinPlacement,
      hairpinPosAbove,
      hairpinPosBelow,
      hairpinHeight,
      hairpinContHeight,
      hairpinLineWidth,

      pedalPlacement,
      pedalPosAbove,
      pedalPosBelow,
      pedalLineWidth,
      pedalLineStyle,
      pedalBeginTextOffset,
      pedalHookHeight,

      trillPlacement,
      trillPosAbove,
      trillPosBelow,

      vibratoPlacement,
      vibratoPosAbove,
      vibratoPosBelow,

      harmonyY,
      harmonyFretDist,
      minHarmonyDistance,
      maxHarmonyBarDistance,
      capoPosition,
      fretNumMag,
      fretNumPos,
      fretY,
      fretMinDistance,

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

      voltaY,
      voltaHook,
      voltaLineWidth,
      voltaLineStyle,

      ottavaPlacement,
      ottavaPosAbove,
      ottavaPosBelow,
      ottavaHook,
      ottavaLineWidth,
      ottavaLineStyle,
      ottavaNumbersOnly,

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

      barreLineWidth,
      fretMag,
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

      tremoloBarLineWidth,
      jumpPosAbove,
      markerPosAbove,

      defaultFontFace,
      defaultFontSize,
      defaultFontSpatiumDependent,
      defaultFontBold,
      defaultFontItalic,
      defaultFontUnderline,
      defaultAlign,
      defaultFrame,
      defaultFrameSquare,
      defaultFrameCircle,
      defaultFramePadding,
      defaultFrameWidth,
      defaultFrameRound,
      defaultFrameFgColor,
      defaultFrameBgColor,
      defaultOffset,
      defaultOffsetType,
      defaultSystemFlag,

      titleFontFace,
      titleFontSize,
      titleFontSpatiumDependent,
      titleFontBold,
      titleFontItalic,
      titleFontUnderline,
      titleAlign,
      titleOffset,
      titleOffsetType,

      subTitleFontFace,
      subTitleFontSize,
      subTitleFontSpatiumDependent,
      subTitleFontBold,
      subTitleFontItalic,
      subTitleFontUnderline,
      subTitleAlign,
      subTitleOffset,
      subTitleOffsetType,

      composerFontFace,
      composerFontSize,
      composerFontSpatiumDependent,
      composerFontBold,
      composerFontItalic,
      composerFontUnderline,
      composerAlign,
      composerOffset,
      composerOffsetType,

      lyricistFontFace,
      lyricistFontSize,
      lyricistFontSpatiumDependent,
      lyricistFontBold,
      lyricistFontItalic,
      lyricistFontUnderline,
      lyricistAlign,
      lyricistOffset,
      lyricistOffsetType,

      lyricsOddFontFace,
      lyricsOddFontSize,
      lyricsOddFontBold,
      lyricsOddFontItalic,
      lyricsOddFontUnderline,
      lyricsOddAlign,
      lyricsOddOffset,

      lyricsEvenFontFace,
      lyricsEvenFontSize,
      lyricsEvenFontBold,
      lyricsEvenFontItalic,
      lyricsEvenFontUnderline,
      lyricsEvenAlign,
      lyricsEvenOffset,

      fingeringFontFace,
      fingeringFontSize,
      fingeringFontBold,
      fingeringFontItalic,
      fingeringFontUnderline,
      fingeringAlign,
      fingeringFrame,
      fingeringFrameSquare,
      fingeringFrameCircle,
      fingeringFramePadding,
      fingeringFrameWidth,
      fingeringFrameRound,
      fingeringFrameFgColor,
      fingeringFrameBgColor,
      fingeringOffset,

      lhGuitarFingeringFontFace,
      lhGuitarFingeringFontSize,
      lhGuitarFingeringFontBold,
      lhGuitarFingeringFontItalic,
      lhGuitarFingeringFontUnderline,
      lhGuitarFingeringAlign,
      lhGuitarFingeringFrame,
      lhGuitarFingeringFrameSquare,
      lhGuitarFingeringFrameCircle,
      lhGuitarFingeringFramePadding,
      lhGuitarFingeringFrameWidth,
      lhGuitarFingeringFrameRound,
      lhGuitarFingeringFrameFgColor,
      lhGuitarFingeringFrameBgColor,
      lhGuitarFingeringOffset,

      rhGuitarFingeringFontFace,
      rhGuitarFingeringFontSize,
      rhGuitarFingeringFontBold,
      rhGuitarFingeringFontItalic,
      rhGuitarFingeringFontUnderline,
      rhGuitarFingeringAlign,
      rhGuitarFingeringFrame,
      rhGuitarFingeringFrameSquare,
      rhGuitarFingeringFrameCircle,
      rhGuitarFingeringFramePadding,
      rhGuitarFingeringFrameWidth,
      rhGuitarFingeringFrameRound,
      rhGuitarFingeringFrameFgColor,
      rhGuitarFingeringFrameBgColor,
      rhGuitarFingeringOffset,

      stringNumberFontFace,
      stringNumberFontSize,
      stringNumberFontBold,
      stringNumberFontItalic,
      stringNumberFontUnderline,
      stringNumberAlign,
      stringNumberFrame,
      stringNumberFrameSquare,
      stringNumberFrameCircle,
      stringNumberFramePadding,
      stringNumberFrameWidth,
      stringNumberFrameRound,
      stringNumberFrameFgColor,
      stringNumberFrameBgColor,
      stringNumberOffset,

      longInstrumentFontFace,
      longInstrumentFontSize,
      longInstrumentFontBold,
      longInstrumentFontItalic,
      longInstrumentFontUnderline,
      longInstrumentAlign,

      shortInstrumentFontFace,
      shortInstrumentFontSize,
      shortInstrumentFontBold,
      shortInstrumentFontItalic,
      shortInstrumentFontUnderline,
      shortInstrumentAlign,

      partInstrumentFontFace,
      partInstrumentFontSize,
      partInstrumentFontBold,
      partInstrumentFontItalic,
      partInstrumentFontUnderline,

      dynamicsFontFace,
      dynamicsFontSize,
      dynamicsFontBold,
      dynamicsFontItalic,
      dynamicsFontUnderline,
      dynamicsAlign,

      expressionFontFace,
      expressionFontSize,
      expressionFontBold,
      expressionFontItalic,
      expressionFontUnderline,
      expressionAlign,

      tempoFontFace,
      tempoFontSize,
      tempoFontBold,
      tempoFontItalic,
      tempoFontUnderline,
      tempoAlign,
      tempoOffset,
      tempoSystemFlag,
      tempoPlacement,
      tempoPosAbove,
      tempoPosBelow,
      tempoMinDistance,

      metronomeFontFace,
      metronomeFontSize,
      metronomeFontBold,
      metronomeFontItalic,
      metronomeFontUnderline,

      measureNumberFontFace,
      measureNumberFontSize,
      measureNumberFontBold,
      measureNumberFontItalic,
      measureNumberFontUnderline,
      measureNumberOffset,
      measureNumberOffsetType,

      translatorFontFace,
      translatorFontSize,
      translatorFontBold,
      translatorFontItalic,
      translatorFontUnderline,

      tupletFontFace,
      tupletFontSize,
      tupletFontBold,
      tupletFontItalic,
      tupletFontUnderline,
      tupletAlign,

      systemFontFace,
      systemFontSize,
      systemFontBold,
      systemFontItalic,
      systemFontUnderline,
      systemOffset,
      systemOffsetType,
      systemAlign,

      staffFontFace,
      staffFontSize,
      staffFontBold,
      staffFontItalic,
      staffFontUnderline,
      staffOffset,
      staffOffsetType,

      chordSymbolFontFace,
      chordSymbolFontSize,
      chordSymbolFontBold,
      chordSymbolFontItalic,
      chordSymbolFontUnderline,
      chordSymbolAlign,

      rehearsalMarkFontFace,
      rehearsalMarkFontSize,
      rehearsalMarkFontBold,
      rehearsalMarkFontItalic,
      rehearsalMarkFontUnderline,
      rehearsalMarkAlign,
      rehearsalMarkFrame,
      rehearsalMarkFrameSquare,
      rehearsalMarkFrameCircle,
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
      repeatLeftFontBold,
      repeatLeftFontItalic,
      repeatLeftFontUnderline,
      repeatLeftAlign,

      repeatRightFontFace,
      repeatRightFontSize,
      repeatRightFontBold,
      repeatRightFontItalic,
      repeatRightFontUnderline,
      repeatRightAlign,

      voltaFontFace,
      voltaFontSize,
      voltaFontBold,
      voltaFontItalic,
      voltaFontUnderline,
      voltaAlign,
      voltaOffset,

      frameFontFace,
      frameFontSize,
      frameFontBold,
      frameFontItalic,
      frameFontUnderline,
      frameAlign,

      textLineFontFace,
      textLineFontSize,
      textLineFontBold,
      textLineFontItalic,
      textLineFontUnderline,

      glissandoFontFace,
      glissandoFontSize,
      glissandoFontBold,
      glissandoFontItalic,
      glissandoFontUnderline,
      glissandoLineWidth,

      ottavaFontFace,
      ottavaFontSize,
      ottavaFontBold,
      ottavaFontItalic,
      ottavaFontUnderline,
      ottavaTextAlign,

      pedalFontFace,
      pedalFontSize,
      pedalFontBold,
      pedalFontItalic,
      pedalFontUnderline,
      pedalTextAlign,

      hairpinFontFace,
      hairpinFontSize,
      hairpinFontBold,
      hairpinFontItalic,
      hairpinFontUnderline,
      hairpinTextAlign,

      bendFontFace,
      bendFontSize,
      bendFontBold,
      bendFontItalic,
      bendFontUnderline,

      headerFontFace,
      headerFontSize,
      headerFontBold,
      headerFontItalic,
      headerFontUnderline,

      footerFontFace,
      footerFontSize,
      footerFontBold,
      footerFontItalic,
      footerFontUnderline,

      instrumentChangeFontFace,
      instrumentChangeFontSize,
      instrumentChangeFontBold,
      instrumentChangeFontItalic,
      instrumentChangeFontUnderline,
      instrumentChangeOffset,

      figuredBassFontFace,
      figuredBassFontSize,
      figuredBassFontBold,
      figuredBassFontItalic,
      figuredBassFontUnderline,

      user1FontFace,
      user1FontSize,
      user1FontBold,
      user1FontItalic,
      user1FontUnderline,

      user2FontFace,
      user2FontSize,
      user2FontBold,
      user2FontItalic,
      user2FontUnderline,

      letRingFontFace,
      letRingFontSize,
      letRingFontBold,
      letRingFontItalic,
      letRingFontUnderline,
      letRingTextAlign,
      letRingHookHeight,
      letRingPlacement,
      letRingPosAbove,
      letRingPosBelow,
      letRingLineWidth,
      letRingLineStyle,
      letRingBeginTextOffset,
      letRingText,

      palmMuteFontFace,
      palmMuteFontSize,
      palmMuteFontBold,
      palmMuteFontItalic,
      palmMuteFontUnderline,
      palmMuteTextAlign,
      palmMuteHookHeight,
      palmMutePlacement,
      palmMutePosAbove,
      palmMutePosBelow,
      palmMuteLineWidth,
      palmMuteLineStyle,
      palmMuteBeginTextOffset,
      palmMuteText,

      fermataPosAbove,
      fermataPosBelow,
      fermataMinDistance,

      STYLES
      };

//---------------------------------------------------------
//   StyledProperty
//---------------------------------------------------------

struct StyledProperty {
      StyleIdx styleIdx;
      P_ID propertyIdx;
      };

extern const std::vector<StyledProperty> fingeringStyle;
extern const std::vector<StyledProperty> titleStyle;

//-------------------------------------------------------------------
//   SubStyle
//    Enumerate the list of built-in substyles
//    must be in sync with namedStyles array
//-------------------------------------------------------------------

enum class SubStyle {
      DEFAULT,
      TITLE,
      SUBTITLE,
      COMPOSER,
      POET,
      LYRIC1,
      LYRIC2,
      FINGERING,
      LH_GUITAR_FINGERING,
      RH_GUITAR_FINGERING,
      STRING_NUMBER,
      INSTRUMENT_LONG,
      INSTRUMENT_SHORT,
      INSTRUMENT_EXCERPT,
      DYNAMICS,
      EXPRESSION,
      TEMPO,
      METRONOME,
      MEASURE_NUMBER,
      TRANSLATOR,
      TUPLET,
      SYSTEM,
      STAFF,
      HARMONY,
      REHEARSAL_MARK,
      REPEAT_LEFT,       // align to start of measure
      REPEAT_RIGHT,      // align to end of measure
      FRAME,
      TEXTLINE,
      GLISSANDO,
      OTTAVA,
      PEDAL,
      LET_RING,
      PALM_MUTE,
      HAIRPIN,
      BEND,
      HEADER,
      FOOTER,
      INSTRUMENT_CHANGE,
      FIGURED_BASS,
      USER1,
      USER2,
      SUBSTYLES
      };

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

class MStyle {
      std::array<QVariant, int(StyleIdx::STYLES)> _values;
      std::array<qreal, int(StyleIdx::STYLES)> _precomputedValues;

      ChordList _chordList;
      bool _customChordList;        // if true, chordlist will be saved as part of score

   public:
      MStyle();

      void precomputeValues();
      QVariant value(StyleIdx idx) const;
      qreal pvalue(StyleIdx idx) const    { return _precomputedValues[int(idx)]; }
      void set(StyleIdx idx, const QVariant& v);

      bool isDefault(StyleIdx idx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList()  { return &_chordList; }
      void setChordList(ChordList*, bool custom = true);    // Style gets ownership of ChordList
      void setCustomChordList(bool t) { _customChordList = t; }

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(XmlWriter& xml, bool optimize);
      bool readProperties(XmlReader&);

      static const char* valueType(const StyleIdx);
      static const char* valueName(const StyleIdx);
      static StyleIdx styleIdx(const QString& name);
      };

const std::vector<StyledProperty>& subStyle(const char*);
const std::vector<StyledProperty>& subStyle(SubStyle);
const char* subStyleName(SubStyle);
QString subStyleUserName(SubStyle);
SubStyle subStyleFromName(const QString&);

#ifndef NDEBUG
extern void checkStyles();
#endif

}     // namespace Ms


#endif
