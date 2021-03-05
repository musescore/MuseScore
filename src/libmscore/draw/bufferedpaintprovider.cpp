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

using namespace mu::draw;

BufferedPaintProvider::BufferedPaintProvider()
{
    //! NOTE Make data with default state
    m_buf.datas.push_back(DrawBuffer::Data());
}

QPaintDevice* BufferedPaintProvider::device() const
{
    return nullptr;
}

QPainter* BufferedPaintProvider::qpainter() const
{
    return nullptr;
}

void BufferedPaintProvider::begin(const std::string&)
{
    m_isActive = true;
}

bool BufferedPaintProvider::end(const std::string&)
{
    m_isActive = false;
    return true;
}

bool BufferedPaintProvider::isActive() const
{
    return m_isActive;
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

void BufferedPaintProvider::setWorldTransform(const QTransform& matrix, bool combine)
{
    DrawBuffer::State& st = editableState();
    st.worldTransform = matrix;
    st.worldTransformCombine = combine;
}

const QTransform& BufferedPaintProvider::worldTransform() const
{
    return currentState().worldTransform;
}

void BufferedPaintProvider::setTransform(const QTransform& transform, bool combine)
{
    DrawBuffer::State& st = editableState();
    st.transform = transform;
    st.transformCombine = combine;
}

const QTransform& BufferedPaintProvider::transform() const
{
    return currentState().transform;
}

void BufferedPaintProvider::scale(qreal sx, qreal sy)
{
    editableState().scale = Scale{ sx, sy };
}

void BufferedPaintProvider::rotate(qreal a)
{
    editableState().rotate = a;
}

void BufferedPaintProvider::translate(const QPointF& offset)
{
    editableState().translate = offset;
}

QRect BufferedPaintProvider::window() const
{
    return m_buf.window;
}

void BufferedPaintProvider::setWindow(const QRect& window)
{
    m_buf.window = window;
}

QRect BufferedPaintProvider::viewport() const
{
    return m_buf.viewport;
}

void BufferedPaintProvider::setViewport(const QRect& viewport)
{
    m_buf.viewport = viewport;
}

// drawing functions

void BufferedPaintProvider::fillPath(const QPainterPath& path, const QBrush& brush)
{
    editableData().fillPaths.push_back(FillPath { path, brush });
}

void BufferedPaintProvider::drawPath(const QPainterPath& path)
{
    editableData().paths.push_back(path);
}

void BufferedPaintProvider::strokePath(const QPainterPath& path, const QPen& pen)
{
    editableData().drawPaths.push_back(DrawPath { path, pen, true });
}

void BufferedPaintProvider::drawLines(const QLineF* lines, int lineCount)
{
    for (int i = 0; i < lineCount; ++i) {
        editableData().lines.push_back(lines[i]);
    }
}

void BufferedPaintProvider::drawRects(const QRectF* rects, int rectCount)
{
    for (int i = 0; i < rectCount; ++i) {
        editableData().rects.push_back(rects[i]);
    }
}

void BufferedPaintProvider::drawEllipse(const QRectF& r)
{
    editableData().ellipses.push_back(r);
}

void BufferedPaintProvider::drawPolyline(const QPointF* points, int pointCount)
{
    DrawBuffer::Data& d = editableData();
    for (int i = 1; i < pointCount; ++i) {
        QPointF p1 = points[i - 1];
        QPointF p2 = points[i];
        d.lines.push_back(QLineF(p1, p2));
    }
}

void BufferedPaintProvider::drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule)
{
    QVector<QPointF> vec(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        vec[i] = points[i];
    }
    editableData().polygons.push_back(FillPolygon { QPolygonF(vec), fillRule, false });
}

void BufferedPaintProvider::drawConvexPolygon(const QPointF* points, int pointCount)
{
    QVector<QPointF> vec(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        vec[i] = points[i];
    }
    editableData().polygons.push_back(FillPolygon { QPolygonF(vec), Qt::OddEvenFill, true });
}

void BufferedPaintProvider::drawText(const QPointF& p, const QString& s)
{
    editableData().texts.push_back(DrawText { p, s });
}

void BufferedPaintProvider::drawText(const QRectF& r, int flags, const QString& text)
{
    editableData().rectTexts.push_back(DrawRectText { r, flags, text });
}

void BufferedPaintProvider::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    editableData().glyphs.push_back(DrawGlyphRun { position, glyphRun });
}

void BufferedPaintProvider::fillRect(const QRectF& r, const QBrush& brush)
{
    editableData().fillRects.push_back(FillRect { r, brush });
}

void BufferedPaintProvider::drawPixmap(const QPointF& p, const QPixmap& pm)
{
    editableData().pixmaps.push_back(DrawPixmap { p, pm });
}

void BufferedPaintProvider::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    editableData().tiledPixmap.push_back(DrawTiledPixmap { rect, pm, offset });
}
