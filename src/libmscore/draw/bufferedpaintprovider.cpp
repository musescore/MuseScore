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
#include "bufferedpaintprovider.h"

#include "log.h"

using namespace mu::draw;

BufferedPaintProvider::BufferedPaintProvider()
{
    clear();
}

QPaintDevice* BufferedPaintProvider::device() const
{
    return nullptr;
}

QPainter* BufferedPaintProvider::qpainter() const
{
    return nullptr;
}

void BufferedPaintProvider::beginTarget(const std::string&)
{
    clear();
    m_isActive = true;
}

bool BufferedPaintProvider::endTarget(const std::string&, bool endDraw)
{
    UNUSED(endDraw);
    m_isActive = false;
    return true;
}

bool BufferedPaintProvider::isActive() const
{
    return m_isActive;
}

void BufferedPaintProvider::beginObject(const std::string& name, const QPointF& pagePos)
{
    m_drawObjectsLogger.beginObject(name, pagePos);
}

void BufferedPaintProvider::endObject(const std::string& name, const QPointF& pagePos)
{
    m_drawObjectsLogger.endObject(name, pagePos);
}

const DrawBuffer::State& BufferedPaintProvider::currentState() const
{
    return m_buf.datas.back().state;
}

DrawBuffer::State& BufferedPaintProvider::editableState()
{
    DrawBuffer::Data& data = m_buf.datas.back();
    if (m_isCurDataEmpty) {
        return data.state;
    }

    {
        DrawBuffer::Data newData;
        newData.state = data.state;
        m_buf.datas.push_back(std::move(newData));
        m_isCurDataEmpty = true;
    }
    return m_buf.datas.back().state;
}

DrawBuffer::Data& BufferedPaintProvider::editableData()
{
    m_isCurDataEmpty = false;
    return m_buf.datas.back();
}

void BufferedPaintProvider::setAntialiasing(bool arg)
{
    editableState().isAntialiasing = arg;
}

void BufferedPaintProvider::setCompositionMode(CompositionMode mode)
{
    editableState().compositionMode = mode;
}

void BufferedPaintProvider::setFont(const QFont& f)
{
    editableState().font = f;
}

const QFont& BufferedPaintProvider::font() const
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
    DrawBuffer::State& st = editableState();
    st.transform = transform;
}

const QTransform& BufferedPaintProvider::transform() const
{
    return currentState().transform;
}

// drawing functions

void BufferedPaintProvider::drawPath(const QPainterPath& path)
{
    const DrawBuffer::State& st = currentState();
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

void BufferedPaintProvider::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    editableData().glyphs.push_back(DrawGlyphRun { position, glyphRun });
}

void BufferedPaintProvider::drawPixmap(const QPointF& p, const QPixmap& pm)
{
    editableData().pixmaps.push_back(DrawPixmap { p, pm });
}

void BufferedPaintProvider::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    editableData().tiledPixmap.push_back(DrawTiledPixmap { rect, pm, offset });
}

const DrawBuffer& BufferedPaintProvider::buffer() const
{
    return m_buf;
}

void BufferedPaintProvider::clear()
{
    m_buf = DrawBuffer();
    //! NOTE Make data with default state
    m_buf.datas.push_back(DrawBuffer::Data());
    m_isCurDataEmpty = true;
}
