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

//------------------------------------------------------------------------
//   Element Properties
//    accessible through
//    virtual QVariant Element::getProperty(int propertyId)
//    virtual void Element::setProperty(int propertyId, const QVariant&)
//------------------------------------------------------------------------

enum P_ID {
      P_SUBTYPE,
      P_SELECTED,
      P_COLOR,
      P_VISIBLE,
      P_SMALL,
      P_SHOW_COURTESY,
      P_LINE_TYPE,
      P_PITCH,
      P_TPC,
      P_HEAD_TYPE,

      P_HEAD_GROUP,
      P_VELO_TYPE,
      P_VELO_OFFSET,
      P_ARTICULATION_ANCHOR,
      P_DIRECTION,
      P_STEM_DIRECTION,
      P_NO_STEM,
      P_SLUR_DIRECTION,
      P_LEADING_SPACE,
      P_TRAILING_SPACE,

/*20*/P_DISTRIBUTE,
      P_MIRROR_HEAD,
      P_DOT_POSITION,
      P_TUNING,
      P_PAUSE,
      P_BARLINE_SPAN,
      P_BARLINE_SPAN_FROM,
      P_BARLINE_SPAN_TO,
      P_USER_OFF,
      P_FRET,

      P_STRING,
      P_GHOST,
      P_PLAY,
      P_TIMESIG_NOMINAL,
      P_TIMESIG_ACTUAL,
      P_NUMBER_TYPE,
      P_BRACKET_TYPE,
      P_NORMAL_NOTES,
      P_ACTUAL_NOTES,
      P_P1,

/*40*/P_P2,
      P_GROW_LEFT,
      P_GROW_RIGHT,
      P_BOX_HEIGHT,
      P_BOX_WIDTH,
      P_TOP_GAP,
      P_BOTTOM_GAP,
      P_LEFT_MARGIN,
      P_RIGHT_MARGIN,
      P_TOP_MARGIN,

      P_BOTTOM_MARGIN,
      P_LAYOUT_BREAK,
      P_AUTOSCALE,
      P_SIZE,
      P_SCALE,
      P_LOCK_ASPECT_RATIO,
      P_SIZE_IS_SPATIUM,
      P_TEXT_STYLE,
      P_TEXT,
      P_HTML_TEXT,

      P_USER_MODIFIED,
      P_BEAM_POS,
      P_BEAM_MODE,
      P_BEAM_NO_SLOPE,
      P_USER_LEN,       // used for stems
      P_SPACE,          // used for spacer
      P_TEMPO,
      P_TEMPO_FOLLOW_TEXT,
      P_ACCIDENTAL_BRACKET,
      P_NUMERATOR_STRING,

      P_DENOMINATOR_STRING,
      P_SHOW_NATURALS,
      P_BREAK_HINT,
      P_FBPREFIX,             // used for FiguredBassItem
      P_FBDIGIT,              //    "           "
      P_FBSUFFIX,             //    "           "
      P_FBCONTINUATIONLINE,   //    "           "
      P_FBPARENTHESIS1,       //    "           "
      P_FBPARENTHESIS2,       //    "           "
      P_FBPARENTHESIS3,       //    "           "

      P_FBPARENTHESIS4,       //    "           "
      P_FBPARENTHESIS5,       //    "           "
      P_VOLTA_TYPE,
      P_OTTAVA_TYPE,
      P_NUMBERS_ONLY,
      P_TRILL_TYPE,
      P_HAIRPIN_TYPE,
      P_HAIRPIN_HEIGHT,
      P_HAIRPIN_CONT_HEIGHT,
      P_VELO_CHANGE,

      P_DYNAMIC_RANGE,
      P_PLACEMENT,
      P_VELOCITY,
      P_JUMP_TO,
      P_PLAY_UNTIL,
      P_CONTINUE_AT,
      P_LABEL,
      P_MARKER_TYPE,
      P_ARP_USER_LEN1,
      P_ARP_USER_LEN2,

      P_REPEAT_FLAGS,
      P_END_BARLINE_TYPE,
      P_END_BARLINE_VISIBLE,
      P_END_BARLINE_COLOR,
      P_MEASURE_NUMBER_MODE,
      P_GLISS_TYPE,
      P_GLISS_TEXT,
      P_GLISS_SHOW_TEXT,
      P_DIAGONAL,
      P_GROUPS,

      P_LINE_STYLE,
      P_LINE_COLOR,
      P_LINE_WIDTH,
      P_LASSO_POS,
      P_LASSO_SIZE,
      P_TIME_STRETCH,
      P_TIMESIG,
      P_TIMESIG_GLOBAL,
      P_SPANNER_TICK,
      P_SPANNER_TICK2,
      P_SPANNER_TRACK2,

      P_USER_OFF2,
      P_BEGIN_TEXT_PLACE,
      P_CONTINUE_TEXT_PLACE,
      P_BEGIN_HOOK,
      P_END_HOOK,
      P_BEGIN_HOOK_HEIGHT,
      P_END_HOOK_HEIGHT,
      P_BEGIN_HOOK_TYPE,
      P_END_HOOK_TYPE,
      P_BEGIN_SYMBOL,

      P_CONTINUE_SYMBOL,
      P_END_SYMBOL,
      P_BEGIN_SYMBOL_OFFSET,
      P_CONTINUE_SYMBOL_OFFSET,
      P_END_SYMBOL_OFFSET,

      P_END
      };

enum P_TYPE {
      T_SUBTYPE,
      T_BOOL,
      T_INT,
      T_REAL,
      T_SPATIUM,
      T_SP_REAL,
      T_FRACTION,
      T_POINT,
      T_POINT_MM,
      T_SIZE,
      T_SIZE_MM,
      T_STRING,
      T_SCALE,
      T_COLOR,
      T_DIRECTION,      // enum Direction
      T_DIRECTION_H,    // enum DirectionH
      T_LAYOUT_BREAK,
      T_VALUE_TYPE,
      T_BEAM_MODE,
      T_PLACEMENT,
      T_TEMPO,
      T_GROUPS,
      T_SYMID,
      };

extern QVariant getProperty(P_ID type, XmlReader& e);
extern P_TYPE propertyType(P_ID);
extern const char* propertyName(P_ID);
extern bool propertyLink(P_ID id);


}     // namespace Ms
#endif

