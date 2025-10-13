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

#include "scoreelement.h"

#include <QQmlListProperty>

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/notedot.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/page.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/stemslash.h"
#include "engraving/dom/system.h"
#include "engraving/dom/systemdivider.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/accidental.h"

#include "engraving/editing/undo.h"

#include "playevent.h"

Q_MOC_INCLUDE("engraving/api/v1/part.h")

namespace mu::engraving::apiv1 {
class FractionWrapper;
class IntervalWrapper;
class EngravingItem;
class Part;
class Spanner;
class Staff;
class System;
class Tie;
class Tuplet;
class Measure;

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

extern EngravingItem* wrap(mu::engraving::EngravingItem* se, Ownership own = Ownership::SCORE);

#define API_PROPERTY(name, pid) \
    Q_PROPERTY(QVariant name READ get_##name WRITE set_##name RESET reset_##name) \
    QVariant get_##name() const { return get(mu::engraving::Pid::pid); }  \
    void set_##name(QVariant val) { set(mu::engraving::Pid::pid, val); }  \
    void reset_##name() { reset(mu::engraving::Pid::pid); }

/**
 * API_PROPERTY flavor which allows to define the property type.
 * Can be used if it is known that this property is always valid
 * for this type, otherwise this macro won't allow an `undefined`
 * value to be exposed to QML in case of invalid property.
 */
#define API_PROPERTY_T(type, name, pid) \
    Q_PROPERTY(type name READ get_##name WRITE set_##name RESET reset_##name) \
    type get_##name() const { return get(mu::engraving::Pid::pid).value<type>(); }  \
    void set_##name(type val) { set(mu::engraving::Pid::pid, QVariant::fromValue(val)); }  \
    void reset_##name() { reset(mu::engraving::Pid::pid); }

#define API_PROPERTY_READ_ONLY(name, pid) \
    Q_PROPERTY(QVariant name READ get_##name) \
    QVariant get_##name() const { return get(mu::engraving::Pid::pid); }

#define API_PROPERTY_READ_ONLY_T(type, name, pid) \
    Q_PROPERTY(type name READ get_##name) \
    type get_##name() const { return get(mu::engraving::Pid::pid).value<type>(); }  \

//---------------------------------------------------------
//   EngravingItem
//    EngravingItem wrapper
//---------------------------------------------------------

class EngravingItem : public apiv1::ScoreElement
{
    Q_OBJECT

    /// Parent element for this element.
    /// \since 3.3
    Q_PROPERTY(apiv1::EngravingItem * parent READ parent)
    /// Staff which this element belongs to.
    /// \since MuseScore 3.5
    Q_PROPERTY(apiv1::Staff * staff READ staff)
    /// X-axis offset from a reference position in spatium units.
    /// \see EngravingItem::offset
    Q_PROPERTY(qreal offsetX READ offsetX WRITE setOffsetX)
    /// Y-axis offset from a reference position in spatium units.
    /// \see EngravingItem::offset
    Q_PROPERTY(qreal offsetY READ offsetY WRITE setOffsetY)
    /// Reference position of this element relative to its parent element.
    ///
    /// This is an offset from the parent object that is determined by the
    /// autoplace feature. It includes any other offsets applied to the
    /// element. You can use this value to accurately position other elements
    /// related to the same parent.
    ///
    /// This value is in spatium units for compatibility with EngravingItem.offsetX.
    /// \since MuseScore 3.3
    Q_PROPERTY(qreal posX READ posX)
    /// Reference position of this element relative to its parent element.
    ///
    /// This is an offset from the parent object that is determined by the
    /// autoplace feature. It includes any other offsets applied to the
    /// element. You can use this value to accurately position other elements
    /// related to the same parent.
    ///
    /// This value is in spatium units for compatibility with EngravingItem.offsetY.
    /// \since MuseScore 3.3
    Q_PROPERTY(qreal posY READ posY)
    /// Reference position of this element relative to its parent element, in spatium units.
    /// Use `pos.x` or `pos.y` to access the X and Y components of this point.
    /// \see EngravingItem::posX
    /// \see EngravingItem::posY
    /// \since MuseScore 4.6
    Q_PROPERTY(QPointF pos READ pos)
    /// Position of this element in page coordinates, in spatium units.
    /// \since MuseScore 3.5
    Q_PROPERTY(QPointF pagePos READ pagePos)
    /// Position of this element relative to the canvas (user interface), in spatium units.
    /// \since MuseScore 4.6
    Q_PROPERTY(QPointF canvasPos READ canvasPos)

    /// Bounding box of this element.
    ///
    /// This value is in spatium units for compatibility with other EngravingItem positioning properties.
    /// \since MuseScore 3.3.1
    Q_PROPERTY(QRectF bbox READ bbox)

    /// Subtype of this element.
    /// \since MuseScore 4.6
    Q_PROPERTY(int subtype READ subtype)

    /// Staff index for this element.
    /// \since MuseScore 4.6
    Q_PROPERTY(int staffIdx READ staffIdx)
    /// Effective staff index for this element. Used by system objects,
    /// as they may not always appear at their staffIdx
    /// \since MuseScore 4.6
    Q_PROPERTY(int effectiveStaffIdx READ effectiveStaffIdx)
    /// Staff index for this element, accounting for cross-staffing.
    /// \since MuseScore 4.6
    Q_PROPERTY(int vStaffIdx READ vStaffIdx)

    /// If the element points upwards.
    /// Valid for: Chords, stems, beams, ties, slurs,
    /// guitar bends, tuplets, tremolos, articulations
    /// \since MuseScore 4.6
    Q_PROPERTY(bool up READ up)

    /// The header element flag.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool header READ header)
    /// The trailer element flag.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool trailer READ trailer)
    /// The isMovable element flag.
    /// Controls whether this element can be dragged by the mouse.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isMovable READ isMovable)
    /// The enabled element flag.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool enabled READ enabled)
    /// Whether this element is accounted for in layout calculations.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool addToSkyline READ addToSkyline)

    /// Unlike the name might suggest, this property no longer returns the subtype and is scarcely used.
    /// Named 'subtype' prior to MuseScore 4.6
    API_PROPERTY(subType,                 SUBTYPE)
    API_PROPERTY_READ_ONLY_T(bool, selected, SELECTED)
    API_PROPERTY_READ_ONLY_T(bool, generated, GENERATED)
    /// EngravingItem color. See https://doc.qt.io/qt-5/qml-color.html
    /// for the reference on color type in QML.
    API_PROPERTY_T(QColor, color,         COLOR)
    API_PROPERTY_T(bool,   visible,       VISIBLE)
    /// Stacking order of this element
    API_PROPERTY_T(int,    z,             Z)
    /// Whether this element is cue size.
    API_PROPERTY_T(bool,   small,         SMALL)
    /// For staves and parts: Whether to hide systems when they are empty.
    /// One of PluginAPI::PluginAPI::AutoOnOff values.
    /// \since MuseScore 4.6
    API_PROPERTY(hideWhenEmpty,           HIDE_WHEN_EMPTY)
    /// For parts: Whether to only hide staves on a system if the entire instrument is empty.
    /// \since MuseScore 4.6
    API_PROPERTY_T(bool, hideStavesWhenIndividuallyEmpty, HIDE_STAVES_WHEN_INDIVIDUALLY_EMPTY)
    /// For clefs, key signatures, time signatures and
    /// system breaks: Whether to generate courtesy objects.
    API_PROPERTY(showCourtesy,            SHOW_COURTESY)
    /// For key signatures: The key signature mode.
    /// One of PluginAPI::PluginAPI::KeyMode values.
    ///\since MuseScore 4.6
    API_PROPERTY(keysig_mode,             KEYSIG_MODE)
    /// For slurs & ties: The line style of the slur /tie.
    /// One of PluginAPI::PluginAPI::SlurStyleType values.
    API_PROPERTY(lineType,                SLUR_STYLE_TYPE)

    /// Notehead type, one of PluginAPI::PluginAPI::NoteHeadType values
    API_PROPERTY(headType,                HEAD_TYPE)
    /// Notehead group, one of PluginAPI::PluginAPI::NoteHeadGroup values
    API_PROPERTY(headGroup,               HEAD_GROUP)
    API_PROPERTY(articulationAnchor,      ARTICULATION_ANCHOR)

    /// The direction of this element,
    /// one of PluginAPI::PluginAPI::Direction values.
    API_PROPERTY(direction,               DIRECTION)
    /// For parentheses: The horizontal direction.
    /// One of PluginAPI::PluginAPI::DirectionH values.
    ///\since MuseScore 4.6
    API_PROPERTY(horizontalDirection,     HORIZONTAL_DIRECTION)
    /// For chords, stems, beams and two-chord tremolos: The stem direction.
    /// One of PluginAPI::PluginAPI::Direction values.
    API_PROPERTY(stemDirection,           STEM_DIRECTION)
    /// For chords, stems, beams and two-chord tremolos: The stem direction.
    /// One of PluginAPI::PluginAPI::Direction values.
    API_PROPERTY(slurDirection,           SLUR_DIRECTION)
    /// For notes: The horizontal direction of the notehead.
    /// One of PluginAPI::PluginAPI::DirectionH values.
    ///\since MuseScore 4.6
    API_PROPERTY(mirrorHead,              MIRROR_HEAD)
    ///\since MuseScore 4.6
    API_PROPERTY(hasParentheses, HAS_PARENTHESES)
    /// For breath marks and section breaks: The amount to
    /// pause playback by, in seconds.
    API_PROPERTY_T(qreal,  pause,         PAUSE)

    /// For barlines: The barline type.
    API_PROPERTY(barlineType,             BARLINE_TYPE)
    /// For barlines: Whether the barline spans to the next stave.
    API_PROPERTY(barlineSpan,             BARLINE_SPAN)
    /// For barlines: The offset of the start of this barline from the
    /// default position. Measured in steps equivalent to half a spatium.
    API_PROPERTY_T(int, barlineSpanFrom,  BARLINE_SPAN_FROM)
    /// For barlines: The offset of the end of this barline from the
    /// default position. Measured in steps equivalent to half a spatium.
    API_PROPERTY_T(int, barlineSpanTo,    BARLINE_SPAN_TO)
    /// For repeat barlines: Whether they display winged tips.
    /// \note this property controls the global style setting \p repeatBarTips
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, barlineShowTips, BARLINE_SHOW_TIPS)

    /// Offset from a reference position in spatium units.
    /// Use `Qt.point(x, y)` to create a point value which can be
    /// assigned to this property.
    /// \see EngravingItem::offsetX
    /// \see EngravingItem::offsetY
    API_PROPERTY_T(QPointF, offset,       OFFSET)
    API_PROPERTY_T(bool, ghost,           GHOST)
    API_PROPERTY_T(bool, play,            PLAY)
    /// For beams: The feathering on its left side.
    API_PROPERTY_T(qreal, growLeft,       GROW_LEFT)
    /// For beams: The feathering on its right side.
    API_PROPERTY_T(qreal, growRight,      GROW_RIGHT)

    /// For vertical frames and text frames: Their height.
    API_PROPERTY(boxHeight,               BOX_HEIGHT)
    /// For horizontal frames: Its width.
    API_PROPERTY(boxWidth,                BOX_WIDTH)
    /// Whether frames should autosize themselves to their
    /// contents, rather than use their set height / width.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, boxAutoSize,     BOX_AUTOSIZE)

    /// The top padding for a given frame. Affects the
    /// positioning of its contents.
    API_PROPERTY(topGap,                  TOP_GAP)
    /// The bottom padding for a given frame.
    API_PROPERTY(bottomGap,               BOTTOM_GAP)
    /// The left padding for a given frame.
    API_PROPERTY_T(qreal, leftMargin,     LEFT_MARGIN)
    /// The right padding for a given frame.
    API_PROPERTY_T(qreal, rightMargin,    RIGHT_MARGIN)
    /// The top gap for a given frame. Affects the positioning
    /// of the frame relative to surrounding elements.
    API_PROPERTY_T(qreal, topMargin,      TOP_MARGIN)
    /// The bottom gap for a given frame.
    API_PROPERTY_T(qreal, bottomMargin,   BOTTOM_MARGIN)
    /// For vertical-type frames: The clearance for notation above the frame.
    /// \since MuseScore 4.6
    API_PROPERTY(paddingToNotationAbove,  PADDING_TO_NOTATION_ABOVE)
    /// For vertical-type frames: The clearance for notation below the frame.
    /// \since MuseScore 4.6
    API_PROPERTY(paddingToNotationBelow,  PADDING_TO_NOTATION_BELOW)
    /// For layout breaks: The layout break type.
    /// One of PluginAPI::PluginAPI::LayoutBreakType values
    API_PROPERTY(layoutBreakType,         LAYOUT_BREAK)
    /// For images attached to frames: Whether they
    /// scale themselves to the frame's size.
    API_PROPERTY_T(bool, autoscale,       AUTOSCALE)
    /// For images: The size of an image.
    API_PROPERTY(size,                    SIZE)

    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, imageHeight,    IMAGE_HEIGHT)
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, imageWidth,     IMAGE_WIDTH)
    /// For images: Whether this image is part of a frame.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, imageFramed,     IMAGE_FRAMED)

    /// For fretboard diagram legends: The text (chord symbols) scale.
    API_PROPERTY_T(qreal, fretFrameTextScale, FRET_FRAME_TEXT_SCALE)
    /// For fretboard diagram legends: The fretboard diagram scale.
    API_PROPERTY_T(qreal, fretFrameDiagramScale, FRET_FRAME_DIAGRAM_SCALE)
    /// For fretboard diagram legends: The gap between columns, in spatiums.
    API_PROPERTY(fretFrameColumnGap,      FRET_FRAME_COLUMN_GAP)
    /// For fretboard diagram legends: The gap between rows, in spatiums.
    API_PROPERTY(fretFrameRowGap,         FRET_FRAME_ROW_GAP)
    /// For fretboard diagram legends: The number of chords per row.
    API_PROPERTY_T(int, fretFrameChordPerRow, FRET_FRAME_CHORDS_PER_ROW)
    /// For fretboard diagram legends: The horizontal alignment of its contents.
    API_PROPERTY_T(int, fretFrameHAlign,  FRET_FRAME_H_ALIGN)
    /// For fretboard diagram legends: The order the diagrams are displayed in
    API_PROPERTY(fretFrameDiagramsOrder,  FRET_FRAME_DIAGRAMS_ORDER)

    /// For time signatures: Their scale.
    API_PROPERTY(scale,                   SCALE)
    /// For images: Whether the aspect ratio is locked.
    API_PROPERTY_T(bool, lockAspectRatio, LOCK_ASPECT_RATIO)
    /// For images: Whether their size is measured in spatiums.
    API_PROPERTY_T(bool, sizeIsSpatium,   SIZE_IS_SPATIUM)
    /// For text based elements.
    API_PROPERTY(text,                    TEXT)
    ///\since MuseScore 4.6
    API_PROPERTY(htmlText,                HTML_TEXT)
    /// For beams and two-chord tremolos: Whether their
    /// position has been modified by the user.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, userModified,    USER_MODIFIED)
    /// For beams and two-chord tremolos: Their position.
    API_PROPERTY(beamPos,                 BEAM_POS)
    /// For beams and two-chord tremolos: Whether they are forcibly horizontal.
    API_PROPERTY_T(bool, beamNoSlope,     BEAM_NO_SLOPE)
    /// For cross-staff beams: Their positioning relative to the staves.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, crossStaffMove,   BEAM_CROSS_STAFF_MOVE)
    /// For stems: The user-added length offset.
    API_PROPERTY(userLen,                 USER_LEN)

    /// For spacers: amount of space between staves.
    API_PROPERTY(space,                   SPACE)
    /// For tempo text: The tempo
    API_PROPERTY(tempo,                   TEMPO)
    /// For tempo text: Whether the tempo follows the written value.
    API_PROPERTY_T(bool, tempoFollowText, TEMPO_FOLLOW_TEXT)
    /// For tempo text: Whether the tempo is aligned right of rehearsal marks.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, tempoAlignRightOfRehearsalMark, TEMPO_ALIGN_RIGHT_OF_REHEARSAL_MARK)
    /// For accidentals: Controls what type of brackets to show.
    /// One of PluginAPI::PluginAPI::AccidentalBracket values
    API_PROPERTY_T(int, accidentalBracket, ACCIDENTAL_BRACKET)
    /// For accidentals: Controls the type of accidental.
    /// One of PluginAPI::PluginAPI::AccidentalType values
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, accidentalType,   ACCIDENTAL_TYPE)
    /// For accidentals: The vertical order relative to other accidentals.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, stackingOrderOffset, ACCIDENTAL_STACKING_ORDER_OFFSET)
    /// For time signatures: Custom numerator text to override the default.
    API_PROPERTY(numeratorString,         NUMERATOR_STRING)
    /// For time signatures: Custom denominator text to override the default.
    API_PROPERTY(denominatorString,       DENOMINATOR_STRING)
    API_PROPERTY_T(int, fbprefix,         FBPREFIX)
    API_PROPERTY_T(int, fbdigit,          FBDIGIT)
    API_PROPERTY_T(int, fbsuffix,         FBSUFFIX)
    API_PROPERTY_T(int, fbcontinuationline, FBCONTINUATIONLINE)

    ///\since MuseScore 4.6
    API_PROPERTY_T(int, fbparenthesis1,   FBPARENTHESIS1)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, fbparenthesis2,   FBPARENTHESIS2)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, fbparenthesis3,   FBPARENTHESIS3)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, fbparenthesis4,   FBPARENTHESIS4)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, fbparenthesis5,   FBPARENTHESIS5)

    /// For ottavas: Their type, one of PluginAPI::PluginAPI::OttavaType values.
    API_PROPERTY_T(int, ottavaType,       OTTAVA_TYPE)
    /// For ottavas: Whether they display numbers or numbers and text.
    API_PROPERTY_T(bool, numbersOnly,     NUMBERS_ONLY)
    /// For trills: Their type, one of PluginAPI::PluginAPI::TrillType.
    API_PROPERTY_T(int, trillType,        TRILL_TYPE)
    /// For vibratos: Their type, one of PluginAPI::PluginAPI::VibratoType.
    API_PROPERTY_T(int, vibratoType,      VIBRATO_TYPE)
    /// For hairpins: Whether their tip is circled.
    API_PROPERTY_T(bool, hairpinCircledTip, HAIRPIN_CIRCLEDTIP)

    /// For hairpins: Their type, one of PluginAPI::PluginAPI::HairpinType values.
    API_PROPERTY_T(int, hairpinType,      HAIRPIN_TYPE)
    /// For hairpins: The height of the hairpin.
    API_PROPERTY(hairpinHeight,           HAIRPIN_HEIGHT)
    /// For hairpins: The height of the hairpin when continuing onto a new system.
    API_PROPERTY(hairpinContHeight,       HAIRPIN_CONT_HEIGHT)
    /// For hairpins: The velocity change.
    API_PROPERTY_T(int, veloChange,       VELO_CHANGE)
    /// The way a hairpin interpolates between values.
    /// one of PluginAPI::PluginAPI::ChangeMethod values
    API_PROPERTY(veloChangeMethod,        VELO_CHANGE_METHOD)
    /// For dynamics such as fp: How fast to change dynamics.
    API_PROPERTY(veloChangeSpeed,         VELO_CHANGE_SPEED)
    /// For dynamics: Their type, one of PluginAPI::PluginAPI::DynamicType values.
    ///\since MuseScore 4.6
    API_PROPERTY(dynamicType,             DYNAMIC_TYPE)

    /// For hairpins: Whether to use single note dynamics
    /// (not change dynamic during individual notes).
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, singleNoteDynamics, SINGLE_NOTE_DYNAMICS)
    ///    The way a ramp interpolates between values.
    ///    \since MuseScore 3.5
    API_PROPERTY(changeMethod,            CHANGE_METHOD)
    /// For text-based elements: The text placement,
    /// one of PluginAPI::PluginAPI::Placement values.
    API_PROPERTY(placement,               PLACEMENT)
    /// For multimeasure rest ranges and measure numbers:
    /// The horizontal placement, one of PluginAPI::PluginAPI::PlacementH values.
    ///\since MuseScore 4.6
    API_PROPERTY(hPlacement,              HPLACEMENT)
    /// For multimeasure rest ranges: The bracket type,
    /// one of PluginAPI::PluginAPI::MMRestRangeBracketType values.
    API_PROPERTY_T(int, mmRestRangeBracketType, MMREST_RANGE_BRACKET_TYPE)
    /// For dynamics: Their velocity
    API_PROPERTY_T(int, velocity,         VELOCITY)
    /// For jumps: Which marker to jump to.
    API_PROPERTY(jumpTo,                  JUMP_TO)
    /// For jumps: Which marker to play until.
    API_PROPERTY(playUntil,               PLAY_UNTIL)
    /// For jumps: Which marker to continue at.
    API_PROPERTY(continueAt,              CONTINUE_AT)
    /// For markers: Their label, used by jumps.
    API_PROPERTY(label,                   LABEL)
    /// For markers: The marker type,
    /// one of PluginAPI::PluginAPI::MarkerType values.
    API_PROPERTY_T(int, markerType,       MARKER_TYPE)
    /// The size of musical symbols used.
    /// \since MuseScore 4.6
    API_PROPERTY_T(qreal, musicSymbolSize, MUSIC_SYMBOL_SIZE)
    /// For markers: Whether the symbol is aligned with the barline.
    /// \since MuseScore 4.6
    API_PROPERTY_T(bool, markerCenterOnSymbol, MARKER_CENTER_ON_SYMBOL)
    /// For arpeggios: The position of the top handle.
    API_PROPERTY_T(qreal, arpUserLen1,    ARP_USER_LEN1)
    /// For arpeggios: The position of the bottom handle.
    API_PROPERTY_T(qreal, arpUserLen2,    ARP_USER_LEN2)

    /// For glissandos: Their type,
    /// one of PluginAPI::PluginAPI::GlissandoType values.
    API_PROPERTY_T(int, glissType,        GLISS_TYPE)
    /// For glissandos: The text they display.
    API_PROPERTY(glissText,               GLISS_TEXT)
    /// For glissandos: Whether they display text.
    API_PROPERTY_T(bool, glissShowText,   GLISS_SHOW_TEXT)
    /// For glissandos: The play style,
    /// one of PluginAPI::PluginAPI::GlissandoStyle values.
    API_PROPERTY(glissandoStyle,          GLISS_STYLE)
    /// For glissandos: The amount to ease in by on playback.
    API_PROPERTY_T(int, glissEaseIn,      GLISS_EASEIN)
    /// For glissandos: The amount to ease out by on playback.
    API_PROPERTY_T(int, glissEaseOut,     GLISS_EASEOUT)
    /// For lines: Whether they can be diagonal,
    /// as opposed to locked horizontal.
    API_PROPERTY_T(bool, diagonal,        DIAGONAL)
    /// For time signatures: Their rhythmic groups.
    API_PROPERTY(groups,                  GROUP_NODES)
    /// For lines: Their line style,
    /// one of PluginAPI::PluginAPI::LineType values.
    API_PROPERTY(lineStyle,               LINE_STYLE)
    /// For lines: The color of the line.
    API_PROPERTY(lineColor,               COLOR)
    /// For lines: The width of the line.
    API_PROPERTY(lineWidth,               LINE_WIDTH)
    /// For fermatas and arpeggios: How much to stretch
    /// their playback by.
    API_PROPERTY_T(qreal, timeStretch,    TIME_STRETCH)
    ///For trills and articulation: The ornament style,
    /// one of PluginAPI::PluginAPI::OrnamentStyle values
    API_PROPERTY(ornamentStyle,           ORNAMENT_STYLE)
    /// For ornaments: Their interval above. To check if
    /// this ornament actually has an interval above,
    /// use ornament.hasIntervalAbove
    ///\since MuseScore 4.6
    API_PROPERTY(intervalAbove,           INTERVAL_ABOVE)
    /// For ornaments: Their interval below. To check if
    /// this ornament actually has an interval above,
    /// use ornament.hasIntervalBelow
    ///\since MuseScore 4.6
    API_PROPERTY(intervalBelow,           INTERVAL_BELOW)
    /// For ornaments: Controls whether this ornament shows an accidental,
    /// one of PluginAPI::PluginAPI::OrnamentShowAccidental values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, ornamentShowAccidental, ORNAMENT_SHOW_ACCIDENTAL)
    /// For ornaments: Controls whether this ornament shows a cue note,
    /// one of PluginAPI::PluginAPI::AutoOnOff values.
    ///\since MuseScore 4.6
    API_PROPERTY(ornamentShowCueNote,     ORNAMENT_SHOW_CUE_NOTE)
    /// Whether this ornament starts on the upper note.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, startOnUpperNote, START_ON_UPPER_NOTE)

    /// For time signatures: The time signature as a fraction.
    API_PROPERTY(timesig,                 TIMESIG)
    /// For local time signatures: The ratio between the
    /// local time signature and the global time signature.
    API_PROPERTY(timesigStretch,          TIMESIG_STRETCH)
    /// For time signatures:  The time signature type,
    /// one of PluginAPI::PluginAPI::TimeSigType values.
    API_PROPERTY_T(int, timesigType,      TIMESIG_TYPE)
    ///\since MuseScore 4.6
    API_PROPERTY(mmRestNumberPos,         MMREST_NUMBER_POS)
    /// For multimeasure rests: The offset of the number.
    ///\since MuseScore 4.6
    API_PROPERTY(mmRestNumberOffset,      MMREST_NUMBER_OFFSET)
    /// For multimeasure rests: Whether the number is visible
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, mmRestNumberVisible, MMREST_NUMBER_VISIBLE)

    /// For measure repeats: The position of the number.
    ///\since MuseScore 4.6
    API_PROPERTY(measureRepeatNumberPos,  MEASURE_REPEAT_NUMBER_POS)

    API_PROPERTY_T(int, verse,            VERSE)

    /// For lyrics: The syllabic, one of
    /// PluginAPI::PluginAPI::Syllabic values.
    API_PROPERTY_T(int, syllabic,         SYLLABIC)
    /// The tick length of a lyrics object.
    API_PROPERTY(lyricTicks,              LYRIC_TICKS)
    /// For voltas: The list of passes on which to play the underlying measures.
    API_PROPERTY(volta_ending,            VOLTA_ENDING)
    /// For line elements: Controls the visibility of the line.
    API_PROPERTY_T(bool, lineVisible,     LINE_VISIBLE)
    /// Controls the scale of various elements.
    API_PROPERTY_T(qreal, mag,            MAG)
    /// For parts: Whether this part uses a drumset.
    API_PROPERTY_T(int, useDrumset,       USE_DRUMSET)
    /// For accidentals: Whether this accidental was added by
    /// the user or generated by the score layout, one of
    /// PluginAPI::PluginAPI::AccidentalRole values
    API_PROPERTY(role,                    ACCIDENTAL_ROLE)
    /// The track of a given element.
    API_PROPERTY_T(int, track,            TRACK) // does conversion work from P_TYPE::SIZE_T ?

    /// For fretboard diagrams: The number of strings to display.
    API_PROPERTY_T(int, fretStrings,      FRET_STRINGS)
    /// For fretboard diagrams: The number of frets to display.
    API_PROPERTY_T(int, fretFrets,        FRET_FRETS)
    /// For fretboard diagrams: Whether to show the nut.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, showNut,         FRET_NUT)
    /// For fretboard diagrams: The fret number to start the diagram from.
    API_PROPERTY_T(int, fretOffset,       FRET_OFFSET)
    /// For fretboard diagrams: The position of the fret number.
    API_PROPERTY_T(int, fretNumPos,       FRET_NUM_POS)
    /// For fretboard diagrams: The orientation, one of
    /// PluginAPI::PluginAPI::Orientation values
    ///\since MuseScore 4.6
    API_PROPERTY(orientation,             ORIENTATION)
    /// For fretboard diagrams: Whether fingerings are displayed.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, fretShowFingering, FRET_SHOW_FINGERINGS)
    /// For fretboard diagrams: The fingering data.
    ///\since MuseScore 4.6
    API_PROPERTY(fretFingering,           FRET_FINGERING)

    /// Whether this chord symbol is voiced literally.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, harmonyVoiceLiteral, HARMONY_VOICE_LITERAL)
    /// The voicing for this chord symbol, one of
    /// PluginAPI::PluginAPI::HarmonyVoicing values.
    ///\since MuseScore 4.6
    API_PROPERTY(harmonyVoicing,          HARMONY_VOICING)
    /// The duration to play this chord symbol for, one of
    /// PluginAPI::PluginAPI::HDuration values.
    ///\since MuseScore 4.6
    API_PROPERTY(harmonyDuration,         HARMONY_DURATION)
    /// The scale of the bass component of a chord symbol.
    ///\since MuseScore 4.6
    API_PROPERTY(harmonyBassScale,        HARMONY_BASS_SCALE)
    /// Whether to stack modifiers in chord symbols.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, harmonyDoNotStackModifiers, HARMONY_DO_NOT_STACK_MODIFIERS)

    /// For brackets: The bracket type, one of
    /// PluginAPI::PluginAPI::BracketType values.
    API_PROPERTY_T(int, systemBracket,    SYSTEM_BRACKET)
    /// For rests in voice 2+: Controls whether they are
    /// displayed in the score.
    API_PROPERTY_T(bool, gap,             GAP)
    /// Whether this element participates in autoplacement
    API_PROPERTY_T(bool, autoplace,       AUTOPLACE)
    /// For lines: The length of the dashes for dashed lines,
    /// in multiples of the line width.
    API_PROPERTY_T(qreal, dashLineLen,    DASH_LINE_LEN)
    /// For lines: The length of the gaps between dashes
    /// for dashed lines, in multiples of the line width.
    API_PROPERTY_T(qreal, dashGapLen,     DASH_GAP_LEN)
//       API_PROPERTY_READ_ONLY( tick,          TICK                      ) // wasn't available in 2.X, disabled due to fractions transition
    /// Symbol ID of this element (if appropriate),
    /// one of PluginAPI::PluginAPI::SymId values.
    /// Valid for symbols, articulation, fermatas and breaths.
    API_PROPERTY(symbol,                  SYMBOL)
    /// For jumps: Whether to play repeats.
    API_PROPERTY_T(bool, playRepeats,     PLAY_REPEATS)
    /// For horizontal frames: Whether to generate brackets,
    /// clefs, and key signatures in the next measure.
    API_PROPERTY_T(bool, createSystemHeader, CREATE_SYSTEM_HEADER)
    /// For stafftype changes: The new number of lines for the staff.
    API_PROPERTY_T(int, staffLines,       STAFF_LINES)
    /// For stafftype changes: The new distance between lines of the staff.
    API_PROPERTY(lineDistance,            LINE_DISTANCE)
    /// For stafftype changes: The new offset for staff contents, in half spaces.
    API_PROPERTY_T(int, stepOffset,       STEP_OFFSET)
    /// For stafftype changes: Whether the staff displays barlines.
    API_PROPERTY_T(bool, staffShowBarlines, STAFF_SHOW_BARLINES)
    /// For stafftype changes: Whether the staff displays ledger lines.
    API_PROPERTY_T(bool, staffShowLedgerlines, STAFF_SHOW_LEDGERLINES)
    /// For stafftype changes: Whether notes on the staff are stemless.
    API_PROPERTY_T(bool, staffStemless,   STAFF_STEMLESS)
    /// For staves and stafftype changes: Whether the staff lines are invisible.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, staffInvisible,  STAFF_INVISIBLE)
    /// For staves and stafftype changes: The color of the staff lines.
    ///\since MuseScore 4.6
    API_PROPERTY_T(QColor, staffColor,    STAFF_COLOR)

    /// Notehead scheme, one of PluginAPI::PluginAPI::NoteHeadScheme values.
    /// \since MuseScore 3.5
    API_PROPERTY(headScheme,              HEAD_SCHEME)
    /// For stafftype changes: Whether the staff displays clefs.
    API_PROPERTY_T(bool, staffGenClef,    STAFF_GEN_CLEF)
    /// For stafftype changes: Whether the staff displays time signatures.
    API_PROPERTY_T(bool, staffGenTimesig, STAFF_GEN_TIMESIG)
    /// For stafftype changes: Whether the staff displays key signatures.
    API_PROPERTY_T(bool, staffGenKeysig,  STAFF_GEN_KEYSIG)
    /// For stafftype changes: The vertical offset relative to the previous stafftype.
    API_PROPERTY(staffYoffset,            STAFF_YOFFSET)
    /// For bracket items: The number of staves they span.
    API_PROPERTY_T(int, bracketSpan,      BRACKET_SPAN)

    /// For bracket items: The column they occupy.
    API_PROPERTY_T(int, bracketColumn,    BRACKET_COLUMN)
    /// For instrument names: Their layout position.
    API_PROPERTY_T(int, inameLayoutPosition, INAME_LAYOUT_POSITION)
    /// For text-based elements: The text style they use.
    /// One of PluginAPI::PluginAPI::Tid values
    API_PROPERTY(subStyle,                TEXT_STYLE)
    /// For text-based elements: The font face they use.
    API_PROPERTY(fontFace,                FONT_FACE)
    /// For text-based elements: The font size they use.
    API_PROPERTY_T(qreal, fontSize,       FONT_SIZE)
    /// For text-based elements: The font style they use.
    API_PROPERTY_T(int, fontStyle,        FONT_STYLE)
    /// For text-based elements: The line spacing they use.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, lineSpacing,    TEXT_LINE_SPACING)

    /// For text-based elements: Their border type,
    /// one of PluginAPI::PluginAPI::FrameType values.
    API_PROPERTY_T(int, frameType,        FRAME_TYPE)
    /// For text-based elements: The width of their border.
    API_PROPERTY(frameWidth,              FRAME_WIDTH)
    /// For text-based elements: The padding of their border.
    API_PROPERTY(framePadding,            FRAME_PADDING)
    /// For text-based elements: The radius of their border.
    API_PROPERTY_T(int, frameRound,       FRAME_ROUND)
    /// For text-based elements: The outline color of their border.
    API_PROPERTY_T(QColor, frameFgColor,  FRAME_FG_COLOR)
    /// For text-based elements: The fill color of their border.
    API_PROPERTY_T(QColor, frameBgColor,  FRAME_BG_COLOR)
    /// Whether an element is sized relative to its spatium size.
    API_PROPERTY_T(bool, sizeSpatiumDependent, SIZE_SPATIUM_DEPENDENT)
    /// For text lines: Whether their text is sized relative to its spatium size.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, textSizeSpatiumDependent, TEXT_SIZE_SPATIUM_DEPENDENT)
    /// The scale of musical symbols in text.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, musicalSymbolsScale, MUSICAL_SYMBOLS_SCALE)
    /// For text-based elements: Their text alignment.
    API_PROPERTY(align,                   ALIGN)
    /// For text-based elements: Their text's vertical alignment,
    /// used in sub- or superscript.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, textScriptAlign,  TEXT_SCRIPT_ALIGN)
    /// Whether this element is displayed above the system
    /// rather than the staff.
    API_PROPERTY_T(bool, systemFlag,      SYSTEM_FLAG)

    /// For text-line-based elements, the text to begin the line.
    API_PROPERTY(beginText,               BEGIN_TEXT)
    /// For text-line-based elements, the alignment of the beginning text.
    API_PROPERTY(beginTextAlign,          BEGIN_TEXT_ALIGN)
    /// For text-line-based elements, the placement of the beginning text,
    /// one of PluginAPI::PluginAPI::TextPlacement values.
    API_PROPERTY(beginTextPlace,          BEGIN_TEXT_PLACE)
    /// For text-line-based elements, the hook type at their beginning,
    /// one of PluginAPI::PluginAPI::HookType values.
    API_PROPERTY(beginHookType,           BEGIN_HOOK_TYPE)
    /// For text-line-based elements, the height of the hook at their beginning.
    API_PROPERTY(beginHookHeight,         BEGIN_HOOK_HEIGHT)
    /// For text-line-based elements, the font face of the beginning text.
    API_PROPERTY(beginFontFace,           BEGIN_FONT_FACE)
    /// For text-line-based elements, the font size of the beginning text.
    API_PROPERTY_T(qreal, beginFontSize,  BEGIN_FONT_SIZE)
    /// For text-line-based elements, the font style of the beginning text.
    API_PROPERTY_T(int, beginFontStyle,   BEGIN_FONT_STYLE)
    /// For text-line-based elements, the offset face of the beginning text.
    API_PROPERTY_T(QPointF, beginTextOffset, BEGIN_TEXT_OFFSET)
    /// For text-line-based elements, the distance between the line and text components.
    ///\since MuseScore 4.6
    API_PROPERTY(gapBetweenTextAndLine,   GAP_BETWEEN_TEXT_AND_LINE)

    /// For text-line-based elements, the text on new systems.
    API_PROPERTY(continueText,            CONTINUE_TEXT)
    /// For text-line-based elements, the alignment of the continuation text.
    API_PROPERTY(continueTextAlign,       CONTINUE_TEXT_ALIGN)
    /// For text-line-based elements, the placement of the continuation text,
    /// one of PluginAPI::PluginAPI::TextPlacement values.
    API_PROPERTY(continueTextPlace,       CONTINUE_TEXT_PLACE)
    /// For text-line-based elements, the font face of the continuation text.
    API_PROPERTY(continueFontFace,        CONTINUE_FONT_FACE)
    /// For text-line-based elements, the font size of the continuation text.
    API_PROPERTY_T(qreal, continueFontSize, CONTINUE_FONT_SIZE)
    /// For text-line-based elements, the font style of the continuation text.
    API_PROPERTY_T(int, continueFontStyle, CONTINUE_FONT_STYLE)
    /// For text-line-based elements, the offset face of the continuation text.
    API_PROPERTY_T(QPointF, continueTextOffset, CONTINUE_TEXT_OFFSET)

    /// For text-line-based elements, the text at the end of the line.
    API_PROPERTY(endText,                 END_TEXT)
    /// For text-line-based elements, the alignment of the ending text.
    API_PROPERTY(endTextAlign,            END_TEXT_ALIGN)
    /// For text-line-based elements, the placement of the ending text,
    /// one of PluginAPI::PluginAPI::TextPlacement values.
    API_PROPERTY(endTextPlace,            END_TEXT_PLACE)
    /// For text-line-based elements, the hook type at their end,
    /// one of PluginAPI::PluginAPI::HookType values.
    API_PROPERTY(endHookType,             END_HOOK_TYPE)
    /// For text-line-based elements, the height of the hook at their end.
    API_PROPERTY(endHookHeight,           END_HOOK_HEIGHT)
    /// For text-line-based elements, the font face of the ending text.
    API_PROPERTY(endFontFace,             END_FONT_FACE)
    /// For text-line-based elements, the font size of the ending text.
    API_PROPERTY_T(qreal, endFontSize,    END_FONT_SIZE)
    /// For text-line-based elements, the font style of the ending text.
    API_PROPERTY_T(int, endFontStyle,     END_FONT_STYLE)
    /// For text-line-based elements, the offset face of the ending text.
    API_PROPERTY_T(QPointF, endTextOffset, END_TEXT_OFFSET)

    /// For note-anchored-lines: Controls their placement relative to notes.
    /// One of PluginAPI::PluginAPI::NoteLineEndPlacement values.
    ///\since MuseScore 4.6
    API_PROPERTY(notelinePlacement,       NOTELINE_PLACEMENT)

    /// For lyrics and dynamics: Whether they avoid barlines on layout.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, avoidBarLines,   AVOID_BARLINES)
    /// For dynamics: Their scale.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, dynamicsSize,   DYNAMICS_SIZE)
    /// For dynamics: Whether they center on the notehead
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, centerOnNotehead, CENTER_ON_NOTEHEAD)
    /// For dynamics: Whether they anchor to end of the preceding segment.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, anchorToEndOfPrevious, ANCHOR_TO_END_OF_PREVIOUS)

    /// For expressions: Whether they align themselves to an accompanying dynamic
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, snapToDynamics,  SNAP_TO_DYNAMICS)
    /// For hairpins and gradual tempo changes:
    /// Whether they snap to the preceding element.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, snapBefore,      SNAP_BEFORE)
    /// For hairpins and gradual tempo changes:
    /// Whether they snap to the succeeding element.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, snapAfter,       SNAP_AFTER)

    /// The voice assignment properties for this element,
    /// one of PluginAPI::PluginAPI::VoiceAssignment values.
    ///\since MuseScore 4.6
    API_PROPERTY(voiceAssignment,         VOICE_ASSIGNMENT)
    /// Whether this element centers itself between staves,
    /// one of PluginAPI::PluginAPI::AutoOnOff values.
    ///\since MuseScore 4.6
    API_PROPERTY(centerBetweenStaves,     CENTER_BETWEEN_STAVES)

    API_PROPERTY(posAbove,                POS_ABOVE)

    ///\since MuseScore 4.6
    API_PROPERTY_T(int, locationStaves,   LOCATION_STAVES)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, locationVoices,   LOCATION_VOICES)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, locationMeasures, LOCATION_MEASURES)
    ///\since MuseScore 4.6
    API_PROPERTY(locationFractions,       LOCATION_FRACTIONS)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, locationGrace,    LOCATION_GRACE)
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, locationNote,     LOCATION_NOTE)

    /// The voice of this element.
    API_PROPERTY_T(int, voice,            VOICE)

    API_PROPERTY(position,                POSITION)

    /// For clefs: The clef type in concert pitch.
    ///\since MuseScore 4.6
    API_PROPERTY(concertClefType,         CLEF_TYPE_CONCERT)
    /// For clefs: The clef type in transposed pitch.
    ///\since MuseScore 4.6
    API_PROPERTY(transposingClefType,     CLEF_TYPE_TRANSPOSING)
    /// For clefs: The position relative to the barline,
    /// one of PluginAPI::PluginAPI::ClefToBarlinePosition values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, clefToBarlinePos, CLEF_TO_BARLINE_POS)
    /// Whether this clef is a header clef.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, isHeader,        IS_HEADER)
    /// For key signatures: The key in concert pitch.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, concertKey,       KEY_CONCERT)
    /// For key signatures: The actual key.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, actualKey,        KEY)
    ///\since MuseScore 4.6
    API_PROPERTY(action,                  ACTION)
    /// The minimum vertical distance between this element and other elements.
    ///\since MuseScore 4.6
    API_PROPERTY(minDistance,             MIN_DISTANCE)

    /// For arpeggios: The arpeggio type, one of
    /// PluginAPI::PluginAPI::ArpeggioType values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, arpeggioType,     ARPEGGIO_TYPE)
    /// For chord lines: The chord line type, one of
    /// PluginAPI::PluginAPI::ChordLineType values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, chordLineType,    CHORD_LINE_TYPE)
    /// Whether this chord line is straight.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, chordLineStraight, CHORD_LINE_STRAIGHT)
    /// Whether this chord line is wavy.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, chordLineWavy,   CHORD_LINE_WAVY)
    /// For tremolos: The tremolo type, one of
    /// PluginAPI::PluginAPI::TremoloType values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, tremoloType,      TREMOLO_TYPE)
    /// For half-note two-chord tremolos: The tremolo style.
    /// One of PluginAPI::PluginAPI::TremoloStyle values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, tremoloStrokeStyle, TREMOLO_STYLE)
    /// For chord symbols, chord symbol type, one of
    /// PluginAPI::PluginAPI::HarmonyType values.
    /// \since MuseScore 3.6
    API_PROPERTY_T(int, harmonyType,      HARMONY_TYPE)

    /// For arpeggios: The number of tracks it spans.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, arpeggioSpan,     ARPEGGIO_SPAN)

    /// For (obsolete) bends: The bend type.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, bendType,         BEND_TYPE)
    /// For (obsolete) bends: The bend curve.
    ///\since MuseScore 4.6
    API_PROPERTY(bendCurve,               BEND_CURVE)
    /// For guitar bends: The offset for the bend's midpoint.
    ///\since MuseScore 4.6
    API_PROPERTY_T(QPointF, bendVertexOffset, BEND_VERTEX_OFF)
    /// For gutar bends: Whether to show the hold line,
    /// one of PluginAPI::PluginAPI::GuitarBendShowHoldLine values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, bendShowHoldLine, BEND_SHOW_HOLD_LINE)
    /// For guitar bends: When to start the bend.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, bendStartTimeFactor, BEND_START_TIME_FACTOR)
    /// For guitar bends: When to end the bend.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, bendEndTimeFactor, BEND_END_TIME_FACTOR)

    /// For tremolo bars: The tremolo bar type,
    /// one of PluginAPI::PluginAPI::TremoloBarType values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, tremoloBarType,   TREMOLOBAR_TYPE)
    /// For tremolo bars: The tremolo bar curve.
    ///\since MuseScore 4.6
    API_PROPERTY(tremoloBarCurve,         TREMOLOBAR_CURVE)

    /// For section breaks: Whether to start the next measure with long names
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, startWithLongNames, START_WITH_LONG_NAMES)
    /// For section breaks: Whether to reset the measure number for the next measure
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, startWithMeasureOne, START_WITH_MEASURE_ONE)
    /// For section breaks: Whether to add an indentation before the next measure
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, firstSystemIndentation, FIRST_SYSTEM_INDENTATION)

    /// For chord lines: Their line path.
    ///\since MuseScore 4.6
    API_PROPERTY(path,                    PATH)

    /// For parts: Whether this part prefers sharps or flats,
    /// one of PluginAPI::PluginAPI::PreferSharpFlat values.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, preferSharpFlat,  PREFER_SHARP_FLAT)

    /// For playing technique annotations: Their type,
    /// one of PluginAPI::PluginAPI::PlayingTechniqueType values.
    ///\since MuseScore 4.6
    API_PROPERTY(playTechType,            PLAY_TECH_TYPE)

    /// For gradual tempo changes: Their type,
    /// one of PluginAPI::PluginAPI::GradualTempoChangeType values.
    ///\since MuseScore 4.6
    API_PROPERTY(tempoChangeType,         TEMPO_CHANGE_TYPE)
    /// The way a gradual tempo change interpolates between values.
    /// one of PluginAPI::PluginAPI::ChangeMethod values
    ///\since MuseScore 4.6
    API_PROPERTY(tempoEasingMethod,       TEMPO_EASING_METHOD)
    /// For gradual tempo changes: The factor by which to change
    /// the tempo, relative to the old tempo.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, tempoChangeFactor, TEMPO_CHANGE_FACTOR)

    /// For harp diagrams: Whether this diagram is a diagram,
    /// as opposed to text.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, isDiagram,       HARP_IS_DIAGRAM)

    /// For capos: Whether a capo is active and affects the sore.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, active,          ACTIVE)

    /// For capos: The fret position of the capo.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, fretPosition,     CAPO_FRET_POSITION)
    /// For capos: The ignored strings of the capo.
    ///\since MuseScore 4.6
    API_PROPERTY(ignoredStrings,          CAPO_IGNORED_STRINGS)
    /// For capos: If the capo should automatically generate text.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, generateText,    CAPO_GENERATE_TEXT)

    /// For tie elements: Their tie placement, one of
    /// PluginAPI::PluginAPI::TiePlacement values.
    ///\since MuseScore 4.6
    API_PROPERTY(tiePlacement,            TIE_PLACEMENT)
    /// For laissez vibrer elements: Their minimum length in spatiums.
    ///\since MuseScore 4.6
    API_PROPERTY(minLength,               MIN_LENGTH)
    /// For partial ties and slurs: Their partial spanner direction,
    /// one of PluginAPI::PluginAPI::PartialSpannerDirection alues.
    ///\since MuseScore 4.6
    API_PROPERTY(partialSpannerDirection, PARTIAL_SPANNER_DIRECTION)

    /// If the position of this part score element is linked to the main score.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, positionLinkedToMaster, POSITION_LINKED_TO_MASTER)
    /// If the appearance of this part score element is linked to the main score.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, appearanceLinkedToMaster, APPEARANCE_LINKED_TO_MASTER)
    /// If the text of this part score element is linked to the main score.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, textLinkedToMaster, TEXT_LINKED_TO_MASTER)
    /// If this element is included in the main score as well as the part score.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, excludeFromParts,  EXCLUDE_FROM_OTHER_PARTS)

    /// For string tunings: The number of strings to tune.
    ///\since MuseScore 4.6
    API_PROPERTY_T(int, stringsCount,     STRINGTUNINGS_STRINGS_COUNT)
    /// For string tunings: The active preset tuning setting.
    ///\since MuseScore 4.6
    API_PROPERTY(preset,                  STRINGTUNINGS_PRESET)
    /// For string tunings: Which tuned strings to display in the score.
    ///\since MuseScore 4.6
    API_PROPERTY(visibleStrings,          STRINGTUNINGS_VISIBLE_STRINGS)

    /// For symbols: Their musical symbols font face.
    ///\since MuseScore 4.6
    API_PROPERTY(scoreFont,               SCORE_FONT)
    /// For symbols: Their scale.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, symbolsSize,    SYMBOLS_SIZE)
    /// For symbols: Their rotation angle.
    ///\since MuseScore 4.6
    API_PROPERTY_T(qreal, symbolAngle,    SYMBOL_ANGLE)

    /// For sound flags: Whether they apply to all staves.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, applyToAllStaves, APPLY_TO_ALL_STAVES)
    /// For key signatures, clefs, and time signatures:
    /// Whether this element is a courtesy element.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, isCourtesy,      IS_COURTESY)
    /// Whether this fretboard diagram is excluded from vertical alignnment.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, excludeVerticalAlign, EXCLUDE_VERTICAL_ALIGN)

    /// For barlines & play count text: Controls how to display the play count text.
    /// One of PluginAPI::PluginAPI::AutoCustomHide values.
    API_PROPERTY(playCountTextSetting,    PLAY_COUNT_TEXT_SETTING)
    /// For barlines & play count text: Controls the text of the play count text.
    API_PROPERTY(playCountText,           PLAY_COUNT_TEXT)

    /// For rests: Whether to vertically align them with other rests in the same voice.
    /// \since MuseScore 4.6
    API_PROPERTY_T(bool, alignWithOtherRests, ALIGN_WITH_OTHER_RESTS)

    //  API_PROPERTY(end,                     END)

    qreal offsetX() const { return element()->offset().x() / element()->spatium(); }
    qreal offsetY() const { return element()->offset().y() / element()->spatium(); }
    void setOffsetX(qreal offX);
    void setOffsetY(qreal offY);

    qreal posX() const { return element()->pos().x() / element()->spatium(); }
    qreal posY() const { return element()->pos().y() / element()->spatium(); }

    QPointF pos() const { return PointF(element()->pos() / element()->spatium()).toQPointF(); }
    QPointF pagePos() const { return PointF(element()->pagePos() / element()->spatium()).toQPointF(); }
    QPointF canvasPos() const { return PointF(element()->canvasPos() / element()->spatium()).toQPointF(); }

    apiv1::EngravingItem* parent() const { return wrap(element()->parentItem()); }
    Staff* staff() { return wrap<Staff>(element()->staff()); }

    QRectF bbox() const;

    int subtype() const { return element()->subtype(); }

    int staffIdx() const { return int(element()->staffIdx()); }
    int effectiveStaffIdx() const { return int(element()->effectiveStaffIdx()); }
    int vStaffIdx() const { return int(element()->vStaffIdx()); }

    bool up() const;

    /// \brief Current tick for this element
    /// \returns Tick of this element, i.e. fraction of ticks from the beginning
    /// of the score to this element. Not valid for all elements.
    /// For the integer value, call \ref fraction.ticks
    /// \see \ref ticklength
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::FractionWrapper * fraction READ tick)
    /// \brief Current beat of this element
    /// \returns The beat this element starts on, as a fraction.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::FractionWrapper * beat READ beat)

public:
    /// \cond MS_INTERNAL
    EngravingItem(mu::engraving::EngravingItem* e = nullptr, Ownership own = Ownership::PLUGIN)
        : apiv1::ScoreElement(e, own) {}

    /// \brief muse::Returns the underlying mu::engraving::EngravingItem
    /// \{
    mu::engraving::EngravingItem* element() { return toEngravingItem(e); }
    const mu::engraving::EngravingItem* element() const { return toEngravingItem(e); }
    /// \}

    bool header() { return element()->header(); }
    bool trailer() { return element()->trailer(); }
    virtual bool isMovable() { return element()->isMovable(); }
    bool enabled() { return element()->enabled(); }
    bool addToSkyline() { return element()->addToSkyline(); }
    bool generated() { return element()->generated(); }
    /// \endcond

    /// Create a copy of the element
    Q_INVOKABLE apiv1::EngravingItem* clone() const { return wrap(element()->clone(), Ownership::PLUGIN); }

    Q_INVOKABLE QString subtypeName() const { return element()->translatedSubtypeUserName().toQString(); }
    /// Deprecated: same as ScoreElement::name. Left for compatibility purposes.
    Q_INVOKABLE QString _name() const { return name(); }

    FractionWrapper* tick() const;
    FractionWrapper* beat() const;
};

//---------------------------------------------------------
//   Note
//    Note wrapper
//---------------------------------------------------------

class Note : public EngravingItem
{
    Q_OBJECT
    Q_PROPERTY(apiv1::EngravingItem * accidental READ accidental)
    Q_PROPERTY(mu::engraving::AccidentalType accidentalType READ accidentalType WRITE setAccidentalType)
    /// List of dots attached to this note
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> dots READ dots)
//       Q_PROPERTY(int                            dotsCount         READ qmlDotsCount)
    /// List of other elements attached to this note: fingerings, symbols, bends etc.
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)
    /// List of PlayEvents associated with this note.
    /// Important: You must call Score.createPlayEvents()
    /// to see meaningful data in the PlayEvent lists.
    /// \since MuseScore 3.3
    Q_PROPERTY(QQmlListProperty<apiv1::PlayEvent> playEvents READ playEvents)
    /// List of spanners attached to and starting on this note
    /// e.g. glissandos, guitar bends
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> spannerForward READ spannerFor)
    /// List of spanners attached to and ending on this note
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> spannerBack READ spannerBack)
//       Q_PROPERTY(bool                           ghost             READ ghost              WRITE undoSetGhost)
//       Q_PROPERTY(mu::engraving::NoteHead::Group            headGroup         READ headGroup          WRITE undoSetHeadGroup)
//       Q_PROPERTY(mu::engraving::NoteHead::Type             headType          READ headType           WRITE undoSetHeadType)
//       Q_PROPERTY(bool                           hidden            READ hidden)
//       Q_PROPERTY(bool                           mirror            READ mirror)
//       Q_PROPERTY(int                            ppitch            READ ppitch)
//       Q_PROPERTY(bool                           small             READ isSmall            WRITE undoSetSmall)
//       Q_PROPERTY(int                            string            READ string             WRITE undoSetString)
//       Q_PROPERTY(int                            subchannel        READ subchannel)
    /// Backward tie for this Note.
    /// \since MuseScore 3.3
    Q_PROPERTY(apiv1::Tie * tieBack READ tieBack)
    /// Forward tie for this Note.
    /// \since MuseScore 3.3
    Q_PROPERTY(apiv1::Tie * tieForward READ tieForward)
    /// The first note of a series of ties to this note.
    /// This will return the calling note if there is not tieBack.
    /// \since MuseScore 3.3
    Q_PROPERTY(apiv1::Note * firstTiedNote READ firstTiedNote)
    /// The last note of a series of ties to this note.
    /// This will return the calling note if there is not tieForward.
    /// \since MuseScore 3.3
    Q_PROPERTY(apiv1::Note * lastTiedNote READ lastTiedNote)
    /// The NoteType of the note.
    /// \since MuseScore 3.2.1
    Q_PROPERTY(mu::engraving::NoteType noteType READ noteType)

    /// MIDI pitch of this note
    /// \see \ref pitch
    API_PROPERTY_T(int, pitch,            PITCH)
    /// Concert pitch of the note
    /// \see \ref tpc
    API_PROPERTY_T(int, tpc1,             TPC1)
    /// Transposing pitch of the note
    /// \see \ref tpc
    API_PROPERTY_T(int, tpc2,             TPC2)
    /// Concert or transposing pitch of this note,
    /// as per current "Concert Pitch" setting value.
    /// \see \ref tpc
    Q_PROPERTY(int tpc READ tpc WRITE setTpc)
    /// The position of a note's dot, one of
    /// PluginAPI::PluginAPI::Direction values.
    API_PROPERTY(dotPosition,             DOT_POSITION)
    /// See PluginAPI::PluginAPI::NoteValueType
    API_PROPERTY(veloType,                VELO_TYPE)
    /// The velocity of this note.
    API_PROPERTY_T(int,    userVelocity,  USER_VELOCITY)
    /// The tuning of this note, in cents.
    API_PROPERTY_T(qreal,  tuning,        TUNING)

    /// For notes on non-tab staves: the line this note is on.
    API_PROPERTY_T(int,    line,          LINE)
    /// For notes on non-tab staves: Whether this note is
    /// fixed to a specific line, irrespective of its pitch.
    API_PROPERTY_T(bool,   fixed,         FIXED)
    /// For notes on non-tab staves: if \p fixed is true,
    /// the line this note is fixed to.
    API_PROPERTY_T(int,    fixedLine,     FIXED_LINE)

    /// For notes on tab staves: The fret this note is on.
    API_PROPERTY_T(int, fret,             FRET)
    /// For notes on tab staves: The string this note is on.
    API_PROPERTY_T(int, string,           STRING)
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, dead,            DEAD)
    /// If the note is a trill cue note (used in ornaments and trills)
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isTrillCueNote READ isTrillCueNote)

public:
    /// \cond MS_INTERNAL
    Note(mu::engraving::Note* c = nullptr, Ownership own = Ownership::PLUGIN)
        : EngravingItem(c, own) {}

    mu::engraving::Note* note() { return toNote(e); }
    const mu::engraving::Note* note() const { return toNote(e); }

    int tpc() const { return note()->tpc(); }
    void setTpc(int val);

    bool isTrillCueNote() { return note()->isTrillCueNote(); }

    apiv1::Tie* tieBack() const { return wrap<Tie>(note()->tieBack()); }
    apiv1::Tie* tieForward() const { return wrap<Tie>(note()->tieFor()); }

    apiv1::Note* firstTiedNote() { return wrap<Note>(note()->firstTiedNote()); }
    apiv1::Note* lastTiedNote() { return wrap<Note>(note()->lastTiedNote()); }

    QQmlListProperty<EngravingItem> dots() { return wrapContainerProperty<EngravingItem>(this, note()->dots()); }
    QQmlListProperty<EngravingItem> elements() { return wrapContainerProperty<EngravingItem>(this, note()->el()); }
    QQmlListProperty<PlayEvent> playEvents() { return wrapPlayEventsContainerProperty(this, note()->playEvents()); }

    QQmlListProperty<EngravingItem> spannerFor()
    {
        return wrapContainerProperty<EngravingItem>(this, note()->spannerFor());
    }

    QQmlListProperty<EngravingItem> spannerBack()
    {
        return wrapContainerProperty<EngravingItem>(this, note()->spannerBack());
    }

    EngravingItem* accidental() { return wrap<EngravingItem>(note()->accidental()); }

    mu::engraving::AccidentalType accidentalType() { return note()->accidentalType(); }
    void setAccidentalType(mu::engraving::AccidentalType t) { note()->setAccidentalType(t); }
    mu::engraving::NoteType noteType() { return note()->noteType(); }

    static void addInternal(mu::engraving::Note* note, mu::engraving::EngravingItem* el);
    static bool isChildAllowed(mu::engraving::ElementType elementType);
    /// \endcond

    /// Creates a PlayEvent object for use in JavaScript.
    /// \since MuseScore 3.3
    Q_INVOKABLE apiv1::PlayEvent* createPlayEvent() { return playEventWrap(new engraving::NoteEvent(), nullptr); }

    /// Add to a note's elements.
    /// \since MuseScore 3.3.3
    Q_INVOKABLE void add(apiv1::EngravingItem* wrapped);
    /// Remove a note's element.
    /// \since MuseScore 3.3.3
    Q_INVOKABLE void remove(apiv1::EngravingItem* wrapped);
};

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

class DurationElement : public EngravingItem
{
    Q_OBJECT

    /// Nominal duration of this element.
    /// The duration is represented as a fraction of whole note length.
    /// This property can be modified for chords and rests since MuseScore 4.6
    //  prior to 4.6 this was called as API_PROPERTY_READ_ONLY(duration, DURATION).
    Q_PROPERTY(apiv1::FractionWrapper * duration READ ticks WRITE changeCRlen)

    /// Global duration of this element, taking into account ratio of
    /// parent tuplets if there are any.
    /// \since MuseScore 3.5
    Q_PROPERTY(apiv1::FractionWrapper * globalDuration READ globalDuration)

    /// Actual duration of this element, taking into account ratio of
    /// parent tuplets and local time signatures if there are any.
    /// \since MuseScore 3.5
    Q_PROPERTY(apiv1::FractionWrapper * actualDuration READ actualDuration)

    /// Tuplet which this element belongs to. If there is no parent tuplet, returns null.
    /// \since MuseScore 3.5
    Q_PROPERTY(apiv1::Tuplet * tuplet READ parentTuplet)

    /// Outermost tuplet which this element belongs to. If there is no parent tuplet, returns null.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Tuplet * topTuplet READ topTuplet)

    /// Measure which this element belongs to.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Measure * measure READ parentMeasure)

public:
    /// \cond MS_INTERNAL
    DurationElement(mu::engraving::DurationElement* de = nullptr, Ownership own = Ownership::PLUGIN)
        : EngravingItem(de, own) {}

    mu::engraving::DurationElement* durationElement() { return toDurationElement(e); }
    const mu::engraving::DurationElement* durationElement() const { return toDurationElement(e); }

    FractionWrapper* ticks() const;
    void changeCRlen(FractionWrapper* len);
    FractionWrapper* globalDuration() const;
    FractionWrapper* actualDuration() const;

    Tuplet* parentTuplet();
    Tuplet* topTuplet() { return wrap<Tuplet>(durationElement()->topTuplet(), Ownership::SCORE); }

    Measure* parentMeasure() { return wrap<Measure>(durationElement()->measure(), Ownership::SCORE); }
    /// \endcond
};

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

class Tuplet : public DurationElement
{
    Q_OBJECT

    /// The number type of this tuplet, one of
    /// PluginAPI::PluginAPI::TupletNumberType values.
    API_PROPERTY_T(int, numberType, NUMBER_TYPE)
    /// The bracket type of this tuplet, one of
    /// PluginAPI::PluginAPI::TupletBracketType values.
    API_PROPERTY_T(int, bracketType, BRACKET_TYPE)

    /// Whether this tuplet has a bracket.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool hasBracket READ hasBracket)

    /// Actual number of notes of base nominal length in this tuplet.
    API_PROPERTY_READ_ONLY_T(int, actualNotes, ACTUAL_NOTES)

    /// Number of "normal" notes of base nominal length which correspond
    /// to this tuplet's duration.
    API_PROPERTY_READ_ONLY_T(int, normalNotes, NORMAL_NOTES)

    /// The user offset for the left point of this tuplet, in spatium units.
    API_PROPERTY_T(QPointF, p1, P1)
    /// The user offset for the right point of this tuplet, in spatium units.
    API_PROPERTY_T(QPointF, p2, P2)

    /// The actual position of the left point of this tuplet, in spatium units.
    /// \since MuseScore 4.6
    Q_PROPERTY(QPointF defaultP1 READ defaultP1)
    /// The actual position of the right point of this tuplet, in spatium units.
    /// \since MuseScore 4.6
    Q_PROPERTY(QPointF defaultP2 READ defaultP2)

    /// List of elements which belong to this tuplet.
    /// \since MuseScore 3.5
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)

public:
    /// \cond MS_INTERNAL
    Tuplet(mu::engraving::Tuplet* t = nullptr, Ownership own = Ownership::PLUGIN)
        : DurationElement(t, own) {}

    mu::engraving::Tuplet* tuplet() { return toTuplet(e); }
    const mu::engraving::Tuplet* tuplet() const { return toTuplet(e); }

    bool hasBracket() { return tuplet()->hasBracket(); }

    QPointF defaultP1() const { return PointF(tuplet()->p1() / tuplet()->spatium()).toQPointF(); }
    QPointF defaultP2() const { return PointF(tuplet()->p2() / tuplet()->spatium()).toQPointF(); }

    QQmlListProperty<EngravingItem> elements() { return wrapContainerProperty<EngravingItem>(this, tuplet()->elements()); }
    /// \endcond
};

//---------------------------------------------------------
//   ChordRest
//    ChordRest wrapper
//---------------------------------------------------------

class ChordRest : public DurationElement
{
    Q_OBJECT
    /// Lyrics corresponding to this chord or rest, if any.
    /// Before 3.6 version this property was only available for \ref Chord objects.
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> lyrics READ lyrics)
    /// Beam which covers this chord/rest, if such exists.
    /// \since MuseScore 3.6
    Q_PROPERTY(apiv1::EngravingItem * beam READ beam)
    /// Whether this element is a full measure rest.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isFullMeasureRest READ isFullMeasureRest)

    /// For cross-staff chords and rests; The amount of staves this
    /// chord or rest has been moved by.
    API_PROPERTY_T(int, staffMove,        STAFF_MOVE)
    API_PROPERTY(durationTypeWithDots,    DURATION_TYPE_WITH_DOTS)
    /// The beam mode for this chord or rest, one of
    /// PluginAPI::PluginAPI::BeamMode values.
    API_PROPERTY(beamMode,                BEAM_MODE)

    /// List of elements attached to this chord or rest,
    /// such as fermatas or chord lines.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)

public:
    /// \cond MS_INTERNAL
    ChordRest(mu::engraving::ChordRest* c = nullptr, Ownership own = Ownership::PLUGIN)
        : DurationElement(c, own) {}

    mu::engraving::ChordRest* chordRest() { return toChordRest(e); }
    const mu::engraving::ChordRest* chordRest() const { return toChordRest(e); }

    QQmlListProperty<EngravingItem> lyrics() { return wrapContainerProperty<EngravingItem>(this, chordRest()->lyrics()); }   // TODO: special type for Lyrics?
    EngravingItem* beam() { return wrap(chordRest()->beam()); }

    bool isFullMeasureRest() { return chordRest()->isFullMeasureRest(); }

    QQmlListProperty<EngravingItem> elements() { return wrapContainerProperty<EngravingItem>(this, chordRest()->el()); }
    /// \endcond

    /// The actual beam mode for this element, as determined by
    /// user settings and the current time signature.
    /// \param beamRests Whether to beam over rests
    /// \since MuseScore 4.6
    Q_INVOKABLE int actualBeamMode(bool beamRests = false);
};

//---------------------------------------------------------
//   Chord
//    Chord wrapper
//---------------------------------------------------------

class Chord : public ChordRest
{
    Q_OBJECT
    /// List of grace notes (grace chords) belonging to this chord.
    Q_PROPERTY(QQmlListProperty<apiv1::Chord> graceNotes READ graceNotes)
    /// List of grace notes (grace chords) placed before this chord.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::Chord> graceNotesBefore READ graceNotesBefore)
    /// List of grace notes (grace chords) placed after this chord.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::Chord> graceNotesAfter READ graceNotesAfter)
    /// List of notes belonging to this chord.
    Q_PROPERTY(QQmlListProperty<apiv1::Note> notes READ notes)
    /// List of articulations belonging to this chord.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> articulations READ articulations)
    /// Stem of this chord, if exists. \since MuseScore 3.6
    Q_PROPERTY(apiv1::EngravingItem * stem READ stem)
    /// Stem slash of this chord, if exists. Stem slashes are present in grace notes of type acciaccatura.
    /// \since MuseScore 3.6
    Q_PROPERTY(apiv1::EngravingItem * stemSlash READ stemSlash)
    /// Hook on a stem of this chord, if exists. \since MuseScore 3.6
    Q_PROPERTY(apiv1::EngravingItem * hook READ hook)
    /// The NoteType of the chord.
    /// \since MuseScore 3.2.1
    Q_PROPERTY(mu::engraving::NoteType noteType READ noteType)
    /// The PlayEventType of the chord.
    /// \since MuseScore 3.3
    Q_PROPERTY(mu::engraving::PlayEventType playEventType READ playEventType WRITE setPlayEventType)

    /// Whether this chord is stemless. Does not override
    /// its measure's or staff's stemless settings.
    API_PROPERTY(noStem,                  NO_STEM)
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, showStemSlash,   SHOW_STEM_SLASH)
    ///\since MuseScore 4.6
    API_PROPERTY(combineVoice,            COMBINE_VOICE)
    /// If the note is a trill cue note (used in ornaments and trills)
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isTrillCueNote READ isTrillCueNote)
    /// The top note in this chord.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Note * upNote READ upNote)
    /// The bottom note in this chord.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Note * downNote READ downNote)
    /// The arpeggio attached to this chord, if it exists.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::EngravingItem * arpeggio READ arpeggio)
    /// The cross-voice arpeggio for this chord, if it exists.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::EngravingItem * spanArpeggio READ spanArpeggio)
    /// The single-chord tremolo for this chord, if it exists.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::EngravingItem * tremoloSingleChord READ tremoloSingleChord)
    /// The two-chord tremolo for this chord, if it exists.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::EngravingItem * tremoloTwoChord READ tremoloTwoChord)

public:
    /// \cond MS_INTERNAL
    Chord(mu::engraving::Chord* c = nullptr, Ownership own = Ownership::PLUGIN)
        : ChordRest(c, own) {}

    mu::engraving::Chord* chord() { return toChord(e); }
    const mu::engraving::Chord* chord() const { return toChord(e); }

    QQmlListProperty<Chord> graceNotes() { return wrapContainerProperty<Chord>(this, chord()->graceNotes()); }
    QQmlListProperty<Chord> graceNotesBefore() { return wrapContainerProperty<Chord>(this, chord()->graceNotesBefore()); }
    QQmlListProperty<Chord> graceNotesAfter() { return wrapContainerProperty<Chord>(this, chord()->graceNotesAfter()); }
    QQmlListProperty<Note> notes() { return wrapContainerProperty<Note>(this, chord()->notes()); }
    QQmlListProperty<EngravingItem> articulations() { return wrapContainerProperty<EngravingItem>(this, chord()->articulations()); }
    EngravingItem* stem() { return wrap(chord()->stem()); }
    EngravingItem* stemSlash() { return wrap(chord()->stemSlash()); }
    EngravingItem* hook() { return wrap(chord()->hook()); }
    mu::engraving::NoteType noteType() { return chord()->noteType(); }
    mu::engraving::PlayEventType playEventType() { return chord()->playEventType(); }
    void setPlayEventType(mu::engraving::PlayEventType v);

    bool isTrillCueNote() { return chord()->isTrillCueNote(); }

    Note* upNote() { return wrap<Note>(chord()->upNote()); }
    Note* downNote() { return wrap<Note>(chord()->downNote()); }
    EngravingItem* arpeggio() { return wrap(chord()->arpeggio()); }
    EngravingItem* spanArpeggio() { return wrap(chord()->spanArpeggio()); }
    EngravingItem* tremoloSingleChord() { return wrap(chord()->tremoloSingleChord()); }
    EngravingItem* tremoloTwoChord() { return wrap(chord()->tremoloTwoChord()); }

    static void addInternal(mu::engraving::Chord* chord, mu::engraving::EngravingItem* el);
    /// \endcond

    /// Add to a chord's elements.
    /// \since MuseScore 3.3
    Q_INVOKABLE void add(apiv1::EngravingItem* wrapped);
    /// Remove a chord's element.
    /// \since MuseScore 3.3
    Q_INVOKABLE void remove(apiv1::EngravingItem* wrapped);
};

//---------------------------------------------------------
//   Segment
//    Segment
//---------------------------------------------------------

class Segment : public EngravingItem
{
    Q_OBJECT
    /// The added space preceding this segment.
    API_PROPERTY(leadingSpace,            LEADING_SPACE)
    /// The list of annotations. Articulations, staff/system/expression
    /// text are examples of what is considered to be segment annotations.
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> annotations READ annotations)
    /// \brief Next segment in this measure
    /// \returns The next segment in this segment's measure.
    /// Null if there is no such segment.
    Q_PROPERTY(apiv1::Segment * nextInMeasure READ nextInMeasure)
    /// \brief Next segment in this score.\ Doesn't stop at measure border.
    /// \returns The next segment in this score. Null if there is
    /// no such segment (i.e. this is the last segment in the score).
    Q_PROPERTY(apiv1::Segment * next READ nextInScore)
    /// \brief Previous segment in this measure
    /// \returns The previous segment in this segment's measure.
    /// Null if there is no such segment.
    Q_PROPERTY(apiv1::Segment * prevInMeasure READ prevInMeasure)
    /// \brief Previous segment in this score.\ Doesn't stop at measure border.
    /// \returns The previous segment in this score. Null if there is
    /// no such segment (i.e. this is the first segment in the score).
    Q_PROPERTY(apiv1::Segment * prev READ prevInScore)
    // segmentType was read&write in MuseScore 2.X plugin API.
    // Allowing plugins to change random segments types doesn't seem to be a
    // good idea though.
    /// Type of this segment, one of PluginAPI::PluginAPI::Segment values.
    Q_PROPERTY(int segmentType READ segmentType)
    /// \brief Current tick for this segment
    /// \returns Tick of this segment, i.e. number of ticks from the beginning
    /// of the score to this segment.
    /// \see \ref ticklength
    Q_PROPERTY(int tick READ tick)                               // TODO: revise engraving (or this API):
                                                                 // Pid::TICK is relative or absolute in different contexts
    /// \brief Current tick fraction for this element
    /// \returns Tick of this element, i.e. fraction of ticks from the beginning
    /// of the score to this element. Not valid for all elements.
    /// For the integer value, call \ref fraction.ticks
    /// \see \ref ticklength
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::FractionWrapper * fraction READ fraction)

public:
    /// \cond MS_INTERNAL
    Segment(mu::engraving::Segment* s = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(s, own) {}

    mu::engraving::Segment* segment() { return toSegment(e); }
    const mu::engraving::Segment* segment() const { return toSegment(e); }

    int tick() const { return segment()->tick().ticks(); }
    FractionWrapper* fraction() const;

    int segmentType() const { return int(segment()->segmentType()); }

    Segment* nextInScore() { return wrap<Segment>(segment()->next1()); }
    Segment* nextInMeasure() { return wrap<Segment>(segment()->next()); }
    Segment* prevInScore() { return wrap<Segment>(segment()->prev1()); }
    Segment* prevInMeasure() { return wrap<Segment>(segment()->prev()); }
    QQmlListProperty<EngravingItem> annotations() { return wrapContainerProperty<EngravingItem>(this, segment()->annotations()); }
    /// \endcond

    /// \returns Element at the given \p track (null if there is no such an element)
    /// \param track track number
    Q_INVOKABLE apiv1::EngravingItem* elementAt(int track);
};

//---------------------------------------------------------
//   MeasureBase
//    MeasureBase wrapper (Measures, frames)
//---------------------------------------------------------

class MeasureBase : public EngravingItem
{
    Q_OBJECT

    /// Whether a repeat ends on this measureBase
    API_PROPERTY_T(bool, repeatEnd,       REPEAT_END)
    /// Whether a repeat starts on this measureBase
    API_PROPERTY_T(bool, repeatStart,     REPEAT_START)
    /// Whether this measureBase contaions a jump.
    API_PROPERTY_T(bool, repeatJump,      REPEAT_JUMP)
    /// The measure number offset of this measureBase.
    API_PROPERTY_T(int, noOffset,         NO_OFFSET)
    /// Whether this measureBase is included in the measure count.
    API_PROPERTY_T(bool, irregular,       IRREGULAR)

    /// \brief Measure number, counting from 1.
    /// Number of this measure in the score counting from 1, i.e.
    /// for the first measure its \p no value will be equal to 1.
    /// User-visible measure number can be calculated as
    /// \code
    /// measure.no + measure.noOffset
    /// \endcode
    /// where \p measure is the relevant \ref Measure object.
    /// \since MuseScore 4.6
    /// \see ScoreElement::noOffset
    Q_PROPERTY(int no READ no)
    /// \brief Current tick for this measure
    /// \returns Tick of this measure, i.e. number of ticks from the beginning
    /// of the score to this measure, as a fraction.
    /// \see \ref ticklength
    Q_PROPERTY(apiv1::FractionWrapper * tick READ tick)
    /// \brief Length of this measure in ticks.
    /// \returns Length of this measure, i.e. number of ticks from its beginning
    /// to its end, as a fraction.
    /// \see \ref ticklength
    Q_PROPERTY(apiv1::FractionWrapper * ticks READ ticks)
    /// List of measure-related elements: layout breaks, jump/repeat markings etc.
    /// For frames (since MuseScore 4.6), also contains their text elements.
    /// \since MuseScore 3.3
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)

    /// Next measure.
    Q_PROPERTY(apiv1::Measure * nextMeasure READ nextMeasure)
    /// Next measure, accounting for multimeasure rests.
    /// This property may differ from \ref nextMeasure if multimeasure rests
    /// are enabled. If next measure is a multimeasure rest, this property
    /// points to the multimeasure rest measure while \ref nextMeasure in the
    /// same case will point to the first underlying empty measure. Therefore
    /// if visual properties of a measure are needed (as opposed to logical
    /// score structure) this property should be preferred.
    /// \see \ref Score.firstMeasureMM
    /// \since MuseScore 3.6
    Q_PROPERTY(apiv1::Measure * nextMeasureMM READ nextMeasureMM)
    /// Previous measure.
    Q_PROPERTY(apiv1::Measure * prevMeasure READ prevMeasure)
    /// Previous measure, accounting for multimeasure rests.
    /// See \ref nextMeasureMM for a reference on multimeasure rests.
    /// \see \ref Score.lastMeasureMM
    /// \since MuseScore 3.6
    Q_PROPERTY(apiv1::Measure * prevMeasureMM READ prevMeasureMM)
    /// Next measure or frame.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::MeasureBase * next READ next)
    /// Next measure or frame, accounting for multimeasure rests.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::MeasureBase * nextMM READ nextMM)
    /// Next measure or frame.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::MeasureBase * prev READ prev)
    /// Next measure or frame, accounting for multimeasure rests.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::MeasureBase * prevMM READ prevMM)

public:
    /// \cond MS_INTERNAL
    MeasureBase(mu::engraving::MeasureBase* mb = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(mb, own) {}

    mu::engraving::MeasureBase* measureBase() { return toMeasureBase(e); }
    const mu::engraving::MeasureBase* measureBase() const { return toMeasureBase(e); }

    int no() { return measureBase()->no(); }

    FractionWrapper* tick() const;
    FractionWrapper* ticks() const;

    Measure* prevMeasure() { return wrap<Measure>(measureBase()->prevMeasure(), Ownership::SCORE); }
    Measure* nextMeasure() { return wrap<Measure>(measureBase()->nextMeasure(), Ownership::SCORE); }
    Measure* prevMeasureMM() { return wrap<Measure>(measureBase()->prevMeasureMM(), Ownership::SCORE); }
    Measure* nextMeasureMM() { return wrap<Measure>(measureBase()->nextMeasureMM(), Ownership::SCORE); }

    MeasureBase* prev() { return wrap<MeasureBase>(measureBase()->prev(), Ownership::SCORE); }
    MeasureBase* next() { return wrap<MeasureBase>(measureBase()->next(), Ownership::SCORE); }
    MeasureBase* prevMM() { return wrap<MeasureBase>(measureBase()->prevMM(), Ownership::SCORE); }
    MeasureBase* nextMM() { return wrap<MeasureBase>(measureBase()->nextMM(), Ownership::SCORE); }

    QQmlListProperty<EngravingItem> elements() { return wrapContainerProperty<EngravingItem>(this, measureBase()->el()); }

    static void addInternal(mu::engraving::MeasureBase* measureBase, mu::engraving::EngravingItem* el);
    /// \endcond

    /// Add to a MeasureBases's elements.
    /// \since MuseScore 4.6
    Q_INVOKABLE void add(apiv1::EngravingItem* wrapped);
    /// Remove a MeasureBase's element.
    /// \since MuseScore 4.6
    Q_INVOKABLE void remove(apiv1::EngravingItem* wrapped);
};

//---------------------------------------------------------
//   Measure
//    Measure wrapper
//---------------------------------------------------------

class Measure : public MeasureBase
{
    Q_OBJECT
    /// The first segment of this measure
    Q_PROPERTY(apiv1::Segment * firstSegment READ firstSegment)
    /// The last segment of this measure
    Q_PROPERTY(apiv1::Segment * lastSegment READ lastSegment)

//       Q_PROPERTY(bool         lineBreak         READ lineBreak   WRITE undoSetLineBreak)
//       Q_PROPERTY(bool         pageBreak         READ pageBreak   WRITE undoSetPageBreak)

    /// The nominal (written) time signature for this measure.
    API_PROPERTY(timesigNominal,          TIMESIG_NOMINAL)
    /// The actual time signature / duration of this measure.
    API_PROPERTY(timesigActual,           TIMESIG_ACTUAL)

    /// Controls whether this measure displays a measure number,
    /// one of PluginAPI::PluginAPI::MeasureNumberMode values.
    /// \since MuseScore 4.6
    API_PROPERTY_T(int, measureNumberMode, MEASURE_NUMBER_MODE)
    /// Whether this measure displays a measure number when
    /// \ref measureNumberMode is set to AUTO.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool showsMeasureNumberInAutoMode READ showsMeasureNumberInAutoMode)

    /// Whether this measure explicitly breaks a multimeasure rest.
    /// The multimeasure rest may still be broken by measure contents.
    API_PROPERTY_T(bool, breakMmr,        BREAK_MMR)
    /// For measures before repeats: How many times
    /// the repeat is to be played.
    API_PROPERTY_T(int, repeatCount,      REPEAT_COUNT)
    /// The spacing ratio for this measure.
    API_PROPERTY_T(qreal, userStretch,    USER_STRETCH)

    /// If this measure is part of a multimeasure rest,
    /// returns the first measure included in it.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Measure * mmRest READ mmRest)
    /// If this measure is the first measure of a multimeasure rest.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isMMRestStart READ isMMRest)

    /// List of segments in the measure.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::Segment> segments READ segments)

public:
    /// \cond MS_INTERNAL
    Measure(mu::engraving::Measure* m = nullptr, Ownership own = Ownership::SCORE)
        : MeasureBase(m, own) {}

    mu::engraving::Measure* measure() { return toMeasure(e); }
    const mu::engraving::Measure* measure() const { return toMeasure(e); }

    bool showsMeasureNumberInAutoMode() { return measure()->showMeasureNumberInAutoMode(); }

    Segment* firstSegment() { return wrap<Segment>(measure()->firstEnabled(), Ownership::SCORE); }
    Segment* lastSegment() { return wrap<Segment>(measure()->last(), Ownership::SCORE); }

    bool isMMRest() const { return measure()->isMMRest(); }
    Measure* mmRest() const { return wrap<Measure>(measure()->mmRest(), Ownership::SCORE); }

    QQmlListProperty<Segment> segments() { return wrapContainerProperty<Segment>(this, measure()->segments()); }
    /// \endcond

    /// Up spacer for a given staff.
    /// \param staffIdx staff to retrieve the spacer from
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* vspacerUp(int staffIdx);
    /// Down spacer for a given staff.
    /// \param staffIdx staff to retrieve the spacer from
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* vspacerDown(int staffIdx);
    /// Measure number object at a given staff.
    /// \param staffIdx staff to retrieve the object from
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* measureNumber(int staffIdx);
    /// The mmRestRange object at a given staff.
    /// \param staffIdx staff to retrieve the object from
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* mmRangeText(int staffIdx);
    /// Whether the measure is corrupted at a given staff.
    /// \param staffIdx staff to check if corrupted
    /// \since MuseScore 4.6
    Q_INVOKABLE bool corrupted(int staffIdx);
    /// Whether the measure is visible at a given staff.
    /// \note This option does not override a staff's visibility
    /// setting, so a measure may not necessarily be visible.
    /// \param staffIdx staff to check if visible
    /// \since MuseScore 4.6
    Q_INVOKABLE bool visible(int staffIdx);
    /// Whether the measure is stemless at a given staff.
    /// \note This option does not override a staff's stemless
    /// setting, so a measure may actually be stemless.
    /// \param staffIdx staff to check if stemless
    /// \since MuseScore 4.6
    Q_INVOKABLE bool stemless(int staffIdx);
};

//---------------------------------------------------------
//   System
///    \since MuseScore 4.6
//---------------------------------------------------------

class System : public EngravingItem
{
    Q_OBJECT
    /// List of measures and frames in this system.
    Q_PROPERTY(QQmlListProperty<apiv1::MeasureBase> measures READ measures)
    /// The first measure of this system
    Q_PROPERTY(apiv1::Measure * firstMeasure READ firstMeasure)
    /// The last measure of this system
    Q_PROPERTY(apiv1::Measure * lastMeasure READ lastMeasure)
    /// The first measure or frame of this system
    Q_PROPERTY(apiv1::MeasureBase * first READ first)
    /// The last measure or frame of this system
    Q_PROPERTY(apiv1::MeasureBase * last READ last)
    /// Indicates whether this system is locked
    Q_PROPERTY(bool isLocked READ isLocked WRITE setIsLocked)
    /// Indicates whether this system has a page break
    Q_PROPERTY(bool pageBreak READ pageBreak)
    /// The left system divider for this system, if it exists.
    Q_PROPERTY(apiv1::EngravingItem * systemDividerLeft READ systemDividerLeft)
    /// The right system divider for this system, if it exists.
    Q_PROPERTY(apiv1::EngravingItem * systemDividerRight READ systemDividerRight)

public:
    /// \cond MS_INTERNAL
    System(mu::engraving::System* sys = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(sys, own) {}

    mu::engraving::System* system() { return toSystem(e); }
    const mu::engraving::System* system() const { return toSystem(e); }

    QQmlListProperty<MeasureBase> measures() { return wrapContainerProperty<MeasureBase>(this, system()->measures()); }

    Measure* firstMeasure() { return wrap<Measure>(system()->firstMeasure(), Ownership::SCORE); }
    Measure* lastMeasure() { return wrap<Measure>(system()->lastMeasure(), Ownership::SCORE); }
    MeasureBase* first() { return wrap<MeasureBase>(system()->first(), Ownership::SCORE); }
    MeasureBase* last() { return wrap<MeasureBase>(system()->last(), Ownership::SCORE); }
    bool isLocked() { return system()->isLocked(); }
    void setIsLocked(bool locked);
    bool pageBreak() { return system()->pageBreak(); }
    EngravingItem* systemDividerLeft() { return wrap<EngravingItem>(system()->systemDividerLeft(), Ownership::SCORE); }
    EngravingItem* systemDividerRight() { return wrap<EngravingItem>(system()->systemDividerRight(), Ownership::SCORE); }
    /// \endcond

    /// Bounding box for a given staff.
    /// \param staffIdx staff number
    Q_INVOKABLE QRectF bbox(int staffIdx);
    /// Y position of the bbox relative to the top staff line.
    /// \param staffIdx staff number
    Q_INVOKABLE qreal yOffset(int staffIdx);
    /// Whether the given staff is visible for this system.
    /// This can differ from \ref Staff.show (e.g. due to 'hide when empty' rules).
    /// \param staffIdx staff number
    Q_INVOKABLE bool show(int staffIdx);
};

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page : public EngravingItem
{
    Q_OBJECT
    /// \brief Page number, counting from 0.
    /// Number of this page in the score counting from 0, i.e.
    /// for the first page its \p pagenumber value will be equal to 0.
    /// User-visible page number can be calculated as
    /// \code
    /// page.pagenumber + 1 + score.pageNumberOffset
    /// \endcode
    /// where \p score is the relevant \ref Score object.
    /// \since MuseScore 3.5
    /// \see Score::pageNumberOffset
    Q_PROPERTY(int pagenumber READ pagenumber)

    /// List of systems on this page.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::System> systems READ systems)

public:
    /// \cond MS_INTERNAL
    Page(mu::engraving::Page* p = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(p, own) {}

    mu::engraving::Page* page() { return toPage(e); }
    const mu::engraving::Page* page() const { return toPage(e); }

    int pagenumber() const;

    QQmlListProperty<System> systems() { return wrapContainerProperty<System>(this, page()->systems()); }
    /// \endcond
};

//---------------------------------------------------------
//   Ornament
///    \since MuseScore 4.6
//---------------------------------------------------------

class Ornament : public EngravingItem
{
    Q_OBJECT
    /// Whether this ornament has an interval above the attached note.
    Q_PROPERTY(bool hasIntervalAbove READ hasIntervalAbove)
    /// Whether this ornament has an interval below the attached note.
    Q_PROPERTY(bool hasIntervalBelow READ hasIntervalBelow)
    /// Whether this ornament displays a cue note.
    Q_PROPERTY(bool showCueNote READ showCueNote)
    /// The accidental for the interval above the attached note.
    Q_PROPERTY(apiv1::EngravingItem * accidentalAbove READ accidentalAbove)
    /// The accidental for the interval below the attached note.
    Q_PROPERTY(apiv1::EngravingItem * accidentalBelow READ accidentalBelow)

public:
    /// \cond MS_INTERNAL
    Ornament(mu::engraving::Ornament* o = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(o, own) {}

    mu::engraving::Ornament* ornament() { return toOrnament(e); }
    const mu::engraving::Ornament* ornament() const { return toOrnament(e); }

    bool hasIntervalAbove() const { return ornament()->hasIntervalAbove(); }
    bool hasIntervalBelow() const { return ornament()->hasIntervalBelow(); }
    bool showCueNote() { return ornament()->showCueNote(); }
    EngravingItem* accidentalAbove() const { return wrap<EngravingItem>(ornament()->accidentalAbove()); }
    EngravingItem* accidentalBelow() const { return wrap<EngravingItem>(ornament()->accidentalBelow()); }
    /// \endcond
};

//---------------------------------------------------------
//   Staff
///   \since MuseScore 3.5
//---------------------------------------------------------

class Staff : public ScoreElement
{
    Q_OBJECT

    API_PROPERTY_T(bool, small,           SMALL)
    API_PROPERTY_T(qreal, mag,            MAG)
    /// Staff color. See https://doc.qt.io/qt-5/qml-color.html
    /// for the reference on color type in QML.
    API_PROPERTY_T(QColor, color,         STAFF_COLOR)

    /// Whether voice 1 participates in playback.
    API_PROPERTY_T(bool, playbackVoice1,  PLAYBACK_VOICE1)
    /// Whether voice 2 participates in playback.
    API_PROPERTY_T(bool, playbackVoice2,  PLAYBACK_VOICE2)
    /// Whether voice 3 participates in playback.
    API_PROPERTY_T(bool, playbackVoice3,  PLAYBACK_VOICE3)
    /// Whether voice 4 participates in playback.
    API_PROPERTY_T(bool, playbackVoice4,  PLAYBACK_VOICE4)

    /// Whether a staff should display measure numbers.
    /// One of PluginAPI::PluginAPI::AutoOnOff values.
    /// \since MuseScore 4.6
    API_PROPERTY(showMeasureNumbers,      SHOW_MEASURE_NUMBERS)

    /// Whether to show this staff if the entire system is empty.
    /// \since MuseScore 4.6
    API_PROPERTY_T(bool, showIfEntireSystemEmpty, SHOW_IF_ENTIRE_SYSTEM_EMPTY)

    API_PROPERTY_T(int, staffBarlineSpan,     STAFF_BARLINE_SPAN)
    API_PROPERTY_T(int, staffBarlineSpanFrom, STAFF_BARLINE_SPAN_FROM)
    API_PROPERTY_T(int, staffBarlineSpanTo,   STAFF_BARLINE_SPAN_TO)

    /// Controls whether the staff lines are visible.
    ///\since MuseScore 4.6
    API_PROPERTY_T(bool, staffInvisible,  STAFF_INVISIBLE)

    /// User-defined amount of additional space before this staff.
    /// It is recommended to consider adding a spacer instead as it
    /// allows adjusting staff spacing locally as opposed to this
    /// property.
    /// \see \ref EngravingItem.space
    API_PROPERTY_T(qreal, staffUserdist,  STAFF_USERDIST)

    /// Part which this staff belongs to.
    Q_PROPERTY(apiv1::Part * part READ part);

    /// The index of this staff.
    /// \since MuseScore 4.6
    Q_PROPERTY(int idx READ idx)

    /// Whether the staff is visible.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool show READ show)
    /// Whether this is a cutaway staff, which hides itself
    /// mid-system when measures are empty.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool cutaway READ cutaway)
    /// Whether to display the system barline (leftmost barline).
    /// \since MuseScore 4.6
    Q_PROPERTY(bool hideSystemBarLine READ hideSystemBarLine)
    /// Whether to merge matching rests across voices.
    /// One of PluginAPI::PluginAPI::AutoOnOff values.
    /// If Auto, determined by the global style setting \p mergeMatchingRests .
    /// \since MuseScore 4.6
    Q_PROPERTY(int mergeMatchingRests READ mergeMatchingRests)
    /// Whether matching rests are to be merged.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool shouldMergeMatchingRests READ shouldMergeMatchingRests)
    /// The primary (not linked) staff of this staff.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Staff * primaryStaff READ primaryStaff)

    /// List of bracket items for this staff.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> brackets READ brackets)

public:
    /// \cond MS_INTERNAL
    Staff(mu::engraving::Staff* staff, Ownership own = Ownership::PLUGIN)
        : apiv1::ScoreElement(staff, own) {}

    mu::engraving::Staff* staff() { return toStaff(e); }
    const mu::engraving::Staff* staff() const { return toStaff(e); }

    Part* part();
    int idx() { return int(staff()->idx()); }
    bool show() { return staff()->show(); }
    bool cutaway() { return staff()->cutaway(); }
    bool hideSystemBarLine() { return staff()->hideSystemBarLine(); }
    int mergeMatchingRests() { return int(staff()->mergeMatchingRests()); }
    bool shouldMergeMatchingRests() { return staff()->shouldMergeMatchingRests(); }
    Staff* primaryStaff() { return wrap<Staff>(staff()->primaryStaff()); }
    QQmlListProperty<EngravingItem> brackets() { return wrapContainerProperty<EngravingItem>(this, staff()->brackets()); }
    /// \endcond

    /// The current clef type at a given tick in the score, one of
    /// PluginAPI::PluginAPI::ClefType values.
    /// \param tick Tick location in the score, as a fraction.
    /// \see PluginAPI::PluginAPI::ClefType
    /// \since MuseScore 4.6
    Q_INVOKABLE int clefType(apiv1::FractionWrapper* tick);
    /// The current timestretch factor at a given tick in the score, i.e. the
    /// ratio of the local time signature over the global time signature.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::FractionWrapper* timeStretch(apiv1::FractionWrapper* tick);
    /// The currently active time signature at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE EngravingItem* timeSig(apiv1::FractionWrapper* tick);
    /// The current written key at a given tick in the score, one of
    /// PluginAPI::PluginAPI::Key values.
    /// \param tick Tick location in the score, as a fraction.
    /// \see PluginAPI::PluginAPI::Key
    /// \since MuseScore 4.6
    Q_INVOKABLE int key(apiv1::FractionWrapper* tick);
    /// The transposition at a given tick in the score, active if the
    /// score is not in concert pitch.
    /// \param tick Tick location in the score, as a fraction.
    /// \see PluginAPI::IntervalWrapper
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::IntervalWrapper* transpose(apiv1::FractionWrapper* tick);

    /// The swing settings at a given tick.
    /// \returns An object with the following fields:
    /// - \p swingUnit - raw tick length value of the swing unit
    /// - \p swingRatio - value of the ratio, percentage.
    /// - \p isOn - whether swing is active.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QVariantMap swing(apiv1::FractionWrapper* tick);
    /// The capo settings at a given tick.
    /// \returns An object with the following fields:
    /// - \p active - whether there is a capo active.
    /// - \p fretPosition - the fret of the capo
    /// - \p ignoredStrings - list of strings not affected by the capo.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QVariantMap capo(apiv1::FractionWrapper* tick);

    /// Whether the notes at a given tick are stemless
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE bool stemless(apiv1::FractionWrapper* tick);
    /// The staff height at a given tick in spatium units.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE qreal staffHeight(apiv1::FractionWrapper* tick);
    // StaffType helper functions
    /// Whether the staff is a pitched staff at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE bool isPitchedStaff(apiv1::FractionWrapper* tick);
    /// Whether the staff is a tab staff at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE bool isTabStaff(apiv1::FractionWrapper* tick);
    /// Whether the staff is a drum staff at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE bool isDrumStaff(apiv1::FractionWrapper* tick);
    /// The number of staff lines at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE int lines(apiv1::FractionWrapper* tick);
    /// The distance between staff lines at a given tick, in spatium units.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE qreal lineDistance(apiv1::FractionWrapper* tick);
    /// Whether the staff lines are invisible at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE bool isLinesInvisible(apiv1::FractionWrapper* tick);
    /// The middle line of this staff at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE int middleLine(apiv1::FractionWrapper* tick);
    /// The bottom line of this staff at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE int bottomLine(apiv1::FractionWrapper* tick);
    /// The staff scaling at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE qreal staffMag(apiv1::FractionWrapper* tick);
    /// The spatium value at a given tick.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE qreal spatium(apiv1::FractionWrapper* tick);
    /// The pitch offset value at a given tick, determined by active ottavas.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE int pitchOffset(apiv1::FractionWrapper* tick);

    /// For staves in part scores: Whether the given voice
    /// is displayed in the score.
    /// \param voice The voice number (0-3) to check.
    /// \since MuseScore 4.6
    Q_INVOKABLE bool isVoiceVisible(int voice);
};

//---------------------------------------------------------
//   SpannerSegment
///  Provides access to internal mu::engraving::SpannerSegment objects.
///  \since MuseScore 4.6
//---------------------------------------------------------

class SpannerSegment : public EngravingItem
{
    Q_OBJECT
    /// The spanner object of this spanner segment.
    /// \see PluginAPI::Spanner
    Q_PROPERTY(apiv1::Spanner * spanner READ spanner)
    /// The spanner segment type of this spanner segment,
    /// one of PluginAPI::PluginAPI::SpannerSegmentType values.
    Q_PROPERTY(int spannerSegmentType READ spannerSegmentType)
    /// Position of the spanner segment's end part,
    /// including manual offset through \ref userOff2.
    /// \see EngravingItem::userOff2
    Q_PROPERTY(QPointF pos2 READ pos2)
    /// The manual offset of the spanner segment's end part.
    API_PROPERTY_T(QPointF, userOff2,     OFFSET2)

    /// \cond MS_INTERNAL

public:
    SpannerSegment(mu::engraving::SpannerSegment* spannerSegment, Ownership own = Ownership::PLUGIN)
        : EngravingItem(spannerSegment, own) {}

    mu::engraving::SpannerSegment* spannerSegment() { return toSpannerSegment(e); }
    const mu::engraving::SpannerSegment* spannerSegment() const { return toSpannerSegment(e); }

    Spanner* spanner() { return wrap<Spanner>(spannerSegment()->spanner()); }
    int spannerSegmentType() { return int(spannerSegment()->spannerSegmentType()); }
    QPointF pos2() const { return PointF(spannerSegment()->pos2() / spannerSegment()->spatium()).toQPointF(); }

    /// \endcond
};

//---------------------------------------------------------
//   Spanner
///  Provides access to internal mu::engraving::Spanner objects.
///  \since MuseScore 4.6
//---------------------------------------------------------

class Spanner : public EngravingItem
{
    Q_OBJECT
    /// The tick this spanner starts at.
    API_PROPERTY(spannerTick,             SPANNER_TICK)
    /// The tick this spanner end at.
    API_PROPERTY(spannerTicks,            SPANNER_TICKS)
    /// The track this spanner end at.
    API_PROPERTY_T(int, spannerTrack2,    SPANNER_TRACK2)

    /// The Anchor type for this spanner,
    /// one of PluginAPI::PluginAPI::Anchor values.
    API_PROPERTY_T(int, anchor,           ANCHOR)
    /// For slur and tie segments:
    /// The manual offset applied to the first point.
    API_PROPERTY_T(QPointF, slurUoff1,    SLUR_UOFF1)
    /// For slur and tie segments:
    /// The manual offset applied to the second point.
    API_PROPERTY_T(QPointF, slurUoff2,    SLUR_UOFF2)
    /// For slur and tie segments:
    /// The manual offset applied to the third point.
    API_PROPERTY_T(QPointF, slurUoff3,    SLUR_UOFF3)
    /// For slur and tie segments:
    /// The manual offset applied to the fourth point.
    API_PROPERTY_T(QPointF, slurUoff4,    SLUR_UOFF4)

    /// The starting element of the spanner.
    Q_PROPERTY(apiv1::EngravingItem * startElement READ startElement)
    /// The ending element of the spanner.
    Q_PROPERTY(apiv1::EngravingItem * endElement READ endElement)
    /// List of spanner segments belonging to this spanner.
    Q_PROPERTY(QQmlListProperty<apiv1::SpannerSegment> spannerSegments READ spannerSegments)

    /// \cond MS_INTERNAL

public:
    Spanner(mu::engraving::Spanner* spanner, Ownership own = Ownership::PLUGIN)
        : EngravingItem(spanner, own) {}

    mu::engraving::Spanner* spanner() { return toSpanner(e); }
    const mu::engraving::Spanner* spanner() const { return toSpanner(e); }

    EngravingItem* startElement() const { return wrap(spanner()->startElement()); }
    EngravingItem* endElement() const { return wrap(spanner()->startElement()); }

    QQmlListProperty<SpannerSegment> spannerSegments()
    {
        return wrapContainerProperty<SpannerSegment>(this, spanner()->spannerSegments());
    }

    /// \endcond
};

//---------------------------------------------------------
//   Tie
///  Provides access to internal mu::engraving::Tie objects.
///  \since MuseScore 3.3
//---------------------------------------------------------

class Tie : public Spanner
{
    Q_OBJECT
    /// The starting note of the tie.
    Q_PROPERTY(apiv1::Note * startNote READ startNote)
    /// The ending note of the tie.
    Q_PROPERTY(apiv1::Note * endNote READ endNote)
    /// Whether the placement is inside.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isInside READ isInside)

    /// \cond MS_INTERNAL

public:
    Tie(mu::engraving::Tie* tie, Ownership own = Ownership::PLUGIN)
        : Spanner(tie, own) {}

    mu::engraving::Tie* tie() { return toTie(e); }
    const mu::engraving::Tie* tie() const { return toTie(e); }

    Note* startNote() const { return wrap<Note>(tie()->startNote()); }
    Note* endNote() const { return wrap<Note>(tie()->startNote()); }
    bool isInside() const { return tie()->isInside(); }

    /// \endcond
};

#undef API_PROPERTY
#undef API_PROPERTY_T
#undef API_PROPERTY_READ_ONLY
#undef API_PROPERTY_READ_ONLY_T
}
