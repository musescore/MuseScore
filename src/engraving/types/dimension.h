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

namespace mu::engraving {
//---------------------------------------------------------
//   Millimetre
//    - just millimetre
//---------------------------------------------------------
class Millimetre
{
public:
    constexpr Millimetre() = default;
    explicit constexpr Millimetre(double v)
        : m_val(v) {}

    constexpr double val() const { return m_val; }
    constexpr operator double() const {
        return m_val;
    }

    constexpr bool operator>(const Millimetre& a) const { return m_val > a.m_val; }
    constexpr bool operator<(const Millimetre& a) const { return m_val < a.m_val; }
    bool operator==(const Millimetre& a) const { return muse::RealIsEqual(m_val, a.m_val); }
    bool operator!=(const Millimetre& a) const { return m_val != a.m_val; }
    bool isZero() const { return muse::RealIsNull(m_val); }

    constexpr Millimetre& operator=(double v)
    {
        m_val = v;
        return *this;
    }

    constexpr Millimetre& operator+=(const Millimetre& a)
    {
        m_val += a.m_val;
        return *this;
    }

    constexpr Millimetre& operator+=(const double& a)
    {
        m_val += a;
        return *this;
    }

    constexpr Millimetre operator+(const Millimetre& a) const
    {
        Millimetre r(*this);
        r += a;
        return r;
    }

    constexpr Millimetre& operator-=(const Millimetre& a)
    {
        m_val -= a.m_val;
        return *this;
    }

    constexpr Millimetre& operator-=(const double& a)
    {
        m_val -= a;
        return *this;
    }

    constexpr Millimetre operator-(const Millimetre& a) const
    {
        Millimetre r(*this);
        r -= a;
        return r;
    }

    constexpr Millimetre& operator/=(double d)
    {
        m_val /= d;
        return *this;
    }

    constexpr Millimetre operator/(const Millimetre& b) const
    {
        return Millimetre(m_val / b.m_val);
    }

    constexpr Millimetre operator/(double b) const
    {
        return Millimetre(m_val / b);
    }

    constexpr Millimetre operator/(int b) const
    {
        return Millimetre(m_val / b);
    }

    constexpr Millimetre& operator*=(int d)
    {
        m_val *= d;
        return *this;
    }

    constexpr Millimetre& operator*=(double d)
    {
        m_val *= d;
        return *this;
    }

    constexpr Millimetre operator*(double b) const
    {
        Millimetre r(*this);
        r *= b;
        return r;
    }

    constexpr Millimetre operator-() const { return Millimetre(-m_val); }

private:
    double m_val = 0.0;
};

constexpr Millimetre operator""_mm(long double v) { return Millimetre(static_cast<double>(v)); }
constexpr Millimetre operator""_mm(unsigned long long v) { return Millimetre(static_cast<double>(v)); }

constexpr Millimetre operator*(double a, const Millimetre& b)
{
    Millimetre r(b);
    r *= a;
    return r;
}

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

    constexpr Millimetre toMM(double spval) const { return Millimetre(m_val * spval); }
    static constexpr Spatium fromMM(double mm, double spval) { return Spatium(mm / spval); }
    static constexpr Spatium fromMM(Millimetre mm, double spval) { return Spatium(mm.val() / spval); }

    constexpr bool operator>(const Spatium& a) const { return m_val > a.m_val; }
    constexpr bool operator<(const Spatium& a) const { return m_val < a.m_val; }
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
