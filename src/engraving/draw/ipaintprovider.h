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
#ifndef MU_DRAW_IPAINTPROVIDER_H
#define MU_DRAW_IPAINTPROVIDER_H

#include <memory>

#include <QPoint>
#include <QPointF>
#include <QPen>
#include <QColor>
#include <QFont>

#include "drawtypes.h"
#include "font.h"

namespace mu::draw {
class Painter;
class IPaintProvider
{
public:
    virtual ~IPaintProvider() = default;

    virtual QPaintDevice* device() const = 0;
    virtual QPainter* qpainter() const = 0;

    virtual bool isActive() const = 0;
    virtual void beginTarget(const std::string& name) = 0;
    virtual void beforeEndTargetHook(Painter* painter) = 0;
    virtual bool endTarget(bool endDraw = false) = 0;
    virtual void beginObject(const std::string& name, const QPointF& pagePos) = 0;
    virtual void endObject() = 0;

    virtual void setAntialiasing(bool arg) = 0;
    virtual void setCompositionMode(CompositionMode mode) = 0;

    virtual void setFont(const Font& font) = 0;
    virtual const Font& font() const = 0;

    virtual void setPen(const QPen& pen) = 0;
    virtual void setNoPen() = 0;
    virtual const QPen& pen() const = 0;

    virtual void setBrush(const QBrush& brush) = 0;
    virtual const QBrush& brush() const = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual void setTransform(const QTransform& transform) = 0;
    virtual const QTransform& transform() const = 0;

    // drawing functions
    virtual void drawPath(const QPainterPath& path) = 0;
    virtual void drawPolygon(const QPointF* points, int pointCount, PolygonMode mode) = 0;

    virtual void drawText(const QPointF& point, const QString& text) = 0;
    virtual void drawText(const QRectF& rect, int flags, const QString& text) = 0;
    virtual void drawTextWorkaround(mu::draw::Font& f, const QPointF& pos, const QString& text) = 0; // see Painter::drawTextWorkaround .h file

    virtual void drawSymbol(const QPointF& point, uint ucs4Code) = 0;

    virtual void drawPixmap(const QPointF& point, const QPixmap& pm) = 0;
    virtual void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF()) = 0;
};

using IPaintProviderPtr = std::shared_ptr<IPaintProvider>;
}

#endif // MU_DRAW_IPAINTPROVIDER_H
