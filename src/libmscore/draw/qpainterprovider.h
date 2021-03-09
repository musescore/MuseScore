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
#ifndef MU_DRAW_QPAINTERPROVIDER_H
#define MU_DRAW_QPAINTERPROVIDER_H

#include "ipaintprovider.h"
#include "drawobjectslogger.h"

class QPainter;
class QImage;

namespace mu::draw {
class QPainterProvider : public IPaintProvider
{
public:
    QPainterProvider(QPainter* painter, bool overship = false);
    ~QPainterProvider();

    static IPaintProviderPtr make(QPaintDevice* dp);
    static IPaintProviderPtr make(QPainter* qp, bool overship = false);

    QPaintDevice* device() const override;
    QPainter* qpainter() const override;

    void begin(const std::string& name) override;
    bool end(const std::string& name) override;
    bool isActive() const override;

    void beginObject(const std::string& name, const QPointF& pagePos) override;
    void endObject(const std::string& name, const QPointF& pagePos) override;

    void setAntialiasing(bool arg) override;
    void setCompositionMode(CompositionMode mode) override;

    void setFont(const QFont& font) override;
    const QFont& font() const override;

    void setPen(const QPen& pen) override;
    void setNoPen() override;
    const QPen& pen() const override;

    void setBrush(const QBrush& brush) override;
    const QBrush& brush() const override;

    void save() override;
    void restore() override;

    void setWorldTransform(const QTransform& matrix, bool combine = false) override;
    const QTransform& worldTransform() const override;

    void setTransform(const QTransform& transform, bool combine = false) override;
    const QTransform& transform() const override;

    void scale(qreal sx, qreal sy) override;
    void rotate(qreal angle) override;

    void translate(const QPointF& offset) override;

    QRect window() const override;
    void setWindow(const QRect& window) override;
    QRect viewport() const override;
    void setViewport(const QRect& viewport) override;

    // drawing functions
    void fillPath(const QPainterPath& path, const QBrush& brush) override;
    void drawPath(const QPainterPath& path) override;
    void strokePath(const QPainterPath& path, const QPen& pen) override;

    void drawLines(const QLineF* lines, int lineCount) override;

    void drawRects(const QRectF* rects, int rectCount) override;

    void drawEllipse(const QRectF& rect) override;

    void drawPolyline(const QPointF* points, int pointCount) override;

    void drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill) override;
    void drawConvexPolygon(const QPointF* points, int pointCount) override;

    void drawText(const QPointF& point, const QString& text) override;
    void drawText(const QRectF& rect, int flags, const QString& text) override;

    void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun) override;

    void fillRect(const QRectF& rect, const QBrush& brush) override;

    void drawPixmap(const QPointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) override;

private:
    QPainter* m_painter = nullptr;
    bool m_overship = false;
    DrawObjectsLogger m_drawObjectsLogger;
};
}

#endif // MU_DRAW_QPAINTERPROVIDER_H
