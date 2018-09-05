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

#include "skyline.h"
#include "segment.h"

namespace Ms {

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Skyline::add(const QRectF& r)
      {
      north.add(r.x(), r.top(), r.width());
      south.add(r.x(), r.bottom(), r.width());
      }

void Skyline::add(const QRectF& r, const char*)
      {
      north.add(r.x(), r.top(), r.width());
      south.add(r.x(), r.bottom(), r.width());
      }

void Skyline::add(const Shape& s)
      {
      for (const auto& r : s) {
#ifndef NDEBUG
            add(r, r.text);
#else
            add(r, 0);
#endif
            }
      }

void SkylineLine::add(qreal x, qreal y, qreal w)
      {
//      Q_ASSERT(w >= 0.0);
      if (x < 0.0)
            return;

      qreal cx = 0.0;
      qreal cy = 0.0;
      for (auto i = begin(); i != end(); ++i) {
            cy = i->y;
            if ((x + w) <= cx)                                          // A
                  return; // break;
            if (x > (cx + i->w)) {                                      // B
                  cx += i->w;
                  continue;
                  }
            if ((north && (cy <= y)) || (!north && (cy > y))) {
                  cx += i->w;
                  continue;
                  }
            if ((x >= cx) && ((x+w) < (cx+i->w))) {                     // (E) insert segment
//                  printf("    insert at %f %f   x:%f w:%f\n", cx, i->w, x, w);
                  qreal w1 = x - cx;
                  qreal w2 = w;
                  qreal w3 = i->w - (w1 + w2);
                  if (w1 > 0.0) {
                        i->w = w1;
                        ++i;
                        i = insert(i, SkylineSegment(y, w2));
                        }
                  else
                        i->w = w2;
                  if (w3 > 0.0) {
                        ++i;
                        insert(i, SkylineSegment(cy, w3));
                        }
                  return;
                  }
            else if ((x <= cx) && ((x + w) >= (cx + i->w))) {               // F
//                  printf("    change(F) cx %f y %f\n", cx, y);
                  i->y = y;
                  }
            else if (x < cx) {                                          // C
                  qreal w1 = x + w - cx;
                  i->w    -= w1;
//                  printf("    add(C) cx %f y %f w %f w1 %f\n", cx, y, w1, i->w);
                  insert(i, SkylineSegment(y, w1));
                  return;
                  }
            else {                                                      // D
                  qreal w1 = x - cx;
                  qreal w2 = i->w - w1;
                  i->w = w1;
                  cx  += w1;
//                  printf("    add(D) %f %f\n", y, w2);
                  ++i;
                  i = insert(i, SkylineSegment(y, w2));
                  }
            cx += i->w;
            }
      if (x >= cx) {
            if (x > cx) {
                  cy = 0.0;
//                  printf("    append1 %f %f\n", cy, x - cx);
                  push_back(SkylineSegment(cy, x - cx));
                  }
//            printf("    append2 %f %f\n", y, w);
            push_back(SkylineSegment(y, w));
            }
      }

//---------------------------------------------------------
//   empty
//---------------------------------------------------------

bool Skyline::empty() const
      {
      return north.empty() && south.empty();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Skyline::clear()
      {
      north.clear();
      south.clear();
      }

//---------------------------------------------------------
//   translate
//---------------------------------------------------------

void Skyline::translate(const QPointF&)
      {
      }

void Skyline::translateX(qreal)
      {
      }
void Skyline::translateY(qreal)
      {
      }

//---------------------------------------------------------
//   translated
//---------------------------------------------------------

Skyline Skyline::translated(const QPointF&) const
      {
      Skyline s = *this;
      return s;
      }

//-------------------------------------------------------------------
//   minDistance
//    a is located below this skyline.
//    Calculates the minimum distance between two skylines
//-------------------------------------------------------------------

qreal Skyline::minDistance(const Skyline& s) const
      {
      return south.minDistance(s.north);
      }

qreal SkylineLine::minDistance(const SkylineLine& sl) const
      {
      qreal dist = -1000000.0;      // min real

      qreal x1 = 0.0;
      qreal x2 = 0.0;
      auto k   = sl.begin();
      for (auto i = begin(); i != end(); ++i) {
            while (k != sl.end() && (x2 + k->w) < x1) {
                  x2 += k->w;
                  ++k;
                  }
            if (k == sl.end())
                  break;
            for (;;) {
                  if ((x1+i->w > x2) && (x1 < x2+k->w))
                        dist = qMax(dist, i->y - k->y);
                  if (x2 + k->w < x1 + i->w) {
                        x2 += k->w;
                        ++k;
                        if (k == sl.end())
                              break;
                        }
                  else
                        break;
                  }
            if (k == sl.end())
                  break;
            x1 += i->w;
            }
  //    printf("dist %f\n", dist);
      return dist;
      }

//---------------------------------------------------------
//   top
//---------------------------------------------------------

qreal Skyline::top() const
      {
      qreal dist = 0.0;
      return dist;
      }

//---------------------------------------------------------
//   bottom
//---------------------------------------------------------

qreal Skyline::bottom() const
      {
      qreal dist = 0.0;
      return dist;
      }

//---------------------------------------------------------
//   topDistance
//    p is on top of shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Skyline::topDistance(const QPointF&) const
      {
      qreal dist = 1000000.0;
      return dist;
      }

//---------------------------------------------------------
//   bottomDistance
//    p is below the shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Skyline::bottomDistance(const QPointF&) const
      {
      qreal dist = 1000000.0;
      return dist;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Skyline::remove(const QRectF&)
      {
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Skyline::contains(const QPointF&) const
      {
      return false;
      }

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

bool Skyline::intersects(const QRectF&) const
      {
      return false;
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void Skyline::paint(QPainter& p) const
      {
      p.save();
      p.setBrush(Qt::NoBrush);
      p.setPen(QPen(QBrush(Qt::darkYellow), 0.8));
      north.paint(p);
      p.setPen(QPen(QBrush(Qt::green), 0.8));
      south.paint(p);
      p.restore();
      }

void SkylineLine::paint(QPainter& p) const
      {
      qreal x1 = 0.0;
      qreal x2;
      qreal y = 0.0;

      for (const SkylineSegment& s : *this) {
            x2 = x1 + s.w;
            p.drawLine(QLineF(x1, y, x1, s.y));
            y  = s.y;
            p.drawLine(QLineF(x1, y, x2, y));
            x1 = x2;
            }
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Skyline::dump(const char* p) const
      {
      printf("Skyline dump: %p %s\n", this, p);
      south.dump();
      }

void SkylineLine::dump() const
      {
      qreal x = 0.0;
      for (const SkylineSegment& s : *this) {
            printf("   x %f y %f w %f\n", x, s.y, s.w);
            x += s.w;
            }
      }

} // namespace Ms

