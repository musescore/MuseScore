//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SHAPE_H__
#define __SHAPE_H__

namespace Ms {

#define DEBUG_SHAPES

class Segment;

//---------------------------------------------------------
//   Shape
//---------------------------------------------------------

class Shape : QList<QRectF> {
   public:
      Shape() {}
      void draw(QPainter*) const;
      void create(int staffIdx, Segment*);

      void add(const Shape& s)            { append(s); }
      void add(const QRectF& r)           { append(r); }
      void translate(const QPointF&);
      Shape translated(const QPointF&) const;
      qreal minHorizontalDistance(const Shape&) const;
      qreal minVerticalDistance(const Shape&) const;
      qreal left() const;
      qreal right() const;
      qreal top() const;
      qreal bottom() const;

      int size() const { return QList<QRectF>::size(); }
      void clear()     { QList<QRectF>::clear();       }

#ifdef DEBUG_SHAPES
      void dump(const char*) const;
#endif
      };

#ifdef DEBUG_SHAPES
extern void testShapes();
#endif

} // namespace Ms

#endif

