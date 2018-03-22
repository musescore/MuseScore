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

namespace Ms {

class SLine;
class System;

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
   protected:
      virtual void startEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual bool edit(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual QPointF gripAnchor(Grip) const override;
      virtual void startEditDrag(EditData&) override;

   public:
      LineSegment(Score* s) : SpannerSegment(s) {}
      LineSegment(const LineSegment&);
      virtual void draw(QPainter*) const = 0;
      SLine* line() const                         { return (SLine*)spanner(); }
      virtual void spatiumChanged(qreal, qreal) override;
      virtual void localSpatiumChanged(qreal, qreal) override;

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
      Spatium _lineWidth      { 0.15 };
      QColor _lineColor       { MScore::defaultColor };
      Qt::PenStyle _lineStyle { Qt::SolidLine };
      qreal _dashLineLen      { 5.0   };
      qreal _dashGapLen       { 5.0   };
      bool _diagonal          { false };

      PropertyFlags lineWidthStyle;
      PropertyFlags lineStyleStyle;
      PropertyFlags lineColorStyle;

   protected:
      virtual QPointF linePos(Grip, System** system) const;

   public:
      SLine(Score* s, ElementFlags = ElementFlag::NOTHING);
      SLine(const SLine&);

      virtual void layout() override;
      virtual SpannerSegment* layoutSystem(System*) override;

      bool readProperties(XmlReader& node);
      void writeProperties(XmlWriter& xml) const;
      virtual LineSegment* createLineSegment() = 0;
      void setLen(qreal l);
      using Element::bbox;
      virtual const QRectF& bbox() const override;

      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      bool diagonal() const               { return _diagonal; }
      void setDiagonal(bool v)            { _diagonal = v;    }

      Spatium lineWidth() const           { return _lineWidth;            }
      QColor lineColor() const            { return _lineColor;            }
      Qt::PenStyle lineStyle() const      { return _lineStyle;            }
      void setLineWidth(const Spatium& v) { _lineWidth = v;               }
      void setLineColor(const QColor& v)  { _lineColor = v;               }
      void setLineStyle(Qt::PenStyle v)   { _lineStyle = v;               }

      qreal dashLineLen() const           { return _dashLineLen; }
      void setDashLineLen(qreal val)      { _dashLineLen = val; }
      qreal dashGapLen() const            { return _dashGapLen; }
      void setDashGapLen(qreal val)       { _dashGapLen = val; }

      LineSegment* frontSegment() const   { return (LineSegment*)spannerSegments().front(); }
      LineSegment* backSegment() const    { return (LineSegment*)spannerSegments().back();  }
      LineSegment* takeFirstSegment()     { return (LineSegment*)spannerSegments().takeFirst(); }
      LineSegment* takeLastSegment()      { return (LineSegment*)spannerSegments().takeLast(); }
      LineSegment* segmentAt(int n) const { return (LineSegment*)spannerSegments().at(n); }

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual StyleIdx getPropertyStyle(P_ID) const override;

      friend class LineSegment;
      };


}     // namespace Ms
#endif

