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

#ifndef MU_ENGRAVING_EDITDATA_H
#define MU_ENGRAVING_EDITDATA_H

#include <memory>

#include "types/string.h"
#include "types/flags.h"
#include "infrastructure/draw/geometry.h"

namespace mu::engraving {
enum KeyboardModifier {
    NoModifier           = 0x00000000,
    ShiftModifier        = 0x02000000,
    ControlModifier      = 0x04000000,
    AltModifier          = 0x08000000,
    MetaModifier         = 0x10000000,
    KeypadModifier       = 0x20000000,
    GroupSwitchModifier  = 0x40000000,
    // Do not extend the mask to include 0x01000000
    KeyboardModifierMask = 0xfe000000
};
DECLARE_FLAGS(KeyboardModifiers, KeyboardModifier)
DECLARE_OPERATORS_FOR_FLAGS(KeyboardModifiers)

#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER ControlModifier
#endif

enum MouseButton {
    NoButton         = 0x00000000,
    LeftButton       = 0x00000001,
    RightButton      = 0x00000002,
    MiddleButton     = 0x00000004
};
DECLARE_FLAGS(MouseButtons, MouseButton)
DECLARE_OPERATORS_FOR_FLAGS(MouseButtons)

//---------------------------------------------------------
//   Grip
//---------------------------------------------------------

enum class Grip {
    NO_GRIP = -1,
    START = 0, END = 1,                           // arpeggio etc.
    MIDDLE = 2, APERTURE = 3,                     // Line
    /*START, END , */
    BEZIER1 = 2, SHOULDER = 3, BEZIER2 = 4, DRAG = 5,       // Slur
    GRIPS = 6                       // number of grips for slur
};

//---------------------------------------------------------
//   EditData
//    used in editDrag
//---------------------------------------------------------
class ElementEditData;
using ElementEditDataPtr = std::shared_ptr<ElementEditData>;

class MuseScoreView;

class EngravingItem;
class EditData
{
    std::list<std::shared_ptr<ElementEditData> > data;
    MuseScoreView* view_ { 0 };

public:
    MuseScoreView* view() const { return view_; }

    std::vector<RectF> grip;
    int grips                        { 0 };                 // number of grips
    Grip curGrip                     { Grip::NO_GRIP };

    PointF pos;
    PointF startMove;
    PointF normalizedStartMove; ///< Introduced for transition of drag logic. Don't use in new code.
    Point startMovePixel;
    PointF lastPos;
    PointF delta;               ///< This property is deprecated, use evtDelta or moveDelta instead. In normal drag equals to moveDelta, in edit drag - to evtDelta
    PointF evtDelta;            ///< Mouse offset for the last mouse move event
    PointF moveDelta;           ///< Mouse offset from the start of mouse move
    bool hRaster                     { false };
    bool vRaster                     { false };

    int key                          { 0 };
    KeyboardModifiers modifiers  { /*0*/ };   // '0' initialized via default constructor, doing it here too results in compiler warning with Qt 5.15
    String s;
    String preeditString;

    MouseButtons buttons         { NoButton };

    // drop data:
    PointF dragOffset;
    EngravingItem* element                 { 0 };
    EngravingItem* dropElement             { 0 };

    EditData(MuseScoreView* v = nullptr)
        : view_(v) {}

    void clear();

    std::shared_ptr<ElementEditData> getData(const EngravingItem*) const;
    void addData(std::shared_ptr<ElementEditData>);
    bool control(bool textEditing = false) const;
    bool shift() const { return modifiers & ShiftModifier; }
    bool isStartEndGrip() { return curGrip == Grip::START || curGrip == Grip::END; }
};
}

#endif // MU_ENGRAVING_EDITDATA_H
