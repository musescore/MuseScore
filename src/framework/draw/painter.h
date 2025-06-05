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

#include <stack>

#include "global/types/string.h"

#include "types/color.h"
#include "types/geometry.h"
#include "types/drawtypes.h"
#include "types/font.h"
#include "types/pen.h"
#include "types/pixmap.h"

#include "ipaintprovider.h"

class QPaintDevice;

#ifndef NO_QT_SUPPORT
class QPainter;
#endif

namespace muse::draw {
class Painter
{
public:
    Painter(IPaintProviderPtr provider, const std::string& name);

#ifndef NO_QT_SUPPORT
    Painter(QPaintDevice* dp, const std::string& name);
    Painter(QPainter* qp, const std::string& name, bool ownsQPainter = false);
#endif

    ~Painter();

    IPaintProviderPtr provider() const;
    void setProvider(const IPaintProviderPtr& p, bool reinit = true);

    bool isActive() const;
    bool endDraw();

    //! NOTE These are methods for debugging and automated testing.
    void beginObject(const std::string& name);
    void endObject();

    // state
    void setAntialiasing(bool arg);
    void setCompositionMode(CompositionMode mode);

    void setFont(const Font& font);
    const Font& font() const;

    void setPen(const Pen& pen);
    inline void setPen(const Color& color);
    void setNoPen();
    const Pen& pen() const;

    void setBrush(const Brush& brush);
    const Brush& brush() const;

    void setWorldTransform(const Transform& matrix, bool combine = false);
    const Transform& worldTransform() const;
    void scale(double sx, double sy);
    void rotate(double angle);
    void translate(double dx, double dy);
    inline void translate(const PointF& offset);

    RectF window() const;
    void setWindow(const RectF& window);
    RectF viewport() const;
    void setViewport(const RectF& viewport);

    void save();
    void restore();

    // drawing
    void fillPath(const PainterPath& path, const Brush& brush);
    void drawPath(const PainterPath& path);
    void strokePath(const PainterPath& path, const Pen& pen);

    void drawLines(const LineF* lines, size_t lineCount);
    void drawLines(const PointF* pointPairs, size_t lineCount);
    inline void drawLine(const LineF& line);
    inline void drawLine(const PointF& p1, const PointF& p2);
    inline void drawLine(double x1, double y1, double x2, double y2);
    inline void drawLines(const std::vector<LineF>& lines);

    inline void drawRect(const RectF& rect);
    inline void drawRect(double x1, double y1, double w, double h);

    void drawRects(const RectF* rects, size_t rectCount);

    void drawRoundedRect(const RectF& rect, double xRadius, double yRadius);

    void drawEllipse(const RectF& rect);
    inline void drawEllipse(const PointF& center, double rx, double ry);

    void drawPolyline(const PointF* points, size_t pointCount);
    inline void drawPolyline(const PolygonF& polyline);

    void drawPolygon(const PointF* points, size_t pointCount, FillRule fillRule = FillRule::OddEvenFill);
    inline void drawPolygon(const PolygonF& polygon, FillRule fillRule = FillRule::OddEvenFill);

    void drawConvexPolygon(const PointF* points, size_t pointCount);
    inline void drawConvexPolygon(const PolygonF& polygon);

    void drawArc(const RectF& rect, int a, int alen);

    void drawText(const PointF& point, const String& text);
    inline void drawText(double x, double y, const String& text);

    void drawText(const RectF& rect, int flags, const String& text);

    //! NOTE workaround for https://musescore.org/en/node/284218
    //! and https://musescore.org/en/node/281601
    //! only needed for certain artificially emboldened fonts
    //! see https://musescore.org/en/node/281601#comment-900261
    //! in Qt 5.12.x this workaround should be no more necessary if
    //! env variable QT_MAX_CACHED_GLYPH_SIZE is set to 1.
    //! The workaround works badly if the text is at the same time
    //! bold and underlined or struck out.
    //! (moved from TextBase::drawTextWorkaround)
    void drawTextWorkaround(Font& f, const PointF pos, const String& text);

    void drawSymbol(const PointF& point, char32_t ucs4Code);

    void fillRect(const RectF& rect, const Brush& brush);

    void drawPixmap(const PointF& point, const Pixmap& pm);
    void drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset = PointF());

#ifndef NO_QT_SUPPORT
    void drawPixmap(const PointF& point, const QPixmap& pm);
    void drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset = PointF());
#endif

    bool hasClipping() const;

    void setClipRect(const RectF& rect);
    void setMask(const RectF& background, const std::vector<RectF>& maskRects);
    void setClipping(bool enable);

    //! NOTE Provider for tests.
    //! We're not ready to use DI (ModuleIoC) here yet
    static IPaintProviderPtr extended;

private:

    struct State {
        RectF window;
        RectF viewport;
        bool isVxF = false;
        Transform viewTransform;
        bool isWxF = false;
        Transform worldTransform;       // World transformation matrix, not window and viewport
        Transform transform;            // Complete transformation matrix
    };

    void init();
    State& editableState();
    const State& state() const;
    void updateViewTransform();
    void updateMatrix();

    bool endTarget(bool endDraw);

    IPaintProviderPtr m_provider;
    std::string m_name;
    std::stack<State> m_states;
};

inline void Painter::setPen(const Color& color)
{
    setPen(Pen(color.isValid() ? color : Color::BLACK));
}

inline void Painter::translate(const PointF& offset)
{
    translate(offset.x(), offset.y());
}

inline void Painter::drawLine(const LineF& l)
{
    drawLines(&l, 1);
}

inline void Painter::drawLine(const PointF& p1, const PointF& p2)
{
    drawLine(LineF(p1, p2));
}

inline void Painter::drawLine(double x1, double y1, double x2, double y2)
{
    LineF l(PointF(x1, y1), PointF(x2, y2));
    drawLines(&l, 1);
}

inline void Painter::drawLines(const std::vector<LineF>& lines)
{
    drawLines(lines.data(), lines.size());
}

inline void Painter::drawRect(const RectF& rect)
{
    drawRects(&rect, 1);
}

inline void Painter::drawRect(double x, double y, double w, double h)
{
    RectF r(x, y, w, h);
    drawRects(&r, 1);
}

inline void Painter::drawEllipse(const PointF& center, double rx, double ry)
{
    drawEllipse(RectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void Painter::drawPolyline(const PolygonF& polyline)
{
    drawPolyline(polyline.data(), polyline.size());
}

inline void Painter::drawPolygon(const PolygonF& polygon, FillRule fillRule)
{
    drawPolygon(polygon.data(), polygon.size(), fillRule);
}

inline void Painter::drawConvexPolygon(const PolygonF& poly)
{
    drawConvexPolygon(poly.data(), poly.size());
}

inline void Painter::drawText(double x, double y, const String& text)
{
    drawText(PointF(x, y), text);
}

class PainterItemMarker
{
public:
    PainterItemMarker(Painter* p, const std::string& name)
        : m_painter(p)
    {
        if (!enabled) {
            return;
        }
        p->beginObject(name);
    }

    ~PainterItemMarker()
    {
        if (!enabled) {
            return;
        }
        m_painter->endObject();
    }

    static bool enabled;

private:
    Painter* m_painter = nullptr;
};
}

#ifdef MUSE_MODULE_DRAW_TRACE
    #define TRACE_ITEM_DRAW \
    muse::draw::PainterItemMarker __drawItemMarker(painter, typeName())

    #define TRACE_DRAW_ITEM \
    muse::draw::PainterItemMarker __drawItemMarker(painter, item->typeName())

    #define TRACE_ITEM_DRAW_C(painter, itemName) \
    muse::draw::PainterItemMarker __drawItemMarker(painter, itemName)
#else
    #define TRACE_ITEM_DRAW
    #define TRACE_DRAW_ITEM
    #define TRACE_ITEM_DRAW_C(painter, objName)
#endif
