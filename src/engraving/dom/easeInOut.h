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

#ifndef MU_ENGRAVING_EASEINOUT_H
#define MU_ENGRAVING_EASEINOUT_H

#include <vector>

#include "draw/types/geometry.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ EaseInOut
///   \brief Specialized transfer curve using an underlying Bezier curve.
///
///   The second and third control points of the Bezier curve are moveable
///   along the x axis according to two parameters 'EaseIn' and 'EaseOut',
///   which results in a transfer curve that goes from a straight line to an S
///   shaped curve.
//---------------------------------------------------------

class EaseInOut final
{
public:
    EaseInOut()
        : m_easeIn(0.0), m_easeOut(1.0) {}
    EaseInOut(double easeIn, double easeOut)
        : m_easeIn(easeIn), m_easeOut(easeOut) {}

    void SetEases(double easeIn, double easeOut) { m_easeIn = easeIn; m_easeOut = easeOut; }
    double EvalX(const double t) const
    {
        double tCompl = 1.0 - t;
        return (3.0 * m_easeIn * tCompl * tCompl + (3.0 - 3.0 * m_easeOut * tCompl - 2.0 * t) * t) * t;
    }

    double EvalY(const double t) const { return -(t * t) * (2.0 * t - 3.0); }
    muse::PointF Eval(const double t) const { return { EvalX(t), EvalY(t) }; }
    double tFromX(const double x) const;
    double tFromY(const double y) const;
    double YfromX(const double x) const { return EvalY(tFromX(x)); }
    double XfromY(const double y) const { return EvalX(tFromY(y)); }
    void timeList(const int nbNotes, const int duration, std::vector<int>* times) const;

private:
    double m_easeIn = 0.0;
    double m_easeOut = 0.0;
};
} // namespace mu::engraving
#endif
