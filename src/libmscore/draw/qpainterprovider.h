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
#include "drawlogger.h"

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

    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(Painter* painter) override;
    bool endTarget(bool endDraw = false) override;
    bool isActive() const override;

    void beginObject(const std::string& name, const QPointF& pagePos) override;
    void endObject() override;

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

    void setTransform(const QTransform& transform) override;
    const QTransform& transform() const override;

    // drawing functions
    void drawPath(const QPainterPath& path) override;
    void drawPolygon(const QPointF* points, int pointCount, PolygonMode mode) override;

    void drawText(const QPointF& point, const QString& text) override;
    void drawText(const QRectF& rect, int flags, const QString& text) override;

    void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun) override;

    void drawPixmap(const QPointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) override;

private:
    QPainter* m_painter = nullptr;
    bool m_overship = false;
    DrawObjectsLogger m_drawObjectsLogger;

    QTransform m_transform;
};
}

#endif // MU_DRAW_QPAINTERPROVIDER_H
