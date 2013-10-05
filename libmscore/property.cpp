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

#include "property.h"
#include "mscore.h"
#include "layoutbreak.h"
#include "groups.h"

namespace Ms {

//---------------------------------------------------------
//   PropertyData
//---------------------------------------------------------

struct PropertyData {
      P_ID id;
      bool link;              // change for linked elements
      const char* name;       // xml name of property
      P_TYPE type;
      };

//
// always: property[subtype].id == subtype
//
//
static const PropertyData propertyList[] = {
      { P_SUBTYPE,             false, "subtype",       T_INT   },
      { P_SELECTED,            false, "selected",      T_BOOL  },
      { P_COLOR,               false, "color",         T_COLOR },
      { P_VISIBLE,             false, "visible",       T_BOOL  },
      { P_SMALL,               false, "small",         T_BOOL  },
      { P_SHOW_COURTESY,       false, "",              T_INT   },
      { P_LINE_TYPE,           false, "",              T_INT   },
      { P_PITCH,               false, "pitch",         T_INT },
      { P_TPC,                 false, "tpc",           T_INT },
      { P_HEAD_TYPE,           false, "headType",      T_INT },

      { P_HEAD_GROUP,          false, "head",          T_INT },
      { P_VELO_TYPE,           false, "veloType",      T_VALUE_TYPE },
      { P_VELO_OFFSET,         false, "velocity",      T_INT },
      { P_ARTICULATION_ANCHOR, false, "anchor",        T_INT },
      { P_DIRECTION,           false, "direction",     T_DIRECTION },
      { P_STEM_DIRECTION,      true,  "StemDirection", T_DIRECTION },
      { P_NO_STEM,             false, "",              T_INT },
      { P_SLUR_DIRECTION,      false, "",              T_INT },
      { P_LEADING_SPACE,       false, "",              T_SPATIUM },
      { P_TRAILING_SPACE,      false, "",              T_SPATIUM },

      { P_DISTRIBUTE,          false, "distribute",    T_BOOL },
      { P_MIRROR_HEAD,         false, "mirror",        T_DIRECTION_H },
      { P_DOT_POSITION,        false, "dotPosition",   T_DIRECTION },
      { P_TUNING,              false, "tuning",        T_REAL  },
      { P_PAUSE,               false, "pause",         T_REAL  },
      { P_BARLINE_SPAN,        false, "",              T_INT   },
      { P_BARLINE_SPAN_FROM,   false, 0,               T_INT   },
      { P_BARLINE_SPAN_TO,     false, 0,               T_INT   },
      { P_USER_OFF,            false, "userOff",       T_POINT },
      { P_FRET,                false, "fret",          T_INT   },

      { P_STRING,              false, "string",        T_INT   },
      { P_GHOST,               false, "ghost",         T_BOOL  },
      { P_PLAY,                false, "play",          T_BOOL  },
      { P_TIMESIG_NOMINAL,     false, 0,               T_FRACTION },
      { P_TIMESIG_ACTUAL,      false, 0,               T_FRACTION },
      { P_NUMBER_TYPE,         false, "numberType",    T_INT   },
      { P_BRACKET_TYPE,        false, "bracketType",   T_INT   },
      { P_NORMAL_NOTES,        false, "normalNotes",   T_INT   },
      { P_ACTUAL_NOTES,        false, "actualNotes",   T_INT   },
      { P_P1,                  false, "p1",            T_POINT },

      { P_P2,                  false, "p2",            T_POINT },
      { P_GROW_LEFT,           false, "growLeft",      T_REAL    },
      { P_GROW_RIGHT,          false, "growRight",     T_REAL    },
      { P_BOX_HEIGHT,          false, "height",        T_SPATIUM },
      { P_BOX_WIDTH,           false, "width",         T_SPATIUM },
      { P_TOP_GAP,             false, "topGap",        T_SP_REAL },
      { P_BOTTOM_GAP,          false, "bottomGap",     T_SP_REAL },
      { P_LEFT_MARGIN,         false, "leftMargin",    T_REAL    },
      { P_RIGHT_MARGIN,        false, "rightMargin",   T_REAL    },
      { P_TOP_MARGIN,          false, "topMargin",     T_REAL    },

      { P_BOTTOM_MARGIN,       false, "bottomMargin",  T_REAL    },
      { P_LAYOUT_BREAK,        false, "subtype",       T_LAYOUT_BREAK },
      { P_AUTOSCALE,           false, "autoScale",     T_BOOL   },
      { P_SIZE,                false, "size",            T_SIZE },
      { P_SCALE,               false, 0,                 T_SCALE  },
      { P_LOCK_ASPECT_RATIO,   false, "lockAspectRatio", T_BOOL },
      { P_SIZE_IS_SPATIUM,     false, "sizeIsSpatium",   T_BOOL },
      { P_TEXT_STYLE,          false, "textStyle",       T_INT  },
      { P_TEXT,                false, 0,               T_STRING },
      { P_HTML_TEXT,           false, 0,               T_STRING },

      { P_USER_MODIFIED,       false, 0,               T_BOOL   },
      { P_BEAM_POS,            false, 0,               T_POINT  },
      { P_BEAM_MODE,           true, "BeamMode",       T_BEAM_MODE  },
      { P_BEAM_NO_SLOPE,       true, "noSlope",        T_BOOL   },
      { P_USER_LEN,            false, "",              T_REAL   },
      { P_SPACE,               false, "space",         T_SP_REAL},
      { P_TEMPO,               false, "tempo",         T_TEMPO  },
      { P_TEMPO_FOLLOW_TEXT,   false, "followText",    T_BOOL   },
      { P_ACCIDENTAL_BRACKET,  false, "bracket",       T_BOOL   },
      { P_NUMERATOR_STRING,    false, "textN",         T_STRING },

      { P_DENOMINATOR_STRING,  false, "textD",         T_STRING },
      { P_SHOW_NATURALS,       false, "showNaturals",  T_BOOL   },
      { P_BREAK_HINT,          false, "",              T_BOOL   },
      { P_FBPREFIX,            false, "prefix",        T_INT    },
      { P_FBDIGIT,             false, "digit",         T_INT    },
      { P_FBSUFFIX,            false, "suffix",        T_INT    },
      { P_FBCONTINUATIONLINE,  false, "continuationLine", T_INT },
      { P_FBPARENTHESIS1,      false, "",              T_INT    },
      { P_FBPARENTHESIS2,      false, "",              T_INT    },
      { P_FBPARENTHESIS3,      false, "",              T_INT    },

      { P_FBPARENTHESIS4,      false, "",              T_INT    },
      { P_FBPARENTHESIS5,      false, "",              T_INT    },
      { P_VOLTA_TYPE,          false, "",              T_INT    },
      { P_OTTAVA_TYPE,         false, "",              T_INT    },
      { P_NUMBERS_ONLY,        false, "numbersOnly",   T_BOOL   },
      { P_TRILL_TYPE,          false, "",              T_INT    },
      { P_HAIRPIN_TYPE,        false, "",              T_INT     },
      { P_HAIRPIN_HEIGHT,      false, "hairpinHeight",     T_SPATIUM },
      { P_HAIRPIN_CONT_HEIGHT, false, "hairpinContHeight", T_SPATIUM },
      { P_VELO_CHANGE,         false, "",              T_INT    },

      { P_DYNAMIC_RANGE,       false, "dynType",       T_INT    },
      { P_PLACEMENT,           false, "placement",     T_PLACEMENT    },
      { P_VELOCITY,            false, "velocity",      T_INT    },
      { P_JUMP_TO,             false, "jumpTo",        T_STRING },
      { P_PLAY_UNTIL,          false, "playUntil",     T_STRING },
      { P_CONTINUE_AT,         false, "continueAt",    T_STRING },
      { P_LABEL,               false, "label",         T_STRING },
      { P_MARKER_TYPE,         false, 0,               T_INT    },
      { P_ARP_USER_LEN1,       false, 0,               T_REAL   },
      { P_ARP_USER_LEN2,       false, 0,               T_REAL   },

      { P_REPEAT_FLAGS,        false, 0,               T_INT    },
      { P_END_BARLINE_TYPE,    false, 0,               T_INT    },
      { P_END_BARLINE_VISIBLE, false, 0,               T_BOOL   },
      { P_END_BARLINE_COLOR,   false, 0,               T_COLOR  },
      { P_MEASURE_NUMBER_MODE, false, 0,               T_INT    },
      { P_GLISS_TYPE,          false, 0,               T_INT    },
      { P_GLISS_TEXT,          false, 0,               T_STRING },
      { P_GLISS_SHOW_TEXT,     false, 0,               T_BOOL   },
      { P_DIAGONAL,            false, 0,               T_BOOL     },
      { P_GROUPS,              false, 0,               T_GROUPS   },

      { P_LINE_STYLE,          false, "lineStyle",     T_INT      },
      { P_LINE_COLOR,          false, 0,               T_COLOR    },
      { P_LINE_WIDTH,          false, 0,               T_SPATIUM  },
      { P_LASSO_POS,           false, 0,               T_POINT_MM },
      { P_LASSO_SIZE,          false, 0,               T_SIZE_MM  },
      { P_TIME_STRETCH,        false, 0,               T_REAL     },
      { P_TIMESIG,             false, 0,               T_FRACTION },
      { P_TIMESIG_GLOBAL,      false, 0,               T_FRACTION },
      { P_SPANNER_TICK,        true,  "tick",          T_INT      },
      { P_SPANNER_TICK2,       true,  "tick2",         T_INT      },
      { P_SPANNER_TRACK2,      true,  "track2",        T_INT      },

      { P_USER_OFF2,              false, "userOff2",             T_POINT   },
      { P_BEGIN_TEXT_PLACE,       false, "beginTextPlace",       T_INT     },
      { P_CONTINUE_TEXT_PLACE,    false, "beginTextPlace",       T_INT     },
      { P_BEGIN_HOOK,             false, "beginHook",            T_BOOL    },
      { P_END_HOOK,               false, "endHook",              T_BOOL    },
      { P_BEGIN_HOOK_HEIGHT,      false, "beginHookHeight",      T_SPATIUM },
      { P_END_HOOK_HEIGHT,        false, "endHookHeight",        T_SPATIUM },
      { P_BEGIN_HOOK_TYPE,        false, "beginHookType",        T_INT     },
      { P_END_HOOK_TYPE,          false, "endHookType",          T_INT     },
      { P_BEGIN_SYMBOL,           false, "beginSymbol",          T_SYMID   },

      { P_CONTINUE_SYMBOL,        false, "continueSymbol",       T_SYMID   },
      { P_END_SYMBOL,             false, "endSymbol",            T_SYMID   },
      { P_BEGIN_SYMBOL_OFFSET,    false, "beginSymbolOffset",    T_POINT   },
      { P_CONTINUE_SYMBOL_OFFSET, false, "continueSymbolOffset", T_POINT   },
      { P_END_SYMBOL_OFFSET,      false, "endSymbolOffset",      T_POINT   },

      { P_END,                 false, "",              T_INT      }
      };

//---------------------------------------------------------
//   propertyType
//---------------------------------------------------------

P_TYPE propertyType(P_ID id)
      {
      return propertyList[id].type;
      }

//---------------------------------------------------------
//   propertyLink
//---------------------------------------------------------

bool propertyLink(P_ID id)
      {
      return propertyList[id].link;
      }

//---------------------------------------------------------
//   propertyName
//---------------------------------------------------------

const char* propertyName(P_ID id)
      {
      return propertyList[id].name;
      }

//---------------------------------------------------------
//    getProperty
//---------------------------------------------------------

QVariant getProperty(P_ID id, XmlReader& e)
      {
      switch(propertyType(id)) {
            case T_BOOL:
                  return QVariant(bool(e.readInt()));
            case T_SUBTYPE:
            case T_INT:
                  return QVariant(e.readInt());
            case T_REAL:
            case T_SPATIUM:
            case T_SP_REAL:
            case T_TEMPO:
                  return QVariant(e.readDouble());
            case T_FRACTION:
                  return QVariant::fromValue(e.readFraction());
            case T_COLOR:
                  return QVariant(e.readColor());
            case T_POINT:
                  return QVariant(e.readPoint());
            case T_SCALE:
            case T_SIZE:
                  return QVariant(e.readSize());
            case T_STRING:
                  return QVariant(e.readElementText());
            case T_DIRECTION:
                  {
                  QString value(e.readElementText());
                  if (value == "up")
                        return QVariant(MScore::UP);
                  else if (value == "down")
                        return QVariant(MScore::DOWN);
                  else if (value == "auto")
                        return QVariant(MScore::AUTO);
                  }
                  break;
            case T_DIRECTION_H:
                  {
                  QString value(e.readElementText());
                  if (value == "left")
                        return QVariant(MScore::DH_LEFT);
                  else if (value == "right")
                        return QVariant(MScore::DH_RIGHT);
                  else if (value == "auto")
                        return QVariant(MScore::DH_AUTO);
                  }
                  break;
            case T_LAYOUT_BREAK: {
                  QString value(e.readElementText());
                  if (value == "line")
                        return QVariant(int(LayoutBreak::LINE));
                  if (value == "page")
                        return QVariant(int(LayoutBreak::PAGE));
                  if (value == "section")
                        return QVariant(int(LayoutBreak::SECTION));
                  qDebug("getProperty: invalid T_LAYOUT_BREAK: <%s>", qPrintable(value));
                  }
                  break;
            case T_VALUE_TYPE: {
                  QString value(e.readElementText());
                  if (value == "offset")
                        return QVariant(int(MScore::OFFSET_VAL));
                  else if (value == "user")
                        return QVariant(int(MScore::USER_VAL));
                  }
                  break;
            case T_PLACEMENT: {
                  QString value(e.readElementText());
                  if (value == "above")
                        return QVariant(int(Element::ABOVE));
                  else if (value == "below")
                        return QVariant(int(Element::BELOW));
                  }
                  break;
            case T_BEAM_MODE:             // TODO
                  return QVariant(int(0));

            case T_GROUPS:
                  {
                  Groups g;
                  g.read(e);
                  return QVariant::fromValue(g);
                  }
            case T_POINT_MM:
            case T_SIZE_MM:
                  return QVariant();
            }
      return QVariant();
      }

}

