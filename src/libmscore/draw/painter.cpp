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

Painter::Painter(IPaintProviderPtr provider)
    : m_provider(provider)
{
}

QPaintDevice* Painter::device() const
{
    return m_provider->device();
}

QPainter* Painter::qpainter() const
{
    return m_provider->qpainter();
}

bool Painter::end()
{
    return m_provider->end();
}

bool Painter::isActive() const
{
    return m_provider->isActive();
}

void Painter::setAntialiasing(bool arg)
{
    m_provider->setAntialiasing(arg);
}

void Painter::setCompositionMode(CompositionMode mode)
{
    m_provider->setCompositionMode(mode);
}

void Painter::setFont(const QFont& f)
{
    m_provider->setFont(f);
}

const QFont& Painter::font() const
{
    return m_provider->font();
}

void Painter::setPen(const QColor& color)
{
    m_provider->setPen(color);
}

void Painter::setPen(const QPen& pen)
{
    m_provider->setPen(pen);
}

void Painter::setNoPen()
{
    m_provider->setPen(QPen(Qt::NoPen));
}

const QPen& Painter::pen() const
{
    return m_provider->pen();
}

void Painter::setBrush(const QBrush& brush)
{
    m_provider->setBrush(brush);
}

const QBrush& Painter::brush() const
{
    return m_provider->brush();
}

void Painter::save()
{
    m_provider->save();
}

void Painter::restore()
{
    m_provider->restore();
}

void Painter::setWorldTransform(const QTransform& matrix, bool combine)
{
    m_provider->setWorldTransform(matrix, combine);
}

const QTransform& Painter::worldTransform() const
{
    return m_provider->worldTransform();
}

void Painter::setTransform(const QTransform& transform, bool combine)
{
    m_provider->setTransform(transform, combine);
}

const QTransform& Painter::transform() const
{
    return m_provider->transform();
}

void Painter::setMatrix(const QMatrix& matrix, bool combine)
{
    m_provider->setMatrix(matrix, combine);
}

void Painter::scale(qreal sx, qreal sy)
{
    m_provider->scale(sx, sy);
}

void Painter::rotate(qreal a)
{
    m_provider->rotate(a);
}

void Painter::translate(const QPointF& offset)
{
    m_provider->translate(offset);
}

QRect Painter::window() const
{
    return m_provider->window();
}

void Painter::setWindow(const QRect& window)
{
    m_provider->setWindow(window);
}

QRect Painter::viewport() const
{
    return m_provider->viewport();
}

void Painter::setViewport(const QRect& viewport)
{
    m_provider->setViewport(viewport);
}

// drawing functions

void Painter::fillPath(const QPainterPath& path, const QBrush& brush)
{
    m_provider->fillPath(path, brush);
}

void Painter::drawPath(const QPainterPath& path)
{
    m_provider->drawPath(path);
}

void Painter::drawLines(const QLineF* lines, int lineCount)
{
    m_provider->drawLines(lines, lineCount);
}

void Painter::drawLines(const QPointF* pointPairs, int lineCount)
{
    m_provider->drawLines(pointPairs, lineCount);
}

void Painter::drawRects(const QRectF* rects, int rectCount)
{
    m_provider->drawRects(rects, rectCount);
}

void Painter::drawEllipse(const QRectF& r)
{
    m_provider->drawEllipse(r);
}

void Painter::drawPolyline(const QPointF* points, int pointCount)
{
    m_provider->drawPolyline(points, pointCount);
}

void Painter::drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule)
{
    m_provider->drawPolygon(points, pointCount, fillRule);
}

void Painter::drawConvexPolygon(const QPointF* points, int pointCount)
{
    m_provider->drawConvexPolygon(points, pointCount);
}

void Painter::drawArc(const QRectF& rect, int a, int alen)
{
    m_provider->drawArc(rect, a, alen);
}

void Painter::drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
    m_provider->drawRoundedRect(rect, xRadius, yRadius, mode);
}

void Painter::drawText(const QPointF& p, const QString& s)
{
    m_provider->drawText(p, s);
}

void Painter::drawText(const QRectF& r, int flags, const QString& text, QRectF* br)
{
    m_provider->drawText(r, flags, text, br);
}

void Painter::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    m_provider->drawGlyphRun(position, glyphRun);
}

void Painter::fillRect(const QRectF& r, const QColor& color)
{
    m_provider->fillRect(r, color);
}

void Painter::drawPixmap(const QPointF& p, const QPixmap& pm)
{
    m_provider->drawPixmap(p, pm);
}

void Painter::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    m_provider->drawTiledPixmap(rect, pm, offset);
}
