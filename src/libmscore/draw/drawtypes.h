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
#ifndef MU_DRAW_DRAWTYPES_H
#define MU_DRAW_DRAWTYPES_H

#include <QPainterPath>
#include <QBrush>
#include <QPen>
#include <QGlyphRun>

namespace mu::draw {
enum class CompositionMode {
    SourceOver,
    HardLight
};

enum class DrawMode {
    Stroke = 0,
    Fill,
    StrokeAndFill
};

enum class PolygonMode {
    OddEven,
    Winding,
    Convex,
    Polyline
};

struct Scale {
    double x = 0.0;
    double y = 0.0;
};

struct DrawPath {
    QPainterPath path;
    QPen pen;
    QBrush brush;
    DrawMode mode = DrawMode::StrokeAndFill;
};

struct DrawRect {
    QRectF rect;
    QPen pen;
    QBrush brush;
    DrawMode mode = DrawMode::StrokeAndFill;
};

struct DrawPolygon {
    QPolygonF polygon;
    PolygonMode mode = PolygonMode::OddEven;
};

struct DrawText {
    QPointF point;
    QString text;
};

struct DrawRectText {
    QRectF rect;
    int flags = 0;
    QString text;
};

struct DrawGlyphRun {
    QPointF pos;
    QGlyphRun glyphRun;
};

struct DrawPixmap {
    QPointF point;
    QPixmap pm;
};

struct DrawTiledPixmap {
    QRectF rect;
    QPixmap pm;
    QPointF offset;
};
}
#endif // MU_DRAW_DRAWTYPES_H
