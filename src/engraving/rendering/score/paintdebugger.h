/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_PAINTDEBUGGER_DEV_H
#define MU_ENGRAVING_PAINTDEBUGGER_DEV_H

#include "draw/ipaintprovider.h"

namespace mu::engraving::rendering::score {
class PaintDebugger : public muse::draw::IPaintProvider
{
public:
    PaintDebugger(muse::draw::IPaintProviderPtr real);

    muse::draw::IPaintProviderPtr realProvider() const;
    void setDebugPenColor(const muse::draw::Color& c);
    void restorePenColor();

    bool isActive() const override;
    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(muse::draw::Painter* painter) override;
    bool endTarget(bool endDraw = false) override;

    void beginObject(const std::string& name) override;
    void endObject() override;

    void setAntialiasing(bool arg) override;
    void setCompositionMode(muse::draw::CompositionMode mode) override;
    void setWindow(const muse::RectF& window) override;
    void setViewport(const muse::RectF& viewport) override;

    void setFont(const muse::draw::Font& font) override;
    const muse::draw::Font& font() const override;

    void setPen(const muse::draw::Pen& pen) override;
    void setNoPen() override;
    const muse::draw::Pen& pen() const override;

    void setBrush(const muse::draw::Brush& brush) override;
    const muse::draw::Brush& brush() const override;

    void save() override;
    void restore() override;

    void setTransform(const muse::draw::Transform& transform) override;
    const muse::draw::Transform& transform() const override;

    void drawPath(const muse::draw::PainterPath& path) override;
    void drawPolygon(const muse::PointF* points, size_t pointCount, muse::draw::PolygonMode mode) override;

    void drawText(const muse::PointF& point, const muse::String& text) override;
    void drawText(const muse::RectF& rect, int flags, const muse::String& text) override;
    void drawTextWorkaround(const muse::draw::Font& f, const muse::PointF& pos, const muse::String& text) override;

    void drawSymbol(const muse::PointF& point, char32_t ucs4Code) override;

    void drawPixmap(const muse::PointF& p, const muse::draw::Pixmap& pm) override;
    void drawTiledPixmap(const muse::RectF& rect, const muse::draw::Pixmap& pm, const muse::PointF& offset = muse::PointF()) override;

#ifndef NO_QT_SUPPORT
    void drawPixmap(const muse::PointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const muse::RectF& rect, const QPixmap& pm, const muse::PointF& offset = muse::PointF()) override;
#endif

    bool hasClipping() const override;

    void setClipRect(const muse::RectF& rect) override;
    void setClipping(bool enable) override;

private:
    muse::draw::IPaintProviderPtr m_real = nullptr;

    muse::draw::Color m_originalPenColor;
    muse::draw::Color m_debugPenColor;
};
}

#endif // MU_ENGRAVING_PAINTDEBUGGER_DEV_H
