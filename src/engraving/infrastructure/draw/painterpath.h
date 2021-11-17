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

#ifndef MU_DRAW_PAINTERPATH_H
#define MU_DRAW_PAINTERPATH_H

#include <cmath>

#ifndef NO_QT_SUPPORT
#include <QPainterPath>
#endif

#include "geometry.h"
#include "bezier.h"
#include "drawtypes.h"

namespace mu {
class PainterPath
{
public:
    enum class ElementType {
        MoveToElement,
        LineToElement,
        CurveToElement,
        CurveToDataElement
    };

    enum class FillRule {
        OddEvenFill,
        WindingFill
    };

    class Element
    {
    public:
        double x = 0.0;
        double y = 0.0;
        ElementType type = ElementType::MoveToElement;

        Element() {}
        Element(double x, double y, ElementType type)
            : x(x), y(y), type(type) {}
        bool isMoveTo() const { return type == ElementType::MoveToElement; }
        bool isLineTo() const { return type == ElementType::LineToElement; }
        bool isCurveTo() const { return type == ElementType::CurveToElement; }
        operator PointF() const {
            return PointF(x, y);
        }
        bool operator==(const Element& e) const
        {
            return qFuzzyCompare(x, e.x)
                   && qFuzzyCompare(y, e.y) && type == e.type;
        }

        inline bool operator!=(const Element& e) const { return !operator==(e); }
    };

    PainterPath() = default;

    bool operator ==(const PainterPath& other) const { return other.m_elements == m_elements; }
    bool operator !=(const PainterPath& other) const { return !operator ==(other); }

    void moveTo(const PointF& p);
    inline void moveTo(double x, double y) { moveTo(PointF(x, y)); }
    void lineTo(const PointF& p);
    inline void lineTo(double x, double y) { lineTo(PointF(x, y)); }

    void cubicTo(const PointF& ctrlPt1, const PointF& ctrlPt2, const PointF& endPt);

    inline void cubicTo(double ctrlPt1x, double ctrlPt1y, double ctrlPt2x, double ctrlPt2y, double endPtx, double endPty)
    {
        cubicTo(PointF(ctrlPt1x, ctrlPt1y), PointF(ctrlPt2x, ctrlPt2y), PointF(endPtx, endPty));
    }

    void translate(const PointF& offset);
    void translate(double dx, double dy);

    RectF boundingRect() const;

    bool isEmpty() const;
    size_t elementCount() const;
    PainterPath::Element elementAt(size_t i) const;
    void addRect(const RectF& r);
    inline void addRect(double x, double y, double w, double h)
    {
        addRect(RectF(x, y, w, h));
    }

    void addEllipse(const RectF& boundingRect);

    void addRoundedRect(const RectF& rect, double xRadius, double yRadius);

    void arcMoveTo(const RectF& rect, double angle);

    inline void arcMoveTo(double x, double y, double w, double h, double angle)
    {
        arcMoveTo(RectF(x, y, w, h), angle);
    }

    void arcTo(const RectF& rect, double startAngle, double sweepLength);

    inline void arcTo(double x, double y, double w, double h, double startAngle, double arcLength)
    {
        arcTo(RectF(x, y, w, h), startAngle, arcLength);
    }

    void closeSubpath();

    PainterPath::FillRule fillRule() const;
    void setFillRule(PainterPath::FillRule fillRule);

#ifndef NO_QT_SUPPORT
    QPainterPath toQPainterPath() const { return toQPainterPath(*this); }
    static QPainterPath toQPainterPath(const PainterPath& path);
#endif

private:

    void ensureData();

    void computeBoundingRect() const;

    static bool hasValidCoords(PointF p);

    inline void maybeMoveTo()
    {
        if (m_requireMoveTo) {
            Element e = m_elements.back();
            e.type = ElementType::MoveToElement;
            m_elements.push_back(e);
            m_requireMoveTo = false;
        }
    }

    inline bool isClosed() const
    {
        const PainterPath::Element& first = m_elements.at(m_cStart);
        const PainterPath::Element& last = m_elements.back();
        return first.x == last.x && first.y == last.y;
    }

    static inline bool isValidCoord(double c)
    {
        return std::isfinite(c) && std::fabs(c) < 1e128;
    }

    static bool hasValidCoords(RectF r);

    static RectF painterpathBezierExtrema(const Bezier& b);

    void setDirty();

    int m_cStart = 0;
    mutable RectF m_bounds;
    bool m_requireMoveTo = false;
    mutable bool m_dirtyBounds = false;
    bool m_convex = false;
    FillRule m_fillRule = FillRule::OddEvenFill;

    std::vector<Element> m_elements;

    friend class Transform;
};
}

Q_DECLARE_METATYPE(mu::PainterPath)

#endif // MU_DRAW_PAINTERPATH_H
