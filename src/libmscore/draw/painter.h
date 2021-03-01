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
#ifndef MU_DOM_PAINTER_H
#define MU_DOM_PAINTER_H

#include <QPoint>
#include <QPointF>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QPainter>

class QPaintDevice;
class QImage;

namespace mu::draw {
class Painter
{
public:
    Painter(QPainter* painter);

    enum RenderHint {
        Antialiasing = 0x01,
        TextAntialiasing = 0x02,
        SmoothPixmapTransform = 0x04,
        LosslessImageRendering = 0x40,
    };

    QPaintDevice* device() const;
    QPainter* qpainter() const;

    bool begin(QPaintDevice*);
    bool end();
    bool isActive() const;

    void setRenderHint(RenderHint hint, bool on = true);
    void setCompositionMode(QPainter::CompositionMode mode);

    void setFont(const QFont& f);
    const QFont& font() const;

    void setPen(const QColor& color);
    void setPen(const QPen& pen);
    void setPen(Qt::PenStyle style);
    const QPen& pen() const;

    void setBrush(const QBrush& brush);
    void setBrush(Qt::BrushStyle style);
    const QBrush& brush() const;

    void save();
    void restore();

    void setWorldTransform(const QTransform& matrix, bool combine = false);
    const QTransform& worldTransform() const;

    void setTransform(const QTransform& transform, bool combine = false);
    const QTransform& transform() const;

    void setMatrix(const QMatrix& matrix, bool combine = false);

    void scale(qreal sx, qreal sy);
    void rotate(qreal a);

    void translate(const QPointF& offset);
    inline void translate(const QPoint& offset);
    inline void translate(qreal dx, qreal dy);

    QRect window() const;
    void setWindow(const QRect& window);
    inline void setWindow(int x, int y, int w, int h);

    QRect viewport() const;
    void setViewport(const QRect& viewport);
    inline void setViewport(int x, int y, int w, int h);

    // drawing functions
    void strokePath(const QPainterPath& path, const QPen& pen);
    void fillPath(const QPainterPath& path, const QBrush& brush);
    void drawPath(const QPainterPath& path);

    inline void drawLine(const QLineF& line);
    inline void drawLine(const QPointF& p1, const QPointF& p2);
    inline void drawLine(int x1, int y1, int x2, int y2);

    void drawLines(const QLineF* lines, int lineCount);
    inline void drawLines(const QVector<QLineF>& lines);
    void drawLines(const QLine* lines, int lineCount);
    void drawLines(const QPointF* pointPairs, int lineCount);

    inline void drawRect(const QRectF& rect);
    inline void drawRect(int x1, int y1, int w, int h);
    inline void drawRect(const QRect& rect);

    void drawRects(const QRectF* rects, int rectCount);
    inline void drawRects(const QVector<QRectF>& rectangles);
    void drawRects(const QRect* rects, int rectCount);
    inline void drawRects(const QVector<QRect>& rectangles);

    void drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);
    inline void drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);
    inline void drawRoundedRect(const QRect& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);

    void drawEllipse(const QRectF& r);
    void drawEllipse(const QRect& r);
    inline void drawEllipse(int x, int y, int w, int h);
    inline void drawEllipse(const QPointF& center, qreal rx, qreal ry);
    inline void drawEllipse(const QPoint& center, int rx, int ry);

    void drawPolyline(const QPointF* points, int pointCount);
    inline void drawPolyline(const QPolygonF& polyline);
    void drawPolyline(const QPoint* points, int pointCount);
    inline void drawPolyline(const QPolygon& polygon);

    void drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
    inline void drawPolygon(const QPolygonF& polygon, Qt::FillRule fillRule = Qt::OddEvenFill);
    void drawPolygon(const QPoint* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
    inline void drawPolygon(const QPolygon& polygon, Qt::FillRule fillRule = Qt::OddEvenFill);

    void drawConvexPolygon(const QPointF* points, int pointCount);
    inline void drawConvexPolygon(const QPolygonF& polygon);

    void drawArc(const QRectF& rect, int a, int alen);
    inline void drawArc(const QRect&, int a, int alen);
    inline void drawArc(int x, int y, int w, int h, int a, int alen);

    void drawText(const QPointF& p, const QString& s);
    void drawText(const QRectF& r, int flags, const QString& text, QRectF* br = nullptr);
    inline void drawText(const QPoint& p, const QString& s);
    inline void drawText(int x, int y, const QString& s);

    void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun);

    void fillRect(const QRectF& r, const QColor& color);

    void drawPixmap(const QPointF& p, const QPixmap& pm);
    inline void drawPixmap(int x, int y, const QPixmap& pm);

    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF());

private:
    QPainter* m_painter = nullptr;
};

inline void Painter::translate(qreal dx, qreal dy)
{
    translate(QPointF(dx, dy));
}

inline void Painter::translate(const QPoint& offset)
{
    translate(offset.x(), offset.y());
}

inline void Painter::setViewport(int x, int y, int w, int h)
{
    setViewport(QRect(x, y, w, h));
}

inline void Painter::setWindow(int x, int y, int w, int h)
{
    setWindow(QRect(x, y, w, h));
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
    QLine l(x1, y1, x2, y2);
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
    QRect r(x, y, w, h);
    drawRects(&r, 1);
}

inline void Painter::drawRect(const QRect& r)
{
    drawRects(&r, 1);
}

inline void Painter::drawRects(const QVector<QRectF>& rects)
{
    drawRects(rects.constData(), rects.size());
}

inline void Painter::drawRects(const QVector<QRect>& rects)
{
    drawRects(rects.constData(), rects.size());
}

inline void Painter::drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
    drawRoundedRect(QRectF(x, y, w, h), xRadius, yRadius, mode);
}

inline void Painter::drawRoundedRect(const QRect& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
    drawRoundedRect(QRectF(rect), xRadius, yRadius, mode);
}

inline void Painter::drawEllipse(int x, int y, int w, int h)
{
    drawEllipse(QRect(x, y, w, h));
}

inline void Painter::drawEllipse(const QPointF& center, qreal rx, qreal ry)
{
    drawEllipse(QRectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void Painter::drawEllipse(const QPoint& center, int rx, int ry)
{
    drawEllipse(QRect(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void Painter::drawPolyline(const QPolygonF& polyline)
{
    drawPolyline(polyline.constData(), polyline.size());
}

inline void Painter::drawPolyline(const QPolygon& polyline)
{
    drawPolyline(polyline.constData(), polyline.size());
}

inline void Painter::drawPolygon(const QPolygonF& polygon, Qt::FillRule fillRule)
{
    drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void Painter::drawPolygon(const QPolygon& polygon, Qt::FillRule fillRule)
{
    drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void Painter::drawConvexPolygon(const QPolygonF& poly)
{
    drawConvexPolygon(poly.constData(), poly.size());
}

inline void Painter::drawArc(const QRect& r, int a, int alen)
{
    drawArc(QRectF(r), a, alen);
}

inline void Painter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    drawArc(QRectF(x, y, w, h), a, alen);
}

inline void Painter::drawText(const QPoint& p, const QString& s)
{
    drawText(QPointF(p), s);
}

inline void Painter::drawText(int x, int y, const QString& s)
{
    drawText(QPointF(x, y), s);
}

inline void Painter::drawPixmap(int x, int y, const QPixmap& pm)
{
    drawPixmap(QPointF(x, y), pm);
}
}

#endif // MU_DOM_PAINTER_H
