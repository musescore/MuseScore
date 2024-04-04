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
#ifndef MUSE_DRAW_BUFFEREDDRAWTYPES_H
#define MUSE_DRAW_BUFFEREDDRAWTYPES_H

#include <memory>

#include "brush.h"
#include "drawtypes.h"
#include "geometry.h"
#include "font.h"
#include "pen.h"
#include "pixmap.h"
#include "transform.h"
#include "painterpath.h"

namespace muse::draw {
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
    bool operator==(const DrawPath& o) const
    {
        return mode == o.mode && pen == o.pen && brush == o.brush && path == o.path;
    }

    bool operator!=(const DrawPath& o) const { return !this->operator==(o); }
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
    bool operator==(const DrawPolygon& o) const { return mode == o.mode && polygon == o.polygon; }
    bool operator!=(const DrawPolygon& o) const { return !this->operator==(o); }
};

struct DrawText {
    enum Mode {
        Undefined = 0,
        Point,
        Rect
    };

    Mode mode = Mode::Undefined;
    RectF rect;     // If mode is Point when use topLeft point
    int flags = 0;
    String text;
    bool operator==(const DrawText& o) const
    {
        return mode == o.mode && flags == o.flags && rect == o.rect && text == o.text;
    }

    bool operator!=(const DrawText& o) const { return !this->operator==(o); }
};

struct DrawPixmap {
    enum Mode {
        Undefined = 0,
        Single,
        Tiled
    };

    Mode mode = Mode::Undefined;
    RectF rect;     // If mode is Single when use topLeft point
    Pixmap pm;
    PointF offset;  // used only for Tiled mode
};

struct DrawData
{
    static const int CANVAS_DPI = 360;

    struct State {
        Pen pen;
        Brush brush;
        Font font;
        Transform transform;
        bool isAntialiasing = false;
        CompositionMode compositionMode = CompositionMode::SourceOver;

        bool operator==(const State& o) const
        {
            return pen == o.pen && brush == o.brush && font == o.font && transform == o.transform
                   && isAntialiasing == o.isAntialiasing && compositionMode == o.compositionMode;
        }

        bool operator!=(const State& o) const { return !this->operator==(o); }
    };

    struct Data {
        int state = 0;

        std::vector<DrawPath> paths;
        std::vector<DrawPolygon> polygons;
        std::vector<DrawText> texts;
        std::vector<DrawPixmap> pixmaps;

        bool empty() const { return paths.empty() && polygons.empty() && texts.empty() && pixmaps.empty(); }
    };

    struct Item {
        std::string name;
        std::vector<Data> datas;
        std::vector<Item> chilren;

        Item() = default;
        Item(const std::string& n)
            : name(n) {}
    };

    std::string name;
    RectF viewport;
    Item item;
    std::map<int, State> states;

    bool empty() const { return item.datas.empty() && item.chilren.empty(); }
};

using DrawDataPtr = std::shared_ptr<DrawData>;

struct Diff {
    DrawDataPtr dataAdded;
    DrawDataPtr dataRemoved;

    bool empty() const
    {
        bool ret = true;
        if (dataAdded) {
            ret = dataAdded->empty();
        }

        if (ret && dataRemoved) {
            ret = dataRemoved->empty();
        }

        return ret;
    }
};
}
#endif // MUSE_DRAW_BUFFEREDDRAWTYPES_H
