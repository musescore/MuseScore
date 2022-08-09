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
#include <limits>
#include <numeric>

#include "types/string.h"

#include "constants.h"

// everything contained in .h file for performance reasons

namespace mu::engraving {
class Fraction
{
    // ensure 64 bit to avoid overflows in comparisons
    int64_t m_numerator = 0;
    int64_t m_denominator = 1;

public:
    // no implicit conversion from int to Fraction:
    constexpr Fraction() = default;
    constexpr Fraction(int z, int n)
        : m_numerator{n < 0 ? -z : z}, m_denominator{n < 0 ? -n : n} {}

    int numerator() const { return static_cast<int>(m_numerator); }
    int denominator() const { return static_cast<int>(m_denominator); }

    /// Use this when you need to initialize a Fraction to an arbitrary high value
    static constexpr Fraction max() { return Fraction(std::numeric_limits<int>::max(), 1); }

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
        return Fraction(static_cast<int>(std::abs(m_numerator)), static_cast<int>(m_denominator));
    }

    Fraction inverse() const
    {
        return Fraction(static_cast<int>(m_denominator), static_cast<int>(m_numerator));
    }

    // reduction

    void reduce()
    {
        const int64_t g = std::gcd(m_numerator, m_denominator);
        if (g) {
            m_numerator /= g;
            m_denominator /= g;
        }
    }

    Fraction reduced() const
    {
        const int64_t g = std::gcd(m_numerator, m_denominator);
        if (g) {
            return Fraction(static_cast<int>(m_numerator / g), static_cast<int>(m_denominator / g));
        }
        return Fraction(m_numerator, m_denominator);
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
            const int64_t g = std::gcd(m_denominator, val.m_denominator);
            if (g) {
                const int64_t m1 = val.m_denominator / g;       // This saves one division over straight lcm
                m_numerator = m_numerator * m1 + val.m_numerator * (m_denominator / g);
                m_denominator = m1 * m_denominator;
            }
        }
        return *this;
    }

    Fraction& operator-=(const Fraction& val)
    {
        if (m_denominator == val.m_denominator) {
            m_numerator -= val.m_numerator;       // Common enough use case to be handled separately for efficiency
        } else {
            const int64_t g = std::gcd(m_denominator, val.m_denominator);
            if (g) {
                const int64_t m1 = val.m_denominator / g;       // This saves one division over straight lcm
                m_numerator = m_numerator * m1 - val.m_numerator * (m_denominator / g);
                m_denominator = m1 * m_denominator;
            }
        }
        return *this;
    }

    Fraction& operator*=(const Fraction& val)
    {
        m_numerator *= val.m_numerator;
        m_denominator *= val.m_denominator;
        if (abs(val.m_denominator) > 1) {
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
    Fraction operator-() const { return Fraction(static_cast<int>(-m_numerator), static_cast<int>(m_denominator)); }
    Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
    Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }
    Fraction operator/(int v)             const { return Fraction(*this) /= v; }

    // conversion
    int ticks() const
    {
        if (m_numerator == -1 && m_denominator == 1) {                  // HACK
            return -1;
        }

        // Constants::division     - ticks per quarter note
        // Constants::division * 4 - ticks per whole note
        // result: rounded (Constants::division * 4 * m_numerator * 1.0 / m_denominator) value
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

    static Fraction fromFloat(float ticks)
    {
        return Fraction((int)(ticks * Constants::division * 4), Constants::division * 4).reduced();
    }

    static Fraction fromDouble(double ticks)
    {
        return Fraction((int)(ticks * Constants::division * 4), Constants::division * 4).reduced();
    }

    float toFloat()
    {
        return m_numerator / (float)m_denominator;
    }

    double toDouble()
    {
        return m_numerator / (double)m_denominator;
    }

    String toString() const { return String(u"%1/%2").arg(m_numerator, m_denominator); }
    static Fraction fromString(const String& str)
    {
        const size_t i = str.indexOf(u'/');
        return (i == mu::nidx) ? Fraction(str.toInt(), 1) : Fraction(str.left(i).toInt(), str.mid(i + 1).toInt());
    }

    constexpr double toDouble() const { return static_cast<double>(m_numerator) / static_cast<double>(m_denominator); }
};

inline Fraction operator*(const Fraction& f, int v) { return Fraction(f) *= v; }
inline Fraction operator*(int v, const Fraction& f) { return Fraction(f) *= v; }
}

#endif // MU_ENGRAVING_FRACTION_H
