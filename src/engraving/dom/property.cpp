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
    P_TYPE type;              // associated P_TYPE
    PropertyGroup group;
    bool link;                // link this property for linked elements
    const char* name;         // xml name of property
    const char* userName;     // user-visible name of property
};

//
// always: propertyList[subtype].id == subtype
//
//

/* *INDENT-OFF* */
static constexpr PropertyMetaData propertyList[] = {
    { Pid::SUBTYPE,                             P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "subtype") },
    { Pid::SELECTED,                            P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "selected",                        QT_TRANSLATE_NOOP("engraving/propertyName", "selected") },
    { Pid::GENERATED,                           P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "generated",                       QT_TRANSLATE_NOOP("engraving/propertyName", "generated") },
    { Pid::COLOR,                               P_TYPE::COLOR,                     PropertyGroup::APPEARANCE, false, "color",                           QT_TRANSLATE_NOOP("engraving/propertyName", "color") },
    { Pid::VISIBLE,                             P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "visible",                         QT_TRANSLATE_NOOP("engraving/propertyName", "visible") },
    { Pid::Z,                                   P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "z",                               QT_TRANSLATE_NOOP("engraving/propertyName", "stacking order") },
    { Pid::SMALL,                               P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "small",                           QT_TRANSLATE_NOOP("engraving/propertyName", "small") },
    { Pid::HIDE_WHEN_EMPTY,                     P_TYPE::AUTO_ON_OFF,               PropertyGroup::APPEARANCE, false, "hideWhenEmpty",                   QT_TRANSLATE_NOOP("engraving/propertyName", "hide when empty") },
    { Pid::HIDE_STAVES_WHEN_INDIVIDUALLY_EMPTY, P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "hideStavesWhenIndividuallyEmpty", QT_TRANSLATE_NOOP("engraving/propertyName", "hide staves when individually empty") },
    { Pid::SHOW_IF_ENTIRE_SYSTEM_EMPTY,         P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "showIfEntireSystemEmpty",         QT_TRANSLATE_NOOP("engraving/propertyName", "show if entire system empty") },
    { Pid::SHOW_COURTESY,                       P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "showCourtesySig",                 QT_TRANSLATE_NOOP("engraving/propertyName", "show courtesy") },
    { Pid::KEYSIG_MODE,                         P_TYPE::KEY_MODE,                  PropertyGroup::APPEARANCE, false, "keysig_mode",                     QT_TRANSLATE_NOOP("engraving/propertyName", "key signature mode") },
    { Pid::SLUR_STYLE_TYPE,                     P_TYPE::SLUR_STYLE_TYPE,           PropertyGroup::APPEARANCE, false, "lineType",                        QT_TRANSLATE_NOOP("engraving/propertyName", "line type") },
    { Pid::PITCH,                               P_TYPE::INT,                       PropertyGroup::NONE,       true,  "pitch",                           QT_TRANSLATE_NOOP("engraving/propertyName", "pitch") },

    { Pid::TPC1,                                P_TYPE::INT,                       PropertyGroup::NONE,       true,  "tpc",                             QT_TRANSLATE_NOOP("engraving/propertyName", "tonal pitch class") },
    { Pid::TPC2,                                P_TYPE::INT,                       PropertyGroup::NONE,       true,  "tpc2",                            QT_TRANSLATE_NOOP("engraving/propertyName", "transposed tonal pitch class") },
    { Pid::LINE,                                P_TYPE::INT,                       PropertyGroup::NONE,       false, "line",                            QT_TRANSLATE_NOOP("engraving/propertyName", "line") },
    { Pid::FIXED,                               P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "fixed",                           QT_TRANSLATE_NOOP("engraving/propertyName", "fixed") },
    { Pid::FIXED_LINE,                          P_TYPE::INT,                       PropertyGroup::NONE      , true,  "fixedLine",                       QT_TRANSLATE_NOOP("engraving/propertyName", "fixed line") },
    { Pid::HEAD_TYPE,                           P_TYPE::NOTEHEAD_TYPE,             PropertyGroup::APPEARANCE, false, "headType",                        QT_TRANSLATE_NOOP("engraving/propertyName", "head type") },
    { Pid::HEAD_GROUP,                          P_TYPE::NOTEHEAD_GROUP,            PropertyGroup::APPEARANCE, true,  "head",                            QT_TRANSLATE_NOOP("engraving/propertyName", "head") },
    { Pid::VELO_TYPE,                           P_TYPE::VELO_TYPE,                 PropertyGroup::APPEARANCE, false, "veloType",                        QT_TRANSLATE_NOOP("engraving/propertyName", "velocity type") },
    { Pid::USER_VELOCITY,                       P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "velocity",                        QT_TRANSLATE_NOOP("engraving/propertyName", "velocity") },
    { Pid::ARTICULATION_ANCHOR,                 P_TYPE::INT,                       PropertyGroup::POSITION,   false, "anchor",                          QT_TRANSLATE_NOOP("engraving/propertyName", "anchor") },

    { Pid::DIRECTION,                           P_TYPE::DIRECTION_V,               PropertyGroup::POSITION,   false, "direction",                       QT_TRANSLATE_NOOP("engraving/propertyName", "direction") },
    { Pid::HORIZONTAL_DIRECTION,                P_TYPE::DIRECTION_H,               PropertyGroup::POSITION,   false, "horizontalDirection",             QT_TRANSLATE_NOOP("engraving/propertyName", "horizontal direction") },
    { Pid::STEM_DIRECTION,                      P_TYPE::DIRECTION_V,               PropertyGroup::APPEARANCE, false, "StemDirection",                   QT_TRANSLATE_NOOP("engraving/propertyName", "stem direction") },
    { Pid::NO_STEM,                             P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "noStem",                          QT_TRANSLATE_NOOP("engraving/propertyName", "stemless") },
    { Pid::SLUR_DIRECTION,                      P_TYPE::DIRECTION_V,               PropertyGroup::POSITION,   false, "up",                              QT_TRANSLATE_NOOP("engraving/propertyName", "up") },
    { Pid::LEADING_SPACE,                       P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "leadingSpace",                    QT_TRANSLATE_NOOP("engraving/propertyName", "leading space") },
    { Pid::MIRROR_HEAD,                         P_TYPE::DIRECTION_H,               PropertyGroup::POSITION,   false, "mirror",                          QT_TRANSLATE_NOOP("engraving/propertyName", "mirror") },
    { Pid::HAS_PARENTHESES,                     P_TYPE::PARENTHESES_MODE,          PropertyGroup::APPEARANCE, true , "parentheses",                     QT_TRANSLATE_NOOP("engraving/propertyName", "parentheses") },
    { Pid::DOT_POSITION,                        P_TYPE::DIRECTION_V,               PropertyGroup::POSITION,   false, "dotPosition",                     QT_TRANSLATE_NOOP("engraving/propertyName", "dot position") },
    { Pid::COMBINE_VOICE,                       P_TYPE::AUTO_ON_OFF,               PropertyGroup::POSITION,   true,  "combineVoice",                    QT_TRANSLATE_NOOP("engraving/propertyName", "combine voice") },
    { Pid::TUNING,                              P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "tuning",                          QT_TRANSLATE_NOOP("engraving/propertyName", "tuning") },
    { Pid::PAUSE,                               P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "pause",                           QT_TRANSLATE_NOOP("engraving/propertyName", "pause") },

    { Pid::BARLINE_TYPE,                        P_TYPE::BARLINE_TYPE,              PropertyGroup::APPEARANCE, false, "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "barline type") },
    { Pid::BARLINE_SPAN,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "span",                            QT_TRANSLATE_NOOP("engraving/propertyName", "span") },
    { Pid::BARLINE_SPAN_FROM,                   P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "spanFromOffset",                  QT_TRANSLATE_NOOP("engraving/propertyName", "span from") },
    { Pid::BARLINE_SPAN_TO,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "spanToOffset",                    QT_TRANSLATE_NOOP("engraving/propertyName", "span to") },
    { Pid::BARLINE_SHOW_TIPS,                   P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "showTips",                        QT_TRANSLATE_NOOP("engraving/propertyName", "show tips") },

    { Pid::OFFSET,                              P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "offset",                          QT_TRANSLATE_NOOP("engraving/propertyName", "offset") },
    { Pid::FRET,                                P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "fret",                            QT_TRANSLATE_NOOP("engraving/propertyName", "fret") },
    { Pid::STRING,                              P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "string",                          QT_TRANSLATE_NOOP("engraving/propertyName", "string") },
    { Pid::GHOST,                               P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "ghost",                           QT_TRANSLATE_NOOP("engraving/propertyName", "ghost") },
    { Pid::DEAD,                                P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "dead",                            QT_TRANSLATE_NOOP("engraving/propertyName", "dead") },
    { Pid::PLAY,                                P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "play",                            QT_TRANSLATE_NOOP("engraving/propertyName", "played") },
    { Pid::TIMESIG_NOMINAL,                     P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, false, "timesigNominal",                  QT_TRANSLATE_NOOP("engraving/propertyName", "nominal time signature") },
    { Pid::TIMESIG_ACTUAL,                      P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, true,  "timesigActual",                   QT_TRANSLATE_NOOP("engraving/propertyName", "actual time signature") },
    { Pid::NUMBER_TYPE,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "numberType",                      QT_TRANSLATE_NOOP("engraving/propertyName", "number type") },
    { Pid::BRACKET_TYPE,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "bracketType",                     QT_TRANSLATE_NOOP("engraving/propertyName", "bracket type") },
    { Pid::NORMAL_NOTES,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "normalNotes",                     QT_TRANSLATE_NOOP("engraving/propertyName", "normal notes") },
    { Pid::ACTUAL_NOTES,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "actualNotes",                     QT_TRANSLATE_NOOP("engraving/propertyName", "actual notes") },
    { Pid::P1,                                  P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "p1",                              QT_TRANSLATE_NOOP("engraving/propertyName", "bracket start offset") },
    { Pid::P2,                                  P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "p2",                              QT_TRANSLATE_NOOP("engraving/propertyName", "bracket end offset") },
    { Pid::GROW_LEFT,                           P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "growLeft",                        QT_TRANSLATE_NOOP("engraving/propertyName", "grow left") },
    { Pid::GROW_RIGHT,                          P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "growRight",                       QT_TRANSLATE_NOOP("engraving/propertyName", "grow right") },

    { Pid::BOX_HEIGHT,                          P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "height",                          QT_TRANSLATE_NOOP("engraving/propertyName", "height") },
    { Pid::BOX_WIDTH,                           P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "width",                           QT_TRANSLATE_NOOP("engraving/propertyName", "width") },
    { Pid::BOX_AUTOSIZE,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "boxAutoSize",                     QT_TRANSLATE_NOOP("engraving/propertyName", "autosize frame") },
    { Pid::TOP_GAP,                             P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "topGap",                          QT_TRANSLATE_NOOP("engraving/propertyName", "top gap") },
    { Pid::BOTTOM_GAP,                          P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "bottomGap",                       QT_TRANSLATE_NOOP("engraving/propertyName", "bottom gap") },
    { Pid::LEFT_MARGIN,                         P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "leftMargin",                      QT_TRANSLATE_NOOP("engraving/propertyName", "left padding") },
    { Pid::RIGHT_MARGIN,                        P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "rightMargin",                     QT_TRANSLATE_NOOP("engraving/propertyName", "right padding") },
    { Pid::TOP_MARGIN,                          P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "topMargin",                       QT_TRANSLATE_NOOP("engraving/propertyName", "top padding") },
    { Pid::BOTTOM_MARGIN,                       P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "bottomMargin",                    QT_TRANSLATE_NOOP("engraving/propertyName", "bottom padding") },
    { Pid::PADDING_TO_NOTATION_ABOVE,           P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "paddingToNotationAbove",          QT_TRANSLATE_NOOP("engraving/propertyName", "padding to notation above") },
    { Pid::PADDING_TO_NOTATION_BELOW,           P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "paddingToNotationBelow",          QT_TRANSLATE_NOOP("engraving/propertyName", "padding to notation below") },

    { Pid::LAYOUT_BREAK,                        P_TYPE::LAYOUTBREAK_TYPE,          PropertyGroup::APPEARANCE, false, "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "layout break type") },
    { Pid::AUTOSCALE,                           P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "autoScale",                       QT_TRANSLATE_NOOP("engraving/propertyName", "autoscale") },
    { Pid::SIZE,                                P_TYPE::SIZE,                      PropertyGroup::APPEARANCE, false, "size",                            QT_TRANSLATE_NOOP("engraving/propertyName", "size") },

    { Pid::IMAGE_HEIGHT,                        P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "imageHeight",                     QT_TRANSLATE_NOOP("engraving/propertyName", "image height") },
    { Pid::IMAGE_WIDTH,                         P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "imageWidth",                      QT_TRANSLATE_NOOP("engraving/propertyName", "image width") },
    { Pid::IMAGE_FRAMED,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "imageFramed",                     QT_TRANSLATE_NOOP("engraving/propertyName", "image framed") },

    { Pid::FRET_FRAME_TEXT_SCALE,               P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "fretFrameTextScale",              QT_TRANSLATE_NOOP("engraving/propertyName", "text scale") },
    { Pid::FRET_FRAME_DIAGRAM_SCALE,            P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "fretFrameDiagramScale",           QT_TRANSLATE_NOOP("engraving/propertyName", "diagram scale") },
    { Pid::FRET_FRAME_COLUMN_GAP,               P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "fretFrameColumnGap",              QT_TRANSLATE_NOOP("engraving/propertyName", "column gap") },
    { Pid::FRET_FRAME_ROW_GAP,                  P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "fretFrameRowGap",                 QT_TRANSLATE_NOOP("engraving/propertyName", "row gap") },
    { Pid::FRET_FRAME_CHORDS_PER_ROW,           P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "fretFrameChordsPerRow",           QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbols per row") },
    { Pid::FRET_FRAME_H_ALIGN,                  P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "fretFrameHorizontalAlign",        QT_TRANSLATE_NOOP("engraving/propertyName", "horizontal alignment") },
    { Pid::FRET_FRAME_DIAGRAMS_ORDER,           P_TYPE::STRING,                    PropertyGroup::NONE,       false, "fretFrameDiagramsOrder",          QT_TRANSLATE_NOOP("engraving/propertyName", "diagrams order") },

    { Pid::SCALE,                               P_TYPE::SCALE,                     PropertyGroup::APPEARANCE, false, "scale",                           QT_TRANSLATE_NOOP("engraving/propertyName", "scale") },
    { Pid::LOCK_ASPECT_RATIO,                   P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "lockAspectRatio",                 QT_TRANSLATE_NOOP("engraving/propertyName", "aspect ratio locked") },
    { Pid::SIZE_IS_SPATIUM,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "sizeIsSpatium",                   QT_TRANSLATE_NOOP("engraving/propertyName", "size is spatium") },
    { Pid::TEXT,                                P_TYPE::STRING,                    PropertyGroup::TEXT,       false, "text",                            QT_TRANSLATE_NOOP("engraving/propertyName", "text") },
    { Pid::HTML_TEXT,                           P_TYPE::STRING,                    PropertyGroup::TEXT,       false, "",                                nullptr },
    { Pid::USER_MODIFIED,                       P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                nullptr },
    { Pid::BEAM_POS,                            P_TYPE::PAIR_REAL,                 PropertyGroup::POSITION,   false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "beam position") },
    { Pid::BEAM_MODE,                           P_TYPE::BEAM_MODE,                 PropertyGroup::APPEARANCE, true,  "BeamMode",                        QT_TRANSLATE_NOOP("engraving/propertyName", "beam mode") },
    { Pid::BEAM_NO_SLOPE,                       P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "noSlope",                         QT_TRANSLATE_NOOP("engraving/propertyName", "without slope") },
    { Pid::BEAM_CROSS_STAFF_MOVE,               P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "crossStaffMove",                  QT_TRANSLATE_NOOP("engraving/propertyName", "beam staff move") },
    { Pid::USER_LEN,                            P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "userLen",                         QT_TRANSLATE_NOOP("engraving/propertyName", "length") },
    { Pid::SHOW_STEM_SLASH,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "showStemSlash",                   QT_TRANSLATE_NOOP("engraving/propertyName", "show stem slash") },

    { Pid::SPACE,                               P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "space",                           QT_TRANSLATE_NOOP("engraving/propertyName", "space") },
    { Pid::TEMPO,                               P_TYPE::TEMPO,                     PropertyGroup::APPEARANCE, true,  "tempo",                           QT_TRANSLATE_NOOP("engraving/propertyName", "tempo") },
    { Pid::TEMPO_FOLLOW_TEXT,                   P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "followText",                      QT_TRANSLATE_NOOP("engraving/propertyName", "following text") },
    { Pid::TEMPO_ALIGN_RIGHT_OF_REHEARSAL_MARK, P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "tempoAlignRightOfRehearsalMark",  QT_TRANSLATE_NOOP("engraving/propertyName", "tempo align right of rehearsal mark") },

    { Pid::ACCIDENTAL_BRACKET,                  P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "bracket",                         QT_TRANSLATE_NOOP("engraving/propertyName", "bracket") },
    { Pid::ACCIDENTAL_TYPE,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "accidental type") },
    { Pid::ACCIDENTAL_STACKING_ORDER_OFFSET,    P_TYPE::INT,                       PropertyGroup::NONE,       true,  "stackingOrderOffset",             QT_TRANSLATE_NOOP("engraving/propertyName", "stacking order offset") },
    { Pid::NUMERATOR_STRING,                    P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "textN",                           QT_TRANSLATE_NOOP("engraving/propertyName", "numerator string") },
    { Pid::DENOMINATOR_STRING,                  P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "textD",                           QT_TRANSLATE_NOOP("engraving/propertyName", "denominator string") },
    { Pid::FBPREFIX,                            P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "prefix",                          QT_TRANSLATE_NOOP("engraving/propertyName", "prefix") },
    { Pid::FBDIGIT,                             P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "digit",                           QT_TRANSLATE_NOOP("engraving/propertyName", "digit") },
    { Pid::FBSUFFIX,                            P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "suffix",                          QT_TRANSLATE_NOOP("engraving/propertyName", "suffix") },
    { Pid::FBCONTINUATIONLINE,                  P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "continuationLine",                QT_TRANSLATE_NOOP("engraving/propertyName", "continuation line") },

    { Pid::FBPARENTHESIS1,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "",                                nullptr },
    { Pid::FBPARENTHESIS2,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "",                                nullptr },
    { Pid::FBPARENTHESIS3,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "",                                nullptr },
    { Pid::FBPARENTHESIS4,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "",                                nullptr },
    { Pid::FBPARENTHESIS5,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "",                                nullptr },

    { Pid::OTTAVA_TYPE,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "ottava type") },
    { Pid::NUMBERS_ONLY,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "numbersOnly",                     QT_TRANSLATE_NOOP("engraving/propertyName", "numbers only") },
    { Pid::TRILL_TYPE,                          P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "trill type") },
    { Pid::VIBRATO_TYPE,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "vibrato type") },
    { Pid::HAIRPIN_CIRCLEDTIP,                  P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "hairpinCircledTip",               QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin with circled tip") },

    { Pid::HAIRPIN_TYPE,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin type") },
    { Pid::HAIRPIN_HEIGHT,                      P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "hairpinHeight",                   QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin height") },
    { Pid::HAIRPIN_CONT_HEIGHT,                 P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "hairpinContHeight",               QT_TRANSLATE_NOOP("engraving/propertyName", "hairpin cont height") },
    { Pid::VELO_CHANGE,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "veloChange",                      QT_TRANSLATE_NOOP("engraving/propertyName", "velocity change") },
    { Pid::VELO_CHANGE_METHOD,                  P_TYPE::CHANGE_METHOD,             PropertyGroup::APPEARANCE, true,  "veloChangeMethod",                QT_TRANSLATE_NOOP("engraving/propertyName", "velocity change method") }, // left as a compatibility property - we need to be able to read it correctly
    { Pid::VELO_CHANGE_SPEED,                   P_TYPE::DYNAMIC_SPEED,             PropertyGroup::APPEARANCE, true,  "veloChangeSpeed",                 QT_TRANSLATE_NOOP("engraving/propertyName", "velocity change speed") },
    { Pid::DYNAMIC_TYPE,                        P_TYPE::DYNAMIC_TYPE,              PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "dynamic type") },

    { Pid::SINGLE_NOTE_DYNAMICS,                P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "singleNoteDynamics",              QT_TRANSLATE_NOOP("engraving/propertyName", "single note dynamics") },
    { Pid::CHANGE_METHOD,                       P_TYPE::CHANGE_METHOD,             PropertyGroup::APPEARANCE, true,  "changeMethod",                    QT_TRANSLATE_NOOP("engraving/propertyName", "change method") }, // the new, more general version of VELO_CHANGE_METHOD
    { Pid::PLACEMENT,                           P_TYPE::PLACEMENT_V,               PropertyGroup::POSITION,   false, "placement",                       QT_TRANSLATE_NOOP("engraving/propertyName", "placement") },
    { Pid::HPLACEMENT,                          P_TYPE::PLACEMENT_H,               PropertyGroup::POSITION,   false, "hplacement",                      QT_TRANSLATE_NOOP("engraving/propertyName", "horizontal placement") },
    { Pid::MMREST_RANGE_BRACKET_TYPE,           P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "mmrestRangeBracketType",          QT_TRANSLATE_NOOP("engraving/propertyName", "multimeasure rest range bracket type") },
    { Pid::VELOCITY,                            P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "velocity",                        QT_TRANSLATE_NOOP("engraving/propertyName", "velocity") },
    { Pid::JUMP_TO,                             P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "jumpTo",                          QT_TRANSLATE_NOOP("engraving/propertyName", "jump to") },
    { Pid::PLAY_UNTIL,                          P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "playUntil",                       QT_TRANSLATE_NOOP("engraving/propertyName", "play until") },
    { Pid::CONTINUE_AT,                         P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "continueAt",                      QT_TRANSLATE_NOOP("engraving/propertyName", "continue at") },
    { Pid::LABEL,                               P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "label",                           QT_TRANSLATE_NOOP("engraving/propertyName", "label") },
    { Pid::MARKER_TYPE,                         P_TYPE::MARKER_TYPE,               PropertyGroup::APPEARANCE, true,  "markerType",                      QT_TRANSLATE_NOOP("engraving/propertyName", "marker type") },
    { Pid::MUSIC_SYMBOL_SIZE,                   P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "musicSymbolSize",                 QT_TRANSLATE_NOOP("engraving/propertyName", "music symbol size") },
    { Pid::MARKER_CENTER_ON_SYMBOL,             P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "markerCenterOnSymbol",            QT_TRANSLATE_NOOP("engraving/propertyName", "marker center on symbol") },
    { Pid::ARP_USER_LEN1,                       P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "arpUserLen1",                     QT_TRANSLATE_NOOP("engraving/propertyName", "length 1") },
    { Pid::ARP_USER_LEN2,                       P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "arpUserLen2",                     QT_TRANSLATE_NOOP("engraving/propertyName", "length 2") },
    { Pid::REPEAT_END,                          P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "repeatEnd",                       QT_TRANSLATE_NOOP("engraving/propertyName", "repeat end") },
    { Pid::REPEAT_START,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "repeatStart",                     QT_TRANSLATE_NOOP("engraving/propertyName", "repeat start") },
    { Pid::REPEAT_JUMP,                         P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "repeatJump",                      QT_TRANSLATE_NOOP("engraving/propertyName", "repeat jump") },
    { Pid::MEASURE_NUMBER_MODE,                 P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "measureNumberMode",               QT_TRANSLATE_NOOP("engraving/propertyName", "measure number mode") },

    { Pid::GLISS_TYPE,                          P_TYPE::GLISS_TYPE,                PropertyGroup::APPEARANCE, false, "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "glissando type") },
    { Pid::GLISS_TEXT,                          P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "text",                            QT_TRANSLATE_NOOP("engraving/propertyName", "text") },
    { Pid::GLISS_SHOW_TEXT,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "glissandoShowText",               QT_TRANSLATE_NOOP("engraving/propertyName", "showing text") },
    { Pid::GLISS_STYLE,                         P_TYPE::GLISS_STYLE,               PropertyGroup::APPEARANCE, true,  "glissandoStyle",                  QT_TRANSLATE_NOOP("engraving/propertyName", "glissando style") },
    { Pid::GLISS_SHIFT,                         P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "glissandoShift",                  QT_TRANSLATE_NOOP("engraving/propertyName", "glissando shift") },
    { Pid::GLISS_EASEIN,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "easeInSpin",                      QT_TRANSLATE_NOOP("engraving/propertyName", "ease in") },
    { Pid::GLISS_EASEOUT,                       P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "easeOutSpin",                     QT_TRANSLATE_NOOP("engraving/propertyName", "ease out") },
    { Pid::DIAGONAL,                            P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "diagonal",                        QT_TRANSLATE_NOOP("engraving/propertyName", "diagonal") },
    { Pid::GROUP_NODES,                         P_TYPE::GROUPS,                    PropertyGroup::NONE,       false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "groups") },
    { Pid::LINE_STYLE,                          P_TYPE::LINE_TYPE,                 PropertyGroup::APPEARANCE, true,  "lineStyle",                       QT_TRANSLATE_NOOP("engraving/propertyName", "line style") },
    { Pid::LINE_WIDTH,                          P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "lineWidth",                       QT_TRANSLATE_NOOP("engraving/propertyName", "line width") },
    { Pid::TIME_STRETCH,                        P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "timeStretch",                     QT_TRANSLATE_NOOP("engraving/propertyName", "time stretch") },
    { Pid::ORNAMENT_STYLE,                      P_TYPE::ORNAMENT_STYLE,            PropertyGroup::APPEARANCE, true,  "ornamentStyle",                   QT_TRANSLATE_NOOP("engraving/propertyName", "ornament style") },
    { Pid::INTERVAL_ABOVE,                      P_TYPE::ORNAMENT_INTERVAL,         PropertyGroup::APPEARANCE, true,  "intervalAbove",                   QT_TRANSLATE_NOOP("engraving/propertyName", "interval above") },
    { Pid::INTERVAL_BELOW,                      P_TYPE::ORNAMENT_INTERVAL,         PropertyGroup::APPEARANCE, true,  "intervalBelow",                   QT_TRANSLATE_NOOP("engraving/propertyName", "interval below") },
    { Pid::ORNAMENT_SHOW_ACCIDENTAL,            P_TYPE::ORNAMENT_SHOW_ACCIDENTAL,  PropertyGroup::APPEARANCE, true,  "ornamentShowAccidental",          QT_TRANSLATE_NOOP("engraving/propertyName", "ornament show accidental") },
    { Pid::ORNAMENT_SHOW_CUE_NOTE,              P_TYPE::AUTO_ON_OFF,               PropertyGroup::APPEARANCE, true,  "ornamentShowCueNote",             QT_TRANSLATE_NOOP("engraving/propertyName", "ornament show cue note") },
    { Pid::START_ON_UPPER_NOTE,                 P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "startOnUpperNote",                QT_TRANSLATE_NOOP("engraving/propertyName", "start on upper note") },

    { Pid::TIMESIG,                             P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, false, "timesig",                         QT_TRANSLATE_NOOP("engraving/propertyName", "time signature") },
    { Pid::TIMESIG_STRETCH,                     P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "time signature stretch") },
    { Pid::TIMESIG_TYPE,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "time signature type") },
    { Pid::SPANNER_TICK,                        P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, true,  "tick",                            QT_TRANSLATE_NOOP("engraving/propertyName", "tick") },
    { Pid::SPANNER_TICKS,                       P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, true,  "ticks",                           QT_TRANSLATE_NOOP("engraving/propertyName", "ticks") },
    { Pid::SPANNER_TRACK2,                      P_TYPE::SIZE_T,                    PropertyGroup::NONE,       false, "track2",                          QT_TRANSLATE_NOOP("engraving/propertyName", "end track") },
    { Pid::OFFSET2,                             P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "userOff2",                        QT_TRANSLATE_NOOP("engraving/propertyName", "end offset") },
    { Pid::BREAK_MMR,                           P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "breakMultiMeasureRest",           QT_TRANSLATE_NOOP("engraving/propertyName", "breaking multimeasure rest") },
    { Pid::MMREST_NUMBER_POS,                   P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "mmRestNumberPos",                 QT_TRANSLATE_NOOP("engraving/propertyName", "vertical position of multimeasure rest number") }, // Deprecated
    { Pid::MMREST_NUMBER_OFFSET,                P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "mmRestNumberOffset",              QT_TRANSLATE_NOOP("engraving/propertyName", "vertical offset of multimeasure rest number") },
    { Pid::MMREST_NUMBER_VISIBLE,               P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "mmRestNumberVisible",             QT_TRANSLATE_NOOP("engraving/propertyName", "visibility of multimeasure rest number") },

    { Pid::MEASURE_REPEAT_NUMBER_POS,           P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "measureRepeatNumberPos",          QT_TRANSLATE_NOOP("engraving/propertyName", "vertical position of measure repeat number") },
    { Pid::REPEAT_COUNT,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "repeatCount",                     QT_TRANSLATE_NOOP("engraving/propertyName", "repeat count") },

    { Pid::USER_STRETCH,                        P_TYPE::REAL,                      PropertyGroup::NONE,       false, "stretch",                         QT_TRANSLATE_NOOP("engraving/propertyName", "stretch") },
    { Pid::NO_OFFSET,                           P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "noOffset",                        QT_TRANSLATE_NOOP("engraving/propertyName", "numbering offset") },
    { Pid::IRREGULAR,                           P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "irregular",                       QT_TRANSLATE_NOOP("engraving/propertyName", "irregular") },
    { Pid::ANCHOR,                              P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "anchor",                          QT_TRANSLATE_NOOP("engraving/propertyName", "anchor") },
    { Pid::SLUR_UOFF1,                          P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "o1",                              QT_TRANSLATE_NOOP("engraving/propertyName", "start offset") },
    { Pid::SLUR_UOFF2,                          P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "o2",                              QT_TRANSLATE_NOOP("engraving/propertyName", "left shoulder offset") },
    { Pid::SLUR_UOFF3,                          P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "o3",                              QT_TRANSLATE_NOOP("engraving/propertyName", "right shoulder offset") },
    { Pid::SLUR_UOFF4,                          P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "o4",                              QT_TRANSLATE_NOOP("engraving/propertyName", "end offset") },
    { Pid::STAFF_MOVE,                          P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "staffMove",                       QT_TRANSLATE_NOOP("engraving/propertyName", "staff move") },
    { Pid::VERSE,                               P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "no",                              QT_TRANSLATE_NOOP("engraving/propertyName", "verse") },

    { Pid::SYLLABIC,                            P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "syllabic",                        QT_TRANSLATE_NOOP("engraving/propertyName", "syllabic") },
    { Pid::LYRIC_TICKS,                         P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, true,  "ticks_f",                         QT_TRANSLATE_NOOP("engraving/propertyName", "ticks") },
    { Pid::VOLTA_ENDING,                        P_TYPE::INT_VEC,                   PropertyGroup::APPEARANCE, true,  "endings",                         QT_TRANSLATE_NOOP("engraving/propertyName", "endings") },
    { Pid::LINE_VISIBLE,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "lineVisible",                     QT_TRANSLATE_NOOP("engraving/propertyName", "visible line") },
    { Pid::MAG,                                 P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "mag",                             QT_TRANSLATE_NOOP("engraving/propertyName", "magnification") },
    { Pid::USE_DRUMSET,                         P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "useDrumset",                      QT_TRANSLATE_NOOP("engraving/propertyName", "using drumset") },
    { Pid::DURATION,                            P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, true,  "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "duration") },
    { Pid::DURATION_TYPE_WITH_DOTS,             P_TYPE::DURATION_TYPE_WITH_DOTS,   PropertyGroup::APPEARANCE, true,  "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "duration type") },
    { Pid::ACCIDENTAL_ROLE,                     P_TYPE::ACCIDENTAL_ROLE,           PropertyGroup::APPEARANCE, false, "role",                            QT_TRANSLATE_NOOP("engraving/propertyName", "role") },
    { Pid::TRACK,                               P_TYPE::SIZE_T,                    PropertyGroup::NONE,       false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "track") },

    { Pid::FRET_STRINGS,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "strings",                         QT_TRANSLATE_NOOP("engraving/propertyName", "strings") },
    { Pid::FRET_FRETS,                          P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "frets",                           QT_TRANSLATE_NOOP("engraving/propertyName", "frets") },
    { Pid::FRET_NUT,                            P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "showNut",                         QT_TRANSLATE_NOOP("engraving/propertyName", "show nut") },
    { Pid::FRET_OFFSET,                         P_TYPE::INT,                       PropertyGroup::POSITION,   true,  "fretOffset",                      QT_TRANSLATE_NOOP("engraving/propertyName", "fret offset") },
    { Pid::FRET_NUM_POS,                        P_TYPE::INT,                       PropertyGroup::POSITION,   true,  "fretNumPos",                      QT_TRANSLATE_NOOP("engraving/propertyName", "fret number position") },
    { Pid::ORIENTATION,                         P_TYPE::ORIENTATION,               PropertyGroup::APPEARANCE, true,  "orientation",                     QT_TRANSLATE_NOOP("engraving/propertyName", "orientation") },
    { Pid::FRET_SHOW_FINGERINGS,                P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "fretShowFingering",               QT_TRANSLATE_NOOP("engraving/propertyName", "fretboard diagram fingering visible") },
    { Pid::FRET_FINGERING,                      P_TYPE::INT_VEC,                   PropertyGroup::APPEARANCE, true,  "fretFingering",                   QT_TRANSLATE_NOOP("engraving/propertyName", "fretboard diagram fingering") },

    { Pid::HARMONY_VOICE_LITERAL,               P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "harmonyVoiceLiteral",             QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol interpretation") },
    { Pid::HARMONY_VOICING,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "harmonyVoicing",                  QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol voicing") },
    { Pid::HARMONY_DURATION,                    P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "harmonyDuration",                 QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol duration") },
    { Pid::HARMONY_BASS_SCALE,                  P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "harmonyBassScale",                QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol bass scale") },
    { Pid::HARMONY_DO_NOT_STACK_MODIFIERS,      P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "harmonyDoNotStackModifiers",      QT_TRANSLATE_NOOP("engraving/propertyName", "chord symbol do not stack modifiers") },

    { Pid::SYSTEM_BRACKET,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "type",                            QT_TRANSLATE_NOOP("engraving/propertyName", "type") },
    { Pid::GAP,                                 P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "gap") },
    { Pid::AUTOPLACE,                           P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "autoplace",                       QT_TRANSLATE_NOOP("engraving/propertyName", "autoplace") },
    { Pid::DASH_LINE_LEN,                       P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "dashLineLength",                  QT_TRANSLATE_NOOP("engraving/propertyName", "dash line length") },
    { Pid::DASH_GAP_LEN,                        P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "dashGapLength",                   QT_TRANSLATE_NOOP("engraving/propertyName", "dash gap length") },
    { Pid::TICK,                                P_TYPE::FRACTION,                  PropertyGroup::NONE,       false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "tick") },
    { Pid::PLAYBACK_VOICE1,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "playbackVoice1",                  QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 1") },
    { Pid::PLAYBACK_VOICE2,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "playbackVoice2",                  QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 2") },
    { Pid::PLAYBACK_VOICE3,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "playbackVoice3",                  QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 3") },

    { Pid::PLAYBACK_VOICE4,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "playbackVoice4",                  QT_TRANSLATE_NOOP("engraving/propertyName", "playback voice 4") },
    { Pid::SYMBOL,                              P_TYPE::SYMID,                     PropertyGroup::APPEARANCE, true,  "symbol",                          QT_TRANSLATE_NOOP("engraving/propertyName", "symbol") },
    { Pid::PLAY_REPEATS,                        P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "playRepeats",                     QT_TRANSLATE_NOOP("engraving/propertyName", "playing repeats") },
    { Pid::CREATE_SYSTEM_HEADER,                P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "createSystemHeader",              QT_TRANSLATE_NOOP("engraving/propertyName", "creating system header") },
    { Pid::STAFF_LINES,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "lines",                           QT_TRANSLATE_NOOP("engraving/propertyName", "lines") },
    { Pid::LINE_DISTANCE,                       P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, true,  "lineDistance",                    QT_TRANSLATE_NOOP("engraving/propertyName", "line distance") },
    { Pid::STEP_OFFSET,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "stepOffset",                      QT_TRANSLATE_NOOP("engraving/propertyName", "step offset") },
    { Pid::STAFF_SHOW_BARLINES,                 P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "showing barlines") },
    { Pid::STAFF_SHOW_LEDGERLINES,              P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "showing ledgerlines") },
    { Pid::STAFF_STEMLESS,                      P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "stemless") },
    { Pid::STAFF_INVISIBLE,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "invisible") },
    { Pid::STAFF_COLOR,                         P_TYPE::COLOR,                     PropertyGroup::APPEARANCE, false, "color",                           QT_TRANSLATE_NOOP("engraving/propertyName", "color") },

    { Pid::HEAD_SCHEME,                         P_TYPE::NOTEHEAD_SCHEME,           PropertyGroup::APPEARANCE, false, "headScheme",                      QT_TRANSLATE_NOOP("engraving/propertyName", "notehead scheme") },
    { Pid::STAFF_GEN_CLEF,                      P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "generating clefs") },
    { Pid::STAFF_GEN_TIMESIG,                   P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "generating time signature") },
    { Pid::STAFF_GEN_KEYSIG,                    P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "generating key signature") },
    { Pid::STAFF_YOFFSET,                       P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "",                                QT_TRANSLATE_NOOP("engraving/propertyName", "y-offset") },
    { Pid::STAFF_USERDIST,                      P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "distOffset",                      QT_TRANSLATE_NOOP("engraving/propertyName", "distance offset") },
    { Pid::STAFF_BARLINE_SPAN,                  P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "barLineSpan",                     QT_TRANSLATE_NOOP("engraving/propertyName", "barline span") },
    { Pid::STAFF_BARLINE_SPAN_FROM,             P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "barLineSpanFrom",                 QT_TRANSLATE_NOOP("engraving/propertyName", "barline span from") },
    { Pid::STAFF_BARLINE_SPAN_TO,               P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "barLineSpanTo",                   QT_TRANSLATE_NOOP("engraving/propertyName", "barline span to") },
    { Pid::BRACKET_SPAN,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "bracketSpan",                     QT_TRANSLATE_NOOP("engraving/propertyName", "bracket span") },

    { Pid::BRACKET_COLUMN,                      P_TYPE::SIZE_T,                    PropertyGroup::APPEARANCE, false, "level",                           QT_TRANSLATE_NOOP("engraving/propertyName", "level") },
    { Pid::INAME_LAYOUT_POSITION,               P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "layoutPosition",                  QT_TRANSLATE_NOOP("engraving/propertyName", "layout position") },
    { Pid::TEXT_STYLE,                          P_TYPE::TEXT_STYLE,                PropertyGroup::TEXT,       false, "style",                           QT_TRANSLATE_NOOP("engraving/propertyName", "style") },
    { Pid::FONT_FACE,                           P_TYPE::STRING,                    PropertyGroup::TEXT,       false, "family",                          QT_TRANSLATE_NOOP("engraving/propertyName", "family") },
    { Pid::FONT_SIZE,                           P_TYPE::REAL,                      PropertyGroup::TEXT,       false, "size",                            QT_TRANSLATE_NOOP("engraving/propertyName", "size") },
    { Pid::FONT_STYLE,                          P_TYPE::INT,                       PropertyGroup::TEXT,       false, "fontStyle",                       QT_TRANSLATE_NOOP("engraving/propertyName", "font style") },
    { Pid::TEXT_LINE_SPACING,                   P_TYPE::REAL,                      PropertyGroup::TEXT,       false, "textLineSpacing",                 QT_TRANSLATE_NOOP("engraving/propertyName", "user line distancing") },

    { Pid::FRAME_TYPE,                          P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "frameType",                       QT_TRANSLATE_NOOP("engraving/propertyName", "frame type") },
    { Pid::FRAME_WIDTH,                         P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "frameWidth",                      QT_TRANSLATE_NOOP("engraving/propertyName", "frame width") },
    { Pid::FRAME_PADDING,                       P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "framePadding",                    QT_TRANSLATE_NOOP("engraving/propertyName", "frame padding") },
    { Pid::FRAME_ROUND,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "frameRound",                      QT_TRANSLATE_NOOP("engraving/propertyName", "frame round") },
    { Pid::FRAME_FG_COLOR,                      P_TYPE::COLOR,                     PropertyGroup::APPEARANCE, false, "frameFgColor",                    QT_TRANSLATE_NOOP("engraving/propertyName", "frame foreground color") },
    { Pid::FRAME_BG_COLOR,                      P_TYPE::COLOR,                     PropertyGroup::APPEARANCE, false, "frameBgColor",                    QT_TRANSLATE_NOOP("engraving/propertyName", "frame background color") },
    { Pid::SIZE_SPATIUM_DEPENDENT,              P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "sizeIsSpatiumDependent",          QT_TRANSLATE_NOOP("engraving/propertyName", "spatium dependent size") },
    { Pid::TEXT_SIZE_SPATIUM_DEPENDENT,         P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "textSizeIsSpatiumDependent",      QT_TRANSLATE_NOOP("engraving/propertyName", "spatium dependent text size") },
    { Pid::MUSICAL_SYMBOLS_SCALE,               P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "musicalSymbolsScale",             QT_TRANSLATE_NOOP("engraving/propertyName", "musical symbols scale") },
    { Pid::ALIGN,                               P_TYPE::ALIGN,                     PropertyGroup::POSITION,   false, "align",                           QT_TRANSLATE_NOOP("engraving/propertyName", "align") },
    { Pid::TEXT_SCRIPT_ALIGN,                   P_TYPE::INT,                       PropertyGroup::POSITION,   false, "align",                           QT_TRANSLATE_NOOP("engraving/propertyName", "text script align") },
    { Pid::SYSTEM_FLAG,                         P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "systemFlag",                      QT_TRANSLATE_NOOP("engraving/propertyName", "system flag") },

    { Pid::BEGIN_TEXT,                          P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "beginText",                       QT_TRANSLATE_NOOP("engraving/propertyName", "begin text") },
    { Pid::BEGIN_TEXT_ALIGN,                    P_TYPE::ALIGN,                     PropertyGroup::APPEARANCE, false, "beginTextAlign",                  QT_TRANSLATE_NOOP("engraving/propertyName", "begin text align") },
    { Pid::BEGIN_TEXT_POSITION,                 P_TYPE::ALIGN_H,                   PropertyGroup::APPEARANCE, false, "beginTextPosition",               QT_TRANSLATE_NOOP("engraving/propertyName", "begin text position") },
    { Pid::BEGIN_TEXT_PLACE,                    P_TYPE::TEXT_PLACE,                PropertyGroup::APPEARANCE, false, "beginTextPlace",                  QT_TRANSLATE_NOOP("engraving/propertyName", "begin text place") },
    { Pid::BEGIN_HOOK_TYPE,                     P_TYPE::HOOK_TYPE,                 PropertyGroup::APPEARANCE, true,  "beginHookType",                   QT_TRANSLATE_NOOP("engraving/propertyName", "begin hook type") },
    { Pid::BEGIN_HOOK_HEIGHT,                   P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "beginHookHeight",                 QT_TRANSLATE_NOOP("engraving/propertyName", "begin hook height") },
    { Pid::BEGIN_FONT_FACE,                     P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "beginFontFace",                   QT_TRANSLATE_NOOP("engraving/propertyName", "begin font face") },
    { Pid::BEGIN_FONT_SIZE,                     P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "beginFontSize",                   QT_TRANSLATE_NOOP("engraving/propertyName", "begin font size") },
    { Pid::BEGIN_FONT_STYLE,                    P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "beginFontStyle",                  QT_TRANSLATE_NOOP("engraving/propertyName", "begin font style") },
    { Pid::BEGIN_TEXT_OFFSET,                   P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "beginTextOffset",                 QT_TRANSLATE_NOOP("engraving/propertyName", "begin text offset") },
    { Pid::GAP_BETWEEN_TEXT_AND_LINE,           P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "gapBetweenTextAndLine",           QT_TRANSLATE_NOOP("engraving/propertyName", "gap between text and line") },

    { Pid::CONTINUE_TEXT,                       P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "continueText",                    QT_TRANSLATE_NOOP("engraving/propertyName", "continue text") },
    { Pid::CONTINUE_TEXT_ALIGN,                 P_TYPE::ALIGN,                     PropertyGroup::APPEARANCE, false, "continueTextAlign",               QT_TRANSLATE_NOOP("engraving/propertyName", "continue text align") },
    { Pid::CONTINUE_TEXT_POSITION,              P_TYPE::ALIGN_H,                   PropertyGroup::APPEARANCE, false, "continueTextPosition",            QT_TRANSLATE_NOOP("engraving/propertyName", "continue text position") },
    { Pid::CONTINUE_TEXT_PLACE,                 P_TYPE::TEXT_PLACE,                PropertyGroup::APPEARANCE, false, "continueTextPlace",               QT_TRANSLATE_NOOP("engraving/propertyName", "continue text place") },
    { Pid::CONTINUE_FONT_FACE,                  P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "continueFontFace",                QT_TRANSLATE_NOOP("engraving/propertyName", "continue font face") },
    { Pid::CONTINUE_FONT_SIZE,                  P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "continueFontSize",                QT_TRANSLATE_NOOP("engraving/propertyName", "continue font size") },
    { Pid::CONTINUE_FONT_STYLE,                 P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "continueFontStyle",               QT_TRANSLATE_NOOP("engraving/propertyName", "continue font style") },
    { Pid::CONTINUE_TEXT_OFFSET,                P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "continueTextOffset",              QT_TRANSLATE_NOOP("engraving/propertyName", "continue text offset") },

    { Pid::END_TEXT,                            P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "endText",                         QT_TRANSLATE_NOOP("engraving/propertyName", "end text") },
    { Pid::END_TEXT_ALIGN,                      P_TYPE::ALIGN,                     PropertyGroup::APPEARANCE, false, "endTextAlign",                    QT_TRANSLATE_NOOP("engraving/propertyName", "end text align") },
    { Pid::END_TEXT_POSITION,                   P_TYPE::ALIGN_H,                   PropertyGroup::APPEARANCE, false, "endTextPosition",                 QT_TRANSLATE_NOOP("engraving/propertyName", "end text position") },
    { Pid::END_TEXT_PLACE,                      P_TYPE::TEXT_PLACE,                PropertyGroup::APPEARANCE, false, "endTextPlace",                    QT_TRANSLATE_NOOP("engraving/propertyName", "end text place") },
    { Pid::END_HOOK_TYPE,                       P_TYPE::HOOK_TYPE,                 PropertyGroup::APPEARANCE, true,  "endHookType",                     QT_TRANSLATE_NOOP("engraving/propertyName", "end hook type") },
    { Pid::END_HOOK_HEIGHT,                     P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, false, "endHookHeight",                   QT_TRANSLATE_NOOP("engraving/propertyName", "end hook height") },
    { Pid::END_FONT_FACE,                       P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "endFontFace",                     QT_TRANSLATE_NOOP("engraving/propertyName", "end font face") },
    { Pid::END_FONT_SIZE,                       P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "endFontSize",                     QT_TRANSLATE_NOOP("engraving/propertyName", "end font size") },
    { Pid::END_FONT_STYLE,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "endFontStyle",                    QT_TRANSLATE_NOOP("engraving/propertyName", "end font style") },
    { Pid::END_TEXT_OFFSET,                     P_TYPE::POINT,                     PropertyGroup::POSITION,   false, "endTextOffset",                   QT_TRANSLATE_NOOP("engraving/propertyName", "end text offset") },

    { Pid::NOTELINE_PLACEMENT,                  P_TYPE::NOTELINE_PLACEMENT_TYPE,   PropertyGroup::APPEARANCE, false, "noteLinePlacement",               QT_TRANSLATE_NOOP("engraving/propertyName", "note-anchored line placement") },

    { Pid::AVOID_BARLINES,                      P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "avoidBarLines",                   QT_TRANSLATE_NOOP("engraving/propertyName", "avoid barlines") },
    { Pid::DYNAMICS_SIZE,                       P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "dynamicsSize",                    QT_TRANSLATE_NOOP("engraving/propertyName", "dynamic size") },
    { Pid::CENTER_ON_NOTEHEAD,                  P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "centerOnNotehead",                QT_TRANSLATE_NOOP("engraving/propertyName", "use text alignment") },
    { Pid::ANCHOR_TO_END_OF_PREVIOUS,           P_TYPE::BOOL,                      PropertyGroup::NONE,       true, "anchorToEndOfPrevious",            QT_TRANSLATE_NOOP("engraving/propertyName", "anchor to end of previous") },

    { Pid::SNAP_TO_DYNAMICS,                    P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "snapToDynamics",                  QT_TRANSLATE_NOOP("engraving/propertyName", "snap expression") }, // for expressions
    { Pid::SNAP_BEFORE,                         P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "snapBefore",                      QT_TRANSLATE_NOOP("engraving/propertyName", "snap before") },     // <
    { Pid::SNAP_AFTER,                          P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "snapAfter",                       QT_TRANSLATE_NOOP("engraving/propertyName", "snap after") },      // < for hairpins

    { Pid::VOICE_ASSIGNMENT,                    P_TYPE::VOICE_ASSIGNMENT,          PropertyGroup::NONE,       true,  "voiceAssignment",                 QT_TRANSLATE_NOOP("engraving/propertyName", "voice assignment") },
    { Pid::CENTER_BETWEEN_STAVES,               P_TYPE::AUTO_ON_OFF,               PropertyGroup::POSITION,   false, "centerBetweenStaves",             QT_TRANSLATE_NOOP("engraving/propertyName", "center between staves") },

    { Pid::POS_ABOVE,                           P_TYPE::MILLIMETRE,                PropertyGroup::POSITION,   false, "posAbove",                        QT_TRANSLATE_NOOP("engraving/propertyName", "position above") },

    { Pid::LOCATION_STAVES,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "staves",                          QT_TRANSLATE_NOOP("engraving/propertyName", "staves distance") },
    { Pid::LOCATION_VOICES,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "voices",                          QT_TRANSLATE_NOOP("engraving/propertyName", "voices distance") },
    { Pid::LOCATION_MEASURES,                   P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "measures",                        QT_TRANSLATE_NOOP("engraving/propertyName", "measures distance") },
    { Pid::LOCATION_FRACTIONS,                  P_TYPE::FRACTION,                  PropertyGroup::APPEARANCE, false, "fractions",                       QT_TRANSLATE_NOOP("engraving/propertyName", "position distance") },
    { Pid::LOCATION_GRACE,                      P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "grace",                           QT_TRANSLATE_NOOP("engraving/propertyName", "grace note index") },
    { Pid::LOCATION_NOTE,                       P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "note",                            QT_TRANSLATE_NOOP("engraving/propertyName", "note index") },

    { Pid::VOICE,                               P_TYPE::SIZE_T,                    PropertyGroup::NONE,       true,  "voice",                           QT_TRANSLATE_NOOP("engraving/propertyName", "voice") },
    { Pid::POSITION,                            P_TYPE::ALIGN_H,                   PropertyGroup::POSITION,   false, "position",                        QT_TRANSLATE_NOOP("engraving/propertyName", "position") },

    { Pid::CLEF_TYPE_CONCERT,                   P_TYPE::CLEF_TYPE,                 PropertyGroup::APPEARANCE, true,  "concertClefType",                 QT_TRANSLATE_NOOP("engraving/propertyName", "concert clef type") },
    { Pid::CLEF_TYPE_TRANSPOSING,               P_TYPE::CLEF_TYPE,                 PropertyGroup::APPEARANCE, true,  "transposingClefType",             QT_TRANSLATE_NOOP("engraving/propertyName", "transposing clef type") },
    { Pid::CLEF_TO_BARLINE_POS,                 P_TYPE::CLEF_TO_BARLINE_POS,       PropertyGroup::APPEARANCE, true,  "clefToBarlinePos",                QT_TRANSLATE_NOOP("engraving/propertyName", "clef to barline position") },
    { Pid::IS_HEADER,                           P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "isHeader",                        QT_TRANSLATE_NOOP("engraving/propertyName", "is header") },
    { Pid::KEY_CONCERT,                         P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "concertKey",                      QT_TRANSLATE_NOOP("engraving/propertyName", "concert key") },
    { Pid::KEY,                                 P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "actualKey",                       QT_TRANSLATE_NOOP("engraving/propertyName", "key") },
    { Pid::ACTION,                              P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "action",                          nullptr },
    { Pid::MIN_DISTANCE,                        P_TYPE::SPATIUM,                   PropertyGroup::POSITION,   false, "minDistance",                     QT_TRANSLATE_NOOP("engraving/propertyName", "autoplace minimum distance") },

    { Pid::ARPEGGIO_TYPE,                       P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "arpeggio type") },
    { Pid::CHORD_LINE_TYPE,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "chord line type") },
    { Pid::CHORD_LINE_STRAIGHT,                 P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "straight",                        QT_TRANSLATE_NOOP("engraving/propertyName", "straight chord line") },
    { Pid::CHORD_LINE_WAVY,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "wavy",                            QT_TRANSLATE_NOOP("engraving/propertyName", "wavy chord line") },
    { Pid::TREMOLO_TYPE,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "subtype",                         QT_TRANSLATE_NOOP("engraving/propertyName", "tremolo type") },
    { Pid::TREMOLO_STYLE,                       P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "strokeStyle",                     QT_TRANSLATE_NOOP("engraving/propertyName", "tremolo style") },
    { Pid::HARMONY_TYPE,                        P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "harmonyType",                     QT_TRANSLATE_NOOP("engraving/propertyName", "harmony type") },

    { Pid::ARPEGGIO_SPAN,                       P_TYPE::INT,                       PropertyGroup::NONE,       true,  "arpeggioSpan",                    QT_TRANSLATE_NOOP("engraving/propertyName", "arpeggio span") },

    { Pid::BEND_TYPE,                           P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "bendType",                        QT_TRANSLATE_NOOP("engraving/propertyName", "bend type") },
    { Pid::BEND_CURVE,                          P_TYPE::PITCH_VALUES,              PropertyGroup::APPEARANCE, true,  "bendCurve",                       QT_TRANSLATE_NOOP("engraving/propertyName", "bend curve") },
    { Pid::BEND_VERTEX_OFF,                     P_TYPE::POINT,                     PropertyGroup::POSITION  , false, "bendVertexOffset",                QT_TRANSLATE_NOOP("engraving/propertyName", "bend vertex offset") },
    { Pid::BEND_SHOW_HOLD_LINE,                 P_TYPE::INT,                       PropertyGroup::APPEARANCE, false, "bendShowHoldLine",                QT_TRANSLATE_NOOP("engraving/propertyName", "bend show hold line") },
    { Pid::BEND_START_TIME_FACTOR,              P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "bendStartTimeFactor",             QT_TRANSLATE_NOOP("engraving/propertyName", "bend start time factor") },
    { Pid::BEND_END_TIME_FACTOR,                P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "bendEndTimeFactor",               QT_TRANSLATE_NOOP("engraving/propertyName", "bend end time factor") },

    { Pid::TREMOLOBAR_TYPE,                     P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "tremoloBarType",                  QT_TRANSLATE_NOOP("engraving/propertyName", "tremolobar type") },
    { Pid::TREMOLOBAR_CURVE,                    P_TYPE::PITCH_VALUES,              PropertyGroup::APPEARANCE, true,  "tremoloBarCurve",                 QT_TRANSLATE_NOOP("engraving/propertyName", "tremolobar curve") },

    { Pid::START_WITH_LONG_NAMES,               P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "startWithLongNames",              QT_TRANSLATE_NOOP("engraving/propertyName", "start with long names") },
    { Pid::START_WITH_MEASURE_ONE,              P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "startWithMeasureOne",             QT_TRANSLATE_NOOP("engraving/propertyName", "start with measure one") },
    { Pid::FIRST_SYSTEM_INDENTATION,            P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "firstSystemIndentation",          QT_TRANSLATE_NOOP("engraving/propertyName", "first system indentation") },

    { Pid::PATH,                                P_TYPE::DRAW_PATH,                 PropertyGroup::APPEARANCE, false, "path",                            QT_TRANSLATE_NOOP("engraving/propertyName", "path") },

    { Pid::PREFER_SHARP_FLAT,                   P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "preferSharpFlat",                 QT_TRANSLATE_NOOP("engraving/propertyName", "prefer sharps or flats") },

    { Pid::PLAY_TECH_TYPE,                      P_TYPE::PLAYTECH_TYPE,             PropertyGroup::APPEARANCE, true,  "playTechType",                    QT_TRANSLATE_NOOP("engraving/propertyName", "playing technique type") },

    { Pid::TEMPO_CHANGE_TYPE,                   P_TYPE::TEMPOCHANGE_TYPE,          PropertyGroup::APPEARANCE, true,  "tempoChangeType",                 QT_TRANSLATE_NOOP("engraving/propertyName", "gradual tempo change type") },
    { Pid::TEMPO_EASING_METHOD,                 P_TYPE::CHANGE_METHOD,             PropertyGroup::APPEARANCE, true,  "tempoEasingMethod",               QT_TRANSLATE_NOOP("engraving/propertyName", "tempo easing method") },
    { Pid::TEMPO_CHANGE_FACTOR,                 P_TYPE::REAL,                      PropertyGroup::APPEARANCE, true,  "tempoChangeFactor",               QT_TRANSLATE_NOOP("engraving/propertyName", "tempo change factor") },

    { Pid::HARP_IS_DIAGRAM,                     P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, false, "isDiagram",                       QT_TRANSLATE_NOOP("engraving/propertyName", "is diagram") },

    { Pid::ACTIVE,                              P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "active",                          QT_TRANSLATE_NOOP("engraving/propertyName", "active") },

    { Pid::CAPO_FRET_POSITION,                  P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "fretPosition",                    QT_TRANSLATE_NOOP("engraving/propertyName", "fret position") },
    { Pid::CAPO_IGNORED_STRINGS,                P_TYPE::INT_VEC,                   PropertyGroup::APPEARANCE, true,  "ignoredStrings",                  QT_TRANSLATE_NOOP("engraving/propertyName", "ignored strings") },
    { Pid::CAPO_GENERATE_TEXT,                  P_TYPE::BOOL,                      PropertyGroup::APPEARANCE, true,  "generateText",                    QT_TRANSLATE_NOOP("engraving/propertyName", "automatically generate text") },

    { Pid::TIE_PLACEMENT,                       P_TYPE::TIE_PLACEMENT,             PropertyGroup::APPEARANCE, true,  "tiePlacement",                    QT_TRANSLATE_NOOP("engraving/propertyName", "tie placement") },
    { Pid::MIN_LENGTH,                          P_TYPE::SPATIUM,                   PropertyGroup::APPEARANCE, true,  "minLength",                       QT_TRANSLATE_NOOP("engraving/propertyName", "minimum length") },
    { Pid::PARTIAL_SPANNER_DIRECTION,           P_TYPE::PARTIAL_SPANNER_DIRECTION, PropertyGroup::NONE,       true,  "partialSpannerDirection",         QT_TRANSLATE_NOOP("engraving/propertyName", "partial spanner direction") },

    { Pid::POSITION_LINKED_TO_MASTER,           P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "positionLinkedToMaster",          QT_TRANSLATE_NOOP("engraving/propertyName", "position linked to main score") },
    { Pid::APPEARANCE_LINKED_TO_MASTER,         P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "appearanceLinkedToMaster",        QT_TRANSLATE_NOOP("engraving/propertyName", "appearance linked to main score") },
    { Pid::TEXT_LINKED_TO_MASTER,               P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "textLinkedToMaster",              QT_TRANSLATE_NOOP("engraving/propertyName", "text linked to main score") },
    { Pid::EXCLUDE_FROM_OTHER_PARTS,            P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "excludeFromParts",                QT_TRANSLATE_NOOP("engraving/propertyName", "exclude from parts") },

    { Pid::STRINGTUNINGS_STRINGS_COUNT,         P_TYPE::INT,                       PropertyGroup::APPEARANCE, true,  "stringsCount",                    QT_TRANSLATE_NOOP("engraving/propertyName", "strings count") },
    { Pid::STRINGTUNINGS_PRESET,                P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "preset",                          QT_TRANSLATE_NOOP("engraving/propertyName", "strings preset") },
    { Pid::STRINGTUNINGS_VISIBLE_STRINGS,       P_TYPE::INT_VEC,                   PropertyGroup::APPEARANCE, true,  "visibleStrings",                  QT_TRANSLATE_NOOP("engraving/propertyName", "visible strings") },

    { Pid::SCORE_FONT,                          P_TYPE::STRING,                    PropertyGroup::APPEARANCE, true,  "scoreFont",                       QT_TRANSLATE_NOOP("engraving/propertyName", "score font") },
    { Pid::SYMBOLS_SIZE,                        P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "symbolsSize",                     QT_TRANSLATE_NOOP("engraving/propertyName", "symbols size") },
    { Pid::SYMBOL_ANGLE,                        P_TYPE::REAL,                      PropertyGroup::APPEARANCE, false, "symbolAngle",                     QT_TRANSLATE_NOOP("engraving/propertyName", "symbol angle") },

    { Pid::APPLY_TO_ALL_STAVES,                 P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "applyToAllStaves",                QT_TRANSLATE_NOOP("engraving/propertyName", "apply to all staves") },
    { Pid::IS_COURTESY,                         P_TYPE::BOOL,                      PropertyGroup::NONE,       false, "isCourtesy",                      QT_TRANSLATE_NOOP("engraving/propertyName", "is courtesy") },
    { Pid::EXCLUDE_VERTICAL_ALIGN,              P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "excludeVerticalAlign",            QT_TRANSLATE_NOOP("engraving/propertyName", "exclude vertical align") },

    { Pid::SHOW_MEASURE_NUMBERS,                P_TYPE::AUTO_ON_OFF,               PropertyGroup::NONE,       false, "showMeasureNumbers",              QT_TRANSLATE_NOOP("engraving/propertyName", "show measure numbers") },
    { Pid::PLAY_COUNT_TEXT_SETTING,             P_TYPE::AUTO_CUSTOM_HIDE,          PropertyGroup::APPEARANCE, false, "playCountTextSetting",            QT_TRANSLATE_NOOP("engraving/propertyName", "play count text setting") },
    { Pid::PLAY_COUNT_TEXT,                     P_TYPE::STRING,                    PropertyGroup::APPEARANCE, false, "playCountCustomText",             QT_TRANSLATE_NOOP("engraving/propertyName", "play count text") },

    { Pid::ALIGN_WITH_OTHER_RESTS,              P_TYPE::BOOL,                      PropertyGroup::POSITION,   false, "alignWithOtherRests",             QT_TRANSLATE_NOOP("engraving/propertyName", "align with other rests in the same voice") },

    { Pid::END,                                 P_TYPE::INT,                       PropertyGroup::NONE,       false, "++end++",                         nullptr }
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
