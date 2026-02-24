/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include "realfn.h"
#include "draw/types/geometry.h"

namespace mu::engraving {
using PointF = muse::PointF;
//---------------------------------------------------------
//   Spatium
//    - a unit of measure
//    - the distance between two note lines
//    - used for many layout items
//---------------------------------------------------------
class Spatium
{
public:
    constexpr Spatium() = default;
    explicit constexpr Spatium(double v)
        : m_val(v) {}

    constexpr double val() const { return m_val; }

    constexpr double toAbsolute(double spval) const { return m_val * spval; }
    static constexpr Spatium fromAbsolute(double absolute, double spval) { return Spatium(absolute / spval); }

    constexpr bool operator>(const Spatium& a) const { return m_val > a.m_val; }
    constexpr bool operator<(const Spatium& a) const { return m_val < a.m_val; }
    constexpr bool operator>=(const Spatium& a) const { return m_val >= a.m_val; }
    constexpr bool operator<=(const Spatium& a) const { return m_val <= a.m_val; }
    bool operator==(const Spatium& a) const { return muse::RealIsEqual(m_val, a.m_val); }
    bool operator!=(const Spatium& a) const { return m_val != a.m_val; }
    bool isZero() const { return muse::RealIsNull(m_val); }

    constexpr Spatium& operator+=(const Spatium& a)
    {
        m_val += a.m_val;
        return *this;
    }

    constexpr Spatium operator+(const Spatium& a) const
    {
        Spatium r(*this);
        r += a;
        return r;
    }

    constexpr Spatium& operator-=(const Spatium& a)
    {
        m_val -= a.m_val;
        return *this;
    }

    constexpr Spatium operator-(const Spatium& a) const
    {
        Spatium r(*this);
        r -= a;
        return r;
    }

    constexpr Spatium& operator/=(double d)
    {
        m_val /= d;
        return *this;
    }

    constexpr double operator/(const Spatium& b) const
    {
        return m_val / b.m_val;
    }

    constexpr Spatium operator/(double b) const
    {
        Spatium r(*this);
        r /= b;
        return r;
    }

    constexpr Spatium& operator*=(int d)
    {
        m_val *= d;
        return *this;
    }

    constexpr Spatium& operator*=(double d)
    {
        m_val *= d;
        return *this;
    }

    constexpr Spatium operator*(int b) const
    {
        Spatium r(*this);
        r *= b;
        return r;
    }

    constexpr Spatium operator*(double b) const
    {
        Spatium r(*this);
        r *= b;
        return r;
    }

    constexpr Spatium operator-() const { return Spatium(-m_val); }

private:
    double m_val = 0.0;
};

//---------------------------------------------------------
//   PointSp
//    - a point in spatium units
//---------------------------------------------------------
class PointSp
{
public:
    constexpr PointSp() = default;
    constexpr PointSp(Spatium x, Spatium y)
        : m_x(x), m_y(y) {}

    constexpr Spatium x() const { return m_x; }
    constexpr Spatium y() const { return m_y; }

    constexpr void setX(Spatium x) { m_x = x; }
    constexpr void setY(Spatium y) { m_y = y; }
    constexpr void setXY(Spatium x, Spatium y) { m_x = x; m_y = y; }

    bool operator==(const PointSp& p) const { return m_x == p.m_x && m_y == p.m_y; }
    bool operator!=(const PointSp& p) const { return !(*this == p); }

    constexpr PointSp operator-() const { return PointSp(-m_x, -m_y); }
    constexpr PointSp operator-(const PointSp& p) const { return PointSp(m_x - p.m_x, m_y - p.m_y); }
    constexpr PointSp operator+(const PointSp& p) const { return PointSp(m_x + p.m_x, m_y + p.m_y); }

    constexpr PointSp& operator+=(const PointSp& p) { m_x += p.m_x; m_y += p.m_y; return *this; }
    constexpr PointSp& operator-=(const PointSp& p) { m_x -= p.m_x; m_y -= p.m_y; return *this; }
    constexpr PointSp& operator*=(double c) { m_x *= c; m_y *= c; return *this; }
    constexpr PointSp& operator/=(double divisor) { m_x /= divisor; m_y /= divisor; return *this; }

    PointF toAbsolute(double spval) const
    {
        return PointF(m_x.toAbsolute(spval), m_y.toAbsolute(spval));
    }

    static PointSp fromAbsolute(const PointF& pf, double spval)
    {
        return PointSp(Spatium::fromAbsolute(pf.x(), spval), Spatium::fromAbsolute(pf.y(), spval));
    }

private:
    Spatium m_x;
    Spatium m_y;
};

inline PointSp operator*(const PointSp& p, double c) { return PointSp(p.x() * c, p.y() * c); }
inline PointSp operator*(double c, const PointSp& p) { return PointSp(p.x() * c, p.y() * c); }
inline PointSp operator/(const PointSp& p, double c) { return PointSp(p.x() / c, p.y() / c); }

constexpr Spatium operator""_sp(long double v) { return Spatium(static_cast<double>(v)); }
constexpr Spatium operator""_sp(unsigned long long v) { return Spatium(static_cast<double>(v)); }

constexpr Spatium operator*(int a, const Spatium& b)
{
    Spatium r(b);
    r *= a;
    return r;
}

constexpr Spatium operator*(double a, const Spatium& b)
{
    Spatium r(b);
    r *= a;
    return r;
}
}
