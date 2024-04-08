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

#include "bezier.h"
#include "global/realfn.h"

using namespace muse;
using namespace muse::draw;

Bezier::Bezier(double x1, double y1,
               double x2, double y2,
               double x3, double y3,
               double x4, double y4)
    : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2),
    m_x3(x3), m_y3(y3), m_x4(x4), m_y4(y4)
{
}

Bezier Bezier::fromPoints(const PointF& p1, const PointF& p2,
                          const PointF& p3, const PointF& p4)
{
    return { p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y() };
}

void Bezier::coefficients(double t, double& a, double& b, double& c, double& d)
{
    double t1 = 1. - t;
    b = t1 * t1;
    c = t * t;
    d = c * t;
    a = b * t1;
    b *= 3. * t;
    c *= 3. * t1;
}

Bezier Bezier::bezierOnInterval(double t0, double t1) const
{
    if (RealIsNull(t0) && RealIsEqual(t1, 1)) {
        return *this;
    }
    Bezier bezier = *this;
    Bezier result;
    bezier.parameterSplitLeft(t0, &result);
    double trueT = (t1 - t0) / (1 - t0);
    bezier.parameterSplitLeft(trueT, &result);
    return result;
}

void Bezier::parameterSplitLeft(double t, Bezier* left)
{
    left->m_x1 = m_x1;
    left->m_y1 = m_y1;
    left->m_x2 = m_x1 + t * (m_x2 - m_x1);
    left->m_y2 = m_y1 + t * (m_y2 - m_y1);
    left->m_x3 = m_x2 + t * (m_x3 - m_x2);   // temporary holding spot
    left->m_y3 = m_y2 + t * (m_y3 - m_y2);   // temporary holding spot
    m_x3 = m_x3 + t * (m_x4 - m_x3);
    m_y3 = m_y3 + t * (m_y4 - m_y3);
    m_x2 = left->m_x3 + t * (m_x3 - left->m_x3);
    m_y2 = left->m_y3 + t * (m_y3 - left->m_y3);
    left->m_x3 = left->m_x2 + t * (left->m_x3 - left->m_x2);
    left->m_y3 = left->m_y2 + t * (left->m_y3 - left->m_y2);
    left->m_x4 = m_x1 = left->m_x3 + t * (m_x2 - left->m_x3);
    left->m_y4 = m_y1 = left->m_y3 + t * (m_y2 - left->m_y3);
}

PointF Bezier::pointAt(double t) const
{
    // numerically more stable:
    double x, y;
    double t1 = 1. - t;
    {
        double a = m_x1 * t1 + m_x2 * t;
        double b = m_x2 * t1 + m_x3 * t;
        double c = m_x3 * t1 + m_x4 * t;
        a = a * t1 + b * t;
        b = b * t1 + c * t;
        x = a * t1 + b * t;
    }
    {
        double a = m_y1 * t1 + m_y2 * t;
        double b = m_y2 * t1 + m_y3 * t;
        double c = m_y3 * t1 + m_y4 * t;
        a = a * t1 + b * t;
        b = b * t1 + c * t;
        y = a * t1 + b * t;
    }
    return PointF(x, y);
}
