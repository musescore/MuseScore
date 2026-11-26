/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <memory>

#include "types/flags.h"
#include "types/inputtypes.h"
#include "types/string.h"
#include "../types/types.h"

namespace mu::engraving {
#ifdef Q_OS_MAC
constexpr muse::KeyboardModifier TextEditingControlModifier = muse::KeyboardModifier::AltModifier;
#else
constexpr muse::KeyboardModifier TextEditingControlModifier = muse::KeyboardModifier::ControlModifier;
#endif

//---------------------------------------------------------
//   Grip
//---------------------------------------------------------

enum class Grip : signed char {
    NO_GRIP = -1,
    START = 0, END = 1,                           // arpeggio etc.
    LEFT = START, RIGHT = END,                    // aliases for dynamic
    MIDDLE = 2, APERTURE = 3,                     // Line
    /*START, END , */
    BEZIER1 = 2, SHOULDER = 3, BEZIER2 = 4, DRAG = 5,       // Slur
    GRIPS = 6                       // number of grips for slur
};

//---------------------------------------------------------
//   EditData
//    used in dragGrip
//---------------------------------------------------------
class ElementEditData;
using ElementEditDataPtr = std::shared_ptr<ElementEditData>;

class MuseScoreView;

class EngravingItem;
class EditData
{
public:
    MuseScoreView* view() const { return m_view; }

    bool isEditMode = false; // refers to the legacy concept of "edit mode"

    std::vector<RectF> grip;
    int grips = 0;                 // number of grips
    Grip curGrip = Grip::NO_GRIP;

    PointF pos;
    PointF startMove;
    PointF normalizedStartMove; ///< Introduced for transition of drag logic. Don't use in new code.
    Point startMovePixel;
    PointF lastPos;
    PointF delta;               ///< This property is deprecated, use evtDelta or moveDelta instead. In normal drag equals to moveDelta, in edit drag - to evtDelta
    PointF evtDelta;            ///< Mouse offset for the last mouse move event
    PointF moveDelta;           ///< Mouse offset from the start of mouse move
    bool hRaster = false;
    bool vRaster = false;
    bool isHairpinDragCreatedFromDynamic = false;

    int key = 0;
    muse::KeyboardModifiers modifiers;
    String s;
    String preeditString;

    muse::MouseButtons buttons = muse::MouseButton::NoButton;

    // drop data:
    PointF dragOffset;
    EngravingItem* element = nullptr;
    EngravingItem* dropElement = nullptr;
    track_idx_t track = muse::nidx;

    EditData(MuseScoreView* v = nullptr)
        : m_view(v) {}

    void clear();

    std::shared_ptr<ElementEditData> getData(const EngravingItem*) const;
    void addData(std::shared_ptr<ElementEditData>);
    bool control(bool textEditing = false) const;
    bool shift() const { return modifiers & muse::KeyboardModifier::ShiftModifier; }
    bool isStartEndGrip() const { return curGrip == Grip::START || curGrip == Grip::END; }
    bool hasCurrentGrip() const { return curGrip != Grip::NO_GRIP; }

private:
    std::vector<std::shared_ptr<ElementEditData> > m_data;
    MuseScoreView* m_view = nullptr;
};
}
