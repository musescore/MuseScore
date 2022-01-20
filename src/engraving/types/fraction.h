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

#ifndef MU_ENGRAVING_FRACTION_H
#define MU_ENGRAVING_FRACTION_H

#include <cstdint>

#include <QString>

#include "constants.h"

// everything contained in .h file for performance reasons

namespace mu::engraving {
//---------------------------------------------------------
//   gcd
//    greatest common divisor. always returns a positive val
//    however, since int / uint = uint by C++ rules,
//    return int to avoid accidental implicit unsigned cast
//---------------------------------------------------------

inline int_least64_t gcd(int_least64_t a, int_least64_t b)
{
    int bp;
    if (b > a) {
        bp = b;
        b = a;
        a = bp;
    }                                       // Saves one % if true
    while (b != 0) {
        bp = b;
        b = a % b;
        a = bp;
    }

    return a >= 0 ? a : -a;
}

class Fraction
{
    // ensure 64 bit to avoid overflows in comparisons
    int_least64_t m_numerator = 0;
    int_least64_t m_denominator = 1;

public:
    // no implicit conversion from int to Fraction:
    constexpr Fraction() = default;
    constexpr Fraction(int z, int n)
        : m_numerator{n < 0 ? -z : z}, m_denominator{n < 0 ? -n : n} {}

    int numerator() const { return m_numerator; }
    int denominator() const { return m_denominator; }

    static constexpr Fraction max() { return Fraction(10000, 1); }
    // Use this when you need to initialize a Fraction to an arbitrary high value

    void setNumerator(int v) { m_numerator = v; }
    void setDenominator(int v)
    {
        if (v < 0) {
            m_numerator = -m_numerator;
            m_denominator = -v;
        } else {
            m_denominator = v;
        }
    }

    void set(int z, int n)
    {
        if (n < 0) {
            m_numerator = -z;
            m_denominator = -n;
        } else {
            m_numerator = z;
            m_denominator = n;
        }
    }

    bool isZero() const { return m_numerator == 0; }
    bool isNotZero() const { return m_numerator != 0; }
    bool negative() const { return m_numerator < 0; }

    bool isValid() const { return m_denominator != 0; }

    // check if two fractions are identical (numerator & denominator)
    // == operator checks for equal value:
    bool identical(const Fraction& v) const
    {
        return (m_numerator == v.m_numerator)
               && (m_denominator == v.m_denominator);
    }

    Fraction absValue() const
    {
        return Fraction(qAbs(m_numerator), m_denominator);
    }

    Fraction inverse() const
    {
        return Fraction(m_denominator, m_numerator);
    }

    // reduction

    void reduce()
    {
        const int g = gcd(m_numerator, m_denominator);
        m_numerator /= g;
        m_denominator /= g;
    }

    Fraction reduced() const
    {
        const int g = gcd(m_numerator, m_denominator);
        return Fraction(m_numerator / g, m_denominator / g);
    }

    // comparison

    bool operator<(const Fraction& val) const
    {
        return m_numerator * val.m_denominator < val.m_numerator * m_denominator;
    }

    bool operator<=(const Fraction& val) const
    {
        return m_numerator * val.m_denominator <= val.m_numerator * m_denominator;
    }

    bool operator>=(const Fraction& val) const
    {
        return m_numerator * val.m_denominator >= val.m_numerator * m_denominator;
    }

    bool operator>(const Fraction& val) const
    {
        return m_numerator * val.m_denominator > val.m_numerator * m_denominator;
    }

    bool operator==(const Fraction& val) const
    {
        return m_numerator * val.m_denominator == val.m_numerator * m_denominator;
    }

    bool operator!=(const Fraction& val) const
    {
        return m_numerator * val.m_denominator != val.m_numerator * m_denominator;
    }

    // arithmetic

    Fraction& operator+=(const Fraction& val)
    {
        if (m_denominator == val.m_denominator) {
            m_numerator += val.m_numerator;        // Common enough use case to be handled separately for efficiency
        } else {
            const int g = gcd(m_denominator, val.m_denominator);
            const int m1 = val.m_denominator / g;       // This saves one division over straight lcm
            m_numerator = m_numerator * m1 + val.m_numerator * (m_denominator / g);
            m_denominator = m1 * m_denominator;
        }
        return *this;
    }

    Fraction& operator-=(const Fraction& val)
    {
        if (m_denominator == val.m_denominator) {
            m_numerator -= val.m_numerator;       // Common enough use case to be handled separately for efficiency
        } else {
            const int g = gcd(m_denominator, val.m_denominator);
            const int m1 = val.m_denominator / g;       // This saves one division over straight lcm
            m_numerator = m_numerator * m1 - val.m_numerator * (m_denominator / g);
            m_denominator = m1 * m_denominator;
        }
        return *this;
    }

    Fraction& operator*=(const Fraction& val)
    {
        m_numerator *= val.m_numerator;
        m_denominator *= val.m_denominator;
        if (val.m_denominator != 1) {
            reduce();                            // We should be free to fully reduce here
        }
        return *this;
    }

    Fraction& operator*=(int val)
    {
        m_numerator *= val;
        return *this;
    }

    Fraction& operator/=(const Fraction& val)
    {
        const int sign = (val.m_numerator >= 0 ? 1 : -1);
        m_numerator   *= (sign * val.m_denominator);
        m_denominator *= (sign * val.m_numerator);
        if (val.m_numerator != sign) {
            reduce();
        }
        return *this;
    }

    Fraction& operator/=(int val)
    {
        m_denominator *= val;
        if (m_denominator < 0) {
            m_denominator = -m_denominator;
            m_numerator = -m_numerator;
        }
        reduce();
        return *this;
    }

    Fraction operator+(const Fraction& v) const { return Fraction(*this) += v; }
    Fraction operator-(const Fraction& v) const { return Fraction(*this) -= v; }
    Fraction operator-() const { return Fraction(-m_numerator, m_denominator); }
    Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
    Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }
    Fraction operator/(int v)             const { return Fraction(*this) /= v; }

    // convertion
    int ticks() const
    {
        if (m_numerator == -1 && m_denominator == 1) {                  // HACK
            return -1;
        }

        // Constant::division     - ticks per quarter note
        // Constant::division * 4 - ticks per whole note
        // result: rounded (Constant::division * 4 * m_numerator * 1.0 / m_denominator) value
        const int sgn = (m_numerator < 0) ? -1 : 1;
        const auto result = sgn * (static_cast<int_least64_t>(sgn * m_numerator) * Constants::division * 4 + (m_denominator / 2))
                            / m_denominator;
        return static_cast<int>(result);
    }

    static Fraction fromTicks(int ticks)
    {
        if (ticks == -1) {
            return Fraction(-1, 1);        // HACK
        }
        return Fraction(ticks, Constants::division * 4).reduced();
    }

    // A very small fraction, corresponds to 1 MIDI tick
    static Fraction eps() { return Fraction(1, Constants::division * 4); }

    QString toString() const { return QString("%1/%2").arg(m_numerator).arg(m_denominator); }
    static Fraction fromString(const QString& str)
    {
        const int i = str.indexOf('/');
        return (i == -1) ? Fraction(str.toInt(), 1) : Fraction(str.leftRef(i).toInt(), str.midRef(i + 1).toInt());
    }
};

inline Fraction operator*(const Fraction& f, int v) { return Fraction(f) *= v; }
inline Fraction operator*(int v, const Fraction& f) { return Fraction(f) *= v; }
}

//! NOTE compat
namespace Ms {
using Fraction = mu::engraving::Fraction;
}

#endif // MU_ENGRAVING_FRACTION_H
