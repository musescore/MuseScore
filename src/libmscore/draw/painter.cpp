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
#include "painter.h"

using namespace mu::draw;

Painter::Painter(QPainter* painter)
    : m_painter(painter)
{
}

QPaintDevice* Painter::device() const
{
    return m_painter->device();
}

QPainter* Painter::qpainter() const
{
    return m_painter;
}

bool Painter::begin(QPaintDevice* d)
{
    return m_painter->begin(d);
}

bool Painter::end()
{
    return m_painter->end();
}

bool Painter::isActive() const
{
    return m_painter->isActive();
}

void Painter::setAntialiasing(bool arg)
{
    m_painter->setRenderHint(QPainter::Antialiasing, arg);
    m_painter->setRenderHint(QPainter::TextAntialiasing, arg);
}

void Painter::setCompositionMode(CompositionMode mode)
{
    auto toQPainter = [](CompositionMode mode) {
        switch (mode) {
        case CompositionMode::SourceOver: return QPainter::CompositionMode_SourceOver;
        case CompositionMode::HardLight: return QPainter::CompositionMode_HardLight;
        }
        return QPainter::CompositionMode_SourceOver;
    };
    m_painter->setCompositionMode(toQPainter(mode));
}

void Painter::setFont(const QFont& f)
{
    m_painter->setFont(f);
}

const QFont& Painter::font() const
{
    return m_painter->font();
}

void Painter::setPen(const QColor& color)
{
    m_painter->setPen(color);
}

void Painter::setPen(const QPen& pen)
{
    m_painter->setPen(pen);
}

void Painter::setNoPen()
{
    m_painter->setPen(QPen(Qt::NoPen));
}

const QPen& Painter::pen() const
{
    return m_painter->pen();
}

void Painter::setBrush(const QBrush& brush)
{
    m_painter->setBrush(brush);
}

const QBrush& Painter::brush() const
{
    return m_painter->brush();
}

void Painter::save()
{
    m_painter->save();
}

void Painter::restore()
{
    m_painter->restore();
}

void Painter::setWorldTransform(const QTransform& matrix, bool combine)
{
    m_painter->setWorldTransform(matrix, combine);
}

const QTransform& Painter::worldTransform() const
{
    return m_painter->worldTransform();
}

void Painter::setTransform(const QTransform& transform, bool combine)
{
    m_painter->setTransform(transform, combine);
}

const QTransform& Painter::transform() const
{
    return m_painter->transform();
}

void Painter::setMatrix(const QMatrix& matrix, bool combine)
{
    m_painter->setMatrix(matrix, combine);
}

void Painter::scale(qreal sx, qreal sy)
{
    m_painter->scale(sx, sy);
}

void Painter::rotate(qreal a)
{
    m_painter->rotate(a);
}

void Painter::translate(const QPointF& offset)
{
    m_painter->translate(offset);
}

QRect Painter::window() const
{
    return m_painter->window();
}

void Painter::setWindow(const QRect& window)
{
    m_painter->setWindow(window);
}

QRect Painter::viewport() const
{
    return m_painter->viewport();
}

void Painter::setViewport(const QRect& viewport)
{
    m_painter->setViewport(viewport);
}

// drawing functions

void Painter::fillPath(const QPainterPath& path, const QBrush& brush)
{
    m_painter->fillPath(path, brush);
}

void Painter::drawPath(const QPainterPath& path)
{
    m_painter->drawPath(path);
}

void Painter::drawLines(const QLineF* lines, int lineCount)
{
    m_painter->drawLines(lines, lineCount);
}

void Painter::drawLines(const QPointF* pointPairs, int lineCount)
{
    m_painter->drawLines(pointPairs, lineCount);
}

void Painter::drawRects(const QRectF* rects, int rectCount)
{
    m_painter->drawRects(rects, rectCount);
}

void Painter::drawEllipse(const QRectF& r)
{
    m_painter->drawEllipse(r);
}

void Painter::drawPolyline(const QPointF* points, int pointCount)
{
    m_painter->drawPolyline(points, pointCount);
}

void Painter::drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule)
{
    m_painter->drawPolygon(points, pointCount, fillRule);
}

void Painter::drawConvexPolygon(const QPointF* points, int pointCount)
{
    m_painter->drawConvexPolygon(points, pointCount);
}

void Painter::drawArc(const QRectF& rect, int a, int alen)
{
    m_painter->drawArc(rect, a, alen);
}

void Painter::drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
    m_painter->drawRoundedRect(rect, xRadius, yRadius, mode);
}

void Painter::drawText(const QPointF& p, const QString& s)
{
    m_painter->drawText(p, s);
}

void Painter::drawText(const QRectF& r, int flags, const QString& text, QRectF* br)
{
    m_painter->drawText(r, flags, text, br);
}

void Painter::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    m_painter->drawGlyphRun(position, glyphRun);
}

void Painter::fillRect(const QRectF& r, const QColor& color)
{
    m_painter->fillRect(r, color);
}

void Painter::drawPixmap(const QPointF& p, const QPixmap& pm)
{
    m_painter->drawPixmap(p, pm);
}

void Painter::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    m_painter->drawTiledPixmap(rect, pm, offset);
}
