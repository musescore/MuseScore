//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SKYLINE_H__
#define __SKYLINE_H__

namespace Ms {

#ifndef NDEBUG
#define DEBUG_SKYLINE    // enable skyline debugging
#endif

class Segment;
class Shape;

//---------------------------------------------------------
//   SkylineSegment
//---------------------------------------------------------

struct SkylineSegment {
      qreal y;
      qreal w;

      SkylineSegment(qreal _y, qreal _w) : y(_y), w(_w) {}
      };

//---------------------------------------------------------
//   SkylineLine
//---------------------------------------------------------

struct SkylineLine : public std::vector<SkylineSegment> {
      bool north;

   public:
      void add(qreal x, qreal y, qreal w);
      void paint(QPainter&) const;
      void dump() const;
      void setNorth(bool v) { north = v; }
      };

//---------------------------------------------------------
//   Skyline
//---------------------------------------------------------

class Skyline {
      SkylineLine north;
      SkylineLine south;

   public:
      Skyline() {
            north.setNorth(true);
            south.setNorth(false);
            }
      Skyline(const QRectF& r) {
            north.setNorth(true);
            south.setNorth(false);
            add(r);
            }

      void add(const Shape& s);
      void add(const QRectF& r);
      void add(const QRectF& r, const char*);
      void remove(const QRectF&);

      void translate(const QPointF&);
      void translateX(qreal);
      void translateY(qreal);
      Skyline translated(const QPointF&) const;

      qreal minDistance(const Skyline&) const;
      qreal topDistance(const QPointF&) const;
      qreal bottomDistance(const QPointF&) const;
      qreal top() const;
      qreal bottom() const;

      bool empty() const;
      void clear();

      bool contains(const QPointF&) const;
      bool intersects(const QRectF& rr) const;
      void paint(QPainter&) const;

#ifndef NDEBUG
      void dump(const char*) const;
#endif
      };

} // namespace Ms

#endif

