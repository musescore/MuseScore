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
#ifndef MU_DRAW_QPAINTERPROVIDER_H
#define MU_DRAW_QPAINTERPROVIDER_H

#include "ipaintprovider.h"

class QPainter;
class QImage;

namespace mu::draw {
class DrawObjectsLogger;
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

    void setFont(const Font& font) override;
    const Font& font() const override;

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
    void drawTextWorkaround(mu::draw::Font& f, const QPointF& pos, const QString& text) override;

    void drawSymbol(const QPointF& point, uint ucs4Code) override;

    void drawPixmap(const QPointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) override;

private:
    QPainter* m_painter = nullptr;
    Font m_font;
    bool m_overship = false;
    DrawObjectsLogger* m_drawObjectsLogger = nullptr;

    QTransform m_transform;
};
}

#endif // MU_DRAW_QPAINTERPROVIDER_H
