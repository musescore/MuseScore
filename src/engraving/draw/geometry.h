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
#include <QtGlobal>

#ifndef NO_QT_SUPPORT
#include <QPointF>
#include <QLineF>
#include <QPolygonF>
#include <QRectF>
#endif

namespace mu::draw {
inline bool isEqual(qreal a1, qreal a2) { return qFuzzyCompare(a1, a2); }
inline bool isEqual(int a1, int a2) { return a1 == a2; }

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

    inline bool operator==(const PointX<T>& p) const { return isEqual(p.m_x, m_x) && isEqual(p.m_y, m_y); }
    inline bool operator!=(const PointX<T>& p) const { return !this->operator ==(p); }

    inline PointX<T> operator-() const { return PointX<T>(-m_x, -m_y); }
    inline PointX<T> operator-(const PointX<T>& p) const { return PointX<T>(m_x - p.m_x, m_y - p.m_y); }

    inline PointX<T>& operator+=(const PointX<T>& p) { m_x+=p.m_x; m_y+=p.m_y; return *this; }

    static inline qreal dotProduct(const PointX<T>& p1, const PointX<T>& p2) { return p1.m_x * p2.m_x + p1.m_y * p2.m_y; }

#ifndef NO_QT_SUPPORT
    inline PointX(const QPointF& p)
        : m_x(p.x()), m_y(p.y()) {}

    inline QPointF toQPointF() const { return QPointF(m_x, m_y); }

    inline PointX<T>& operator=(const QPointF& p) { m_x = p.x(); m_y = p.y(); return *this; }
#endif

private:
    //! NOTE We should not swap fields
    //! We should not add new fields
    //! If we really need to do this, then we need to change the implementation of QPainterProvider
    T m_x = T();
    T m_y = T();
};

using PointF = PointX<qreal>;
using Point = PointX<int>;

#ifndef NO_QT_SUPPORT
inline QPointF operator+(const QPointF& p1, const PointF& p2) { return QPointF(p1.x() + p2.x(), p1.y() + p2.y()); }
#endif

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

    inline void setP1(const PointX<T>& p) { m_p1 = p; }
    inline void setP2(const PointX<T>& p) { m_p2 = p; }
    inline void setLine(T aX1, T aY1, T aX2, T aY2) { m_p1 = PointX<T>(aX1, aY1); m_p2 = PointX<T>(aX2, aY2); }

    inline PointX<T> pointAt(T t) const
    {
        return PointX<T>(m_p1.x() + (m_p2.x() - m_p1.x()) * t, m_p1.y() + (m_p2.y() - m_p1.y()) * t);
    }

#ifndef NO_QT_SUPPORT
    inline LineX(const QLineF& l)
        : m_p1(l.p1()), m_p2(l.p2()) {}
#endif

private:
    PointX<T> m_p1;
    PointX<T> m_p2;
};

using LineF = LineX<qreal>;
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

#ifndef NO_QT_SUPPORT
    inline SizeX(const QSizeF& s)
        : m_w(s.width()), m_h(s.height()) {}
#endif

private:
    T m_w = T();
    T m_h = T();
};

using SizeF = SizeX<qreal>;
using Size = SizeX<int>;

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

    inline bool isNull() const { return isEqual(m_w, T()) && isEqual(m_h, T()); }

    inline T x() const { return m_x; }
    inline T y() const { return m_y; }
    inline T width() const { return m_w; }
    inline T height() const { return m_h; }

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

    inline RectX<T> translated(T dx, T dy) const { return RectX<T>(m_x + dx, m_y + dy, m_w, m_h); }
    inline RectX<T> translated(const PointX<T>& p) const { return RectX<T>(m_x + p.x(), m_y + p.y(), m_w, m_h); }

    inline RectX<T> adjusted(T xp1, T yp1, T xp2, T yp2) const { return RectX<T>(m_x + xp1, m_y + yp1, m_w + xp2 - xp1, m_h + yp2 - yp1); }

    RectX<T> united(const RectX<T>& r) const
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
            left = qMin(left, r.m_x + r.m_w);
            right = qMax(right, r.m_x);
        } else {
            left = qMin(left, r.m_x);
            right = qMax(right, r.m_x + r.m_w);
        }

        qreal top = m_y;
        qreal bottom = m_y;
        if (m_h < 0) {
            top += m_h;
        } else {
            bottom += m_h;
        }
        if (r.m_h < 0) {
            top = qMin(top, r.m_y + r.m_h);
            bottom = qMax(bottom, r.m_y);
        } else {
            top = qMin(top, r.m_y);
            bottom = qMax(bottom, r.m_y + r.m_h);
        }
        return RectX<T>(left, top, right - left, bottom - top);
    }

#ifndef NO_QT_SUPPORT
    inline RectX(const QRectF& r)
        : m_x(r.x()), m_y(r.y()), m_w(r.width()), m_h(r.height()) {}

    inline RectX(const QRect& r)
        : m_x(r.x()), m_y(r.y()), m_w(r.width()), m_h(r.height()) {}

    inline RectX<T> united(const QRectF& r) const { return united(RectX<T>(r)); }

    inline QRectF toQRectF() const { return QRectF(m_x, m_y, m_w, m_h); }
    inline QRect toQRect() const { return QRect(m_x, m_y, m_w, m_h); }
#endif

private:
    T m_x = T();
    T m_y = T();
    T m_w = T();
    T m_h = T();
};

using RectF = RectX<qreal>;
class Rect : public RectX<int>
{
public:
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
    inline PolygonX<T>(const PolygonX<T>& p) = default;
    inline PolygonX<T>(const std::vector<PointX<T> >& v)
        : std::vector<PointX<T> >(v) {}
    inline PolygonX<T>(size_t size)
        : std::vector<PointX<T> >(size) {}

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

using PolygonF = PolygonX<qreal>;
using Polygon = PolygonX<int>;
}

#endif // MU_DRAW_GEOMETRY_H
