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
#include "importmidi_fraction.h"
#include "engraving/dom/mscore.h"

#include <limits>
#include <QtGlobal>

namespace mu::iex::midi {
#ifdef QT_DEBUG

//---------------------------------------------------------------------------------------
// https://www.securecoding.cert.org/confluence/display/seccode/
// INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow?showComments=false
//
// Access date: 2013.11.28

bool isAdditionOverflow(int a, int b)           // a + b
{
    return (b > 0 && a > (std::numeric_limits<int>::max() - b))
           || (b < 0 && a < std::numeric_limits<int>::min() - b);
}

bool isSubtractionOverflow(int a, int b)        // a - b
{
    return (b > 0 && a < std::numeric_limits<int>::min() + b)
           || (b < 0 && a > std::numeric_limits<int>::max() + b);
}

bool isMultiplicationOverflow(int a, int b)     // a * b
{
    if (a > 0) {
        if (b > 0) {
            if (a > std::numeric_limits<int>::max() / b) {
                return true;
            }
        } else {
            if (b < std::numeric_limits<int>::min() / a) {
                return true;
            }
        }
    } else {
        if (b > 0) {
            if (a < std::numeric_limits<int>::min() / b) {
                return true;
            }
        } else {
            if (a != 0 && b < std::numeric_limits<int>::max() / a) {
                return true;
            }
        }
    }

    return false;
}

bool isDivisionOverflow(int a, int b)           // a / b
{
    return (b == 0) || ((a == std::numeric_limits<int>::min()) && (b == -1));
}

bool isRemainderOverflow(int a, int b)          // a % b
{
    return (b == 0) || ((a == std::numeric_limits<int>::min()) && (b == -1));
}

bool isUnaryNegationOverflow(int a)             // -a
{
    return a == std::numeric_limits<int>::min();
}

#endif

namespace {
//---------------------------------------------------------
//   lcm
//    least common multiple. always returns a positive val
//---------------------------------------------------------

static unsigned lcm(int a, int b)
{
    const int g =  std::gcd(a, b);

#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(a, g),
               "ReducedFraction, lcm", "Division overflow");
    Q_ASSERT_X(!isMultiplicationOverflow(a / g, b),
               "ReducedFraction, lcm", "Multiplication overflow");
#endif

    const int l = (a / g) * b;   // Divide first to minimize overflow risk
    return l >= 0 ? l : -l;
}
}

//-----------------------------------------------------------------------------

ReducedFraction::ReducedFraction()
    : numerator_(0)
    , denominator_(1)
{
}

ReducedFraction::ReducedFraction(int z, int n)
    : numerator_(z)
    , denominator_(n)
{
}

ReducedFraction::ReducedFraction(const engraving::Fraction& fraction)
    : numerator_(fraction.numerator())
    , denominator_(fraction.denominator())
{
}

ReducedFraction ReducedFraction::fromTicks(int ticks)
{
    return ReducedFraction(ticks, engraving::Constants::DIVISION * 4).reduced();
}

ReducedFraction ReducedFraction::reduced() const
{
    const int tmp = std::gcd(numerator_, denominator_);

#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(numerator_, tmp),
               "ReducedFraction::reduced", "Division overflow");
    Q_ASSERT_X(!isDivisionOverflow(denominator_, tmp),
               "ReducedFraction::reduced", "Division overflow");
#endif

    return ReducedFraction(numerator_ / tmp, denominator_ / tmp);
}

int ReducedFraction::ticks() const
{
    int integral = numerator_ / denominator_;
    int newNumerator = numerator_ % denominator_;
    int division = engraving::Constants::DIVISION * 4;

#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(newNumerator, division),
               "ReducedFraction::ticks", "Multiplication overflow");
    Q_ASSERT_X(!isAdditionOverflow(newNumerator * division, denominator_ / 2),
               "ReducedFraction::ticks", "Addition overflow");
#endif

    const int tmp = newNumerator * division + denominator_ / 2;

#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(tmp, denominator_),
               "ReducedFraction::ticks", "Division overflow");
    Q_ASSERT_X(!isMultiplicationOverflow(integral, division),
               "ReducedFraction::ticks", "Multiplication overflow");
    Q_ASSERT_X(!isAdditionOverflow(tmp / denominator_, integral * division),
               "ReducedFraction::ticks", "Addition overflow");
#endif

    return tmp / denominator_ + integral * division;
}

void ReducedFraction::reduce()
{
    if (numerator_ == 0) {
        denominator_ = 1;
        return;
    }
    const int tmp = std::gcd(numerator_, denominator_);

#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(numerator_, tmp),
               "ReducedFraction::reduce", "Division overflow");
    Q_ASSERT_X(!isDivisionOverflow(denominator_, tmp),
               "ReducedFraction::reduce", "Division overflow");
#endif

    numerator_ /= tmp;
    denominator_ /= tmp;
}

void ReducedFraction::preventOverflow()
{
    static const int reduceLimit = 10000;
    if (numerator_ >= reduceLimit || denominator_ >= reduceLimit) {
        reduce();
    }
}

// helper function

int fractionPart(int lcmPart, int numerator, int denominator)
{
#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(lcmPart, denominator),
               "ReducedFraction::fractionPart", "Division overflow");
#endif
    const int part = lcmPart / denominator;

#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(numerator, part),
               "ReducedFraction::fractionPart", "Multiplication overflow");
#endif
    return numerator * part;
}

ReducedFraction& ReducedFraction::operator+=(const ReducedFraction& val)
{
    preventOverflow();
    ReducedFraction value = val;
    value.preventOverflow();

    const int tmp = lcm(denominator_, val.denominator_);
    numerator_ = fractionPart(tmp, numerator_, denominator_)
                 + fractionPart(tmp, val.numerator_, val.denominator_);
    denominator_ = tmp;
    return *this;
}

ReducedFraction& ReducedFraction::operator-=(const ReducedFraction& val)
{
    preventOverflow();
    ReducedFraction value = val;
    value.preventOverflow();

    const int tmp = lcm(denominator_, val.denominator_);
    numerator_ = fractionPart(tmp, numerator_, denominator_)
                 - fractionPart(tmp, val.numerator_, val.denominator_);
    denominator_ = tmp;
    return *this;
}

ReducedFraction& ReducedFraction::operator*=(const ReducedFraction& val)
{
    preventOverflow();
    ReducedFraction value = val;
    value.preventOverflow();

#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(numerator_, val.numerator_),
               "ReducedFraction::operator*=", "Multiplication overflow");
    Q_ASSERT_X(!isMultiplicationOverflow(denominator_, val.denominator_),
               "ReducedFraction::operator*=", "Multiplication overflow");
#endif
    numerator_ *= val.numerator_;
    denominator_ *= val.denominator_;
    return *this;
}

ReducedFraction& ReducedFraction::operator*=(int val)
{
    preventOverflow();
#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(numerator_, val),
               "ReducedFraction::operator*=", "Multiplication overflow");
#endif
    numerator_ *= val;
    return *this;
}

ReducedFraction& ReducedFraction::operator/=(const ReducedFraction& val)
{
    preventOverflow();
    ReducedFraction value = val;
    value.preventOverflow();
#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(numerator_, val.denominator_),
               "ReducedFraction::operator/=", "Multiplication overflow");
    Q_ASSERT_X(!isMultiplicationOverflow(denominator_, val.numerator_),
               "ReducedFraction::operator/=", "Multiplication overflow");
#endif
    numerator_ *= val.denominator_;
    denominator_ *= val.numerator_;
    return *this;
}

ReducedFraction& ReducedFraction::operator/=(int val)
{
    preventOverflow();
#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(denominator_, val),
               "ReducedFraction::operator/=", "Multiplication overflow");
#endif
    denominator_ *= val;
    return *this;
}

bool ReducedFraction::operator<(const ReducedFraction& val) const
{
    const int v = lcm(denominator_, val.denominator_);
    return fractionPart(v, numerator_, denominator_)
           < fractionPart(v, val.numerator_, val.denominator_);
}

bool ReducedFraction::operator<=(const ReducedFraction& val) const
{
    const int v = lcm(denominator_, val.denominator_);
    return fractionPart(v, numerator_, denominator_)
           <= fractionPart(v, val.numerator_, val.denominator_);
}

bool ReducedFraction::operator>(const ReducedFraction& val) const
{
    const int v = lcm(denominator_, val.denominator_);
    return fractionPart(v, numerator_, denominator_)
           > fractionPart(v, val.numerator_, val.denominator_);
}

bool ReducedFraction::operator>=(const ReducedFraction& val) const
{
    const int v = lcm(denominator_, val.denominator_);
    return fractionPart(v, numerator_, denominator_)
           >= fractionPart(v, val.numerator_, val.denominator_);
}

bool ReducedFraction::operator==(const ReducedFraction& val) const
{
    const int v = lcm(denominator_, val.denominator_);
    return fractionPart(v, numerator_, denominator_)
           == fractionPart(v, val.numerator_, val.denominator_);
}

bool ReducedFraction::operator!=(const ReducedFraction& val) const
{
    const int v = lcm(denominator_, val.denominator_);
    return fractionPart(v, numerator_, denominator_)
           != fractionPart(v, val.numerator_, val.denominator_);
}

//-------------------------------------------------------------------------

ReducedFraction toMuseScoreTicks(int tick, int oldDivision, bool isDivisionInTps)
{
    if (isDivisionInTps) {
        return ReducedFraction::fromTicks(tick);
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(tick, oldDivision),
               "ReducedFraction::toMuseScoreTicks", "Division overflow");
    Q_ASSERT_X(!isRemainderOverflow(tick, oldDivision),
               "ReducedFraction::toMuseScoreTicks", "Remainder overflow");
#endif
    const int integral = tick / oldDivision;
    const int remainder = tick % oldDivision;
#ifdef QT_DEBUG
    Q_ASSERT_X(!isMultiplicationOverflow(remainder, engraving::Constants::DIVISION),
               "ReducedFraction::toMuseScoreTicks", "Multiplication overflow");
    Q_ASSERT_X(!isAdditionOverflow(remainder * engraving::Constants::DIVISION, oldDivision / 2),
               "ReducedFraction::toMuseScoreTicks", "Addition overflow");
#endif
    const int tmp = remainder * engraving::Constants::DIVISION + oldDivision / 2;
#ifdef QT_DEBUG
    Q_ASSERT_X(!isDivisionOverflow(tmp, oldDivision),
               "ReducedFraction::toMuseScoreTicks", "Division overflow");
    Q_ASSERT_X(!isMultiplicationOverflow(integral, engraving::Constants::DIVISION),
               "ReducedFraction::toMuseScoreTicks", "Multiplication overflow");
    Q_ASSERT_X(!isAdditionOverflow(tmp / oldDivision, integral * engraving::Constants::DIVISION),
               "ReducedFraction::toMuseScoreTicks", "Addition overflow");
#endif
    return ReducedFraction::fromTicks(tmp / oldDivision + integral * engraving::Constants::DIVISION);
}
} // namespace mu::iex::midi
