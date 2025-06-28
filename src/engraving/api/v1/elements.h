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
#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/notedot.h"
#include "engraving/dom/page.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/stemslash.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/undo.h"

#include "playevent.h"

Q_MOC_INCLUDE("engraving/api/v1/part.h")
Q_MOC_INCLUDE("engraving/api/v1/tie.h")

namespace mu::engraving::apiv1 {
class FractionWrapper;
class EngravingItem;
class Part;
class Staff;
class Tie;
class Tuplet;

extern Tie* tieWrap(mu::engraving::Tie* tie);

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

extern EngravingItem* wrap(mu::engraving::EngravingItem* se, Ownership own = Ownership::SCORE);

// TODO: add RESET functions
#define API_PROPERTY(name, pid) \
    Q_PROPERTY(QVariant name READ get_##name WRITE set_##name) \
    QVariant get_##name() const { return get(mu::engraving::Pid::pid); \
    }  \
    void set_##name(QVariant val) { set(mu::engraving::Pid::pid, val); }

/**
 * API_PROPERTY flavor which allows to define the property type.
 * Can be used if it is known that this property is always valid
 * for this type, otherwise this macro won't allow an `undefined`
 * value to be exposed to QML in case of invalid property.
 */
#define API_PROPERTY_T(type, name, pid) \
    Q_PROPERTY(type name READ get_##name WRITE set_##name) \
    type get_##name() const { return get(mu::engraving::Pid::pid).value<type>(); }  \
    void set_##name(type val) { set(mu::engraving::Pid::pid, QVariant::fromValue(val)); }

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

    /**
     * Parent element for this element.
     * \since 3.3
     */
    Q_PROPERTY(apiv1::EngravingItem * parent READ parent)
    /**
     * Staff which this element belongs to.
     * \since MuseScore 3.5
     */
    Q_PROPERTY(apiv1::Staff * staff READ staff)
    /**
     * X-axis offset from a reference position in spatium units.
     * \see EngravingItem::offset
     */
    Q_PROPERTY(qreal offsetX READ offsetX WRITE setOffsetX)
    /**
     * Y-axis offset from a reference position in spatium units.
     * \see EngravingItem::offset
     */
    Q_PROPERTY(qreal offsetY READ offsetY WRITE setOffsetY)
    /**
     * Reference position of this element relative to its parent element.
     *
     * This is an offset from the parent object that is determined by the
     * autoplace feature. It includes any other offsets applied to the
     * element. You can use this value to accurately position other elements
     * related to the same parent.
     *
     * This value is in spatium units for compatibility with EngravingItem.offsetX.
     * \since MuseScore 3.3
     */
    Q_PROPERTY(qreal posX READ posX)
    /**
     * Reference position of this element relative to its parent element.
     *
     * This is an offset from the parent object that is determined by the
     * autoplace feature. It includes any other offsets applied to the
     * element. You can use this value to accurately position other elements
     * related to the same parent.
     *
     * This value is in spatium units for compatibility with EngravingItem.offsetY.
     * \since MuseScore 3.3
     */
    Q_PROPERTY(qreal posY READ posY)
    /**
     * Position of this element in page coordinates, in spatium units.
     * \since MuseScore 3.5
     */
    Q_PROPERTY(QPointF pagePos READ pagePos)

    /**
     * Bounding box of this element.
     *
     * This value is in spatium units for compatibility with other EngravingItem positioning properties.
     * \since MuseScore 3.3.1
     */
    Q_PROPERTY(QRectF bbox READ bbox)

    /**
     * Subtype of this element.
     * \since MuseScore 4.6
     */
    Q_PROPERTY(int subtype READ subtype)
    /**
     * Unlike the name might suggest, this property no longer returns the subtype and is scarcely used.
     * Named 'subtype' prior to MuseScore 4.6
     */
    API_PROPERTY(subType,                 SUBTYPE)
    API_PROPERTY_READ_ONLY_T(bool, selected, SELECTED)
    API_PROPERTY_READ_ONLY_T(bool, generated, GENERATED)
    /**
     * EngravingItem color. See https://doc.qt.io/qt-5/qml-color.html
     * for the reference on color type in QML.
     */
    API_PROPERTY_T(QColor, color,         COLOR)
    API_PROPERTY_T(bool,   visible,       VISIBLE)
    /** Stacking order of this element */
    API_PROPERTY_T(int,    z,             Z)
    API_PROPERTY(small,                   SMALL)
    API_PROPERTY(showCourtesy,            SHOW_COURTESY)
    API_PROPERTY(lineType,                SLUR_STYLE_TYPE)
    API_PROPERTY(line,                    LINE)
    API_PROPERTY(fixed,                   FIXED)
    API_PROPERTY(fixedLine,               FIXED_LINE)
    /** Notehead type, one of PluginAPI::PluginAPI::NoteHeadType values */
    API_PROPERTY(headType,                HEAD_TYPE)
    /**
     * Notehead scheme, one of PluginAPI::PluginAPI::NoteHeadScheme values.
     * \since MuseScore 3.5
     */
    API_PROPERTY(headScheme,              HEAD_SCHEME)
    /** Notehead group, one of PluginAPI::PluginAPI::NoteHeadGroup values */
    API_PROPERTY(headGroup,               HEAD_GROUP)
    API_PROPERTY(articulationAnchor,      ARTICULATION_ANCHOR)
    API_PROPERTY(direction,               DIRECTION)
    API_PROPERTY(stemDirection,           STEM_DIRECTION)
    API_PROPERTY(noStem,                  NO_STEM)
    API_PROPERTY(slurDirection,           SLUR_DIRECTION)
    API_PROPERTY(leadingSpace,            LEADING_SPACE)
    API_PROPERTY(mirrorHead,              MIRROR_HEAD)
    API_PROPERTY(dotPosition,             DOT_POSITION)
    API_PROPERTY(tuning,                  TUNING)
    API_PROPERTY(pause,                   PAUSE)
    API_PROPERTY(barlineType,             BARLINE_TYPE)
    API_PROPERTY(barlineSpan,             BARLINE_SPAN)
    API_PROPERTY(barlineSpanFrom,         BARLINE_SPAN_FROM)
    API_PROPERTY(barlineSpanTo,           BARLINE_SPAN_TO)
    /**
     * Offset from a reference position in spatium units.
     * Use `Qt.point(x, y)` to create a point value which can be
     * assigned to this property.
     * \see EngravingItem::offsetX
     * \see EngravingItem::offsetY
     */
    API_PROPERTY_T(QPointF, offset,       OFFSET)
    API_PROPERTY(fret,                    FRET)
    API_PROPERTY(string,                  STRING)
    API_PROPERTY(ghost,                   GHOST)
    API_PROPERTY(play,                    PLAY)
    API_PROPERTY(timesigNominal,          TIMESIG_NOMINAL)
    API_PROPERTY(timesigActual,           TIMESIG_ACTUAL)
    API_PROPERTY(growLeft,                GROW_LEFT)
    API_PROPERTY(growRight,               GROW_RIGHT)
    API_PROPERTY(boxHeight,               BOX_HEIGHT)
    API_PROPERTY(boxWidth,                BOX_WIDTH)
    API_PROPERTY(topGap,                  TOP_GAP)
    API_PROPERTY(bottomGap,               BOTTOM_GAP)
    API_PROPERTY(leftMargin,              LEFT_MARGIN)
    API_PROPERTY(rightMargin,             RIGHT_MARGIN)
    API_PROPERTY(topMargin,               TOP_MARGIN)
    API_PROPERTY(bottomMargin,            BOTTOM_MARGIN)
    API_PROPERTY(layoutBreakType,         LAYOUT_BREAK)
    API_PROPERTY(autoscale,               AUTOSCALE)
    API_PROPERTY(size,                    SIZE)
    API_PROPERTY(scale,                   SCALE)
    API_PROPERTY(lockAspectRatio,         LOCK_ASPECT_RATIO)
    API_PROPERTY(sizeIsSpatium,           SIZE_IS_SPATIUM)
    API_PROPERTY(text,                    TEXT)
    API_PROPERTY(beamPos,                 BEAM_POS)
    API_PROPERTY(beamMode,                BEAM_MODE)
    API_PROPERTY(beamNoSlope,             BEAM_NO_SLOPE)
    API_PROPERTY(userLen,                 USER_LEN)
    /** For spacers: amount of space between staves. */
    API_PROPERTY(space,                   SPACE)
    API_PROPERTY(tempo,                   TEMPO)
    API_PROPERTY(tempoFollowText,         TEMPO_FOLLOW_TEXT)
    API_PROPERTY(accidentalBracket,       ACCIDENTAL_BRACKET)
    API_PROPERTY(numeratorString,         NUMERATOR_STRING)
    API_PROPERTY(denominatorString,       DENOMINATOR_STRING)
    API_PROPERTY(fbprefix,                FBPREFIX)
    API_PROPERTY(fbdigit,                 FBDIGIT)
    API_PROPERTY(fbsuffix,                FBSUFFIX)
    API_PROPERTY(fbcontinuationline,      FBCONTINUATIONLINE)
    API_PROPERTY(ottavaType,              OTTAVA_TYPE)
    API_PROPERTY(numbersOnly,             NUMBERS_ONLY)
    API_PROPERTY(trillType,               TRILL_TYPE)
    API_PROPERTY(vibratoType,             VIBRATO_TYPE)
    API_PROPERTY(hairpinCircledTip,       HAIRPIN_CIRCLEDTIP)
    API_PROPERTY(hairpinType,             HAIRPIN_TYPE)
    API_PROPERTY(hairpinHeight,           HAIRPIN_HEIGHT)
    API_PROPERTY(hairpinContHeight,       HAIRPIN_CONT_HEIGHT)
    API_PROPERTY(veloChange,              VELO_CHANGE)
    API_PROPERTY(singleNoteDynamics,      SINGLE_NOTE_DYNAMICS)
    API_PROPERTY(veloChangeMethod,        VELO_CHANGE_METHOD)
    API_PROPERTY(veloChangeSpeed,         VELO_CHANGE_SPEED)
    API_PROPERTY(dynamicRange,            END) // obsolete
    /**
     *    The way a ramp interpolates between values.
     *    \since MuseScore 3.5
     */
    API_PROPERTY(changeMethod,            CHANGE_METHOD)
    API_PROPERTY(placement,               PLACEMENT)
    API_PROPERTY(velocity,                VELOCITY)
    API_PROPERTY(jumpTo,                  JUMP_TO)
    API_PROPERTY(playUntil,               PLAY_UNTIL)
    API_PROPERTY(continueAt,              CONTINUE_AT)
    API_PROPERTY(label,                   LABEL)
    API_PROPERTY(markerType,              MARKER_TYPE)
    API_PROPERTY(arpUserLen1,             ARP_USER_LEN1)
    API_PROPERTY(arpUserLen2,             ARP_USER_LEN2)
    API_PROPERTY(measureNumberMode,       MEASURE_NUMBER_MODE)
    API_PROPERTY(glissType,               GLISS_TYPE)
    API_PROPERTY(glissText,               GLISS_TEXT)
    API_PROPERTY(glissShowText,           GLISS_SHOW_TEXT)
    API_PROPERTY(glissandoStyle,          GLISS_STYLE)
    API_PROPERTY(glissEaseIn,             GLISS_EASEIN)
    API_PROPERTY(glissEaseOut,            GLISS_EASEOUT)
    API_PROPERTY(diagonal,                DIAGONAL)
    API_PROPERTY(groups,                  GROUP_NODES)
    API_PROPERTY(lineStyle,               LINE_STYLE)
    API_PROPERTY(lineColor,               COLOR)
    API_PROPERTY(lineWidth,               LINE_WIDTH)
    API_PROPERTY(timeStretch,             TIME_STRETCH)
    API_PROPERTY(ornamentStyle,           ORNAMENT_STYLE)
    API_PROPERTY(timesig,                 TIMESIG)
    API_PROPERTY(timesigStretch,          TIMESIG_STRETCH)
    API_PROPERTY(timesigType,             TIMESIG_TYPE)
    API_PROPERTY(spannerTick,             SPANNER_TICK)
    API_PROPERTY(spannerTicks,            SPANNER_TICKS)
    API_PROPERTY(spannerTrack2,           SPANNER_TRACK2)
    API_PROPERTY(userOff2,                OFFSET2)
    API_PROPERTY(breakMmr,                BREAK_MMR)
    API_PROPERTY(repeatCount,             REPEAT_COUNT)
    API_PROPERTY(userStretch,             USER_STRETCH)
    API_PROPERTY(noOffset,                NO_OFFSET)
    API_PROPERTY(irregular,               IRREGULAR)
    API_PROPERTY(anchor,                  ANCHOR)
    API_PROPERTY(slurUoff1,               SLUR_UOFF1)
    API_PROPERTY(slurUoff2,               SLUR_UOFF2)
    API_PROPERTY(slurUoff3,               SLUR_UOFF3)
    API_PROPERTY(slurUoff4,               SLUR_UOFF4)
    API_PROPERTY(staffMove,               STAFF_MOVE)
    API_PROPERTY(verse,                   VERSE)
    API_PROPERTY(syllabic,                SYLLABIC)
    API_PROPERTY(lyricTicks,              LYRIC_TICKS)
    API_PROPERTY(volta_ending,            VOLTA_ENDING)
    API_PROPERTY(lineVisible,             LINE_VISIBLE)
    API_PROPERTY(mag,                     MAG)
    API_PROPERTY(useDrumset,              USE_DRUMSET)
    API_PROPERTY(durationType,            DURATION_TYPE_WITH_DOTS)
    API_PROPERTY(role,                    ACCIDENTAL_ROLE)
    API_PROPERTY_T(int, track,            TRACK)
    API_PROPERTY(fretStrings,             FRET_STRINGS)
    API_PROPERTY(fretFrets,               FRET_FRETS)
    /*API_PROPERTY( fretBarre,               FRET_BARRE                )*/
    API_PROPERTY(fretOffset,              FRET_OFFSET)
    API_PROPERTY(fretNumPos,              FRET_NUM_POS)
    API_PROPERTY(systemBracket,           SYSTEM_BRACKET)
    API_PROPERTY(gap,                     GAP)
    /** Whether this element participates in autoplacement */
    API_PROPERTY_T(bool, autoplace,       AUTOPLACE)
    API_PROPERTY(dashLineLen,             DASH_LINE_LEN)
    API_PROPERTY(dashGapLen,              DASH_GAP_LEN)
//       API_PROPERTY_READ_ONLY( tick,          TICK                      ) // wasn't available in 2.X, disabled due to fractions transition
    /**
     * Symbol ID of this element (if appropriate),
     * one of PluginAPI::PluginAPI::SymId values.
     */
    API_PROPERTY(symbol,                  SYMBOL)
    API_PROPERTY(playRepeats,             PLAY_REPEATS)
    API_PROPERTY(createSystemHeader,      CREATE_SYSTEM_HEADER)
    API_PROPERTY(staffLines,              STAFF_LINES)
    API_PROPERTY(lineDistance,            LINE_DISTANCE)
    API_PROPERTY(stepOffset,              STEP_OFFSET)
    API_PROPERTY(staffShowBarlines,       STAFF_SHOW_BARLINES)
    API_PROPERTY(staffShowLedgerlines,    STAFF_SHOW_LEDGERLINES)
    API_PROPERTY(staffStemless,           STAFF_STEMLESS)
    API_PROPERTY(staffGenClef,            STAFF_GEN_CLEF)
    API_PROPERTY(staffGenTimesig,         STAFF_GEN_TIMESIG)
    API_PROPERTY(staffGenKeysig,          STAFF_GEN_KEYSIG)
    API_PROPERTY(staffYoffset,            STAFF_YOFFSET)
    API_PROPERTY(bracketSpan,             BRACKET_SPAN)
    API_PROPERTY(bracketColumn,           BRACKET_COLUMN)
    API_PROPERTY(inameLayoutPosition,     INAME_LAYOUT_POSITION)
    API_PROPERTY(subStyle,                TEXT_STYLE)
    API_PROPERTY(fontFace,                FONT_FACE)
    API_PROPERTY(fontSize,                FONT_SIZE)
    API_PROPERTY(fontStyle,               FONT_STYLE)
    API_PROPERTY(frameType,               FRAME_TYPE)
    API_PROPERTY(frameWidth,              FRAME_WIDTH)
    API_PROPERTY(framePadding,            FRAME_PADDING)
    API_PROPERTY(frameRound,              FRAME_ROUND)
    API_PROPERTY(frameFgColor,            FRAME_FG_COLOR)
    API_PROPERTY(frameBgColor,            FRAME_BG_COLOR)
    API_PROPERTY(sizeSpatiumDependent,    SIZE_SPATIUM_DEPENDENT)
    API_PROPERTY(align,                   ALIGN)
    API_PROPERTY(systemFlag,              SYSTEM_FLAG)
    API_PROPERTY(beginText,               BEGIN_TEXT)
    API_PROPERTY(beginTextAlign,          BEGIN_TEXT_ALIGN)
    API_PROPERTY(beginTextPlace,          BEGIN_TEXT_PLACE)
    API_PROPERTY(beginHookType,           BEGIN_HOOK_TYPE)
    API_PROPERTY(beginHookHeight,         BEGIN_HOOK_HEIGHT)
    API_PROPERTY(beginFontFace,           BEGIN_FONT_FACE)
    API_PROPERTY(beginFontSize,           BEGIN_FONT_SIZE)
    API_PROPERTY(beginFontStyle,          BEGIN_FONT_STYLE)
    API_PROPERTY(beginTextOffset,         BEGIN_TEXT_OFFSET)
    API_PROPERTY(continueText,            CONTINUE_TEXT)
    API_PROPERTY(continueTextAlign,       CONTINUE_TEXT_ALIGN)
    API_PROPERTY(continueTextPlace,       CONTINUE_TEXT_PLACE)
    API_PROPERTY(continueFontFace,        CONTINUE_FONT_FACE)
    API_PROPERTY(continueFontSize,        CONTINUE_FONT_SIZE)
    API_PROPERTY(continueFontStyle,       CONTINUE_FONT_STYLE)
    API_PROPERTY(continueTextOffset,      CONTINUE_TEXT_OFFSET)
    API_PROPERTY(endText,                 END_TEXT)
    API_PROPERTY(endTextAlign,            END_TEXT_ALIGN)
    API_PROPERTY(endTextPlace,            END_TEXT_PLACE)
    API_PROPERTY(endHookType,             END_HOOK_TYPE)
    API_PROPERTY(endHookHeight,           END_HOOK_HEIGHT)
    API_PROPERTY(endFontFace,             END_FONT_FACE)
    API_PROPERTY(endFontSize,             END_FONT_SIZE)
    API_PROPERTY(endFontStyle,            END_FONT_STYLE)
    API_PROPERTY(endTextOffset,           END_TEXT_OFFSET)
    API_PROPERTY(posAbove,                POS_ABOVE)
    API_PROPERTY_T(int, voice,            VOICE)
    API_PROPERTY(position,                POSITION)
    /**
     * For chord symbols, chord symbol type, one of
     * PluginAPI::PluginAPI::HarmonyType values.
     * \since MuseScore 3.6
     */
    API_PROPERTY(harmonyType,             HARMONY_TYPE)

    qreal offsetX() const { return element()->offset().x() / element()->spatium(); }
    qreal offsetY() const { return element()->offset().y() / element()->spatium(); }
    void setOffsetX(qreal offX);
    void setOffsetY(qreal offY);

    qreal posX() const { return element()->pos().x() / element()->spatium(); }
    qreal posY() const { return element()->pos().y() / element()->spatium(); }

    QPointF pagePos() const { return PointF(element()->pagePos() / element()->spatium()).toQPointF(); }

    apiv1::EngravingItem* parent() const { return wrap(element()->parentItem()); }
    Staff* staff() { return wrap<Staff>(element()->staff()); }

    QRectF bbox() const;

    int subtype() const { return element()->subtype(); }

public:
    /// \cond MS_INTERNAL
    EngravingItem(mu::engraving::EngravingItem* e = nullptr, Ownership own = Ownership::PLUGIN)
        : apiv1::ScoreElement(e, own) {}

    /// \brief muse::Returns the underlying mu::engraving::EngravingItem
    /// \{
    mu::engraving::EngravingItem* element() { return toEngravingItem(e); }
    const mu::engraving::EngravingItem* element() const { return toEngravingItem(e); }
    /// \}
    /// \endcond

    /// Create a copy of the element
    Q_INVOKABLE apiv1::EngravingItem* clone() const { return wrap(element()->clone(), Ownership::PLUGIN); }

    Q_INVOKABLE QString subtypeName() const { return element()->translatedSubtypeUserName().toQString(); }
    /// Deprecated: same as ScoreElement::name. Left for compatibility purposes.
    Q_INVOKABLE QString _name() const { return name(); }
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
    /** List of dots attached to this note */
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> dots READ dots)
//       Q_PROPERTY(int                            dotsCount         READ qmlDotsCount)
    /** List of other elements attached to this note: fingerings, symbols, bends etc. */
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)
    /// List of PlayEvents associated with this note.
    /// Important: You must call Score.createPlayEvents()
    /// to see meaningful data in the PlayEvent lists.
    /// \since MuseScore 3.3
    Q_PROPERTY(QQmlListProperty<apiv1::PlayEvent> playEvents READ playEvents)
//       Q_PROPERTY(int                            fret              READ fret               WRITE undoSetFret)
//       Q_PROPERTY(bool                           ghost             READ ghost              WRITE undoSetGhost)
//       Q_PROPERTY(mu::engraving::NoteHead::Group            headGroup         READ headGroup          WRITE undoSetHeadGroup)
//       Q_PROPERTY(mu::engraving::NoteHead::Type             headType          READ headType           WRITE undoSetHeadType)
//       Q_PROPERTY(bool                           hidden            READ hidden)
//       Q_PROPERTY(int                            line              READ line)
//       Q_PROPERTY(bool                           mirror            READ mirror)
//       Q_PROPERTY(int                            pitch             READ pitch              WRITE undoSetPitch)
//       Q_PROPERTY(bool                           play              READ play               WRITE undoSetPlay)
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

    /**
     * MIDI pitch of this note
     * \see \ref pitch
     */
    API_PROPERTY_T(int, pitch,                   PITCH)
    /**
     * Concert pitch of the note
     * \see \ref tpc
     */
    API_PROPERTY_T(int, tpc1,             TPC1)
    /**
     * Transposing pitch of the note
     * \see \ref tpc
     */
    API_PROPERTY_T(int, tpc2,             TPC2)
    /**
     * Concert or transposing pitch of this note,
     * as per current "Concert Pitch" setting value.
     * \see \ref tpc
     */
    Q_PROPERTY(int tpc READ tpc WRITE setTpc)
//       Q_PROPERTY(qreal                          tuning            READ tuning             WRITE undoSetTuning)
//       Q_PROPERTY(mu::engraving::MScore::Direction          userDotPosition   READ userDotPosition    WRITE undoSetUserDotPosition)
//       Q_PROPERTY(mu::engraving::DirectionH         userMirror        READ userMirror         WRITE undoSetUserMirror)
    /** See PluginAPI::PluginAPI::NoteValueType */
    API_PROPERTY(veloType,                VELO_TYPE)
    API_PROPERTY_T(int, userVelocity,       USER_VELOCITY)

public:
    /// \cond MS_INTERNAL
    Note(mu::engraving::Note* c = nullptr, Ownership own = Ownership::PLUGIN)
        : EngravingItem(c, own) {}

    mu::engraving::Note* note() { return toNote(e); }
    const mu::engraving::Note* note() const { return toNote(e); }

    int tpc() const { return note()->tpc(); }
    void setTpc(int val);

    apiv1::Tie* tieBack()    const
    {
        return note()->tieBack() != nullptr ? tieWrap(note()->tieBack()) : nullptr;
    }

    apiv1::Tie* tieForward() const { return note()->tieFor() != nullptr ? tieWrap(note()->tieFor()) : nullptr; }

    apiv1::Note* firstTiedNote() { return wrap<Note>(note()->firstTiedNote()); }
    apiv1::Note* lastTiedNote() { return wrap<Note>(note()->lastTiedNote()); }

    QQmlListProperty<EngravingItem> dots() { return wrapContainerProperty<EngravingItem>(this, note()->dots()); }
    QQmlListProperty<EngravingItem> elements() { return wrapContainerProperty<EngravingItem>(this, note()->el()); }
    QQmlListProperty<PlayEvent> playEvents() { return wrapPlayEventsContainerProperty(this, note()->playEvents()); }

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

    /**
    * Nominal duration of this element.
    * The duration is represented as a fraction of whole note length.
    */
    API_PROPERTY_READ_ONLY(duration, DURATION)

    /**
    * Global duration of this element, taking into account ratio of
    * parent tuplets if there are any.
    * \since MuseScore 3.5
    */
    Q_PROPERTY(apiv1::FractionWrapper * globalDuration READ globalDuration)

    /**
    * Actual duration of this element, taking into account ratio of
    * parent tuplets and local time signatures if there are any.
    * \since MuseScore 3.5
    */
    Q_PROPERTY(apiv1::FractionWrapper * actualDuration READ actualDuration)

    /**
    * Tuplet which this element belongs to. If there is no parent tuplet, returns null.
    * \since MuseScore 3.5
    */
    Q_PROPERTY(apiv1::Tuplet * tuplet READ parentTuplet)

public:
    /// \cond MS_INTERNAL
    DurationElement(mu::engraving::DurationElement* de = nullptr, Ownership own = Ownership::PLUGIN)
        : EngravingItem(de, own) {}

    mu::engraving::DurationElement* durationElement() { return toDurationElement(e); }
    const mu::engraving::DurationElement* durationElement() const { return toDurationElement(e); }

    FractionWrapper* globalDuration() const;
    FractionWrapper* actualDuration() const;

    Tuplet* parentTuplet();
    /// \endcond
};

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

class Tuplet : public DurationElement
{
    Q_OBJECT

    API_PROPERTY(numberType, NUMBER_TYPE)
    API_PROPERTY(bracketType, BRACKET_TYPE)

    /** Actual number of notes of base nominal length in this tuplet. */
    API_PROPERTY_READ_ONLY_T(int, actualNotes, ACTUAL_NOTES)

    /**
    * Number of "normal" notes of base nominal length which correspond
    * to this tuplet's duration.
    */
    API_PROPERTY_READ_ONLY_T(int, normalNotes, NORMAL_NOTES)

    API_PROPERTY(p1, P1)
    API_PROPERTY(p2, P2)

    /**
    * List of elements which belong to this tuplet.
    * \since MuseScore 3.5
    */
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)

public:
    /// \cond MS_INTERNAL
    Tuplet(mu::engraving::Tuplet* t = nullptr, Ownership own = Ownership::PLUGIN)
        : DurationElement(t, own) {}

    mu::engraving::Tuplet* tuplet() { return toTuplet(e); }
    const mu::engraving::Tuplet* tuplet() const { return toTuplet(e); }

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
    /**
     * Lyrics corresponding to this chord or rest, if any.
     * Before 3.6 version this property was only available for \ref Chord objects.
     */
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> lyrics READ lyrics)
    /**
     * Beam which covers this chord/rest, if such exists.
     * \since MuseScore 3.6
     */
    Q_PROPERTY(apiv1::EngravingItem * beam READ beam)

public:
    /// \cond MS_INTERNAL
    ChordRest(mu::engraving::ChordRest* c = nullptr, Ownership own = Ownership::PLUGIN)
        : DurationElement(c, own) {}

    mu::engraving::ChordRest* chordRest() { return toChordRest(e); }

    QQmlListProperty<EngravingItem> lyrics() { return wrapContainerProperty<EngravingItem>(this, chordRest()->lyrics()); }   // TODO: special type for Lyrics?
    EngravingItem* beam() { return wrap(chordRest()->beam()); }
    /// \endcond
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

public:
    /// \cond MS_INTERNAL
    Chord(mu::engraving::Chord* c = nullptr, Ownership own = Ownership::PLUGIN)
        : ChordRest(c, own) {}

    mu::engraving::Chord* chord() { return toChord(e); }
    const mu::engraving::Chord* chord() const { return toChord(e); }

    QQmlListProperty<Chord> graceNotes() { return wrapContainerProperty<Chord>(this, chord()->graceNotes()); }
    QQmlListProperty<Note> notes() { return wrapContainerProperty<Note>(this, chord()->notes()); }
    QQmlListProperty<EngravingItem> articulations() { return wrapContainerProperty<EngravingItem>(this, chord()->articulations()); }
    EngravingItem* stem() { return wrap(chord()->stem()); }
    EngravingItem* stemSlash() { return wrap(chord()->stemSlash()); }
    EngravingItem* hook() { return wrap(chord()->hook()); }
    mu::engraving::NoteType noteType() { return chord()->noteType(); }
    mu::engraving::PlayEventType playEventType() { return chord()->playEventType(); }
    void setPlayEventType(mu::engraving::PlayEventType v);

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
    /**
     * The list of annotations. Articulations, staff/system/expression
     * text are examples of what is considered to be segment annotations.
     */
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
    Q_PROPERTY(mu::engraving::SegmentType segmentType READ segmentType)
    /**
     * \brief Current tick for this segment
     * \returns Tick of this segment, i.e. number of ticks from the beginning
     * of the score to this segment.
     * \see \ref ticklength
     */
    Q_PROPERTY(int tick READ tick)                               // TODO: revise engraving (or this API):
                                                                 // Pid::TICK is relative or absolute in different contexts

public:
    /// \cond MS_INTERNAL
    Segment(mu::engraving::Segment* s = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(s, own) {}

    mu::engraving::Segment* segment() { return toSegment(e); }
    const mu::engraving::Segment* segment() const { return toSegment(e); }

    int tick() const { return segment()->tick().ticks(); }

    mu::engraving::SegmentType segmentType() const { return segment()->segmentType(); }

    Segment* nextInScore() { return wrap<Segment>(segment()->next1()); }
    Segment* nextInMeasure() { return wrap<Segment>(segment()->next()); }
    Segment* prevInScore() { return wrap<Segment>(segment()->prev1()); }
    Segment* prevInMeasure() { return wrap<Segment>(segment()->prev()); }
    QQmlListProperty<EngravingItem> annotations() { return wrapContainerProperty<EngravingItem>(this, segment()->annotations()); }
    /// \endcond

    /// \return EngravingItem at the given \p track (null if there is no such an element)
    /// \param track track number
    Q_INVOKABLE apiv1::EngravingItem* elementAt(int track);
};

//---------------------------------------------------------
//   Measure
//    Measure wrapper
//---------------------------------------------------------

class Measure : public EngravingItem
{
    Q_OBJECT
    /// The first segment of this measure
    Q_PROPERTY(apiv1::Segment * firstSegment READ firstSegment)
    /// The last segment of this measure
    Q_PROPERTY(apiv1::Segment * lastSegment READ lastSegment)

    // TODO: to MeasureBase?
//       Q_PROPERTY(bool         lineBreak         READ lineBreak   WRITE undoSetLineBreak)
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
//       Q_PROPERTY(bool         pageBreak         READ pageBreak   WRITE undoSetPageBreak)
    /// Previous measure.
    Q_PROPERTY(apiv1::Measure * prevMeasure READ prevMeasure)
    /// Previous measure, accounting for multimeasure rests.
    /// See \ref nextMeasureMM for a reference on multimeasure rests.
    /// \see \ref Score.lastMeasureMM
    /// \since MuseScore 3.6
    Q_PROPERTY(apiv1::Measure * prevMeasureMM READ prevMeasureMM)

    /// List of measure-related elements: layout breaks, jump/repeat markings etc.
    /// \since MuseScore 3.3
    Q_PROPERTY(QQmlListProperty<apiv1::EngravingItem> elements READ elements)

public:
    /// \cond MS_INTERNAL
    Measure(mu::engraving::Measure* m = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(m, own) {}

    mu::engraving::Measure* measure() { return toMeasure(e); }
    const mu::engraving::Measure* measure() const { return toMeasure(e); }

    Segment* firstSegment() { return wrap<Segment>(measure()->firstEnabled(), Ownership::SCORE); }
    Segment* lastSegment() { return wrap<Segment>(measure()->last(), Ownership::SCORE); }

    Measure* prevMeasure() { return wrap<Measure>(measure()->prevMeasure(), Ownership::SCORE); }
    Measure* nextMeasure() { return wrap<Measure>(measure()->nextMeasure(), Ownership::SCORE); }

    Measure* prevMeasureMM() { return wrap<Measure>(measure()->prevMeasureMM(), Ownership::SCORE); }
    Measure* nextMeasureMM() { return wrap<Measure>(measure()->nextMeasureMM(), Ownership::SCORE); }

    QQmlListProperty<EngravingItem> elements() { return wrapContainerProperty<EngravingItem>(this, measure()->el()); }
    /// \endcond
};

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page : public EngravingItem
{
    Q_OBJECT
    /**
     * \brief Page number, counting from 0.
     * Number of this page in the score counting from 0, i.e.
     * for the first page its \p pagenumber value will be equal to 0.
     * User-visible page number can be calculated as
     * \code
     * page.pagenumber + 1 + score.pageNumberOffset
     * \endcode
     * where \p score is the relevant \ref Score object.
     * \since MuseScore 3.5
     * \see Score::pageNumberOffset
     */
    Q_PROPERTY(int pagenumber READ pagenumber)

public:
    /// \cond MS_INTERNAL
    Page(mu::engraving::Page* p = nullptr, Ownership own = Ownership::SCORE)
        : EngravingItem(p, own) {}

    mu::engraving::Page* page() { return toPage(e); }
    const mu::engraving::Page* page() const { return toPage(e); }

    int pagenumber() const;
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
    /**
     * Staff color. See https://doc.qt.io/qt-5/qml-color.html
     * for the reference on color type in QML.
     */
    API_PROPERTY_T(QColor, color,         COLOR)

    /** Whether voice 1 participates in playback. */
    API_PROPERTY_T(bool, playbackVoice1,  PLAYBACK_VOICE1)
    /** Whether voice 2 participates in playback. */
    API_PROPERTY_T(bool, playbackVoice2,  PLAYBACK_VOICE2)
    /** Whether voice 3 participates in playback. */
    API_PROPERTY_T(bool, playbackVoice3,  PLAYBACK_VOICE3)
    /** Whether voice 4 participates in playback. */
    API_PROPERTY_T(bool, playbackVoice4,  PLAYBACK_VOICE4)

    API_PROPERTY_T(int, staffBarlineSpan,     STAFF_BARLINE_SPAN)
    API_PROPERTY_T(int, staffBarlineSpanFrom, STAFF_BARLINE_SPAN_FROM)
    API_PROPERTY_T(int, staffBarlineSpanTo,   STAFF_BARLINE_SPAN_TO)

    /**
     * User-defined amount of additional space before this staff.
     * It is recommended to consider adding a spacer instead as it
     * allows adjusting staff spacing locally as opposed to this
     * property.
     * \see \ref EngravingItem.space
     */
    API_PROPERTY_T(qreal, staffUserdist,  STAFF_USERDIST)

    /** Part which this staff belongs to. */
    Q_PROPERTY(apiv1::Part * part READ part);

public:
    /// \cond MS_INTERNAL
    Staff(mu::engraving::Staff* staff, Ownership own = Ownership::PLUGIN)
        : apiv1::ScoreElement(staff, own) {}

    mu::engraving::Staff* staff() { return toStaff(e); }
    const mu::engraving::Staff* staff() const { return toStaff(e); }

    Part* part();
    /// \endcond
};

#undef API_PROPERTY
#undef API_PROPERTY_T
#undef API_PROPERTY_READ_ONLY
#undef API_PROPERTY_READ_ONLY_T
}
