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
#include "shape.h"

namespace Ms {

// #define SKL_DEBUG

#ifdef SKL_DEBUG
#define DP(...)   printf(__VA_ARGS__)
#else
#define DP(...)
#endif


//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Skyline::add(const QRectF& r)
      {
      _north.add(r.x(), r.top(), r.width());
      _south.add(r.x(), r.bottom(), r.width());
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

SkylineLine::SegIter SkylineLine::insert(SegIter i, qreal x, qreal y, qreal w)
      {
      const qreal xr = x + w;
      // Only x coordinate change is handled here as width change gets handled
      // in SkylineLine::add().
      if (i != seg.end() && xr > i->x)
            i->x = xr;
      return seg.emplace(i, x, y, w);
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void SkylineLine::append(qreal x, qreal y, qreal w)
      {
      seg.emplace_back(x, y, w);
      }

//---------------------------------------------------------
//   getApproxPosition
//---------------------------------------------------------

SkylineLine::SegIter SkylineLine::find(qreal x)
      {
      auto it = std::upper_bound(seg.begin(), seg.end(), x, [](qreal x, const SkylineSegment& s) { return x < s.x; });
      if (it == seg.begin())
            return it;
      return (--it);
      }

SkylineLine::SegConstIter SkylineLine::find(qreal x) const
      {
      return const_cast<SkylineLine*>(this)->find(x);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SkylineLine::add(const Shape& s)
      {
      for (const auto& r : s)
            add(r);
      }

void SkylineLine::add(const QRectF& r)
      {
      if (north)
            add(r.x(), r.top(), r.width());
      else
            add(r.x(), r.bottom(), r.width());
      }

void Skyline::add(const Shape& s)
      {
      for (const auto& r : s)
            add(r);
      }

void SkylineLine::add(qreal x, qreal y, qreal w)
      {
//      Q_ASSERT(w >= 0.0);
      if (x < 0.0) {
            w -= -x;
            x = 0.0;
            if (w <= 0.0)
                  return;
            }

      DP("===add  %f %f %f\n", x, y, w);

      SegIter i = find(x);
      qreal cx = seg.empty() ? 0.0 : i->x;
      for (; i != seg.end(); ++i) {
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
                  DP("    insert at %f %f   x:%f w:%f\n", cx, i->w, x, w);
                  qreal w1 = x - cx;
                  qreal w2 = w;
                  qreal w3 = i->w - (w1 + w2);
                  if (w1 > 0.0000001) {
                        i->w = w1;
                        ++i;
                        i = insert(i, x, y, w2);
                        DP("       A w1 %f w2 %f\n", w1, w2);
                        }
                  else {
                        i->w = w2;
                        i->y = y;
                        DP("       B w2 %f\n", w2);
                        }
                  if (w3 > 0.0000001) {
                        ++i;
                        DP("       C w3 %f\n", w3);
                        insert(i, x + w2, cy, w3);
                        }
                  return;
                  }
            else if ((x <= cx) && ((x + w) >= (cx + i->w))) {               // F
                  DP("    change(F) cx %f y %f\n", cx, y);
                  i->y = y;
                  }
            else if (x < cx) {                                          // C
                  qreal w1 = x + w - cx;
                  i->w    -= w1;
                  DP("    add(C) cx %f y %f w %f w1 %f\n", cx, y, w1, i->w);
                  insert(i, cx, y, w1);
                  return;
                  }
            else {                                                      // D
                  qreal w1 = x - cx;
                  qreal w2 = i->w - w1;
                  if (w2 > 0.0000001) {
                        i->w = w1;
                        cx  += w1;
                        DP("    add(D) %f %f\n", y, w2);
                        ++i;
                        i = insert(i, cx, y, w2);
                        }
                  }
            cx += i->w;
            }
      if (x >= cx) {
            if (x > cx) {
                  qreal cy = north ? DBL_MAX : -DBL_MAX;
                  DP("    append1 %f %f\n", cy, x - cx);
                  append(cx, cy, x - cx);
                  }
            DP("    append2 %f %f\n", y, w);
            append(x, y, w);
            }
      else if (x + w > cx)
            append(cx, y, x + w - cx);
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
      qreal dist = -DBL_MAX;

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
                  if ((x1 + i->w > x2) && (x1 < x2 + k->w))
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
      return dist;
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void Skyline::paint(QPainter& p) const
      {
      p.save();

      p.setBrush(Qt::NoBrush);
      QTransform transform = p.worldTransform();
      p.setPen(QPen(QBrush(Qt::darkYellow), 2.0 / transform.m11()));
      _north.paint(p);
      p.setPen(QPen(QBrush(Qt::green), 2.0 / transform.m11()));
      _south.paint(p);
      p.restore();
      }

void SkylineLine::paint(QPainter& p) const
      {
      qreal x1 = 0.0;
      qreal x2;
      qreal y = 0.0;

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

bool SkylineLine::valid() const
      {
      return !seg.empty();
      }

bool SkylineLine::valid(const SkylineSegment& s) const
      {
      return north ? (s.y != DBL_MAX) : (s.y != -DBL_MAX);
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

//---------------------------------------------------------
//   max
//---------------------------------------------------------

qreal SkylineLine::max() const
      {
      qreal val;
      if (north) {
            val = DBL_MAX;
            for (const SkylineSegment& s : *this)
                  val = qMin(val, s.y);
            }
      else {
            val = -DBL_MAX;
            for (const SkylineSegment& s : *this)
                  val = qMax(val, s.y);
            }
      return val;
      }


} // namespace Ms

