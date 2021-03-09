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

#ifndef NO_QT_SUPPORT
#include "qpainterprovider.h"
#endif

using namespace mu::draw;

IPaintProviderPtr Painter::extended;

Painter::Painter(IPaintProviderPtr provider, const std::string& name)
    : m_provider(provider), m_name(name)
{
    m_provider->begin(name);
    if (extended) {
        extended->begin(name);
    }
}

#ifndef NO_QT_SUPPORT
Painter::Painter(QPaintDevice* dp, const std::string& name)
    : m_name(name)
{
    m_provider = QPainterProvider::make(dp);
    m_provider->begin(name);
    if (extended) {
        extended->begin(name);
    }
}

Painter::Painter(QPainter* qp, const std::string& name, bool overship)
    : m_name(name)
{
    m_provider = QPainterProvider::make(qp, overship);
    m_provider->begin(name);
    if (extended) {
        extended->begin(name);
    }
}

#endif

Painter::~Painter()
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
    bool ok = m_provider->end(m_name);
    if (extended) {
        extended->end(m_name);
    }
    return ok;
}

bool Painter::isActive() const
{
    return m_provider->isActive();
}

void Painter::beginObject(const std::string& name, const QPointF& pagePos)
{
    m_provider->beginObject(name, pagePos);
    if (extended) {
        extended->beginObject(name, pagePos);
    }
}

void Painter::endObject(const std::string& name, const QPointF& pagePos)
{
    m_provider->endObject(name, pagePos);
    if (extended) {
        extended->endObject(name, pagePos);
    }
}

void Painter::setAntialiasing(bool arg)
{
    m_provider->setAntialiasing(arg);
    if (extended) {
        extended->setAntialiasing(arg);
    }
}

void Painter::setCompositionMode(CompositionMode mode)
{
    m_provider->setCompositionMode(mode);
    if (extended) {
        extended->setCompositionMode(mode);
    }
}

void Painter::setFont(const QFont& font)
{
    m_provider->setFont(font);
    if (extended) {
        extended->setFont(font);
    }
}

const QFont& Painter::font() const
{
    return m_provider->font();
}

void Painter::setPen(const QPen& pen)
{
    m_provider->setPen(pen);
    if (extended) {
        extended->setPen(pen);
    }
}

void Painter::setNoPen()
{
    setPen(QPen(Qt::NoPen));
}

const QPen& Painter::pen() const
{
    return m_provider->pen();
}

void Painter::setBrush(const QBrush& brush)
{
    m_provider->setBrush(brush);
    if (extended) {
        extended->setBrush(brush);
    }
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
    if (extended) {
        extended->setWorldTransform(matrix, combine);
    }
}

const QTransform& Painter::worldTransform() const
{
    return m_provider->worldTransform();
}

void Painter::setTransform(const QTransform& transform, bool combine)
{
    m_provider->setTransform(transform, combine);
    if (extended) {
        extended->setTransform(transform, combine);
    }
}

const QTransform& Painter::transform() const
{
    return m_provider->transform();
}

void Painter::scale(qreal sx, qreal sy)
{
    m_provider->scale(sx, sy);
    if (extended) {
        extended->scale(sx, sy);
    }
}

void Painter::rotate(qreal angle)
{
    m_provider->rotate(angle);
    if (extended) {
        extended->rotate(angle);
    }
}

void Painter::translate(const QPointF& offset)
{
    m_provider->translate(offset);
    if (extended) {
        extended->translate(offset);
    }
}

QRect Painter::window() const
{
    return m_provider->window();
}

void Painter::setWindow(const QRect& window)
{
    m_provider->setWindow(window);
    if (extended) {
        extended->setWindow(window);
    }
}

QRect Painter::viewport() const
{
    return m_provider->viewport();
}

void Painter::setViewport(const QRect& viewport)
{
    m_provider->setViewport(viewport);
    if (extended) {
        extended->setViewport(viewport);
    }
}

// drawing functions

void Painter::fillPath(const QPainterPath& path, const QBrush& brush)
{
    m_provider->fillPath(path, brush);
    if (extended) {
        extended->fillPath(path, brush);
    }
}

void Painter::drawPath(const QPainterPath& path)
{
    m_provider->drawPath(path);
    if (extended) {
        extended->drawPath(path);
    }
}

void Painter::strokePath(const QPainterPath& path, const QPen& pen)
{
    m_provider->strokePath(path, pen);
    if (extended) {
        extended->strokePath(path, pen);
    }
}

void Painter::drawLines(const QLineF* lines, int lineCount)
{
    m_provider->drawLines(lines, lineCount);
    if (extended) {
        extended->drawLines(lines, lineCount);
    }
}

void Painter::drawLines(const QPointF* pointPairs, int lineCount)
{
    Q_ASSERT(sizeof(QLineF) == 2 * sizeof(QPointF));

    drawLines((const QLineF*)pointPairs, lineCount);
}

void Painter::drawRects(const QRectF* rects, int rectCount)
{
    m_provider->drawRects(rects, rectCount);
    if (extended) {
        extended->drawRects(rects, rectCount);
    }
}

void Painter::drawEllipse(const QRectF& rect)
{
    m_provider->drawEllipse(rect);
    if (extended) {
        extended->drawEllipse(rect);
    }
}

void Painter::drawPolyline(const QPointF* points, int pointCount)
{
    m_provider->drawPolyline(points, pointCount);
    if (extended) {
        extended->drawPolyline(points, pointCount);
    }
}

void Painter::drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule)
{
    m_provider->drawPolygon(points, pointCount, fillRule);
    if (extended) {
        extended->drawPolygon(points, pointCount, fillRule);
    }
}

void Painter::drawConvexPolygon(const QPointF* points, int pointCount)
{
    m_provider->drawConvexPolygon(points, pointCount);
    if (extended) {
        extended->drawConvexPolygon(points, pointCount);
    }
}

void Painter::drawArc(const QRectF& r, int a, int alen)
{
    //! NOTE Copied from QPainter source code

    QRectF rect = r.normalized();

    QPainterPath path;
    path.arcMoveTo(rect, a / 16.0);
    path.arcTo(rect, a / 16.0, alen / 16.0);
    strokePath(path, pen());
}

void Painter::drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
    if (xRadius <= 0 || yRadius <= 0) {             // draw normal rectangle
        drawRect(rect);
        return;
    }

    QPainterPath path;
    path.addRoundedRect(rect, xRadius, yRadius, mode);
    drawPath(path);
}

void Painter::drawText(const QPointF& point, const QString& text)
{
    m_provider->drawText(point, text);
    if (extended) {
        extended->drawText(point, text);
    }
}

void Painter::drawText(const QRectF& rect, int flags, const QString& text)
{
    m_provider->drawText(rect, flags, text);
    if (extended) {
        extended->drawText(rect, flags, text);
    }
}

void Painter::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    m_provider->drawGlyphRun(position, glyphRun);
    if (extended) {
        extended->drawGlyphRun(position, glyphRun);
    }
}

void Painter::fillRect(const QRectF& rect, const QBrush& brush)
{
    m_provider->fillRect(rect, brush);
    if (extended) {
        extended->fillRect(rect, brush);
    }
}

void Painter::drawPixmap(const QPointF& point, const QPixmap& pm)
{
    m_provider->drawPixmap(point, pm);
    if (extended) {
        extended->drawPixmap(point, pm);
    }
}

void Painter::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    m_provider->drawTiledPixmap(rect, pm, offset);
    if (extended) {
        extended->drawTiledPixmap(rect, pm, offset);
    }
}
