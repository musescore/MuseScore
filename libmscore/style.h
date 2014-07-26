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

namespace Ms {

class Xml;
struct ChordDescription;
class PageFormat;
class ChordList;
class Element;

class TextStyleData;

enum class TextStyleHidden : unsigned char {
      NEVER     = 0,
      IN_EDITOR = 1,
      IN_LISTS  = 2,
      ALWAYS    = 0xFF
      };

constexpr bool operator& (TextStyleHidden h1, TextStyleHidden h2) {
      return static_cast<unsigned char>(h1) & static_cast<unsigned char>(h2);
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

class TextStyle {
   private:
      QSharedDataPointer<TextStyleData> d;
      TextStyleHidden _hidden;               // read-only parameter for text style visibility in various program places

   public:
      TextStyle();
      TextStyle(QString _name, QString _family,
         qreal _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         const QPointF& _off = QPointF(), OffsetType _ot = OffsetType::SPATIUM,
         bool sd = false,
         bool hasFrame = false, Spatium fw = Spatium(0.2), Spatium pw = Spatium(0.5), int fr = 25,
         QColor co = QColor(Qt::black), bool circle = false, bool systemFlag = false,
         QColor fg = QColor(Qt::black), QColor bg = QColor(255, 255, 255, 0), TextStyleHidden hidden = TextStyleHidden::NEVER);

      TextStyle(const TextStyle&);
      ~TextStyle();
      TextStyle& operator=(const TextStyle&);

      friend class TextStyleDialog;             // allow TextStyleDialog to access _hidden without making it globally writeable

      QString name() const;
      QString family() const;
      qreal size() const;
      bool bold() const;
      bool italic() const;
      bool underline() const;
      bool hasFrame() const;
      Align align() const;
      OffsetType offsetType() const;
      const QPointF& offset() const;
      QPointF offset(qreal spatium) const;
      bool sizeIsSpatiumDependent() const;

      Spatium frameWidth()  const;
      Spatium paddingWidth() const;
      qreal frameWidthMM()  const;
      qreal paddingWidthMM() const;
      void setFrameWidth(Spatium v);
      void setPaddingWidth(Spatium v);

      int frameRound() const;
      QColor frameColor() const;
      bool circle() const;
      bool systemFlag() const;
      QColor foregroundColor() const;
      QColor backgroundColor() const;
      void setName(const QString& s);
      void setFamily(const QString& s);
      void setSize(qreal v);
      void setBold(bool v);
      void setItalic(bool v);
      void setUnderline(bool v);
      void setHasFrame(bool v);
      void setAlign(Align v);
      void setXoff(qreal v);
      void setYoff(qreal v);
      void setOffsetType(OffsetType v);
      void setSizeIsSpatiumDependent(bool v);
      void setFrameRound(int v);
      void setFrameColor(const QColor& v);
      void setCircle(bool v);
      void setSystemFlag(bool v);
      void setForegroundColor(const QColor& v);
      void setBackgroundColor(const QColor& v);
      TextStyleHidden hidden() const   { return _hidden; }
      void write(Xml& xml) const;
      void writeProperties(Xml& xml) const;
      void writeProperties(Xml& xml, const TextStyle&) const;
      void read(XmlReader& v);
      bool readProperties(XmlReader& v);
      QFont font(qreal spatium) const;
      QFont fontPx(qreal spatium) const;
      QRectF bbox(qreal spatium, const QString& s) const;
      QFontMetricsF fontMetrics(qreal spatium) const;
      bool operator!=(const TextStyle& s) const;
      void layout(Element*) const;
      void restyle(const TextStyle& os, const TextStyle& ns);
      };

//---------------------------------------------------------
//   StyleValueType
//---------------------------------------------------------

enum class StyleValueType : char {
      SPATIUM, DOUBLE, BOOL, INT, DIRECTION, STRING
      };

//---------------------------------------------------------
//   StyleType
//---------------------------------------------------------

class StyleType {
      const char* _name;       // xml name for read()/write()
      StyleValueType _valueType;

   public:
      StyleType() { _name = 0; }
      StyleType(const char* n, StyleValueType v) : _name(n), _valueType(v) {}
      StyleValueType valueType() const { return _valueType; }
      const char* name() const         { return _name; }
      };

//---------------------------------------------------------
//   StyleIdx
//---------------------------------------------------------

enum class StyleIdx : unsigned char {
      staffUpperBorder,
      staffLowerBorder,
      staffDistance,
      akkoladeDistance,

      minSystemDistance,
      maxSystemDistance,

      lyricsDistance,
      lyricsMinBottomDistance,
      lyricsLineHeight,

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

      clefLeftMargin,
      keysigLeftMargin,
      timesigLeftMargin,

      clefKeyRightMargin,
      clefBarlineDistance,
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
      genTimesig,
      genCourtesyTimesig,
      genCourtesyKeysig,
      genCourtesyClef,

      swingRatio,
      swingUnit,

      useStandardNoteNames,
      useGermanNoteNames,
      useSolfeggioNoteNames,
      lowerCaseMinorChords,
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
      FixMeasureNumbers,
      FixMeasureWidth,

      SlurEndWidth,
      SlurMidWidth,
      SlurDottedWidth,
      MinTieLength,

      SectionPause,
      MusicalSymbolFont,
      MusicalTextFont,

      showHeader,
//      headerStyled,
      headerFirstPage,
      headerOddEven,
      evenHeaderL,
      evenHeaderC,
      evenHeaderR,
      oddHeaderL,
      oddHeaderC,
      oddHeaderR,

      showFooter,
//      footerStyled,
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

      STYLES
      };

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

class StyleData;

class MStyle {
      QSharedDataPointer<StyleData> d;

   public:
      MStyle();
      MStyle(const MStyle&);
      MStyle& operator=(const MStyle&);
      ~MStyle();

      bool isDefault(StyleIdx idx) const;
      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList();

      void setChordList(ChordList*, bool custom = true);    // Style gets ownership of ChordList

      const TextStyle& textStyle(TextStyleType) const;
      const TextStyle& textStyle(const QString& name) const;
      TextStyleType textStyleType(const QString& name) const;
      void setTextStyle(const TextStyle& ts);
      void addTextStyle(const TextStyle& ts);
      void removeTextStyle(const TextStyle& ts);
      const QList<TextStyle>& textStyles() const;
      void set(StyleIdx t, Spatium val);
      void set(StyleIdx t, const QString& val);
      void set(StyleIdx t, bool val);
      void set(StyleIdx t, qreal val);
      void set(StyleIdx t, int val);
      void set(StyleIdx t, MScore::Direction val);
      void set(StyleIdx t, const QVariant& v);

      QVariant value(StyleIdx idx) const;

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(Xml& xml, bool optimize);
      const PageFormat* pageFormat() const;
      void setPageFormat(const PageFormat& pf);
      qreal spatium() const;
      void setSpatium(qreal v);
      ArticulationAnchor articulationAnchor(int id) const;
      void setArticulationAnchor(int id, ArticulationAnchor val);
      };

extern QVector<TextStyle> defaultTextStyles;
extern const TextStyle defaultTextStyleArray[];

extern void initStyle(MStyle*);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::TextStyle);

#endif
