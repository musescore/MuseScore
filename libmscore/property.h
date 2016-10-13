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
enum class StyleIdx : int;

//---------------------------------------------------------
//   PropertyStyle
//---------------------------------------------------------

enum class PropertyStyle : char {
      NOSTYLE, UNSTYLED, STYLED
      };

//------------------------------------------------------------------------
//   Element Properties
//------------------------------------------------------------------------

enum class P_ID : int {
      SUBTYPE,
      SELECTED,
      GENERATED,
      COLOR,
      VISIBLE,
      Z,
      SMALL,
      SHOW_COURTESY,
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
      USER_OFF,
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
      TEXT_STYLE,
      TEXT_STYLE_TYPE,
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
      VOLTA_TYPE,
      OTTAVA_TYPE,
      NUMBERS_ONLY,
      TRILL_TYPE,

      HAIRPIN_CIRCLEDTIP,
      HAIRPIN_TYPE,
      HAIRPIN_HEIGHT,
      HAIRPIN_CONT_HEIGHT,
      VELO_CHANGE,
      DYNAMIC_RANGE,
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
      LINE_COLOR,
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
      USER_OFF2,
      BEGIN_TEXT_PLACE,
      CONTINUE_TEXT_PLACE,

      END_TEXT_PLACE,
      BEGIN_HOOK,
      END_HOOK,
      BEGIN_HOOK_HEIGHT,
      END_HOOK_HEIGHT,
      BEGIN_HOOK_TYPE,
      END_HOOK_TYPE,
      BEGIN_TEXT,
      CONTINUE_TEXT,
      END_TEXT,

      BEGIN_TEXT_STYLE,
      CONTINUE_TEXT_STYLE,
      END_TEXT_STYLE,
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
      PART_VOLUME,
      PART_MUTE,
      PART_PAN,
      PART_REVERB,
      PART_CHORUS,

      DURATION,
      DURATION_TYPE,
      ROLE,
      TRACK,

      GLISSANDO_STYLE,

      FRET_STRINGS,
      FRET_FRETS,
      FRET_BARRE,
      FRET_OFFSET,

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

      END
      };

enum class P_TYPE : char {
      SUBTYPE,
      BOOL,
      INT,
      REAL,
      SPATIUM,
      SP_REAL,
      FRACTION,
      POINT,
      POINT_MM,
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
      TEMPO,
      GROUPS,
      SYMID,
      TEXT_STYLE,
      INT_LIST,
      GLISSANDO_STYLE,
      BARLINE_TYPE,
      ZERO_INT,         // displayed with offset +1
      };

extern QVariant getProperty(P_ID type, XmlReader& e);
extern P_TYPE propertyType(P_ID);
extern const char* propertyName(P_ID);
extern bool propertyLink(P_ID id);

}     // namespace Ms
#endif

