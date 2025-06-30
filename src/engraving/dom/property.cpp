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

#include "property.h"

#include "translation.h"

#include "types/typesconv.h"

#include "accidental.h"
#include "bracket.h"
#include "note.h"
#include "ottava.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   PropertyMetaData
//---------------------------------------------------------

struct PropertyMetaData {
    Pid id;                   // associated Pid
    bool link;                // link this property for linked elements
    const char* name;         // xml name of property
    P_TYPE type;              // associated P_TYPE
    PropertyGroup group;
    const char* userName;     // user-visible name of property
};

//
// always: propertyList[subtype].id == subtype
//
//

//keep this properties untranslatable for now until we put the same strings to all UI elements
#define DUMMY_QT_TR_NOOP(x, y) y
/* *INDENT-OFF* */
static constexpr PropertyMetaData propertyList[] = {
    { Pid::SUBTYPE,                 false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "subtype") },
    { Pid::SELECTED,                false, "selected",              P_TYPE::BOOL,               PropertyGroup::NONE,      DUMMY_QT_TR_NOOP("propertyName", "selected") },
    { Pid::GENERATED,               false, "generated",             P_TYPE::BOOL,               PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "generated") },
    { Pid::COLOR,                   false, "color",                 P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "color") },
    { Pid::VISIBLE,                 false, "visible",               P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "visible") },
    { Pid::Z,                       false, "z",                     P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "z") },
    { Pid::SMALL,                   false, "small",                 P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "small") },
    { Pid::SHOW_COURTESY,           false, "showCourtesySig",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "show courtesy") },
    { Pid::KEYSIG_MODE,             false, "keysig_mode",           P_TYPE::KEY_MODE,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "show courtesy") },
    { Pid::SLUR_STYLE_TYPE,         false, "lineType",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "line type") },
    { Pid::PITCH,                   true,  "pitch",                 P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "pitch") },

    { Pid::TPC1,                    true,  "tpc",                   P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "tonal pitch class") },
    { Pid::TPC2,                    true,  "tpc2",                  P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "tonal pitch class") },
    { Pid::LINE,                    false, "line",                  P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "line") },
    { Pid::FIXED,                   true,  "fixed",                 P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "fixed") },
    { Pid::FIXED_LINE,              true,  "fixedLine",             P_TYPE::INT,                PropertyGroup::NONE      ,      DUMMY_QT_TR_NOOP("propertyName", "fixed line") },
    { Pid::HEAD_TYPE,               false, "headType",              P_TYPE::NOTEHEAD_TYPE,      PropertyGroup::APPEARANCE,          DUMMY_QT_TR_NOOP("propertyName", "head type") },
    { Pid::HEAD_GROUP,              true,  "head",                   P_TYPE::NOTEHEAD_GROUP,     PropertyGroup::APPEARANCE,         DUMMY_QT_TR_NOOP("propertyName", "head") },
    { Pid::VELO_TYPE,               false, "veloType",              P_TYPE::VELO_TYPE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "velocity type") },
    { Pid::USER_VELOCITY,           false, "velocity",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "velocity") },
    { Pid::ARTICULATION_ANCHOR,     false, "anchor",                P_TYPE::INT,                PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "anchor") },

    { Pid::DIRECTION,               false, "direction",             P_TYPE::DIRECTION_V,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "direction") },
    { Pid::HORIZONTAL_DIRECTION,    false, "horizontalDirection",   P_TYPE::DIRECTION_H,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "horizontal direction") },
    { Pid::STEM_DIRECTION,          false, "StemDirection",         P_TYPE::DIRECTION_V,        PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "stem direction") },
    { Pid::NO_STEM,                 false, "noStem",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "no stem") },
    { Pid::SLUR_DIRECTION,          false, "up",                    P_TYPE::DIRECTION_V,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "up") },
    { Pid::LEADING_SPACE,           false, "leadingSpace",          P_TYPE::SPATIUM,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "leading space") },
    { Pid::MIRROR_HEAD,             false, "mirror",                P_TYPE::DIRECTION_H,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "mirror") },
    { Pid::HEAD_HAS_PARENTHESES,    true , "parentheses",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "parentheses") },
    { Pid::DOT_POSITION,            false, "dotPosition",           P_TYPE::DIRECTION_V,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "dot position") },
    { Pid::COMBINE_VOICE,           true,  "combineVoice",          P_TYPE::AUTO_ON_OFF,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "combine voice") },
    { Pid::TUNING,                  false, "tuning",                P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tuning") },
    { Pid::PAUSE,                   true,  "pause",                 P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "pause") },

    { Pid::BARLINE_TYPE,            false, "subtype",               P_TYPE::BARLINE_TYPE,       PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "subtype") },
    { Pid::BARLINE_SPAN,            false, "span",                  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "span") },
    { Pid::BARLINE_SPAN_FROM,       false, "spanFromOffset",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "span from") },
    { Pid::BARLINE_SPAN_TO,         false, "spanToOffset",          P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "span to") },
    { Pid::BARLINE_SHOW_TIPS,       false, "showTips",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "show tips") },

    { Pid::OFFSET,                  false, "offset",                P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "offset") },
    { Pid::FRET,                    true,  "fret",                  P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "fret") },
    { Pid::STRING,                  true,  "string",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "string") },
    { Pid::GHOST,                   true,  "ghost",                 P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ghost") },
    { Pid::DEAD,                    true,  "dead",                  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "dead") },
    { Pid::PLAY,                    false, "play",                  P_TYPE::BOOL,               PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "played") },
    { Pid::TIMESIG_NOMINAL,         false, 0,                       P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "nominal time signature") },
    { Pid::TIMESIG_ACTUAL,          true,  0,                       P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "actual time signature") },
    { Pid::NUMBER_TYPE,             false, "numberType",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "number type") },
    { Pid::BRACKET_TYPE,            false, "bracketType",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bracket type") },
    { Pid::NORMAL_NOTES,            false, "normalNotes",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "normal notes") },
    { Pid::ACTUAL_NOTES,            false, "actualNotes",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "actual notes") },
    { Pid::P1,                      false, "p1",                    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "p1") },
    { Pid::P2,                      false, "p2",                    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "p2") },
    { Pid::GROW_LEFT,               false, "growLeft",              P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "grow left") },
    { Pid::GROW_RIGHT,              false, "growRight",             P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "grow right") },

    { Pid::BOX_HEIGHT,              false, "height",                P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "height") },
    { Pid::BOX_WIDTH,               false, "width",                 P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "width") },
    { Pid::BOX_AUTOSIZE,            false, "boxAutoSize",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "autosize frame") },
    { Pid::TOP_GAP,                 false, "topGap",                P_TYPE::SPATIUM,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "top gap") },
    { Pid::BOTTOM_GAP,              false, "bottomGap",             P_TYPE::SPATIUM,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "bottom gap") },
    { Pid::LEFT_MARGIN,             false, "leftMargin",            P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "left padding") },
    { Pid::RIGHT_MARGIN,            false, "rightMargin",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "right padding") },
    { Pid::TOP_MARGIN,              false, "topMargin",             P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "top padding") },
    { Pid::BOTTOM_MARGIN,           false, "bottomMargin",          P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bottom padding") },
    { Pid::LAYOUT_BREAK,            false, "subtype",               P_TYPE::LAYOUTBREAK_TYPE,   PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "subtype") },
    { Pid::AUTOSCALE,               false, "autoScale",             P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "autoscale") },
    { Pid::SIZE,                    false, "size",                  P_TYPE::SIZE,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "size") },

    { Pid::IMAGE_HEIGHT,            false, "imageHeight",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "imageHeight") },
    { Pid::IMAGE_WIDTH,             false, "imageWidth",            P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "imageWidth") },
    { Pid::IMAGE_FRAMED,            false, "imageFramed",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "imageFramed") },

    { Pid::FRET_FRAME_TEXT_SCALE,               false, "fretFrameTextScale",            P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "text scale") },
    { Pid::FRET_FRAME_DIAGRAM_SCALE,            false, "fretFrameDiagramScale",         P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "diagram scale") },
    { Pid::FRET_FRAME_COLUMN_GAP,               false, "fretFrameColumnGap",            P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "column gap") },
    { Pid::FRET_FRAME_ROW_GAP,                  false, "fretFrameRowGap",               P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "row gap") },
    { Pid::FRET_FRAME_CHORDS_PER_ROW,           false, "fretFrameChordsPerRow",         P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "chords per row") },
    { Pid::FRET_FRAME_H_ALIGN,                  false, "fretFrameHorizontalAlign",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "horizontal alignment") },

    { Pid::SCALE,                   false, "scale",                 P_TYPE::SCALE,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "scale") },
    { Pid::LOCK_ASPECT_RATIO,       false, "lockAspectRatio",       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "aspect ratio locked") },
    { Pid::SIZE_IS_SPATIUM,         false, "sizeIsSpatium",         P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "size is spatium") },
    { Pid::TEXT,                    false,  "text",                 P_TYPE::STRING,             PropertyGroup::TEXT,            DUMMY_QT_TR_NOOP("propertyName", "text") },
    { Pid::HTML_TEXT,               false, 0,                       P_TYPE::STRING,             PropertyGroup::TEXT,            "" },
    { Pid::USER_MODIFIED,           false, 0,                       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      "" },
    { Pid::BEAM_POS,                false, 0,                       P_TYPE::PAIR_REAL,          PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "beam position") },
    { Pid::BEAM_MODE,               true, "BeamMode",               P_TYPE::BEAM_MODE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "beam mode") },
    { Pid::BEAM_NO_SLOPE,           true, "noSlope",                P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "without slope") },
    { Pid::BEAM_CROSS_STAFF_MOVE,   true, "crossStaffMove",         P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "beam staff move") },
    { Pid::USER_LEN,                false, "userLen",               P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "length") },
    { Pid::SHOW_STEM_SLASH,         true,  "showStemSlash",         P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "show stem slash") },

    { Pid::SPACE,                   false, "space",                 P_TYPE::SPATIUM,         PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "space") },
    { Pid::TEMPO,                   true,  "tempo",                 P_TYPE::TEMPO,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tempo") },
    { Pid::TEMPO_FOLLOW_TEXT,       true,  "followText",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "following text") },
    { Pid::TEMPO_ALIGN_RIGHT_OF_REHEARSAL_MARK, false, "tempoAlignRightOfRehearsalMark", P_TYPE::BOOL, PropertyGroup::APPEARANCE, DUMMY_QT_TR_NOOP("propertyName", "tempo align right of rehearsal mark") },
    { Pid::ACCIDENTAL_BRACKET,      false, "bracket",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bracket") },
    { Pid::ACCIDENTAL_TYPE,         true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "type") },
    { Pid::ACCIDENTAL_STACKING_ORDER_OFFSET, true, "stackingOrderOffset", P_TYPE::INT,          PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "stacking order offset") },
    { Pid::NUMERATOR_STRING,        false, "textN",                 P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "numerator string") },
    { Pid::DENOMINATOR_STRING,      false, "textD",                 P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "denominator string") },
    { Pid::FBPREFIX,                false, "prefix",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "prefix") },
    { Pid::FBDIGIT,                 false, "digit",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "digit") },
    { Pid::FBSUFFIX,                false, "suffix",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "suffix") },
    { Pid::FBCONTINUATIONLINE,      false, "continuationLine",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continuation line") },

    { Pid::FBPARENTHESIS1,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS2,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS3,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS4,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS5,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },

    { Pid::OTTAVA_TYPE,             true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ottava type") },
    { Pid::NUMBERS_ONLY,            false, "numbersOnly",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "numbers only") },
    { Pid::TRILL_TYPE,              false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "trill type") },
    { Pid::VIBRATO_TYPE,            false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "vibrato type") },
    { Pid::HAIRPIN_CIRCLEDTIP,      false, "hairpinCircledTip",     P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "hairpin with circled tip") },

    { Pid::HAIRPIN_TYPE,            true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "hairpin type") },
    { Pid::HAIRPIN_HEIGHT,          false, "hairpinHeight",         P_TYPE::SPATIUM,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "hairpin height") },
    { Pid::HAIRPIN_CONT_HEIGHT,     false, "hairpinContHeight",     P_TYPE::SPATIUM,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "hairpin cont height") },
    { Pid::VELO_CHANGE,             true,  "veloChange",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "velocity change") },
    { Pid::VELO_CHANGE_METHOD,      true,  "veloChangeMethod",      P_TYPE::CHANGE_METHOD,      PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "velocity change method") }, // left as a compatibility property - we need to be able to read it correctly
    { Pid::VELO_CHANGE_SPEED,       true,  "veloChangeSpeed",       P_TYPE::DYNAMIC_SPEED,      PropertyGroup::APPEARANCE,       DUMMY_QT_TR_NOOP("propertyName", "velocity change speed") },
    { Pid::DYNAMIC_TYPE,            true,  "subtype",               P_TYPE::DYNAMIC_TYPE,       PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "dynamic type") },

    { Pid::SINGLE_NOTE_DYNAMICS,    true,  "singleNoteDynamics",    P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "single note dynamics") },
    { Pid::CHANGE_METHOD,           true,  "changeMethod",          P_TYPE::CHANGE_METHOD,      PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "change method") }, // the new, more general version of VELO_CHANGE_METHOD
    { Pid::PLACEMENT,               false, "placement",             P_TYPE::PLACEMENT_V,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "placement") },
    { Pid::HPLACEMENT,              false, "hplacement",            P_TYPE::PLACEMENT_H,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "horizontal placement") },
    { Pid::MMREST_RANGE_BRACKET_TYPE, false, "mmrestRangeBracketType", P_TYPE::INT,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "multimeasure rest range bracket type") },
    { Pid::VELOCITY,                false, "velocity",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "velocity") },
    { Pid::JUMP_TO,                 true,  "jumpTo",                P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "jump to") },
    { Pid::PLAY_UNTIL,              true,  "playUntil",             P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "play until") },
    { Pid::CONTINUE_AT,             true,  "continueAt",            P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue at") },
    { Pid::LABEL,                   true,  "label",                 P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "label") },
    { Pid::MARKER_TYPE,             true,  0,                       P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "marker type") },
    { Pid::MARKER_SYMBOL_SIZE,      true,  "markerSymbolSize",      P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "marker symbol size") },
    { Pid::MARKER_CENTER_ON_SYMBOL, true,  "markerCenterOnSymbol",  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "marker center on symbol") },
    { Pid::ARP_USER_LEN1,           false, 0,                       P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "length 1") },
    { Pid::ARP_USER_LEN2,           false, 0,                       P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "length 2") },
    { Pid::REPEAT_END,              true,  0,                       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      "" },
    { Pid::REPEAT_START,            true,  0,                       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      "" },
    { Pid::REPEAT_JUMP,             true,  0,                       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      "" },
    { Pid::MEASURE_NUMBER_MODE,     false, "measureNumberMode",     P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "measure number mode") },

    { Pid::GLISS_TYPE,              false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "subtype") },
    { Pid::GLISS_TEXT,              false, "text",                  P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "text") },
    { Pid::GLISS_SHOW_TEXT,         false, "glissandoShowText",     P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "showing text") },
    { Pid::GLISS_STYLE,             true,  "glissandoStyle",        P_TYPE::GLISS_STYLE,        PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "glissando style") },
    { Pid::GLISS_SHIFT,             false, "glissandoShift",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "glissando shift") },
    { Pid::GLISS_EASEIN,            false, "easeInSpin",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName","ease in") },
    { Pid::GLISS_EASEOUT,           false, "easeOutSpin",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ease out") },
    { Pid::DIAGONAL,                false, 0,                       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "diagonal") },
    { Pid::GROUP_NODES,             false, 0,                       P_TYPE::GROUPS,             PropertyGroup::NONE      ,      DUMMY_QT_TR_NOOP("propertyName", "groups") },
    { Pid::LINE_STYLE,              true,  "lineStyle",             P_TYPE::LINE_TYPE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "line style") },
    { Pid::LINE_WIDTH,              false, "lineWidth",             P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "line width") },
    { Pid::TIME_STRETCH,            true,  "timeStretch",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "time stretch") },
    { Pid::ORNAMENT_STYLE,          true,  "ornamentStyle",         P_TYPE::ORNAMENT_STYLE,     PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ornament style") },
    { Pid::INTERVAL_ABOVE,          true,  "intervalAbove",         P_TYPE::ORNAMENT_INTERVAL,  PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "interval above") },
    { Pid::INTERVAL_BELOW,          true,  "intervalBelow",         P_TYPE::ORNAMENT_INTERVAL,  PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "interval below") },
    { Pid::ORNAMENT_SHOW_ACCIDENTAL,true,  "ornamentShowAccidental",P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ornament show accidental") },
    { Pid::ORNAMENT_SHOW_CUE_NOTE,  true,  "ornamentShowCueNote",   P_TYPE::AUTO_ON_OFF,        PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ornament show cue note") },
    { Pid::START_ON_UPPER_NOTE,     true,  "startOnUpperNote",      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "start on upper note") },

    { Pid::TIMESIG,                 false, "timesig",               P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "time signature") },
    { Pid::TIMESIG_STRETCH,         false, 0,                       P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "time signature stretch") },
    { Pid::TIMESIG_TYPE,            true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "subtype") },
    { Pid::SPANNER_TICK,            true,  "tick",                  P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tick") },
    { Pid::SPANNER_TICKS,           true,  "ticks",                 P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ticks") },
    { Pid::SPANNER_TRACK2,          false, "track2",                P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "track2") },
    { Pid::OFFSET2,                 false, "userOff2",              P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "offset2") },
    { Pid::BREAK_MMR,               false, "breakMultiMeasureRest", P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "breaking multimeasure rest") },
    { Pid::MMREST_NUMBER_POS,       false, "mmRestNumberPos",       P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "vertical position of multimeasure rest number") }, // Deprecated
    { Pid::MMREST_NUMBER_OFFSET,    false, "mmRestNumberOffset",    P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "vertical offset of multimeasure rest number") },
    { Pid::MMREST_NUMBER_VISIBLE,   false, "mmRestNumberVisible",   P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "visibility of multimeasure rest number") },

    { Pid::MEASURE_REPEAT_NUMBER_POS, false, "measureRepeatNumberPos", P_TYPE::SPATIUM,         PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "vertical position of measure repeat number") },
    { Pid::REPEAT_COUNT,            true,  "endRepeat",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end repeat") },

    { Pid::USER_STRETCH,            false, "stretch",               P_TYPE::REAL,               PropertyGroup::NONE      ,      DUMMY_QT_TR_NOOP("propertyName", "stretch") },
    { Pid::NO_OFFSET,               true,  "noOffset",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "numbering offset") },
    { Pid::IRREGULAR,               true,  "irregular",             P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "irregular") },
    { Pid::ANCHOR,                  false, "anchor",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "anchor") },
    { Pid::SLUR_UOFF1,              false, "o1",                    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "o1") },
    { Pid::SLUR_UOFF2,              false, "o2",                    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "o2") },
    { Pid::SLUR_UOFF3,              false, "o3",                    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "o3") },
    { Pid::SLUR_UOFF4,              false, "o4",                    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "o4") },
    { Pid::STAFF_MOVE,              true,  "staffMove",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "staff move") },
    { Pid::VERSE,                   true,  "no",                    P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "verse") },

    { Pid::SYLLABIC,                true,  "syllabic",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "syllabic") },
    { Pid::LYRIC_TICKS,             true,  "ticks_f",               P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ticks") },
    { Pid::VOLTA_ENDING,            true,  "endings",               P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "endings") },
    { Pid::LINE_VISIBLE,            true,  "lineVisible",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "visible line") },
    { Pid::MAG,                     false, "mag",                   P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "mag") },
    { Pid::USE_DRUMSET,             false, "useDrumset",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "using drumset") },
    { Pid::DURATION,                true,  0,                       P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "duration") },
    { Pid::DURATION_TYPE_WITH_DOTS, true,  0,                       P_TYPE::DURATION_TYPE_WITH_DOTS, PropertyGroup::APPEARANCE, DUMMY_QT_TR_NOOP("propertyName", "duration type") },
    { Pid::ACCIDENTAL_ROLE,         false, "role",                  P_TYPE::ACCIDENTAL_ROLE,    PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "role") },
    { Pid::TRACK,                   false, 0,                       P_TYPE::SIZE_T,             PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "track") },

    { Pid::FRET_STRINGS,            true,  "strings",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "strings") },
    { Pid::FRET_FRETS,              true,  "frets",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frets") },
    { Pid::FRET_NUT,                true,  "showNut",               P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "show nut") },
    { Pid::FRET_OFFSET,             true,  "fretOffset",            P_TYPE::INT,                PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "fret offset") },
    { Pid::FRET_NUM_POS,            true,  "fretNumPos",            P_TYPE::INT,                PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "fret number position") },
    { Pid::ORIENTATION,             true,  "orientation",           P_TYPE::ORIENTATION,        PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "orientation") },
    { Pid::FRET_SHOW_FINGERINGS,    true,  "fretShowFingering",     P_TYPE::BOOL       ,        PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "fretShowFingering") },
    { Pid::FRET_FINGERING,          true,  "fretFingering",         P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "fretFingering") },

    { Pid::HARMONY_VOICE_LITERAL,   true,  "harmonyVoiceLiteral",   P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "harmony voice literal") },
    { Pid::HARMONY_VOICING,         true,  "harmonyVoicing",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "harmony voicing") },
    { Pid::HARMONY_DURATION,        true,  "harmonyDuration",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "harmony duration") },
    { Pid::HARMONY_BASS_SCALE,      true,  "harmonyBassScale",      P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "harmony bass scale") },

    { Pid::SYSTEM_BRACKET,          false, "type",                  P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "type") },
    { Pid::GAP,                     false, 0,                       P_TYPE::BOOL,               PropertyGroup::NONE,      DUMMY_QT_TR_NOOP("propertyName", "gap") },
    { Pid::AUTOPLACE,               false, "autoplace",             P_TYPE::BOOL,               PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "autoplace") },
    { Pid::DASH_LINE_LEN,           false, "dashLineLength",        P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "dash line length") },
    { Pid::DASH_GAP_LEN,            false, "dashGapLength",         P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "dash gap length") },
    { Pid::TICK,                    false, 0,                       P_TYPE::FRACTION,           PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "tick") },
    { Pid::PLAYBACK_VOICE1,         false, "playbackVoice1",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "playback voice 1") },
    { Pid::PLAYBACK_VOICE2,         false, "playbackVoice2",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "playback voice 2") },
    { Pid::PLAYBACK_VOICE3,         false, "playbackVoice3",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "playback voice 3") },

    { Pid::PLAYBACK_VOICE4,         false, "playbackVoice4",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "playback voice 4") },
    { Pid::SYMBOL,                  true,  "symbol",                P_TYPE::SYMID,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "symbol") },
    { Pid::PLAY_REPEATS,            true,  "playRepeats",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "playing repeats") },
    { Pid::CREATE_SYSTEM_HEADER,    false, "createSystemHeader",    P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "creating system header") },
    { Pid::STAFF_LINES,             true,  "lines",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "lines") },
    { Pid::LINE_DISTANCE,           true,  "lineDistance",          P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "line distance") },
    { Pid::STEP_OFFSET,             true,  "stepOffset",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "step offset") },
    { Pid::STAFF_SHOW_BARLINES,     false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "showing barlines") },
    { Pid::STAFF_SHOW_LEDGERLINES,  false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "showing ledgerlines") },
    { Pid::STAFF_STEMLESS,          false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "stemless") },
    { Pid::STAFF_INVISIBLE,         false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "invisible") },
    { Pid::STAFF_COLOR,             false, "color",                 P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "color") },

    { Pid::HEAD_SCHEME,             false, "headScheme",            P_TYPE::NOTEHEAD_SCHEME,    PropertyGroup::APPEARANCE,          DUMMY_QT_TR_NOOP("propertyName", "notehead scheme") },
    { Pid::STAFF_GEN_CLEF,          false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "generating clefs") },
    { Pid::STAFF_GEN_TIMESIG,       false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "generating time signature") },
    { Pid::STAFF_GEN_KEYSIG,        false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "generating key signature") },
    { Pid::STAFF_YOFFSET,           false, "",                      P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "y-offset") },
    { Pid::STAFF_USERDIST,          false, "distOffset",            P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "distance offset") },
    { Pid::STAFF_BARLINE_SPAN,      false, "barLineSpan",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "barline span") },
    { Pid::STAFF_BARLINE_SPAN_FROM, false, "barLineSpanFrom",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "barline span from") },
    { Pid::STAFF_BARLINE_SPAN_TO,   false, "barLineSpanTo",         P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "barline span to") },
    { Pid::BRACKET_SPAN,            false, "bracketSpan",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bracket span") },

    { Pid::BRACKET_COLUMN,          false, "level",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "level") },
    { Pid::INAME_LAYOUT_POSITION,   false, "layoutPosition",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "layout position") },
    { Pid::TEXT_STYLE,              false, "style",                 P_TYPE::TEXT_STYLE,         PropertyGroup::TEXT,            DUMMY_QT_TR_NOOP("propertyName", "style") },
    { Pid::FONT_FACE,               false, "family",                P_TYPE::STRING,             PropertyGroup::TEXT,            DUMMY_QT_TR_NOOP("propertyName", "family") },
    { Pid::FONT_SIZE,               false, "size",                  P_TYPE::REAL,               PropertyGroup::TEXT,            DUMMY_QT_TR_NOOP("propertyName", "size") },
    { Pid::FONT_STYLE,              false, "fontStyle",             P_TYPE::INT,                PropertyGroup::TEXT,            DUMMY_QT_TR_NOOP("propertyName", "font style") },
    { Pid::TEXT_LINE_SPACING,       false, "textLineSpacing",       P_TYPE::REAL,               PropertyGroup::TEXT,            DUMMY_QT_TR_NOOP("propertyName", "user line distancing") },

    { Pid::FRAME_TYPE,              false, "frameType",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frame type") },
    { Pid::FRAME_WIDTH,             false, "frameWidth",            P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frame width") },
    { Pid::FRAME_PADDING,           false, "framePadding",          P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frame padding") },
    { Pid::FRAME_ROUND,             false, "frameRound",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frame round") },
    { Pid::FRAME_FG_COLOR,          false, "frameFgColor",          P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frame foreground color") },
    { Pid::FRAME_BG_COLOR,          false, "frameBgColor",          P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "frame background color") },
    { Pid::SIZE_SPATIUM_DEPENDENT,  false, "sizeIsSpatiumDependent",P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "spatium dependent size") },
    { Pid::TEXT_SIZE_SPATIUM_DEPENDENT, false, "textSizeIsSpatiumDependent", P_TYPE::BOOL,      PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "spatium dependent text size") },
    { Pid::MUSICAL_SYMBOLS_SCALE,   false, "musicalSymbolsScale",   P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "musical symbols scale") },
    { Pid::ALIGN,                   false, "align",                 P_TYPE::ALIGN,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "align") },
    { Pid::TEXT_SCRIPT_ALIGN,       false, "align",                 P_TYPE::INT,                PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "text script align") },
    { Pid::SYSTEM_FLAG,             false, "systemFlag",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "system flag") },

    { Pid::BEGIN_TEXT,              true,  "beginText",             P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin text") },
    { Pid::BEGIN_TEXT_ALIGN,        false, "beginTextAlign",        P_TYPE::ALIGN,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin text align") },
    { Pid::BEGIN_TEXT_PLACE,        false, "beginTextPlace",        P_TYPE::TEXT_PLACE,         PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin text place") },
    { Pid::BEGIN_HOOK_TYPE,         true,  "beginHookType",         P_TYPE::HOOK_TYPE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin hook type") },
    { Pid::BEGIN_HOOK_HEIGHT,       false, "beginHookHeight",       P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin hook height") },
    { Pid::BEGIN_FONT_FACE,         false, "beginFontFace",         P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin font face") },
    { Pid::BEGIN_FONT_SIZE,         false, "beginFontSize",         P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin font size") },
    { Pid::BEGIN_FONT_STYLE,        false, "beginFontStyle",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "begin font style") },
    { Pid::BEGIN_TEXT_OFFSET,       false, "beginTextOffset",       P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "begin text offset") },
    { Pid::GAP_BETWEEN_TEXT_AND_LINE, false, "gapBetweenTextAndLine", P_TYPE::SPATIUM,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "gap between text and line") },

    { Pid::CONTINUE_TEXT,           true,  "continueText",          P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue text") },
    { Pid::CONTINUE_TEXT_ALIGN,     false, "continueTextAlign",     P_TYPE::ALIGN,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue text align") },
    { Pid::CONTINUE_TEXT_PLACE,     false, "continueTextPlace",     P_TYPE::TEXT_PLACE,         PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue text place") },
    { Pid::CONTINUE_FONT_FACE,      false, "continueFontFace",      P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue font face") },
    { Pid::CONTINUE_FONT_SIZE,      false, "continueFontSize",      P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue font size") },
    { Pid::CONTINUE_FONT_STYLE,     false, "continueFontStyle",     P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "continue font style") },
    { Pid::CONTINUE_TEXT_OFFSET,    false, "continueTextOffset",    P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "continue text offset") },

    { Pid::END_TEXT,                true,  "endText",               P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end text") },
    { Pid::END_TEXT_ALIGN,          false, "endTextAlign",          P_TYPE::ALIGN,              PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end text align") },
    { Pid::END_TEXT_PLACE,          false, "endTextPlace",          P_TYPE::TEXT_PLACE,         PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end text place") },
    { Pid::END_HOOK_TYPE,           true,  "endHookType",           P_TYPE::HOOK_TYPE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end hook type") },
    { Pid::END_HOOK_HEIGHT,         false, "endHookHeight",         P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end hook height") },
    { Pid::END_FONT_FACE,           false, "endFontFace",           P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end font face") },
    { Pid::END_FONT_SIZE,           false, "endFontSize",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end font size") },
    { Pid::END_FONT_STYLE,          false, "endFontStyle",          P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "end font style") },
    { Pid::END_TEXT_OFFSET,         false, "endTextOffset",         P_TYPE::POINT,              PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "end text offset") },

    { Pid::NOTELINE_PLACEMENT,      false, "noteLinePlacement",     P_TYPE::NOTELINE_PLACEMENT_TYPE,    PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "note-anchored line placement") },

    { Pid::AVOID_BARLINES,          false, "avoidBarLines",         P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "avoid barlines") },
    { Pid::DYNAMICS_SIZE,           false, "dynamicsSize",          P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "dynamic size") },
    { Pid::CENTER_ON_NOTEHEAD,      false, "centerOnNotehead",      P_TYPE::BOOL,               PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "use text alignment") },
    { Pid::ANCHOR_TO_END_OF_PREVIOUS, true, "anchorToEndOfPrevious", P_TYPE::BOOL,              PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "anchor to end of previous") },

    { Pid::SNAP_TO_DYNAMICS,         false, "snapToDynamics",       P_TYPE::BOOL,               PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "snap expression") }, // for expressions
    { Pid::SNAP_BEFORE,              false, "snapBefore",           P_TYPE::BOOL,               PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "snap before") },     // <
    { Pid::SNAP_AFTER,               false, "snapAfter",            P_TYPE::BOOL,               PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "snap after") },      // < for hairpins

    { Pid::VOICE_ASSIGNMENT,        true,  "voiceAssignment",       P_TYPE::VOICE_ASSIGNMENT,   PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "voice assignment") },
    { Pid::CENTER_BETWEEN_STAVES,   false, "centerBetweenStaves",   P_TYPE::AUTO_ON_OFF,        PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "center between staves") },

    { Pid::POS_ABOVE,               false, "posAbove",              P_TYPE::MILLIMETRE,         PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "position above") },

    { Pid::LOCATION_STAVES,         false, "staves",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "staves distance") },
    { Pid::LOCATION_VOICES,         false, "voices",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "voices distance") },
    { Pid::LOCATION_MEASURES,       false, "measures",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "measures distance") },
    { Pid::LOCATION_FRACTIONS,      false, "fractions",             P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "position distance") },
    { Pid::LOCATION_GRACE,          false, "grace",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "grace note index") },
    { Pid::LOCATION_NOTE,           false, "note",                  P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "note index") },

    { Pid::VOICE,                   true,  "voice",                 P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "voice") },
    { Pid::POSITION,                false, "position",              P_TYPE::ALIGN_H,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "position") },

    { Pid::CLEF_TYPE_CONCERT,       true,  "concertClefType",       P_TYPE::CLEF_TYPE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "concert clef type") },
    { Pid::CLEF_TYPE_TRANSPOSING,   true,  "transposingClefType",   P_TYPE::CLEF_TYPE,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "transposing clef type") },
    { Pid::CLEF_TO_BARLINE_POS,     true,  "clefToBarlinePos",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "clef to barline position") },
    { Pid::IS_HEADER,               true,  "isHeader",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "is header")},
    { Pid::KEY_CONCERT,             true,  "concertKey",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "concert key") },
    { Pid::KEY,                     true,  "actualKey",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "key") },
    { Pid::ACTION,                  false, "action",                P_TYPE::STRING,             PropertyGroup::APPEARANCE,      0 },
    { Pid::MIN_DISTANCE,            false, "minDistance",           P_TYPE::SPATIUM,            PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "autoplace minimum distance") },

    { Pid::ARPEGGIO_TYPE,           true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "arpeggio type") },
    { Pid::CHORD_LINE_TYPE,         true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "chord line type") },
    { Pid::CHORD_LINE_STRAIGHT,     true,  "straight",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "straight chord line") },
    { Pid::CHORD_LINE_WAVY,         true,  "wavy",                  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "wavy chord line") },
    { Pid::TREMOLO_TYPE,            true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tremolo type") },
    { Pid::TREMOLO_STYLE,           true,  "strokeStyle",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tremolo style") },
    { Pid::HARMONY_TYPE,            true,  "harmonyType",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "harmony type") },

    { Pid::ARPEGGIO_SPAN,           true,  "arpeggioSpan",          P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "arpeggio span") },

    { Pid::BEND_TYPE,               true,  "bendType",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bend type") },
    { Pid::BEND_CURVE,              true,  "bendCurve",             P_TYPE::PITCH_VALUES,       PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bend curve") },
    { Pid::BEND_VERTEX_OFF,         false, "bendVertexOffset",      P_TYPE::POINT,              PropertyGroup::POSITION  ,      DUMMY_QT_TR_NOOP("propertyName", "bend vertex offset") },
    { Pid::BEND_SHOW_HOLD_LINE,     false, "bendShowHoldLine",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bend show hold line") },
    { Pid::BEND_START_TIME_FACTOR,  true,  "bendStartTimeFactor",   P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bend start time factor") },
    { Pid::BEND_END_TIME_FACTOR,    true,  "bendEndTimeFactor",     P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "bend end time factor") },

    { Pid::TREMOLOBAR_TYPE,         true,  "tremoloBarType",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tremolobar type") },
    { Pid::TREMOLOBAR_CURVE,        true,  "tremoloBarCurve",       P_TYPE::PITCH_VALUES,       PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tremolobar curve") },

    { Pid::START_WITH_LONG_NAMES,   false, "startWithLongNames",    P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "start with long names") },
    { Pid::START_WITH_MEASURE_ONE,  true,  "startWithMeasureOne",   P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "start with measure one") },
    { Pid::FIRST_SYSTEM_INDENTATION,true,  "firstSystemIndentation",P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "first system indentation") },

    { Pid::PATH,                    false, "path",                  P_TYPE::DRAW_PATH,          PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "path") },

    { Pid::PREFER_SHARP_FLAT,       true,  "preferSharpFlat",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "prefer sharps or flats") },

    { Pid::PLAY_TECH_TYPE,          true,  "playTechType",          P_TYPE::PLAYTECH_TYPE,      PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "playing technique type") },

    { Pid::TEMPO_CHANGE_TYPE,       true,  "tempoChangeType",       P_TYPE::TEMPOCHANGE_TYPE,   PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "gradual tempo change type") },
    { Pid::TEMPO_EASING_METHOD,     true,  "tempoEasingMethod",     P_TYPE::CHANGE_METHOD,      PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tempo easing method") },
    { Pid::TEMPO_CHANGE_FACTOR,     true,  "tempoChangeFactor",     P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tempo change factor") },

    { Pid::HARP_IS_DIAGRAM,         false,  "isDiagram",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "is diagram") },

    { Pid::ACTIVE,                  true,  "active",                P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "active") },

    { Pid::CAPO_FRET_POSITION,      true,  "fretPosition",          P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "fret position") },
    { Pid::CAPO_IGNORED_STRINGS,    true,  "ignoredStrings",        P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "ignored strings") },
    { Pid::CAPO_GENERATE_TEXT,      true,  "generateText",          P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "automatically generate text") },

    { Pid::TIE_PLACEMENT,           true,  "tiePlacement",          P_TYPE::TIE_PLACEMENT,      PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "tie placement") },
    { Pid::MIN_LENGTH,              true,  "minLength",             P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "minimum length") },
    { Pid::PARTIAL_SPANNER_DIRECTION,   true,  "partialSpannerDirection",   P_TYPE::PARTIAL_SPANNER_DIRECTION,  PropertyGroup::NONE,  DUMMY_QT_TR_NOOP("propertyName", "partial spanner direction") },

    { Pid::POSITION_LINKED_TO_MASTER,   false, "positionLinkedToMaster",   P_TYPE::BOOL,        PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "position linked to master") },
    { Pid::APPEARANCE_LINKED_TO_MASTER, false, "appearanceLinkedToMaster", P_TYPE::BOOL,        PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "appearance linked to master") },
    { Pid::TEXT_LINKED_TO_MASTER,       false, "textLinkedToMaster",       P_TYPE::BOOL,        PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "text linked to master") },
    { Pid::EXCLUDE_FROM_OTHER_PARTS,    false, "excludeFromParts",         P_TYPE::BOOL,        PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "exclude from parts") },

    { Pid::STRINGTUNINGS_STRINGS_COUNT, true,  "stringsCount",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "strings count") },
    { Pid::STRINGTUNINGS_PRESET,    true,  "preset",                P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "strings preset") },
    { Pid::STRINGTUNINGS_VISIBLE_STRINGS,   true,  "visibleStrings",P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "visible strings") },

    { Pid::SCORE_FONT,              true,  "scoreFont",             P_TYPE::STRING,             PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "score font") },
    { Pid::SYMBOLS_SIZE,            false, "symbolsSize",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "symbols size") },
    { Pid::SYMBOL_ANGLE,            false, "symbolAngle",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      DUMMY_QT_TR_NOOP("propertyName", "symbol angle") },

    { Pid::APPLY_TO_ALL_STAVES,     false, "applyToAllStaves",      P_TYPE::BOOL,               PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "apply to all staves") },
    { Pid::IS_COURTESY,             false, "isCourtesy",            P_TYPE::BOOL,               PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "is courtesy") },
    { Pid::EXCLUDE_VERTICAL_ALIGN,  false, "excludeVerticalAlign",  P_TYPE::BOOL,               PropertyGroup::POSITION,        DUMMY_QT_TR_NOOP("propertyName", "exclude vertical align") },

    { Pid::END,                     false, "++end++",               P_TYPE::INT,                PropertyGroup::NONE,            DUMMY_QT_TR_NOOP("propertyName", "<invalid property>") }
};
/* *INDENT-ON* */

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid propertyId(const AsciiStringView& s)
{
    for (const PropertyMetaData& pd : propertyList) {
        if (s == pd.name) {
            return pd.id;
        }
    }
    return Pid::END;
}

//---------------------------------------------------------
//   propertyType
//---------------------------------------------------------

P_TYPE propertyType(Pid id)
{
    assert(propertyList[int(id)].id == id);
    return propertyList[int(id)].type;
}

PropertyGroup propertyGroup(Pid id)
{
    assert(propertyList[int(id)].id == id);
    return propertyList[int(id)].group;
}

//---------------------------------------------------------
//   propertyLink
//---------------------------------------------------------

bool propertyLink(Pid id)
{
    assert(propertyList[int(id)].id == id);
    return propertyList[int(id)].link;
}

//---------------------------------------------------------
//   propertyLinkSameScore
//---------------------------------------------------------

bool propertyLinkSameScore(Pid id)
{
    assert(id < Pid::END);
    switch (id) {
    case Pid::STAFF_BARLINE_SPAN:
    case Pid::STAFF_BARLINE_SPAN_FROM:
    case Pid::STAFF_BARLINE_SPAN_TO:
        return false;
    default:
        return true;
    }
}

//---------------------------------------------------------
//   propertyName
//---------------------------------------------------------

const char* propertyName(Pid id)
{
    assert(propertyList[int(id)].id == id);
    return propertyList[int(id)].name;
}

//---------------------------------------------------------
//   propertyUserName
//---------------------------------------------------------

String propertyUserName(Pid id)
{
    assert(propertyList[int(id)].id == id);
    return muse::mtrc("engraving", propertyList[int(id)].userName, "propertyName");
}

//---------------------------------------------------------
//    propertyFromString
//---------------------------------------------------------

PropertyValue propertyFromString(P_TYPE type, String)
{
    switch (type) {
    case P_TYPE::BEAM_MODE:
        return PropertyValue(int(0));
    case P_TYPE::GROUPS:
        // unsupported
        return PropertyValue();
    case P_TYPE::DURATION_TYPE_WITH_DOTS:
    case P_TYPE::INT_VEC:
        return PropertyValue();
    default:
        break;
    }
    return PropertyValue();
}

//---------------------------------------------------------
//   propertyToString
//    Originally extracted from XmlWriter
//---------------------------------------------------------

String propertyToString(Pid id, const PropertyValue& value, bool mscx)
{
    if (!value.isValid()) {
        return String();
    }

    switch (id) {
    case Pid::SYSTEM_BRACKET:         // system bracket type
        return String::fromAscii(TConv::toXml(BracketType(value.toInt())).ascii());
    case Pid::ACCIDENTAL_TYPE:
        return String::fromAscii(Accidental::subtype2name(AccidentalType(value.toInt())).ascii());
    case Pid::OTTAVA_TYPE:
        return String::fromAscii(Ottava::ottavaTypeName(OttavaType(value.toInt())));
    case Pid::TREMOLO_TYPE:
        return String::fromAscii(TConv::toXml(TremoloType(value.toInt())).ascii());
    case Pid::TRILL_TYPE:
        return String::fromAscii(TConv::toXml(TrillType(value.toInt())).ascii());
    case Pid::VIBRATO_TYPE:
        return String::fromAscii(TConv::toXml(VibratoType(value.toInt())).ascii());
    default:
        break;
    }

    switch (propertyType(id)) {
    case P_TYPE::DURATION_TYPE_WITH_DOTS:
        ASSERT_X("unknown: TDURATION");
        break;
    case P_TYPE::TEMPO:
        ASSERT_X("unknown: TEMPO");
        break;
    case P_TYPE::GROUPS:
        ASSERT_X("unknown: GROUPS");
        break;
    default: {
        break;
    }
    }

    if (!mscx) {
        // String representation for properties that are written
        // to MSCX in other way (e.g. as XML tag properties).
        switch (value.type()) {
        case P_TYPE::POINT: {
            const PointF p(value.value<PointF>());
            return String(u"%1;%2").arg(String::number(p.x()), String::number(p.y()));
        }
        case P_TYPE::SIZE: {
            const SizeF s(value.value<SizeF>());
            return String(u"%1x%2").arg(String::number(s.width()), String::number(s.height()));
        }
        case P_TYPE::STRING: {
            return value.value<String>();
        }
        // TODO: support QVariant::Rect and QVariant::RectF?
        default:
            break;
        }
    }

    return String();
}
}
