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

//! HACK to access QPainterPath private data
#undef QPAINTERPATH_H
#undef QPAINTERPATH_P_H
#define private public
#include <QPainterPath>
#include <private/qpainterpath_p.h>
#undef private

#include "drawbufferjson.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

using namespace mu::draw;

static int rtoi(qreal v)
{
    return v * 1000;
}

static QJsonObject toObj(const QPen& pen)
{
    QJsonObject o;
    o["style"] = static_cast<int>(pen.style());
    o["color"] = pen.color().name();
    o["width"] = pen.width();
    return o;
}

static QJsonObject toObj(const QBrush& brush)
{
    QJsonObject o;
    o["style"] = static_cast<int>(brush.style());
    o["color"] = brush.color().name();
    return o;
}

static QJsonObject toObj(const QFont& font)
{
    QJsonObject o;
    o["family"] = font.family();
    o["pointSize"] = font.pointSize();
    o["weight"] = font.weight();
    o["italic"] = font.italic();
    return o;
}

static QJsonArray toArr(const QTransform& t)
{
    return QJsonArray({ rtoi(t.m11()), rtoi(t.m12()), rtoi(t.m13()),
                        rtoi(t.m21()), rtoi(t.m22()), rtoi(t.m23()),
                        rtoi(t.m31()), rtoi(t.m32()), rtoi(t.m33()),
                        rtoi(t.dx()), rtoi(t.dy()) });
}

static QJsonArray toArr(const QPointF& p)
{
    return QJsonArray({ rtoi(p.x()), rtoi(p.y()) });
}

static QJsonArray toArr(const QRectF& r)
{
    return QJsonArray({ rtoi(r.x()), rtoi(r.y()), rtoi(r.width()), rtoi(r.height()) });
}

static QJsonObject toObj(const DrawBuffer::State& st)
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

static QJsonObject toObj(const QPainterPath& path)
{
    QJsonObject o;
    o["fillRule"] = static_cast<int>(path.fillRule());

    QJsonArray elsArr;
    for (int i = 0; i < path.elementCount(); ++i) {
        QPainterPath::Element e = path.elementAt(i);
        elsArr.append(QJsonArray({ static_cast<int>(e.type), rtoi(e.x), rtoi(e.y) }));
    }
    o["elements"] = elsArr;
    return o;
}

static QJsonObject toObj(const DrawPath& path)
{
    QJsonObject o;
    o["path"] = toObj(path.path);
    o["pen"] = toObj(path.pen);
    o["brush"] = toObj(path.brush);
    o["mode"] = static_cast<int>(path.mode);
    return o;
}

static QJsonArray toArr(const QPolygonF& pol)
{
    QJsonArray a;
    for (const QPointF& p : pol) {
        a.append(toArr(p));
    }
    return a;
}

static QJsonObject toObj(const DrawPolygon& pol)
{
    QJsonObject o;
    o["polygon"] = toArr(pol.polygon);
    o["mode"] = static_cast<int>(pol.mode);
    return o;
}

static QJsonObject toObj(const DrawText& text)
{
    QJsonObject o;
    o["pos"] = toArr(text.pos);
    o["text"] = text.text;
    return o;
}

static QJsonObject toObj(const DrawRectText& text)
{
    QJsonObject o;
    o["rect"] = toArr(text.rect);
    o["flags"] = text.flags;
    o["text"] = text.text;
    return o;
}

static QJsonObject toObj(const DrawPixmap& pm)
{
    QJsonObject o;
    o["pos"] = toArr(pm.pos);
    o["pmRect"] = toArr(pm.pm.rect());
    return o;
}

static QJsonObject toObj(const DrawTiledPixmap& pm)
{
    QJsonObject o;
    o["rect"] = toArr(pm.rect);
    o["pmRect"] = toArr(pm.pm.rect());
    o["offset"] = toArr(pm.offset);
    return o;
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

QByteArray DrawBufferJson::toJson(const DrawBuffer& buf)
{
    //! NOTE 'a' added to the beginning of some field names for convenient sorting

    QJsonObject root;
    root["a_name"] = QString::fromStdString(buf.name);

    QJsonArray objsArr;
    for (const DrawBuffer::Object& obj : buf.objects) {
        QJsonObject objObj;
        objObj["a_name"] = QString::fromStdString(obj.name);
        objObj["a_pagePos"] = toArr(obj.pagePos);
        QJsonArray datasArr;
        for (const DrawBuffer::Data& data : obj.datas) {
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

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}
