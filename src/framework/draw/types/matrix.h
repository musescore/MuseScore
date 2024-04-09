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

#ifndef MUSE_DRAW_MATRIX_H
#define MUSE_DRAW_MATRIX_H

#include "global/realfn.h"

namespace muse::draw {
class Matrix
{
public:
    Matrix() {}

    Matrix(double m11, double m12, double m21, double m22, double dx, double dy)
        : m_11(m11)
        , m_12(m12)
        , m_21(m21)
        , m_22(m22)
        , m_dx(dx)
        , m_dy(dy)
    {
    }

    Matrix inverted(bool* invertible) const
    {
        double dtr = determinant();
        if (muse::RealIsNull(dtr)) {
            if (invertible) {
                *invertible = false;
            }
            return Matrix();
        } else {
            if (invertible) {
                *invertible = true;
            }
            double dinv = 1.0 / dtr;
            return Matrix((m_22 * dinv), (-m_12 * dinv),
                          (-m_21 * dinv), (m_11 * dinv),
                          ((m_21 * m_dy - m_22 * m_dx) * dinv),
                          ((m_12 * m_dx - m_11 * m_dy) * dinv));
        }
    }

    double determinant() const { return m_11 * m_22 - m_12 * m_21; }

    double m11() const { return m_11; }
    double m12() const { return m_12; }
    double m21() const { return m_21; }
    double m22() const { return m_22; }
    double dx() const { return m_dx; }
    double dy() const { return m_dy; }

    bool operator ==(const Matrix& other) const
    {
        return RealIsEqual(m_11, other.m_11)
               && RealIsEqual(m_12, other.m_12)
               && RealIsEqual(m_21, other.m_21)
               && RealIsEqual(m_22, other.m_22)
               && RealIsEqual(m_dx, other.m_dx)
               && RealIsEqual(m_dy, other.m_dy);
    }

private:
    friend class Transform;

    double m_11 = 1;
    double m_12 = 0;
    double m_21 = 0;
    double m_22 = 1;

    double m_dx = 0;
    double m_dy = 0;
};
}

#endif // MUSE_DRAW_MATRIX_H
