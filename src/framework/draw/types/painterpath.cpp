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

#include <cassert>
#include "painterpath.h"

#include "log.h"

namespace muse::draw {
static constexpr double pi = 3.14159265358979323846;
static constexpr double pathKappa = 0.5522847498;

static void findEllipseCoords(const RectF& r, double angle, double length, PointF* startPoint, PointF* endPoint);

static PointF curvesForArc(const RectF& rect, double startAngle, double sweepLength, PointF* curves, int* point_count);

static double angleForArc(double angle);

void PainterPath::moveTo(const PointF& p)
{
    if (!hasValidCoords(p)) {
#ifdef MUSE_MODULE_DRAW_TRACE
        LOGW() << "PainterPath::moveTo: Adding point with invalid coordinates, ignoring call";
#endif
        return;
    }
    ensureData();
    setDirty();
    assert(!m_elements.empty());
    m_requireMoveTo = false;
    if (m_elements.back().type == ElementType::MoveToElement) {
        m_elements.back().x = p.x();
        m_elements.back().y = p.y();
    } else {
        m_elements.push_back({ p.x(), p.y(), ElementType::MoveToElement });
    }
    m_cStart = static_cast<int>(m_elements.size() - 1);
}

void PainterPath::lineTo(const PointF& p)
{
    if (!hasValidCoords(p)) {
#ifdef MUSE_MODULE_DRAW_TRACE
        LOGW() << "PainterPath::lineTo: Adding point with invalid coordinates, ignoring call";
#endif
        return;
    }
    ensureData();
    setDirty();

    assert(!m_elements.empty());
    maybeMoveTo();
    if (p == PointF(m_elements.back())) {
        return;
    }

    m_elements.push_back({ p.x(), p.y(), ElementType::LineToElement });
    m_convex = m_elements.size() == 3 || (m_elements.size() == 4 && isClosed());
}

void PainterPath::cubicTo(const PointF& ctrlPt1, const PointF& ctrlPt2, const PointF& endPt)
{
    if (!hasValidCoords(ctrlPt1) || !hasValidCoords(ctrlPt2) || !hasValidCoords(endPt)) {
   #ifdef MUSE_MODULE_DRAW_TRACE
        static int count = 0;
        LOGW() << "[" << ++count << "] Points with invalid coordinates, ignoring it,"
               << " point1: " << ctrlPt1
               << " point2: " << ctrlPt2
               << " point3: " << endPt;
   #endif
        return;
    }
    ensureData();
    setDirty();

    assert(!m_elements.empty());
    // Abort on empty curve as a stroker cannot handle this and the
    // curve is irrelevant anyway.

    if (ctrlPt1 == m_elements.back() && ctrlPt1 == ctrlPt2 && ctrlPt2 == endPt) {
        return;
    }
    maybeMoveTo();

    m_elements.push_back({ ctrlPt1.x(), ctrlPt1.y(), ElementType::CurveToElement });
    m_elements.push_back({ ctrlPt2.x(), ctrlPt2.y(), ElementType::CurveToDataElement });
    m_elements.push_back({ endPt.x(), endPt.y(), ElementType::CurveToDataElement });
}

void PainterPath::translate(double dx, double dy)
{
    if (RealIsNull(dx) && RealIsNull(dy)) {
        return;
    }
    auto m_elementsLeft = m_elements.size();
    if (m_elementsLeft <= 0) {
        return;
    }

    setDirty();
    PainterPath::Element* element = m_elements.data();
    assert(element);
    while (m_elementsLeft--) {
        element->x += dx;
        element->y += dy;
        ++element;
    }
}

void PainterPath::translate(const PointF& offset)
{
    translate(offset.x(), offset.y());
}

RectF PainterPath::boundingRect() const
{
    if (m_dirtyBounds) {
        computeBoundingRect();
    }
    return m_bounds;
}

bool PainterPath::isEmpty() const
{
    return m_elements.empty() || (m_elements.size() == 1 && m_elements.front().type == ElementType::MoveToElement);
}

size_t PainterPath::elementCount() const
{
    return m_elements.size();
}

PainterPath::Element PainterPath::elementAt(size_t i) const
{
    assert(i < elementCount());
    return m_elements.at(i);
}

void PainterPath::addRect(const RectF& r)
{
    if (!hasValidCoords(r)) {
#ifdef MUSE_MODULE_DRAW_TRACE
        LOGW() << "PainterPath::addRect: Adding point with invalid coordinates, ignoring call";
#endif
        return;
    }
    if (r.isNull()) {
        return;
    }
    ensureData();
    setDirty();

    bool first = m_elements.size() < 2;
    moveTo(r.x(), r.y());
    m_elements.push_back({ r.x() + r.width(), r.y(), ElementType::LineToElement });
    m_elements.push_back({ r.x() + r.width(), r.y() + r.height(), ElementType::LineToElement });
    m_elements.push_back({ r.x(), r.y() + r.height(), ElementType::LineToElement });
    m_elements.push_back({ r.x(), r.y(), ElementType::LineToElement });

    m_requireMoveTo = true;
    m_convex = first;
}

void PainterPath::addEllipse(const RectF& boundingRect)
{
    if (!hasValidCoords(boundingRect)) {
#ifdef MUSE_MODULE_DRAW_TRACE
        LOGW() << "PainterPath::addEllipse: Adding point with invalid coordinates, ignoring call";
#endif
        return;
    }
    if (boundingRect.isNull()) {
        return;
    }
    ensureData();
    setDirty();

    bool first = m_elements.size() < 2;
    PointF pts[12];
    int point_count;
    PointF start = curvesForArc(boundingRect, 0, -360, pts, &point_count);
    moveTo(start);
    cubicTo(pts[0], pts[1], pts[2]);           // 0 -> 270
    cubicTo(pts[3], pts[4], pts[5]);           // 270 -> 180
    cubicTo(pts[6], pts[7], pts[8]);           // 180 -> 90
    cubicTo(pts[9], pts[10], pts[11]);         // 90 - >0
    m_requireMoveTo = true;
    m_convex = first;
}

void PainterPath::addRoundedRect(const RectF& rect, double xRadius, double yRadius)
{
    RectF r = rect.normalized();
    if (r.isNull()) {
        return;
    }

    {
        double w = r.width() / 2;
        double h = r.height() / 2;
        if (RealIsNull(w)) {
            xRadius = 0.0;
        } else {
            xRadius = 100 * std::min(xRadius, w) / w;
        }
        if (RealIsNull(h)) {
            yRadius = 0.0;
        } else {
            yRadius = 100 * std::min(yRadius, h) / h;
        }
    }

    if (xRadius <= 0 || yRadius <= 0) {             // add normal rectangle
        addRect(r);
        return;
    }
    double x = r.x();
    double y = r.y();
    double w = r.width();
    double h = r.height();
    double rxx2 = w * xRadius / 100;
    double ryy2 = h * yRadius / 100;
    ensureData();
    setDirty();

    bool first = m_elements.size() < 2;
    arcMoveTo(x, y, rxx2, ryy2, 180);
    arcTo(x, y, rxx2, ryy2, 180, -90);
    arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
    arcTo(x + w - rxx2, y + h - ryy2, rxx2, ryy2, 0, -90);
    arcTo(x, y + h - ryy2, rxx2, ryy2, 270, -90);
    closeSubpath();
    m_requireMoveTo = true;
    m_convex = first;
}

void PainterPath::arcMoveTo(const RectF& rect, double angle)
{
    if (rect.isNull()) {
        return;
    }
    PointF pt;
    findEllipseCoords(rect, angle, 0, &pt, 0);
    moveTo(pt);
}

void PainterPath::arcTo(const RectF& rect, double startAngle, double sweepLength)
{
    if (!hasValidCoords(rect) || !isValidCoord(startAngle) || !isValidCoord(sweepLength)) {
#ifdef MUSE_MODULE_DRAW_TRACE
        LOGW() << "PainterPath::arcTo: Adding point with invalid coordinates, ignoring call";
#endif
        return;
    }
    if (rect.isNull()) {
        return;
    }
    ensureData();
    setDirty();

    int point_count;
    PointF pts[15];
    PointF curve_start = curvesForArc(rect, startAngle, sweepLength, pts, &point_count);
    lineTo(curve_start);
    for (int i=0; i < point_count; i+=3) {
        cubicTo(pts[i].x(), pts[i].y(),
                pts[i + 1].x(), pts[i + 1].y(),
                pts[i + 2].x(), pts[i + 2].y());
    }
}

#ifndef NO_QT_SUPPORT
QPainterPath PainterPath::toQPainterPath(const PainterPath& path)
{
    QPainterPath qpath;
    std::vector<QPainterPath::Element> curveEls;

    qpath.setFillRule(static_cast<Qt::FillRule>(path.fillRule()));

    for (size_t i = 0; i < path.elementCount(); i++) {
        auto elem = path.elementAt(i);

        QPainterPath::ElementType type = static_cast<QPainterPath::ElementType>(elem.type);
        double x = elem.x;
        double y = elem.y;

        switch (type) {
        case QPainterPath::MoveToElement: {
            qpath.moveTo(x, y);
        } break;
        case QPainterPath::LineToElement: {
            qpath.lineTo(x, y);
        } break;
        case QPainterPath::CurveToElement: {
            IF_ASSERT_FAILED(curveEls.empty()) {
                continue;
            }
            QPainterPath::Element e;
            e.type = type;
            e.x = x;
            e.y = y;
            curveEls.push_back(std::move(e));
        } break;
        case QPainterPath::CurveToDataElement: {
            if (curveEls.size() == 1) { // only CurveToElement
                QPainterPath::Element e;
                e.type = type;
                e.x = x;
                e.y = y;
                curveEls.push_back(std::move(e));
                continue;
            }

            IF_ASSERT_FAILED(curveEls.size() == 2) { // must be CurveToElement and one CurveToDataElement
                curveEls.clear();
                continue;
            }

            qpath.cubicTo(curveEls.at(0).x, curveEls.at(0).y, curveEls.at(1).x, curveEls.at(1).y, x, y);
            curveEls.clear();
        } break;
        }
    }
    return qpath;
}

#endif

void PainterPath::closeSubpath()
{
    if (isEmpty()) {
        return;
    }
    setDirty();
    m_requireMoveTo = true;
    const Element& first = m_elements.at(m_cStart);
    Element& last = m_elements.back();
    if (first.x != last.x || first.y != last.y) {
        if (RealIsEqual(first.x, last.x) && RealIsEqual(first.y, last.y)) {
            last.x = first.x;
            last.y = first.y;
        } else {
            m_elements.push_back({ first.x, first.y, ElementType::LineToElement });
        }
    }
}

PainterPath::FillRule PainterPath::fillRule() const
{
    return isEmpty() ? FillRule::OddEvenFill : m_fillRule;
}

void PainterPath::setFillRule(PainterPath::FillRule fillRule)
{
    ensureData();
    if (m_fillRule == fillRule) {
        return;
    }
    setDirty();
    m_fillRule = fillRule;
}

void PainterPath::ensureData()
{
    if (m_elements.empty()) {
        m_elements.reserve(16);
        m_elements.push_back({ 0, 0, ElementType::MoveToElement });
    }
}

bool PainterPath::hasValidCoords(const PointF& p)
{
    return isValidCoord(p.x()) && isValidCoord(p.y());
}

bool PainterPath::hasValidCoords(const RectF& r)
{
    return isValidCoord(r.x()) && isValidCoord(r.y()) && isValidCoord(r.width()) && isValidCoord(r.height());
}

void PainterPath::computeBoundingRect() const
{
    m_dirtyBounds = false;

    double maxx, maxy;
    double minx = maxx = m_elements.at(0).x;
    double miny = maxy = m_elements.at(0).y;
    for (size_t i = 1; i < m_elements.size(); ++i) {
        const Element& e = m_elements.at(i);
        switch (e.type) {
        case ElementType::MoveToElement:
        case ElementType::LineToElement:
            if (e.x > maxx) {
                maxx = e.x;
            } else if (e.x < minx) {
                minx = e.x;
            }
            if (e.y > maxy) {
                maxy = e.y;
            } else if (e.y < miny) {
                miny = e.y;
            }
            break;
        case ElementType::CurveToElement:
        {
            Bezier b = Bezier::fromPoints(m_elements.at(i - 1),
                                          e,
                                          m_elements.at(i + 1),
                                          m_elements.at(i + 2));
            RectF r = painterpathBezierExtrema(b);
            double right = r.right();
            double bottom = r.bottom();
            if (r.x() < minx) {
                minx = r.x();
            }
            if (right > maxx) {
                maxx = right;
            }
            if (r.y() < miny) {
                miny = r.y();
            }
            if (bottom > maxy) {
                maxy = bottom;
            }
            i += 2;
        }
        break;
        default:
            break;
        }
    }
    m_bounds = RectF(minx, miny, maxx - minx, maxy - miny);
}

static void findEllipseCoords(const RectF& r, double angle, double length,
                              PointF* startPoint, PointF* endPoint)
{
    if (r.isNull()) {
        if (startPoint) {
            *startPoint = PointF();
        }
        if (endPoint) {
            *endPoint = PointF();
        }
        return;
    }
    double w2 = r.width() / 2;
    double h2 = r.height() / 2;
    double angles[2] = { angle, angle + length };
    PointF* points[2] = { startPoint, endPoint };
    for (int i = 0; i < 2; ++i) {
        if (!points[i]) {
            continue;
        }
        double theta = angles[i] - 360 * static_cast<int>(std::floor(angles[i] / 360));
        double t = theta / 90;
        // truncate
        int quadrant = int(t);
        t -= quadrant;
        t = angleForArc(90 * t);
        // swap x and y?
        if (quadrant & 1) {
            t = 1 - t;
        }
        double a, b, c, d;
        Bezier::coefficients(t, a, b, c, d);
        PointF p(a + b + c * pathKappa, d + c + b * pathKappa);
        // left quadrants
        if (quadrant == 1 || quadrant == 2) {
            p.rx() = -p.x();
        }
        // top quadrants
        if (quadrant == 0 || quadrant == 1) {
            p.ry() = -p.y();
        }
        *points[i] = r.center() + PointF(w2 * p.x(), h2 * p.y());
    }
}

static PointF curvesForArc(const RectF& rect, double startAngle, double sweepLength,
                           PointF* curves, int* point_count)
{
    assert(point_count);
    assert(curves);
    *point_count = 0;
    if (std::isnan(rect.x()) || std::isnan(rect.y()) || std::isnan(rect.width()) || std::isnan(rect.height())
        || std::isnan(startAngle) || std::isnan(sweepLength)) {
        LOGW("PainterPath::arcTo: Adding arc where a parameter is NaN, results are undefined");
        return PointF();
    }
    if (rect.isNull()) {
        return PointF();
    }
    double x = rect.x();
    double y = rect.y();
    double w = rect.width();
    double w2 = rect.width() / 2;
    double w2k = w2 * pathKappa;
    double h = rect.height();
    double h2 = rect.height() / 2;
    double h2k = h2 * pathKappa;
    PointF points[16] =
    {
        // start point
        PointF(x + w, y + h2),
        // 0 -> 270 degrees
        PointF(x + w, y + h2 + h2k),
        PointF(x + w2 + w2k, y + h),
        PointF(x + w2, y + h),
        // 270 -> 180 degrees
        PointF(x + w2 - w2k, y + h),
        PointF(x, y + h2 + h2k),
        PointF(x, y + h2),
        // 180 -> 90 degrees
        PointF(x, y + h2 - h2k),
        PointF(x + w2 - w2k, y),
        PointF(x + w2, y),
        // 90 -> 0 degrees
        PointF(x + w2 + w2k, y),
        PointF(x + w, y + h2 - h2k),
        PointF(x + w, y + h2)
    };
    if (sweepLength > 360) {
        sweepLength = 360;
    } else if (sweepLength < -360) {
        sweepLength = -360;
    }
    // Special case fast paths
    if (RealIsNull(startAngle)) {
        if (RealIsEqual(sweepLength, 360.0)) {
            for (int i = 11; i >= 0; --i) {
                curves[(*point_count)++] = points[i];
            }
            return points[12];
        } else if (RealIsEqual(sweepLength, -360.0)) {
            for (int i = 1; i <= 12; ++i) {
                curves[(*point_count)++] = points[i];
            }
            return points[0];
        }
    }
    int startSegment = int(std::floor(startAngle / 90));
    int endSegment = int(std::floor((startAngle + sweepLength) / 90));
    double startT = (startAngle - startSegment * 90) / 90;
    double endT = (startAngle + sweepLength - endSegment * 90) / 90;
    int delta = sweepLength > 0 ? 1 : -1;
    if (delta < 0) {
        startT = 1 - startT;
        endT = 1 - endT;
    }
    // avoid empty start segment
    if (RealIsNull(startT - double(1))) {
        startT = 0.0;
        startSegment += delta;
    }
    // avoid empty end segment
    if (RealIsNull(endT)) {
        endT = 1;
        endSegment -= delta;
    }
    startT = angleForArc(startT * 90);
    endT = angleForArc(endT * 90);
    const bool splitAtStart = !RealIsNull(startT);
    const bool splitAtEnd = !RealIsNull(endT - double(1));
    const int end = endSegment + delta;
    // empty arc?
    if (startSegment == end) {
        const int quadrant = 3 - ((startSegment % 4) + 4) % 4;
        const int j = 3 * quadrant;
        return delta > 0 ? points[j + 3] : points[j];
    }
    PointF startPoint, endPoint;
    findEllipseCoords(rect, startAngle, sweepLength, &startPoint, &endPoint);
    for (int i = startSegment; i != end; i += delta) {
        const int quadrant = 3 - ((i % 4) + 4) % 4;
        const int j = 3 * quadrant;
        Bezier b;
        if (delta > 0) {
            b = Bezier::fromPoints(points[j + 3], points[j + 2], points[j + 1], points[j]);
        } else {
            b = Bezier::fromPoints(points[j], points[j + 1], points[j + 2], points[j + 3]);
        }
        // empty arc?
        if (startSegment == endSegment && RealIsEqual(startT, endT)) {
            return startPoint;
        }
        if (i == startSegment) {
            if (i == endSegment && splitAtEnd) {
                b = b.bezierOnInterval(startT, endT);
            } else if (splitAtStart) {
                b = b.bezierOnInterval(startT, 1);
            }
        } else if (i == endSegment && splitAtEnd) {
            b = b.bezierOnInterval(0, endT);
        }
        // push control points
        curves[(*point_count)++] = b.pt2();
        curves[(*point_count)++] = b.pt3();
        curves[(*point_count)++] = b.pt4();
    }
    assert(*point_count > 0);
    curves[*(point_count) - 1] = endPoint;
    return startPoint;
}

static double angleForArc(double angle)
{
    if (RealIsNull(angle)) {
        return 0;
    }
    if (RealIsEqual(angle, double(90))) {
        return 1;
    }
    double radians = angle * (pi / 180);
    double cosAngle = std::cos(radians);
    double sinAngle = std::sin(radians);
    // initial guess
    double tc = angle / 90;
    // do some iterations of newton's method to approximate cosAngle
    // finds the zero of the function b.pointAt(tc).x() - cosAngle
    tc -= ((((2 - 3 * pathKappa) * tc + 3 * (pathKappa - 1)) * tc) * tc + 1 - cosAngle) // value
          / (((6 - 9 * pathKappa) * tc + 6 * (pathKappa - 1)) * tc); // derivative
    tc -= ((((2 - 3 * pathKappa) * tc + 3 * (pathKappa - 1)) * tc) * tc + 1 - cosAngle) // value
          / (((6 - 9 * pathKappa) * tc + 6 * (pathKappa - 1)) * tc); // derivative
    // initial guess
    double ts = tc;
    // do some iterations of newton's method to approximate sinAngle
    // finds the zero of the function b.pointAt(tc).y() - sinAngle
    ts -= ((((3 * pathKappa - 2) * ts - 6 * pathKappa + 3) * ts + 3 * pathKappa) * ts - sinAngle)
          / (((9 * pathKappa - 6) * ts + 12 * pathKappa - 6) * ts + 3 * pathKappa);
    ts -= ((((3 * pathKappa - 2) * ts - 6 * pathKappa + 3) * ts + 3 * pathKappa) * ts - sinAngle)
          / (((9 * pathKappa - 6) * ts + 12 * pathKappa - 6) * ts + 3 * pathKappa);
    // use the average of the t that best approximates cosAngle
    // and the t that best approximates sinAngle
    double t = 0.5 * (tc + ts);

    return t;
}

#define BEZIER_A(bezier, coord) 3 * (-bezier.m_##coord##1 \
                                     + 3 * bezier.m_##coord##2 \
                                     - 3 * bezier.m_##coord##3 \
                                     + bezier.m_##coord##4)
#define BEZIER_B(bezier, coord) 6 * (bezier.m_##coord##1 \
                                     - 2 * bezier.m_##coord##2 \
                                     + bezier.m_##coord##3)
#define BEZIER_C(bezier, coord) 3 * (-bezier.m_##coord##1 \
                                     + bezier.m_##coord##2)

#define BEZIER_CHECK_T(bezier, t) \
    if (t >= 0 && t <= 1) { \
        PointF p(b.pointAt(t)); \
        if (p.x() < minx) minx = p.x(); \
        else if (p.x() > maxx) maxx = p.x(); \
        if (p.y() < miny) miny = p.y(); \
        else if (p.y() > maxy) maxy = p.y(); \
    }

RectF PainterPath::painterpathBezierExtrema(const Bezier& b)
{
    double minx, miny, maxx, maxy;
    // initialize with end points
    if (b.m_x1 < b.m_x4) {
        minx = b.m_x1;
        maxx = b.m_x4;
    } else {
        minx = b.m_x4;
        maxx = b.m_x1;
    }
    if (b.m_y1 < b.m_y4) {
        miny = b.m_y1;
        maxy = b.m_y4;
    } else {
        miny = b.m_y4;
        maxy = b.m_y1;
    }
    // Update for the X extrema
    {
        double ax = BEZIER_A(b, x);
        double bx = BEZIER_B(b, x);
        double cx = BEZIER_C(b, x);
        // specialcase quadratic curves to avoid div by zero
        if (RealIsNull(ax)) {
            // linear curves are covered by initialization.
            if (!RealIsNull(bx)) {
                double t = -cx / bx;
                BEZIER_CHECK_T(b, t);
            }
        } else {
            const double tx = bx * bx - 4 * ax * cx;
            if (tx >= 0) {
                double temp = std::sqrt(tx);
                double rcp = 1 / (2 * ax);
                double t1 = (-bx + temp) * rcp;
                BEZIER_CHECK_T(b, t1);
                double t2 = (-bx - temp) * rcp;
                BEZIER_CHECK_T(b, t2);
            }
        }
    }
    // Update for the Y extrema
    {
        double ay = BEZIER_A(b, y);
        double by = BEZIER_B(b, y);
        double cy = BEZIER_C(b, y);
        // specialcase quadratic curves to avoid div by zero
        if (RealIsNull(ay)) {
            // linear curves are covered by initialization.
            if (!RealIsNull(by)) {
                double t = -cy / by;
                BEZIER_CHECK_T(b, t);
            }
        } else {
            const double ty = by * by - 4 * ay * cy;
            if (ty > 0) {
                double temp = std::sqrt(ty);
                double rcp = 1 / (2 * ay);
                double t1 = (-by + temp) * rcp;
                BEZIER_CHECK_T(b, t1);
                double t2 = (-by - temp) * rcp;
                BEZIER_CHECK_T(b, t2);
            }
        }
    }
    return RectF(minx, miny, maxx - minx, maxy - miny);
}

void PainterPath::setDirty()
{
    m_dirtyBounds = true;
    m_convex = false;
}
}
