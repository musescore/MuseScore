//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "drawcomp.h"

#include "realfn.h"
#include "log.h"

using namespace mu;
using namespace mu::draw;

namespace mu::draw::comp {
static const int DEFAULT_PREC(3);

template<class T>
static bool isEqual(const std::vector<T>& v1, const std::vector<T>& v2, DrawComp::Tolerance tolerance);

static bool isEqual(const qreal& v1, const qreal& v2, double tolerance)
{
    if (tolerance > 0) {
        double delta = RealFloor(v1, DEFAULT_PREC) - RealFloor(v2, DEFAULT_PREC);
        return std::fabs(delta) < tolerance;
    }
    return RealIsEqual(RealFloor(v1, DEFAULT_PREC), RealFloor(v2, DEFAULT_PREC));
}

static bool isEqual(const QPointF& p1, const QPointF& p2, double tolerance)
{
    if (!isEqual(p1.x(), p2.x(), tolerance)) {
        return false;
    }

    if (!isEqual(p1.y(), p2.y(), tolerance)) {
        return false;
    }

    return true;
}

static bool isEqual(const QRectF& r1, const QRectF& r2, double tolerance)
{
    if (!isEqual(r1.x(), r2.x(), tolerance)) {
        return false;
    }

    if (!isEqual(r1.y(), r2.y(), tolerance)) {
        return false;
    }

    if (!isEqual(r1.width(), r2.width(), tolerance)) {
        return false;
    }

    if (!isEqual(r1.height(), r2.height(), tolerance)) {
        return false;
    }

    return true;
}

static bool isEqual(const QSize& sz1, const QSize& sz2)
{
    if (sz1.width() != sz2.width()) {
        return false;
    }

    if (sz1.height() != sz2.height()) {
        return false;
    }

    return true;
}

static bool isEqual(const QPen& p1, const QPen& p2)
{
    if (p1.style() != p2.style()) {
        return false;
    }

    if (p1.width() != p2.width()) {
        return false;
    }

    if (p1.color().name() != p2.color().name()) {
        return false;
    }

    return true;
}

static bool isEqual(const QBrush& b1, const QBrush& b2)
{
    if (b1.style() != b2.style()) {
        return false;
    }

    if (b1.color().name() != b2.color().name()) {
        return false;
    }

    return true;
}

static bool isEqual(const QFont& f1, const QFont& f2)
{
    if (f1.family() != f2.family()) {
        return false;
    }

    if (f1.pointSize() != f2.pointSize()) {
        return false;
    }

    if (f1.weight() != f2.weight()) {
        return false;
    }

    if (f1.italic() != f2.italic()) {
        return false;
    }

    return true;
}

static bool isEqual(const QTransform& t1, const QTransform& t2, double tolerance)
{
    if (!isEqual(t1.m11(), t2.m11(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m12(), t2.m12(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m13(), t2.m13(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m21(), t2.m21(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m22(), t2.m22(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m23(), t2.m23(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m31(), t2.m31(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m32(), t2.m32(), tolerance)) {
        return false;
    }

    if (!isEqual(t1.m33(), t2.m33(), tolerance)) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawData::State& s1, const DrawData::State& s2, DrawComp::Tolerance tolerance)
{
    if (s1.isAntialiasing != s2.isAntialiasing) {
        return false;
    }

    if (s1.compositionMode != s2.compositionMode) {
        return false;
    }

    if (!isEqual(s1.pen, s2.pen)) {
        return false;
    }

    if (!isEqual(s1.brush, s2.brush)) {
        return false;
    }

    if (!isEqual(s1.font, s2.font)) {
        return false;
    }

    if (!isEqual(s1.transform, s2.transform, tolerance.base)) {
        return false;
    }

    return true;
}

static bool isEqual(const QPainterPath& v1, const QPainterPath& v2, double tolerance)
{
    if (v1.elementCount() != v2.elementCount()) {
        return false;
    }

    for (int i = 0; i < v1.elementCount(); ++i) {
        QPainterPath::Element e1 = v1.elementAt(i);
        QPainterPath::Element e2 = v2.elementAt(i);

        if (e1.type != e2.type) {
            return false;
        }

        if (!isEqual(e1.x, e2.x, tolerance)) {
            return false;
        }

        if (!isEqual(e1.y, e2.y, tolerance)) {
            return false;
        }
    }

    return true;
}

static bool isEqual(const DrawPath& v1, const DrawPath& v2, DrawComp::Tolerance tolerance)
{
    if (v1.mode != v2.mode) {
        return false;
    }

    if (!isEqual(v1.path, v2.path, tolerance.base)) {
        return false;
    }

    if (!isEqual(v1.pen, v2.pen)) {
        return false;
    }

    if (!isEqual(v1.brush, v2.brush)) {
        return false;
    }

    return true;
}

static bool isEqual(const QPolygonF& v1, const QPolygonF& v2, double tolerance)
{
    if (v1.size() != v2.size()) {
        return false;
    }

    for (int i = 0; i < v1.size(); ++i) {
        const QPointF& p1 = v1.at(i);
        const QPointF& p2 = v2.at(i);

        if (!isEqual(p1, p2, tolerance)) {
            return false;
        }
    }

    return true;
}

static bool isEqual(const DrawPolygon& v1, const DrawPolygon& v2, DrawComp::Tolerance tolerance)
{
    if (v1.mode != v2.mode) {
        return false;
    }

    if (!isEqual(v1.polygon, v2.polygon, tolerance.base)) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawText& v1, const DrawText& v2, DrawComp::Tolerance tolerance)
{
    if (!isEqual(v1.pos, v2.pos, tolerance.base)) {
        return false;
    }

    if (v1.text != v2.text) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawRectText& v1, const DrawRectText& v2, DrawComp::Tolerance tolerance)
{
    if (v1.flags != v2.flags) {
        return false;
    }

    if (!isEqual(v1.rect, v2.rect, tolerance.base)) {
        return false;
    }

    if (v1.text != v2.text) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawPixmap& v1, const DrawPixmap& v2, DrawComp::Tolerance tolerance)
{
    if (!isEqual(v1.pos, v2.pos, tolerance.base)) {
        return false;
    }

    if (!isEqual(v1.pm.size(), v2.pm.size())) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawTiledPixmap& v1, const DrawTiledPixmap& v2, DrawComp::Tolerance tolerance)
{
    if (!isEqual(v1.rect, v2.rect, tolerance.base)) {
        return false;
    }

    if (!isEqual(v1.pm.size(), v2.pm.size())) {
        return false;
    }

    if (!isEqual(v1.offset, v2.offset, tolerance.base)) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawData::Data& d1, const DrawData::Data& d2, DrawComp::Tolerance tolerance)
{
    if (!isEqual(d1.state, d2.state, tolerance)) {
        return false;
    }

    if (!isEqual(d1.paths, d2.paths, tolerance)) {
        return false;
    }

    if (!isEqual(d1.polygons, d2.polygons, tolerance)) {
        return false;
    }

    if (!isEqual(d1.texts, d2.texts, tolerance)) {
        return false;
    }

    if (!isEqual(d1.rectTexts, d2.rectTexts, tolerance)) {
        return false;
    }

    if (!isEqual(d1.pixmaps, d2.pixmaps, tolerance)) {
        return false;
    }

    if (!isEqual(d1.tiledPixmap, d2.tiledPixmap, tolerance)) {
        return false;
    }

    return true;
}

template<class T>
static bool isEqual(const std::vector<T>& v1, const std::vector<T>& v2, DrawComp::Tolerance tolerance)
{
    if (v1.size() != v2.size()) {
        return false;
    }

    for (size_t i = 0; i < v1.size(); ++i) {
        const T& vi1 = v1[i];
        const T& vi2 = v2[i];

        if (isEqual(vi1, vi2, tolerance)) {
            continue;
        } else {
            bool found = false;
            for (size_t j = 0; j < v2.size(); ++j) {
                const T& vj2 = v2[j];

                if (isEqual(vi1, vj2, tolerance)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }
    }
    return true;
}

static bool isEqual(const DrawData::Object& o1, const DrawData::Object& o2, DrawComp::Tolerance tolerance)
{
    if (o1.name != o2.name) {
        return false;
    }

    if (!isEqual(o1.pagePos, o2.pagePos, tolerance.base)) {
        return false;
    }

    if (!isEqual(o1.datas, o2.datas, tolerance)) {
        return false;
    }

    return true;
}

template<class T>
static bool contains(const std::vector<T>& v1, const T& val, DrawComp::Tolerance tolerance)
{
    for (size_t i = 0; i < v1.size(); ++i) {
        const T& t = v1.at(i);
        if (isEqual(t, val, tolerance)) {
            return true;
        }
    }
    return false;
}

template<class T>
static void difference(std::vector<T>& diff, const std::vector<T>& v1, const std::vector<T>& v2, DrawComp::Tolerance tolerance)
{
    for (size_t i = 0; i < v1.size(); ++i) {
        const T& t = v1.at(i);
        if (!contains(v2, t, tolerance)) {
            diff.push_back(t);
        }
    }
}
} // mu::draw::comp

Diff DrawComp::compare(const DrawDataPtr& data, const DrawDataPtr& origin, Tolerance tolerance)
{
    Diff diff;
    IF_ASSERT_FAILED(data) {
        return diff;
    }

    IF_ASSERT_FAILED(origin) {
        return diff;
    }

    diff.dataAdded = std::make_shared<DrawData>();
    diff.dataAdded->name = data->name;

    diff.dataRemoved = std::make_shared<DrawData>();
    diff.dataRemoved->name = data->name;

    comp::difference(diff.dataRemoved->objects, data->objects, origin->objects, tolerance);
    comp::difference(diff.dataAdded->objects, origin->objects, data->objects, tolerance);

    return diff;
}
