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
#ifndef MU_ENGRAVING_PAINTDEBUGGER_H
#define MU_ENGRAVING_PAINTDEBUGGER_H

#include "infrastructure/draw/ipaintprovider.h"

namespace mu::engraving {
class PaintDebugger : public draw::IPaintProvider
{
public:
    PaintDebugger(draw::IPaintProviderPtr real);

    draw::IPaintProviderPtr realProvider() const;
    void setDebugPenColor(const draw::Color& c);
    void restorePenColor();

    bool isActive() const override;
    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(draw::Painter* painter) override;
    bool endTarget(bool endDraw = false) override;

    void beginObject(const std::string& name, const PointF& pagePos) override;
    void endObject() override;

    void setAntialiasing(bool arg) override;
    void setCompositionMode(draw::CompositionMode mode) override;

    void setFont(const draw::Font& font) override;
    const draw::Font& font() const override;

    void setPen(const draw::Pen& pen) override;
    void setNoPen() override;
    const draw::Pen& pen() const override;

    void setBrush(const draw::Brush& brush) override;
    const draw::Brush& brush() const override;

    void save() override;
    void restore() override;

    void setTransform(const Transform& transform) override;
    const Transform& transform() const override;

    void drawPath(const PainterPath& path) override;
    void drawPolygon(const PointF* points, size_t pointCount, draw::PolygonMode mode) override;

    void drawText(const PointF& point, const QString& text) override;
    void drawText(const RectF& rect, int flags, const QString& text) override;
    void drawTextWorkaround(const draw::Font& f, const PointF& pos, const QString& text) override;

    void drawSymbol(const PointF& point, uint ucs4Code) override;

    void drawPixmap(const PointF& p, const draw::Pixmap& pm) override;
    void drawTiledPixmap(const RectF& rect, const draw::Pixmap& pm, const PointF& offset = PointF()) override;

#ifndef NO_QT_SUPPORT
    void drawPixmap(const PointF& point, const QPixmap& pm) override;
    void drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset = PointF()) override;
#endif

    void setClipRect(const RectF& rect) override;
    void setClipping(bool enable) override;

private:
    draw::IPaintProviderPtr m_real = nullptr;

    draw::Color m_originalPenColor;
    draw::Color m_debugPenColor;
};
}

#endif // MU_ENGRAVING_PAINTDEBUGGER_H
