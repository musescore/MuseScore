//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: line.h 5214 2012-01-11 09:45:40Z wschweer $
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

class SLine;
class System;
class MuseScoreView;
class QPainter;

//---------------------------------------------------------
//   @@ LineSegment
///   Virtual base class for segmented lines segments
///   (OttavaSegment, HairpinSegment, TrillSegment...)
//
///   This class describes one segment of an segmented
///   line object. Line objects can span multiple staves.
///   For every staff a segment is created.
//---------------------------------------------------------

class LineSegment : public SpannerSegment {
      Q_OBJECT

   protected:
      QPointF _p2;
      QPointF _userOff2;

      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void setGrip(int grip, const QPointF& p);
      virtual QPointF getGrip(int) const;
      virtual QPointF gripAnchor(int) const;
      virtual void layout() {}

   public:
      LineSegment(Score* s);
      LineSegment(const LineSegment&);
      virtual LineSegment* clone() const = 0;
      virtual void draw(QPainter*) const = 0;
      SLine* line() const                         { return (SLine*)spanner(); }
      const QPointF& userOff2() const             { return _userOff2;       }
      void setUserOff2(const QPointF& o)          { _userOff2 = o;          }
      void setUserXoffset2(qreal x)               { _userOff2.setX(x);      }
      void setPos2(const QPointF& p)              { _p2 = p;                }
      QPointF pos2() const                        { return _p2 + _userOff2; }
      const QPointF& ipos2() const                { return _p2;             }
      virtual void reset();
      virtual void spatiumChanged(qreal, qreal);
      virtual QPointF pagePos() const;
      virtual bool isEdited(SpannerSegment*) const;

      friend class SLine;
      virtual void read(const QDomElement&);
      bool readProperties(const QDomElement&);
      };

//---------------------------------------------------------
//   @@ SLine
///    virtual base class for Hairpin, Trill and TextLine
//---------------------------------------------------------

class SLine : public Spanner {
      Q_OBJECT

   protected:
      bool _diagonal;

   public:
      SLine(Score* s);
      SLine(const SLine&);

      virtual void layout();
      bool readProperties(const QDomElement& node);
      void writeProperties(Xml& xml, const SLine* proto = 0) const;
      virtual LineSegment* createLineSegment() = 0;
      void setLen(qreal l);
      virtual const QRectF& bbox() const;

      virtual QPointF linePos(int grip, System** system);

      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);

      bool diagonal() const         { return _diagonal; }
      void setDiagonal(bool v)      { _diagonal = v;    }
      int tick()  const;
      int tick2() const;

      LineSegment* frontSegment() const   { return (LineSegment*)spannerSegments().front(); }
      LineSegment* backSegment() const    { return (LineSegment*)spannerSegments().back();  }
      LineSegment* takeFirstSegment()     { return (LineSegment*)spannerSegments().takeFirst(); }
      LineSegment* takeLastSegment()      { return (LineSegment*)spannerSegments().takeLast(); }
      LineSegment* segmentAt(int n) const { return (LineSegment*)spannerSegments().at(n); }
      };

#endif

