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

#ifndef MUSE_DRAW_BEZIER_H
#define MUSE_DRAW_BEZIER_H

#include "geometry.h"

namespace muse::draw {
class Bezier
{
public:
    Bezier() = default;
    Bezier(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

    static Bezier fromPoints(const PointF& p1, const PointF& p2, const PointF& p3, const PointF& p4);
    static void coefficients(double t, double& a, double& b, double& c, double& d);
    Bezier bezierOnInterval(double t0, double t1) const;

    PointF pt1() const { return PointF(m_x1, m_y1); }
    PointF pt2() const { return PointF(m_x2, m_y2); }
    PointF pt3() const { return PointF(m_x3, m_y3); }
    PointF pt4() const { return PointF(m_x4, m_y4); }

    PointF pointAt(double t) const;

private:
    void parameterSplitLeft(double t, Bezier* left);

    friend class PainterPath;

    double m_x1, m_y1, m_x2, m_y2, m_x3, m_y3, m_x4, m_y4;
};
}

#endif // MUSE_DRAW_BEZIER_H
