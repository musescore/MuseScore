/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "types/flags.h"
#include "types/string.h"
#include "../types/types.h"

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
constexpr KeyboardModifier TextEditingControlModifier = AltModifier;
#else
constexpr KeyboardModifier TextEditingControlModifier = ControlModifier;
#endif

enum KeyboardKey {
    Key_Escape = 0x01000000,                // misc keys
    Key_Tab = 0x01000001,
    Key_Backtab = 0x01000002,
    Key_Backspace = 0x01000003,
    Key_Return = 0x01000004,
    Key_Enter = 0x01000005,
    Key_Insert = 0x01000006,
    Key_Delete = 0x01000007,
    Key_Pause = 0x01000008,
    Key_Print = 0x01000009,               // print screen
    Key_SysReq = 0x0100000a,
    Key_Clear = 0x0100000b,
    Key_Home = 0x01000010,                // cursor movement
    Key_End = 0x01000011,
    Key_Left = 0x01000012,
    Key_Up = 0x01000013,
    Key_Right = 0x01000014,
    Key_Down = 0x01000015,
    Key_PageUp = 0x01000016,
    Key_PageDown = 0x01000017,
    Key_Shift = 0x01000020,                // modifiers
    Key_Control = 0x01000021,
    Key_Meta = 0x01000022,
    Key_Alt = 0x01000023,
    Key_CapsLock = 0x01000024,
    Key_NumLock = 0x01000025,
    Key_ScrollLock = 0x01000026,
    Key_F1 = 0x01000030,                // function keys
    Key_F2 = 0x01000031,
    Key_F3 = 0x01000032,
    Key_F4 = 0x01000033,
    Key_F5 = 0x01000034,
    Key_F6 = 0x01000035,
    Key_F7 = 0x01000036,
    Key_F8 = 0x01000037,
    Key_F9 = 0x01000038,
    Key_F10 = 0x01000039,
    Key_F11 = 0x0100003a,
    Key_F12 = 0x0100003b,
    Key_F13 = 0x0100003c,
    Key_F14 = 0x0100003d,
    Key_F15 = 0x0100003e,
    Key_F16 = 0x0100003f,
    Key_F17 = 0x01000040,
    Key_F18 = 0x01000041,
    Key_F19 = 0x01000042,
    Key_F20 = 0x01000043,
    Key_F21 = 0x01000044,
    Key_F22 = 0x01000045,
    Key_F23 = 0x01000046,
    Key_F24 = 0x01000047,
    Key_F25 = 0x01000048,                // F25 .. F35 only on X11
    Key_F26 = 0x01000049,
    Key_F27 = 0x0100004a,
    Key_F28 = 0x0100004b,
    Key_F29 = 0x0100004c,
    Key_F30 = 0x0100004d,
    Key_F31 = 0x0100004e,
    Key_F32 = 0x0100004f,
    Key_F33 = 0x01000050,
    Key_F34 = 0x01000051,
    Key_F35 = 0x01000052,
    Key_Super_L = 0x01000053,                 // extra keys
    Key_Super_R = 0x01000054,
    Key_Menu = 0x01000055,
    Key_Hyper_L = 0x01000056,
    Key_Hyper_R = 0x01000057,
    Key_Help = 0x01000058,
    Key_Direction_L = 0x01000059,
    Key_Direction_R = 0x01000060,
    Key_Space = 0x20,                // 7 bit printable ASCII
    Key_Any = Key_Space,
    Key_Exclam = 0x21,
    Key_QuoteDbl = 0x22,
    Key_NumberSign = 0x23,
    Key_Dollar = 0x24,
    Key_Percent = 0x25,
    Key_Ampersand = 0x26,
    Key_Apostrophe = 0x27,
    Key_ParenLeft = 0x28,
    Key_ParenRight = 0x29,
    Key_Asterisk = 0x2a,
    Key_Plus = 0x2b,
    Key_Comma = 0x2c,
    Key_Minus = 0x2d,
    Key_Period = 0x2e,
    Key_Slash = 0x2f,
    Key_0 = 0x30,
    Key_1 = 0x31,
    Key_2 = 0x32,
    Key_3 = 0x33,
    Key_4 = 0x34,
    Key_5 = 0x35,
    Key_6 = 0x36,
    Key_7 = 0x37,
    Key_8 = 0x38,
    Key_9 = 0x39,
    Key_Colon = 0x3a,
    Key_Semicolon = 0x3b,
    Key_Less = 0x3c,
    Key_Equal = 0x3d,
    Key_Greater = 0x3e,
    Key_Question = 0x3f,
    Key_At = 0x40,
    Key_A = 0x41,
    Key_B = 0x42,
    Key_C = 0x43,
    Key_D = 0x44,
    Key_E = 0x45,
    Key_F = 0x46,
    Key_G = 0x47,
    Key_H = 0x48,
    Key_I = 0x49,
    Key_J = 0x4a,
    Key_K = 0x4b,
    Key_L = 0x4c,
    Key_M = 0x4d,
    Key_N = 0x4e,
    Key_O = 0x4f,
    Key_P = 0x50,
    Key_Q = 0x51,
    Key_R = 0x52,
    Key_S = 0x53,
    Key_T = 0x54,
    Key_U = 0x55,
    Key_V = 0x56,
    Key_W = 0x57,
    Key_X = 0x58,
    Key_Y = 0x59,
    Key_Z = 0x5a,
    Key_BracketLeft = 0x5b,
    Key_Backslash = 0x5c,
    Key_BracketRight = 0x5d,
    Key_AsciiCircum = 0x5e,
    Key_Underscore = 0x5f,
    Key_QuoteLeft = 0x60,
    Key_BraceLeft = 0x7b,
    Key_Bar = 0x7c,
    Key_BraceRight = 0x7d,
    Key_AsciiTilde = 0x7e,

    Key_nobreakspace = 0x0a0,
    Key_periodcentered = 0x0b7,
    Key_ydiaeresis = 0x0ff,
};

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
//    used in editDrag
//---------------------------------------------------------
class ElementEditData;
using ElementEditDataPtr = std::shared_ptr<ElementEditData>;

class MuseScoreView;

class EngravingItem;
class EditData
{
public:
    MuseScoreView* view() const { return m_view; }

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
    bool editTextualProperties = true;
    bool isHairpinDragCreatedFromDynamic = false;

    int key = 0;
    bool isKeyRelease = false;
    KeyboardModifiers modifiers  { /*0*/ };   // '0' initialized via default constructor, doing it here too results in compiler warning with Qt 5.15
    String s;
    String preeditString;

    MouseButtons buttons = NoButton;

    // drop data:
    PointF dragOffset;
    EngravingItem* element = nullptr;
    EngravingItem* dropElement = nullptr;

    EditData(MuseScoreView* v = nullptr)
        : m_view(v) {}

    void clear();

    std::shared_ptr<ElementEditData> getData(const EngravingItem*) const;
    void addData(std::shared_ptr<ElementEditData>);
    bool control(bool textEditing = false) const;
    bool shift() const { return modifiers & ShiftModifier; }
    bool isStartEndGrip() const { return curGrip == Grip::START || curGrip == Grip::END; }
    bool hasCurrentGrip() const { return curGrip != Grip::NO_GRIP; }

private:
    std::list<std::shared_ptr<ElementEditData> > m_data;
    MuseScoreView* m_view = nullptr;
};
}

#endif // MU_ENGRAVING_EDITDATA_H
