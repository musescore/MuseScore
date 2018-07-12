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

enum class Pid : int;
class XmlWriter;
struct ChordDescription;
class Element;

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
      lyricsMelismaAlign,

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
      hairpinFontFace,
      hairpinFontSize,
      hairpinFontBold,
      hairpinFontItalic,
      hairpinFontUnderline,
      hairpinTextAlign,

      pedalPlacement,
      pedalPosAbove,
      pedalPosBelow,
      pedalLineWidth,
      pedalLineStyle,
      pedalBeginTextOffset,
      pedalHookHeight,
      pedalFontFace,
      pedalFontSize,
      pedalFontBold,
      pedalFontItalic,
      pedalFontUnderline,
      pedalTextAlign,

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
      harmonyPlacement,
      chordSymbolFontFace,
      chordSymbolFontSize,
      chordSymbolFontBold,
      chordSymbolFontItalic,
      chordSymbolFontUnderline,
      chordSymbolAlign,

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

      voltaY,
      voltaHook,
      voltaLineWidth,
      voltaLineStyle,
      voltaFontFace,
      voltaFontSize,
      voltaFontBold,
      voltaFontItalic,
      voltaFontUnderline,
      voltaAlign,
      voltaOffset,

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
      ottavaFontBold,
      ottavaFontItalic,
      ottavaFontUnderline,
      ottavaTextAlign,

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
      tupletFontBold,
      tupletFontItalic,
      tupletFontUnderline,
      tupletAlign,

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
      defaultText,

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

      systemFontFace,
      systemFontSize,
      systemFontBold,
      systemFontItalic,
      systemFontUnderline,
      systemOffset,
      systemOffsetType,
      systemAlign,

      staffTextFontFace,
      staffTextFontSize,
      staffTextFontBold,
      staffTextFontItalic,
      staffTextFontUnderline,
      staffTextAlign,
      staffTextOffset,
      staffTextOffsetType,
      staffTextPlacement,
      staffTextPosAbove,
      staffTextPosBelow,
      staffTextMinDistance,

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
      repeatLeftPlacement,

      repeatRightFontFace,
      repeatRightFontSize,
      repeatRightFontBold,
      repeatRightFontItalic,
      repeatRightFontUnderline,
      repeatRightAlign,
      repeatRightPlacement,

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
      glissandoText,

      bendFontFace,
      bendFontSize,
      bendFontBold,
      bendFontItalic,
      bendFontUnderline,
      bendLineWidth,
      bendArrowWidth,

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
      instrumentChangeAlign,
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
      user1Align,
      user1Offset,
      user1OffsetType,

      user2FontFace,
      user2FontSize,
      user2FontBold,
      user2FontItalic,
      user2FontUnderline,
      user2Align,
      user2Offset,
      user2OffsetType,

      user3FontFace,
      user3FontSize,
      user3FontBold,
      user3FontItalic,
      user3FontUnderline,
      user3Align,
      user3Offset,
      user3OffsetType,

      user4FontFace,
      user4FontSize,
      user4FontBold,
      user4FontItalic,
      user4FontUnderline,
      user4Align,
      user4Offset,
      user4OffsetType,

      user5FontFace,
      user5FontSize,
      user5FontBold,
      user5FontItalic,
      user5FontUnderline,
      user5Align,
      user5Offset,
      user5OffsetType,

      user6FontFace,
      user6FontSize,
      user6FontBold,
      user6FontItalic,
      user6FontUnderline,
      user6Align,
      user6Offset,
      user6OffsetType,

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
      Sid sid;
      Pid pid;
      };

//-------------------------------------------------------------------
//   SubStyleId
//    Enumerate the list of built-in substyles
//    must be in sync with namedStyles array
//-------------------------------------------------------------------

enum class SubStyleId {
      EMPTY,
      DEFAULT,
      TITLE,
      SUBTITLE,
      COMPOSER,
      POET,
      LYRIC,
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
      VOLTA,
      PEDAL,
      LET_RING,
      PALM_MUTE,
      HAIRPIN,
      BEND,
      HEADER,
      FOOTER,
      INSTRUMENT_CHANGE,
      FIGURED_BASS,
      BEAM,
      BOX,
      FRET,
      TREMOLO_BAR,
      TIMESIG,
      STEM,
      USER1,
      USER2,
      USER3,
      USER4,
      USER5,
      USER6,
      SUBSTYLES
      };

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

class MStyle {
      std::array<QVariant, int(Sid::STYLES)> _values;
      std::array<qreal, int(Sid::STYLES)> _precomputedValues;

      ChordList _chordList;
      bool _customChordList;        // if true, chordlist will be saved as part of score

   public:
      MStyle();

      void precomputeValues();
      QVariant value(Sid idx) const;
      qreal pvalue(Sid idx) const    { return _precomputedValues[int(idx)]; }
      void set(Sid idx, const QVariant& v);

      bool isDefault(Sid idx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList()  { return &_chordList; }
      void setChordList(ChordList*, bool custom = true);    // Style gets ownership of ChordList
      void setCustomChordList(bool t) { _customChordList = t; }

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(XmlWriter& xml, bool optimize);
      bool readProperties(XmlReader&);

      static const char* valueType(const Sid);
      static const char* valueName(const Sid);
      static Sid styleIdx(const QString& name);
      };

typedef std::vector<StyledProperty> SubStyle;

extern const SubStyle emptyStyle;
extern const SubStyle defaultStyle;
extern const SubStyle fingeringStyle;
extern const SubStyle lyricsStyle;
extern const SubStyle ottavaStyle;

const SubStyle& subStyle(SubStyleId);
const SubStyle& subStyle(const char*);

const char* subStyleName(SubStyleId);
QString subStyleUserName(SubStyleId);
SubStyleId subStyleFromName(const QString&);

#ifndef NDEBUG
extern void checkStyles();
#endif

}     // namespace Ms


#endif
