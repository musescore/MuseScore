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

#include "fontcompat.h"
#include "utils/drawlogger.h"
#include "log.h"

using namespace mu::draw;

QPainterProvider::QPainterProvider(QPainter* painter, bool overship)
    : m_painter(painter), m_overship(overship)
{
    m_drawObjectsLogger = new DrawObjectsLogger();
}

QPainterProvider::~QPainterProvider()
{
    if (m_overship) {
        delete m_painter;
    }

    delete m_drawObjectsLogger;
}

IPaintProviderPtr QPainterProvider::make(QPaintDevice* dp)
{
    return std::make_shared<QPainterProvider>(new QPainter(dp), true);
}

IPaintProviderPtr QPainterProvider::make(QPainter* qp, bool overship)
{
    return std::make_shared<QPainterProvider>(qp, overship);
}

QPaintDevice* QPainterProvider::device() const
{
    return m_painter->device();
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

void QPainterProvider::beginObject(const std::string& name, const QPointF& pagePos)
{
    m_drawObjectsLogger->beginObject(name, pagePos);
}

void QPainterProvider::endObject()
{
    m_drawObjectsLogger->endObject();
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

void QPainterProvider::setFont(const QFont& font)
{
    m_painter->setFont(font);
}

const QFont& QPainterProvider::qFont() const
{
    return m_painter->font();
}

void QPainterProvider::setFont(const Font& font)
{
    m_painter->setFont(mu::draw::toQFont(font));
}

Font QPainterProvider::font() const
{
    return mu::draw::fromQFont(m_painter->font());
}

void QPainterProvider::setPen(const QPen& pen)
{
    m_painter->setPen(pen);
}

void QPainterProvider::setNoPen()
{
    m_painter->setPen(QPen(Qt::NoPen));
}

const QPen& QPainterProvider::pen() const
{
    return m_painter->pen();
}

void QPainterProvider::setBrush(const QBrush& brush)
{
    m_painter->setBrush(brush);
}

const QBrush& QPainterProvider::brush() const
{
    return m_painter->brush();
}

void QPainterProvider::save()
{
    m_painter->save();
}

void QPainterProvider::restore()
{
    m_painter->restore();
}

void QPainterProvider::setTransform(const QTransform& transform)
{
    m_transform = transform;
    m_painter->setTransform(transform);
}

const QTransform& QPainterProvider::transform() const
{
    return m_painter->transform();
}

// drawing functions

void QPainterProvider::drawPath(const QPainterPath& path)
{
    m_painter->drawPath(path);
}

void QPainterProvider::drawPolygon(const QPointF* points, int pointCount, PolygonMode mode)
{
    switch (mode) {
    case PolygonMode::OddEven: {
        m_painter->drawPolygon(points, pointCount, Qt::OddEvenFill);
    } break;
    case PolygonMode::Winding: {
        m_painter->drawPolygon(points, pointCount, Qt::WindingFill);
    } break;
    case PolygonMode::Convex: {
        m_painter->drawConvexPolygon(points, pointCount);
    } break;
    case PolygonMode::Polyline: {
        m_painter->drawPolyline(points, pointCount);
    } break;
    }
}

void QPainterProvider::drawText(const QPointF& point, const QString& text)
{
    m_painter->drawText(point, text);
}

void QPainterProvider::drawText(const QRectF& rect, int flags, const QString& text)
{
    m_painter->drawText(rect, flags, text);
}

void QPainterProvider::drawTextWorkaround(mu::draw::Font& f, const QPointF& pos, const QString& text)
{
    m_painter->save();
    qreal mm = m_painter->worldTransform().m11();
    qreal dx = m_painter->worldTransform().dx();
    qreal dy = m_painter->worldTransform().dy();
    // diagonal elements will now be changed to 1.0
    m_painter->setWorldTransform(QTransform(1.0, 0.0, 0.0, 1.0, dx, dy));

    // correction factor for bold text drawing, due to the change of the diagonal elements
    qreal factor = 1.0 / mm;
    QFont fnew(mu::draw::toQFont(f), m_painter->device());
    fnew.setPointSizeF(f.pointSizeF() / factor);
    QRawFont fRaw = QRawFont::fromFont(fnew);
    QTextLayout textLayout(text, mu::draw::toQFont(f), m_painter->device());
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
    qreal offset = 0;
    // glyphrun drawing has an offset equal to the max ascent of the text fragment
    for (int i = 0; i < glyphruns.length(); i++) {
        qreal value = glyphruns.at(i).rawFont().ascent() / factor;
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

void QPainterProvider::drawPixmap(const QPointF& point, const QPixmap& pm)
{
    m_painter->drawPixmap(point, pm);
}

void QPainterProvider::drawTiledPixmap(const QRectF& rect, const QPixmap& pm, const QPointF& offset)
{
    m_painter->drawTiledPixmap(rect, pm, offset);
}
