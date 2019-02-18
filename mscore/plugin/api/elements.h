//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_ELEMENTS_H__
#define __PLUGIN_API_ELEMENTS_H__

#include "scoreelement.h"
#include "libmscore/element.h"
#include "libmscore/chord.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/notedot.h"
#include "libmscore/segment.h"

namespace Ms {
namespace PluginAPI {

class Element;

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

extern Element* wrap(Ms::Element* se, Ownership own = Ownership::SCORE);

// TODO: add RESET functions
#define API_PROPERTY(name, pid) \
      Q_PROPERTY(QVariant name READ get_##name WRITE set_##name) \
      QVariant get_##name() const { return get(Ms::Pid::pid); }  \
      void set_##name(QVariant val) { set(Ms::Pid::pid, val); }

#define API_PROPERTY_READ_ONLY(name, pid) \
      Q_PROPERTY(QVariant name READ get_##name) \
      QVariant get_##name() const { return get(Ms::Pid::pid); }

//---------------------------------------------------------
//   Element
//    Element wrapper
//---------------------------------------------------------

class Element : public Ms::PluginAPI::ScoreElement {
      Q_OBJECT

      Q_PROPERTY(qreal offsetX READ offsetX WRITE setOffsetX)
      Q_PROPERTY(qreal offsetY READ offsetY WRITE setOffsetY)

      // TODO: move some of them to ScoreElement
      API_PROPERTY( subtype,                 SUBTYPE                   )
      API_PROPERTY_READ_ONLY( selected,      SELECTED                  )
      API_PROPERTY_READ_ONLY( generated,     GENERATED                 )
      API_PROPERTY( color,                   COLOR                     )
      API_PROPERTY( visible,                 VISIBLE                   )
      API_PROPERTY( z,                       Z                         )
      API_PROPERTY( small,                   SMALL                     )
      API_PROPERTY( showCourtesy,            SHOW_COURTESY             )
      API_PROPERTY( lineType,                LINE_TYPE                 )
      API_PROPERTY( pitch,                   PITCH                     )
      API_PROPERTY( tpc1,                    TPC1                      )
      API_PROPERTY( tpc2,                    TPC2                      )
      API_PROPERTY( line,                    LINE                      )
      API_PROPERTY( fixed,                   FIXED                     )
      API_PROPERTY( fixedLine,               FIXED_LINE                )
      API_PROPERTY( headType,                HEAD_TYPE                 )
      API_PROPERTY( headGroup,               HEAD_GROUP                )
      API_PROPERTY( veloType,                VELO_TYPE                 )
      API_PROPERTY( veloOffset,              VELO_OFFSET               )
      API_PROPERTY( articulationAnchor,      ARTICULATION_ANCHOR       )
      API_PROPERTY( direction,               DIRECTION                 )
      API_PROPERTY( stemDirection,           STEM_DIRECTION            )
      API_PROPERTY( noStem,                  NO_STEM                   )
      API_PROPERTY( slurDirection,           SLUR_DIRECTION            )
      API_PROPERTY( leadingSpace,            LEADING_SPACE             )
      API_PROPERTY( distribute,              DISTRIBUTE                )
      API_PROPERTY( mirrorHead,              MIRROR_HEAD               )
      API_PROPERTY( dotPosition,             DOT_POSITION              )
      API_PROPERTY( tuning,                  TUNING                    )
      API_PROPERTY( pause,                   PAUSE                     )
      API_PROPERTY( barlineType,             BARLINE_TYPE              )
      API_PROPERTY( barlineSpan,             BARLINE_SPAN              )
      API_PROPERTY( barlineSpanFrom,         BARLINE_SPAN_FROM         )
      API_PROPERTY( barlineSpanTo,           BARLINE_SPAN_TO           )
      API_PROPERTY( offset,                  OFFSET                    )
      API_PROPERTY( fret,                    FRET                      )
      API_PROPERTY( string,                  STRING                    )
      API_PROPERTY( ghost,                   GHOST                     )
      API_PROPERTY( play,                    PLAY                      )
      API_PROPERTY( timesigNominal,          TIMESIG_NOMINAL           )
      API_PROPERTY( timesigActual,           TIMESIG_ACTUAL            )
      API_PROPERTY( numberType,              NUMBER_TYPE               )
      API_PROPERTY( bracketType,             BRACKET_TYPE              )
      API_PROPERTY( normalNotes,             NORMAL_NOTES              )
      API_PROPERTY( actualNotes,             ACTUAL_NOTES              )
      API_PROPERTY( p1,                      P1                        )
      API_PROPERTY( p2,                      P2                        )
      API_PROPERTY( growLeft,                GROW_LEFT                 )
      API_PROPERTY( growRight,               GROW_RIGHT                )
      API_PROPERTY( boxHeight,               BOX_HEIGHT                )
      API_PROPERTY( boxWidth,                BOX_WIDTH                 )
      API_PROPERTY( topGap,                  TOP_GAP                   )
      API_PROPERTY( bottomGap,               BOTTOM_GAP                )
      API_PROPERTY( leftMargin,              LEFT_MARGIN               )
      API_PROPERTY( rightMargin,             RIGHT_MARGIN              )
      API_PROPERTY( topMargin,               TOP_MARGIN                )
      API_PROPERTY( bottomMargin,            BOTTOM_MARGIN             )
      API_PROPERTY( layoutBreakType,         LAYOUT_BREAK              )
      API_PROPERTY( autoscale,               AUTOSCALE                 )
      API_PROPERTY( size,                    SIZE                      )
      API_PROPERTY( scale,                   SCALE                     )
      API_PROPERTY( lockAspectRatio,         LOCK_ASPECT_RATIO         )
      API_PROPERTY( sizeIsSpatium,           SIZE_IS_SPATIUM           )
      API_PROPERTY( text,                    TEXT                      )
      API_PROPERTY( beamPos,                 BEAM_POS                  )
      API_PROPERTY( beamMode,                BEAM_MODE                 )
      API_PROPERTY( beamNoSlope,             BEAM_NO_SLOPE             )
      API_PROPERTY( userLen,                 USER_LEN                  )
      API_PROPERTY( space,                   SPACE                     )
      API_PROPERTY( tempo,                   TEMPO                     )
      API_PROPERTY( tempoFollowText,         TEMPO_FOLLOW_TEXT         )
      API_PROPERTY( accidentalBracket,       ACCIDENTAL_BRACKET        )
      API_PROPERTY( numeratorString,         NUMERATOR_STRING          )
      API_PROPERTY( denominatorString,       DENOMINATOR_STRING        )
      API_PROPERTY( fbprefix,                FBPREFIX                  )
      API_PROPERTY( fbdigit,                 FBDIGIT                   )
      API_PROPERTY( fbsuffix,                FBSUFFIX                  )
      API_PROPERTY( fbcontinuationline,      FBCONTINUATIONLINE        )
      API_PROPERTY( ottavaType,              OTTAVA_TYPE               )
      API_PROPERTY( numbersOnly,             NUMBERS_ONLY              )
      API_PROPERTY( trillType,               TRILL_TYPE                )
      API_PROPERTY( vibratoType,             VIBRATO_TYPE              )
      API_PROPERTY( hairpinCircledTip,       HAIRPIN_CIRCLEDTIP        )
      API_PROPERTY( hairpinType,             HAIRPIN_TYPE              )
      API_PROPERTY( hairpinHeight,           HAIRPIN_HEIGHT            )
      API_PROPERTY( hairpinContHeight,       HAIRPIN_CONT_HEIGHT       )
      API_PROPERTY( veloChange,              VELO_CHANGE               )
      API_PROPERTY( dynamicRange,            DYNAMIC_RANGE             )
      API_PROPERTY( placement,               PLACEMENT                 )
      API_PROPERTY( velocity,                VELOCITY                  )
      API_PROPERTY( jumpTo,                  JUMP_TO                   )
      API_PROPERTY( playUntil,               PLAY_UNTIL                )
      API_PROPERTY( continueAt,              CONTINUE_AT               )
      API_PROPERTY( label,                   LABEL                     )
      API_PROPERTY( markerType,              MARKER_TYPE               )
      API_PROPERTY( arpUserLen1,             ARP_USER_LEN1             )
      API_PROPERTY( arpUserLen2,             ARP_USER_LEN2             )
      API_PROPERTY( measureNumberMode,       MEASURE_NUMBER_MODE       )
      API_PROPERTY( glissType,               GLISS_TYPE                )
      API_PROPERTY( glissText,               GLISS_TEXT                )
      API_PROPERTY( glissShowText,           GLISS_SHOW_TEXT           )
      API_PROPERTY( diagonal,                DIAGONAL                  )
      API_PROPERTY( groups,                  GROUPS                    )
      API_PROPERTY( lineStyle,               LINE_STYLE                )
      API_PROPERTY( lineColor,               COLOR                     )
      API_PROPERTY( lineWidth,               LINE_WIDTH                )
      API_PROPERTY( lassoPos,                LASSO_POS                 )
      API_PROPERTY( lassoSize,               LASSO_SIZE                )
      API_PROPERTY( timeStretch,             TIME_STRETCH              )
      API_PROPERTY( ornamentStyle,           ORNAMENT_STYLE            )
      API_PROPERTY( timesig,                 TIMESIG                   )
      API_PROPERTY( timesigGlobal,           TIMESIG_GLOBAL            )
      API_PROPERTY( timesigStretch,          TIMESIG_STRETCH           )
      API_PROPERTY( timesigType,             TIMESIG_TYPE              )
      API_PROPERTY( spannerTick,             SPANNER_TICK              )
      API_PROPERTY( spannerTicks,            SPANNER_TICKS             )
      API_PROPERTY( spannerTrack2,           SPANNER_TRACK2            )
      API_PROPERTY( userOff2,                OFFSET2                   )
      API_PROPERTY( breakMmr,                BREAK_MMR                 )
      API_PROPERTY( repeatCount,             REPEAT_COUNT              )
      API_PROPERTY( userStretch,             USER_STRETCH              )
      API_PROPERTY( noOffset,                NO_OFFSET                 )
      API_PROPERTY( irregular,               IRREGULAR                 )
      API_PROPERTY( anchor,                  ANCHOR                    )
      API_PROPERTY( slurUoff1,               SLUR_UOFF1                )
      API_PROPERTY( slurUoff2,               SLUR_UOFF2                )
      API_PROPERTY( slurUoff3,               SLUR_UOFF3                )
      API_PROPERTY( slurUoff4,               SLUR_UOFF4                )
      API_PROPERTY( staffMove,               STAFF_MOVE                )
      API_PROPERTY( verse,                   VERSE                     )
      API_PROPERTY( syllabic,                SYLLABIC                  )
      API_PROPERTY( lyricTicks,              LYRIC_TICKS               )
      API_PROPERTY( volta_ending,            VOLTA_ENDING              )
      API_PROPERTY( lineVisible,             LINE_VISIBLE              )
      API_PROPERTY( mag,                     MAG                       )
      API_PROPERTY( useDrumset,              USE_DRUMSET               )
      API_PROPERTY( duration,                DURATION                  )
      API_PROPERTY( durationType,            DURATION_TYPE             )
      API_PROPERTY( role,                    ROLE                      )
      API_PROPERTY( track,                   TRACK                     )
      API_PROPERTY( glissandoStyle,          GLISSANDO_STYLE           )
      API_PROPERTY( fretStrings,             FRET_STRINGS              )
      API_PROPERTY( fretFrets,               FRET_FRETS                )
      API_PROPERTY( fretBarre,               FRET_BARRE                )
      API_PROPERTY( fretOffset,              FRET_OFFSET               )
      API_PROPERTY( fretNumPos,              FRET_NUM_POS              )
      API_PROPERTY( systemBracket,           SYSTEM_BRACKET            )
      API_PROPERTY( gap,                     GAP                       )
      API_PROPERTY( autoplace,               AUTOPLACE                 )
      API_PROPERTY( dashLineLen,             DASH_LINE_LEN             )
      API_PROPERTY( dashGapLen,              DASH_GAP_LEN              )
      API_PROPERTY_READ_ONLY( tick,          TICK                      )
      API_PROPERTY( playbackVoice1,          PLAYBACK_VOICE1           )
      API_PROPERTY( playbackVoice2,          PLAYBACK_VOICE2           )
      API_PROPERTY( playbackVoice3,          PLAYBACK_VOICE3           )
      API_PROPERTY( playbackVoice4,          PLAYBACK_VOICE4           )
      API_PROPERTY( symbol,                  SYMBOL                    )
      API_PROPERTY( playRepeats,             PLAY_REPEATS              )
      API_PROPERTY( createSystemHeader,      CREATE_SYSTEM_HEADER      )
      API_PROPERTY( staffLines,              STAFF_LINES               )
      API_PROPERTY( lineDistance,            LINE_DISTANCE             )
      API_PROPERTY( stepOffset,              STEP_OFFSET               )
      API_PROPERTY( staffShowBarlines,       STAFF_SHOW_BARLINES       )
      API_PROPERTY( staffShowLedgerlines,    STAFF_SHOW_LEDGERLINES    )
      API_PROPERTY( staffSlashStyle,         STAFF_SLASH_STYLE         )
      API_PROPERTY( staffNoteheadScheme,     STAFF_NOTEHEAD_SCHEME     )
      API_PROPERTY( staffGenClef,            STAFF_GEN_CLEF            )
      API_PROPERTY( staffGenTimesig,         STAFF_GEN_TIMESIG         )
      API_PROPERTY( staffGenKeysig,          STAFF_GEN_KEYSIG          )
      API_PROPERTY( staffYoffset,            STAFF_YOFFSET             )
      API_PROPERTY( staffUserdist,           STAFF_USERDIST            )
      API_PROPERTY( staffBarlineSpan,        STAFF_BARLINE_SPAN        )
      API_PROPERTY( staffBarlineSpanFrom,    STAFF_BARLINE_SPAN_FROM   )
      API_PROPERTY( staffBarlineSpanTo,      STAFF_BARLINE_SPAN_TO     )
      API_PROPERTY( bracketSpan,             BRACKET_SPAN              )
      API_PROPERTY( bracketColumn,           BRACKET_COLUMN            )
      API_PROPERTY( inameLayoutPosition,     INAME_LAYOUT_POSITION     )
      API_PROPERTY( subStyle,                SUB_STYLE                 )
      API_PROPERTY( fontFace,                FONT_FACE                 )
      API_PROPERTY( fontSize,                FONT_SIZE                 )
      API_PROPERTY( fontStyle,               FONT_STYLE                )
      API_PROPERTY( frameType,               FRAME_TYPE                )
      API_PROPERTY( frameWidth,              FRAME_WIDTH               )
      API_PROPERTY( framePadding,            FRAME_PADDING             )
      API_PROPERTY( frameRound,              FRAME_ROUND               )
      API_PROPERTY( frameFgColor,            FRAME_FG_COLOR            )
      API_PROPERTY( frameBgColor,            FRAME_BG_COLOR            )
      API_PROPERTY( sizeSpatiumDependent,    SIZE_SPATIUM_DEPENDENT    )
      API_PROPERTY( align,                   ALIGN                     )
      API_PROPERTY( systemFlag,              SYSTEM_FLAG               )
      API_PROPERTY( beginText,               BEGIN_TEXT                )
      API_PROPERTY( beginTextAlign,          BEGIN_TEXT_ALIGN          )
      API_PROPERTY( beginTextPlace,          BEGIN_TEXT_PLACE          )
      API_PROPERTY( beginHookType,           BEGIN_HOOK_TYPE           )
      API_PROPERTY( beginHookHeight,         BEGIN_HOOK_HEIGHT         )
      API_PROPERTY( beginFontFace,           BEGIN_FONT_FACE           )
      API_PROPERTY( beginFontSize,           BEGIN_FONT_SIZE           )
      API_PROPERTY( beginFontStyle,          BEGIN_FONT_STYLE          )
      API_PROPERTY( beginTextOffset,         BEGIN_TEXT_OFFSET         )
      API_PROPERTY( continueText,            CONTINUE_TEXT             )
      API_PROPERTY( continueTextAlign,       CONTINUE_TEXT_ALIGN       )
      API_PROPERTY( continueTextPlace,       CONTINUE_TEXT_PLACE       )
      API_PROPERTY( continueFontFace,        CONTINUE_FONT_FACE        )
      API_PROPERTY( continueFontSize,        CONTINUE_FONT_SIZE        )
      API_PROPERTY( continueFontStyle,       CONTINUE_FONT_STYLE       )
      API_PROPERTY( continueTextOffset,      CONTINUE_TEXT_OFFSET      )
      API_PROPERTY( endText,                 END_TEXT                  )
      API_PROPERTY( endTextAlign,            END_TEXT_ALIGN            )
      API_PROPERTY( endTextPlace,            END_TEXT_PLACE            )
      API_PROPERTY( endHookType,             END_HOOK_TYPE             )
      API_PROPERTY( endHookHeight,           END_HOOK_HEIGHT           )
      API_PROPERTY( endFontFace,             END_FONT_FACE             )
      API_PROPERTY( endFontSize,             END_FONT_SIZE             )
      API_PROPERTY( endFontStyle,            END_FONT_STYLE            )
      API_PROPERTY( endTextOffset,           END_TEXT_OFFSET           )
      API_PROPERTY( posAbove,                POS_ABOVE                 )
      API_PROPERTY( voice,                   VOICE                     )
      API_PROPERTY_READ_ONLY( position,      POSITION                  ) // TODO: needed?

   public:
      Element(Ms::Element* e = nullptr, Ownership own = Ownership::PLUGIN)
         : Ms::PluginAPI::ScoreElement(e, own) {}

      Ms::Element* element() { return toElement(e); }
      const Ms::Element* element() const { return toElement(e); }

      //@ create a copy of the element
      Q_INVOKABLE Ms::PluginAPI::Element* clone() const { return wrap(element()->clone(), Ownership::PLUGIN); }

      Q_INVOKABLE QString subtypeName() const { return element()->subtypeName(); }
      //@ Deprecated: same as ScoreElement::name property. Left for compatibility purposes.
      Q_INVOKABLE QString _name() const { return name(); }

      qreal offsetX() const { return element()->offset().x() / element()->spatium(); }
      qreal offsetY() const { return element()->offset().y() / element()->spatium(); }
      void setOffsetX(qreal offX);
      void setOffsetY(qreal offY);
      };

//---------------------------------------------------------
//   Note
//    Note wrapper
//---------------------------------------------------------

class Note : public Element {
      Q_OBJECT
//       Q_PROPERTY(Ms::Accidental*                accidental        READ accidental)
//       Q_PROPERTY(int                            accidentalType    READ qmlAccidentalType  WRITE qmlSetAccidentalType)
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Element>  dots              READ dots)
//       Q_PROPERTY(int                            dotsCount         READ qmlDotsCount)
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Element>  elements          READ elements)
//       Q_PROPERTY(int                            fret              READ fret               WRITE undoSetFret)
//       Q_PROPERTY(bool                           ghost             READ ghost              WRITE undoSetGhost)
//       Q_PROPERTY(Ms::NoteHead::Group            headGroup         READ headGroup          WRITE undoSetHeadGroup)
//       Q_PROPERTY(Ms::NoteHead::Type             headType          READ headType           WRITE undoSetHeadType)
//       Q_PROPERTY(bool                           hidden            READ hidden)
//       Q_PROPERTY(int                            line              READ line)
//       Q_PROPERTY(bool                           mirror            READ mirror)
//       Q_PROPERTY(int                            pitch             READ pitch              WRITE undoSetPitch)
//       Q_PROPERTY(bool                           play              READ play               WRITE undoSetPlay)
//       Q_PROPERTY(int                            ppitch            READ ppitch)
//       Q_PROPERTY(bool                           small             READ small              WRITE undoSetSmall)
//       Q_PROPERTY(int                            string            READ string             WRITE undoSetString)
//       Q_PROPERTY(int                            subchannel        READ subchannel)
//       Q_PROPERTY(Ms::Tie*                       tieBack           READ tieBack)
//       Q_PROPERTY(Ms::Tie*                       tieFor            READ tieFor)
      Q_PROPERTY(int                            tpc               READ tpc                WRITE setTpc)
//       Q_PROPERTY(int                            tpc1              READ tpc1               WRITE undoSetTpc1)
//       Q_PROPERTY(int                            tpc2              READ tpc2               WRITE undoSetTpc2)
//       Q_PROPERTY(qreal                          tuning            READ tuning             WRITE undoSetTuning)
//       Q_PROPERTY(Ms::MScore::Direction          userDotPosition   READ userDotPosition    WRITE undoSetUserDotPosition)
//       Q_PROPERTY(Ms::MScore::DirectionH         userMirror        READ userMirror         WRITE undoSetUserMirror)
//       Q_PROPERTY(int                            veloOffset        READ veloOffset         WRITE undoSetVeloOffset)
//       Q_PROPERTY(Ms::Note::ValueType            veloType          READ veloType           WRITE undoSetVeloType)

   public:
      Note(Ms::Note* c = nullptr, Ownership own = Ownership::PLUGIN)
         : Element(c, own) {}

      Ms::Note* note() { return toNote(e); }
      const Ms::Note* note() const { return toNote(e); }

      int tpc() const { return note()->tpc(); }
      void setTpc(int val);

      QQmlListProperty<Element> dots()     { return wrapContainerProperty<Element>(this, note()->dots()); }
      QQmlListProperty<Element> elements() { return wrapContainerProperty<Element>(this, note()->el());   }
      };

//---------------------------------------------------------
//   Chord
//    Chord wrapper
//---------------------------------------------------------

class Chord : public Element {
      Q_OBJECT
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Chord>    graceNotes READ graceNotes)
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Note>     notes      READ notes     )
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Element> lyrics     READ lyrics    ) // TODO: move to ChordRest

   public:
      Chord(Ms::Chord* c = nullptr, Ownership own = Ownership::PLUGIN)
         : Element(c, own) {}

      Ms::Chord* chord() { return toChord(e); }
      const Ms::Chord* chord() const { return toChord(e); }

      QQmlListProperty<Chord> graceNotes()     { return wrapContainerProperty<Chord>(this, chord()->graceNotes()); }
      QQmlListProperty<Note> notes()           { return wrapContainerProperty<Note>(this, chord()->notes());       }
      QQmlListProperty<Element> lyrics()      { return wrapContainerProperty<Element>(this, chord()->lyrics());  } // TODO: move to ChordRest // TODO: special type for Lyrics?
      };

//---------------------------------------------------------
//   Segment
//    Segment
//---------------------------------------------------------

class Segment : public Element {
      Q_OBJECT
      // TODO
//       Q_PROPERTY(QQmlListProperty<Ms::Element> annotations READ qmlAnnotations)
      Q_PROPERTY(Ms::PluginAPI::Segment*       next              READ nextInScore)
      Q_PROPERTY(Ms::PluginAPI::Segment*       nextInMeasure     READ nextInMeasure)
      Q_PROPERTY(Ms::PluginAPI::Segment*       prev              READ prevInScore)
      Q_PROPERTY(Ms::PluginAPI::Segment*       prevInMeasure     READ prevInMeasure)
//       Q_PROPERTY(Ms::Segment::Type  segmentType       READ segmentType WRITE setSegmentType)
      Q_PROPERTY(int                tick              READ tick) // TODO: revise libmscore (or this API):
                                                                 // Pid::TICK is relative or absolute in different contexts
//       Q_ENUMS(Type)

   public:
      Segment(Ms::Segment* s = nullptr, Ownership own = Ownership::SCORE)
         : Element(s, own) {}

      Ms::Segment* segment() { return toSegment(e); }
      const Ms::Segment* segment() const { return toSegment(e); }

      //@ returns the element at track 'track' (null if none)
      Q_INVOKABLE Ms::PluginAPI::Element* elementAt(int track);

      int tick() const { return segment()->tick(); }

      Segment* nextInScore() { return wrap<Segment>(segment()->next1()); }
      Segment* nextInMeasure() { return wrap<Segment>(segment()->next()); }
      Segment* prevInScore() { return wrap<Segment>(segment()->prev1()); }
      Segment* prevInMeasure() { return wrap<Segment>(segment()->prev()); }
      };

//---------------------------------------------------------
//   Measure
//    Measure wrapper
//---------------------------------------------------------

class Measure : public Element {
      Q_OBJECT
      Q_PROPERTY(Ms::PluginAPI::Segment* firstSegment READ firstSegment)
      Q_PROPERTY(Ms::PluginAPI::Segment* lastSegment  READ lastSegment)

      // TODO: to MeasureBase?
//       Q_PROPERTY(bool         lineBreak         READ lineBreak   WRITE undoSetLineBreak)
      Q_PROPERTY(Ms::PluginAPI::Measure* nextMeasure       READ nextMeasure)
//       Q_PROPERTY(Ms::Measure* nextMeasureMM     READ nextMeasureMM)
//       Q_PROPERTY(bool         pageBreak         READ pageBreak   WRITE undoSetPageBreak)
      Q_PROPERTY(Ms::PluginAPI::Measure* prevMeasure       READ prevMeasure)
//       Q_PROPERTY(Ms::Measure* prevMeasureMM     READ prevMeasureMM)

   public:
      Measure(Ms::Measure* m = nullptr, Ownership own = Ownership::SCORE)
         : Element(m, own) {}

      Ms::Measure* measure() { return toMeasure(e); }
      const Ms::Measure* measure() const { return toMeasure(e); }

      Segment* firstSegment() { return wrap<Segment>(measure()->firstEnabled(), Ownership::SCORE); }
      Segment* lastSegment() { return wrap<Segment>(measure()->last(), Ownership::SCORE); }

      Measure* prevMeasure() { return wrap<Measure>(measure()->prevMeasure(), Ownership::SCORE); }
      Measure* nextMeasure() { return wrap<Measure>(measure()->nextMeasure(), Ownership::SCORE); }
      };

#undef API_PROPERTY
#undef API_PROPERTY_READ_ONLY

}     // namespace PluginAPI
}     // namespace Ms
#endif
