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
#ifndef MUSE_DRAW_IPAINTPROVIDER_H
#define MUSE_DRAW_IPAINTPROVIDER_H

#include <memory>

#include "types/brush.h"
#include "types/drawtypes.h"
#include "types/geometry.h"
#include "types/font.h"
#include "types/pen.h"
#include "types/pixmap.h"
#include "types/transform.h"
#include "types/painterpath.h"

namespace muse::draw {
class Painter;
class IPaintProvider
{
public:
    virtual ~IPaintProvider() = default;

    virtual bool isActive() const = 0;
    virtual void beginTarget(const std::string& name) = 0;
    virtual void beforeEndTargetHook(Painter* painter) = 0;
    virtual bool endTarget(bool endDraw = false) = 0;
    virtual void beginObject(const std::string& name) = 0;
    virtual void endObject() = 0;

    virtual void setAntialiasing(bool arg) = 0;
    virtual void setCompositionMode(CompositionMode mode) = 0;
    virtual void setWindow(const RectF& window) = 0;
    virtual void setViewport(const RectF& viewport) = 0;

    virtual void setFont(const Font& font) = 0;
    virtual const Font& font() const = 0;

    virtual void setPen(const Pen& pen) = 0;
    virtual void setNoPen() = 0;
    virtual const Pen& pen() const = 0;

    virtual void setBrush(const Brush& brush) = 0;
    virtual const Brush& brush() const = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual void setTransform(const Transform& transform) = 0;
    virtual const Transform& transform() const = 0;

    // drawing functions
    virtual void drawPath(const PainterPath& path) = 0;
    virtual void drawPolygon(const PointF* points, size_t pointCount, PolygonMode mode) = 0;

    virtual void drawText(const PointF& point, const String& text) = 0;
    virtual void drawText(const RectF& rect, int flags, const String& text) = 0;
    virtual void drawTextWorkaround(const Font& f, const PointF& pos, const String& text) = 0; // see Painter::drawTextWorkaround .h file

    virtual void drawSymbol(const PointF& point, char32_t ucs4Code) = 0;

    virtual void drawPixmap(const PointF& point, const Pixmap& pm) = 0;
    virtual void drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset = PointF()) = 0;

#ifndef NO_QT_SUPPORT
    virtual void drawPixmap(const PointF& point, const QPixmap& pm) = 0;
    virtual void drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset = PointF()) = 0;
#endif

    virtual bool hasClipping() const = 0;

    virtual void setClipRect(const RectF& rect) = 0;
    virtual void setMask(const RectF& background, const std::vector<RectF>& maskRects) = 0;
    virtual void setClipping(bool enable) = 0;
};

using IPaintProviderPtr = std::shared_ptr<IPaintProvider>;
}

#endif // MUSE_DRAW_IPAINTPROVIDER_H
