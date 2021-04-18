//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_DRAW_PAINTER_H
#define MU_DRAW_PAINTER_H

#include <list>
#include <stack>

#include <QPoint>
#include <QPointF>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QPainter>

#include "config.h"
#include "drawtypes.h"
#include "ipaintprovider.h"

class QPaintDevice;
class QImage;

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
    void beginObject(const std::string& name, const QPointF& pagePos);
    void endObject();

    // state
    void setAntialiasing(bool arg);
    void setCompositionMode(CompositionMode mode);

    void setFont(const QFont& font);
    const QFont& font() const;

    void setPen(const QPen& pen);
    inline void setPen(const QColor& color);
    void setNoPen();
    const QPen& pen() const;

    void setBrush(const QBrush& brush);
    const QBrush& brush() const;

    void setWorldTransform(const QTransform& matrix, bool combine = false);
    const QTransform& worldTransform() const;
    void scale(qreal sx, qreal sy);
    void rotate(qreal angle);
    void translate(qreal dx, qreal dy);
    inline void translate(const QPointF& offset);

    QRect window() const;
    void setWindow(const QRect& window);
    QRect viewport() const;
    void setViewport(const QRect& viewport);

    void save();
    void restore();

    // drawing
    void fillPath(const QPainterPath& path, const QBrush& brush);
    void drawPath(const QPainterPath& path);
    void strokePath(const QPainterPath& path, const QPen& pen);

    inline void drawLine(const QLineF& line);
    inline void drawLine(const QPointF& p1, const QPointF& p2);

    //! NOTE Potentially dangerous method.
    //! Most of them are cut with fractional values.
    //! Fractions are also passed to this method, and, accordingly, the fractional part is discarded.
    inline void drawLine(int x1, int y1, int x2, int y2);

    void drawLines(const QLineF* lines, int lineCount);
    inline void drawLines(const QVector<QLineF>& lines);
    void drawLines(const QPointF* pointPairs, int lineCount);

    inline void drawRect(const QRectF& rect);

    //! NOTE Potentially dangerous method.
    //! Most of them are cut with fractional values.
    //! Fractions are also passed to this method, and, accordingly, the fractional part is discarded.
    inline void drawRect(int x1, int y1, int w, int h);

    void drawRects(const QRectF* rects, int rectCount);

    void drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);

    void drawEllipse(const QRectF& rect);
    inline void drawEllipse(const QPointF& center, qreal rx, qreal ry);

    void drawPolyline(const QPointF* points, int pointCount);
    inline void drawPolyline(const QPolygonF& polyline);

    void drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
    inline void drawPolygon(const QPolygonF& polygon, Qt::FillRule fillRule = Qt::OddEvenFill);

    void drawConvexPolygon(const QPointF* points, int pointCount);
    inline void drawConvexPolygon(const QPolygonF& polygon);

    void drawArc(const QRectF& rect, int a, int alen);

    void drawText(const QPointF& point, const QString& text);
    void drawText(const QRectF& rect, int flags, const QString& text);

    //! NOTE Potentially dangerous method.
    //! Most of them are cut with fractional values.
    //! Fractions are also passed to this method, and, accordingly, the fractional part is discarded.
    inline void drawText(int x, int y, const QString& text);

    void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun);

    void fillRect(const QRectF& rect, const QBrush& brush);

    void drawPixmap(const QPointF& point, const QPixmap& pm);
    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF());

    //! NOTE Provider for tests.
    //! We're not ready to use DI (ModuleIoC) here yet
    static IPaintProviderPtr extended;

private:

    struct State {
        QRect window;
        QRect viewport;
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
    setPen(QPen(color.isValid() ? color : QColor(Qt::black)));
}

inline void Painter::translate(const QPointF& offset)
{
    translate(offset.x(), offset.y());
}

inline void Painter::drawLine(const QLineF& l)
{
    drawLines(&l, 1);
}

inline void Painter::drawLine(const QPointF& p1, const QPointF& p2)
{
    drawLine(QLineF(p1, p2));
}

inline void Painter::drawLine(int x1, int y1, int x2, int y2)
{
    QLineF l(x1, y1, x2, y2);
    drawLines(&l, 1);
}

inline void Painter::drawLines(const QVector<QLineF>& lines)
{
    drawLines(lines.constData(), lines.size());
}

inline void Painter::drawRect(const QRectF& rect)
{
    drawRects(&rect, 1);
}

inline void Painter::drawRect(int x, int y, int w, int h)
{
    QRectF r(x, y, w, h);
    drawRects(&r, 1);
}

inline void Painter::drawEllipse(const QPointF& center, qreal rx, qreal ry)
{
    drawEllipse(QRectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void Painter::drawPolyline(const QPolygonF& polyline)
{
    drawPolyline(polyline.constData(), polyline.size());
}

inline void Painter::drawPolygon(const QPolygonF& polygon, Qt::FillRule fillRule)
{
    drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void Painter::drawConvexPolygon(const QPolygonF& poly)
{
    drawConvexPolygon(poly.constData(), poly.size());
}

inline void Painter::drawText(int x, int y, const QString& text)
{
    drawText(QPointF(x, y), text);
}

class PainterObjMarker
{
public:
    PainterObjMarker(Painter* p, const std::string& name, const QPointF& objPagePos)
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
    std::string m_name;
    QPointF m_objPagePos;
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
