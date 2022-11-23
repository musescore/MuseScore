/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_DRAW_GEOMETRY_H
#define MU_DRAW_GEOMETRY_H

#include <vector>
#include <cmath>
#include <cassert>

#include "realfn.h"

#ifndef NO_QT_SUPPORT
#include <QPair>
#include <QRectF>
#include <QSizeF>
#include <QPointF>
#include <QLineF>
#include <QPolygonF>
#endif

namespace mu {
template<typename T>
inline bool isEqual(T a1, T a2)
{
    if constexpr (std::is_same<T, double>::value) {
        return RealIsEqual(a1, a2);
    } else {
        return a1 == a2;
    }
}

// ====================================
// Point
// ====================================
template<typename T>
class PointX
{
public:
    inline PointX() = default;
    inline PointX(T x, T y)
        : m_x(x), m_y(y) {}

    inline bool isNull() const { return isEqual(m_x, T()) && isEqual(m_y, T()); }

    inline void setX(T x) { m_x = x; }
    inline void setY(T y) { m_y = y; }
    inline T x() const { return m_x; }
    inline T y() const { return m_y; }

    //! NOTE I don't like this methods, but now it a lot of using
    inline T& rx() { return m_x; }
    inline T& ry() { return m_y; }

    inline bool operator==(const PointX<T>& p) const { return isEqual(p.m_x, m_x) && isEqual(p.m_y, m_y); }
    inline bool operator!=(const PointX<T>& p) const { return !this->operator ==(p); }

    inline PointX<T> operator-() const { return PointX<T>(-m_x, -m_y); }
    inline PointX<T> operator-(const PointX<T>& p) const { return PointX<T>(m_x - p.m_x, m_y - p.m_y); }
    inline PointX<T> operator+(const PointX<T>& p) const { return PointX<T>(m_x + p.m_x, m_y + p.m_y); }

    inline PointX<T>& operator+=(const PointX<T>& p) { m_x += p.m_x; m_y += p.m_y; return *this; }
    inline PointX<T>& operator-=(const PointX<T>& p) { m_x -= p.m_x; m_y -= p.m_y; return *this; }
    inline PointX<T>& operator*=(T c) { m_x *= c; m_y *= c; return *this; }
    inline PointX<T>& operator/=(T divisor) { m_x /= divisor; m_y /= divisor; return *this; }

    inline T manhattanLength() const { return std::abs(m_x) + std::abs(m_y); }

    static inline double dotProduct(const PointX<T>& p1, const PointX<T>& p2) { return p1.m_x * p2.m_x + p1.m_y * p2.m_y; }

    inline void normalize()
    {
        // Need some extra precision if the length is very small.
        double len = double(m_x) * double(m_x) + double(m_y) * double(m_y);
        if (RealIsNull(len - 1.0f) || RealIsNull(len)) {
            return;
        }

        len = std::sqrt(len);

        m_x /= len;
        m_y /= len;
    }

    inline PointX<T> normalized() const
    {
        PointX<T> copy = *this;
        copy.normalize();
        return copy;
    }

#ifndef NO_QT_SUPPORT
    static PointX<T> fromQPointF(const QPointF& p) { return PointX<T>(p.x(), p.y()); }
    inline QPointF toQPointF() const { return QPointF(m_x, m_y); }
    inline QPoint toQPoint() const { return QPoint(m_x, m_y); }

    inline PointX<T>& operator=(const QPointF& p) { m_x = p.x(); m_y = p.y(); return *this; }
#endif

private:
    //! NOTE We should not swap fields
    //! We should not add new fields
    //! If we really need to do this, then we need to change the implementation of QPainterProvider
    T m_x = T();
    T m_y = T();
};

template<typename T>
inline PointX<T> operator*(const PointX<T>& p, T c) { return PointX<T>(p.x() * c, p.y() * c); }
template<typename T>
inline PointX<T> operator*(T c, const PointX<T>& p) { return PointX<T>(p.x() * c, p.y() * c); }

template<typename T>
inline PointX<T> operator/(const PointX<T>& p, T c) { return PointX<T>(p.x() / c, p.y() / c); }

using PointF = PointX<double>;
using Point = PointX<int>;

#ifndef NO_QT_SUPPORT
inline QPointF operator+(const QPointF& p1, const PointF& p2) { return QPointF(p1.x() + p2.x(), p1.y() + p2.y()); }
#endif

// ====================================
// PairF
// ====================================
class PairF : public std::pair<double, double>
{
public:
    PairF() = default;
    PairF(double f, double s)
        : std::pair<double, double>(f, s) {}

#ifndef NO_QT_SUPPORT
    static PairF fromQPairF(const QPair<double, double>& v) { return PairF(v.first, v.second); }
    QPair<double, double> toQPairF() const { return QPair<double, double>(first, second); }
#endif
};

// ====================================
// Line
// ====================================
template<typename T>
class LineX
{
public:
    inline LineX() = default;
    inline LineX(const PointX<T>& p1, const PointX<T>& p2)
        : m_p1(p1), m_p2(p2) {}

    inline LineX(T x1, T y1, T x2, T y2)
        : m_p1(PointX<T>(x1, y1)), m_p2(PointX<T>(x2, y2)) {}

    inline const PointX<T>& p1() const { return m_p1; }
    inline const PointX<T>& p2() const { return m_p2; }

    inline double x1() const { return m_p1.x(); }
    inline double y1() const { return m_p1.y(); }
    inline double x2() const { return m_p2.x(); }
    inline double y2() const { return m_p2.y(); }

    inline void setP1(const PointX<T>& p) { m_p1 = p; }
    inline void setP2(const PointX<T>& p) { m_p2 = p; }
    inline void setLine(T aX1, T aY1, T aX2, T aY2) { m_p1 = PointX<T>(aX1, aY1); m_p2 = PointX<T>(aX2, aY2); }

    inline bool operator==(const LineX<T>& l) const { return m_p1 == l.m_p1 && m_p2 == l.m_p2; }
    inline bool operator!=(const LineX<T>& l) const { return !this->operator ==(l); }

    inline PointX<T> pointAt(T t) const { return PointX<T>(m_p1.x() + (m_p2.x() - m_p1.x()) * t, m_p1.y() + (m_p2.y() - m_p1.y()) * t); }

    inline void translate(const PointX<T>& point) { m_p1 += point; m_p2 += point; }
    inline LineX<T> translated(const PointX<T>& p) const { return LineX<T>(m_p1 + p, m_p2 + p); }

#ifndef NO_QT_SUPPORT
    static LineX<T> fromQLineF(const QLineF& l) { return LineX<T>(l.x1(), l.y1(), l.x2(), l.y2()); }
    inline QLineF toQLineF() const { return QLineF(m_p1.toQPointF(), m_p2.toQPointF()); }
#endif

private:
    PointX<T> m_p1;
    PointX<T> m_p2;
};

using LineF = LineX<double>;
using Line = LineX<int>;

// ====================================
// Size
// ====================================
template<typename T>
class SizeX
{
public:
    inline SizeX() = default;
    inline SizeX(T w, T h)
        : m_w(w), m_h(h) {}

    inline bool isNull() const { return isEqual(m_w, T()) && isEqual(m_h, T()); }

    inline T width() const { return m_w; }
    inline T height() const { return m_h; }

    inline void setWidth(T w) { m_w = w; }
    inline void setHeight(T h) { m_h = h; }

    inline bool operator==(const SizeX<T>& s) const { return isEqual(s.m_w, m_w) && isEqual(s.m_h, m_h); }
    inline bool operator!=(const SizeX<T>& s) const { return !this->operator ==(s); }

    inline SizeX<T> transposed() const { return SizeX<T>(m_h, m_w); }

#ifndef NO_QT_SUPPORT
    static SizeX<T> fromQSizeF(const QSizeF& s) { return SizeX<T>(s.width(), s.height()); }
    inline QSizeF toQSizeF() const { return QSizeF(m_w, m_h); }
#endif

private:
    T m_w = T();
    T m_h = T();
};

template<typename T>
inline SizeX<T> operator*(const SizeX<T>& s, double c) { return SizeX<T>(s.width() * c, s.height() * c); }

template<typename T>
inline SizeX<T> operator*(double c, const SizeX<T>& s) { return SizeX<T>(s.width() * c, s.height() * c); }

template<typename T>
inline SizeX<T> operator/(const SizeX<T>& s, T c) { assert(!isEqual(c, T())); return SizeX<T>(s.width() / c, s.height() / c); }

using Size = SizeX<int>;
using SizeF = SizeX<double>;

class ScaleF : public SizeX<double>
{
public:
    ScaleF() = default;
    ScaleF(double w, double h)
        : SizeX<double>(w, h) {}

#ifndef NO_QT_SUPPORT
    static ScaleF fromQSizeF(const QSizeF& s) { return ScaleF(s.width(), s.height()); }
#endif
};

// ====================================
// Rect
// ====================================
template<typename T>
class RectX
{
public:
    inline RectX() = default;
    inline RectX(T x, T y, T w, T h)
        : m_x(x), m_y(y), m_w(w), m_h(h) {}
    inline RectX(const PointX<T>& topLeft, const PointX<T>& bottomRight)
        : m_x(topLeft.x()), m_y(topLeft.y()), m_w(bottomRight.x() - topLeft.x()), m_h(bottomRight.y() - topLeft.y()) {}
    inline RectX(const PointX<T>& atopLeft, const SizeX<T>& asize)
        : m_x(atopLeft.x()), m_y(atopLeft.y()), m_w(asize.width()), m_h(asize.height()) {}

    inline bool isNull() const { return isEqual(m_w, T()) && isEqual(m_h, T()); }
    inline bool isEmpty() const { return m_w <= T() || m_h <= T(); }
    inline bool isValid() const { return m_w > T() && m_h > T(); }

    inline T x() const { return m_x; }
    inline T y() const { return m_y; }
    inline T width() const { return m_w; }
    inline T height() const { return m_h; }
    inline SizeX<T> size() const { return SizeX<T>(m_w, m_h); }

    inline T left() const noexcept { return m_x; }
    inline T top() const noexcept { return m_y; }
    inline T right() const noexcept { return m_x + m_w; }
    inline T bottom() const noexcept { return m_y + m_h; }

    inline PointX<T> topLeft() const { return PointX<T>(m_x, m_y); }
    inline PointX<T> bottomRight() const { return PointX<T>(m_x + m_w, m_y + m_h); }
    inline PointX<T> topRight() const { return PointX<T>(m_x + m_w, m_y); }
    inline PointX<T> bottomLeft() const { return PointX<T>(m_x, m_y + m_h); }
    inline PointX<T> center() const { return PointX<T>(m_x + m_w / 2, m_y + m_h / 2); }

    inline void setCoords(T xp1, T yp1, T xp2, T yp2) { m_x = xp1; m_y = yp1; m_w = xp2 - xp1; m_h = yp2 - yp1; }
    inline void setRect(double ax, double ay, double aaw, double aah) { m_x = ax; m_y = ay; m_w = aaw; m_h = aah; }

    inline void setHeight(T h) { m_h = h; }
    inline void setWidth(T w) { m_w = w; }
    inline void setSize(const SizeX<T>& s) { m_w = s.width(); m_h = s.height(); }

    inline void setLeft(T pos) { T diff = pos - m_x; m_x += diff; m_w -= diff; }
    inline void setRight(T pos) { m_w = pos - m_x; }
    inline void setTop(T pos) { T diff = pos - m_y; m_y += diff; m_h -= diff; }
    inline void setBottom(T pos) { m_h = pos - m_y; }
    inline void setX(T x) { m_x = x; }
    inline void setY(T y) { m_y = y; }

    inline void setTopLeft(const PointX<T>& p) { setLeft(p.x()); setTop(p.y()); }
    inline void setTopRight(const PointX<T>& p) { setRight(p.x()); setTop(p.y()); }
    inline void setBottomLeft(const PointX<T>& p) { setLeft(p.x()); setBottom(p.y()); }
    inline void setBottomRight(const PointX<T>& p) { setRight(p.x()); setBottom(p.y()); }

    inline bool operator==(const RectX<T>& r) const
    {
        return isEqual(r.m_x, m_x) && isEqual(r.m_y, m_y) && isEqual(r.m_w, m_w) && isEqual(r.m_h, m_h);
    }

    inline bool operator!=(const RectX<T>& r) const { return !this->operator ==(r); }

    inline void moveTo(double ax, double ay) { m_x = ax; m_y = ay; }
    inline void moveTo(const PointX<T>& p) { m_x = p.x(); m_y = p.y(); }
    inline void moveCenter(const PointX<T>& p) { m_x = p.x() - m_w / 2; m_y = p.y() - m_h / 2; }
    inline void moveTop(double pos) { m_y = pos; }

    inline void translate(T dx, T dy) { m_x += dx; m_y += dy; }
    inline void translate(const PointX<T>& p) { m_x += p.x(); m_y += p.y(); }

    inline RectX<T> translated(T dx, T dy) const { return RectX<T>(m_x + dx, m_y + dy, m_w, m_h); }
    inline RectX<T> translated(const PointX<T>& p) const { return RectX<T>(m_x + p.x(), m_y + p.y(), m_w, m_h); }

    inline void adjust(double xp1, double yp1, double xp2, double yp2) { m_x += xp1; m_y += yp1; m_w += xp2 - xp1; m_h += yp2 - yp1; }
    inline RectX<T> adjusted(T xp1, T yp1, T xp2, T yp2) const { return RectX<T>(m_x + xp1, m_y + yp1, m_w + xp2 - xp1, m_h + yp2 - yp1); }

    bool contains(const PointX<T>& p) const;
    bool contains(const RectX<T>& r) const;

    bool intersects(const RectX<T>& r) const;

    RectX<T> united(const RectX<T>& r) const;
    inline RectX<T>& unite(const RectX<T>& r) { *this = united(r); return *this; }

    RectX<T> intersected(const RectX<T>& r) const;
    inline RectX<T>& intersect(const RectX<T>& r) { *this = intersected(r); return *this; }

    //! NOTE I don't like this operators, but now it a lot of using
    inline RectX<T> operator|(const RectX<T>& r) const { return united(r); }
    inline RectX<T> operator&(const RectX<T>& r) const { return intersected(r); }
    inline RectX<T>& operator|=(const RectX<T>& r) { return unite(r); }
    inline RectX<T>& operator&=(const RectX<T>& r) { return intersect(r); }

    RectX<T> normalized() const;

#ifndef NO_QT_SUPPORT
    static RectX<T> fromQRectF(const QRectF& r) { return RectX<T>(r.x(), r.y(), r.width(), r.height()); }
    inline QRectF toQRectF() const { return QRectF(m_x, m_y, m_w, m_h); }
    inline QRect toQRect() const { return QRect(m_x, m_y, m_w, m_h); }

    inline RectX<T>& operator=(const QRectF& r) { m_x = r.x(); m_y = r.y(); m_w = r.width(); m_h = r.height(); return *this; }

    inline RectX<T> united(const QRectF& r) const { return united(RectX<T>(r)); }
#endif

private:
    T m_x = T();
    T m_y = T();
    T m_w = T();
    T m_h = T();
};

using RectF = RectX<double>;
class Rect : public RectX<int>
{
public:
    inline Rect()
        :  RectX<int>() {}
#ifndef NO_QT_SUPPORT
    inline Rect(const QRect& r)
        : RectX<int>(r.x(), r.y(), r.width(), r.height()) {}
#endif
    inline Rect(int x, int y, int w, int h)
        : RectX<int>(x, y, w, h) {}

    inline RectF toRectF() const { return RectF(x(), y(), width(), height()); }
};

// ====================================
// Polygon
// ====================================
template<typename T>
class PolygonX : public std::vector<PointX<T> >
{
public:

    inline PolygonX<T>() = default;
    inline PolygonX<T>(const std::vector<PointX<T> >& v) : std::vector<PointX<T> >(v) {
    }
    inline PolygonX<T>(size_t size) : std::vector<PointX<T> >(size) {
    }

    inline PolygonX<T>& operator<<(const PointX<T>& p)
    {
        this->push_back(p);
        return *this;
    }

    inline void translate(const PointX<T>& offset)
    {
        if (offset.isNull()) {
            return;
        }
        PointX<T>* p = this->data();
        size_t i = this->size();
        while (i--) {
            *p += offset;
            ++p;
        }
    }

    inline PolygonX<T> translated(T x, T y) const { return translated(PointX<T>(x, y)); }
    inline PolygonX<T> translated(const PointX<T>& offset) const
    {
        PolygonX<T> copy(*this);
        copy.translate(offset);
        return copy;
    }

    RectX<T> boundingRect() const
    {
        const PointX<T>* pd = this->data();
        const PointX<T>* pe = pd + this->size();
        if (pd == pe) {
            return RectX<T>(0, 0, 0, 0);
        }
        T minx, maxx, miny, maxy;
        minx = maxx = pd->x();
        miny = maxy = pd->y();
        ++pd;
        while (pd != pe) {
            if (pd->x() < minx) {
                minx = pd->x();
            } else if (pd->x() > maxx) {
                maxx = pd->x();
            }
            if (pd->y() < miny) {
                miny = pd->y();
            } else if (pd->y() > maxy) {
                maxy = pd->y();
            }
            ++pd;
        }
        return RectX<T>(minx, miny, maxx - minx, maxy - miny);
    }
};

using PolygonF = PolygonX<double>;
using Polygon = PolygonX<int>;

// Implementation ==========================================
template<typename T>
RectX<T> RectX<T>::united(const RectX<T>& r) const
{
    if (isNull()) {
        return r;
    }

    if (r.isNull()) {
        return *this;
    }

    T left = m_x;
    T right = m_x;
    if (m_w < 0) {
        left += m_w;
    } else {
        right += m_w;
    }
    if (r.m_w < 0) {
        left = std::min(left, r.m_x + r.m_w);
        right = std::max(right, r.m_x);
    } else {
        left = std::min(left, r.m_x);
        right = std::max(right, r.m_x + r.m_w);
    }

    double top = m_y;
    double bottom = m_y;
    if (m_h < 0) {
        top += m_h;
    } else {
        bottom += m_h;
    }
    if (r.m_h < 0) {
        top = std::min(top, r.m_y + r.m_h);
        bottom = std::max(bottom, r.m_y);
    } else {
        top = std::min(top, r.m_y);
        bottom = std::max(bottom, r.m_y + r.m_h);
    }
    return RectX<T>(left, top, right - left, bottom - top);
}

template<typename T>
RectX<T> RectX<T>::intersected(const RectX<T>& r) const
{
    T l1 = m_x;
    T r1 = m_x;
    if (m_w < 0) {
        l1 += m_w;
    } else {
        r1 += m_w;
    }
    if (l1 == r1) {   // null rect
        return RectX<T>();
    }
    T l2 = r.m_x;
    T r2 = r.m_x;
    if (r.m_w < 0) {
        l2 += r.m_w;
    } else {
        r2 += r.m_w;
    }
    if (l2 == r2) {   // null rect
        return RectX<T>();
    }
    if (l1 >= r2 || l2 >= r1) {
        return RectX<T>();
    }
    T t1 = m_y;
    T b1 = m_y;
    if (m_h < 0) {
        t1 += m_h;
    } else {
        b1 += m_h;
    }
    if (t1 == b1) {   // null rect
        return RectX<T>();
    }
    T t2 = r.m_y;
    T b2 = r.m_y;
    if (r.m_h < 0) {
        t2 += r.m_h;
    } else {
        b2 += r.m_h;
    }
    if (t2 == b2) {   // null rect
        return RectX<T>();
    }
    if (t1 >= b2 || t2 >= b1) {
        return RectX<T>();
    }
    RectX<T> tmp;
    tmp.m_x = std::max(l1, l2);
    tmp.m_y = std::max(t1, t2);
    tmp.m_w = std::min(r1, r2) - tmp.m_x;
    tmp.m_h = std::min(b1, b2) - tmp.m_y;
    return tmp;
}

template<typename T>
bool RectX<T>::intersects(const RectX<T>& r) const
{
    T l1 = m_x;
    T r1 = m_x;
    if (m_w < 0) {
        l1 += m_w;
    } else {
        r1 += m_w;
    }
    if (isEqual(l1, r1)) { // null rect
        return false;
    }
    T l2 = r.m_x;
    T r2 = r.m_x;
    if (r.m_w < 0) {
        l2 += r.m_w;
    } else {
        r2 += r.m_w;
    }
    if (isEqual(l2, r2)) { // null rect
        return false;
    }
    if (l1 >= r2 || l2 >= r1) {
        return false;
    }
    T t1 = m_y;
    T b1 = m_y;
    if (m_h < 0) {
        t1 += m_h;
    } else {
        b1 += m_h;
    }
    if (isEqual(t1, b1)) { // null rect
        return false;
    }
    T t2 = r.m_y;
    T b2 = r.m_y;
    if (r.m_h < 0) {
        t2 += r.m_h;
    } else {
        b2 += r.m_h;
    }
    if (isEqual(t2, b2)) { // null rect
        return false;
    }
    if (t1 >= b2 || t2 >= b1) {
        return false;
    }
    return true;
}

template<typename T>
bool RectX<T>::contains(const PointX<T>& p) const
{
    T l = m_x;
    T r = m_x;
    if (m_w < 0) {
        l += m_w;
    } else {
        r += m_w;
    }
    if (isEqual(l, r)) { // null rect
        return false;
    }
    if (p.x() < l || p.x() > r) {
        return false;
    }
    T t = m_y;
    T b = m_y;
    if (m_h < 0) {
        t += m_h;
    } else {
        b += m_h;
    }
    if (isEqual(t, b)) { // null rect
        return false;
    }
    if (p.y() < t || p.y() > b) {
        return false;
    }
    return true;
}

template<typename T>
bool RectX<T>::contains(const RectX<T>& r) const
{
    T l1 = m_x;
    T r1 = m_x;
    if (m_w < 0) {
        l1 += m_w;
    } else {
        r1 += m_w;
    }
    if (isEqual(l1, r1)) { // null rect
        return false;
    }
    T l2 = r.m_x;
    T r2 = r.m_x;
    if (r.m_w < 0) {
        l2 += r.m_w;
    } else {
        r2 += r.m_w;
    }
    if (isEqual(l2, r2)) { // null rect
        return false;
    }
    if (l2 < l1 || r2 > r1) {
        return false;
    }
    T t1 = m_y;
    T b1 = m_y;
    if (m_h < 0) {
        t1 += m_h;
    } else {
        b1 += m_h;
    }
    if (isEqual(t1, b1)) { // null rect
        return false;
    }
    T t2 = r.m_y;
    T b2 = r.m_y;
    if (r.m_h < 0) {
        t2 += r.m_h;
    } else {
        b2 += r.m_h;
    }
    if (isEqual(t2, b2)) { // null rect
        return false;
    }
    if (t2 < t1 || b2 > b1) {
        return false;
    }
    return true;
}

template<typename T>
RectX<T> RectX<T>::normalized() const
{
    RectX<T> r = *this;
    if (r.m_w < 0) {
        r.m_x += r.m_w;
        r.m_w = -r.m_w;
    }
    if (r.m_h < 0) {
        r.m_y += r.m_h;
        r.m_h = -r.m_h;
    }
    return r;
}
}

#endif // MU_DRAW_GEOMETRY_H
