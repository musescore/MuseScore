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

#include <vector>
#include <QRectF>
#include <QPainter>

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
    qreal x;
    qreal y;
    qreal w;

    SkylineSegment(qreal _x, qreal _y, qreal _w)
        : x(_x), y(_y), w(_w) {}
};

//---------------------------------------------------------
//   SkylineLine
//---------------------------------------------------------

class SkylineLine
{
    const bool north;
    std::vector<SkylineSegment> seg;
    typedef std::vector<SkylineSegment>::iterator SegIter;
    typedef std::vector<SkylineSegment>::const_iterator SegConstIter;

    SegIter insert(SegIter i, qreal x, qreal y, qreal w);
    void append(qreal x, qreal y, qreal w);
    SegIter find(qreal x);
    SegConstIter find(qreal x) const;

public:
    SkylineLine(bool n)
        : north(n) {}
    void add(const Shape& s);
    void add(const QRectF& r);
    void add(qreal x, qreal y, qreal w);
    void clear() { seg.clear(); }
    void paint(QPainter&) const;
    void dump() const;
    qreal minDistance(const SkylineLine&) const;
    qreal max() const;
    bool valid() const;
    bool valid(const SkylineSegment& s) const;
    bool isNorth() const { return north; }

    SegIter begin() { return seg.begin(); }
    SegConstIter begin() const { return seg.begin(); }
    SegIter end() { return seg.end(); }
    SegConstIter end() const { return seg.end(); }
};

//---------------------------------------------------------
//   Skyline
//---------------------------------------------------------

class Skyline
{
    SkylineLine _north;
    SkylineLine _south;

public:
    Skyline()
        : _north(true), _south(false) {}

    void clear();
    void add(const Shape& s);
    void add(const QRectF& r);

    qreal minDistance(const Skyline&) const;

    SkylineLine& north() { return _north; }
    SkylineLine& south() { return _south; }
    const SkylineLine& north() const { return _north; }
    const SkylineLine& south() const { return _south; }

    void paint(QPainter&) const;
    void dump(const char*, bool north = false) const;
};
} // namespace Ms

#endif
