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

#ifndef MUSE_DRAW_TRANSFORM_H
#define MUSE_DRAW_TRANSFORM_H

#include "global/realfn.h"

#include "matrix.h"
#include "geometry.h"

namespace muse::draw {
class PainterPath;

class Transform
{
public:

    enum class TransformationType {
        None      = 0x00,
        Translate = 0x01,
        Scale     = 0x02,
        Rotate    = 0x04,
        Shear     = 0x08,
        Project   = 0x10
    };

    Transform() = default;

    Transform(double h11, double h12, double h21, double h22, double dx, double dy);

    Transform operator*(const Transform& m) const;
    Transform& operator*=(const Transform& o);
    Transform& operator*=(double num);
    Transform& operator/=(double div);

    bool operator ==(const Transform& other) const
    {
        return m_affine == other.m_affine
               && RealIsEqual(m_13, other.m_13)
               && RealIsEqual(m_23, other.m_23)
               && RealIsEqual(m_33, other.m_33);
    }

    bool operator !=(const Transform& other) const { return !this->operator==(other); }

    double m11() const { return m_affine.m_11; }
    double m12() const { return m_affine.m_12; }
    double m13() const { return m_13; }
    double m21() const { return m_affine.m_21; }
    double m22() const { return m_affine.m_22; }
    double m23() const { return m_23; }
    double m31() const { return m_affine.m_dx; }
    double m32() const { return m_affine.m_dy; }
    double m33() const { return m_33; }
    double dx() const { return m_affine.m_dx; }
    double dy() const { return m_affine.m_dy; }

    PointF map(const PointF& p) const;
    LineF map(const LineF& l) const;
    RectF map(const RectF& rect) const;
    PainterPath map(const PainterPath& path) const;

    const Matrix& toAffine() const { return m_affine; }

    void setMatrix(double m11, double m12, double m13, double m21, double m22, double m23, double m31, double m32, double m33);

    void reset();

    Transform& rotate(double a);
    Transform& rotateRadians(double a);
    Transform& translate(double dx, double dy);
    Transform& scale(double sx, double sy);
    Transform& shear(double sh, double sv);
    Transform inverted() const;

#ifndef NO_QT_SUPPORT
    static QTransform toQTransform(const Transform& transform);
    static Transform fromQTransform(const QTransform& transform);
#endif

private:

    Transform(double h11, double h12, double h13, double h21, double h22, double h23, double h31, double h32, double h33);

    inline double determinant() const
    {
        return m_affine.m_11 * (m_33 * m_affine.m_22 - m_affine.m_dy * m_23)
               - m_affine.m_21 * (m_33 * m_affine.m_12 - m_affine.m_dy * m_13) + m_affine.m_dx
               * (m_23 * m_affine.m_12 - m_affine.m_22 * m_13);
    }

    Transform adjoint() const;

    Transform::TransformationType type() const;

    inline Transform::TransformationType inline_type() const
    {
        if (m_dirty == TransformationType::None) {
            return m_type;
        }
        return type();
    }

    void mapElement(double& nx, double& ny, TransformationType t) const;

#ifdef MUSE_MODULE_DRAW_TRACE
    static void nanWarning(const std::string& func);
#endif

    Matrix m_affine { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
    double m_13 = 0.0;
    double m_23 = 0.0;
    double m_33 = 1.0;

    mutable TransformationType m_type  = TransformationType::None;
    mutable TransformationType m_dirty = TransformationType::None;
};

inline Transform& Transform::operator*=(double num)
{
    if (RealIsEqual(num, 1.)) {
        return *this;
    }
    m_affine.m_11 *= num;
    m_affine.m_12 *= num;
    m_13 *= num;
    m_affine.m_21 *= num;
    m_affine.m_22 *= num;
    m_23 *= num;
    m_affine.m_dx *= num;
    m_affine.m_dy *= num;
    m_33 *= num;
    if (m_dirty < TransformationType::Scale) {
        m_dirty = TransformationType::Scale;
    }
    return *this;
}

inline Transform& Transform::operator/=(double div)
{
    if (RealIsNull(div)) {
        return *this;
    }
    div = 1.0 / div;
    return operator*=(div);
}

inline PointF operator*(const PointF& p, const Transform& m)
{
    return m.map(p);
}

inline Transform operator/(const Transform& a, double n)
{
    Transform t(a);
    t /= n;
    return t;
}
}

#endif // MUSE_DRAW_TRANSFORM_H
