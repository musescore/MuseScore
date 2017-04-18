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

#ifndef __STYLE_P_H__
#define __STYLE_P_H__

//
// private header for MStyle
//

#include "elementlayout.h"
#include "articulation.h"
#include "page.h"
#include "chordlist.h"

namespace Ms {

class Xml;
struct ChordDescription;

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

class TextStyleData : public QSharedData, public ElementLayout {

   protected:
      qreal frameWidthMM;           // for compatibility with old scores
      qreal paddingWidthMM;

      QString name;                 // style name
      QString family;               // font face
      qreal size;
      bool bold;
      bool italic;
      bool underline;

      bool sizeIsSpatiumDependent;        // text point size depends on _spatium unit

      bool hasFrame;
      Spatium frameWidth;
      Spatium paddingWidth;
      int frameRound;
      QColor frameColor;
      bool circle;
      bool systemFlag;
      QColor foregroundColor;
      QColor backgroundColor;             // only for frame

   public:
      TextStyleData(QString _name, QString _family, qreal _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         const QPointF& _off, OffsetType _ot,
         bool sizeSpatiumDependent,
         bool hasFrame, Spatium fw, Spatium pw, int fr,
         QColor co, bool circle, bool systemFlag,
         QColor fg, QColor bg);
      TextStyleData();

      void write(Xml&) const;
      void writeProperties(Xml& xml) const;
      void writeProperties(Xml& xml, const TextStyleData&) const;
      void restyle(const TextStyleData& os, const TextStyleData& ns);
      void read(XmlReader&);
      bool readProperties(XmlReader& v);

      QFont font(qreal space) const;
      QFont fontPx(qreal spatium) const;
      QRectF bbox(qreal space, const QString& s) const { return fontMetrics(space).boundingRect(s); }
      QFontMetricsF fontMetrics(qreal space) const     { return QFontMetricsF(font(space)); }
      bool operator!=(const TextStyleData& s) const;
      friend class TextStyle;
      };

//---------------------------------------------------------
//   StyleData
//    this structure contains all style elements
//---------------------------------------------------------

class StyleData : public QSharedData {
   protected:
      QVector<QVariant> _values;
      ChordList _chordList;
      QList<TextStyle> _textStyles;
      PageFormat _pageFormat;
      qreal _spatium;
      ArticulationAnchor _articulationAnchor[int(ArticulationType::ARTICULATIONS)];
      MScore::OrnamentStyle _ornamentStyle = MScore::OrnamentStyle::DEFAULT;
    
      bool _customChordList;        // if true, chordlist will be saved as part of score

      void set(StyleIdx id, const QVariant& v)            { _values[int(id)] = v; }
      QVariant value(StyleIdx idx) const                  { return _values[int(idx)];     }
      const TextStyle& textStyle(TextStyleType idx) const;
      const TextStyle& textStyle(const QString&) const;
      TextStyleType textStyleType(const QString&) const;
      void setTextStyle(const TextStyle& ts);

   public:
      StyleData();
      StyleData(const StyleData&);
      ~StyleData();

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(Xml& xml, bool optimize) const;
      bool isDefault(StyleIdx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList();
      void setChordList(ChordList*, bool custom);     // Style gets ownership of ChordList
      PageFormat* pageFormat()                       { return &_pageFormat; }
      const PageFormat* pageFormat() const           { return &_pageFormat; }
      void setPageFormat(const PageFormat& pf);
      friend class MStyle;
      qreal spatium() const                                      { return _spatium; }
      void setSpatium(qreal v)                                   { _spatium = v;    }
      ArticulationAnchor articulationAnchor(int id) const        { return _articulationAnchor[id]; }
      void setArticulationAnchor(int id, ArticulationAnchor val) { _articulationAnchor[id] = val;  }
      MScore::OrnamentStyle ornamentStyle()                      { return _ornamentStyle ; }
      void setOrnamentStyle(MScore::OrnamentStyle val)           { _ornamentStyle = val; }
    
      friend class TextStyle;
      };

}     // namespace Ms
#endif

