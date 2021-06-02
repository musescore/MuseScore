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
#include "bufferedpaintprovider.h"

#include "utils/drawlogger.h"
#include "fontcompat.h"
#include "log.h"
#include "config.h"

using namespace mu::draw;

BufferedPaintProvider::BufferedPaintProvider()
{
    m_drawObjectsLogger = new DrawObjectsLogger();
    clear();
}

BufferedPaintProvider::~BufferedPaintProvider()
{
    delete m_drawObjectsLogger;
}

QPaintDevice* BufferedPaintProvider::device() const
{
    return nullptr;
}

QPainter* BufferedPaintProvider::qpainter() const
{
    return nullptr;
}

void BufferedPaintProvider::beginTarget(const std::string& name)
{
    clear();
    m_buf.name = name;
    beginObject(name + "_default", QPointF());
    m_isActive = true;
}

void BufferedPaintProvider::beforeEndTargetHook(Painter*)
{
}

bool BufferedPaintProvider::endTarget(bool endDraw)
{
    UNUSED(endDraw);
    if (m_isActive) {
        m_isActive = false;
        endObject();
    }
    return true;
}

bool BufferedPaintProvider::isActive() const
{
    return m_isActive;
}

void BufferedPaintProvider::beginObject(const std::string& name, const QPointF& pagePos)
{
    // add new object
    m_currentObjects.push(DrawData::Object(name, pagePos));

#ifdef TRACE_DRAW_OBJ_ENABLED
    m_drawObjectsLogger.beginObject(name, pagePos);
#endif
}

void BufferedPaintProvider::endObject()
{
    TRACEFUNC;

    // remove last empty state
    DrawData::Object& obj = m_currentObjects.top();
    if (!obj.datas.empty() && obj.datas.back().empty()) {
        obj.datas.pop_back();
    }

    // move object to buffer
    m_buf.objects.push_back(obj);

    // remove obj
    m_currentObjects.pop();

#ifdef TRACE_DRAW_OBJ_ENABLED
    m_drawObjectsLogger.endObject();
#endif
}

const DrawData::Data& BufferedPaintProvider::currentData() const
{
    return m_currentObjects.top().datas.back();
}

const DrawData::State& BufferedPaintProvider::currentState() const
{
    return currentData().state;
}

DrawData::Data& BufferedPaintProvider::editableData()
{
    return m_currentObjects.top().datas.back();
}

DrawData::State& BufferedPaintProvider::editableState()
{
    DrawData::Data& data = m_currentObjects.top().datas.back();
    if (data.empty()) {
        return data.state;
    }

    {
        DrawData::Data newData;
        newData.state = data.state;
        m_currentObjects.top().datas.push_back(std::move(newData));
    }
    return m_currentObjects.top().datas.back().state;
}

void BufferedPaintProvider::setAntialiasing(bool arg)
{
    editableState().isAntialiasing = arg;
}

void BufferedPaintProvider::setCompositionMode(CompositionMode mode)
{
    editableState().compositionMode = mode;
}

void BufferedPaintProvider::setFont(const Font& f)
{
    editableState().font = f;
}

const Font& BufferedPaintProvider::font() const
{
    return currentState().font;
}

void BufferedPaintProvider::setPen(const QPen& pen)
{
    editableState().pen = pen;
}

void BufferedPaintProvider::setNoPen()
{
    editableState().pen.setStyle(Qt::NoPen);
}

const QPen& BufferedPaintProvider::pen() const
{
    return currentState().pen;
}

void BufferedPaintProvider::setBrush(const QBrush& brush)
{
    editableState().brush = brush;
}

const QBrush& BufferedPaintProvider::brush() const
{
    return currentState().brush;
}

void BufferedPaintProvider::save()
{
}

void BufferedPaintProvider::restore()
{
}

void BufferedPaintProvider::setTransform(const QTransform& transform)
{
    DrawData::State& st = editableState();
    st.transform = transform;
}

const QTransform& BufferedPaintProvider::transform() const
{
    return currentState().transform;
}

// drawing functions

void BufferedPaintProvider::drawPath(const QPainterPath& path)
{
    const DrawData::State& st = currentState();
    DrawMode mode = DrawMode::StrokeAndFill;
    if (st.pen.style() == Qt::NoPen) {
        mode = DrawMode::Fill;
    } else if (st.brush.style() == Qt::NoBrush) {
        mode = DrawMode::Stroke;
    } else {
        LOGW() << "not set pen or brush, path will not draw";
        return;
    }
    editableData().paths.push_back({ path, st.pen, st.brush, mode });
}

void BufferedPaintProvider::drawPolygon(const QPointF* points, int pointCount, PolygonMode mode)
{
    QPolygonF pol(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        pol[i] = points[i];
    }
    editableData().polygons.push_back(DrawPolygon { pol, mode });
}

void BufferedPaintProvider::drawText(const QPointF& point, const QString& text)
{
    editableData().texts.push_back(DrawText { point, text });
}

void BufferedPaintProvider::drawText(const QRectF& rect, int flags, const QString& text)
{
    editableData().rectTexts.push_back(DrawRectText { rect, flags, text });
}

void BufferedPaintProvider::drawTextWorkaround(mu::draw::Font& f, const QPointF& pos, const QString& text)
{
    setFont(f);
    drawText(pos, text);
}

void BufferedPaintProvider::drawSymbol(const QPointF& point, uint ucs4Code)
{
    drawText(point, QString::fromUcs4(&ucs4Code, 1));
}

void BufferedPaintProvider::drawPixmap(const QPointF& p, const QPixmap& pm)
{
    editableData().pixmaps.push_back(DrawPixmap { p, pm });
}

void BufferedPaintProvider::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    editableData().tiledPixmap.push_back(DrawTiledPixmap { rect, pm, offset });
}

const DrawData& BufferedPaintProvider::drawData() const
{
    return m_buf;
}

void BufferedPaintProvider::clear()
{
    m_buf = DrawData();
    std::stack<DrawData::Object> empty;
    m_currentObjects.swap(empty);
}
