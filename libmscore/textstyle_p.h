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

#ifndef __TEXTSTYLE_P_H__
#define __TEXTSTYLE_P_H__

#include "elementlayout.h"
#include "articulation.h"
#include "page.h"

namespace Ms {

class Xml;

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
      bool _square;
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
         bool hasFrame, bool square, Spatium fw, Spatium pw, int fr,
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
      QRectF bbox(qreal space, const QString& s) const { return fontMetrics(space).boundingRect(s); }
      QFontMetricsF fontMetrics(qreal space) const     { return QFontMetricsF(font(space), MScore::paintDevice()); }
      bool operator!=(const TextStyleData& s) const;
      friend class TextStyle;
      };

}     // namespace Ms
#endif

