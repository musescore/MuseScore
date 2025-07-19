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
#include "painter.h"

#include "types/brush.h"
#include "types/painterpath.h"

#ifndef NO_QT_SUPPORT
#include "internal/qpainterprovider.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::draw;

IPaintProviderPtr Painter::extended;
bool PainterItemMarker::enabled = true;

Painter::Painter(IPaintProviderPtr provider, const std::string& name)
    : m_provider(provider), m_name(name)
{
    init();
}

#ifndef NO_QT_SUPPORT
Painter::Painter(QPaintDevice* dp, const std::string& name)
    : m_name(name)
{
    m_provider = QPainterProvider::make(dp);
    init();
}

Painter::Painter(QPainter* qp, const std::string& name, bool ownsQPainter)
    : m_name(name)
{
    m_provider = QPainterProvider::make(qp, ownsQPainter);
    init();
}

#endif

Painter::~Painter()
{
    endTarget(false);
}

void Painter::init()
{
    m_provider->beginTarget(m_name);
    if (extended) {
        extended->beginTarget(m_name);
    }

    State st;
    st.worldTransform = m_provider->transform();
    st.isWxF = true;
    st.transform = st.worldTransform;

    m_states = std::stack<State>();
    m_states.push(std::move(st));
}

IPaintProviderPtr Painter::provider() const
{
    return m_provider;
}

void Painter::setProvider(const IPaintProviderPtr& p, bool reinit)
{
    m_provider = p;
    if (reinit) {
        init();
    }
}

bool Painter::endDraw()
{
    return endTarget(true);
}

bool Painter::endTarget(bool endDraw)
{
    m_provider->beforeEndTargetHook(this);
    if (extended) {
        extended->beforeEndTargetHook(this);
    }

    bool ok = m_provider->endTarget(endDraw);
    if (extended) {
        extended->endTarget(endDraw);
    }
    return ok;
}

bool Painter::isActive() const
{
    return m_provider->isActive();
}

void Painter::beginObject(const std::string& name)
{
    m_provider->beginObject(name);
    if (extended) {
        extended->beginObject(name);
    }
}

void Painter::endObject()
{
    m_provider->endObject();
    if (extended) {
        extended->endObject();
    }
}

void Painter::setAntialiasing(bool arg)
{
    m_provider->setAntialiasing(arg);
    if (extended) {
        extended->setAntialiasing(arg);
    }
}

void Painter::setCompositionMode(CompositionMode mode)
{
    m_provider->setCompositionMode(mode);
    if (extended) {
        extended->setCompositionMode(mode);
    }
}

void Painter::setFont(const Font& font)
{
    m_provider->setFont(font);
    if (extended) {
        extended->setFont(font);
    }
}

const Font& Painter::font() const
{
    return m_provider->font();
}

void Painter::setPen(const Pen& pen)
{
    m_provider->setPen(pen);
    if (extended) {
        extended->setPen(pen);
    }
}

void Painter::setNoPen()
{
    setPen(Pen(PenStyle::NoPen));
}

const Pen& Painter::pen() const
{
    return m_provider->pen();
}

void Painter::setBrush(const Brush& brush)
{
    m_provider->setBrush(brush);
    if (extended) {
        extended->setBrush(brush);
    }
}

const Brush& Painter::brush() const
{
    return m_provider->brush();
}

void Painter::save()
{
    State newSt = m_states.top();
    m_states.push(newSt);

    m_provider->save();
    if (extended) {
        extended->save();
    }
}

void Painter::restore()
{
    if (m_states.size() > 0) {
        m_states.pop();
    }

    m_provider->restore();
    if (extended) {
        extended->restore();
    }
}

void Painter::setWorldTransform(const Transform& matrix, bool combine)
{
    State& st = editableState();
    if (combine) {
        st.worldTransform = matrix * st.worldTransform;                        // combines
    } else {
        st.worldTransform = matrix;                                // set new matrix
    }
    st.isWxF = true;
    updateMatrix();
}

const Transform& Painter::worldTransform() const
{
    return state().worldTransform;
}

void Painter::scale(double sx, double sy)
{
    State& st = editableState();
    st.worldTransform.scale(sx, sy);
    st.isWxF = true;
    updateMatrix();
}

void Painter::rotate(double angle)
{
    State& st = editableState();
    st.worldTransform.rotate(angle);
    st.isWxF = true;
    updateMatrix();
}

void Painter::translate(double dx, double dy)
{
    State& st = editableState();
    st.worldTransform.translate(dx, dy);
    st.isWxF = true;
    updateMatrix();
}

RectF Painter::window() const
{
    return state().window;
}

void Painter::setWindow(const RectF& window)
{
    State& st = editableState();
    st.window = window;
    st.isVxF = true;
    updateViewTransform();

    // for debug purpose
    m_provider->setWindow(window);
    if (extended) {
        extended->setWindow(window);
    }
}

RectF Painter::viewport() const
{
    return state().viewport;
}

void Painter::setViewport(const RectF& viewport)
{
    State& st = editableState();
    st.viewport = viewport;
    st.isVxF = true;
    updateViewTransform();

    // for debug purpose
    m_provider->setViewport(viewport);
    if (extended) {
        extended->setViewport(viewport);
    }
}

// drawing functions

void Painter::fillPath(const PainterPath& path, const Brush& brush)
{
    Pen oldPen = this->pen();
    Brush oldBrush = this->brush();
    setPen(Pen(PenStyle::NoPen));
    setBrush(brush);

    drawPath(path);

    setPen(oldPen);
    setBrush(oldBrush);
}

void Painter::strokePath(const PainterPath& path, const Pen& pen)
{
    Pen oldPen = this->pen();
    Brush oldBrush = this->brush();
    setPen(pen);
    setBrush(BrushStyle::NoBrush);

    drawPath(path);

    setPen(oldPen);
    setBrush(oldBrush);
}

void Painter::drawPath(const PainterPath& path)
{
    m_provider->drawPath(path);
    if (extended) {
        extended->drawPath(path);
    }
}

void Painter::drawLines(const LineF* lines, size_t lineCount)
{
    for (size_t i = 0; i < lineCount; ++i) {
        PointF pts[2] = { lines[i].p1(), lines[i].p2() };
        if (pts[0] == pts[1]) {
            LOGE() << "draw point not implemented";
        }
        drawPolyline(pts, 2);
    }
}

void Painter::drawLines(const PointF* pointPairs, size_t lineCount)
{
    static_assert(sizeof(LineF) == 2 * sizeof(PointF), "must be: sizeof(LineF) == 2 * sizeof(PointF)");

    drawLines((const LineF*)pointPairs, lineCount);
}

void Painter::drawRects(const RectF* rects, size_t rectCount)
{
    for (size_t i = 0; i < rectCount; ++i) {
        PainterPath path;
        path.addRect(rects[i]);
        if (path.isEmpty()) {
            continue;
        }
        drawPath(path);
    }
}

void Painter::drawEllipse(const RectF& rect)
{
    PainterPath path;
    path.addEllipse(rect);
    m_provider->drawPath(path);
    if (extended) {
        extended->drawPath(path);
    }
}

void Painter::drawPolyline(const PointF* points, size_t pointCount)
{
    m_provider->drawPolygon(points, pointCount, PolygonMode::Polyline);
    if (extended) {
        extended->drawPolygon(points, pointCount, PolygonMode::Polyline);
    }
}

void Painter::drawPolygon(const PointF* points, size_t pointCount, FillRule fillRule)
{
    PolygonMode mode = (fillRule == FillRule::OddEvenFill) ? PolygonMode::OddEven : PolygonMode::Winding;
    m_provider->drawPolygon(points, pointCount, mode);
    if (extended) {
        extended->drawPolygon(points, pointCount, mode);
    }
}

void Painter::drawConvexPolygon(const PointF* points, size_t pointCount)
{
    m_provider->drawPolygon(points, pointCount, PolygonMode::Convex);
    if (extended) {
        extended->drawPolygon(points, pointCount, PolygonMode::Convex);
    }
}

void Painter::drawArc(const RectF& r, int a, int alen)
{
    //! NOTE Copied from QPainter source code

    RectF rect = r.normalized();

    PainterPath path;
    path.arcMoveTo(rect, a / 16.0);
    path.arcTo(rect, a / 16.0, alen / 16.0);
    strokePath(path, pen());
}

void Painter::drawRoundedRect(const RectF& rect, double xRadius, double yRadius)
{
    if (xRadius <= 0 || yRadius <= 0) {             // draw normal rectangle
        drawRect(rect);
        return;
    }

    PainterPath path;
    path.addRoundedRect(rect, xRadius, yRadius);
    drawPath(path);
}

void Painter::drawText(const PointF& point, const String& text)
{
    m_provider->drawText(point, text);
    if (extended) {
        extended->drawText(point, text);
    }
}

void Painter::drawText(const RectF& rect, int flags, const String& text)
{
    m_provider->drawText(rect, flags, text);
    if (extended) {
        extended->drawText(rect, flags, text);
    }
}

void Painter::drawTextWorkaround(Font& f, const PointF pos, const String& text)
{
    m_provider->drawTextWorkaround(f, pos, text);
    if (extended) {
        extended->drawTextWorkaround(f, pos, text);
    }
}

void Painter::drawSymbol(const PointF& point, char32_t ucs4Code)
{
    m_provider->drawSymbol(point, ucs4Code);
    if (extended) {
        extended->drawSymbol(point, ucs4Code);
    }
}

void Painter::fillRect(const RectF& rect, const Brush& brush)
{
    Pen oldPen = this->pen();
    Brush oldBrush = this->brush();
    setPen(Pen(PenStyle::NoPen));
    setBrush(brush);

    drawRect(rect);

    setBrush(oldBrush);
    setPen(oldPen);
}

void Painter::drawPixmap(const PointF& point, const Pixmap& pm)
{
    m_provider->drawPixmap(point, pm);
    if (extended) {
        extended->drawPixmap(point, pm);
    }
}

void Painter::drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset)
{
    m_provider->drawTiledPixmap(rect, pm, offset);
    if (extended) {
        extended->drawTiledPixmap(rect, pm, offset);
    }
}

#ifndef NO_QT_SUPPORT
void Painter::drawPixmap(const PointF& point, const QPixmap& pm)
{
    m_provider->drawPixmap(point, pm);
    if (extended) {
        extended->drawPixmap(point, pm);
    }
}

void Painter::drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset)
{
    m_provider->drawTiledPixmap(rect, pm, offset);
    if (extended) {
        extended->drawTiledPixmap(rect, pm, offset);
    }
}

#endif

Painter::State& Painter::editableState()
{
    return m_states.top();
}

const Painter::State& Painter::state() const
{
    return m_states.top();
}

void Painter::updateViewTransform()
{
    State& st = editableState();
    double scaleW = double(st.viewport.width()) / double(st.window.width());
    double scaleH = double(st.viewport.height()) / double(st.window.height());
    st.viewTransform = Transform(scaleW, 0, 0, scaleH, st.viewport.x() - st.window.x() * scaleW, st.viewport.y() - st.window.y() * scaleH);
    updateMatrix();
}

void Painter::updateMatrix()
{
    Painter::State& st = editableState();
    st.transform = st.isWxF ? st.worldTransform : Transform();
    if (st.isVxF) {
        st.transform *= st.viewTransform;
    }

    m_provider->setTransform(st.transform);
    if (extended) {
        extended->setTransform(st.transform);
    }
}

bool Painter::hasClipping() const
{
    return m_provider->hasClipping();
}

void Painter::setClipRect(const RectF& rect)
{
    m_provider->setClipRect(rect);
}

void Painter::setMask(const RectF& background, const std::vector<RectF>& maskRects)
{
    m_provider->setMask(background, maskRects);
}

void Painter::setClipping(bool enable)
{
    m_provider->setClipping(enable);
}
