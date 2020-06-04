//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef MU_FRAMEWORK_ICONCODE_H
#define MU_FRAMEWORK_ICONCODE_H

#include <QObject>

#ifdef DELETE
#undef DELETE
#endif
#ifdef FILE_OPEN
#undef FILE_OPEN
#endif

namespace mu {
namespace framework {
class IconCode
{
    Q_GADGET

    Q_ENUMS(Code)

public:

    enum class Code : char16_t {
        SMALL_ARROW_UP = 0xEF10,
        SMALL_ARROW_RIGHT = 0xEF11,
        SMALL_ARROW_DOWN = 0xEF12,
        MENU_THREE_DOTS = 0xEF13,
        CLOSE_X_ROUNDED = 0xEF14,
        SPLIT_OUT_ARROWS = 0xEF15,
        ZOOM_OUT = 0xEF16,
        SEARCH = 0xEF17,
        ZOOM_IN = 0xEF18,
        UNDO = 0xEF19,
        REDO = 0xEF1A,
        PAGE_VIEW = 0xEF1B,
        CONTINUOUS_VIEW = 0xEF1C,
        PLAY = 0xEF1D,
        STOP = 0xEF1E,
        LOOP = 0xEF1F,
        METRONOME = 0xEF20,
        TUNING_FORK = 0xEF21,
        FILE_NEW = 0xEF22,
        FILE_OPEN = 0xEF23,
        FILE_SHARE = 0xEF24,
        FILE_CLOUD = 0xEF25,
        REWIND = 0xEF26,
        MIXER = 0xEF27,
        CONFIGURE = 0xEF28,
        SAVE = 0xEF29,
        PLUS = 0xEF2A,
        MINUS = 0xEF2B,
        DELETE = 0xEF2C,
        FEEDBACK = 0xEF2D,
        LINK = 0xEF2E,
        TICK = 0xEF2F,
        CROSS = 0xEF30,
        TICK_RIGHT_ANGLE = 0xEF31,
        HORIZONTAL = 0xEF32,
        VERTICAL = 0xEF33,
        ARROW_RIGHT = 0xEF34,
        ARROW_LEFT = 0xEF35,
        ARROW_DOWN = 0xEF36,
        ARROW_UP = 0xEF37,
        POSITION_ARROWS = 0xEF38,
        TEXT_ALIGN_BASELINE = 0xEF39,
        TEXT_ALIGN_UNDER = 0xEF3A,
        TEXT_ALIGN_ABOVE = 0xEF3B,
        TEXT_ALIGN_MIDDLE = 0xEF3C,
        TEXT_ALIGN_LEFT = 0xEF3D,
        TEXT_ALIGN_CENTER = 0xEF3E,
        TEXT_ALIGN_RIGHT = 0xEF3F,
        TEXT_ITALIC = 0xEF40,
        TEXT_UNDERLINE = 0xEF41,
        TEXT_BOLD = 0xEF42,
        APPLY_GLOBAL_STYLE = 0xEF43,
        HAIRPIN = 0xEF44,
        ACCIDENTAL_SHARP = 0xEF45,
        SLUR = 0xEF46,
        DYNAMIC_FORTE = 0xEF47,
        CRESCENDO_LINE = 0xEF48,
        FRAME_SQUARE = 0xEF49,
        FRAME_CIRCLE = 0xEF4A,
        MUSIC_NOTES = 0xEF4B,
        TEXT_SUBSCRIPT = 0xEF4C,
        TEXT_SUPERSCRIPT = 0xEF4D,
        AUDIO = 0xEF4E,
        VISIBILITY_ON = 0xEF53,
        VISIBILITY_OFF = 0xEF54,
        SETTINGS_COG = 0xEF55,
        FEATHERED_RIGHT_HEIGHT = 0xEF56,
        FEATHERED_LEFT_HEIGHT = 0xEF57,
        BEAM_RIGHT_Y_POSITION = 0xEF5A,
        BEAM_LEFT_Y_POSITION = 0xEF5B,
        LOCK_CLOSED = 0xEF5C,
        LOCK_OPEN = 0xEF5D,
        DOT_ABOVE_LINE = 0xEF5E,
        DOT_BELOW_LINE = 0xEF5F,
        TEXT_BELOW_STAFF = 0xEF60,
        TEXT_ABOVE_STAFF = 0xEF61,
        GLISSANDO = 0xEF62,
        TIME_SIGNATURE = 0xEF64,
        PEDAL_MARKING = 0xEF65,
        MARKER = 0xEF66,
        JUMP = 0xEF67,
        REPEAT_START = 0xEF68,
        FERMATA = 0xEF69,
        SECTION_BREAK = 0xEF6A,
        SPACER = 0xEF6B,
        VERTICAL_FRAME = 0xEF6C,
        HORIZONTAL_FRAME = 0xEF6D,
        TEXT_FRAME = 0xEF6E,
        ORNAMENT = 0xEF6F,
        ARTICULATION = 0xEF70,
        BRACKET = 0xEF71,
        BRACE = 0xEF72,
        CLEF_BASS = 0xEF73,
        MORTAR_BOARD = 0xEF74,
        FRETBOARD_DIAGRAM = 0xEF75,
        FRETBOARD_MARKER_TRIANGLE = 0xEF76,
        FRETBOARD_MARKER_CIRCLE_OUTLINE = 0xEF77,
        FRETBOARD_MARKER_CIRCLE_FILLED = 0xEF78,
        AMBITUS = 0xEF79,
        AMBITUS_LEANING_LEFT = 0xEF7A,
        AMBITUS_LEANING_RIGHT = 0xEF7B,
        BRACKET_PARENTHESES = 0xEF7C,
        BRACKET_PARENTHESES_SQUARE = 0xEF7D,
        STAFF_TYPE_CHANGE = 0xEF7E,
        SPLIT_VIEW_HORIZONTAL = 0xEF7F,
        SPLIT_VIEW_VERTICAL = 0xEF80,
        KEY_SIGNATURE = 0xEF81,
        LINE_DASHED = 0xEF82,
        LINE_DOTTED = 0xEF83,
        LINE_NORMAL = 0xEF84,
        LINE_WITH_HOOK = 0xEF85,
        LINE_WITH_ANGLED_HOOK = 0xEF86,
        LINE_PEDAL_STAR_ENDING = 0xEF87,
        AUTO = 0xEF88,
        NONE = 0xEF89
    };
};
}
}

#endif // MU_FRAMEWORK_ICONCODE_H
