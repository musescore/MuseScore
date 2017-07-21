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

#include "shape.h"
#include "segment.h"

namespace Ms {

//---------------------------------------------------------
//   translate
//---------------------------------------------------------

void Shape::translate(const QPointF& pt)
      {
      for (QRectF& r : *this)
            r.translate(pt);
      }

//---------------------------------------------------------
//   translated
//---------------------------------------------------------

Shape Shape::translated(const QPointF& pt) const
      {
      Shape s;
      for (const QRectF& r : *this)
            s.add(r.translated(pt));
      return s;
      }

//---------------------------------------------------------
//   draw
//    Draw outline of shape. For testing only.
//---------------------------------------------------------

void Shape::draw(QPainter* p) const
      {
      p->save();
      for (const QRectF& r : *this)
            p->drawRect(r);
      p->restore();
      }

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum vertical distance between the two shapes
//    so they dont touch.
//-------------------------------------------------------------------

qreal Shape::minHorizontalDistance(const Shape& a) const
      {
      qreal dist = -1000000.0;      // min real
      for (const QRectF& r2 : a) {
            qreal by1 = r2.top();
            qreal by2 = r2.bottom();
            for (const QRectF& r1 : *this) {
                  qreal ay1 = r1.top();
                  qreal ay2 = r1.bottom();
                  if (Ms::intersects(ay1, ay2, by1, by2)
                     || ((r1.height() == 0.0) && (r2.height() == 0.0) && (ay1 == by1))
                     || ((r1.width() == 0.0) || (r2.width() == 0.0)))
                        dist = qMax(dist, r1.right() - r2.left());
                  }
            }
      return dist;
      }

//-------------------------------------------------------------------
//   minVerticalDistance
//    a is located below of this shape.
//    Calculates the minimum distance between two shapes.
//-------------------------------------------------------------------

qreal Shape::minVerticalDistance(const Shape& a) const
      {
      qreal dist = -1000000.0;      // min real
      for (const QRectF& r2 : a) {
            qreal bx1 = r2.left();
            qreal bx2 = r2.right();
            for (const QRectF& r1 : *this) {
                  qreal ax1 = r1.left();
                  qreal ax2 = r1.right();
                  if (Ms::intersects(ax1, ax2, bx1, bx2))
                        dist = qMax(dist, r1.bottom() - r2.top());
                  }
            }
      return dist;
      }

//---------------------------------------------------------
//   left
//    compute left border
//---------------------------------------------------------

qreal Shape::left() const
      {
      qreal dist = 0.0;
      for (const QRectF& r : *this) {
            if (r.height() != 0.0 && r.left() < dist)
            // if (r.left() < dist)
                  dist = r.left();
            }
      return -dist;
      }

//---------------------------------------------------------
//   right
//    compute right border
//---------------------------------------------------------

qreal Shape::right() const
      {
      qreal dist = 0.0;
      for (const QRectF& r : *this) {
            if (r.right() > dist)
                  dist = r.right();
            }
      return dist;
      }

//---------------------------------------------------------
//   top
//---------------------------------------------------------

qreal Shape::top() const
      {
      qreal dist = 0.0;
      for (const QRectF& r : *this) {
            if (r.top() < dist)
                  dist = r.top();
            }
      return dist;
      }

//---------------------------------------------------------
//   bottom
//---------------------------------------------------------

qreal Shape::bottom() const
      {
      qreal dist = 0.0;
      for (const QRectF& r : *this) {
            if (r.bottom() > dist)
                  dist = r.bottom();
            }
      return dist;
      }

//---------------------------------------------------------
//   topDistance
//    p is on top of shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Shape::topDistance(const QPointF& p) const
      {
      qreal dist = 1000000.0;
      for (const QRectF& r : *this) {
            if (p.x() >= r.left() && p.x() < r.right())
                  dist = qMin(dist, r.top() - p.y());
            }
      return dist;
      }

//---------------------------------------------------------
//   bottomDistance
//    p is below the shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Shape::bottomDistance(const QPointF& p) const
      {
      qreal dist = 1000000.0;
      for (const QRectF& r : *this) {
            if (p.x() >= r.left() && p.x() < r.right())
                  dist = qMin(dist, p.y() - r.bottom());
            }
      return dist;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Shape::remove(const QRectF& r)
      {
      for (auto i = begin(); i != end(); ++i) {
            if (*i == r) {
                  erase(i);
                  return;
                  }
            }
      // qWarning("Shape::remove: QRectF not found in Shape");
      qFatal("Shape::remove: QRectF not found in Shape");
      }

void Shape::remove(const Shape& s)
      {
      for (const QRectF& r : s)
            remove(r);
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Shape::contains(const QPointF& p) const
      {
      for (const QRectF& r : *this) {
            if (r.contains(p))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

bool Shape::intersects(const QRectF& rr) const
      {
      for (const QRectF& r : *this) {
            if (r.intersects(rr))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void Shape::paint(QPainter& p)
      {
      for (const QRectF& r : *this)
            p.drawRect(r);
      }


#ifdef DEBUG_SHAPES

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Shape::dump(const char* p) const
      {
      printf("Shape dump: %p %s size %d\n", this, p, size());
      for (const QRectF& r : *this) {
            printf("   %f %f %f %f\n", r.x(), r.y(), r.width(), r.height());
            }

      }

//---------------------------------------------------------
//   testShapes
//---------------------------------------------------------

void testShapes()
      {
      printf("======test shapes======\n");

      //=======================
      //    minDistance()
      //=======================
      Shape a;
      Shape b;

      a.add(QRectF(-10, -10, 20, 20));
      qreal d = a.minHorizontalDistance(b);           // b is empty
      printf("      minHDistance (0.0): %f", d);
      if (d != 0.0)
            printf("   =====error");
      printf("\n");

      b.add(QRectF(0, 0, 10, 10));
      d = a.minHorizontalDistance(b);
      printf("      minHDistance (10.0): %f", d);
      if (d != 10.0)
            printf("   =====error");
      printf("\n");

      d = a.minVerticalDistance(b);
      printf("      minVDistance (10.0): %f", d);
      if (d != 10.0)
            printf("   =====error");
      printf("\n");
      }
#endif // DEBUG_SHAPES


} // namespace Ms

