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
#ifndef MU_DRAW_BUFFEREDDRAWTYPES_H
#define MU_DRAW_BUFFEREDDRAWTYPES_H

#include <memory>

#include "brush.h"
#include "drawtypes.h"
#include "geometry.h"
#include "font.h"
#include "pen.h"
#include "pixmap.h"
#include "transform.h"
#include "painterpath.h"

namespace mu::draw {
enum class DrawMode {
    Stroke = 0,
    Fill,
    StrokeAndFill
};

struct Scale {
    double x = 0.0;
    double y = 0.0;
};

struct DrawPath {
    PainterPath path;
    Pen pen;
    Brush brush;
    DrawMode mode = DrawMode::StrokeAndFill;
};

struct DrawRect {
    RectF rect;
    Pen pen;
    Brush brush;
    DrawMode mode = DrawMode::StrokeAndFill;
};

struct DrawPolygon {
    PolygonF polygon;
    PolygonMode mode = PolygonMode::OddEven;
};

struct DrawText {
    PointF pos;
    QString text;
};

struct DrawRectText {
    RectF rect;
    int flags = 0;
    QString text;
};

struct DrawPixmap {
    PointF pos;
    Pixmap pm;
};

struct DrawTiledPixmap {
    RectF rect;
    Pixmap pm;
    PointF offset;
};

struct DrawData
{
    struct State {
        Pen pen;
        Brush brush;
        Font font;
        Transform transform;
        bool isAntialiasing = false;
        CompositionMode compositionMode = CompositionMode::SourceOver;
    };

    struct Data {
        State state;

        std::vector<DrawPath> paths;
        std::vector<DrawPolygon> polygons;
        std::vector<DrawText> texts;
        std::vector<DrawRectText> rectTexts;
        std::vector<DrawPixmap> pixmaps;
        std::vector<DrawTiledPixmap> tiledPixmap;

        bool empty() const
        {
            return paths.empty()
                   && polygons.empty()
                   && texts.empty()
                   && rectTexts.empty()
                   && pixmaps.empty()
                   && tiledPixmap.empty();
        }
    };

    struct Object {
        std::string name;
        PointF pagePos;
        std::vector<Data> datas;

        Object() = default;
        Object(const std::string& n, const PointF& p)
            : name(n), pagePos(p)
        {
            //! NOTE Make data with default state
            datas.push_back(DrawData::Data());
        }
    };

    std::string name;
    std::vector<Object> objects;
};

using DrawDataPtr = std::shared_ptr<DrawData>;

struct Diff {
    DrawDataPtr dataAdded;
    DrawDataPtr dataRemoved;

    bool empty() const
    {
        bool ret = true;
        if (dataAdded) {
            ret = dataAdded->objects.empty();
        }

        if (ret && dataRemoved) {
            ret = dataRemoved->objects.empty();
        }

        return ret;
    }
};
}
#endif // MU_DRAW_BUFFEREDDRAWTYPES_H
