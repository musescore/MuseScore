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

/* *INDENT-OFF* */
static constexpr PropertyMetaData propertyList[] = {
    { Pid::SUBTYPE,                 false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "subtype") },
    { Pid::SELECTED,                false, "selected",              P_TYPE::BOOL,               PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "selected") },
    { Pid::GENERATED,               false, "generated",             P_TYPE::BOOL,               PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "generated") },
    { Pid::COLOR,                   false, "color",                 P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "color") },
    { Pid::VISIBLE,                 false, "visible",               P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "visible") },
    { Pid::Z,                       false, "z",                     P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "stacking order") },
    { Pid::SMALL,                   false, "small",                 P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "small") },
    { Pid::HIDE_WHEN_EMPTY,         false, "hideWhenEmpty",         P_TYPE::AUTO_ON_OFF,        PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "hide when empty") },
    { Pid::HIDE_STAVES_WHEN_INDIVIDUALLY_EMPTY, false, "hideStavesWhenIndividuallyEmpty", P_TYPE::BOOL, PropertyGroup::APPEARANCE, QT_TRANSLATE_NOOP("engraving/propertyName", "hide staves when individually empty") },
    { Pid::SHOW_IF_ENTIRE_SYSTEM_EMPTY, false, "showIfEntireSystemEmpty", P_TYPE::BOOL,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "show if entire system empty") },
    { Pid::SHOW_COURTESY,           false, "showCourtesySig",       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "show courtesy") },
    { Pid::KEYSIG_MODE,             false, "keysig_mode",           P_TYPE::KEY_MODE,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "key signature mode") },
    { Pid::SLUR_STYLE_TYPE,         false, "lineType",              P_TYPE::SLUR_STYLE_TYPE,    PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "line type") },
    { Pid::PITCH,                   true,  "pitch",                 P_TYPE::INT,                PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "pitch") },

    { Pid::TPC1,                    true,  "tpc",                   P_TYPE::INT,                PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "tonal pitch class") },
    { Pid::TPC2,                    true,  "tpc2",                  P_TYPE::INT,                PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "transposed tonal pitch class") },
    { Pid::LINE,                    false, "line",                  P_TYPE::INT,                PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "line") },
    { Pid::FIXED,                   true,  "fixed",                 P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "fixed") },
    { Pid::FIXED_LINE,              true,  "fixedLine",             P_TYPE::INT,                PropertyGroup::NONE      ,      QT_TRANSLATE_NOOP("engraving/propertyName", "fixed line") },
    { Pid::HEAD_TYPE,               false, "headType",              P_TYPE::NOTEHEAD_TYPE,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "head type") },
    { Pid::HEAD_GROUP,              true,  "head",                  P_TYPE::NOTEHEAD_GROUP,     PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "head") },
    { Pid::VELO_TYPE,               false, "veloType",              P_TYPE::VELO_TYPE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity type") },
    { Pid::USER_VELOCITY,           false, "velocity",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity") },
    { Pid::ARTICULATION_ANCHOR,     false, "anchor",                P_TYPE::INT,                PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "anchor") },

    { Pid::DIRECTION,               false, "direction",             P_TYPE::DIRECTION_V,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "direction") },
    { Pid::HORIZONTAL_DIRECTION,    false, "horizontalDirection",   P_TYPE::DIRECTION_H,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "horizontal direction") },
    { Pid::STEM_DIRECTION,          false, "StemDirection",         P_TYPE::DIRECTION_V,        PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "stem direction") },
    { Pid::NO_STEM,                 false, "noStem",                P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "stemless") },
    { Pid::SLUR_DIRECTION,          false, "up",                    P_TYPE::DIRECTION_V,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "up") },
    { Pid::LEADING_SPACE,           false, "leadingSpace",          P_TYPE::SPATIUM,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "leading space") },
    { Pid::MIRROR_HEAD,             false, "mirror",                P_TYPE::DIRECTION_H,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "mirror") },
    { Pid::HAS_PARENTHESES,         true , "parentheses",           P_TYPE::PARENTHESES_MODE,   PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "parentheses") },
    { Pid::DOT_POSITION,            false, "dotPosition",           P_TYPE::DIRECTION_V,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "dot position") },
    { Pid::COMBINE_VOICE,           true,  "combineVoice",          P_TYPE::AUTO_ON_OFF,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "combine voice") },
    { Pid::TUNING,                  false, "tuning",                P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tuning") },
    { Pid::PAUSE,                   true,  "pause",                 P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "pause") },

    { Pid::BARLINE_TYPE,            false, "subtype",               P_TYPE::BARLINE_TYPE,       PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "barline type") },
    { Pid::BARLINE_SPAN,            false, "span",                  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "span") },
    { Pid::BARLINE_SPAN_FROM,       false, "spanFromOffset",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "span from") },
    { Pid::BARLINE_SPAN_TO,         false, "spanToOffset",          P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "span to") },
    { Pid::BARLINE_SHOW_TIPS,       false, "showTips",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "show tips") },

    { Pid::OFFSET,                  false, "offset",                P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "offset") },
    { Pid::FRET,                    true,  "fret",                  P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "fret") },
    { Pid::STRING,                  true,  "string",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "string") },
    { Pid::GHOST,                   true,  "ghost",                 P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ghost") },
    { Pid::DEAD,                    true,  "dead",                  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "dead") },
    { Pid::PLAY,                    false, "play",                  P_TYPE::BOOL,               PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "played") },
    { Pid::TIMESIG_NOMINAL,         false, "timesigNominal",        P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "nominal time signature") },
    { Pid::TIMESIG_ACTUAL,          true,  "timesigActual",         P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "actual time signature") },
    { Pid::NUMBER_TYPE,             false, "numberType",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "number type") },
    { Pid::BRACKET_TYPE,            false, "bracketType",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bracket type") },
    { Pid::NORMAL_NOTES,            false, "normalNotes",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "normal notes") },
    { Pid::ACTUAL_NOTES,            false, "actualNotes",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "actual notes") },
    { Pid::P1,                      false, "p1",                    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "bracket start offset") },
    { Pid::P2,                      false, "p2",                    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "bracket end offset") },
    { Pid::GROW_LEFT,               false, "growLeft",              P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "grow left") },
    { Pid::GROW_RIGHT,              false, "growRight",             P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "grow right") },

    { Pid::BOX_HEIGHT,              false, "height",                P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "height") },
    { Pid::BOX_WIDTH,               false, "width",                 P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "width") },
    { Pid::BOX_AUTOSIZE,            false, "boxAutoSize",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "autosize frame") },
    { Pid::TOP_GAP,                 false, "topGap",                P_TYPE::SPATIUM,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "top gap") },
    { Pid::BOTTOM_GAP,              false, "bottomGap",             P_TYPE::SPATIUM,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "bottom gap") },
    { Pid::LEFT_MARGIN,             false, "leftMargin",            P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "left padding") },
    { Pid::RIGHT_MARGIN,            false, "rightMargin",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "right padding") },
    { Pid::TOP_MARGIN,              false, "topMargin",             P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "top padding") },
    { Pid::BOTTOM_MARGIN,           false, "bottomMargin",          P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bottom padding") },
    { Pid::PADDING_TO_NOTATION_ABOVE, false, "paddingToNotationAbove", P_TYPE::SPATIUM,         PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "padding to notation above") },
    { Pid::PADDING_TO_NOTATION_BELOW, false, "paddingToNotationBelow", P_TYPE::SPATIUM,         PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "padding to notation below") },

    { Pid::LAYOUT_BREAK,            false, "subtype",               P_TYPE::LAYOUTBREAK_TYPE,   PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "layout break type") },
    { Pid::AUTOSCALE,               false, "autoScale",             P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "autoscale") },
    { Pid::SIZE,                    false, "size",                  P_TYPE::SIZE,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "size") },

    { Pid::IMAGE_HEIGHT,            false, "imageHeight",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "image height") },
    { Pid::IMAGE_WIDTH,             false, "imageWidth",            P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "image width") },
    { Pid::IMAGE_FRAMED,            false, "imageFramed",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "image framed") },

    { Pid::FRET_FRAME_TEXT_SCALE,     false, "fretFrameTextScale",       P_TYPE::REAL,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "text scale") },
    { Pid::FRET_FRAME_DIAGRAM_SCALE,  false, "fretFrameDiagramScale",    P_TYPE::REAL,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "diagram scale") },
    { Pid::FRET_FRAME_COLUMN_GAP,     false, "fretFrameColumnGap",       P_TYPE::SPATIUM,       PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "column gap") },
    { Pid::FRET_FRAME_ROW_GAP,        false, "fretFrameRowGap",          P_TYPE::SPATIUM,       PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "row gap") },
    { Pid::FRET_FRAME_CHORDS_PER_ROW, false, "fretFrameChordsPerRow",    P_TYPE::INT,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbols per row") },
    { Pid::FRET_FRAME_H_ALIGN,        false, "fretFrameHorizontalAlign", P_TYPE::INT,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "horizontal alignment") },
    { Pid::FRET_FRAME_DIAGRAMS_ORDER, false, "fretFrameDiagramsOrder",   P_TYPE::STRING,        PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "diagrams order") },

    { Pid::SCALE,                   false, "scale",                 P_TYPE::SCALE,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "scale") },
    { Pid::LOCK_ASPECT_RATIO,       false, "lockAspectRatio",       P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "aspect ratio locked") },
    { Pid::SIZE_IS_SPATIUM,         false, "sizeIsSpatium",         P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "size is spatium") },
    { Pid::TEXT,                    false,  "text",                 P_TYPE::STRING,             PropertyGroup::TEXT,            QT_TRANSLATE_NOOP("engraving/propertyName", "text") },
    { Pid::HTML_TEXT,               false, "",                      P_TYPE::STRING,             PropertyGroup::TEXT,            "" },
    { Pid::USER_MODIFIED,           false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      "" },
    { Pid::BEAM_POS,                false, "",                      P_TYPE::PAIR_REAL,          PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "beam position") },
    { Pid::BEAM_MODE,               true, "BeamMode",               P_TYPE::BEAM_MODE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "beam mode") },
    { Pid::BEAM_NO_SLOPE,           true, "noSlope",                P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "without slope") },
    { Pid::BEAM_CROSS_STAFF_MOVE,   true, "crossStaffMove",         P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "beam staff move") },
    { Pid::USER_LEN,                false, "userLen",               P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "length") },
    { Pid::SHOW_STEM_SLASH,         true,  "showStemSlash",         P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "show stem slash") },

    { Pid::SPACE,                   false, "space",                 P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "space") },
    { Pid::TEMPO,                   true,  "tempo",                 P_TYPE::TEMPO,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tempo") },
    { Pid::TEMPO_FOLLOW_TEXT,       true,  "followText",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "following text") },
    { Pid::TEMPO_ALIGN_RIGHT_OF_REHEARSAL_MARK, false, "tempoAlignRightOfRehearsalMark", P_TYPE::BOOL, PropertyGroup::APPEARANCE, QT_TRANSLATE_NOOP("engraving/propertyName", "tempo align right of rehearsal mark") },

    { Pid::ACCIDENTAL_BRACKET,      false, "bracket",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bracket") },
    { Pid::ACCIDENTAL_TYPE,         true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "accidental type") },
    { Pid::ACCIDENTAL_STACKING_ORDER_OFFSET, true, "stackingOrderOffset", P_TYPE::INT,          PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "stacking order offset") },
    { Pid::NUMERATOR_STRING,        false, "textN",                 P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "numerator string") },
    { Pid::DENOMINATOR_STRING,      false, "textD",                 P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "denominator string") },
    { Pid::FBPREFIX,                false, "prefix",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "prefix") },
    { Pid::FBDIGIT,                 false, "digit",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "digit") },
    { Pid::FBSUFFIX,                false, "suffix",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "suffix") },
    { Pid::FBCONTINUATIONLINE,      false, "continuationLine",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continuation line") },

    { Pid::FBPARENTHESIS1,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS2,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS3,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS4,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },
    { Pid::FBPARENTHESIS5,          false, "",                      P_TYPE::INT,                PropertyGroup::APPEARANCE,      "" },

    { Pid::OTTAVA_TYPE,             true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ottava type") },
    { Pid::NUMBERS_ONLY,            false, "numbersOnly",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "numbers only") },
    { Pid::TRILL_TYPE,              false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "trill type") },
    { Pid::VIBRATO_TYPE,            false, "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "vibrato type") },
    { Pid::HAIRPIN_CIRCLEDTIP,      false, "hairpinCircledTip",     P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin with circled tip") },

    { Pid::HAIRPIN_TYPE,            true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin type") },
    { Pid::HAIRPIN_HEIGHT,          false, "hairpinHeight",         P_TYPE::SPATIUM,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin height") },
    { Pid::HAIRPIN_CONT_HEIGHT,     false, "hairpinContHeight",     P_TYPE::SPATIUM,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin cont height") },
    { Pid::VELO_CHANGE,             true,  "veloChange",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity change") },
    { Pid::VELO_CHANGE_METHOD,      true,  "veloChangeMethod",      P_TYPE::CHANGE_METHOD,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity change method") }, // left as a compatibility property - we need to be able to read it correctly
    { Pid::VELO_CHANGE_SPEED,       true,  "veloChangeSpeed",       P_TYPE::DYNAMIC_SPEED,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity change speed") },
    { Pid::DYNAMIC_TYPE,            true,  "subtype",               P_TYPE::DYNAMIC_TYPE,       PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "dynamic type") },

    { Pid::SINGLE_NOTE_DYNAMICS,    true,  "singleNoteDynamics",    P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "single note dynamics") },
    { Pid::CHANGE_METHOD,           true,  "changeMethod",          P_TYPE::CHANGE_METHOD,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "change method") }, // the new, more general version of VELO_CHANGE_METHOD
    { Pid::PLACEMENT,               false, "placement",             P_TYPE::PLACEMENT_V,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "placement") },
    { Pid::HPLACEMENT,              false, "hplacement",            P_TYPE::PLACEMENT_H,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "horizontal placement") },
    { Pid::MMREST_RANGE_BRACKET_TYPE, false, "mmrestRangeBracketType", P_TYPE::INT,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "multimeasure rest range bracket type") },
    { Pid::VELOCITY,                false, "velocity",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity") },
    { Pid::JUMP_TO,                 true,  "jumpTo",                P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "jump to") },
    { Pid::PLAY_UNTIL,              true,  "playUntil",             P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "play until") },
    { Pid::CONTINUE_AT,             true,  "continueAt",            P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue at") },
    { Pid::LABEL,                   true,  "label",                 P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "label") },
    { Pid::MARKER_TYPE,             true,  "markerType",            P_TYPE::MARKER_TYPE,        PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "marker type") },
    { Pid::MUSIC_SYMBOL_SIZE,       true,  "musicSymbolSize",       P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "music symbol size") },
    { Pid::MARKER_CENTER_ON_SYMBOL, true,  "markerCenterOnSymbol",  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "marker center on symbol") },
    { Pid::ARP_USER_LEN1,           false, "arpUserLen1",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "length 1") },
    { Pid::ARP_USER_LEN2,           false, "arpUserLen2",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "length 2") },
    { Pid::REPEAT_END,              true,  "repeatEnd",             P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "repeat end") },
    { Pid::REPEAT_START,            true,  "repeatStart",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "repeat start") },
    { Pid::REPEAT_JUMP,             true,  "repeatJump",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "repeat jump") },
    { Pid::MEASURE_NUMBER_MODE,     false, "measureNumberMode",     P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "measure number mode") },

    { Pid::GLISS_TYPE,              false, "subtype",               P_TYPE::GLISS_TYPE,         PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "glissando type") },
    { Pid::GLISS_TEXT,              false, "text",                  P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "text") },
    { Pid::GLISS_SHOW_TEXT,         false, "glissandoShowText",     P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "showing text") },
    { Pid::GLISS_STYLE,             true,  "glissandoStyle",        P_TYPE::GLISS_STYLE,        PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "glissando style") },
    { Pid::GLISS_SHIFT,             false, "glissandoShift",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "glissando shift") },
    { Pid::GLISS_EASEIN,            false, "easeInSpin",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ease in") },
    { Pid::GLISS_EASEOUT,           false, "easeOutSpin",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ease out") },
    { Pid::DIAGONAL,                false, "diagonal",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "diagonal") },
    { Pid::GROUP_NODES,             false, "",                      P_TYPE::GROUPS,             PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "groups") },
    { Pid::LINE_STYLE,              true,  "lineStyle",             P_TYPE::LINE_TYPE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "line style") },
    { Pid::LINE_WIDTH,              false, "lineWidth",             P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "line width") },
    { Pid::TIME_STRETCH,            true,  "timeStretch",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "time stretch") },
    { Pid::ORNAMENT_STYLE,          true,  "ornamentStyle",         P_TYPE::ORNAMENT_STYLE,     PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ornament style") },
    { Pid::INTERVAL_ABOVE,          true,  "intervalAbove",         P_TYPE::ORNAMENT_INTERVAL,  PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "interval above") },
    { Pid::INTERVAL_BELOW,          true,  "intervalBelow",         P_TYPE::ORNAMENT_INTERVAL,  PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "interval below") },
    { Pid::ORNAMENT_SHOW_ACCIDENTAL,true,  "ornamentShowAccidental",P_TYPE::ORNAMENT_SHOW_ACCIDENTAL, PropertyGroup::APPEARANCE, QT_TRANSLATE_NOOP("engraving/propertyName", "ornament show accidental") },
    { Pid::ORNAMENT_SHOW_CUE_NOTE,  true,  "ornamentShowCueNote",   P_TYPE::AUTO_ON_OFF,        PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ornament show cue note") },
    { Pid::START_ON_UPPER_NOTE,     true,  "startOnUpperNote",      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "start on upper note") },

    { Pid::TIMESIG,                 false, "timesig",               P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "time signature") },
    { Pid::TIMESIG_STRETCH,         false, "",                      P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "time signature stretch") },
    { Pid::TIMESIG_TYPE,            true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "time signature type") },
    { Pid::SPANNER_TICK,            true,  "tick",                  P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tick") },
    { Pid::SPANNER_TICKS,           true,  "ticks",                 P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ticks") },
    { Pid::SPANNER_TRACK2,          false, "track2",                P_TYPE::SIZE_T,             PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "end track") },
    { Pid::OFFSET2,                 false, "userOff2",              P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "end offset") },
    { Pid::BREAK_MMR,               false, "breakMultiMeasureRest", P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "breaking multimeasure rest") },
    { Pid::MMREST_NUMBER_POS,       false, "mmRestNumberPos",       P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "vertical position of multimeasure rest number") }, // Deprecated
    { Pid::MMREST_NUMBER_OFFSET,    false, "mmRestNumberOffset",    P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "vertical offset of multimeasure rest number") },
    { Pid::MMREST_NUMBER_VISIBLE,   false, "mmRestNumberVisible",   P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "visibility of multimeasure rest number") },

    { Pid::MEASURE_REPEAT_NUMBER_POS, false, "measureRepeatNumberPos", P_TYPE::SPATIUM,         PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "vertical position of measure repeat number") },
    { Pid::REPEAT_COUNT,            true,  "repeatCount",             P_TYPE::INT,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "repeat count") },

    { Pid::USER_STRETCH,            false, "stretch",               P_TYPE::REAL,               PropertyGroup::NONE      ,      QT_TRANSLATE_NOOP("engraving/propertyName", "stretch") },
    { Pid::NO_OFFSET,               true,  "noOffset",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "numbering offset") },
    { Pid::IRREGULAR,               true,  "irregular",             P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "irregular") },
    { Pid::ANCHOR,                  false, "anchor",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "anchor") },
    { Pid::SLUR_UOFF1,              false, "o1",                    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "start offset") },
    { Pid::SLUR_UOFF2,              false, "o2",                    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "left shoulder offset") },
    { Pid::SLUR_UOFF3,              false, "o3",                    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "right shoulder offset") },
    { Pid::SLUR_UOFF4,              false, "o4",                    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "end offset") },
    { Pid::STAFF_MOVE,              true,  "staffMove",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "staff move") },
    { Pid::VERSE,                   true,  "no",                    P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "verse") },

    { Pid::SYLLABIC,                true,  "syllabic",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "syllabic") },
    { Pid::LYRIC_TICKS,             true,  "ticks_f",               P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ticks") },
    { Pid::VOLTA_ENDING,            true,  "endings",               P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "endings") },
    { Pid::LINE_VISIBLE,            true,  "lineVisible",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "visible line") },
    { Pid::MAG,                     false, "mag",                   P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "magnification") },
    { Pid::USE_DRUMSET,             false, "useDrumset",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "using drumset") },
    { Pid::DURATION,                true,  "",                      P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "duration") },
    { Pid::DURATION_TYPE_WITH_DOTS, true,  "",                      P_TYPE::DURATION_TYPE_WITH_DOTS, PropertyGroup::APPEARANCE, QT_TRANSLATE_NOOP("engraving/propertyName", "duration type") },
    { Pid::ACCIDENTAL_ROLE,         false, "role",                  P_TYPE::ACCIDENTAL_ROLE,    PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "role") },
    { Pid::TRACK,                   false, "",                      P_TYPE::SIZE_T,             PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "track") },

    { Pid::FRET_STRINGS,            true,  "strings",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "strings") },
    { Pid::FRET_FRETS,              true,  "frets",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frets") },
    { Pid::FRET_NUT,                true,  "showNut",               P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "show nut") },
    { Pid::FRET_OFFSET,             true,  "fretOffset",            P_TYPE::INT,                PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "fret offset") },
    { Pid::FRET_NUM_POS,            true,  "fretNumPos",            P_TYPE::INT,                PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "fret number position") },
    { Pid::ORIENTATION,             true,  "orientation",           P_TYPE::ORIENTATION,        PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "orientation") },
    { Pid::FRET_SHOW_FINGERINGS,    true,  "fretShowFingering",     P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "fretboard diagram fingering visible") },
    { Pid::FRET_FINGERING,          true,  "fretFingering",         P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "fretboard diagram fingering") },

    { Pid::HARMONY_VOICE_LITERAL,   true,  "harmonyVoiceLiteral",   P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol interpretation") },
    { Pid::HARMONY_VOICING,         true,  "harmonyVoicing",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol voicing") },
    { Pid::HARMONY_DURATION,        true,  "harmonyDuration",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol duration") },
    { Pid::HARMONY_BASS_SCALE,      true,  "harmonyBassScale",      P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol bass scale") },
    { Pid::HARMONY_DO_NOT_STACK_MODIFIERS, true, "harmonyDoNotStackModifiers", P_TYPE::BOOL,    PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol do not stack modifiers") },

    { Pid::SYSTEM_BRACKET,          false, "type",                  P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "type") },
    { Pid::GAP,                     false, "",                      P_TYPE::BOOL,               PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "gap") },
    { Pid::AUTOPLACE,               false, "autoplace",             P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "autoplace") },
    { Pid::DASH_LINE_LEN,           false, "dashLineLength",        P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "dash line length") },
    { Pid::DASH_GAP_LEN,            false, "dashGapLength",         P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "dash gap length") },
    { Pid::TICK,                    false, "",                      P_TYPE::FRACTION,           PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "tick") },
    { Pid::PLAYBACK_VOICE1,         false, "playbackVoice1",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 1") },
    { Pid::PLAYBACK_VOICE2,         false, "playbackVoice2",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 2") },
    { Pid::PLAYBACK_VOICE3,         false, "playbackVoice3",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 3") },

    { Pid::PLAYBACK_VOICE4,         false, "playbackVoice4",        P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 4") },
    { Pid::SYMBOL,                  true,  "symbol",                P_TYPE::SYMID,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "symbol") },
    { Pid::PLAY_REPEATS,            true,  "playRepeats",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "playing repeats") },
    { Pid::CREATE_SYSTEM_HEADER,    false, "createSystemHeader",    P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "creating system header") },
    { Pid::STAFF_LINES,             true,  "lines",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "lines") },
    { Pid::LINE_DISTANCE,           true,  "lineDistance",          P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "line distance") },
    { Pid::STEP_OFFSET,             true,  "stepOffset",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "step offset") },
    { Pid::STAFF_SHOW_BARLINES,     false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "showing barlines") },
    { Pid::STAFF_SHOW_LEDGERLINES,  false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "showing ledgerlines") },
    { Pid::STAFF_STEMLESS,          false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "stemless") },
    { Pid::STAFF_INVISIBLE,         false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "invisible") },
    { Pid::STAFF_COLOR,             false, "color",                 P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "color") },

    { Pid::HEAD_SCHEME,             false, "headScheme",            P_TYPE::NOTEHEAD_SCHEME,    PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "notehead scheme") },
    { Pid::STAFF_GEN_CLEF,          false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "generating clefs") },
    { Pid::STAFF_GEN_TIMESIG,       false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "generating time signature") },
    { Pid::STAFF_GEN_KEYSIG,        false, "",                      P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "generating key signature") },
    { Pid::STAFF_YOFFSET,           false, "",                      P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "y-offset") },
    { Pid::STAFF_USERDIST,          false, "distOffset",            P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "distance offset") },
    { Pid::STAFF_BARLINE_SPAN,      false, "barLineSpan",           P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "barline span") },
    { Pid::STAFF_BARLINE_SPAN_FROM, false, "barLineSpanFrom",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "barline span from") },
    { Pid::STAFF_BARLINE_SPAN_TO,   false, "barLineSpanTo",         P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "barline span to") },
    { Pid::BRACKET_SPAN,            false, "bracketSpan",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bracket span") },

    { Pid::BRACKET_COLUMN,          false, "level",                 P_TYPE::SIZE_T,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "level") },
    { Pid::INAME_LAYOUT_POSITION,   false, "layoutPosition",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "layout position") },
    { Pid::TEXT_STYLE,              false, "style",                 P_TYPE::TEXT_STYLE,         PropertyGroup::TEXT,            QT_TRANSLATE_NOOP("engraving/propertyName", "style") },
    { Pid::FONT_FACE,               false, "family",                P_TYPE::STRING,             PropertyGroup::TEXT,            QT_TRANSLATE_NOOP("engraving/propertyName", "family") },
    { Pid::FONT_SIZE,               false, "size",                  P_TYPE::REAL,               PropertyGroup::TEXT,            QT_TRANSLATE_NOOP("engraving/propertyName", "size") },
    { Pid::FONT_STYLE,              false, "fontStyle",             P_TYPE::INT,                PropertyGroup::TEXT,            QT_TRANSLATE_NOOP("engraving/propertyName", "font style") },
    { Pid::TEXT_LINE_SPACING,       false, "textLineSpacing",       P_TYPE::REAL,               PropertyGroup::TEXT,            QT_TRANSLATE_NOOP("engraving/propertyName", "user line distancing") },

    { Pid::FRAME_TYPE,              false, "frameType",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frame type") },
    { Pid::FRAME_WIDTH,             false, "frameWidth",            P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frame width") },
    { Pid::FRAME_PADDING,           false, "framePadding",          P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frame padding") },
    { Pid::FRAME_ROUND,             false, "frameRound",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frame round") },
    { Pid::FRAME_FG_COLOR,          false, "frameFgColor",          P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frame foreground color") },
    { Pid::FRAME_BG_COLOR,          false, "frameBgColor",          P_TYPE::COLOR,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "frame background color") },
    { Pid::SIZE_SPATIUM_DEPENDENT,  false, "sizeIsSpatiumDependent",P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "spatium dependent size") },
    { Pid::TEXT_SIZE_SPATIUM_DEPENDENT, false, "textSizeIsSpatiumDependent", P_TYPE::BOOL,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "spatium dependent text size") },
    { Pid::MUSICAL_SYMBOLS_SCALE,   false, "musicalSymbolsScale",   P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "musical symbols scale") },
    { Pid::ALIGN,                   false, "align",                 P_TYPE::ALIGN,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "align") },
    { Pid::TEXT_SCRIPT_ALIGN,       false, "align",                 P_TYPE::INT,                PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "text script align") },
    { Pid::SYSTEM_FLAG,             false, "systemFlag",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "system flag") },

    { Pid::BEGIN_TEXT,              true,  "beginText",             P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin text") },
    { Pid::BEGIN_TEXT_ALIGN,        false, "beginTextAlign",        P_TYPE::ALIGN,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin text align") },
    { Pid::BEGIN_TEXT_PLACE,        false, "beginTextPlace",        P_TYPE::TEXT_PLACE,         PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin text place") },
    { Pid::BEGIN_HOOK_TYPE,         true,  "beginHookType",         P_TYPE::HOOK_TYPE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin hook type") },
    { Pid::BEGIN_HOOK_HEIGHT,       false, "beginHookHeight",       P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin hook height") },
    { Pid::BEGIN_FONT_FACE,         false, "beginFontFace",         P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin font face") },
    { Pid::BEGIN_FONT_SIZE,         false, "beginFontSize",         P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin font size") },
    { Pid::BEGIN_FONT_STYLE,        false, "beginFontStyle",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "begin font style") },
    { Pid::BEGIN_TEXT_OFFSET,       false, "beginTextOffset",       P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "begin text offset") },
    { Pid::GAP_BETWEEN_TEXT_AND_LINE, false, "gapBetweenTextAndLine", P_TYPE::SPATIUM,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "gap between text and line") },

    { Pid::CONTINUE_TEXT,           true,  "continueText",          P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue text") },
    { Pid::CONTINUE_TEXT_ALIGN,     false, "continueTextAlign",     P_TYPE::ALIGN,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue text align") },
    { Pid::CONTINUE_TEXT_PLACE,     false, "continueTextPlace",     P_TYPE::TEXT_PLACE,         PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue text place") },
    { Pid::CONTINUE_FONT_FACE,      false, "continueFontFace",      P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue font face") },
    { Pid::CONTINUE_FONT_SIZE,      false, "continueFontSize",      P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue font size") },
    { Pid::CONTINUE_FONT_STYLE,     false, "continueFontStyle",     P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "continue font style") },
    { Pid::CONTINUE_TEXT_OFFSET,    false, "continueTextOffset",    P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "continue text offset") },

    { Pid::END_TEXT,                true,  "endText",               P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end text") },
    { Pid::END_TEXT_ALIGN,          false, "endTextAlign",          P_TYPE::ALIGN,              PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end text align") },
    { Pid::END_TEXT_PLACE,          false, "endTextPlace",          P_TYPE::TEXT_PLACE,         PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end text place") },
    { Pid::END_HOOK_TYPE,           true,  "endHookType",           P_TYPE::HOOK_TYPE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end hook type") },
    { Pid::END_HOOK_HEIGHT,         false, "endHookHeight",         P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end hook height") },
    { Pid::END_FONT_FACE,           false, "endFontFace",           P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end font face") },
    { Pid::END_FONT_SIZE,           false, "endFontSize",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end font size") },
    { Pid::END_FONT_STYLE,          false, "endFontStyle",          P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "end font style") },
    { Pid::END_TEXT_OFFSET,         false, "endTextOffset",         P_TYPE::POINT,              PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "end text offset") },

    { Pid::NOTELINE_PLACEMENT,      false, "noteLinePlacement",     P_TYPE::NOTELINE_PLACEMENT_TYPE, PropertyGroup::APPEARANCE, QT_TRANSLATE_NOOP("engraving/propertyName", "note-anchored line placement") },

    { Pid::AVOID_BARLINES,          false, "avoidBarLines",         P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "avoid barlines") },
    { Pid::DYNAMICS_SIZE,           false, "dynamicsSize",          P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "dynamic size") },
    { Pid::CENTER_ON_NOTEHEAD,      false, "centerOnNotehead",      P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "use text alignment") },
    { Pid::ANCHOR_TO_END_OF_PREVIOUS, true, "anchorToEndOfPrevious", P_TYPE::BOOL,              PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "anchor to end of previous") },

    { Pid::SNAP_TO_DYNAMICS,         false, "snapToDynamics",       P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "snap expression") }, // for expressions
    { Pid::SNAP_BEFORE,              false, "snapBefore",           P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "snap before") },     // <
    { Pid::SNAP_AFTER,               false, "snapAfter",            P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "snap after") },      // < for hairpins

    { Pid::VOICE_ASSIGNMENT,        true,  "voiceAssignment",       P_TYPE::VOICE_ASSIGNMENT,   PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "voice assignment") },
    { Pid::CENTER_BETWEEN_STAVES,   false, "centerBetweenStaves",   P_TYPE::AUTO_ON_OFF,        PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "center between staves") },

    { Pid::POS_ABOVE,               false, "posAbove",              P_TYPE::MILLIMETRE,         PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "position above") },

    { Pid::LOCATION_STAVES,         false, "staves",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "staves distance") },
    { Pid::LOCATION_VOICES,         false, "voices",                P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "voices distance") },
    { Pid::LOCATION_MEASURES,       false, "measures",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "measures distance") },
    { Pid::LOCATION_FRACTIONS,      false, "fractions",             P_TYPE::FRACTION,           PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "position distance") },
    { Pid::LOCATION_GRACE,          false, "grace",                 P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "grace note index") },
    { Pid::LOCATION_NOTE,           false, "note",                  P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "note index") },

    { Pid::VOICE,                   true,  "voice",                 P_TYPE::SIZE_T,             PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "voice") },
    { Pid::POSITION,                false, "position",              P_TYPE::ALIGN_H,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "position") },

    { Pid::CLEF_TYPE_CONCERT,       true,  "concertClefType",       P_TYPE::CLEF_TYPE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "concert clef type") },
    { Pid::CLEF_TYPE_TRANSPOSING,   true,  "transposingClefType",   P_TYPE::CLEF_TYPE,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "transposing clef type") },
    { Pid::CLEF_TO_BARLINE_POS,     true,  "clefToBarlinePos",      P_TYPE::CLEF_TO_BARLINE_POS, PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "clef to barline position") },
    { Pid::IS_HEADER,               true,  "isHeader",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "is header")},
    { Pid::KEY_CONCERT,             true,  "concertKey",            P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "concert key") },
    { Pid::KEY,                     true,  "actualKey",             P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "key") },
    { Pid::ACTION,                  false, "action",                P_TYPE::STRING,             PropertyGroup::APPEARANCE,      0 },
    { Pid::MIN_DISTANCE,            false, "minDistance",           P_TYPE::SPATIUM,            PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "autoplace minimum distance") },

    { Pid::ARPEGGIO_TYPE,           true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "arpeggio type") },
    { Pid::CHORD_LINE_TYPE,         true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "chord line type") },
    { Pid::CHORD_LINE_STRAIGHT,     true,  "straight",              P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "straight chord line") },
    { Pid::CHORD_LINE_WAVY,         true,  "wavy",                  P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "wavy chord line") },
    { Pid::TREMOLO_TYPE,            true,  "subtype",               P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tremolo type") },
    { Pid::TREMOLO_STYLE,           true,  "strokeStyle",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tremolo style") },
    { Pid::HARMONY_TYPE,            true,  "harmonyType",           P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "harmony type") },

    { Pid::ARPEGGIO_SPAN,           true,  "arpeggioSpan",          P_TYPE::INT,                PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "arpeggio span") },

    { Pid::BEND_TYPE,               true,  "bendType",              P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bend type") },
    { Pid::BEND_CURVE,              true,  "bendCurve",             P_TYPE::PITCH_VALUES,       PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bend curve") },
    { Pid::BEND_VERTEX_OFF,         false, "bendVertexOffset",      P_TYPE::POINT,              PropertyGroup::POSITION  ,      QT_TRANSLATE_NOOP("engraving/propertyName", "bend vertex offset") },
    { Pid::BEND_SHOW_HOLD_LINE,     false, "bendShowHoldLine",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bend show hold line") },
    { Pid::BEND_START_TIME_FACTOR,  true,  "bendStartTimeFactor",   P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bend start time factor") },
    { Pid::BEND_END_TIME_FACTOR,    true,  "bendEndTimeFactor",     P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "bend end time factor") },

    { Pid::TREMOLOBAR_TYPE,         true,  "tremoloBarType",        P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tremolobar type") },
    { Pid::TREMOLOBAR_CURVE,        true,  "tremoloBarCurve",       P_TYPE::PITCH_VALUES,       PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tremolobar curve") },

    { Pid::START_WITH_LONG_NAMES,   false, "startWithLongNames",    P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "start with long names") },
    { Pid::START_WITH_MEASURE_ONE,  true,  "startWithMeasureOne",   P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "start with measure one") },
    { Pid::FIRST_SYSTEM_INDENTATION,true,  "firstSystemIndentation",P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "first system indentation") },

    { Pid::PATH,                    false, "path",                  P_TYPE::DRAW_PATH,          PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "path") },

    { Pid::PREFER_SHARP_FLAT,       true,  "preferSharpFlat",       P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "prefer sharps or flats") },

    { Pid::PLAY_TECH_TYPE,          true,  "playTechType",          P_TYPE::PLAYTECH_TYPE,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "playing technique type") },

    { Pid::TEMPO_CHANGE_TYPE,       true,  "tempoChangeType",       P_TYPE::TEMPOCHANGE_TYPE,   PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "gradual tempo change type") },
    { Pid::TEMPO_EASING_METHOD,     true,  "tempoEasingMethod",     P_TYPE::CHANGE_METHOD,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tempo easing method") },
    { Pid::TEMPO_CHANGE_FACTOR,     true,  "tempoChangeFactor",     P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tempo change factor") },

    { Pid::HARP_IS_DIAGRAM,         false,  "isDiagram",            P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "is diagram") },

    { Pid::ACTIVE,                  true,  "active",                P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "active") },

    { Pid::CAPO_FRET_POSITION,      true,  "fretPosition",          P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "fret position") },
    { Pid::CAPO_IGNORED_STRINGS,    true,  "ignoredStrings",        P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "ignored strings") },
    { Pid::CAPO_GENERATE_TEXT,      true,  "generateText",          P_TYPE::BOOL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "automatically generate text") },

    { Pid::TIE_PLACEMENT,           true,  "tiePlacement",          P_TYPE::TIE_PLACEMENT,      PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "tie placement") },
    { Pid::MIN_LENGTH,              true,  "minLength",             P_TYPE::SPATIUM,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "minimum length") },
    { Pid::PARTIAL_SPANNER_DIRECTION, true, "partialSpannerDirection", P_TYPE::PARTIAL_SPANNER_DIRECTION, PropertyGroup::NONE,  QT_TRANSLATE_NOOP("engraving/propertyName", "partial spanner direction") },

    { Pid::POSITION_LINKED_TO_MASTER,   false, "positionLinkedToMaster",   P_TYPE::BOOL,        PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "position linked to main score") },
    { Pid::APPEARANCE_LINKED_TO_MASTER, false, "appearanceLinkedToMaster", P_TYPE::BOOL,        PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "appearance linked to main score") },
    { Pid::TEXT_LINKED_TO_MASTER,       false, "textLinkedToMaster",       P_TYPE::BOOL,        PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "text linked to main score") },
    { Pid::EXCLUDE_FROM_OTHER_PARTS,    false, "excludeFromParts",         P_TYPE::BOOL,        PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "exclude from parts") },

    { Pid::STRINGTUNINGS_STRINGS_COUNT, true,  "stringsCount",      P_TYPE::INT,                PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "strings count") },
    { Pid::STRINGTUNINGS_PRESET,    true,  "preset",                P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "strings preset") },
    { Pid::STRINGTUNINGS_VISIBLE_STRINGS,   true,  "visibleStrings",P_TYPE::INT_VEC,            PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "visible strings") },

    { Pid::SCORE_FONT,              true,  "scoreFont",             P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "score font") },
    { Pid::SYMBOLS_SIZE,            false, "symbolsSize",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "symbols size") },
    { Pid::SYMBOL_ANGLE,            false, "symbolAngle",           P_TYPE::REAL,               PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "symbol angle") },

    { Pid::APPLY_TO_ALL_STAVES,     false, "applyToAllStaves",      P_TYPE::BOOL,               PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "apply to all staves") },
    { Pid::IS_COURTESY,             false, "isCourtesy",            P_TYPE::BOOL,               PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "is courtesy") },
    { Pid::EXCLUDE_VERTICAL_ALIGN,  false, "excludeVerticalAlign",  P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "exclude vertical align") },

    { Pid::SHOW_MEASURE_NUMBERS,    false, "showMeasureNumbers",    P_TYPE::AUTO_ON_OFF,        PropertyGroup::NONE,            QT_TRANSLATE_NOOP("engraving/propertyName", "show measure numbers")},
    { Pid::PLAY_COUNT_TEXT_SETTING, false, "playCountTextSetting",  P_TYPE::AUTO_CUSTOM_HIDE,   PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "play count text setting") },
    { Pid::PLAY_COUNT_TEXT,         false, "playCountCustomText",   P_TYPE::STRING,             PropertyGroup::APPEARANCE,      QT_TRANSLATE_NOOP("engraving/propertyName", "play count text") },

    { Pid::ALIGN_WITH_OTHER_RESTS,  false, "alignWithOtherRests",   P_TYPE::BOOL,               PropertyGroup::POSITION,        QT_TRANSLATE_NOOP("engraving/propertyName", "align with other rests in the same voice") },

    { Pid::END,                     false, "++end++",               P_TYPE::INT,                PropertyGroup::NONE,            "" }
};
/* *INDENT-ON* */

static constexpr bool isPropertyListConsistent()
{
    for (int i = 0; i <= int(Pid::END); ++i) {
        if (propertyList[i].id != Pid(i)) {
            return false;
        }
    }
    return true;
}

static_assert(isPropertyListConsistent(), "propertyList is not consistent with Pid enum");

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
    return propertyList[size_t(id)].type;
}

PropertyGroup propertyGroup(Pid id)
{
    return propertyList[size_t(id)].group;
}

//---------------------------------------------------------
//   propertyLink
//---------------------------------------------------------

bool propertyLink(Pid id)
{
    return propertyList[size_t(id)].link;
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
    return propertyList[size_t(id)].name;
}

//---------------------------------------------------------
//   propertyUserName
//---------------------------------------------------------

String propertyUserName(Pid id)
{
    return muse::mtrc("engraving/propertyName", propertyList[size_t(id)].userName);
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
