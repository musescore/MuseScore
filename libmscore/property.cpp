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
#include "xml.h"
#include "note.h"

namespace Ms {

//---------------------------------------------------------
//   PropertyData
//---------------------------------------------------------

struct PropertyData {
      P_ID id;
      bool link;              // link this property for linked elements
      const char* name;       // xml name of property
      P_TYPE type;
      };

//
// always: propertyList[subtype].id == subtype
//
//
static const PropertyData propertyList[] = {
      { P_SUBTYPE,             false, "subtype",       P_TYPE::INT   },
      { P_SELECTED,            false, "selected",      P_TYPE::BOOL  },
      { P_COLOR,               false, "color",         P_TYPE::COLOR },
      { P_VISIBLE,             false, "visible",       P_TYPE::BOOL  },
      { P_SMALL,               false, "small",         P_TYPE::BOOL  },
      { P_SHOW_COURTESY,       false, "",              P_TYPE::INT   },
      { P_LINE_TYPE,           false, "",              P_TYPE::INT   },
      { P_PITCH,               true,  "pitch",         P_TYPE::INT   },
      { P_TPC1,                false, "tpc",           P_TYPE::INT   },
      { P_TPC2,                false, "tpc2",          P_TYPE::INT   },

      { P_LINE,                false, "line",          P_TYPE::INT   },
      { P_HEAD_TYPE,           false, "headType",      P_TYPE::INT   },
      { P_HEAD_GROUP,          false, "head",          P_TYPE::INT },
      { P_VELO_TYPE,           false, "veloType",      P_TYPE::VALUE_TYPE },
      { P_VELO_OFFSET,         false, "velocity",      P_TYPE::INT },
      { P_ARTICULATION_ANCHOR, false, "anchor",        P_TYPE::INT },
      { P_DIRECTION,           false, "direction",     P_TYPE::DIRECTION },
      { P_STEM_DIRECTION,      true,  "StemDirection", P_TYPE::DIRECTION },
      { P_NO_STEM,             false, "",              P_TYPE::INT },
      { P_SLUR_DIRECTION,      false, "",              P_TYPE::INT },

      { P_LEADING_SPACE,       false, "",              P_TYPE::SPATIUM },
      { P_TRAILING_SPACE,      false, "",              P_TYPE::SPATIUM },
      { P_DISTRIBUTE,          false, "distribute",    P_TYPE::BOOL },
      { P_MIRROR_HEAD,         false, "mirror",        P_TYPE::DIRECTION_H },
      { P_DOT_POSITION,        false, "dotPosition",   P_TYPE::DIRECTION },
      { P_TUNING,              false, "tuning",        P_TYPE::REAL  },
      { P_PAUSE,               false, "pause",         P_TYPE::REAL  },
      { P_BARLINE_SPAN,        false, "",              P_TYPE::INT   },
      { P_BARLINE_SPAN_FROM,   false, 0,               P_TYPE::INT   },
      { P_BARLINE_SPAN_TO,     false, 0,               P_TYPE::INT   },

      { P_USER_OFF,            false, "userOff",       P_TYPE::POINT },
      { P_FRET,                false, "fret",          P_TYPE::INT   },
      { P_STRING,              false, "string",        P_TYPE::INT   },
      { P_GHOST,               false, "ghost",         P_TYPE::BOOL  },
      { P_PLAY,                false, "play",          P_TYPE::BOOL  },
      { P_TIMESIG_NOMINAL,     false, 0,               P_TYPE::FRACTION },
      { P_TIMESIG_ACTUAL,      true,  0,               P_TYPE::FRACTION },
      { P_NUMBER_TYPE,         false, "numberType",    P_TYPE::INT   },
      { P_BRACKET_TYPE,        false, "bracketType",   P_TYPE::INT   },
      { P_NORMAL_NOTES,        false, "normalNotes",   P_TYPE::INT   },

      { P_ACTUAL_NOTES,        false, "actualNotes",   P_TYPE::INT   },
      { P_P1,                  false, "p1",            P_TYPE::POINT },
      { P_P2,                  false, "p2",            P_TYPE::POINT },
      { P_GROW_LEFT,           false, "growLeft",      P_TYPE::REAL    },
      { P_GROW_RIGHT,          false, "growRight",     P_TYPE::REAL    },
      { P_BOX_HEIGHT,          false, "height",        P_TYPE::SPATIUM },
      { P_BOX_WIDTH,           false, "width",         P_TYPE::SPATIUM },
      { P_TOP_GAP,             false, "topGap",        P_TYPE::SP_REAL },
      { P_BOTTOM_GAP,          false, "bottomGap",     P_TYPE::SP_REAL },
      { P_LEFT_MARGIN,         false, "leftMargin",    P_TYPE::REAL    },

      { P_RIGHT_MARGIN,        false, "rightMargin",   P_TYPE::REAL    },
      { P_TOP_MARGIN,          false, "topMargin",     P_TYPE::REAL    },
      { P_BOTTOM_MARGIN,       false, "bottomMargin",  P_TYPE::REAL    },
      { P_LAYOUT_BREAK,        false, "subtype",       P_TYPE::LAYOUT_BREAK },
      { P_AUTOSCALE,           false, "autoScale",     P_TYPE::BOOL   },
      { P_SIZE,                false, "size",            P_TYPE::SIZE },
      { P_SCALE,               false, 0,                 P_TYPE::SCALE  },
      { P_LOCK_ASPECT_RATIO,   false, "lockAspectRatio", P_TYPE::BOOL },
      { P_SIZE_IS_SPATIUM,     false, "sizeIsSpatium",   P_TYPE::BOOL },
      { P_TEXT_STYLE,          false, "textStyle",       P_TYPE::TEXT_STYLE  },

      { P_TEXT_STYLE_TYPE,     false, "textStyleType", P_TYPE::INT  },
      { P_TEXT,                false, 0,               P_TYPE::STRING },
      { P_HTML_TEXT,           false, 0,               P_TYPE::STRING },
      { P_USER_MODIFIED,       false, 0,               P_TYPE::BOOL   },
      { P_BEAM_POS,            false, 0,               P_TYPE::POINT  },
      { P_BEAM_MODE,           true, "BeamMode",       P_TYPE::BEAM_MODE  },
      { P_BEAM_NO_SLOPE,       true, "noSlope",        P_TYPE::BOOL   },
      { P_USER_LEN,            false, "",              P_TYPE::REAL   },
      { P_SPACE,               false, "space",         P_TYPE::SP_REAL},
      { P_TEMPO,               false, "tempo",         P_TYPE::TEMPO  },

      { P_TEMPO_FOLLOW_TEXT,   false, "followText",    P_TYPE::BOOL   },
      { P_ACCIDENTAL_BRACKET,  false, "bracket",       P_TYPE::BOOL   },
      { P_NUMERATOR_STRING,    false, "textN",         P_TYPE::STRING },
      { P_DENOMINATOR_STRING,  false, "textD",         P_TYPE::STRING },
      { P_BREAK_HINT,          false, "",              P_TYPE::BOOL   },
      { P_FBPREFIX,            false, "prefix",        P_TYPE::INT    },
      { P_FBDIGIT,             false, "digit",         P_TYPE::INT    },
      { P_FBSUFFIX,            false, "suffix",        P_TYPE::INT    },
      { P_FBCONTINUATIONLINE,  false, "continuationLine", P_TYPE::INT },
      { P_FBPARENTHESIS1,      false, "",              P_TYPE::INT    },

      { P_FBPARENTHESIS2,      false, "",              P_TYPE::INT    },
      { P_FBPARENTHESIS3,      false, "",              P_TYPE::INT    },
      { P_FBPARENTHESIS4,      false, "",              P_TYPE::INT    },
      { P_FBPARENTHESIS5,      false, "",              P_TYPE::INT    },
      { P_VOLTA_TYPE,          false, "",              P_TYPE::INT    },
      { P_OTTAVA_TYPE,         false, "",              P_TYPE::INT    },
      { P_NUMBERS_ONLY,        false, "numbersOnly",   P_TYPE::BOOL   },
      { P_TRILL_TYPE,          false, "",              P_TYPE::INT    },
      { P_HAIRPIN_CIRCLEDTIP,  false, "hairpinCircledTip", P_TYPE::BOOL     },
      { P_HAIRPIN_TYPE,        false, "",              P_TYPE::INT     },

      { P_HAIRPIN_HEIGHT,      false, "hairpinHeight",     P_TYPE::SPATIUM },
      { P_HAIRPIN_CONT_HEIGHT, false, "hairpinContHeight", P_TYPE::SPATIUM },
      { P_VELO_CHANGE,         false, "",              P_TYPE::INT    },
      { P_DYNAMIC_RANGE,       false, "dynType",       P_TYPE::INT    },
      { P_PLACEMENT,           false, "placement",     P_TYPE::PLACEMENT    },
      { P_VELOCITY,            false, "velocity",      P_TYPE::INT    },
      { P_JUMP_TO,             false, "jumpTo",        P_TYPE::STRING },
      { P_PLAY_UNTIL,          false, "playUntil",     P_TYPE::STRING },
      { P_CONTINUE_AT,         false, "continueAt",    P_TYPE::STRING },
      { P_LABEL,               false, "label",         P_TYPE::STRING },

      { P_MARKER_TYPE,         false, 0,               P_TYPE::INT    },
      { P_ARP_USER_LEN1,       false, 0,               P_TYPE::REAL   },
      { P_ARP_USER_LEN2,       false, 0,               P_TYPE::REAL   },
      { P_REPEAT_FLAGS,        false, 0,               P_TYPE::INT    },
      { P_END_BARLINE_TYPE,    false, 0,               P_TYPE::INT    },
      { P_END_BARLINE_VISIBLE, false, 0,               P_TYPE::BOOL   },
      { P_END_BARLINE_COLOR,   false, 0,               P_TYPE::COLOR  },
      { P_MEASURE_NUMBER_MODE, false, 0,               P_TYPE::INT    },
      { P_GLISS_TYPE,          false, 0,               P_TYPE::INT    },
      { P_GLISS_TEXT,          false, 0,               P_TYPE::STRING },

      { P_GLISS_SHOW_TEXT,     false, 0,               P_TYPE::BOOL   },
      { P_DIAGONAL,            false, 0,               P_TYPE::BOOL     },
      { P_GROUPS,              false, 0,               P_TYPE::GROUPS   },
      { P_LINE_STYLE,          false, "lineStyle",     P_TYPE::INT      },
      { P_LINE_COLOR,          false, 0,               P_TYPE::COLOR    },
      { P_LINE_WIDTH,          false, 0,               P_TYPE::SPATIUM  },
      { P_LASSO_POS,           false, 0,               P_TYPE::POINT_MM },
      { P_LASSO_SIZE,          false, 0,               P_TYPE::SIZE_MM  },
      { P_TIME_STRETCH,        false, 0,               P_TYPE::REAL     },
      { P_TIMESIG,             false, 0,               P_TYPE::FRACTION },

      { P_TIMESIG_GLOBAL,      false, 0,               P_TYPE::FRACTION },
      { P_SPANNER_TICK,        true,  "tick",          P_TYPE::INT      },
      { P_SPANNER_TICK2,       true,  "tick2",         P_TYPE::INT      },
      { P_SPANNER_TRACK2,      true,  "track2",          P_TYPE::INT      },
      { P_USER_OFF2,           false, "userOff2",        P_TYPE::POINT   },
      { P_BEGIN_TEXT_PLACE,    false, "beginTextPlace",  P_TYPE::INT     },
      { P_CONTINUE_TEXT_PLACE, false, "beginTextPlace",  P_TYPE::INT     },
      { P_END_TEXT_PLACE,      false, "endTextPlace",    P_TYPE::INT     },
      { P_BEGIN_HOOK,          false, "beginHook",       P_TYPE::BOOL    },
      { P_END_HOOK,            false, "endHook",         P_TYPE::BOOL    },

      { P_BEGIN_HOOK_HEIGHT,   false, "beginHookHeight", P_TYPE::SPATIUM },
      { P_END_HOOK_HEIGHT,     false, "endHookHeight",   P_TYPE::SPATIUM },
      { P_BEGIN_HOOK_TYPE,     false, "beginHookType",   P_TYPE::INT     },
      { P_END_HOOK_TYPE,       false, "endHookType",     P_TYPE::INT     },
      { P_BEGIN_TEXT,          true,  "beginText",       P_TYPE::STRING  },
      { P_CONTINUE_TEXT,       true,  "continueText",    P_TYPE::STRING  },
      { P_END_TEXT,            true,  "endText",         P_TYPE::STRING  },
      { P_BEGIN_TEXT_STYLE,    false, "beginTextStyle",    P_TYPE::TEXT_STYLE },
      { P_CONTINUE_TEXT_STYLE, false, "continueTextStyle", P_TYPE::TEXT_STYLE },
      { P_END_TEXT_STYLE,      false, "endTextStyle",      P_TYPE::TEXT_STYLE },

      { P_BREAK_MMR,           false, "breakMultiMeasureRest", P_TYPE::BOOL },
      { P_REPEAT_COUNT,        true,  "endRepeat",       P_TYPE::INT  },
      { P_USER_STRETCH,        false, "stretch",         P_TYPE::REAL },
      { P_NO_OFFSET,           false, "noOffset",        P_TYPE::INT  },
      { P_IRREGULAR,           true,  "irregular",       P_TYPE::BOOL },

      { P_END,                 false, "",              P_TYPE::INT      }
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
            case P_TYPE::BOOL:
                  return QVariant(bool(e.readInt()));
            case P_TYPE::SUBTYPE:
            case P_TYPE::INT:
                  return QVariant(e.readInt());
            case P_TYPE::REAL:
            case P_TYPE::SPATIUM:
            case P_TYPE::SP_REAL:
            case P_TYPE::TEMPO:
                  return QVariant(e.readDouble());
            case P_TYPE::FRACTION:
                  return QVariant::fromValue(e.readFraction());
            case P_TYPE::COLOR:
                  return QVariant(e.readColor());
            case P_TYPE::POINT:
                  return QVariant(e.readPoint());
            case P_TYPE::SCALE:
            case P_TYPE::SIZE:
                  return QVariant(e.readSize());
            case P_TYPE::STRING:
                  return QVariant(e.readElementText());
            case P_TYPE::DIRECTION:
                  {
                  QString value(e.readElementText());
                  if (value == "up")
                        return QVariant(int(Direction::UP));
                  else if (value == "down")
                        return QVariant(int(Direction::DOWN));
                  else if (value == "auto")
                        return QVariant(int(Direction::AUTO));
                  }
                  break;
            case P_TYPE::DIRECTION_H:
                  {
                  QString value(e.readElementText());
                  if (value == "left" || value == "1")
                        return QVariant(int(DirectionH::DH_LEFT));
                  else if (value == "right" || value == "2")
                        return QVariant(int(DirectionH::DH_RIGHT));
                  else if (value == "auto")
                        return QVariant(int(DirectionH::DH_AUTO));
                  }
                  break;
            case P_TYPE::LAYOUT_BREAK: {
                  QString value(e.readElementText());
                  if (value == "line")
                        return QVariant(int(LayoutBreak::LayoutBreakType::LINE));
                  if (value == "page")
                        return QVariant(int(LayoutBreak::LayoutBreakType::PAGE));
                  if (value == "section")
                        return QVariant(int(LayoutBreak::LayoutBreakType::SECTION));
                  qDebug("getProperty: invalid P_TYPE::LAYOUT_BREAK: <%s>", qPrintable(value));
                  }
                  break;
            case P_TYPE::VALUE_TYPE: {
                  QString value(e.readElementText());
                  if (value == "offset")
                        return QVariant(int(ValueType::OFFSET_VAL));
                  else if (value == "user")
                        return QVariant(int(ValueType::USER_VAL));
                  }
                  break;
            case P_TYPE::PLACEMENT: {
                  QString value(e.readElementText());
                  if (value == "above")
                        return QVariant(int(Element::Placement::ABOVE));
                  else if (value == "below")
                        return QVariant(int(Element::Placement::BELOW));
                  }
                  break;
            case P_TYPE::BEAM_MODE:             // TODO
                  return QVariant(int(0));

            case P_TYPE::GROUPS:
                  {
                  Groups g;
                  g.read(e);
                  return QVariant::fromValue(g);
                  }
            case P_TYPE::POINT_MM:
            case P_TYPE::SIZE_MM:
            case P_TYPE::SYMID:
            case P_TYPE::TEXT_STYLE:
                  return QVariant();
            }
      return QVariant();
      }

}

