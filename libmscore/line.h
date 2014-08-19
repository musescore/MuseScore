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

#ifndef __LINE_H__
#define __LINE_H__

#include "spanner.h"
#include "mscore.h"

class QPainter;

namespace Ms {

class SLine;
class System;
class MuseScoreView;

enum class GripLine : char { START, MIDDLE, END, APERTURE };

//---------------------------------------------------------
//   LineStyle
//---------------------------------------------------------

enum class LineStyle : char {
      Solid      = Qt::SolidLine,
      Dash       = Qt::DashLine,
      Dot        = Qt::DotLine,
      DashDot    = Qt::DashDotLine,
      DashDotDot = Qt::DashDotDotLine
      };

//---------------------------------------------------------
//   @@ LineSegment
///    Virtual base class for segmented lines segments
///    (OttavaSegment, HairpinSegment, TrillSegment...)
///
///    This class describes one segment of an segmented
///    line object. Line objects can span multiple staves.
///    For every staff a segment is created.
//---------------------------------------------------------

class LineSegment : public SpannerSegment {
      Q_OBJECT

   protected:
      virtual bool isEditable() const override { return true; }
      virtual void editDrag(const EditData&) override;
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s) override;
      virtual void updateGrips(int*, int*, QRectF*) const override;
      virtual void setGrip(int grip, const QPointF& p) override;
      virtual QPointF getGrip(int) const override;
      virtual QPointF gripAnchor(int) const override;

   public:
      LineSegment(Score* s) : SpannerSegment(s) {}
      LineSegment(const LineSegment&);
      virtual LineSegment* clone() const = 0;
      virtual void draw(QPainter*) const = 0;
      SLine* line() const                         { return (SLine*)spanner(); }
      virtual void spatiumChanged(qreal, qreal) override;

      friend class SLine;
      virtual void read(XmlReader&) override;
      bool readProperties(XmlReader&);

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual QLineF dragAnchor() const override;
      };

//---------------------------------------------------------
//   @@ SLine
///    virtual base class for Hairpin, Trill and TextLine
//---------------------------------------------------------

class SLine : public Spanner {
      Q_OBJECT

      Spatium _lineWidth;
      QColor _lineColor;
      Qt::PenStyle _lineStyle;
      bool _diagonal;

   protected:
      virtual QPointF linePos(GripLine grip, System** system) const;

   public:
      SLine(Score* s);
      SLine(const SLine&);

      virtual void layout() override;
      bool readProperties(XmlReader& node);
      void writeProperties(Xml& xml) const;
      virtual LineSegment* createLineSegment() = 0;
      void setLen(qreal l);
      virtual const QRectF& bbox() const override;

      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      bool diagonal() const               { return _diagonal; }
      void setDiagonal(bool v)            { _diagonal = v;    }

      Spatium lineWidth() const           { return _lineWidth;            }
      QColor lineColor() const            { return _lineColor;            }
      Qt::PenStyle lineStyle() const      { return _lineStyle;            }
      void setLineWidth(const Spatium& v) { _lineWidth = v;               }
      void setLineColor(const QColor& v)  { _lineColor = v;               }
      void setLineStyle(Qt::PenStyle v)   { _lineStyle = v;               }

      LineSegment* frontSegment() const   { return (LineSegment*)spannerSegments().front(); }
      LineSegment* backSegment() const    { return (LineSegment*)spannerSegments().back();  }
      LineSegment* takeFirstSegment()     { return (LineSegment*)spannerSegments().takeFirst(); }
      LineSegment* takeLastSegment()      { return (LineSegment*)spannerSegments().takeLast(); }
      LineSegment* segmentAt(int n) const { return (LineSegment*)spannerSegments().at(n); }

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;

      friend class LineSegment;
      };


}     // namespace Ms
#endif

