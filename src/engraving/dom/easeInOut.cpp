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

/**
 \file
 Implementation of class EaseInOut for implementing transfer curve with
 parametrerable ease-In and ease-Out.
*/

#include "easeInOut.h"

#include <cmath>

#include "realfn.h"

using namespace mu;
using namespace muse;

namespace mu::engraving {
//-------------------------------------------------------------------------------------------------
// The following function is inspired by "A Primer on Bézier Curve" sections 17 and 23 by Pomax:
// https://pomax.github.io/bezierinfo/
// However, the function is greatly specialized, simplified and optimized for use as an ease-in and
// ease-out transfer curve for bends, glissandi and portamenti in MuseScore. The function computes
// Y from X by performing a cubic root finding to compute t from X on the X component of the
// transfer curve and then a cubic Bezier evaluation to compute Y from t on the Y component of the
// transfer curve. The code could be simplified because the Bezier curve is constrained as a well
// defined transfer function. It is not suitable for general or arbitrary Bezier curve work. -YP
//-------------------------------------------------------------------------------------------------
double EaseInOut::tFromX(const double x) const
{
    const double pi = 3.14159265358979323846;
    double w1 = m_easeIn - x;
    double w2 = 1.0 - m_easeOut - x;
    double d = x + 3.0 * w1 - 3.0 * w2 + (1.0 - x);
    double a = (-3.0 * x - 6.0 * w1 + 3 * w2) / d;
    double b = (3.0 * x + 3.0 * w1) / d;
    double c = -x / d;
    double a2 = a * a;
    double p = (3.0 * b - a2) / 3.0;
    double q = (2.0 * a2 * a - 9.0 * a * b + 27.0 * c) / 27.0;
    double discr = (q * q) / 4.0 + (p * p * p) / 27.0;
    double t = 0.0;
    // Crazy idea to first test the least probable case with such an expensive test but...
    if (muse::RealIsNull(discr)) {
        // Case that happens extremely rarely --> 2 roots.
        double q2 = q / 2.0;
        double u = q2 < 0.0 ? std::pow(-q2, 1.0 / 3.0) : -std::pow(q2, 1.0 / 3.0);
        // Find the only root that is within the 0 to 1 interval.
        t = 2.0 * u - a / 3.0;
        if (0.0 > t || t > 1.0) {
            t = -u - a / 3.0;
        }
    } else if (discr < 0.0) {
        // Case that happens about 75% of the time --> 3 roots.
        double mp3 = -p / 3.0;
        double r = std::sqrt(mp3 * mp3 * mp3);
        double phi = std::acos(std::min(std::max(-1.0, -q / (2.0 * r)), 1.0));
        double t1 = 2.0 * std::pow(r, 1.0 / 3.0);
        // Find the only root that is within the 0 to 1 interval.
        t = t1 * std::cos(phi / 3.0) - a / 3.0;
        if (0.0 > t || t > 1.0) {
            t = t1 * std::cos((phi + 2.0 * pi) / 3.0) - a / 3.0;
            if (0.0 > t || t > 1.0) {
                t = t1 * std::cos((phi + 4.0 * pi) / 3.0) - a / 3.0;
            }
        }
    } else if (discr > 0.0) {
        // Case that happens about 25% of the time --> 1 root.
        double q2 = q / 2.0;
        double sd = std::sqrt(discr);
        double u = std::pow(-q2 + sd, 1.0 / 3.0);
        double v = std::pow(q2 + sd, 1.0 / 3.0);
        t = u - v - a / 3.0;
    }
    return t;
}

//-------------------------------------------------------------------------------------------------
// This function is even more simplified and optimized because all the Bezier control points are
// constant. Thus, when computing the Y root there is only one case and the math simplifies to the
// following simple expression.
//-------------------------------------------------------------------------------------------------
double EaseInOut::tFromY(const double y) const
{
    return 0.5 + std::cos((4.0 * M_PI + std::acos(1.0 - 2.0 * y)) / 3.0);
}

//-------------------------------------------------------------------------------------------------
// Given a number of note to place within the given duration, return the list of on-times for each
// note given the current ease-in and ease-out parameters. The first note is at time 0 while the
//-------------------------------------------------------------------------------------------------
void EaseInOut::timeList(const int nbNotes, const int duration, std::vector<int>* times) const
{
    double nNotes = double(nbNotes);
    double space = double(duration);
    if (RealIsNull(m_easeIn) && RealIsNull(m_easeOut)) {
        for (int n = 0; n <= nbNotes; n++) {
            times->push_back(static_cast<int>(std::lround((static_cast<double>(n) / nNotes) * space)));
        }
    } else {
        for (int n = 0; n <= nbNotes; n++) {
            times->push_back(static_cast<int>(std::lround(XfromY(static_cast<double>(n) / nNotes) * space)));
        }
    }
}
}
