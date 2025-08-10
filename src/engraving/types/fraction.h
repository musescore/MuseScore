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

#include <cstdint>
#include <cmath>
#include <limits>
#include <numeric>

#include "global/types/string.h"

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

    constexpr int numerator() const { return static_cast<int>(m_numerator); }
    constexpr int denominator() const { return static_cast<int>(m_denominator); }

    /// Use this when you need to initialize a Fraction to an arbitrary high value
    static constexpr Fraction max() { return Fraction(std::numeric_limits<int>::max(), 1); }

    constexpr void setNumerator(int v) { m_numerator = v; }
    constexpr void setDenominator(int v)
    {
        if (v < 0) {
            m_numerator = -m_numerator;
            m_denominator = -v;
        } else {
            m_denominator = v;
        }
    }

    constexpr void set(int z, int n)
    {
        if (n < 0) {
            m_numerator = -z;
            m_denominator = -n;
        } else {
            m_numerator = z;
            m_denominator = n;
        }
    }

    constexpr bool isZero() const { return m_numerator == 0; }
    constexpr bool isNotZero() const { return m_numerator != 0; }
    constexpr bool negative() const { return m_numerator < 0; }

    constexpr bool isValid() const { return m_denominator != 0; }

    // check if two fractions are identical (numerator & denominator)
    // == operator checks for equal value:
    constexpr bool identical(const Fraction& v) const
    {
        return (m_numerator == v.m_numerator)
               && (m_denominator == v.m_denominator);
    }

    Fraction absValue() const
    {
        return Fraction(static_cast<int>(std::abs(m_numerator)), static_cast<int>(m_denominator));
    }

    constexpr Fraction inverse() const
    {
        return Fraction(static_cast<int>(m_denominator), static_cast<int>(m_numerator));
    }

    // reduction

    constexpr void reduce()
    {
        const int64_t g = std::gcd(m_numerator, m_denominator);
        if (g) {
            m_numerator /= g;
            m_denominator /= g;
        }
    }

    constexpr Fraction reduced() const
    {
        const int64_t g = std::gcd(m_numerator, m_denominator);
        if (g) {
            return Fraction(static_cast<int>(m_numerator / g), static_cast<int>(m_denominator / g));
        }
        return Fraction(m_numerator, m_denominator);
    }

    // comparison

    constexpr bool operator<(const Fraction& val) const
    {
        return m_numerator * val.m_denominator < val.m_numerator * m_denominator;
    }

    constexpr bool operator<=(const Fraction& val) const
    {
        return m_numerator * val.m_denominator <= val.m_numerator * m_denominator;
    }

    constexpr bool operator>=(const Fraction& val) const
    {
        return m_numerator * val.m_denominator >= val.m_numerator * m_denominator;
    }

    constexpr bool operator>(const Fraction& val) const
    {
        return m_numerator * val.m_denominator > val.m_numerator * m_denominator;
    }

    constexpr bool operator==(const Fraction& val) const
    {
        return m_numerator * val.m_denominator == val.m_numerator * m_denominator;
    }

    constexpr bool operator!=(const Fraction& val) const
    {
        return m_numerator * val.m_denominator != val.m_numerator * m_denominator;
    }

    // arithmetic

    constexpr Fraction& operator+=(const Fraction& val)
    {
        if (m_denominator == val.m_denominator) {
            // Common enough use case to be handled separately for efficiency
            m_numerator += val.m_numerator;
        } else {
            const int64_t g = std::gcd(m_denominator, val.m_denominator);
            if (g) {
                const int64_t m1 = val.m_denominator / g; // This saves one division over straight lcm
                m_numerator = m_numerator * m1 + val.m_numerator * (m_denominator / g);
                m_denominator = m1 * m_denominator;
            }
        }
        return *this;
    }

    constexpr Fraction& operator-=(const Fraction& val)
    {
        if (m_denominator == val.m_denominator) {
            // Common enough use case to be handled separately for efficiency
            m_numerator -= val.m_numerator;
        } else {
            const int64_t g = std::gcd(m_denominator, val.m_denominator);
            if (g) {
                const int64_t m1 = val.m_denominator / g; // This saves one division over straight lcm
                m_numerator = m_numerator * m1 - val.m_numerator * (m_denominator / g);
                m_denominator = m1 * m_denominator;
            }
        }
        return *this;
    }

    constexpr Fraction& operator*=(const Fraction& val)
    {
        m_numerator *= val.m_numerator;
        m_denominator *= val.m_denominator;
        if (val.m_denominator > 1 || val.m_denominator < -1) {
            // We should be free to fully reduce here
            reduce();
        }
        return *this;
    }

    constexpr Fraction& operator*=(int val)
    {
        m_numerator *= val;
        return *this;
    }

    constexpr Fraction& operator/=(const Fraction& val)
    {
        const int sign = (val.m_numerator >= 0 ? 1 : -1);
        m_numerator   *= (sign * val.m_denominator);
        m_denominator *= (sign * val.m_numerator);
        if (val.m_numerator != sign) {
            reduce();
        }
        return *this;
    }

    constexpr Fraction& operator/=(int val)
    {
        m_denominator *= val;
        if (m_denominator < 0) {
            m_denominator = -m_denominator;
            m_numerator = -m_numerator;
        }
        reduce();
        return *this;
    }

    constexpr Fraction operator+(const Fraction& v) const { return Fraction(*this) += v; }
    constexpr Fraction operator-(const Fraction& v) const { return Fraction(*this) -= v; }
    constexpr Fraction operator-() const { return Fraction(static_cast<int>(-m_numerator), static_cast<int>(m_denominator)); }
    constexpr Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
    constexpr Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }
    constexpr Fraction operator/(int v)             const { return Fraction(*this) /= v; }

    // conversion
    constexpr int ticks() const
    {
        if (m_numerator == -1 && m_denominator == 1) { // HACK
            return -1;
        }

        // Constants::division     - ticks per quarter note
        // Constants::division * 4 - ticks per whole note
        // result: rounded (Constants::division * 4 * m_numerator * 1.0 / m_denominator) value
        const int sgn = (m_numerator < 0) ? -1 : 1;
        const auto result = sgn * (static_cast<int_least64_t>(sgn * m_numerator) * Constants::DIVISION * 4 + (m_denominator / 2))
                            / m_denominator;
        return static_cast<int>(result);
    }

    static constexpr Fraction fromTicks(int ticks)
    {
        if (ticks == -1) {
            return Fraction(-1, 1); // HACK
        }
        return Fraction(ticks, Constants::DIVISION * 4).reduced();
    }

    // A very small fraction, corresponds to 1 MIDI tick
    static constexpr Fraction eps() { return Fraction(1, Constants::DIVISION * 4); }

    muse::String toString() const { return muse::String(u"%1/%2").arg(m_numerator, m_denominator); }
    static Fraction fromString(const muse::String& str, bool* ok = nullptr)
    {
        const size_t i = str.indexOf(u'/');
        if (i == muse::nidx) {
            return Fraction(str.toInt(ok), 1);
        }

        int numerator = str.left(i).toInt(ok);
        if (ok && !*ok) {
            return Fraction();
        }

        int denominator = str.mid(i + 1).toInt(ok);
        if (ok && !*ok) {
            return Fraction();
        }

        return Fraction(numerator, denominator);
    }

    constexpr double toDouble() const { return static_cast<double>(m_numerator) / static_cast<double>(m_denominator); }
};

constexpr Fraction operator*(const Fraction& f, int v) { return Fraction(f) *= v; }
constexpr Fraction operator*(int v, const Fraction& f) { return Fraction(f) *= v; }
}
