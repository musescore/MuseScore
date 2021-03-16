//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_FRAMEWORK_REALFN_H
#define MU_FRAMEWORK_REALFN_H

#include <cmath>
#include <algorithm>

namespace mu {
//constexpr int COMPARE_DOUBLE_PREC(9);
//constexpr int COMPARE_FLOAT_PREC(6);

inline /*constexpr*/ int _pow10(int power)
{
    int result = 1;
    for (int i = 0; i < power; ++i) {
        result *= 10;
    }
    return result;
}

//inline constexpr double _pow_10(int power)
//{
//    double result = 1;
//    for (int i = 0; i < power; ++i) {
//        result /= 10;
//    }
//    return result;
//}

//constexpr double COMPARE_DOUBLE_EPSILON(_pow10(COMPARE_DOUBLE_PREC));
//constexpr double COMPARE_DOUBLE_NULL(_pow_10(COMPARE_DOUBLE_PREC));

//constexpr float COMPARE_FLOAT_EPSILON(_pow10(COMPARE_FLOAT_PREC));
//constexpr float COMPARE_FLOAT_NULL(static_cast<float>(_pow_10(COMPARE_DOUBLE_PREC)));

constexpr double COMPARE_DOUBLE_EPSILON(1000000000.0);
constexpr double COMPARE_DOUBLE_NULL(0.000000001);

constexpr float COMPARE_FLOAT_EPSILON(1000000.0);
constexpr float COMPARE_FLOAT_NULL(0.000001F);

inline bool RealIsEqual(double p1, double p2)
{
    return std::abs(p1 - p2) * COMPARE_DOUBLE_EPSILON <= std::min(std::abs(p1), std::abs(p2));
}

inline bool RealIsEqual(float p1, float p2)
{
    return std::fabs(p1 - p2) * COMPARE_FLOAT_EPSILON <= std::min(std::fabs(p1), std::fabs(p2));
}

inline bool RealIsEqualOrMore(double p1, double p2)
{
    return p1 > p2 || RealIsEqual(p1, p2);
}

inline bool RealIsEqualOrLess(double p1, double p2)
{
    return p1 < p2 || RealIsEqual(p1, p2);
}

inline bool RealIsEqualOrMore(float p1, float p2)
{
    return p1 > p2 || RealIsEqual(p1, p2);
}

inline bool RealIsEqualOrLess(float p1, float p2)
{
    return p1 < p2 || RealIsEqual(p1, p2);
}

inline bool RealIsNull(double d)
{
    return std::abs(d) <= COMPARE_DOUBLE_NULL;
}

inline bool RealIsNull(float d)
{
    return std::fabs(d) <= COMPARE_FLOAT_NULL;
}

inline double RealRound(double value, int prec)
{
    int round = _pow10(prec);
    return std::floor(value * round + 0.5) / round;
}

inline double RealRound(float value, int prec)
{
    return RealRound(static_cast<double>(value), prec);
}

inline double RealFloor(double value, int prec)
{
    int round = _pow10(prec);
    return std::floor(value * round) / round;
}

inline double RealFloor(float value, int prec)
{
    return RealRound(static_cast<double>(value), prec);
}
}

#endif // MU_FRAMEWORK_REALFN_H
