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
#include "painter.h"

#include "log.h"

#ifndef NO_QT_SUPPORT
#include "qpainterprovider.h"
#endif

using namespace mu::draw;

IPaintProviderPtr Painter::extended;

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

Painter::Painter(QPainter* qp, const std::string& name, bool overship)
    : m_name(name)
{
    m_provider = QPainterProvider::make(qp, overship);
    init();
}

#endif

Painter::~Painter()
{
    endTarget(false);
}

void Painter::init()
{
    State st;
    st.worldTransform = m_provider->transform();
    st.isWxF = true;
    m_states.push(std::move(st));

    m_provider->beginTarget(m_name);
    if (extended) {
        extended->beginTarget(m_name);
    }
}

QPaintDevice* Painter::device() const
{
    return m_provider->device();
}

QPainter* Painter::qpainter() const
{
    return m_provider->qpainter();
}

IPaintProviderPtr Painter::provider() const
{
    return m_provider;
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

void Painter::beginObject(const std::string& name, const QPointF& pagePos)
{
    m_provider->beginObject(name, pagePos);
    if (extended) {
        extended->beginObject(name, pagePos);
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

void Painter::setFont(const QFont& font)
{
    m_provider->setFont(font);
    if (extended) {
        extended->setFont(font);
    }
}

const QFont& Painter::font() const
{
    return m_provider->font();
}

void Painter::setPen(const QPen& pen)
{
    m_provider->setPen(pen);
    if (extended) {
        extended->setPen(pen);
    }
}

void Painter::setNoPen()
{
    setPen(QPen(Qt::NoPen));
}

const QPen& Painter::pen() const
{
    return m_provider->pen();
}

void Painter::setBrush(const QBrush& brush)
{
    m_provider->setBrush(brush);
    if (extended) {
        extended->setBrush(brush);
    }
}

const QBrush& Painter::brush() const
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

void Painter::setWorldTransform(const QTransform& matrix, bool combine)
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

const QTransform& Painter::worldTransform() const
{
    return state().worldTransform;
}

void Painter::scale(qreal sx, qreal sy)
{
    State& st = editableState();
    st.worldTransform.scale(sx,sy);
    st.isWxF = true;
    updateMatrix();
}

void Painter::rotate(qreal angle)
{
    State& st = editableState();
    st.worldTransform.rotate(angle);
    st.isWxF = true;
    updateMatrix();
}

void Painter::translate(qreal dx, qreal dy)
{
    State& st = editableState();
    st.worldTransform.translate(dx, dy);
    st.isWxF = true;
    updateMatrix();
}

QRect Painter::window() const
{
    return state().window;
}

void Painter::setWindow(const QRect& window)
{
    State& st = editableState();
    st.window = window;
    st.isVxF = true;
    st.viewTransform = makeViewTransform();
}

QRect Painter::viewport() const
{
    return state().viewport;
}

void Painter::setViewport(const QRect& viewport)
{
    State& st = editableState();
    st.viewport = viewport;
    st.isVxF = true;
    st.viewTransform = makeViewTransform();
}

// drawing functions

void Painter::fillPath(const QPainterPath& path, const QBrush& brush)
{
    QPen oldPen = this->pen();
    QBrush oldBrush = this->brush();
    setPen(QPen(Qt::NoPen));
    setBrush(brush);

    drawPath(path);

    setPen(oldPen);
    setBrush(oldBrush);
}

void Painter::strokePath(const QPainterPath& path, const QPen& pen)
{
    QPen oldPen = this->pen();
    QBrush oldBrush = this->brush();
    setPen(pen);
    setBrush(Qt::NoBrush);

    drawPath(path);

    setPen(oldPen);
    setBrush(oldBrush);
}

void Painter::drawPath(const QPainterPath& path)
{
    m_provider->drawPath(path);
    if (extended) {
        extended->drawPath(path);
    }
}

void Painter::drawLines(const QLineF* lines, int lineCount)
{
    for (int i = 0; i < lineCount; ++i) {
        QPointF pts[2] = { lines[i].p1(), lines[i].p2() };
        IF_ASSERT_FAILED(pts[0] != pts[1]) {
            LOGE() << "draw point not implemented";
        }
        drawPolyline(pts, 2);
    }
}

void Painter::drawLines(const QPointF* pointPairs, int lineCount)
{
    Q_ASSERT(sizeof(QLineF) == 2 * sizeof(QPointF));

    drawLines((const QLineF*)pointPairs, lineCount);
}

void Painter::drawRects(const QRectF* rects, int rectCount)
{
    for (int i = 0; i < rectCount; ++i) {
        QPainterPath path;
        path.addRect(rects[i]);
        if (path.isEmpty()) {
            continue;
        }
        drawPath(path);
    }
}

void Painter::drawEllipse(const QRectF& rect)
{
    QPainterPath path;
    path.addEllipse(rect);
    m_provider->drawPath(path);
    if (extended) {
        extended->drawPath(path);
    }
}

void Painter::drawPolyline(const QPointF* points, int pointCount)
{
    m_provider->drawPolygon(points, pointCount, PolygonMode::Polyline);
    if (extended) {
        extended->drawPolygon(points, pointCount, PolygonMode::Polyline);
    }
}

void Painter::drawPolygon(const QPointF* points, int pointCount, Qt::FillRule fillRule)
{
    PolygonMode mode = (fillRule == Qt::OddEvenFill) ? PolygonMode::OddEven : PolygonMode::Winding;
    m_provider->drawPolygon(points, pointCount, mode);
    if (extended) {
        extended->drawPolygon(points, pointCount, mode);
    }
}

void Painter::drawConvexPolygon(const QPointF* points, int pointCount)
{
    m_provider->drawPolygon(points, pointCount, PolygonMode::Convex);
    if (extended) {
        extended->drawPolygon(points, pointCount, PolygonMode::Convex);
    }
}

void Painter::drawArc(const QRectF& r, int a, int alen)
{
    //! NOTE Copied from QPainter source code

    QRectF rect = r.normalized();

    QPainterPath path;
    path.arcMoveTo(rect, a / 16.0);
    path.arcTo(rect, a / 16.0, alen / 16.0);
    strokePath(path, pen());
}

void Painter::drawRoundedRect(const QRectF& rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
    if (xRadius <= 0 || yRadius <= 0) {             // draw normal rectangle
        drawRect(rect);
        return;
    }

    QPainterPath path;
    path.addRoundedRect(rect, xRadius, yRadius, mode);
    drawPath(path);
}

void Painter::drawText(const QPointF& point, const QString& text)
{
    m_provider->drawText(point, text);
    if (extended) {
        extended->drawText(point, text);
    }
}

void Painter::drawText(const QRectF& rect, int flags, const QString& text)
{
    m_provider->drawText(rect, flags, text);
    if (extended) {
        extended->drawText(rect, flags, text);
    }
}

void Painter::drawGlyphRun(const QPointF& position, const QGlyphRun& glyphRun)
{
    m_provider->drawGlyphRun(position, glyphRun);
    if (extended) {
        extended->drawGlyphRun(position, glyphRun);
    }
}

void Painter::fillRect(const QRectF& rect, const QBrush& brush)
{
    QPen oldPen = this->pen();
    QBrush oldBrush = this->brush();
    setPen(QPen(Qt::NoPen));
    setBrush(brush);

    drawRect(rect);

    setBrush(oldBrush);
    setPen(oldPen);
}

void Painter::drawPixmap(const QPointF& point, const QPixmap& pm)
{
    m_provider->drawPixmap(point, pm);
    if (extended) {
        extended->drawPixmap(point, pm);
    }
}

void Painter::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    m_provider->drawTiledPixmap(rect, pm, offset);
    if (extended) {
        extended->drawTiledPixmap(rect, pm, offset);
    }
}

Painter::State& Painter::editableState()
{
    return m_states.top();
}

const Painter::State& Painter::state() const
{
    return m_states.top();
}

QTransform Painter::makeViewTransform() const
{
    const State& st = state();
    qreal scaleW = qreal(st.viewport.width()) / qreal(st.window.width());
    qreal scaleH = qreal(st.viewport.height()) / qreal(st.window.height());
    return QTransform(scaleW, 0, 0, scaleH, st.viewport.x() - st.window.x() * scaleW, st.viewport.y() - st.window.y() * scaleH);
}

void Painter::updateMatrix()
{
    Painter::State& st = editableState();
    st.transform = st.isWxF ? st.worldTransform : QTransform();
    if (st.isVxF) {
        st.transform *= st.viewTransform;
    }

    m_provider->setTransform(st.transform);
    if (extended) {
        extended->setTransform(st.transform);
    }
}
