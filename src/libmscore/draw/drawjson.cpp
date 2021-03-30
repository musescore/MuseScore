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
#include "drawjson.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>

#include "realfn.h"
#include "log.h"

using namespace mu::draw;

static int rtoi(qreal v)
{
    return static_cast<int>(v * 1000.0);
}

static qreal itor(int v)
{
    return static_cast<qreal>(v) / 1000.0;
}

static QJsonObject toObj(const QPen& pen)
{
    QJsonObject obj;
    obj["style"] = static_cast<int>(pen.style());
    obj["color"] = pen.color().name();
    obj["width"] = pen.width();
    return obj;
}

static void fromObj(const QJsonObject& obj, QPen& pen)
{
    pen.setStyle(static_cast<Qt::PenStyle>(obj["style"].toInt()));
    pen.setColor(QColor(obj["color"].toString()));
    pen.setWidth(obj["width"].toInt());
}

static QJsonObject toObj(const QBrush& brush)
{
    QJsonObject obj;
    obj["style"] = static_cast<int>(brush.style());
    obj["color"] = brush.color().name();
    return obj;
}

static void fromObj(const QJsonObject& obj, QBrush& brush)
{
    brush.setStyle(static_cast<Qt::BrushStyle>(obj["style"].toInt()));
    brush.setColor(QColor(obj["color"].toString()));
}

static QJsonObject toObj(const QFont& font)
{
    QJsonObject obj;
    obj["family"] = font.family();
    obj["pointSize"] = font.pointSize();
    obj["weight"] = font.weight();
    obj["italic"] = font.italic();
    return obj;
}

static void fromObj(const QJsonObject& obj, QFont& font)
{
    font.setFamily(obj["family"].toString());
    font.setPointSize(obj["pointSize"].toInt());
    font.setWeight(obj["weight"].toInt());
    font.setItalic(obj["italic"].toBool());
}

static QJsonArray toArr(const QTransform& t)
{
    return QJsonArray({ rtoi(t.m11()), rtoi(t.m12()), rtoi(t.m13()),
                        rtoi(t.m21()), rtoi(t.m22()), rtoi(t.m23()),
                        rtoi(t.m31()), rtoi(t.m32()), rtoi(t.m33()) });
}

static void fromArr(const QJsonArray& arr, QTransform& t)
{
    IF_ASSERT_FAILED(arr.size() == 9) {
        return;
    }

    t.setMatrix(itor(arr.at(0).toInt()), itor(arr.at(1).toInt()), itor(arr.at(2).toInt()),
                itor(arr.at(3).toInt()), itor(arr.at(4).toInt()), itor(arr.at(5).toInt()),
                itor(arr.at(6).toInt()), itor(arr.at(7).toInt()), itor(arr.at(8).toInt()));
}

static QJsonArray toArr(const QPointF& p)
{
    return QJsonArray({ rtoi(p.x()), rtoi(p.y()) });
}

static void fromArr(const QJsonArray& arr, QPointF& p)
{
    IF_ASSERT_FAILED(arr.size() == 2) {
        return;
    }
    p.setX(itor(arr.at(0).toInt()));
    p.setY(itor(arr.at(1).toInt()));
}

static QJsonArray toArr(const QRectF& r)
{
    return QJsonArray({ rtoi(r.x()), rtoi(r.y()), rtoi(r.width()), rtoi(r.height()) });
}

static void fromArr(const QJsonArray& arr, QRectF& r)
{
    IF_ASSERT_FAILED(arr.size() == 4) {
        return;
    }
    r = QRectF(itor(arr.at(0).toInt()), itor(arr.at(1).toInt()), itor(arr.at(2).toInt()), itor(arr.at(3).toInt()));
}

static QJsonArray toArr(const QSize& sz)
{
    return QJsonArray({ sz.width(), sz.height() });
}

static void fromArr(const QJsonArray& arr, QSize& sz)
{
    IF_ASSERT_FAILED(arr.size() == 2) {
        return;
    }
    sz = QSize(arr.at(0).toInt(), arr.at(1).toInt());
}

static QJsonObject toObj(const DrawData::State& st)
{
    QJsonObject obj;
    obj["pen"] = toObj(st.pen);
    obj["brush"] = toObj(st.brush);
    obj["font"] = toObj(st.font);
    obj["isAntialiasing"] = st.isAntialiasing;
    obj["transform"] = toArr(st.transform);
    obj["compositionMode"] = static_cast<int>(st.compositionMode);
    return obj;
}

static void fromObj(const QJsonObject& obj, DrawData::State& st)
{
    fromObj(obj["pen"].toObject(), st.pen);
    fromObj(obj["brush"].toObject(), st.brush);
    fromObj(obj["font"].toObject(), st.font);
    st.isAntialiasing = obj["isAntialiasing"].toBool();
    fromArr(obj["transform"].toArray(), st.transform);
    st.compositionMode = static_cast<CompositionMode>(obj["compositionMode"].toInt());
}

static QJsonObject toObj(const QPainterPath& path)
{
    QJsonObject obj;
    obj["fillRule"] = static_cast<int>(path.fillRule());

    QJsonArray elsArr;
    for (int i = 0; i < path.elementCount(); ++i) {
        QPainterPath::Element e = path.elementAt(i);
        elsArr.append(QJsonArray({ static_cast<int>(e.type), rtoi(e.x), rtoi(e.y) }));
    }
    obj["elements"] = elsArr;
    return obj;
}

static QJsonObject toObj(const DrawPath& path)
{
    QJsonObject obj;
    obj["path"] = toObj(path.path);
    obj["pen"] = toObj(path.pen);
    obj["brush"] = toObj(path.brush);
    obj["mode"] = static_cast<int>(path.mode);
    return obj;
}

static void fromObj(const QJsonObject& obj, QPainterPath& path)
{
    path.setFillRule(static_cast<Qt::FillRule>(obj["fillRule"].toInt()));

    QJsonArray elsArr = obj["elements"].toArray();
    std::vector<QPainterPath::Element> curveEls;
    for (const QJsonValue elVal : elsArr) {
        QJsonArray elArr = elVal.toArray();
        IF_ASSERT_FAILED(elArr.size() == 3) {
            continue;
        }

        QPainterPath::ElementType type = static_cast<QPainterPath::ElementType>(elArr.at(0).toInt());
        qreal x = itor(elArr.at(1).toInt());
        qreal y = itor(elArr.at(2).toInt());

        switch (type) {
        case QPainterPath::MoveToElement: {
            path.moveTo(x, y);
        } break;
        case QPainterPath::LineToElement: {
            path.lineTo(x, y);
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

            path.cubicTo(curveEls.at(0).x, curveEls.at(0).y, curveEls.at(1).x, curveEls.at(1).y, x, y);
            curveEls.clear();
        } break;
        }
    }
}

static void fromObj(const QJsonObject& obj, DrawPath& path)
{
    fromObj(obj["path"].toObject(), path.path);
    fromObj(obj["pen"].toObject(), path.pen);
    fromObj(obj["brush"].toObject(), path.brush);
    path.mode = static_cast<DrawMode>(obj["mode"].toInt());
}

static QJsonArray toArr(const QPolygonF& pol)
{
    QJsonArray a;
    for (const QPointF& p : pol) {
        a.append(toArr(p));
    }
    return a;
}

static void fromArr(const QJsonArray& arr, QPolygonF& pol)
{
    pol.reserve(arr.size());
    for (const QJsonValue& val : arr) {
        QPointF p;
        fromArr(val.toArray(), p);
        pol.push_back(std::move(p));
    }
}

static QJsonObject toObj(const DrawPolygon& pol)
{
    QJsonObject obj;
    obj["polygon"] = toArr(pol.polygon);
    obj["mode"] = static_cast<int>(pol.mode);
    return obj;
}

static void fromObj(const QJsonObject& obj, DrawPolygon& pol)
{
    fromArr(obj["polygon"].toArray(), pol.polygon);
    pol.mode = static_cast<PolygonMode>(obj["mode"].toInt());
}

static QJsonObject toObj(const DrawText& text)
{
    QJsonObject o;
    o["pos"] = toArr(text.pos);
    o["text"] = text.text;
    return o;
}

static void fromObj(const QJsonObject& obj, DrawText& text)
{
    fromArr(obj["pos"].toArray(), text.pos);
    text.text = obj["text"].toString();
}

static QJsonObject toObj(const DrawRectText& text)
{
    QJsonObject o;
    o["rect"] = toArr(text.rect);
    o["flags"] = text.flags;
    o["text"] = text.text;
    return o;
}

static void fromObj(const QJsonObject& obj, DrawRectText& text)
{
    fromArr(obj["rect"].toArray(), text.rect);
    text.flags = obj["flags"].toInt();
    text.text = obj["text"].toString();
}

static QJsonObject toObj(const DrawPixmap& pm)
{
    QJsonObject o;
    o["pos"] = toArr(pm.pos);
    o["pmSize"] = toArr(pm.pm.size());
    return o;
}

static void fromObj(const QJsonObject& obj, DrawPixmap& pm)
{
    fromArr(obj["pos"].toArray(), pm.pos);
    QSize size;
    fromArr(obj["pmSize"].toArray(), size);
    pm.pm = QPixmap(size);
}

static QJsonObject toObj(const DrawTiledPixmap& pm)
{
    QJsonObject o;
    o["rect"] = toArr(pm.rect);
    o["pmSize"] = toArr(pm.pm.size());
    o["offset"] = toArr(pm.offset);
    return o;
}

static void fromObj(const QJsonObject& obj, DrawTiledPixmap& pm)
{
    fromArr(obj["rect"].toArray(), pm.rect);
    QSize size;
    fromArr(obj["pmSize"].toArray(), size);
    pm.pm = QPixmap(size);
    fromArr(obj["offset"].toArray(), pm.offset);
}

template<class T>
static QJsonArray toArr(const std::vector<T>& vals)
{
    QJsonArray arr;
    for (const auto& v : vals) {
        arr << toObj(v);
    }
    return arr;
}

template<class T>
static void fromArr(const QJsonArray& arr, std::vector<T>& vals)
{
    vals.reserve(arr.count());
    for (const auto& v : arr) {
        T val;
        fromObj(v.toObject(), val);
        vals.push_back(std::move(val));
    }
}

QByteArray DrawBufferJson::toJson(const DrawData& buf)
{
    //! NOTE 'a' added to the beginning of some field names for convenient sorting

    QJsonObject root;
    root["a_name"] = QString::fromStdString(buf.name);

    QJsonArray objsArr;
    for (const DrawData::Object& obj : buf.objects) {
        QJsonObject objObj;
        objObj["a_name"] = QString::fromStdString(obj.name);
        objObj["a_pagePos"] = toArr(obj.pagePos);
        QJsonArray datasArr;
        for (const DrawData::Data& data : obj.datas) {
            if (data.empty()) {
                continue;
            }

            QJsonObject dataObj;
            dataObj["state"] = toObj(data.state);
            dataObj["paths"] = toArr(data.paths);
            dataObj["polygons"] = toArr(data.polygons);
            dataObj["texts"] = toArr(data.texts);
            dataObj["rectTexts"] = toArr(data.rectTexts);
            // dataObj["glyphs"] = toArr(data.glyphs);  not implemented
            dataObj["pixmaps"] = toArr(data.pixmaps);
            dataObj["tiledPixmap"] = toArr(data.tiledPixmap);

            datasArr.append(dataObj);
        }
        objObj["datas"] = datasArr;

        objsArr.append(objObj);
    }

    root["objects"] = objsArr;

    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

mu::RetVal<DrawDataPtr> DrawBufferJson::fromJson(const QByteArray& json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError) {
        RetVal<DrawDataPtr> rv;
        rv.ret = make_ret(Ret::Code::UnknownError, err.errorString().toStdString());
        return rv;
    }

    QJsonObject root = doc.object();

    DrawDataPtr buf = std::make_shared<DrawData>();
    buf->name = root["a_name"].toString().toStdString();
    QJsonArray objsArr = root["objects"].toArray();
    for (const QJsonValue objVal: objsArr) {
        QJsonObject objObj = objVal.toObject();
        DrawData::Object obj;
        obj.name = objObj["a_name"].toString().toStdString();
        fromArr(objObj["a_pagePos"].toArray(), obj.pagePos);
        QJsonArray datasArr = objObj["datas"].toArray();
        for (const QJsonValue dataVal : datasArr) {
            QJsonObject dataObj = dataVal.toObject();
            DrawData::Data data;
            fromObj(dataObj["state"].toObject(), data.state);
            fromArr(dataObj["paths"].toArray(), data.paths);
            fromArr(dataObj["polygons"].toArray(), data.polygons);
            fromArr(dataObj["texts"].toArray(), data.texts);
            fromArr(dataObj["rectTexts"].toArray(), data.rectTexts);
            fromArr(dataObj["pixmaps"].toArray(), data.pixmaps);
            fromArr(dataObj["tiledPixmap"].toArray(), data.tiledPixmap);

            obj.datas.push_back(std::move(data));
        }

        buf->objects.push_back(std::move(obj));
    }

    return RetVal<DrawDataPtr>::make_ok(buf);
}
