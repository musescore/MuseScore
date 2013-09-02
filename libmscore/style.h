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

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

class TextStyle {
   public:
      enum Hidden {
            HIDE_NEVER     = 0,
            HIDE_IN_EDITOR = 1,
            HIDE_IN_LISTS  = 2,
            HIDE_ALWAYS    = 0xFFFF
            };

   private:
      QSharedDataPointer<TextStyleData> d;
      Hidden _hidden;               // read-only parameter for text style visibility in various program places

   public:
      TextStyle();
      TextStyle(QString _name, QString _family,
         qreal _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         const QPointF& _off = QPointF(), OffsetType _ot = OFFSET_SPATIUM,
         const QPointF& _roff = QPointF(),
         bool sd = false,
         Spatium fw = Spatium(0.0), Spatium pw = Spatium(0.0), int fr = 25,
         QColor co = QColor(Qt::black), bool circle = false, bool systemFlag = false,
         QColor fg = QColor(Qt::black), QColor bg = QColor(255, 255, 255, 0), Hidden hidden = HIDE_NEVER);

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
      const QPointF& reloff() const;
      void setReloff(const QPointF& p);
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
      void setRxoff(qreal v);
      void setRyoff(qreal v);
      void setSizeIsSpatiumDependent(bool v);
      void setFrameRound(int v);
      void setFrameColor(const QColor& v);
      void setCircle(bool v);
      void setSystemFlag(bool v);
      void setForegroundColor(const QColor& v);
      void setBackgroundColor(const QColor& v);
      Hidden hidden() const   { return _hidden; }
      void write(Xml& xml) const;
      void writeProperties(Xml& xml) const;
      void read(XmlReader& v);
      bool readProperties(XmlReader& v);
      QFont font(qreal space) const;
      QFont fontPx(qreal spatium) const;
      QRectF bbox(qreal space, const QString& s) const;
      QFontMetricsF fontMetrics(qreal space) const;
      bool operator!=(const TextStyle& s) const;
      void layout(Element*) const;
      void setFont(const QFont& f);
      };

//---------------------------------------------------------
//   StyleValueType
//---------------------------------------------------------

enum StyleValueType {
      ST_SPATIUM, ST_DOUBLE, ST_BOOL, ST_INT, ST_DIRECTION, ST_STRING
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

enum StyleIdx {
      ST_staffUpperBorder,
      ST_staffLowerBorder,
      ST_staffDistance,
      ST_akkoladeDistance,

      ST_minSystemDistance,
      ST_maxSystemDistance,

      ST_lyricsDistance,
      ST_lyricsMinBottomDistance,
      ST_lyricsLineHeight,

      ST_figuredBassFontFamily,
      ST_figuredBassFontSize,
      ST_figuredBassYOffset,
      ST_figuredBassLineHeight,
      ST_figuredBassAlignment,
      ST_figuredBassStyle,
      ST_systemFrameDistance,
      ST_frameSystemDistance,
      ST_minMeasureWidth,

      ST_barWidth,
      ST_doubleBarWidth,
      ST_endBarWidth,
      ST_doubleBarDistance,
      ST_endBarDistance,
      ST_repeatBarTips,
      ST_startBarlineSingle,
      ST_startBarlineMultiple,

      ST_bracketWidth,
      ST_bracketDistance,
      ST_akkoladeWidth,
      ST_akkoladeBarDistance,

      ST_clefLeftMargin,
      ST_keysigLeftMargin,
      ST_timesigLeftMargin,

      ST_clefKeyRightMargin,
      ST_clefBarlineDistance,
      ST_stemWidth,
      ST_shortenStem,
      ST_shortStemProgression,
      ST_shortestStem,
      ST_beginRepeatLeftMargin,
      ST_minNoteDistance,
      ST_barNoteDistance,
      ST_barAccidentalDistance,
      ST_multiMeasureRestMargin,
      ST_noteBarDistance,

      ST_measureSpacing,
      ST_staffLineWidth,
      ST_ledgerLineWidth,
      ST_ledgerLineLength,
      ST_accidentalDistance,
      ST_accidentalNoteDistance,
      ST_beamWidth,
      ST_beamDistance,
      ST_beamMinLen,
      ST_beamNoSlope,

      ST_dotMag,
      ST_dotNoteDistance,
      ST_dotRestDistance,
      ST_dotDotDistance,
      ST_propertyDistanceHead,
      ST_propertyDistanceStem,
      ST_propertyDistance,
      ST_articulationMag,
      ST_lastSystemFillLimit,

      ST_hairpinY,
      ST_hairpinHeight,
      ST_hairpinContHeight,
      ST_hairpinLineWidth,

      ST_pedalY,
      ST_pedalLineWidth,
      ST_pedalLineStyle,

      ST_trillY,
      ST_harmonyY,
      ST_harmonyFretDist,
      ST_minHarmonyDistance,

      ST_showPageNumber,
      ST_showPageNumberOne,
      ST_pageNumberOddEven,
      ST_showMeasureNumber,
      ST_showMeasureNumberOne,
      ST_measureNumberInterval,
      ST_measureNumberSystem,

      ST_measureNumberAllStaffs,
      ST_smallNoteMag,
      ST_graceNoteMag,
      ST_smallStaffMag,
      ST_smallClefMag,
      ST_genClef,
      ST_genKeysig,
      ST_genTimesig,
      ST_genCourtesyTimesig,
      ST_genCourtesyKeysig,
      ST_genCourtesyClef,

      ST_useStandardNoteNames,
      ST_useGermanNoteNames,
      ST_useSolfeggioNoteNames,
      ST_lowerCaseMinorChords,
      ST_chordStyle,
      ST_chordsXmlFile,
      ST_chordDescriptionFile,
      ST_concertPitch,
      ST_createMultiMeasureRests,
      ST_minEmptyMeasures,
      ST_minMMRestWidth,
      ST_hideEmptyStaves,
      ST_dontHideStavesInFirstSystem,
      ST_gateTime,
      ST_tenutoGateTime,
      ST_staccatoGateTime,
      ST_slurGateTime,

      ST_ArpeggioNoteDistance,
      ST_ArpeggioLineWidth,
      ST_ArpeggioHookLen,
      ST_FixMeasureNumbers,
      ST_FixMeasureWidth,

      ST_SlurEndWidth,
      ST_SlurMidWidth,
      ST_SlurDottedWidth,

      ST_SectionPause,
      ST_MusicalSymbolFont,

      ST_showHeader,
      ST_headerStyled,
      ST_headerFirstPage,
      ST_headerOddEven,
      ST_evenHeaderL,
      ST_evenHeaderC,
      ST_evenHeaderR,
      ST_oddHeaderL,
      ST_oddHeaderC,
      ST_oddHeaderR,

      ST_showFooter,
      ST_footerStyled,
      ST_footerFirstPage,
      ST_footerOddEven,
      ST_evenFooterL,
      ST_evenFooterC,
      ST_evenFooterR,
      ST_oddFooterL,
      ST_oddFooterC,
      ST_oddFooterR,

      ST_voltaY,
      ST_voltaHook,
      ST_voltaLineWidth,
      ST_voltaLineStyle,

      ST_ottavaY,
      ST_ottavaHook,
      ST_ottavaLineWidth,
      ST_ottavaLineStyle,
      ST_ottavaNumbersOnly,

      ST_tabClef,

      ST_tremoloWidth,
      ST_tremoloBoxHeight,
      ST_tremoloStrokeWidth,
      ST_tremoloDistance,

      ST_linearStretch,
      ST_crossMeasureValues,
      ST_keySigNaturals,

      ST_tupletMaxSlope,
      ST_tupletOufOfStaff,
      ST_tupletVHeadDistance,
      ST_tupletVStemDistance,
      ST_tupletStemLeftDistance,
      ST_tupletStemRightDistance,
      ST_tupletNoteLeftDistance,
      ST_tupletNoteRightDistance,

      ST_STYLES
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

      const TextStyle& textStyle(int) const;
      const TextStyle& textStyle(const QString& name) const;
      int textStyleType(const QString& name) const;
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
#endif
