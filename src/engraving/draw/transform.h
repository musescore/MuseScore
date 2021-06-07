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

#ifndef MU_TRANSFORM_H
#define MU_TRANSFORM_H

#include <QTransform>
#include "geometry.h"

namespace mu {
//! NOTE Temporary implementation
class Transform : public QTransform
{
public:
    Transform() = default;
    Transform(const QTransform& t)
        : QTransform(t) {}

    PointF map(const PointF& p) const { return PointF::fromQPointF(QTransform::map(p.toQPointF())); }
    LineF map(const LineF& p) const { return LineF::fromQLineF(QTransform::map(p.toQLineF())); }
    PainterPath map(const PainterPath& p) const { return QTransform::map(p); }

    Transform inverted(bool* invertible = nullptr) const { return Transform(QTransform::inverted(invertible)); }
};
}

#endif // MU_TRANSFORM_H
