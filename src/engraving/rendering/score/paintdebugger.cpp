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
#include "paintdebugger.h"

using namespace mu;
using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

PaintDebugger::PaintDebugger(IPaintProviderPtr real)
    : m_real(real)
{
}

IPaintProviderPtr PaintDebugger::realProvider() const
{
    return m_real;
}

void PaintDebugger::setDebugPenColor(const Color& c)
{
    m_debugPenColor = c;
    const Pen& originalPen = pen();
    m_originalPenColor = originalPen.color();
    Pen p = originalPen;
    p.setColor(c);
    setPen(p);
}

void PaintDebugger::restorePenColor()
{
    Pen p = pen();
    p.setColor(m_originalPenColor);
    setPen(p);
    m_debugPenColor = Color();
}

void PaintDebugger::beginTarget(const std::string& name)
{
    m_real->beginTarget(name);
}

void PaintDebugger::beforeEndTargetHook(Painter* p)
{
    m_real->beforeEndTargetHook(p);
}

bool PaintDebugger::endTarget(bool endDraw)
{
    return m_real->endTarget(endDraw);
}

bool PaintDebugger::isActive() const
{
    return m_real->isActive();
}

void PaintDebugger::beginObject(const std::string& name)
{
    m_real->beginObject(name);
}

void PaintDebugger::endObject()
{
    m_real->endObject();
}

void PaintDebugger::setAntialiasing(bool arg)
{
    m_real->setAntialiasing(arg);
}

void PaintDebugger::setCompositionMode(CompositionMode mode)
{
    m_real->setCompositionMode(mode);
}

void PaintDebugger::setWindow(const RectF& window)
{
    m_real->setWindow(window);
}

void PaintDebugger::setViewport(const RectF& viewport)
{
    m_real->setViewport(viewport);
}

void PaintDebugger::setFont(const Font& f)
{
    m_real->setFont(f);
}

const Font& PaintDebugger::font() const
{
    return m_real->font();
}

void PaintDebugger::setPen(const Pen& pen)
{
    Pen p = pen;
    if (m_debugPenColor.isValid()) {
        p.setColor(m_debugPenColor);
    }
    m_real->setPen(p);
}

void PaintDebugger::setNoPen()
{
    m_real->setNoPen();
}

const Pen& PaintDebugger::pen() const
{
    return m_real->pen();
}

void PaintDebugger::setBrush(const Brush& brush)
{
    m_real->setBrush(brush);
}

const Brush& PaintDebugger::brush() const
{
    return m_real->brush();
}

void PaintDebugger::save()
{
    m_real->save();
}

void PaintDebugger::restore()
{
    m_real->restore();
}

void PaintDebugger::setTransform(const Transform& transform)
{
    m_real->setTransform(transform);
}

const Transform& PaintDebugger::transform() const
{
    return m_real->transform();
}

// drawing functions

void PaintDebugger::drawPath(const PainterPath& path)
{
    m_real->drawPath(path);
}

void PaintDebugger::drawPolygon(const PointF* points, size_t pointCount, PolygonMode mode)
{
    m_real->drawPolygon(points, pointCount, mode);
}

void PaintDebugger::drawText(const PointF& point, const String& text)
{
    m_real->drawText(point, text);
}

void PaintDebugger::drawText(const RectF& rect, int flags, const String& text)
{
    m_real->drawText(rect, flags, text);
}

void PaintDebugger::drawTextWorkaround(const Font& f, const PointF& pos, const String& text)
{
    m_real->drawTextWorkaround(f, pos, text);
}

void PaintDebugger::drawSymbol(const PointF& point, char32_t ucs4Code)
{
    m_real->drawSymbol(point, ucs4Code);
}

void PaintDebugger::drawPixmap(const PointF& p, const Pixmap& pm)
{
    m_real->drawPixmap(p, pm);
}

void PaintDebugger::drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset)
{
    m_real->drawTiledPixmap(rect, pm, offset);
}

#ifndef NO_QT_SUPPORT
void PaintDebugger::drawPixmap(const PointF& point, const QPixmap& pm)
{
    m_real->drawPixmap(point, pm);
}

void PaintDebugger::drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset)
{
    m_real->drawTiledPixmap(rect, pm, offset);
}

#endif

bool PaintDebugger::hasClipping() const
{
    return m_real->hasClipping();
}

void PaintDebugger::setClipRect(const RectF& rect)
{
    m_real->setClipRect(rect);
}

void PaintDebugger::setClipping(bool enable)
{
    m_real->setClipping(enable);
}
