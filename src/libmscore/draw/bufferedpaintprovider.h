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
#ifndef MU_DRAW_BUFFEREDPAINTPROVIDER_H
#define MU_DRAW_BUFFEREDPAINTPROVIDER_H

#include <vector>
#include <stack>

#include <QPen>
#include <QBrush>
#include <QFont>

#include "ipaintprovider.h"
#include "buffereddrawtypes.h"

namespace mu::draw {
class DrawObjectsLogger;
class BufferedPaintProvider : public IPaintProvider
{
public:
    BufferedPaintProvider();
    ~BufferedPaintProvider();

    QPaintDevice* device() const override;
    QPainter* qpainter() const override;

    bool isActive() const override;
    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(Painter* painter) override;
    bool endTarget(bool endDraw = false) override;

    void beginObject(const std::string& name, const QPointF& pagePos) override;
    void endObject() override;

    void setAntialiasing(bool arg) override;
    void setCompositionMode(CompositionMode mode) override;

    void setFont(const QFont& font) override;
    const QFont& qFont() const override;
    void setFont(const Font& font) override;
    Font font() const override;

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

    void drawPixmap(const QPointF& p, const QPixmap& pm) override;
    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) override;

    // ---

    const DrawData& drawData() const;
    void clear();

private:

    const DrawData::Data& currentData() const;
    DrawData::Data& editableData();

    const DrawData::State& currentState() const;
    DrawData::State& editableState();

    DrawData m_buf;
    std::stack<DrawData::Object> m_currentObjects;
    bool m_isActive = false;
    DrawObjectsLogger* m_drawObjectsLogger = nullptr;
};
}

#endif // MU_DRAW_BUFFEREDPAINTPROVIDER_H
