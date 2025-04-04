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

#pragma once

#include "global/types/string.h"

#include "../types/propertyvalue.h"

namespace mu::engraving {
//------------------------------------------------------------------------
//    M_PROPERTY (type, getter_name, setter_name)
//       helper macro to define a styled ScoreElement property
//
//    usage example:
//    class Text : public EngravingItem {
//          M_PROPERTY(Color, color, setColor)
//          ...
//          };
//    this defines:
//          Color _color;
//          const Color& color() const { return _color; }
//          void setColor(const Color& val) { _color = val; }
//---------------------------------------------------------

#define M_PROPERTY(a, b, c)                                      \
    a _##b { };                                                \
public:                                                     \
    const a& b() const { return _##b; }                  \
    void c(const a& val) { _##b = val; }                  \
private:

#define M_PROPERTY2(a, b, c, d)                                   \
    a _##b { d };                                          \
public:                                                     \
    const a& b() const { return _##b; }                  \
    void c(const a& val) { _##b = val; }                  \
private:

//---------------------------------------------------------
//   PropertyFlags
//---------------------------------------------------------

enum class PropertyFlags : char {
    NOSTYLE, UNSTYLED, STYLED
};

//------------------------------------------------------------------------
//   EngravingItem Properties
//------------------------------------------------------------------------

enum class Pid {
    SUBTYPE,
    SELECTED,
    GENERATED,
    COLOR,
    VISIBLE,
    Z,
    SMALL,
    SHOW_COURTESY,
    KEYSIG_MODE,
    SLUR_STYLE_TYPE,
    PITCH,

    TPC1,
    TPC2,
    LINE,
    FIXED,
    FIXED_LINE,
    HEAD_TYPE,
    HEAD_GROUP,
    VELO_TYPE,
    USER_VELOCITY,
    ARTICULATION_ANCHOR,

    DIRECTION,
    HORIZONTAL_DIRECTION,
    STEM_DIRECTION,
    NO_STEM,
    SLUR_DIRECTION,
    LEADING_SPACE,
    MIRROR_HEAD,
    HEAD_HAS_PARENTHESES,
    DOT_POSITION,
    COMBINE_VOICE,
    TUNING,
    PAUSE,

    BARLINE_TYPE,
    BARLINE_SPAN,
    BARLINE_SPAN_FROM,
    BARLINE_SPAN_TO,
    BARLINE_SHOW_TIPS,
    OFFSET,
    FRET,
    STRING,
    GHOST,
    DEAD,
    PLAY,
    TIMESIG_NOMINAL,

    TIMESIG_ACTUAL,
    NUMBER_TYPE,
    BRACKET_TYPE,
    NORMAL_NOTES,
    ACTUAL_NOTES,
    P1,
    P2,
    GROW_LEFT,
    GROW_RIGHT,
    BOX_HEIGHT,
    BOX_WIDTH,
    BOX_AUTOSIZE,
    TOP_GAP,
    BOTTOM_GAP,
    LEFT_MARGIN,
    RIGHT_MARGIN,
    TOP_MARGIN,
    BOTTOM_MARGIN,
    LAYOUT_BREAK,
    AUTOSCALE,
    SIZE,
    IMAGE_HEIGHT,
    IMAGE_WIDTH,
    IMAGE_FRAMED,

    FRET_FRAME_TEXT_SCALE,
    FRET_FRAME_DIAGRAM_SCALE,
    FRET_FRAME_COLUMN_GAP,
    FRET_FRAME_ROW_GAP,
    FRET_FRAME_CHORDS_PER_ROW,
    FRET_FRAME_H_ALIGN,

    SCALE,
    LOCK_ASPECT_RATIO,
    SIZE_IS_SPATIUM,
    TEXT,
    HTML_TEXT,
    USER_MODIFIED,
    BEAM_POS,
    BEAM_MODE,
    BEAM_NO_SLOPE,
    BEAM_CROSS_STAFF_MOVE,
    USER_LEN,         // used for stems
    SHOW_STEM_SLASH,  // used for grace notes

    SPACE,            // used for spacer
    TEMPO,
    TEMPO_FOLLOW_TEXT,
    ACCIDENTAL_BRACKET,
    ACCIDENTAL_TYPE,
    ACCIDENTAL_STACKING_ORDER_OFFSET,
    NUMERATOR_STRING,
    DENOMINATOR_STRING,
    FBPREFIX,               // used for FiguredBassItem
    FBDIGIT,                //    "           "
    FBSUFFIX,               //    "           "
    FBCONTINUATIONLINE,     //    "           "

    FBPARENTHESIS1,         //    "           "
    FBPARENTHESIS2,         //    "           "
    FBPARENTHESIS3,         //    "           "
    FBPARENTHESIS4,         //    "           "
    FBPARENTHESIS5,         //    "           "
    OTTAVA_TYPE,
    NUMBERS_ONLY,
    TRILL_TYPE,
    VIBRATO_TYPE,
    HAIRPIN_CIRCLEDTIP,

    HAIRPIN_TYPE,
    HAIRPIN_HEIGHT,
    HAIRPIN_CONT_HEIGHT,
    VELO_CHANGE,
    VELO_CHANGE_METHOD,
    VELO_CHANGE_SPEED,
    DYNAMIC_TYPE,

    SINGLE_NOTE_DYNAMICS,
    CHANGE_METHOD,
    PLACEMENT,                // Goes with P_TYPE::PLACEMENT
    HPLACEMENT,               // Goes with P_TYPE::HPLACEMENT
    MMREST_RANGE_BRACKET_TYPE,   // The brackets used arond the measure numbers indicating the range covered by the mmrest
    VELOCITY,
    JUMP_TO,
    PLAY_UNTIL,
    CONTINUE_AT,
    LABEL,
    MARKER_TYPE,
    ARP_USER_LEN1,
    ARP_USER_LEN2,
    REPEAT_END,
    REPEAT_START,
    REPEAT_JUMP,
    MEASURE_NUMBER_MODE,

    GLISS_TYPE,
    GLISS_TEXT,
    GLISS_SHOW_TEXT,
    GLISS_STYLE,
    GLISS_SHIFT,
    GLISS_EASEIN,
    GLISS_EASEOUT,
    DIAGONAL,
    GROUP_NODES,
    LINE_STYLE,
    LINE_WIDTH,
    TIME_STRETCH,
    ORNAMENT_STYLE,
    INTERVAL_ABOVE,
    INTERVAL_BELOW,
    ORNAMENT_SHOW_ACCIDENTAL,
    ORNAMENT_SHOW_CUE_NOTE,
    START_ON_UPPER_NOTE,

    TIMESIG,
    TIMESIG_GLOBAL,
    TIMESIG_STRETCH,
    TIMESIG_TYPE,
    SPANNER_TICK,
    SPANNER_TICKS,
    SPANNER_TRACK2,
    OFFSET2,
    BREAK_MMR,
    MMREST_NUMBER_POS,
    MMREST_NUMBER_OFFSET,
    MMREST_NUMBER_VISIBLE,
    MEASURE_REPEAT_NUMBER_POS,
    REPEAT_COUNT,

    USER_STRETCH,
    NO_OFFSET,
    IRREGULAR,
    ANCHOR,
    SLUR_UOFF1,
    SLUR_UOFF2,
    SLUR_UOFF3,
    SLUR_UOFF4,
    STAFF_MOVE,
    VERSE,

    SYLLABIC,
    LYRIC_TICKS,
    VOLTA_ENDING,
    LINE_VISIBLE,
    MAG,
    USE_DRUMSET,
    DURATION,
    DURATION_TYPE_WITH_DOTS,
    ACCIDENTAL_ROLE,
    TRACK,

    FRET_STRINGS,
    FRET_FRETS,
    FRET_NUT,
    FRET_OFFSET,
    FRET_NUM_POS,
    ORIENTATION,
    FRET_SHOW_FINGERINGS,
    FRET_FINGERING,

    HARMONY_VOICE_LITERAL,
    HARMONY_VOICING,
    HARMONY_DURATION,

    SYSTEM_BRACKET,
    GAP,
    AUTOPLACE,
    DASH_LINE_LEN,
    DASH_GAP_LEN,
    TICK,
    PLAYBACK_VOICE1,
    PLAYBACK_VOICE2,
    PLAYBACK_VOICE3,

    PLAYBACK_VOICE4,
    SYMBOL,
    PLAY_REPEATS,
    CREATE_SYSTEM_HEADER,
    STAFF_LINES,
    LINE_DISTANCE,
    STEP_OFFSET,
    STAFF_SHOW_BARLINES,
    STAFF_SHOW_LEDGERLINES,
    STAFF_STEMLESS,
    STAFF_INVISIBLE,
    STAFF_COLOR,

    HEAD_SCHEME,
    STAFF_GEN_CLEF,
    STAFF_GEN_TIMESIG,
    STAFF_GEN_KEYSIG,
    STAFF_YOFFSET,
    STAFF_USERDIST,
    STAFF_BARLINE_SPAN,
    STAFF_BARLINE_SPAN_FROM,
    STAFF_BARLINE_SPAN_TO,
    BRACKET_SPAN,

    BRACKET_COLUMN,
    INAME_LAYOUT_POSITION,

    TEXT_STYLE,

    FONT_FACE,
    FONT_SIZE,
    FONT_STYLE,
    TEXT_LINE_SPACING,

    FRAME_TYPE,
    FRAME_WIDTH,
    FRAME_PADDING,
    FRAME_ROUND,
    FRAME_FG_COLOR,

    FRAME_BG_COLOR,
    SIZE_SPATIUM_DEPENDENT,
    TEXT_SIZE_SPATIUM_DEPENDENT, // for text component of textLine items
    MUSICAL_SYMBOLS_SCALE,
    ALIGN,
    TEXT_SCRIPT_ALIGN,
    SYSTEM_FLAG,
    BEGIN_TEXT,

    BEGIN_TEXT_ALIGN,
    BEGIN_TEXT_PLACE,
    BEGIN_HOOK_TYPE,
    BEGIN_HOOK_HEIGHT,
    BEGIN_FONT_FACE,
    BEGIN_FONT_SIZE,
    BEGIN_FONT_STYLE,
    BEGIN_TEXT_OFFSET,
    GAP_BETWEEN_TEXT_AND_LINE,

    CONTINUE_TEXT,
    CONTINUE_TEXT_ALIGN,
    CONTINUE_TEXT_PLACE,
    CONTINUE_FONT_FACE,
    CONTINUE_FONT_SIZE,
    CONTINUE_FONT_STYLE,
    CONTINUE_TEXT_OFFSET,
    END_TEXT,

    END_TEXT_ALIGN,
    END_TEXT_PLACE,
    END_HOOK_TYPE,
    END_HOOK_HEIGHT,
    END_FONT_FACE,
    END_FONT_SIZE,
    END_FONT_STYLE,
    END_TEXT_OFFSET,

    NOTELINE_PLACEMENT,

    AVOID_BARLINES, // meant for Dynamics
    DYNAMICS_SIZE,
    CENTER_ON_NOTEHEAD,
    ANCHOR_TO_END_OF_PREVIOUS,

    SNAP_TO_DYNAMICS, // pre-4.4 version of the property, specific for expression
    SNAP_BEFORE,
    SNAP_AFTER,

    VOICE_ASSIGNMENT,
    CENTER_BETWEEN_STAVES,

    POS_ABOVE,

    LOCATION_STAVES,
    LOCATION_VOICES,
    LOCATION_MEASURES,
    LOCATION_FRACTIONS,
    LOCATION_GRACE,
    LOCATION_NOTE,

    VOICE,
    POSITION,

    CLEF_TYPE_CONCERT,
    CLEF_TYPE_TRANSPOSING,
    CLEF_TO_BARLINE_POS,
    IS_HEADER, // for clefs
    KEY_CONCERT,
    KEY,
    ACTION,   // for ActionIcon
    MIN_DISTANCE,

    ARPEGGIO_TYPE,
    CHORD_LINE_TYPE,
    CHORD_LINE_STRAIGHT,
    CHORD_LINE_WAVY,
    TREMOLO_TYPE,
    TREMOLO_STYLE,
    HARMONY_TYPE,

    ARPEGGIO_SPAN,

    BEND_TYPE,
    BEND_CURVE,
    BEND_VERTEX_OFF,
    BEND_SHOW_HOLD_LINE,
    BEND_START_TIME_FACTOR,
    BEND_END_TIME_FACTOR,

    TREMOLOBAR_TYPE,
    TREMOLOBAR_CURVE,

    START_WITH_LONG_NAMES,
    START_WITH_MEASURE_ONE,
    FIRST_SYSTEM_INDENTATION,

    PATH,   // for ChordLine to make its shape changes undoable

    PREFER_SHARP_FLAT,
    PLAY_TECH_TYPE,
    TEMPO_CHANGE_TYPE,
    TEMPO_EASING_METHOD,
    TEMPO_CHANGE_FACTOR,

    HARP_IS_DIAGRAM,

    ACTIVE,

    CAPO_FRET_POSITION,
    CAPO_IGNORED_STRINGS,
    CAPO_GENERATE_TEXT,

    TIE_PLACEMENT,
    MIN_LENGTH,

    PARTIAL_SPANNER_DIRECTION,

    POSITION_LINKED_TO_MASTER,
    APPEARANCE_LINKED_TO_MASTER,
    TEXT_LINKED_TO_MASTER,
    EXCLUDE_FROM_OTHER_PARTS,

    STRINGTUNINGS_STRINGS_COUNT,
    STRINGTUNINGS_PRESET,
    STRINGTUNINGS_VISIBLE_STRINGS,

    SCORE_FONT,
    SYMBOLS_SIZE,
    SYMBOL_ANGLE,

    APPLY_TO_ALL_STAVES,

    IS_COURTESY,

    END
};

// Determines propagation of properties between score and parts
enum class PropertyPropagation : unsigned char {
    NONE,
    PROPAGATE,
    UNLINK,
};

// Each group can be propagated differently between score and parts
enum class PropertyGroup : unsigned char {
    POSITION,
    TEXT,
    APPEARANCE,
    NONE, // Properties that should not be taken into account when propagating
};

using PropertyIdSet = std::unordered_set<Pid>;

extern PropertyValue propertyFromString(P_TYPE type, String value);
extern String propertyToString(Pid, const PropertyValue& value, bool mscx);
extern P_TYPE propertyType(Pid);
extern const char* propertyName(Pid);
extern bool propertyLink(Pid id);
extern bool propertyLinkSameScore(Pid id);
extern PropertyGroup propertyGroup(Pid id);
extern Pid propertyId(const muse::AsciiStringView& name);
extern String propertyUserName(Pid);
} // namespace mu::engraving
