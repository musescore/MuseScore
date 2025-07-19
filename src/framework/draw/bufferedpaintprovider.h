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
#ifndef MUSE_DRAW_BUFFEREDPAINTPROVIDER_H
#define MUSE_DRAW_BUFFEREDPAINTPROVIDER_H

#include "ipaintprovider.h"
#include "types/drawdata.h"
#include "types/pen.h"
#include "types/brush.h"

namespace muse::draw {
class DrawObjectsLogger;
class BufferedPaintProvider : public IPaintProvider
{
public:
    BufferedPaintProvider();
    ~BufferedPaintProvider();

    bool isActive() const override;
    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(Painter* painter) override;
    bool endTarget(bool endDraw = false) override;

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

    void drawPixmap(const PointF& p, const Pixmap& pm) override;
    void drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset = PointF()) override;

#ifndef NO_QT_SUPPORT
    void drawPixmap(const PointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset = PointF()) override;
#endif

    bool hasClipping() const override;

    void setClipRect(const RectF& rect) override;
    void setMask(const RectF& background, const std::vector<RectF>& maskRects) override;
    void setClipping(bool enable) override;

    // ---

    DrawDataPtr drawData() const;
    void clear();

private:

    const DrawData::Item& currentItem() const;
    DrawData::Item& editableItem();

    const DrawData::Data& currentData() const;
    DrawData::Data& editableData();

    const DrawData::State& currentState() const;
    DrawData::State& editableState();

    void ensureItemInit(DrawData::Item& item) const;

    DrawDataPtr m_buf = nullptr;
    int m_itemLevel = -1;
    bool m_stateIsUsed = false;
    int m_currentStateNo = 0;
    bool m_isActive = false;
    DrawObjectsLogger* m_drawObjectsLogger = nullptr;
};
}

#endif // MUSE_DRAW_BUFFEREDPAINTPROVIDER_H
