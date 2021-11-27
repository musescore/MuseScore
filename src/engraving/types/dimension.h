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

#ifndef MU_ENGRAVING_DIMENSION_H
#define MU_ENGRAVING_DIMENSION_H

#include <QtGlobal>
#include "realfn.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Milimetre
//    - just milimetre
//---------------------------------------------------------
class Milimetre
{
public:
    Milimetre() = default;
    explicit Milimetre(qreal v)
        : m_val(v) {}

    qreal val() const { return m_val; }
    operator qreal() const {
        return m_val;
    }

    bool operator>(const Milimetre& a) const { return m_val > a.m_val; }
    bool operator<(const Milimetre& a) const { return m_val < a.m_val; }
    bool operator==(const Milimetre& a) const { return RealIsEqual(m_val, a.m_val); }
    bool operator!=(const Milimetre& a) const { return m_val != a.m_val; }
    bool isZero() const { return RealIsNull(m_val); }

    Milimetre& operator=(qreal v)
    {
        m_val = v;
        return *this;
    }

    Milimetre& operator+=(const Milimetre& a)
    {
        m_val += a.m_val;
        return *this;
    }

    Milimetre& operator+=(const qreal& a)
    {
        m_val += a;
        return *this;
    }

    Milimetre& operator-=(const Milimetre& a)
    {
        m_val -= a.m_val;
        return *this;
    }

    Milimetre& operator-=(const qreal& a)
    {
        m_val -= a;
        return *this;
    }

    Milimetre& operator/=(qreal d)
    {
        m_val /= d;
        return *this;
    }

    Milimetre operator/(const Milimetre& b)
    {
        return Milimetre(m_val / b.m_val);
    }

    Milimetre operator/(qreal b)
    {
        return Milimetre(m_val / b);
    }

    Milimetre operator/(int b)
    {
        return Milimetre(m_val / b);
    }

    Milimetre& operator*=(int d)
    {
        m_val *= d;
        return *this;
    }

    Milimetre& operator*=(qreal d)
    {
        m_val *= d;
        return *this;
    }

    Milimetre operator-() const { return Milimetre(-m_val); }

private:
    qreal m_val = 0.0;
};

inline Milimetre operator+(const Milimetre& a, const Milimetre& b)
{
    Milimetre r(a);
    r += b;
    return r;
}

inline Milimetre operator-(const Milimetre& a, const Milimetre& b)
{
    Milimetre r(a);
    r -= b;
    return r;
}

inline Milimetre operator/(const Milimetre& a, qreal b)
{
    Milimetre r(a);
    r /= b;
    return r;
}

inline Milimetre operator*(const Milimetre& a, qreal b)
{
    Milimetre r(a);
    r *= b;
    return r;
}

inline Milimetre operator*(qreal a, const Milimetre& b)
{
    Milimetre r(b);
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
    explicit Spatium(qreal v)
        : m_val(v) {}

    qreal val() const { return m_val; }

    Milimetre toMM(qreal spval) const { return Milimetre(m_val * spval); }
    static Spatium fromMM(qreal mm, qreal spval) { return Spatium(mm / spval); }
    static Spatium fromMM(Milimetre mm, qreal spval) { return Spatium(mm.val() / spval); }

    bool operator>(const Spatium& a) const { return m_val > a.m_val; }
    bool operator<(const Spatium& a) const { return m_val < a.m_val; }
    bool operator==(const Spatium& a) const { return RealIsEqual(m_val, a.m_val); }
    bool operator!=(const Spatium& a) const { return m_val != a.m_val; }
    bool isZero() const { return RealIsNull(m_val); }

    Spatium& operator+=(const Spatium& a)
    {
        m_val += a.m_val;
        return *this;
    }

    Spatium& operator-=(const Spatium& a)
    {
        m_val -= a.m_val;
        return *this;
    }

    Spatium& operator/=(qreal d)
    {
        m_val /= d;
        return *this;
    }

    qreal operator/(const Spatium& b)
    {
        return m_val / b.m_val;
    }

    Spatium& operator*=(int d)
    {
        m_val *= d;
        return *this;
    }

    Spatium& operator*=(qreal d)
    {
        m_val *= d;
        return *this;
    }

    Spatium operator-() const { return Spatium(-m_val); }

private:
    qreal m_val = 0.0;
};

inline Spatium operator+(const Spatium& a, const Spatium& b)
{
    Spatium r(a);
    r += b;
    return r;
}

inline Spatium operator-(const Spatium& a, const Spatium& b)
{
    Spatium r(a);
    r -= b;
    return r;
}

inline Spatium operator/(const Spatium& a, qreal b)
{
    Spatium r(a);
    r /= b;
    return r;
}

inline Spatium operator*(const Spatium& a, int b)
{
    Spatium r(a);
    r *= b;
    return r;
}

inline Spatium operator*(int a, const Spatium& b)
{
    Spatium r(b);
    r *= a;
    return r;
}

inline Spatium operator*(const Spatium& a, qreal b)
{
    Spatium r(a);
    r *= b;
    return r;
}

inline Spatium operator*(qreal a, const Spatium& b)
{
    Spatium r(b);
    r *= a;
    return r;
}
}

//! NOTE compat
namespace Ms {
using Spatium = mu::engraving::Spatium;
using Milimetre = mu::engraving::Milimetre;
}

#endif //MU_ENGRAVING_DIMENSION_H
