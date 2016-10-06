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

#include "mscore.h"
#include "spatium.h"
#include "articulation.h"
#include "page.h"
#include "chordlist.h"

namespace Ms {

class Xml;
struct ChordDescription;
class PageFormat;
class ChordList;
class Element;

//---------------------------------------------------------
//   StyleIdx
//
//    Keep in sync with styleTypes[] in style.cpp
//---------------------------------------------------------

enum class StyleIdx : int {
      NOSTYLE = -1,
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

      figuredBassFontFamily,
      figuredBassFontSize,
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

      hairpinY,
      hairpinHeight,
      hairpinContHeight,
      hairpinLineWidth,

      pedalY,
      pedalLineWidth,
      pedalLineStyle,

      trillY,
      harmonyY,
      harmonyFretDist,
      minHarmonyDistance,
      maxHarmonyBarDistance,
      capoPosition,
      fretNumMag,
      fretNumPos,
      fretY,

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

      SlurEndWidth,
      SlurMidWidth,
      SlurDottedWidth,
      MinTieLength,

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

      ottavaY,
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
      dynamicsMinDistance,
      autoplaceVerticalAlignRange,

      textLinePlacement,
      textLinePosAbove,
      textLinePosBelow,

      tremoloBarLineWidth,

      STYLES
      };

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


//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

class MStyle {
      QVector<QVariant> _values;
      QVector<qreal> _precomputedValues;

      ChordList _chordList;
      QList<TextStyle> _textStyles;
      PageFormat _pageFormat;

      bool _customChordList;        // if true, chordlist will be saved as part of score

      void precomputeValues();

   public:
      MStyle();
      MStyle(const MStyle&);
      MStyle& operator=(const MStyle&);
      // ~MStyle() {}

      bool isDefault(StyleIdx idx) const;
      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList()  { return &_chordList; }

      void setChordList(ChordList*, bool custom = true);    // Style gets ownership of ChordList
      void setCustomChordList(bool t) { _customChordList = t; }

      const TextStyle& textStyle(TextStyleType) const;
      TextStyle& textStyle(TextStyleType);

      const TextStyle& textStyle(const QString& name) const;
      TextStyleType textStyleType(const QString& name) const;
      void setTextStyle(const TextStyle& ts);
      void addTextStyle(const TextStyle& ts);
      void removeTextStyle(const TextStyle& ts);
      const QList<TextStyle>& textStyles() const;

      void set(StyleIdx idx, const QVariant& v);

      QVariant value(StyleIdx idx) const  { return _values[int(idx)]; }
      qreal pvalue(StyleIdx idx) const    { return _precomputedValues[int(idx)]; }

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(Xml& xml, bool optimize);

      void convertToUnit(const QString& tag, const QString& val);

      PageFormat* pageFormat()             { return &_pageFormat; }
      const PageFormat* pageFormat() const { return &_pageFormat; }

      void setPageFormat(const PageFormat& pf);

      static const char* valueType(const StyleIdx);
      static const char* valueName(const StyleIdx);
      static StyleIdx styleIdx(const QString& name);
      };

extern void initStyle(MStyle*);

}     // namespace Ms


#endif
