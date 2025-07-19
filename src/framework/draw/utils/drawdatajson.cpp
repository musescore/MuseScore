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
#include "drawdatajson.h"

#include "global/serialization/json.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

static int rtoi(double v)
{
    return static_cast<int>(v * 1000.0);
}

static double itor(int v)
{
    return static_cast<double>(v) / 1000.0;
}

static JsonArray toArr(const std::vector<double>& vv)
{
    JsonArray a;
    for (double v : vv) {
        a.append(rtoi(v));
    }
    return a;
}

static void fromArr(const JsonArray& arr, std::vector<double>& vv)
{
    for (size_t i = 0; i < arr.size(); ++i) {
        vv.push_back(itor(arr.at(i).toDouble()));
    }
}

static JsonObject toObj(const Pen& pen)
{
    JsonObject obj;
    obj["style"] = static_cast<int>(pen.style());
    obj["capStyle"] = static_cast<int>(pen.capStyle());
    obj["joinStyle"] = static_cast<int>(pen.joinStyle());
    obj["color"] = pen.color().toString();
    obj["width"] = pen.widthF();
    obj["dashPattern"] = toArr(pen.dashPattern());
    return obj;
}

static void fromObj(const JsonObject& obj, Pen& pen)
{
    pen.setStyle(static_cast<PenStyle>(obj.value("style").toInt()));
    pen.setCapStyle(static_cast<PenCapStyle>(obj.value("capStyle").toInt()));
    pen.setJoinStyle(static_cast<PenJoinStyle>(obj.value("joinStyle").toInt()));
    pen.setColor(Color(obj.value("color").toStdString().c_str()));
    pen.setWidthF(obj.value("width").toDouble());

    std::vector<double> dp;
    fromArr(obj.value("dashPattern").toArray(), dp);
    pen.setDashPattern(dp);
}

static JsonObject toObj(const Brush& brush)
{
    JsonObject obj;
    obj["style"] = static_cast<int>(brush.style());
    obj["color"] = brush.color().toString();
    return obj;
}

static void fromObj(const JsonObject& obj, Brush& brush)
{
    brush.setStyle(static_cast<BrushStyle>(obj["style"].toInt()));
    brush.setColor(Color(obj["color"].toStdString().c_str()));
}

static JsonObject toObj(const Font& font)
{
    JsonObject obj;
    obj["family"] = font.family().id();
    obj["type"] = static_cast<int>(font.type());
    obj["pointSize"] = font.pointSizeF();
    obj["weight"] = font.weight();
    obj["italic"] = font.italic();
    obj["hinting"] = static_cast<int>(font.hinting());
    obj["no_merging"] = font.noFontMerging();
    return obj;
}

static void fromObj(const JsonObject& obj, Font& font)
{
    font.setFamily(obj.value("family").toString(), static_cast<Font::Type>(obj.value("type").toInt()));
    font.setPointSizeF(obj.value("pointSize").toDouble());
    font.setWeight(static_cast<Font::Weight>(obj.value("weight").toInt()));
    font.setItalic(obj.value("italic").toBool());
    font.setHinting(static_cast<Font::Hinting>(obj.value("hinting").toInt()));
    font.setNoFontMerging(obj.value("no_merging").toBool());
}

static JsonArray toArr(const Transform& t)
{
    return JsonArray({ rtoi(t.m11()), rtoi(t.m12()), rtoi(t.m13()),
                       rtoi(t.m21()), rtoi(t.m22()), rtoi(t.m23()),
                       rtoi(t.m31()), rtoi(t.m32()), rtoi(t.m33()) });
}

static void fromArr(const JsonArray& arr, Transform& t)
{
    IF_ASSERT_FAILED(arr.size() == 9) {
        return;
    }

    t.setMatrix(itor(arr.at(0).toInt()), itor(arr.at(1).toInt()), itor(arr.at(2).toInt()),
                itor(arr.at(3).toInt()), itor(arr.at(4).toInt()), itor(arr.at(5).toInt()),
                itor(arr.at(6).toInt()), itor(arr.at(7).toInt()), itor(arr.at(8).toInt()));
}

static JsonArray toArr(const PointF& p)
{
    return JsonArray({ rtoi(p.x()), rtoi(p.y()) });
}

static void fromArr(const JsonArray& arr, PointF& p)
{
    IF_ASSERT_FAILED(arr.size() == 2) {
        return;
    }
    p.setX(itor(arr.at(0).toInt()));
    p.setY(itor(arr.at(1).toInt()));
}

static JsonArray toArr(const RectF& r)
{
    return JsonArray({ rtoi(r.x()), rtoi(r.y()), rtoi(r.width()), rtoi(r.height()) });
}

static void fromArr(const JsonArray& arr, RectF& r)
{
    IF_ASSERT_FAILED(arr.size() == 4) {
        return;
    }
    r = RectF(itor(arr.at(0).toInt()), itor(arr.at(1).toInt()), itor(arr.at(2).toInt()), itor(arr.at(3).toInt()));
}

static JsonArray toArr(const Size& sz)
{
    return JsonArray({ sz.width(), sz.height() });
}

static void fromArr(const JsonArray& arr, Size& sz)
{
    IF_ASSERT_FAILED(arr.size() == 2) {
        return;
    }
    sz = Size(arr.at(0).toInt(), arr.at(1).toInt());
}

static JsonObject toObj(const DrawData::State& st)
{
    JsonObject obj;
    obj["pen"] = toObj(st.pen);
    obj["brush"] = toObj(st.brush);
    obj["font"] = toObj(st.font);
    obj["isAntialiasing"] = st.isAntialiasing;
    obj["transform"] = toArr(st.transform);
    obj["compositionMode"] = static_cast<int>(st.compositionMode);
    return obj;
}

static void fromObj(const JsonObject& obj, DrawData::State& st)
{
    fromObj(obj["pen"].toObject(), st.pen);
    fromObj(obj["brush"].toObject(), st.brush);
    fromObj(obj["font"].toObject(), st.font);
    st.isAntialiasing = obj["isAntialiasing"].toBool();
    fromArr(obj["transform"].toArray(), st.transform);
    st.compositionMode = static_cast<CompositionMode>(obj["compositionMode"].toInt());
}

static JsonObject toObj(const PainterPath& path)
{
    JsonObject obj;
    obj["fillRule"] = static_cast<int>(path.fillRule());

    JsonArray elsArr;
    for (size_t i = 0; i < path.elementCount(); ++i) {
        PainterPath::Element e = path.elementAt(i);
        elsArr.append(JsonArray({ static_cast<int>(e.type), rtoi(e.x), rtoi(e.y) }));
    }
    obj["elements"] = elsArr;
    return obj;
}

static JsonObject toObj(const DrawPath& path)
{
    JsonObject obj;
    obj["path"] = toObj(path.path);
    obj["pen"] = toObj(path.pen);
    obj["brush"] = toObj(path.brush);
    obj["mode"] = static_cast<int>(path.mode);
    return obj;
}

static void fromObj(const JsonObject& obj, PainterPath& path)
{
    path.setFillRule(static_cast<PainterPath::FillRule>(obj["fillRule"].toInt()));

    JsonArray elsArr = obj["elements"].toArray();
    std::vector<PainterPath::Element> curveEls;
    for (size_t i = 0; i < elsArr.size(); ++i) {
        JsonArray elArr = elsArr.at(i).toArray();
        IF_ASSERT_FAILED(elArr.size() == 3) {
            continue;
        }

        PainterPath::ElementType type = static_cast<PainterPath::ElementType>(elArr.at(0).toInt());
        double x = itor(elArr.at(1).toInt());
        double y = itor(elArr.at(2).toInt());

        switch (type) {
        case PainterPath::ElementType::MoveToElement: {
            path.moveTo(x, y);
        } break;
        case PainterPath::ElementType::LineToElement: {
            path.lineTo(x, y);
        } break;
        case PainterPath::ElementType::CurveToElement: {
            IF_ASSERT_FAILED(curveEls.empty()) {
                continue;
            }
            PainterPath::Element e(x, y, type);
            curveEls.push_back(std::move(e));
        } break;
        case PainterPath::ElementType::CurveToDataElement: {
            if (curveEls.size() == 1) { // only CurveToElement
                PainterPath::Element e(x, y, type);
                curveEls.push_back(std::move(e));
                continue;
            }

            IF_ASSERT_FAILED(curveEls.size() == 2) { // must be CurveToElement and one CurveToDataElement
                curveEls.clear();
                continue;
            }

            path.cubicTo(curveEls.at(0).x, curveEls.at(0).y, curveEls.at(1).x, curveEls.at(1).y, x, y);
            curveEls.clear();
        } break;
        }
    }
}

static void fromObj(const JsonObject& obj, DrawPath& path)
{
    fromObj(obj["path"].toObject(), path.path);
    fromObj(obj["pen"].toObject(), path.pen);
    fromObj(obj["brush"].toObject(), path.brush);
    path.mode = static_cast<DrawMode>(obj["mode"].toInt());
}

static JsonArray toArr(const PolygonF& pol)
{
    JsonArray a;
    for (const PointF& p : pol) {
        a.append(toArr(p));
    }
    return a;
}

static void fromArr(const JsonArray& arr, PolygonF& pol)
{
    pol.reserve(arr.size());
    for (size_t i = 0; i < arr.size(); ++i) {
        PointF p;
        fromArr(arr.at(i).toArray(), p);
        pol.push_back(std::move(p));
    }
}

static JsonObject toObj(const DrawPolygon& pol)
{
    JsonObject obj;
    obj["polygon"] = toArr(pol.polygon);
    obj["mode"] = static_cast<int>(pol.mode);
    return obj;
}

static void fromObj(const JsonObject& obj, DrawPolygon& pol)
{
    fromArr(obj["polygon"].toArray(), pol.polygon);
    pol.mode = static_cast<PolygonMode>(obj["mode"].toInt());
}

static JsonObject toObj(const DrawText& text)
{
    JsonObject o;
    if (text.mode == DrawText::Point) {
        o["point"] = toArr(text.rect.topLeft());
    } else {
        o["rect"] = toArr(text.rect);
    }
    o["flags"] = text.flags;
    o["text"] = text.text;
    return o;
}

static void fromObj(const JsonObject& obj, DrawText& text)
{
    if (obj.contains("point")) {
        PointF point;
        fromArr(obj["point"].toArray(), point);
        text.mode = DrawText::Point;
        text.rect = RectF(point, SizeF());
    } else {
        fromArr(obj["rect"].toArray(), text.rect);
        text.mode = DrawText::Rect;
    }
    text.flags = obj["flags"].toInt();
    text.text = obj["text"].toString();
}

static JsonObject toObj(const DrawPixmap& pm)
{
    JsonObject o;
    if (pm.mode == DrawPixmap::Single) {
        o["point"] = toArr(pm.rect.topLeft());
    } else {
        o["rect"] = toArr(pm.rect);
        o["offset"] = toArr(pm.offset);
    }
    o["pmSize"] = toArr(pm.pm.size());
    return o;
}

static void fromObj(const JsonObject& obj, DrawPixmap& pm)
{
    if (obj.contains("point")) {
        pm.mode = DrawPixmap::Single;
        PointF point;
        fromArr(obj["point"].toArray(), point);
        pm.rect = RectF(point, SizeF());
    } else {
        pm.mode = DrawPixmap::Tiled;
        fromArr(obj["rect"].toArray(), pm.rect);
        fromArr(obj["offset"].toArray(), pm.offset);
    }

    Size size;
    fromArr(obj["pmSize"].toArray(), size);
    pm.pm = Pixmap(size);
}

template<class T>
static JsonArray toArr(const std::vector<T>& vals)
{
    JsonArray arr;
    for (size_t i = 0; i < vals.size(); ++i) {
        arr << toObj(vals.at(i));
    }
    return arr;
}

template<class T>
static void fromArr(const JsonArray& arr, std::vector<T>& vals)
{
    vals.reserve(arr.size());
    for (size_t i = 0; i < arr.size(); ++i) {
        T val;
        fromObj(arr.at(i).toObject(), val);
        vals.push_back(std::move(val));
    }
}

static JsonObject toObj(const DrawData::Item& obj)
{
    //! NOTE 'a' added to the beginning of some field names for convenient sorting

    JsonObject objObj;
    objObj["a_name"] = obj.name;

    JsonArray childrenArr;
    for (const DrawData::Item& ch : obj.chilren) {
        childrenArr.append(toObj(ch));
    }
    objObj["children"] = childrenArr;

    JsonArray datasArr;
    for (const DrawData::Data& data : obj.datas) {
        if (data.empty()) {
            continue;
        }

        JsonObject dataObj;
        dataObj["state"] = data.state;
        if (!data.paths.empty()) {
            dataObj["paths"] = toArr(data.paths);
        }
        if (!data.polygons.empty()) {
            dataObj["polygons"] = toArr(data.polygons);
        }
        if (!data.texts.empty()) {
            dataObj["texts"] = toArr(data.texts);
        }
        if (!data.pixmaps.empty()) {
            dataObj["pixmaps"] = toArr(data.pixmaps);
        }

        datasArr.append(dataObj);
    }
    objObj["datas"] = datasArr;

    return objObj;
}

static void fromObj(const JsonObject& objObj, DrawData::Item& obj)
{
    obj.name = objObj.value("a_name").toString().toStdString();
    JsonArray childrenArr = objObj["children"].toArray();
    for (size_t j = 0; j < childrenArr.size(); ++j) {
        DrawData::Item& ch = obj.chilren.emplace_back();
        const JsonObject childObj = childrenArr.at(j).toObject();
        fromObj(childObj, ch);
    }

    JsonArray datasArr = objObj["datas"].toArray();
    for (size_t j = 0; j < datasArr.size(); ++j) {
        const JsonObject dataObj = datasArr.at(j).toObject();
        DrawData::Data data;
        data.state = dataObj["state"].toInt();
        if (dataObj.contains("paths")) {
            fromArr(dataObj.value("paths").toArray(), data.paths);
        }
        if (dataObj.contains("polygons")) {
            fromArr(dataObj.value("polygons").toArray(), data.polygons);
        }
        if (dataObj.contains("texts")) {
            fromArr(dataObj.value("texts").toArray(), data.texts);
        }
        if (dataObj.contains("pixmaps")) {
            fromArr(dataObj.value("pixmaps").toArray(), data.pixmaps);
        }

        obj.datas.push_back(std::move(data));
    }
}

void DrawDataJson::toJson(JsonObject& root, const DrawDataPtr& dd)
{
    //! NOTE 'a' added to the beginning of some field names for convenient sorting

    root["a_name"] = dd->name;
    root["a_viewport"] = toArr(dd->viewport);

    // item
    root["item"] = toObj(dd->item);

    // states
    JsonObject stateObj;
    for (auto it = dd->states.cbegin(); it != dd->states.cend(); ++it) {
        stateObj[std::to_string(it->first)] = toObj(it->second);
    }
    root["states"] = stateObj;
}

void DrawDataJson::fromJson(const JsonObject& root, DrawDataPtr& dd)
{
    // read states
    {
        const JsonObject obj = root.value("states").toObject();
        std::vector<std::string> keys = obj.keys();
        for (const std::string& k : keys) {
            DrawData::State state;
            fromObj(obj.value(k).toObject(), state);
            dd->states[std::stoi(k)] = state;
        }
    }

    dd->name = root.value("a_name").toStdString();
    fromArr(root.value("a_viewport").toArray(), dd->viewport);

    JsonObject itemObj = root.value("item").toObject();
    fromObj(itemObj, dd->item);
}

ByteArray DrawDataJson::toJson(const DrawDataPtr& data, bool prettify)
{
    IF_ASSERT_FAILED(data) {
        return ByteArray();
    }

    JsonObject root;
    toJson(root, data);
    return JsonDocument(root).toJson(prettify ? JsonDocument::Format::Indented : JsonDocument::Format::Compact);
}

RetVal<DrawDataPtr> DrawDataJson::fromJson(const ByteArray& json)
{
    std::string err;
    JsonDocument doc = JsonDocument::fromJson(json, &err);
    if (!err.empty()) {
        RetVal<DrawDataPtr> rv;
        rv.ret = make_ret(Ret::Code::UnknownError, err);
        return rv;
    }

    const JsonObject root = doc.rootObject();
    DrawDataPtr dd = std::make_shared<DrawData>();
    fromJson(root, dd);
    return RetVal<DrawDataPtr>::make_ok(dd);
}

ByteArray DrawDataJson::diffToJson(const Diff& diff)
{
    IF_ASSERT_FAILED(diff.dataAdded && diff.dataRemoved) {
        return ByteArray();
    }

    JsonObject added;
    toJson(added, diff.dataAdded);

    JsonObject removed;
    toJson(removed, diff.dataRemoved);

    JsonObject root;
    root["type"] = "diff";
    root["added"] = added;
    root["removed"] = removed;
    return JsonDocument(root).toJson(JsonDocument::Format::Indented);
}

RetVal<Diff> DrawDataJson::diffFromJson(const ByteArray& json)
{
    std::string err;
    JsonDocument doc = JsonDocument::fromJson(json, &err);
    if (!err.empty()) {
        RetVal<Diff> rv;
        rv.ret = make_ret(Ret::Code::UnknownError, err);
        return rv;
    }

    const JsonObject root = doc.rootObject();
    IF_ASSERT_FAILED(root.value("type").toStdString() == "diff") {
        RetVal<Diff> rv;
        rv.ret = make_ret(Ret::Code::UnknownError, err);
        return rv;
    }

    Diff diff;
    diff.dataAdded = std::make_shared<DrawData>();
    diff.dataRemoved = std::make_shared<DrawData>();

    JsonObject added = root.value("added").toObject();
    fromJson(added, diff.dataAdded);

    JsonObject removed = root.value("removed").toObject();
    fromJson(removed, diff.dataRemoved);

    return RetVal<Diff>::make_ok(diff);
}
