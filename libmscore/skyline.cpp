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

static const qreal MAXIMUM_Y = 1000000.0;
static const qreal MINIMUM_Y = -1000000.0;

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Skyline::add(const QRectF& r)
      {
      _north.add(r.x(), r.top(), r.width());
      _south.add(r.x(), r.bottom(), r.width());
      }

void Skyline::add(const Shape& s)
      {
      for (const auto& r : s)
            add(r);
      }

void SkylineLine::add(qreal x, qreal y, qreal w)
      {
//      Q_ASSERT(w >= 0.0);
      if (x < 0.0)
            return;

      qreal cx = 0.0;
      for (auto i = begin(); i != end(); ++i) {
            qreal cy = i->y;
            if ((x + w) <= cx)                                          // A
                  return; // break;
            if (x > (cx + i->w)) {                                      // B
                  cx += i->w;
                  continue;
                  }
            if ((north && (cy <= y)) || (!north && (cy >= y))) {
                  cx += i->w;
                  continue;
                  }
            if ((x >= cx) && ((x+w) < (cx+i->w))) {                     // (E) insert segment
//                  printf("    insert at %f %f   x:%f w:%f\n", cx, i->w, x, w);
                  qreal w1 = x - cx;
                  qreal w2 = w;
                  qreal w3 = i->w - (w1 + w2);
                  if (w1 > 0.0000001) {
                        i->w = w1;
                        ++i;
                        i = insert(i, SkylineSegment(y, w2));
//                        printf("       A w1 %f w2 %f\n", w1, w2);
                        }
                  else {
                        i->w = w2;
//                        printf("       B w2 %f\n", w2);
                        }
                  if (w3 > 0.0000001) {
                        ++i;
//                        printf("       C w3 %f\n", w3);
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
                  if (w2 > 0.0000001) {
                        i->w = w1;
                        cx  += w1;
//                        printf("    add(D) %f %f\n", y, w2);
                        ++i;
                        i = insert(i, SkylineSegment(y, w2));
                        }
                  }
            cx += i->w;
            }
      if (x >= cx) {
            if (x > cx) {
                  qreal cy = north ? MAXIMUM_Y : MINIMUM_Y;
//                  printf("    append1 %f %f\n", cy, x - cx);
                  push_back(SkylineSegment(cy, x - cx));
                  }
//            printf("    append2 %f %f\n", y, w);
            push_back(SkylineSegment(y, w));
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Skyline::clear()
      {
      _north.clear();
      _south.clear();
      }

//-------------------------------------------------------------------
//   minDistance
//    a is located below this skyline.
//    Calculates the minimum distance between two skylines
//-------------------------------------------------------------------

qreal Skyline::minDistance(const Skyline& s) const
      {
      return south().minDistance(s.north());
      }

qreal SkylineLine::minDistance(const SkylineLine& sl) const
      {
      qreal dist = MINIMUM_Y;

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
                  if ((x1 + i->w > x2) && (x1 < x2 + k->w)) {
                        qreal odist = dist;
                        dist = qMax(dist, i->y - k->y);
//                        printf("%f = Max (%f. %f - %f)\n", dist, odist, i->y, k->y);
                        }
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
      return dist;
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void Skyline::paint(QPainter& p) const
      {
      p.save();

      p.setBrush(Qt::NoBrush);
      p.setPen(QPen(QBrush(Qt::darkYellow), 0.8));
      _north.paint(p);
      p.setPen(QPen(QBrush(Qt::green), 0.8));
      _south.paint(p);
      p.restore();
      }

void SkylineLine::paint(QPainter& p) const
      {
      qreal x1 = 0.0;
      qreal x2;
      qreal y;

      bool pvalid = false;
      for (const SkylineSegment& s : *this) {
            x2 = x1 + s.w;
            if (valid(s)) {
                  if (pvalid)
                        p.drawLine(QLineF(x1, y, x1, s.y));
                  y  = s.y;
                  p.drawLine(QLineF(x1, y, x2, y));
                  pvalid = true;
                  }
            else
                  pvalid = false;
            x1 = x2;
            }
      }

bool SkylineLine::valid(const SkylineSegment& s) const
      {
      return north ? (s.y != MAXIMUM_Y) : (s.y != MINIMUM_Y);
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Skyline::dump(const char* p, bool n) const
      {
      printf("Skyline dump: %p %s\n", this, p);
      if (n)
            _north.dump();
      else
            _south.dump();
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

