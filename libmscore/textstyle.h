//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTSTYLE_H__
#define __TEXTSTYLE_H__

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
         const QPointF& _off = QPointF(),
         OffsetType _ot = OffsetType::SPATIUM,
         bool sd = false,
         bool hasFrame = false,
         bool square = false,
         Spatium fw = Spatium(0.2),
         Spatium pw = Spatium(0.5),
         int fr = 25,
         QColor co = QColor(Qt::black),
         bool circle = false,
         bool systemFlag = false,
         QColor fg = QColor(Qt::black),
         QColor bg = QColor(255, 255, 255, 0),
         TextStyleHidden hidden = TextStyleHidden::NEVER);

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
      bool square() const;
      void setSquare(bool val);
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
      QRectF bbox(qreal spatium, const QString& s) const;
      QFontMetricsF fontMetrics(qreal spatium) const;
      bool operator!=(const TextStyle& s) const;
      void layout(Element*) const;
      void restyle(const TextStyle& os, const TextStyle& ns);
      };

extern QVector<TextStyle> defaultTextStyles;
extern const TextStyle defaultTextStyleArray[];


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::TextStyle);
#endif
