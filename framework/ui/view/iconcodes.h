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

#ifdef FILE_OPEN
#undef FILE_OPEN
#endif

namespace mu {
namespace framework {
/**
 * @brief The IconCode class simplifies access to the icons from the icon font
 *
 * @details Each enum value is a UTF-16-like address of the icon in the icon font.
 *          The current icon-font (MusescoreIcon.ttf) is located in the 'MuseScore/fonts/mscore' folder,
 *          The most actual version can be found by this persistent URL: @link https://www.dropbox.com/s/ip59ren10u69hr7/MusescoreIcon.ttf?dl=0
 */

class IconCode
{
    Q_GADGET

public:
    enum class Code : char16_t {
        TRASH = 0xE812,
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
        NEW_FILE = 0xEF22,
        OPEN_FILE = 0xEF23,
        SHARE_FILE = 0xEF24,
        CLOUD_FILE = 0xEF25,
        REWIND = 0xEF26,
        MIXER = 0xEF27,
        CONFIGURE = 0xEF28,
        SAVE = 0xEF29,
        PLUS = 0xEF2A,
        MINUS = 0xEF2B,
        DELETE_TANK = 0xEF2C,
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
        TEXT_ALIGN_ABOVE = 0xEF3A,
        TEXT_ALIGN_UNDER = 0xEF3B,
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
        LINE_WITH_END_HOOK = 0xEF85,
        LINE_WITH_ANGLED_END_HOOK = 0xEF86,
        LINE_PEDAL_STAR_ENDING = 0xEF87,
        BEAM_FEATHERING_LEFT = 0xEF88,
        BEAM_FEATHERING_RIGHT = 0xEF89,

        RIGHT_GAP = 0xEF90,
        LEFT_GAP = 0xEF91,
        GAP_BELOW = 0xEF92,
        GAP_ABOVE = 0xEF93,
        LEFT_MARGIN = 0xEF94,
        RIGHT_MARGIN = 0xEF95,
        BOTTOM_MARGIN = 0xEF96,
        TOP_MARGIN = 0xEF97,
        ACCOUNT = 0xEF99,

        STAR = 0xEF9A,
        LINE_WITH_T_LIKE_END_HOOK = 0xEF9C,
        LINE_WITH_T_LINE_START_HOOK = 0xEF9D,
        LINE_WITH_START_HOOK = 0xEF9E,
        LINE_WITH_ANGLED_START_HOOK = 0xEF9F,
        IMAGE_MOUNTAINS = 0xEFA1,

        GRADUATION_CAP = 0xF19D,

        QUESTION_MARK = 0xF340,
        NOTE_HEAD_QUARTER = 0xF341,
        NOTE_HEAD_HALF = 0xF342,
        NOTE_HEAD_WHOLE = 0xF343,
        NOTE_HEAD_BREVIS = 0xF344,
        BARLINE_WINGED = 0xF34C,
        BARLINE_UNWINGED = 0xF34D,
        ORIENTATION_PORTRAIT = 0xF350,
        ORIENTATION_LANDSCAPE = 0xF351,

        CHORD_SYMBOL = 0xF352,
        GUITAR_BEND = 0xF353,
        MULTIMEASURE_REST = 0xF355,
        GUITAR_TREMOLO_BAR = 0xF35C,

        TREMOLO_ONE_NOTE = 0xF361,
        TREMOLO_TWO_NOTES = 0xF35F,
        TREMOLO_STYLE_DEFAULT = 0xF35F,
        TREMOLO_STYLE_ALL_STROKES_ATTACHED = 0xF360,
        TREMOLO_STYLE_SINGLE_STROKE_ATTACHED = 0xF362,

        KEY_SIGNATURE_NONE = 0xF377,
        KEY_SIGNATURE_1_SHARP = 0xF378,
        KEY_SIGNATURE_2_SHARPS = 0xF379,
        KEY_SIGNATURE_3_SHARPS = 0xF37A,
        KEY_SIGNATURE_4_SHARPS = 0xF37B,
        KEY_SIGNATURE_5_SHARPS = 0xF37C,
        KEY_SIGNATURE_6_SHARPS = 0xF37D,
        KEY_SIGNATURE_7_SHARPS = 0xF37E,
        KEY_SIGNATURE_1_FLAT = 0xF37F,
        KEY_SIGNATURE_2_FLAT = 0xF380,
        KEY_SIGNATURE_3_FLAT = 0xF381,
        KEY_SIGNATURE_4_FLAT = 0xF382,
        KEY_SIGNATURE_5_FLAT = 0xF383,
        KEY_SIGNATURE_6_FLAT = 0xF384,
        KEY_SIGNATURE_7_FLAT = 0xF385,

        AUTO,
        NONE,
        CUSTOM
    };

    Q_ENUM(Code)
};
}
}

#endif // MU_FRAMEWORK_ICONCODE_H
