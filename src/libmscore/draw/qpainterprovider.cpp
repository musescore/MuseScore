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
#include "qpainterprovider.h"

#include <QPainter>
#include "log.h"

using namespace mu::draw;

QPainterProvider::QPainterProvider(QPainter* painter, bool overship)
    : m_painter(painter), m_overship(overship)
{
}

QPainterProvider::~QPainterProvider()
{
    if (m_overship) {
        delete m_painter;
    }
}

IPaintProviderPtr QPainterProvider::make(QPaintDevice* dp)
{
    return std::make_shared<QPainterProvider>(new QPainter(dp), true);
}

IPaintProviderPtr QPainterProvider::make(QPainter* qp, bool overship)
{
    return std::make_shared<QPainterProvider>(qp, overship);
}

QPaintDevice* QPainterProvider::device() const
{
    return m_painter->device();
}

QPainter* QPainterProvider::qpainter() const
{
    return m_painter;
}

void QPainterProvider::begin(const std::string&)
{
}

bool QPainterProvider::end(const std::string&)
{
    return m_painter->end();
}

bool QPainterProvider::isActive() const
{
    return m_painter->isActive();
}

void QPainterProvider::beginObject(const std::string& name, const QPointF& pagePos)
{
    m_drawObjectsLogger.beginObject(name, pagePos);
}

void QPainterProvider::endObject(const std::string& name, const QPointF& pagePos)
{
    m_drawObjectsLogger.endObject(name, pagePos);
}

void QPainterProvider::setAntialiasing(bool arg)
{
    m_painter->setRenderHint(QPainter::Antialiasing, arg);
    m_painter->setRenderHint(QPainter::TextAntialiasing, arg);
}

void QPainterProvider::setCompositionMode(CompositionMode mode)
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

void QPainterProvider::setFont(const QFont& font)
{
    m_painter->setFont(font);
}

const QFont& QPainterProvider::font() const
{
    return m_painter->font();
}

void QPainterProvider::setPen(const QPen& pen)
{
    m_painter->setPen(pen);
}

void QPainterProvider::setNoPen()
{
    m_painter->setPen(QPen(Qt::NoPen));
}

const QPen& QPainterProvider::pen() const
{
    return m_painter->pen();
}

void QPainterProvider::setBrush(const QBrush& brush)
{
    m_painter->setBrush(brush);
}

const QBrush& QPainterProvider::brush() const
{
    return m_painter->brush();
}

void QPainterProvider::save()
{
    m_painter->save();
}

void QPainterProvider::restore()
{
    m_painter->restore();
}

void QPainterProvider::setWorldTransform(const QTransform& matrix, bool combine)
{
    m_painter->setWorldTransform(matrix, combine);
}

const QTransform& QPainterProvider::worldTransform() const
{
    return m_painter->worldTransform();
}

void QPainterProvider::setTransform(const QTransform& transform, bool combine)
{
    m_painter->setTransform(transform, combine);
}

const QTransform& QPainterProvider::transform() const
{
    return m_painter->transform();
}

void QPainterProvider::scale(qreal sx, qreal sy)
{
    m_painter->scale(sx, sy);
}

void QPainterProvider::rotate(qreal angle)
{
    m_painter->rotate(angle);
}

void QPainterProvider::translate(const QPointF& offset)
{
    m_painter->translate(offset);
}

QRect QPainterProvider::window() const
{
    return m_painter->window();
}

void QPainterProvider::setWindow(const QRect& window)
{
    m_painter->setWindow(window);
}

QRect QPainterProvider::viewport() const
{
    return m_painter->viewport();
}

void QPainterProvider::setViewport(const QRect& viewport)
{
    m_painter->setViewport(viewport);
}

// drawing functions

void QPainterProvider::fillPath(const QPainterPath& path, const QBrush& brush)
{
    m_painter->fillPath(path, brush);
}

void QPainterProvider::drawPath(const QPainterPath& path)
{
    m_painter->drawPath(path);
}

void QPainterProvider::strokePath(const QPainterPath& path, const QPen& pen)
{
    m_painter->strokePath(path, pen);
}

void QPainterProvider::drawLines(const QLineF* lines, int lineCount)
{
    m_painter->drawLines(lines, lineCount);
}

void QPainterProvider::drawRects(const QRectF* rects, int rectCount)
{
    m_painter->drawRects(rects, rectCount);
}

void QPainterProvider::drawEllipse(const QRectF& rect)
{
    m_painter->drawEllipse(rect);
}

void QPainterProvider::drawPolyline(const QPointF* points, int pointCount)
{
    m_painter->drawPolyline(points, pointCount);
}

void QPainterProvider::drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule)
{
    m_painter->drawPolygon(points, pointCount, fillRule);
}

void QPainterProvider::drawConvexPolygon(const QPointF* points, int pointCount)
{
    m_painter->drawConvexPolygon(points, pointCount);
}

void QPainterProvider::drawText(const QPointF& point, const QString& text)
{
    m_painter->drawText(point, text);
}

void QPainterProvider::drawText(const QRectF& rect, int flags, const QString& text)
{
    m_painter->drawText(rect, flags, text);
}

void QPainterProvider::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    m_painter->drawGlyphRun(position, glyphRun);
}

void QPainterProvider::fillRect(const QRectF& rect, const QBrush& brush)
{
    m_painter->fillRect(rect, brush);
}

void QPainterProvider::drawPixmap(const QPointF& point, const QPixmap& pm)
{
    m_painter->drawPixmap(point, pm);
}

void QPainterProvider::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    m_painter->drawTiledPixmap(rect, pm, offset);
}
