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
#include "qpainterprovider.h"

#include <QPainter>
#include <QRawFont>
#include <QTextLayout>
#include <QTextLine>
#include <QGlyphRun>
#include <QPixmapCache>
#include <QStaticText>
#include <QPainterPath>

#include "draw/utils/drawlogger.h"
#include "types/transform.h"
#include "types/painterpath.h"

#include "log.h"

using namespace muse::draw;

QPainterProvider::QPainterProvider(QPainter* painter, bool ownsPainter)
    : m_painter(painter), m_ownsPainter(ownsPainter), m_drawObjectsLogger(new DrawObjectsLogger())
{
    if (painter->isActive()) {
        m_font = Font::fromQFont(m_painter->font(), Font::Type::Undefined);
        m_pen = Pen::fromQPen(m_painter->pen());
        m_brush = Brush::fromQBrush(m_painter->brush());
        m_transform = Transform::fromQTransform(m_painter->transform());
    }
}

QPainterProvider::~QPainterProvider()
{
    if (m_ownsPainter) {
        delete m_painter;
    }

    delete m_drawObjectsLogger;
}

IPaintProviderPtr QPainterProvider::make(QPaintDevice* dp)
{
    return std::make_shared<QPainterProvider>(new QPainter(dp), true);
}

IPaintProviderPtr QPainterProvider::make(QPainter* qp, bool ownsPainter)
{
    return std::make_shared<QPainterProvider>(qp, ownsPainter);
}

QPainter* QPainterProvider::qpainter() const
{
    return m_painter;
}

void QPainterProvider::beginTarget(const std::string&)
{
}

void QPainterProvider::beforeEndTargetHook(Painter*)
{
}

bool QPainterProvider::endTarget(bool endDraw)
{
    if (endDraw) {
        return m_painter->end();
    }
    return true;
}

bool QPainterProvider::isActive() const
{
    return m_painter->isActive();
}

void QPainterProvider::beginObject(const std::string& name)
{
    UNUSED(name)
    //m_drawObjectsLogger->beginObject(name);
}

void QPainterProvider::endObject()
{
    //m_drawObjectsLogger->endObject();
}

void QPainterProvider::setAntialiasing(bool arg)
{
    m_painter->setRenderHint(QPainter::Antialiasing, arg);
    m_painter->setRenderHint(QPainter::TextAntialiasing, arg);
}

void QPainterProvider::setCompositionMode(CompositionMode mode)
{
    auto toQPainter = [](CompositionMode mode) {
        switch (mode) {
        case CompositionMode::SourceOver: return QPainter::CompositionMode_SourceOver;
        case CompositionMode::HardLight: return QPainter::CompositionMode_HardLight;
        }
        return QPainter::CompositionMode_SourceOver;
    };
    m_painter->setCompositionMode(toQPainter(mode));
}

void QPainterProvider::setWindow(const RectF& window)
{
    // no need set
    UNUSED(window);
}

void QPainterProvider::setViewport(const RectF& viewport)
{
    // no need set
    UNUSED(viewport);
}

void QPainterProvider::setFont(const Font& font)
{
    if (m_font != font) {
        m_painter->setFont(font.toQFont());
        m_font = font;
    }
}

const Font& QPainterProvider::font() const
{
    return m_font;
}

void QPainterProvider::setPen(const Pen& pen)
{
    m_pen = pen;
    m_painter->setPen(Pen::toQPen(m_pen));
}

void QPainterProvider::setNoPen()
{
    m_pen = Pen(PenStyle::NoPen);
    m_painter->setPen(Pen::toQPen(m_pen));
}

const Pen& QPainterProvider::pen() const
{
    return m_pen;
}

void QPainterProvider::setBrush(const Brush& brush)
{
    m_brush = brush;
    m_painter->setBrush(Brush::toQBrush(m_brush));
}

const Brush& QPainterProvider::brush() const
{
    return m_brush;
}

void QPainterProvider::save()
{
    m_painter->save();
}

void QPainterProvider::restore()
{
    m_painter->restore();
    m_font = Font::fromQFont(m_painter->font(), Font::Type::Undefined);
    m_pen = Pen::fromQPen(m_painter->pen());
    m_brush = Brush::fromQBrush(m_painter->brush());
    m_transform = Transform::fromQTransform(m_painter->transform());
}

void QPainterProvider::setTransform(const Transform& transform)
{
    m_transform = transform;
    m_painter->setTransform(Transform::toQTransform(m_transform));
}

const Transform& QPainterProvider::transform() const
{
    return m_transform;
}

// drawing functions

void QPainterProvider::drawPath(const PainterPath& path)
{
    m_painter->drawPath(PainterPath::toQPainterPath(path));
}

void QPainterProvider::drawPolygon(const PointF* points, size_t pointCount, PolygonMode mode)
{
    static_assert(sizeof(QPointF) == sizeof(PointF), "sizeof(QPointF) and sizeof(PointF) must be equal");

    const QPointF* qpoints = reinterpret_cast<const QPointF*>(points);

    switch (mode) {
    case PolygonMode::OddEven: {
        m_painter->drawPolygon(qpoints, int(pointCount), Qt::OddEvenFill);
    } break;
    case PolygonMode::Winding: {
        m_painter->drawPolygon(qpoints, int(pointCount), Qt::WindingFill);
    } break;
    case PolygonMode::Convex: {
        m_painter->drawConvexPolygon(qpoints, int(pointCount));
    } break;
    case PolygonMode::Polyline: {
        m_painter->drawPolyline(qpoints, int(pointCount));
    } break;
    }
}

void QPainterProvider::drawText(const PointF& point, const String& text)
{
    QPointF p = point.toQPointF();
    QString t = text.toQString();
    m_painter->drawText(p, t);
}

void QPainterProvider::drawText(const RectF& rect, int flags, const String& text)
{
    m_painter->drawText(rect.toQRectF(), flags, text);
}

void QPainterProvider::drawTextWorkaround(const Font& f, const PointF& pos, const String& text)
{
    m_painter->save();
    double mm = m_painter->worldTransform().m11();
    double dx = m_painter->worldTransform().dx();
    double dy = m_painter->worldTransform().dy();
    // diagonal elements will now be changed to 1.0
    m_painter->setWorldTransform(QTransform(1.0, 0.0, 0.0, 1.0, dx, dy));

    // correction factor for bold text drawing, due to the change of the diagonal elements
    double factor = 1.0 / mm;
    QFont fnew(f.toQFont(), m_painter->device());
    fnew.setPointSizeF(f.pointSizeF() / factor);
    QRawFont fRaw = QRawFont::fromFont(fnew);
    QTextLayout textLayout(text, f.toQFont(), m_painter->device());
    textLayout.beginLayout();
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }
    }
    textLayout.endLayout();
    // glyphruns with correct positions, but potentially wrong glyphs
    // (see bug https://musescore.org/en/node/117191 regarding positions and DPI)
    QList<QGlyphRun> glyphruns = textLayout.glyphRuns();
    double offset = 0;
    // glyphrun drawing has an offset equal to the max ascent of the text fragment
    for (int i = 0; i < glyphruns.length(); i++) {
        double value = glyphruns.at(i).rawFont().ascent() / factor;
        if (value > offset) {
            offset = value;
        }
    }
    for (int i = 0; i < glyphruns.length(); i++) {
        QVector<QPointF> positions1 = glyphruns.at(i).positions();
        QVector<QPointF> positions2;
        // calculate the new positions for the scaled geometry
        for (int j = 0; j < positions1.length(); j++) {
            QPointF newPoint = positions1.at(j) / factor;
            positions2.append(newPoint);
        }
        QGlyphRun glyphrun2 = glyphruns.at(i);
        glyphrun2.setPositions(positions2);
        // change the glyphs with the correct glyphs
        // and account for glyph substitution
        if (glyphrun2.rawFont().familyName() != fnew.family()) {
            QFont f2(fnew);
            f2.setFamily(glyphrun2.rawFont().familyName());
            glyphrun2.setRawFont(QRawFont::fromFont(f2));
        } else {
            glyphrun2.setRawFont(fRaw);
        }
        m_painter->drawGlyphRun(QPointF(pos.x() / factor, pos.y() / factor - offset), glyphrun2);
        positions2.clear();
    }
    // Restore the QPainter to its former state
    m_painter->setWorldTransform(QTransform(mm, 0.0, 0.0, mm, dx, dy));
    m_painter->restore();
}

void QPainterProvider::drawSymbol(const PointF& point, char32_t ucs4Code)
{
    static QHash<char32_t, QString> cache;
    if (!cache.contains(ucs4Code)) {
        cache[ucs4Code] = QString::fromUcs4(&ucs4Code, 1);
    }

    drawText(point, cache.value(ucs4Code));
}

void QPainterProvider::drawPixmap(const PointF& point, const Pixmap& pm)
{
    QString key = QString::number(pm.key());
    QPixmap pixmap;
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap.loadFromData(pm.data().toQByteArrayNoCopy());
        QPixmapCache::insert(key, pixmap);
    }

    m_painter->drawPixmap(QPointF(point.x(), point.y()), pixmap);
}

void QPainterProvider::drawTiledPixmap(const RectF& rect, const Pixmap& pm, const PointF& offset)
{
    QString key = QString::number(pm.key());
    QPixmap pixmap;
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap.loadFromData(pm.data().toQByteArrayNoCopy());
        QPixmapCache::insert(key, pixmap);
    }

    m_painter->drawTiledPixmap(rect.toQRectF(), pixmap, QPointF(offset.x(), offset.y()));
}

void QPainterProvider::drawPixmap(const PointF& point, const QPixmap& pm)
{
    m_painter->drawPixmap(QPointF(point.x(), point.y()), pm);
}

void QPainterProvider::drawTiledPixmap(const RectF& rect, const QPixmap& pm, const PointF& offset)
{
    m_painter->drawTiledPixmap(rect.toQRectF(), pm, QPointF(offset.x(), offset.y()));
}

bool QPainterProvider::hasClipping() const
{
    return m_painter->hasClipping();
}

void QPainterProvider::setClipRect(const RectF& rect)
{
    m_painter->setClipRect(rect.toQRectF());
}

void QPainterProvider::setMask(const RectF& background, const std::vector<RectF>& maskRects)
{
    if (maskRects.empty()) {
        m_painter->setClipPath(QPainterPath(), Qt::NoClip);
        return;
    }

    QPainterPath backgroundPath;
    backgroundPath.addRect(background.toQRectF());

    QPainterPath exclusionRegion;
    exclusionRegion.setFillRule(Qt::WindingFill);
    for (const RectF& rect : maskRects) {
        exclusionRegion.addRect(rect.toQRectF());
    }

    QPainterPath mask = backgroundPath.subtracted(exclusionRegion);

    m_painter->setClipPath(mask);
}

void QPainterProvider::setClipping(bool enable)
{
    m_painter->setClipping(enable);
}
