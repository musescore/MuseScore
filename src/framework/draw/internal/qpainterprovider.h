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
#pragma once

#include "../ipaintprovider.h"

class QPainter;
class QImage;

namespace muse::draw {
class DrawObjectsLogger;
class QPainterProvider : public IPaintProvider
{
public:
    QPainterProvider(QPainter* painter, bool ownsPainter = false);
    ~QPainterProvider();

    static IPaintProviderPtr make(QPaintDevice* dp);
    static IPaintProviderPtr make(QPainter* qp, bool ownsPainter = false);

    QPainter* qpainter() const;

    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(Painter* painter) override;
    bool endTarget(bool endDraw = false) override;
    bool isActive() const override;

    void beginObject(const std::string& name) override;
    void endObject() override;

    void setAntialiasing(bool arg) override;
    void setCompositionMode(CompositionMode mode) override;
    void setWindow(const RectF& window) override;
    void setViewport(const RectF& viewport) override;

    void setFont(const Font& font) override;
    const Font& font() const override;

    void setPen(const Pen& pen) override;
    void setNoPen() override;
    const Pen& pen() const override;

    void setBrush(const Brush& brush) override;
    const Brush& brush() const override;

    void save() override;
    void restore() override;

    void setTransform(const Transform& transform) override;
    const Transform& transform() const override;

    // drawing functions
    void drawPath(const PainterPath& path) override;
    void drawPolygon(const PointF* points, size_t pointCount, PolygonMode mode) override;

    void drawText(const PointF& point, const String& text) override;
    void drawText(const RectF& rect, int flags, const String& text) override;
    void drawTextWorkaround(const Font& f, const PointF& pos, const String& text) override;

    void drawSymbol(const PointF& point, char32_t ucs4Code) override;

    void drawPixmap(const PointF& point, const Pixmap& pm) override;
    void drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset = PointF()) override;

    void drawPixmap(const PointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset = PointF()) override;

    bool hasClipping() const override;

    void setClipRect(const RectF& rect) override;
    void setClipping(bool enable) override;

protected:
    QPainter* m_painter = nullptr;

private:
    bool m_ownsPainter = false;
    DrawObjectsLogger* m_drawObjectsLogger = nullptr;
    Font m_font;
    Pen m_pen;
    Brush m_brush;

    Transform m_transform;
};
}
