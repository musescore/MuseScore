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
#ifndef MU_DRAW_IPAINTPROVIDER_H
#define MU_DRAW_IPAINTPROVIDER_H

namespace mu::draw {
class IPaintProvider
{
public:
    virtual ~IPaintProvider() = default;

//    QPaintDevice* device() const;
//    QPainter* qpainter() const;

//    bool begin(QPaintDevice*);
//    bool end();
//    bool isActive() const;

//    void setRenderHint(RenderHint hint, bool on = true);
//    void setCompositionMode(QPainter::CompositionMode mode);

//    void setFont(const QFont& f);
//    const QFont& font() const;

//    void setPen(const QColor& color);
//    void setPen(const QPen& pen);
//    void setPen(Qt::PenStyle style);
//    const QPen& pen() const;

//    void setBrush(const QBrush& brush);
//    void setBrush(Qt::BrushStyle style);
//    const QBrush& brush() const;

//    void save();
//    void restore();

//    void setWorldTransform(const QTransform& matrix, bool combine = false);
//    const QTransform& worldTransform() const;

//    void setTransform(const QTransform& transform, bool combine = false);
//    const QTransform& transform() const;

//    void setMatrix(const QMatrix& matrix, bool combine = false);

//    void scale(qreal sx, qreal sy);
//    void rotate(qreal a);

//    void translate(const QPointF& offset);
//    inline void translate(const QPoint& offset);
//    inline void translate(qreal dx, qreal dy);

//    QRect window() const;
//    void setWindow(const QRect& window);
//    inline void setWindow(int x, int y, int w, int h);

//    QRect viewport() const;
//    void setViewport(const QRect& viewport);
//    inline void setViewport(int x, int y, int w, int h);

//    // drawing functions
//    void strokePath(const QPainterPath& path, const QPen& pen);
//    void fillPath(const QPainterPath& path, const QBrush& brush);
//    void drawPath(const QPainterPath& path);

//    inline void drawLine(const QLineF& line);
//    inline void drawLine(const QPointF& p1, const QPointF& p2);
//    inline void drawLine(int x1, int y1, int x2, int y2);

//    void drawLines(const QLineF* lines, int lineCount);
//    inline void drawLines(const QVector<QLineF>& lines);
//    void drawLines(const QLine* lines, int lineCount);
//    void drawLines(const QPointF* pointPairs, int lineCount);

//    inline void drawRect(const QRectF& rect);
//    inline void drawRect(int x1, int y1, int w, int h);
//    inline void drawRect(const QRect& rect);

//    void drawRects(const QRectF* rects, int rectCount);
//    inline void drawRects(const QVector<QRectF>& rectangles);
//    void drawRects(const QRect* rects, int rectCount);
//    inline void drawRects(const QVector<QRect>& rectangles);

//    void drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);

//    void drawEllipse(const QRectF& r);
//    void drawEllipse(const QRect& r);

//    void drawPolyline(const QPointF* points, int pointCount);
//    void drawPolyline(const QPoint* points, int pointCount);

//    void drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
//    void drawPolygon(const QPoint* points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
//    void drawConvexPolygon(const QPointF* points, int pointCount);

//    void drawArc(const QRectF& rect, int a, int alen);

//    void drawText(const QPointF& p, const QString& s);
//    void drawText(const QRectF& r, int flags, const QString& text, QRectF* br = nullptr);

//    void drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun);

//    void fillRect(const QRectF& r, const QColor& color);

//    void drawPixmap(const QPointF& p, const QPixmap& pm);
//    void drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset = QPointF());
};
}

#endif // MU_DRAW_IPAINTPROVIDER_H
