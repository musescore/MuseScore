//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

namespace Ms {

class XmlReader;
enum class Sid : int;

//------------------------------------------------------------------------
//    M_PROPERTY (type, getter_name, setter_name)
//       helper macro to define a styled ScoreElement property
//
//    usage example:
//    class Text : public Element {
//          M_PROPERTY(QColor, color, setColor)
//          ...
//          };
//    this defines:
//          QColor _color;
//          const QColor& color() const { return _color; }
//          void setColor(const QColor& val) { _color = val; }
//---------------------------------------------------------

#define M_PROPERTY(a,b,c)                                      \
      a _ ## b;                                                \
   public:                                                     \
      const a& b() const   { return _ ## b; }                  \
      void c(const a& val) { _ ## b = val;  }                  \
   private:

#define M_PROPERTY2(a,b,c,d)                                   \
      a _ ## b { d };                                          \
   public:                                                     \
      const a& b() const   { return _ ## b; }                  \
      void c(const a& val) { _ ## b = val;  }                  \
   private:

//---------------------------------------------------------
//   PropertyFlags
//---------------------------------------------------------

enum class PropertyFlags : char {
      NOSTYLE, UNSTYLED, STYLED
      };

//------------------------------------------------------------------------
//   Element Properties
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
      LINE_TYPE,
      PITCH,

      TPC1,
      TPC2,
      LINE,
      FIXED,
      FIXED_LINE,
      HEAD_TYPE,
      HEAD_GROUP,
      VELO_TYPE,
      VELO_OFFSET,
      ARTICULATION_ANCHOR,

      DIRECTION,
      STEM_DIRECTION,
      NO_STEM,
      SLUR_DIRECTION,
      LEADING_SPACE,
      DISTRIBUTE,
      MIRROR_HEAD,
      DOT_POSITION,
      TUNING,
      PAUSE,

      BARLINE_TYPE,
      BARLINE_SPAN,
      BARLINE_SPAN_FROM,
      BARLINE_SPAN_TO,
      OFFSET,
      FRET,
      STRING,
      GHOST,
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
      TOP_GAP,
      BOTTOM_GAP,
      LEFT_MARGIN,
      RIGHT_MARGIN,
      TOP_MARGIN,
      BOTTOM_MARGIN,
      LAYOUT_BREAK,
      AUTOSCALE,
      SIZE,

      SCALE,
      LOCK_ASPECT_RATIO,
      SIZE_IS_SPATIUM,
      TEXT,
      HTML_TEXT,
      USER_MODIFIED,
      BEAM_POS,
      BEAM_MODE,
      BEAM_NO_SLOPE,
      USER_LEN,       // used for stems

      SPACE,          // used for spacer
      TEMPO,
      TEMPO_FOLLOW_TEXT,
      ACCIDENTAL_BRACKET,
      ACCIDENTAL_TYPE,
      NUMERATOR_STRING,
      DENOMINATOR_STRING,
      FBPREFIX,             // used for FiguredBassItem
      FBDIGIT,              //    "           "
      FBSUFFIX,             //    "           "
      FBCONTINUATIONLINE,   //    "           "

      FBPARENTHESIS1,       //    "           "
      FBPARENTHESIS2,       //    "           "
      FBPARENTHESIS3,       //    "           "
      FBPARENTHESIS4,       //    "           "
      FBPARENTHESIS5,       //    "           "
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
      DYNAMIC_RANGE,
      SINGLE_NOTE_DYNAMICS,
//100
      PLACEMENT,
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
      DIAGONAL,
      GROUPS,
      LINE_STYLE,
      LINE_WIDTH,
      LASSO_POS,
      LASSO_SIZE,
      TIME_STRETCH,
      ORNAMENT_STYLE,

      TIMESIG,
      TIMESIG_GLOBAL,
      TIMESIG_STRETCH,
      TIMESIG_TYPE,
      SPANNER_TICK,
      SPANNER_TICKS,
      SPANNER_TRACK2,
      OFFSET2,
      BREAK_MMR,
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
      DURATION_TYPE,
      ROLE,
      TRACK,

      GLISSANDO_STYLE,
      FRET_STRINGS,
      FRET_FRETS,
      FRET_NUT,
      FRET_OFFSET,
      FRET_NUM_POS,

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

      STAFF_NOTEHEAD_SCHEME,
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
      SUB_STYLE,

      FONT_FACE,
      FONT_SIZE,
      FONT_STYLE,

      FRAME_TYPE,
      FRAME_WIDTH,
      FRAME_PADDING,
      FRAME_ROUND,
      FRAME_FG_COLOR,
//200
      FRAME_BG_COLOR,
      SIZE_SPATIUM_DEPENDENT,
      ALIGN,
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
      KEY,
      ACTION, // for Icon
      MIN_DISTANCE,

      ARPEGGIO_TYPE,
      CHORD_LINE_TYPE,
      CHORD_LINE_STRAIGHT,
      TREMOLO_TYPE,
      TREMOLO_PLACEMENT,
      HARMONY_TYPE,

      START_WITH_LONG_NAMES,
      START_WITH_MEASURE_ONE,

      END
      };

enum class P_TYPE : char {
      BOOL,
      INT,
      REAL,
      SPATIUM,
      SP_REAL,          // real (point) value saved in (score) spatium units
      FRACTION,
      POINT,
      POINT_SP,         // point units, value saved in (score) spatium units
      POINT_MM,
      POINT_SP_MM,      // point units, value saved as mm or spatium depending on Element->sizeIsSpatiumDependent()
      SIZE,
      SIZE_MM,
      STRING,
      SCALE,
      COLOR,
      DIRECTION,      // enum class Direction
      DIRECTION_H,    // enum class MScore::DirectionH
      ORNAMENT_STYLE, // enum class MScore::OrnamentStyle
      TDURATION,
      LAYOUT_BREAK,
      VALUE_TYPE,
      BEAM_MODE,
      PLACEMENT,
      TEXT_PLACE,
      TEMPO,
      GROUPS,
      SYMID,
      INT_LIST,
      GLISSANDO_STYLE,
      BARLINE_TYPE,
      HEAD_TYPE,        // enum class Notehead::Type
      HEAD_GROUP,       // enum class Notehead::Group
      ZERO_INT,         // displayed with offset +1
      FONT,
      SUB_STYLE,
      ALIGN,
      CHANGE_METHOD,    // enum class VeloChangeMethod (for single notedynamics)
      CHANGE_SPEED,     // enum class Dynamic::Speed
      CLEF_TYPE,        // enum class ClefType
      DYNAMIC_TYPE,     // enum class Dynamic::Type
      KEYMODE,          // enum class KeyMode
      };

extern QVariant readProperty(Pid type, XmlReader& e);
extern QVariant propertyFromString(Pid type, QString value);
extern QString propertyToString(Pid, QVariant value, bool mscx);
extern P_TYPE propertyType(Pid);
extern const char* propertyName(Pid);
extern bool propertyLink(Pid id);
extern Pid propertyId(const QString& name);
extern Pid propertyId(const QStringRef& name);
extern QString propertyUserName(Pid);

}     // namespace Ms
#endif

