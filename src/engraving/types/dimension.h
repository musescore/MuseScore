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

#ifndef MU_ENGRAVING_DIMENSION_H
#define MU_ENGRAVING_DIMENSION_H

#include "realfn.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Millimetre
//    - just millimetre
//---------------------------------------------------------
class Millimetre
{
public:
    Millimetre() = default;
    explicit Millimetre(double v)
        : m_val(v) {}

    double val() const { return m_val; }
    operator double() const {
        return m_val;
    }

    bool operator>(const Millimetre& a) const { return m_val > a.m_val; }
    bool operator<(const Millimetre& a) const { return m_val < a.m_val; }
    bool operator==(const Millimetre& a) const { return muse::RealIsEqual(m_val, a.m_val); }
    bool operator!=(const Millimetre& a) const { return m_val != a.m_val; }
    bool isZero() const { return muse::RealIsNull(m_val); }

    Millimetre& operator=(double v)
    {
        m_val = v;
        return *this;
    }

    Millimetre& operator+=(const Millimetre& a)
    {
        m_val += a.m_val;
        return *this;
    }

    Millimetre& operator+=(const double& a)
    {
        m_val += a;
        return *this;
    }

    Millimetre operator+(const Millimetre& a) const
    {
        Millimetre r(*this);
        r += a;
        return r;
    }

    Millimetre& operator-=(const Millimetre& a)
    {
        m_val -= a.m_val;
        return *this;
    }

    Millimetre& operator-=(const double& a)
    {
        m_val -= a;
        return *this;
    }

    Millimetre operator-(const Millimetre& a) const
    {
        Millimetre r(*this);
        r -= a;
        return r;
    }

    Millimetre& operator/=(double d)
    {
        m_val /= d;
        return *this;
    }

    Millimetre operator/(const Millimetre& b) const
    {
        return Millimetre(m_val / b.m_val);
    }

    Millimetre operator/(double b) const
    {
        return Millimetre(m_val / b);
    }

    Millimetre operator/(int b) const
    {
        return Millimetre(m_val / b);
    }

    Millimetre& operator*=(int d)
    {
        m_val *= d;
        return *this;
    }

    Millimetre& operator*=(double d)
    {
        m_val *= d;
        return *this;
    }

    Millimetre operator*(double b) const
    {
        Millimetre r(*this);
        r *= b;
        return r;
    }

    Millimetre operator-() const { return Millimetre(-m_val); }

private:
    double m_val = 0.0;
};

inline Millimetre operator*(double a, const Millimetre& b)
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
    Spatium() = default;
    explicit Spatium(double v)
        : m_val(v) {}

    double val() const { return m_val; }

    Millimetre toMM(double spval) const { return Millimetre(m_val * spval); }
    static Spatium fromMM(double mm, double spval) { return Spatium(mm / spval); }
    static Spatium fromMM(Millimetre mm, double spval) { return Spatium(mm.val() / spval); }

    bool operator>(const Spatium& a) const { return m_val > a.m_val; }
    bool operator<(const Spatium& a) const { return m_val < a.m_val; }
    bool operator==(const Spatium& a) const { return muse::RealIsEqual(m_val, a.m_val); }
    bool operator!=(const Spatium& a) const { return m_val != a.m_val; }
    bool isZero() const { return muse::RealIsNull(m_val); }

    Spatium& operator+=(const Spatium& a)
    {
        m_val += a.m_val;
        return *this;
    }

    Spatium operator+(const Spatium& a) const
    {
        Spatium r(*this);
        r += a;
        return r;
    }

    Spatium& operator-=(const Spatium& a)
    {
        m_val -= a.m_val;
        return *this;
    }

    Spatium operator-(const Spatium& a) const
    {
        Spatium r(*this);
        r -= a;
        return r;
    }

    Spatium& operator/=(double d)
    {
        m_val /= d;
        return *this;
    }

    double operator/(const Spatium& b) const
    {
        return m_val / b.m_val;
    }

    Spatium operator/(double b) const
    {
        Spatium r(*this);
        r /= b;
        return r;
    }

    Spatium& operator*=(int d)
    {
        m_val *= d;
        return *this;
    }

    Spatium& operator*=(double d)
    {
        m_val *= d;
        return *this;
    }

    Spatium operator*(int b) const
    {
        Spatium r(*this);
        r *= b;
        return r;
    }

    Spatium operator*(double b) const
    {
        Spatium r(*this);
        r *= b;
        return r;
    }

    Spatium operator-() const { return Spatium(-m_val); }

private:
    double m_val = 0.0;
};

inline Spatium operator*(int a, const Spatium& b)
{
    Spatium r(b);
    r *= a;
    return r;
}

inline Spatium operator*(double a, const Spatium& b)
{
    Spatium r(b);
    r *= a;
    return r;
}
}

#endif //MU_ENGRAVING_DIMENSION_H
