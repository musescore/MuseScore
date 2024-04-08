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
#include "drawdatacomp.h"

#include <list>

#include "global/realfn.h"
#include "global/containers.h"

#include "log.h"

using namespace muse::draw;

namespace muse::draw::comp {
static const int DEFAULT_PREC(3);

struct Polygon;
static bool isEqual(const Polygon& p1, const Polygon& p2, DrawDataComp::Tolerance tolerance);

struct Path {
    const DrawData::Item* obj = nullptr;
    const DrawData::Data* data = nullptr;
    const DrawPath* path = nullptr;
};

struct Polygon {
    const DrawData::Item* obj = nullptr;
    const DrawData::Data* data = nullptr;
    const DrawPolygon* polygon = nullptr;
    bool operator==(const Polygon& o) const { return isEqual(*this, o, DrawDataComp::Tolerance()); }
};

struct Text {
    const DrawData::Item* obj = nullptr;
    const DrawData::Data* data = nullptr;
    const DrawText* text = nullptr;
};

struct Pixmap {
    const DrawData::Item* obj = nullptr;
    const DrawData::Data* data = nullptr;
    const DrawPixmap* pixmap = nullptr;
};

struct Data {
    std::list<Path> paths;
    std::list<Polygon> polygons;
    std::list<Text> texts;
    std::list<Pixmap> pixmaps;
};

template<class T>
static bool isEqual(const std::vector<T>& v1, const std::vector<T>& v2, DrawDataComp::Tolerance tolerance);

static bool isEqual(const double& v1, const double& v2, double tolerance)
{
    if (tolerance > 0) {
        double delta = RealFloor(v1, DEFAULT_PREC) - RealFloor(v2, DEFAULT_PREC);
        return std::fabs(delta) < tolerance;
    }
    return RealIsEqual(RealFloor(v1, DEFAULT_PREC), RealFloor(v2, DEFAULT_PREC));
}

static bool isEqual(const PointF& p1, const PointF& p2, double tolerance)
{
    if (!isEqual(p1.x(), p2.x(), tolerance)) {
        return false;
    }

    if (!isEqual(p1.y(), p2.y(), tolerance)) {
        return false;
    }

    return true;
}

static bool isEqual(const RectF& r1, const RectF& r2, double tolerance)
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

static bool isEqual(const Size& sz1, const Size& sz2)
{
    if (sz1.width() != sz2.width()) {
        return false;
    }

    if (sz1.height() != sz2.height()) {
        return false;
    }

    return true;
}

static bool isEqual(const Pen& p1, const Pen& p2)
{
    if (p1.style() != p2.style()) {
        return false;
    }

    if (!RealIsEqual(p1.widthF(), p2.widthF())) {
        return false;
    }

    if (p1.color() != p2.color()) {
        return false;
    }

    return true;
}

static bool isEqual(const Brush& b1, const Brush& b2)
{
    if (b1.style() != b2.style()) {
        return false;
    }

    if (b1.color() != b2.color()) {
        return false;
    }

    return true;
}

#if 0 // currently unused, disabled due to an MSVC compiler warning C4505
static bool isEqual(const Font& f1, const Font& f2)
{
    if (f1.family() != f2.family()) {
        return false;
    }

    if (!RealIsEqual(f1.pointSizeF(), f2.pointSizeF())) {
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

static bool isEqual(const Transform& t1, const Transform& t2, double tolerance)
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

static bool isEqual(const DrawData::State& s1, const DrawData::State& s2, DrawDataComp::Tolerance tolerance)
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

#endif

static bool isEqual(const PainterPath& v1, const PainterPath& v2, double tolerance)
{
    if (v1.elementCount() != v2.elementCount()) {
        return false;
    }

    for (size_t i = 0; i < v1.elementCount(); ++i) {
        PainterPath::Element e1 = v1.elementAt(i);
        PainterPath::Element e2 = v2.elementAt(i);

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

static bool isEqual(const DrawPath& v1, const DrawPath& v2, DrawDataComp::Tolerance tolerance)
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

static bool isEqual(const PolygonF& v1, const PolygonF& v2, double tolerance)
{
    if (v1.size() != v2.size()) {
        return false;
    }

    for (size_t i = 0; i < v1.size(); ++i) {
        const PointF& p1 = v1.at(i);
        const PointF& p2 = v2.at(i);

        if (!isEqual(p1, p2, tolerance)) {
            return false;
        }
    }

    return true;
}

static bool isEqual(const DrawPolygon& v1, const DrawPolygon& v2, DrawDataComp::Tolerance tolerance)
{
    if (v1.mode != v2.mode) {
        return false;
    }

    if (!isEqual(v1.polygon, v2.polygon, tolerance.base)) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawText& v1, const DrawText& v2, DrawDataComp::Tolerance tolerance)
{
    if (v1.mode != v2.mode) {
        return false;
    }

    if (!isEqual(v1.rect, v2.rect, tolerance.base)) {
        return false;
    }

    if (v1.flags != v2.flags) {
        return false;
    }

    if (v1.text != v2.text) {
        return false;
    }

    return true;
}

static bool isEqual(const DrawPixmap& v1, const DrawPixmap& v2, DrawDataComp::Tolerance tolerance)
{
    if (v1.mode != v2.mode) {
        return false;
    }

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

#if 0 // currently unused, disabled due to an MSVC compiler warning C4505
static bool isEqual(const DrawData::Data& d1, const DrawData::Data& d2, DrawDataComp::Tolerance tolerance)
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

    if (!isEqual(d1.pixmaps, d2.pixmaps, tolerance)) {
        return false;
    }

    return true;
}

#endif

template<class T>
static bool isEqual(const std::vector<T>& v1, const std::vector<T>& v2, DrawDataComp::Tolerance tolerance)
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

#if 0 // currently unused, disabled due to an MSVC compiler warning C4505
static bool isEqual(const DrawData::Object& o1, const DrawData::Object& o2, DrawDataComp::Tolerance tolerance)
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

#endif

static bool isEqual(const Path& p1, const Path& p2, DrawDataComp::Tolerance tolerance)
{
    if (p1.obj->name != p2.obj->name) {
        return false;
    }

    if (p1.data->state != p2.data->state) {
        return false;
    }

    return isEqual(*p1.path, *p2.path, tolerance);
}

static bool isEqual(const Polygon& p1, const Polygon& p2, DrawDataComp::Tolerance tolerance)
{
    if (p1.obj->name != p2.obj->name) {
        return false;
    }

    if (p1.data->state != p2.data->state) {
        return false;
    }

    return isEqual(*p1.polygon, *p2.polygon, tolerance);
}

static bool isEqual(const Text& p1, const Text& p2, DrawDataComp::Tolerance tolerance)
{
    if (p1.obj->name != p2.obj->name) {
        return false;
    }

    if (p1.data->state != p2.data->state) {
        return false;
    }

    return isEqual(*p1.text, *p2.text, tolerance);
}

static bool isEqual(const comp::Pixmap& p1, const comp::Pixmap& p2, DrawDataComp::Tolerance tolerance)
{
    if (p1.obj->name != p2.obj->name) {
        return false;
    }

    if (p1.data->state != p2.data->state) {
        return false;
    }

    return isEqual(*p1.pixmap, *p2.pixmap, tolerance);
}

template<class T>
static bool contains(const std::vector<T>& v1, const T& val, DrawDataComp::Tolerance tolerance)
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
static void difference(std::vector<T>& diff, const std::vector<T>& v1, const std::vector<T>& v2, DrawDataComp::Tolerance tolerance)
{
    for (size_t i = 0; i < v1.size(); ++i) {
        const T& t = v1.at(i);
        if (!contains(v2, t, tolerance)) {
            diff.push_back(t);
        }
    }
}

template<class T>
static bool contains(const std::list<T>& v1, const T& val, DrawDataComp::Tolerance tolerance)
{
    for (const T& t : v1) {
        if (isEqual(t, val, tolerance)) {
            return true;
        }
    }
    return false;
}

template<class T>
static void difference(std::list<T>& diff, const std::list<T>& v1, const std::list<T>& v2, DrawDataComp::Tolerance tolerance)
{
    for (const T& t : v1) {
        if (!contains(v2, t, tolerance)) {
            diff.push_back(t);
        }
    }
}

static void difference(Data& diff, const Data& d1, const Data& d2, DrawDataComp::Tolerance tolerance)
{
    difference(diff.paths, d1.paths, d2.paths, tolerance);
    difference(diff.polygons, d1.polygons, d2.polygons, tolerance);
    difference(diff.texts, d1.texts, d2.texts, tolerance);
    difference(diff.pixmaps, d1.pixmaps, d2.pixmaps, tolerance);
}

static void toCompData(Data& cd, const DrawData::Item& item)
{
    for (const DrawData::Data& d : item.datas) {
        for (const DrawPath& p : d.paths) {
            cd.paths.push_back(comp::Path { &item, &d, &p });
        }
        for (const DrawPolygon& p : d.polygons) {
            cd.polygons.push_back(comp::Polygon { &item, &d, &p });
        }
        for (const DrawText& p : d.texts) {
            cd.texts.push_back(comp::Text { &item, &d, &p });
        }
        for (const DrawPixmap& p : d.pixmaps) {
            cd.pixmaps.push_back(comp::Pixmap { &item, &d, &p });
        }
    }

    for (const DrawData::Item& ch : item.chilren) {
        toCompData(cd, ch);
    }
}

static Data toCompData(const DrawDataPtr& dd)
{
    Data cd;
    toCompData(cd, dd->item);
    return cd;
}

static void fillDrawData(DrawDataPtr& dd, const comp::Data& cd)
{
    //! NOTE Not effective, but efficiency is not required yet

    // collect objects (save order)
    std::vector<const DrawData::Item*> objs;
    for (const comp::Path& p : cd.paths) {
        if (!muse::contains(objs, p.obj)) {
            objs.push_back(p.obj);
        }
    }
    for (const comp::Polygon& p : cd.polygons) {
        if (!muse::contains(objs, p.obj)) {
            objs.push_back(p.obj);
        }
    }
    for (const comp::Text& p : cd.texts) {
        if (!muse::contains(objs, p.obj)) {
            objs.push_back(p.obj);
        }
    }
    for (const comp::Pixmap& p : cd.pixmaps) {
        if (!muse::contains(objs, p.obj)) {
            objs.push_back(p.obj);
        }
    }

    // collect data
    for (const DrawData::Item* o : objs) {
        DrawData::Item dobj;
        dobj.name = o->name;

        auto findOrCreateDData = [](DrawData::Item& dobj, int state) {
            DrawData::Data* ddata = nullptr;

            // find by state
            for (DrawData::Data& od : dobj.datas) {
                if (od.state == state) {
                    ddata = &od;
                }
            }

            // add new, if not find
            if (!ddata) {
                dobj.datas.push_back(DrawData::Data());
                ddata = &dobj.datas.back();
                ddata->state = state;
            }

            return ddata;
        };

        for (const comp::Path& p : cd.paths) {
            DrawData::Data* ddata = findOrCreateDData(dobj, p.data->state);
            ddata->paths.push_back(*p.path);
        }

        for (const comp::Polygon& p : cd.polygons) {
            DrawData::Data* ddata = findOrCreateDData(dobj, p.data->state);
            ddata->polygons.push_back(*p.polygon);
        }

        for (const comp::Text& p : cd.texts) {
            DrawData::Data* ddata = findOrCreateDData(dobj, p.data->state);
            ddata->texts.push_back(*p.text);
        }

        for (const comp::Pixmap& p : cd.pixmaps) {
            DrawData::Data* ddata = findOrCreateDData(dobj, p.data->state);
            ddata->pixmaps.push_back(*p.pixmap);
        }

        dd->item.chilren.push_back(dobj);
    }
}
} // muse::draw::comp

Diff DrawDataComp::compare(const DrawDataPtr& data, const DrawDataPtr& origin, Tolerance tolerance)
{
    Diff diff;
    IF_ASSERT_FAILED(data) {
        return diff;
    }

    IF_ASSERT_FAILED(origin) {
        return diff;
    }

    comp::Data cdata = comp::toCompData(data);
    comp::Data corigin = comp::toCompData(origin);

    comp::Data added;
    comp::difference(added, corigin, cdata, tolerance);

    comp::Data removed;
    comp::difference(removed, cdata, corigin, tolerance);

    diff.dataAdded = std::make_shared<DrawData>();
    diff.dataAdded->name = origin->name;
    diff.dataAdded->viewport = origin->viewport;
    diff.dataAdded->states = origin->states;
    fillDrawData(diff.dataAdded, added);

    diff.dataRemoved = std::make_shared<DrawData>();
    diff.dataRemoved->name = origin->name;
    diff.dataRemoved->viewport = origin->viewport;
    diff.dataRemoved->states = origin->states;
    fillDrawData(diff.dataRemoved, removed);

    return diff;
}
