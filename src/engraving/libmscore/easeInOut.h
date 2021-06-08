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

#ifndef __EASEINOUT_H__
#define __EASEINOUT_H__

#include <QtGlobal>
#include "draw/geometry.h"

namespace Ms {
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
    qreal _easeIn;
    qreal _easeOut;

public:
    EaseInOut()
        : _easeIn(0.0), _easeOut(1.0) {}
    EaseInOut(qreal easeIn, qreal easeOut)
        : _easeIn(easeIn), _easeOut(easeOut) {}

    void SetEases(qreal easeIn, qreal easeOut) { _easeIn = easeIn; _easeOut = easeOut; }
    qreal EvalX(const qreal t) const
    {
        qreal tCompl = 1.0 - t;
        return (3.0 * _easeIn * tCompl * tCompl + (3.0 - 3.0 * _easeOut * tCompl - 2.0 * t) * t) * t;
    }

    qreal EvalY(const qreal t) const { return -(t * t) * (2.0 * t - 3.0); }
    mu::PointF Eval(const qreal t) const { return { EvalX(t), EvalY(t) }; }
    qreal tFromX(const qreal x) const;
    qreal tFromY(const qreal y) const;
    qreal YfromX(const qreal x) const { return EvalY(tFromX(x)); }
    qreal XfromY(const qreal y) const { return EvalX(tFromY(y)); }
    void timeList(const int nbNotes, const int duration, QList<int>* times) const;
};
}     // namespace Ms
#endif
