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
#ifndef MU_DRAW_PAINTER_H
#define MU_DRAW_PAINTER_H

#include <list>
#include <stack>

#include <QColor>
#include <QPainter>

#include "config.h"
#include "ipaintprovider.h"

#include "geometry.h"
#include "drawtypes.h"
#include "font.h"
#include "pen.h"
#include "pixmap.h"

class QPaintDevice;

#ifndef NO_QT_SUPPORT
class QPainter;
#endif

namespace mu::draw {
class Painter
{
public:
    Painter(IPaintProviderPtr provider, const std::string& name);

#ifndef NO_QT_SUPPORT
    Painter(QPaintDevice* dp, const std::string& name);
    Painter(QPainter* qp, const std::string& name, bool overship = false);
#endif

    ~Painter();

    QPaintDevice* device() const;
    QPainter* qpainter() const;
    IPaintProviderPtr provider() const;

    bool isActive() const;
    bool endDraw();

    //! NOTE These are methods for debugging and automated testing.
    void beginObject(const std::string& name, const PointF& pagePos);
    void endObject();

    // state
    void setAntialiasing(bool arg);
    void setCompositionMode(CompositionMode mode);

    void setFont(const Font& font);
    const Font& font() const;

    void setPen(const Pen& pen);
    inline void setPen(const QColor& color);
    void setNoPen();
    const Pen& pen() const;

    void setBrush(const Brush& brush);
    const Brush& brush() const;

    void setWorldTransform(const QTransform& matrix, bool combine = false);
    const QTransform& worldTransform() const;
    void scale(qreal sx, qreal sy);
    void rotate(qreal angle);
    void translate(qreal dx, qreal dy);
    inline void translate(const PointF& offset);

    RectF window() const;
    void setWindow(const RectF& window);
    RectF viewport() const;
    void setViewport(const RectF& viewport);

    void save();
    void restore();

    // drawing
    void fillPath(const QPainterPath& path, const Brush& brush);
    void drawPath(const QPainterPath& path);
    void strokePath(const QPainterPath& path, const Pen& pen);

    void drawLines(const LineF* lines, size_t lineCount);
    void drawLines(const PointF* pointPairs, size_t lineCount);
    inline void drawLine(const LineF& line);
    inline void drawLine(const PointF& p1, const PointF& p2);
    inline void drawLines(const std::vector<LineF>& lines);

    //! NOTE Potentially dangerous method.
    //! Most of them are cut with fractional values.
    //! Fractions are also passed to this method, and, accordingly, the fractional part is discarded.
    inline void drawLine(int x1, int y1, int x2, int y2);

    inline void drawRect(const RectF& rect);

    //! NOTE Potentially dangerous method.
    //! Most of them are cut with fractional values.
    //! Fractions are also passed to this method, and, accordingly, the fractional part is discarded.
    inline void drawRect(int x1, int y1, int w, int h);

    void drawRects(const RectF* rects, size_t rectCount);

    void drawRoundedRect(const RectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);

    void drawEllipse(const RectF& rect);
    inline void drawEllipse(const PointF& center, qreal rx, qreal ry);

    void drawPolyline(const PointF* points, size_t pointCount);
    inline void drawPolyline(const PolygonF& polyline);

    void drawPolygon(const PointF* points, size_t pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
    inline void drawPolygon(const PolygonF& polygon, Qt::FillRule fillRule = Qt::OddEvenFill);

    void drawConvexPolygon(const PointF* points, size_t pointCount);
    inline void drawConvexPolygon(const PolygonF& polygon);

    void drawArc(const RectF& rect, int a, int alen);

    void drawText(const PointF& point, const QString& text);
    void drawText(const RectF& rect, int flags, const QString& text);

    //! NOTE Potentially dangerous method.
    //! Most of them are cut with fractional values.
    //! Fractions are also passed to this method, and, accordingly, the fractional part is discarded.
    inline void drawText(int x, int y, const QString& text);

    //! NOTE workaround for https://musescore.org/en/node/284218
    //! and https://musescore.org/en/node/281601
    //! only needed for certain artificially emboldened fonts
    //! see https://musescore.org/en/node/281601#comment-900261
    //! in Qt 5.12.x this workaround should be no more necessary if
    //! env variable QT_MAX_CACHED_GLYPH_SIZE is set to 1.
    //! The workaround works badly if the text is at the same time
    //! bold and underlined.
    //! (moved from TextBase::drawTextWorkaround)
    void drawTextWorkaround(Font& f, const PointF pos, const QString text);

    void drawSymbol(const PointF& point, uint ucs4Code);

    void fillRect(const RectF& rect, const Brush& brush);

    void drawPixmap(const PointF& point, const Pixmap& pm);
    void drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset = PointF());

#ifndef NO_QT_SUPPORT
    void drawPixmap(const PointF& point, const QPixmap& pm);
    void drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset = PointF());
#endif

    void setClipRect(const RectF& rect);
    void setClipping(bool enable);

    //! NOTE Provider for tests.
    //! We're not ready to use DI (ModuleIoC) here yet
    static IPaintProviderPtr extended;

private:

    struct State {
        RectF window;
        RectF viewport;
        bool isVxF = false;
        QTransform viewTransform;
        bool isWxF = false;
        QTransform worldTransform;       // World transformation matrix, not window and viewport
        QTransform transform;            // Complete transformation matrix
    };

    void init();
    State& editableState();
    const State& state() const;
    QTransform makeViewTransform() const;
    void updateMatrix();

    bool endTarget(bool endDraw);

    IPaintProviderPtr m_provider;
    std::string m_name;
    std::stack<State> m_states;
};

inline void Painter::setPen(const QColor& color)
{
    setPen(Pen(color.isValid() ? color : QColor(Qt::black)));
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

inline void Painter::drawLine(int x1, int y1, int x2, int y2)
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

inline void Painter::drawRect(int x, int y, int w, int h)
{
    RectF r(x, y, w, h);
    drawRects(&r, 1);
}

inline void Painter::drawEllipse(const PointF& center, qreal rx, qreal ry)
{
    drawEllipse(RectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void Painter::drawPolyline(const PolygonF& polyline)
{
    drawPolyline(polyline.data(), polyline.size());
}

inline void Painter::drawPolygon(const PolygonF& polygon, Qt::FillRule fillRule)
{
    drawPolygon(polygon.data(), polygon.size(), fillRule);
}

inline void Painter::drawConvexPolygon(const PolygonF& poly)
{
    drawConvexPolygon(poly.data(), poly.size());
}

inline void Painter::drawText(int x, int y, const QString& text)
{
    drawText(PointF(x, y), text);
}

class PainterObjMarker
{
public:
    PainterObjMarker(Painter* p, const std::string& name, const PointF& objPagePos)
        : m_painter(p)
    {
        p->beginObject(name, objPagePos);
    }

    ~PainterObjMarker()
    {
        m_painter->endObject();
    }

private:
    Painter* m_painter = nullptr;
};

#ifdef TRACE_DRAW_OBJ_ENABLED
    #define TRACE_OBJ_DRAW \
    mu::draw::PainterObjMarker __drawObjMarker(painter, name(), pagePos())

    #define TRACE_OBJ_DRAW_C(painter, objName, objPagePos) \
    mu::draw::PainterObjMarker __drawObjMarker(painter, objName, objPagePos)
#else
    #define TRACE_OBJ_DRAW
    #define TRACE_OBJ_DRAW_C(painter, objName, objPagePos)
#endif
}

#endif // MU_DRAW_PAINTER_H
