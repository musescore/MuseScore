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
#ifndef MU_DRAW_IPAINTPROVIDER_H
#define MU_DRAW_IPAINTPROVIDER_H

#include <memory>

#include <QPoint>
#include <QPointF>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QGlyphRun>

#include "drawtypes.h"

namespace mu::draw {
class IPaintProvider
{
public:
    virtual ~IPaintProvider() = default;

    virtual QPaintDevice* device() const = 0;
    virtual QPainter* qpainter() const = 0;

    virtual void begin(const std::string& name) = 0;
    virtual bool end(const std::string& name) = 0;
    virtual bool isActive() const = 0;

    virtual void beginObject(const std::string& name, const QPointF& pagePos) = 0;
    virtual void endObject(const std::string& name, const QPointF& pagePos) = 0;

    virtual void setAntialiasing(bool arg) = 0;
    virtual void setCompositionMode(CompositionMode mode) = 0;

    virtual void setFont(const QFont& font) = 0;
    virtual const QFont& font() const = 0;

    virtual void setPen(const QPen& pen) = 0;
    virtual void setNoPen() = 0;
    virtual const QPen& pen() const = 0;

    virtual void setBrush(const QBrush& brush) = 0;
    virtual const QBrush& brush() const = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual void setWorldTransform(const QTransform& matrix, bool combine = false) = 0;
    virtual const QTransform& worldTransform() const = 0;

    virtual void setTransform(const QTransform& transform, bool combine = false) = 0;
    virtual const QTransform& transform() const = 0;

    virtual void scale(qreal sx, qreal sy) = 0;
    virtual void rotate(qreal angle) = 0;

    virtual void translate(const QPointF& offset) = 0;

    virtual QRect window() const = 0;
    virtual void setWindow(const QRect& window) = 0;
    virtual QRect viewport() const = 0;
    virtual void setViewport(const QRect& viewport) = 0;

    // drawing functions
    virtual void fillPath(const QPainterPath& path, const QBrush& brush) = 0;
    virtual void drawPath(const QPainterPath& path) = 0;
    virtual void strokePath(const QPainterPath& path, const QPen& pen) = 0;

    virtual void drawLines(const QLineF* lines, int lineCount) = 0;

    virtual void drawRects(const QRectF* rects, int rectCount) = 0;

    virtual void drawEllipse(const QRectF& rect) = 0;

    virtual void drawPolyline(const QPointF* points, int pointCount) = 0;

    virtual void drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill) = 0;
    virtual void drawConvexPolygon(const QPointF* points, int pointCount) = 0;

    virtual void drawText(const QPointF& point, const QString& text) = 0;
    virtual void drawText(const QRectF& rect, int flags, const QString& text) = 0;

    virtual void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun) = 0;

    virtual void fillRect(const QRectF& rect, const QBrush& brush) = 0;

    virtual void drawPixmap(const QPointF& point, const QPixmap& pm) = 0;
    virtual void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) = 0;
};

using IPaintProviderPtr = std::shared_ptr<IPaintProvider>;
}

#endif // MU_DRAW_IPAINTPROVIDER_H
