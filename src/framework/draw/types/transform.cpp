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

#include "transform.h"

#include "painterpath.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

static constexpr double NEAR_CLIP = 0.000001;

Transform::Transform(double h11, double h12, double h21, double h22, double dx, double dy)
    : m_affine(h11, h12, h21, h22, dx, dy), m_dirty(TransformationType::Shear)
{
}

Transform::Transform(double h11, double h12, double h13,
                     double h21, double h22, double h23,
                     double h31, double h32, double h33)
    : m_affine(h11, h12, h21, h22, h31, h32)
    , m_13(h13), m_23(h23), m_33(h33)
    , m_type(TransformationType::None)
    , m_dirty(TransformationType::Project)
{
}

Transform Transform::operator*(const Transform& m) const
{
    const TransformationType otherType = m.inline_type();

    if (otherType == TransformationType::None) {
        return *this;
    }
    const TransformationType thisType = inline_type();
    if (thisType == TransformationType::None) {
        return m;
    }
    Transform t;
    TransformationType type = std::max(thisType, otherType);
    switch (type) {
    case TransformationType::None:
        break;
    case TransformationType::Translate:
        t.m_affine.m_dx = m_affine.m_dx + m.m_affine.m_dx;
        t.m_affine.m_dy = m_affine.m_dy + m.m_affine.m_dy;
        break;
    case TransformationType::Scale:
    {
        double m11 = m_affine.m_11 * m.m_affine.m_11;
        double m22 = m_affine.m_22 * m.m_affine.m_22;
        double m31 = m_affine.m_dx * m.m_affine.m_11 + m.m_affine.m_dx;
        double m32 = m_affine.m_dy * m.m_affine.m_22 + m.m_affine.m_dy;
        t.m_affine.m_11 = m11;
        t.m_affine.m_22 = m22;
        t.m_affine.m_dx = m31;
        t.m_affine.m_dy = m32;
        break;
    }
    case TransformationType::Rotate:
    case TransformationType::Shear:
    {
        double m11 = m_affine.m_11 * m.m_affine.m_11 + m_affine.m_12 * m.m_affine.m_21;
        double m12 = m_affine.m_11 * m.m_affine.m_12 + m_affine.m_12 * m.m_affine.m_22;
        double m21 = m_affine.m_21 * m.m_affine.m_11 + m_affine.m_22 * m.m_affine.m_21;
        double m22 = m_affine.m_21 * m.m_affine.m_12 + m_affine.m_22 * m.m_affine.m_22;
        double m31 = m_affine.m_dx * m.m_affine.m_11 + m_affine.m_dy * m.m_affine.m_21 + m.m_affine.m_dx;
        double m32 = m_affine.m_dx * m.m_affine.m_12 + m_affine.m_dy * m.m_affine.m_22 + m.m_affine.m_dy;
        t.m_affine.m_11 = m11;
        t.m_affine.m_12 = m12;
        t.m_affine.m_21 = m21;
        t.m_affine.m_22 = m22;
        t.m_affine.m_dx = m31;
        t.m_affine.m_dy = m32;
        break;
    }
    case TransformationType::Project:
    {
        double m11 = m_affine.m_11 * m.m_affine.m_11 + m_affine.m_12 * m.m_affine.m_21 + m_13 * m.m_affine.m_dx;
        double m12 = m_affine.m_11 * m.m_affine.m_12 + m_affine.m_12 * m.m_affine.m_22 + m_13 * m.m_affine.m_dy;
        double m13 = m_affine.m_11 * m.m_13 + m_affine.m_12 * m.m_23 + m_13 * m.m_33;
        double m21 = m_affine.m_21 * m.m_affine.m_11 + m_affine.m_22 * m.m_affine.m_21 + m_23 * m.m_affine.m_dx;
        double m22 = m_affine.m_21 * m.m_affine.m_12 + m_affine.m_22 * m.m_affine.m_22 + m_23 * m.m_affine.m_dy;
        double m23 = m_affine.m_21 * m.m_13 + m_affine.m_22 * m.m_23 + m_23 * m.m_33;
        double m31 = m_affine.m_dx * m.m_affine.m_11 + m_affine.m_dy * m.m_affine.m_21 + m_33 * m.m_affine.m_dx;
        double m32 = m_affine.m_dx * m.m_affine.m_12 + m_affine.m_dy * m.m_affine.m_22 + m_33 * m.m_affine.m_dy;
        double m33 = m_affine.m_dx * m.m_13 + m_affine.m_dy * m.m_23 + m_33 * m.m_33;
        t.m_affine.m_11 = m11;
        t.m_affine.m_12 = m12;
        t.m_13 = m13;
        t.m_affine.m_21 = m21;
        t.m_affine.m_22 = m22;
        t.m_23 = m23;
        t.m_affine.m_dx = m31;
        t.m_affine.m_dy = m32;
        t.m_33 = m33;
    }
    }
    t.m_dirty = type;
    t.m_type = type;
    return t;
}

Transform& Transform::operator*=(const Transform& o)
{
    const TransformationType otherType = o.inline_type();
    if (otherType == TransformationType::None) {
        return *this;
    }
    const TransformationType thisType = inline_type();
    if (thisType == TransformationType::None) {
        return operator=(o);
    }
    TransformationType t = std::max(thisType, otherType);
    switch (t) {
    case TransformationType::None:
        break;
    case TransformationType::Translate:
        m_affine.m_dx += o.m_affine.m_dx;
        m_affine.m_dy += o.m_affine.m_dy;
        break;
    case TransformationType::Scale:
    {
        double m11 = m_affine.m_11 * o.m_affine.m_11;
        double m22 = m_affine.m_22 * o.m_affine.m_22;
        double m31 = m_affine.m_dx * o.m_affine.m_11 + o.m_affine.m_dx;
        double m32 = m_affine.m_dy * o.m_affine.m_22 + o.m_affine.m_dy;
        m_affine.m_11 = m11;
        m_affine.m_22 = m22;
        m_affine.m_dx = m31;
        m_affine.m_dy = m32;
        break;
    }
    case TransformationType::Rotate:
    case TransformationType::Shear:
    {
        double m11 = m_affine.m_11 * o.m_affine.m_11 + m_affine.m_12 * o.m_affine.m_21;
        double m12 = m_affine.m_11 * o.m_affine.m_12 + m_affine.m_12 * o.m_affine.m_22;
        double m21 = m_affine.m_21 * o.m_affine.m_11 + m_affine.m_22 * o.m_affine.m_21;
        double m22 = m_affine.m_21 * o.m_affine.m_12 + m_affine.m_22 * o.m_affine.m_22;
        double m31 = m_affine.m_dx * o.m_affine.m_11 + m_affine.m_dy * o.m_affine.m_21 + o.m_affine.m_dx;
        double m32 = m_affine.m_dx * o.m_affine.m_12 + m_affine.m_dy * o.m_affine.m_22 + o.m_affine.m_dy;
        m_affine.m_11 = m11;
        m_affine.m_12 = m12;
        m_affine.m_21 = m21;
        m_affine.m_22 = m22;
        m_affine.m_dx = m31;
        m_affine.m_dy = m32;
        break;
    }

    case TransformationType::Project:
    {
        double m11 = m_affine.m_11 * o.m_affine.m_11 + m_affine.m_12 * o.m_affine.m_21 + m_13 * o.m_affine.m_dx;
        double m12 = m_affine.m_11 * o.m_affine.m_12 + m_affine.m_12 * o.m_affine.m_22 + m_13 * o.m_affine.m_dy;
        double m13 = m_affine.m_11 * o.m_13 + m_affine.m_12 * o.m_23 + m_13 * o.m_33;
        double m21 = m_affine.m_21 * o.m_affine.m_11 + m_affine.m_22 * o.m_affine.m_21 + m_23 * o.m_affine.m_dx;
        double m22 = m_affine.m_21 * o.m_affine.m_12 + m_affine.m_22 * o.m_affine.m_22 + m_23 * o.m_affine.m_dy;
        double m23 = m_affine.m_21 * o.m_13 + m_affine.m_22 * o.m_23 + m_23 * o.m_33;
        double m31 = m_affine.m_dx * o.m_affine.m_11 + m_affine.m_dy * o.m_affine.m_21 + m_33 * o.m_affine.m_dx;
        double m32 = m_affine.m_dx * o.m_affine.m_12 + m_affine.m_dy * o.m_affine.m_22 + m_33 * o.m_affine.m_dy;
        double m33 = m_affine.m_dx * o.m_13 + m_affine.m_dy * o.m_23 + m_33 * o.m_33;
        m_affine.m_11 = m11;
        m_affine.m_12 = m12;
        m_13 = m13;
        m_affine.m_21 = m21;
        m_affine.m_22 = m22;
        m_23 = m23;
        m_affine.m_dx = m31;
        m_affine.m_dy = m32;
        m_33 = m33;
    }
    }
    m_dirty = t;
    m_type = t;
    return *this;
}

void Transform::setMatrix(double m11, double m12, double m13,
                          double m21, double m22, double m23,
                          double m31, double m32, double m33)
{
    m_affine.m_11 = m11;
    m_affine.m_12 = m12;
    m_13 = m13;
    m_affine.m_21 = m21;
    m_affine.m_22 = m22;
    m_23 = m23;
    m_affine.m_dx = m31;
    m_affine.m_dy = m32;
    m_33 = m33;
}

void Transform::reset()
{
    m_affine.m_11 = m_affine.m_22 = m_33 = 1.0;
    m_affine.m_12 = m_13 = m_affine.m_21 = m_23 = m_affine.m_dx = m_affine.m_dy = 0.0;
}

Transform Transform::adjoint() const
{
    double h11, h12, h13,
           h21, h22, h23,
           h31, h32, h33;
    h11 = m_affine.m_22 * m_33 - m_23 * m_affine.m_dy;
    h21 = m_23 * m_affine.m_dx - m_affine.m_21 * m_33;
    h31 = m_affine.m_21 * m_affine.m_dy - m_affine.m_22 * m_affine.m_dx;
    h12 = m_13 * m_affine.m_dy - m_affine.m_12 * m_33;
    h22 = m_affine.m_11 * m_33 - m_13 * m_affine.m_dx;
    h32 = m_affine.m_12 * m_affine.m_dx - m_affine.m_11 * m_affine.m_dy;
    h13 = m_affine.m_12 * m_23 - m_13 * m_affine.m_22;
    h23 = m_13 * m_affine.m_21 - m_affine.m_11 * m_23;
    h33 = m_affine.m_11 * m_affine.m_22 - m_affine.m_12 * m_affine.m_21;
    return Transform(h11, h12, h13,
                     h21, h22, h23,
                     h31, h32, h33);
}

Transform::TransformationType Transform::type() const
{
    if (m_dirty == TransformationType::None || m_dirty < m_type) {
        return m_type;
    }
    switch (m_dirty) {
    case TransformationType::Project:
        if (!RealIsNull(m_13) || !RealIsNull(m_23) || !RealIsNull(m_33 - 1)) {
            m_type = TransformationType::Project;
            break;
        }
    // fall through
    case TransformationType::Shear:
    case TransformationType::Rotate:
        if (!RealIsNull(m_affine.m_12) || !RealIsNull(m_affine.m_21)) {
            const double dot = m_affine.m_11 * m_affine.m_12 + m_affine.m_21 * m_affine.m_22;
            if (RealIsNull(dot)) {
                m_type = TransformationType::Rotate;
            } else {
                m_type = TransformationType::Shear;
            }
            break;
        }
    // fall through
    case TransformationType::Scale:
        if (!RealIsNull(m_affine.m_11 - 1) || !RealIsNull(m_affine.m_22 - 1)) {
            m_type = TransformationType::Scale;
            break;
        }
    // fall through
    case TransformationType::Translate:
        if (!RealIsNull(m_affine.m_dx) || !RealIsNull(m_affine.m_dy)) {
            m_type = TransformationType::Translate;
            break;
        }
    // fall through
    case TransformationType::None:
        m_type = TransformationType::None;
        break;
    }
    m_dirty = TransformationType::None;
    return m_type;
}

PointF Transform::map(const PointF& p) const
{
    double fx = p.x();
    double fy = p.y();
    double x = 0.0, y = 0.0;
    TransformationType t = inline_type();
    switch (t) {
    case TransformationType::None:
        x = fx;
        y = fy;
        break;
    case TransformationType::Translate:
        x = fx + m_affine.m_dx;
        y = fy + m_affine.m_dy;
        break;
    case TransformationType::Scale:
        x = m_affine.m_11 * fx + m_affine.m_dx;
        y = m_affine.m_22 * fy + m_affine.m_dy;
        break;
    case TransformationType::Rotate:
    case TransformationType::Shear:
    case TransformationType::Project:
        x = m_affine.m_11 * fx + m_affine.m_21 * fy + m_affine.m_dx;
        y = m_affine.m_12 * fx + m_affine.m_22 * fy + m_affine.m_dy;
        if (t == TransformationType::Project) {
            double w = 1. / (m_13 * fx + m_23 * fy + m_33);
            x *= w;
            y *= w;
        }
    }
    return PointF(x, y);
}

LineF Transform::map(const LineF& l) const
{
    double fx1 = l.x1();
    double fy1 = l.y1();
    double fx2 = l.x2();
    double fy2 = l.y2();
    double x1 = 0.0, y1 = 0.0, x2 = 0.0, y2 = 0.0;
    TransformationType t = inline_type();
    switch (t) {
    case TransformationType::None:
        x1 = fx1;
        y1 = fy1;
        x2 = fx2;
        y2 = fy2;
        break;
    case TransformationType::Translate:
        x1 = fx1 + m_affine.m_dx;
        y1 = fy1 + m_affine.m_dy;
        x2 = fx2 + m_affine.m_dx;
        y2 = fy2 + m_affine.m_dy;
        break;
    case TransformationType::Scale:
        x1 = m_affine.m_11 * fx1 + m_affine.m_dx;
        y1 = m_affine.m_22 * fy1 + m_affine.m_dy;
        x2 = m_affine.m_11 * fx2 + m_affine.m_dx;
        y2 = m_affine.m_22 * fy2 + m_affine.m_dy;
        break;
    case TransformationType::Rotate:
    case TransformationType::Shear:
    case TransformationType::Project:
        x1 = m_affine.m_11 * fx1 + m_affine.m_21 * fy1 + m_affine.m_dx;
        y1 = m_affine.m_12 * fx1 + m_affine.m_22 * fy1 + m_affine.m_dy;
        x2 = m_affine.m_11 * fx2 + m_affine.m_21 * fy2 + m_affine.m_dx;
        y2 = m_affine.m_12 * fx2 + m_affine.m_22 * fy2 + m_affine.m_dy;
        if (t == TransformationType::Project) {
            double w = 1. / (m_13 * fx1 + m_23 * fy1 + m_33);
            x1 *= w;
            y1 *= w;
            w = 1. / (m_13 * fx2 + m_23 * fy2 + m_33);
            x2 *= w;
            y2 *= w;
        }
    }
    return LineF(x1, y1, x2, y2);
}

static inline bool needsPerspectiveClipping(const RectF& rect, const Transform& transform)
{
    const double wx = std::min(transform.m13() * rect.left(), transform.m13() * rect.right());
    const double wy = std::min(transform.m23() * rect.top(), transform.m23() * rect.bottom());

    return wx + wy + transform.m33() < NEAR_CLIP;
}

RectF Transform::map(const RectF& rect) const
{
    TransformationType t = inline_type();
    if (t <= TransformationType::Translate) {
        return rect.translated(m_affine.m_dx, m_affine.m_dy);
    }

    if (t <= TransformationType::Scale) {
        double x = m_affine.m_11 * rect.x() + m_affine.m_dx;
        double y = m_affine.m_22 * rect.y() + m_affine.m_dy;
        double w = m_affine.m_11 * rect.width();
        double h = m_affine.m_22 * rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        return RectF(x, y, w, h);
    } else if (t < TransformationType::Project || !needsPerspectiveClipping(rect, *this)) {
        double x = rect.x(), y = rect.y();
        mapElement(x, y, t);
        double xmin = x;
        double ymin = y;
        double xmax = x;
        double ymax = y;

        x = rect.x() + rect.width();
        y = rect.y();
        mapElement(x, y, t);
        xmin = std::min(xmin, x);
        ymin = std::min(ymin, y);
        xmax = std::max(xmax, x);
        ymax = std::max(ymax, y);

        x = rect.x() + rect.width();
        y = rect.y() + rect.height();
        mapElement(x, y, t);
        xmin = std::min(xmin, x);
        ymin = std::min(ymin, y);
        xmax = std::max(xmax, x);
        ymax = std::max(ymax, y);

        x = rect.x();
        y = rect.y() + rect.height();
        mapElement(x, y, t);
        xmin = std::min(xmin, x);
        ymin = std::min(ymin, y);
        xmax = std::max(xmax, x);
        ymax = std::max(ymax, y);

        return RectF(xmin, ymin, xmax - xmin, ymax - ymin);
    } else {
        PainterPath path;
        path.addRect(rect);
        return map(path).boundingRect();
    }
}

PainterPath Transform::map(const PainterPath& path) const
{
    TransformationType t = inline_type();
    if (t == TransformationType::None || path.elementCount() == 0) {
        return path;
    }

    // TransformationType::Project is not supported here, it was not used while mapping PainterPath and it needs a lot of code to handle
    PainterPath copy = path;

    if (t == TransformationType::Translate) {
        copy.translate(m_affine.m_dx, m_affine.m_dy);
    } else {
        // Full xform
        for (size_t i = 0; i < path.elementCount(); ++i) {
            PainterPath::Element& e = copy.m_elements[i];
            mapElement(e.x, e.y, t);
        }

        copy.setDirty();
    }

    return copy;
}

Transform& Transform::rotate(double a)
{
    constexpr double deg2rad = 0.017453292519943295769; // pi/180

    if (RealIsNull(a)) {
        return *this;
    }
#ifdef MUSE_MODULE_DRAW_TRACE
    if (std::isnan(a)) {
        nanWarning("rotate");
        return *this;
    }
#endif

    double sina = 0.0;
    double cosa = 0.0;
    if (RealIsEqual(a, 90.) || RealIsEqual(a, -270.)) {
        sina = 1.;
    } else if (RealIsEqual(a, 270.) || RealIsEqual(a, -90.)) {
        sina = -1.;
    } else if (RealIsEqual(a, 180.)) {
        cosa = -1.;
    } else {
        double b = deg2rad * a;
        sina = std::sin(b);
        cosa = std::cos(b);
    }
    switch (inline_type()) {
    case TransformationType::None:
    case TransformationType::Translate:
        m_affine.m_11 = cosa;
        m_affine.m_12 = sina;
        m_affine.m_21 = -sina;
        m_affine.m_22 = cosa;
        break;
    case TransformationType::Scale: {
        double tm11 = cosa * m_affine.m_11;
        double tm12 = sina * m_affine.m_22;
        double tm21 = -sina * m_affine.m_11;
        double tm22 = cosa * m_affine.m_22;
        m_affine.m_11 = tm11;
        m_affine.m_12 = tm12;
        m_affine.m_21 = tm21;
        m_affine.m_22 = tm22;
        break;
    }
    case TransformationType::Project: {
        double tm13 = cosa * m_13 + sina * m_23;
        double tm23 = -sina * m_13 + cosa * m_23;
        m_13 = tm13;
        m_23 = tm23;
    }
    // fallthrough
    case TransformationType::Rotate:
    case TransformationType::Shear: {
        double tm11 = cosa * m_affine.m_11 + sina * m_affine.m_21;
        double tm12 = cosa * m_affine.m_12 + sina * m_affine.m_22;
        double tm21 = -sina * m_affine.m_11 + cosa * m_affine.m_21;
        double tm22 = -sina * m_affine.m_12 + cosa * m_affine.m_22;
        m_affine.m_11 = tm11;
        m_affine.m_12 = tm12;
        m_affine.m_21 = tm21;
        m_affine.m_22 = tm22;
        break;
    }
    }
    if (m_dirty < TransformationType::Rotate) {
        m_dirty = TransformationType::Rotate;
    }

    return *this;
}

Transform& Transform::rotateRadians(double a)
{
#ifdef MUSE_MODULE_DRAW_TRACE
    if (std::isnan(a)) {
        nanWarning("rotateRadians");
        return *this;
    }
#endif
    double sina = std::sin(a);
    double cosa = std::cos(a);
    switch (inline_type()) {
    case TransformationType::None:
    case TransformationType::Translate:
        m_affine.m_11 = cosa;
        m_affine.m_12 = sina;
        m_affine.m_21 = -sina;
        m_affine.m_22 = cosa;
        break;
    case TransformationType::Scale: {
        double tm11 = cosa * m_affine.m_11;
        double tm12 = sina * m_affine.m_22;
        double tm21 = -sina * m_affine.m_11;
        double tm22 = cosa * m_affine.m_22;
        m_affine.m_11 = tm11;
        m_affine.m_12 = tm12;
        m_affine.m_21 = tm21;
        m_affine.m_22 = tm22;
        break;
    }
    case TransformationType::Project: {
        double tm13 = cosa * m_13 + sina * m_23;
        double tm23 = -sina * m_13 + cosa * m_23;
        m_13 = tm13;
        m_23 = tm23;
    }
    // fallthrough
    case TransformationType::Rotate:
    case TransformationType::Shear: {
        double tm11 = cosa * m_affine.m_11 + sina * m_affine.m_21;
        double tm12 = cosa * m_affine.m_12 + sina * m_affine.m_22;
        double tm21 = -sina * m_affine.m_11 + cosa * m_affine.m_21;
        double tm22 = -sina * m_affine.m_12 + cosa * m_affine.m_22;
        m_affine.m_11 = tm11;
        m_affine.m_12 = tm12;
        m_affine.m_21 = tm21;
        m_affine.m_22 = tm22;
        break;
    }
    }
    if (m_dirty < TransformationType::Rotate) {
        m_dirty = TransformationType::Rotate;
    }

    return *this;
}

Transform& Transform::translate(double dx, double dy)
{
    if (RealIsNull(dx) && RealIsNull(dy)) {
        return *this;
    }
#ifdef MUSE_MODULE_DRAW_TRACE
    if (std::isnan(dx) || std::isnan(dy)) {
        nanWarning("translate");
        return *this;
    }
#endif
    switch (inline_type()) {
    case TransformationType::None:
        m_affine.m_dx = dx;
        m_affine.m_dy = dy;
        break;
    case TransformationType::Translate:
        m_affine.m_dx += dx;
        m_affine.m_dy += dy;
        break;
    case TransformationType::Scale:
        m_affine.m_dx += dx * m_affine.m_11;
        m_affine.m_dy += dy * m_affine.m_22;
        break;
    case TransformationType::Project:
        m_33 += dx * m_13 + dy * m_23;
    // fallthrough
    case TransformationType::Shear:
    case TransformationType::Rotate:
        m_affine.m_dx += dx * m_affine.m_11 + dy * m_affine.m_21;
        m_affine.m_dy += dy * m_affine.m_22 + dx * m_affine.m_12;
        break;
    }
    if (m_dirty < TransformationType::Translate) {
        m_dirty = TransformationType::Translate;
    }
    return *this;
}

Transform& Transform::scale(double sx, double sy)
{
    if (RealIsEqual(sx, 1) && RealIsEqual(sy, 1)) {
        return *this;
    }
#ifdef MUSE_MODULE_DRAW_TRACE
    if (std::isnan(sx) || std::isnan(sy)) {
        nanWarning("scale");
        return *this;
    }
#endif
    switch (inline_type()) {
    case TransformationType::None:
    case TransformationType::Translate:
        m_affine.m_11 = sx;
        m_affine.m_22 = sy;
        break;
    case TransformationType::Project:
        m_13 *= sx;
        m_23 *= sy;
    // fall through
    case TransformationType::Rotate:
    case TransformationType::Shear:
        m_affine.m_12 *= sx;
        m_affine.m_21 *= sy;
    // fall through
    case TransformationType::Scale:
        m_affine.m_11 *= sx;
        m_affine.m_22 *= sy;
        break;
    }
    if (m_dirty < TransformationType::Scale) {
        m_dirty = TransformationType::Scale;
    }
    return *this;
}

Transform& Transform::shear(double sh, double sv)
{
    if (RealIsNull(sh) && RealIsNull(sv)) {
        return *this;
    }
#ifdef MUSE_MODULE_DRAW_TRACE
    if (std::isnan(sh) || std::isnan(sv)) {
        nanWarning("shear");
        return *this;
    }
#endif
    switch (inline_type()) {
    case TransformationType::None:
    case TransformationType::Translate:
        m_affine.m_12 = sv;
        m_affine.m_21 = sh;
        break;
    case TransformationType::Scale:
        m_affine.m_12 = sv * m_affine.m_22;
        m_affine.m_21 = sh * m_affine.m_11;
        break;
    case TransformationType::Project: {
        double tm13 = sv * m_23;
        double tm23 = sh * m_13;
        m_13 += tm13;
        m_23 += tm23;
    }
    // fallthrough
    case TransformationType::Rotate:
    case TransformationType::Shear: {
        double tm11 = sv * m_affine.m_21;
        double tm22 = sh * m_affine.m_12;
        double tm12 = sv * m_affine.m_22;
        double tm21 = sh * m_affine.m_11;
        m_affine.m_11 += tm11;
        m_affine.m_12 += tm12;
        m_affine.m_21 += tm21;
        m_affine.m_22 += tm22;
        break;
    }
    }
    if (m_dirty < TransformationType::Shear) {
        m_dirty = TransformationType::Shear;
    }
    return *this;
}

Transform Transform::inverted() const
{
    Transform invert;
    bool inv = true;
    switch (inline_type()) {
    case TransformationType::None:
        break;
    case TransformationType::Translate:
        invert.m_affine.m_dx = -m_affine.m_dx;
        invert.m_affine.m_dy = -m_affine.m_dy;
        break;
    case TransformationType::Scale:
        inv = !RealIsNull(m_affine.m_11);
        inv &= !RealIsNull(m_affine.m_22);
        if (inv) {
            invert.m_affine.m_11 = 1. / m_affine.m_11;
            invert.m_affine.m_22 = 1. / m_affine.m_22;
            invert.m_affine.m_dx = -m_affine.m_dx * invert.m_affine.m_11;
            invert.m_affine.m_dy = -m_affine.m_dy * invert.m_affine.m_22;
        }
        break;
    case TransformationType::Rotate:
    case TransformationType::Shear:
        invert.m_affine = m_affine.inverted(&inv);
        break;
    default:
        // general case
        double det = determinant();
        inv = !RealIsNull(det);
        if (inv) {
            invert = adjoint() / det;
        }
        break;
    }

    if (inv) {
        // inverting doesn't change the type
        invert.m_type = m_type;
        invert.m_dirty = m_dirty;
    }
    return invert;
}

void Transform::mapElement(double& nx, double& ny, TransformationType t) const
{
    double FX_ = nx;
    double FY_ = ny;
    switch (t) {
    case TransformationType::None:
        nx = FX_;
        ny = FY_;
        break;
    case TransformationType::Translate:
        nx = FX_ + m_affine.m_dx;
        ny = FY_ + m_affine.m_dy;
        break;
    case TransformationType::Scale:
        nx = m_affine.m_11 * FX_ + m_affine.m_dx;
        ny = m_affine.m_22 * FY_ + m_affine.m_dy;
        break;
    case TransformationType::Rotate:
    case TransformationType::Shear:
    case TransformationType::Project:
        nx = m_affine.m_11 * FX_ + m_affine.m_21 * FY_ + m_affine.m_dx;
        ny = m_affine.m_12 * FX_ + m_affine.m_22 * FY_ + m_affine.m_dy;
        if (t == TransformationType::Project) {
            double w = (m_13 * FX_ + m_23 * FY_ + m_33);
            if (w < NEAR_CLIP) {
                w = NEAR_CLIP;
            }
            w = 1. / w;
            nx *= w;
            ny *= w;
        }
    }
}

#ifndef NO_QT_SUPPORT
QTransform Transform::toQTransform(const Transform& transform)
{
    return QTransform(transform.m_affine.m_11, transform.m_affine.m_12, transform.m_13, transform.m_affine.m_21, transform.m_affine.m_22,
                      transform.m_23, transform.m_affine.m_dx, transform.m_affine.m_dy, transform.m_33);
}

Transform Transform::fromQTransform(const QTransform& transform)
{
    return Transform(transform.m11(), transform.m12(), transform.m13(), transform.m21(), transform.m22(), transform.m23(),
                     transform.m31(), transform.m32(), transform.m33());
}

#endif

#ifdef MUSE_MODULE_DRAW_TRACE
void Transform::nanWarning(const std::string& func)
{
    LOGW() << "Transform:: " << func << " with NaN called";
}

#endif
