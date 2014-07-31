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
      { P_ID::SUBTYPE,             false, "subtype",       P_TYPE::INT   },
      { P_ID::SELECTED,            false, "selected",      P_TYPE::BOOL  },
      { P_ID::GENERATED,           false, "",              P_TYPE::BOOL  },
      { P_ID::COLOR,               false, "color",         P_TYPE::COLOR },
      { P_ID::VISIBLE,             false, "visible",       P_TYPE::BOOL  },
      { P_ID::SMALL,               false, "small",         P_TYPE::BOOL  },
      { P_ID::SHOW_COURTESY,       false, "",              P_TYPE::INT   },
      { P_ID::LINE_TYPE,           false, "",              P_TYPE::INT   },
      { P_ID::PITCH,               true,  "pitch",         P_TYPE::INT   },
      { P_ID::TPC1,                false, "tpc",           P_TYPE::INT   },
      { P_ID::TPC2,                false, "tpc2",          P_TYPE::INT   },

      { P_ID::LINE,                false, "line",          P_TYPE::INT   },
      { P_ID::HEAD_TYPE,           false, "headType",      P_TYPE::INT   },
      { P_ID::HEAD_GROUP,          false, "head",          P_TYPE::INT },
      { P_ID::VELO_TYPE,           false, "veloType",      P_TYPE::VALUE_TYPE },
      { P_ID::VELO_OFFSET,         false, "velocity",      P_TYPE::INT },
      { P_ID::ARTICULATION_ANCHOR, false, "anchor",        P_TYPE::INT },
      { P_ID::DIRECTION,           false, "direction",     P_TYPE::DIRECTION },
      { P_ID::STEM_DIRECTION,      true,  "StemDirection", P_TYPE::DIRECTION },
      { P_ID::NO_STEM,             false, "",              P_TYPE::INT },
      { P_ID::SLUR_DIRECTION,      false, "",              P_TYPE::INT },

      { P_ID::LEADING_SPACE,       false, "",              P_TYPE::SPATIUM },
      { P_ID::TRAILING_SPACE,      false, "",              P_TYPE::SPATIUM },
      { P_ID::DISTRIBUTE,          false, "distribute",    P_TYPE::BOOL },
      { P_ID::MIRROR_HEAD,         false, "mirror",        P_TYPE::DIRECTION_H },
      { P_ID::DOT_POSITION,        false, "dotPosition",   P_TYPE::DIRECTION },
      { P_ID::TUNING,              false, "tuning",        P_TYPE::REAL  },
      { P_ID::PAUSE,               false, "pause",         P_TYPE::REAL  },
      { P_ID::BARLINE_SPAN,        false, "",              P_TYPE::INT   },
      { P_ID::BARLINE_SPAN_FROM,   false, 0,               P_TYPE::INT   },
      { P_ID::BARLINE_SPAN_TO,     false, 0,               P_TYPE::INT   },

      { P_ID::USER_OFF,            false, "userOff",       P_TYPE::POINT },
      { P_ID::FRET,                false, "fret",          P_TYPE::INT   },
      { P_ID::STRING,              false, "string",        P_TYPE::INT   },
      { P_ID::GHOST,               false, "ghost",         P_TYPE::BOOL  },
      { P_ID::PLAY,                false, "play",          P_TYPE::BOOL  },
      { P_ID::TIMESIG_NOMINAL,     false, 0,               P_TYPE::FRACTION },
      { P_ID::TIMESIG_ACTUAL,      true,  0,               P_TYPE::FRACTION },

      { P_ID::NUMBER_TYPE,         false, "numberType",    P_TYPE::INT   },
      { P_ID::BRACKET_TYPE,        false, "bracketType",   P_TYPE::INT   },
      { P_ID::NORMAL_NOTES,        false, "normalNotes",   P_TYPE::INT   },

      { P_ID::ACTUAL_NOTES,        false, "actualNotes",   P_TYPE::INT   },
      { P_ID::P1,                  false, "p1",            P_TYPE::POINT },
      { P_ID::P2,                  false, "p2",            P_TYPE::POINT },
      { P_ID::GROW_LEFT,           false, "growLeft",      P_TYPE::REAL    },
      { P_ID::GROW_RIGHT,          false, "growRight",     P_TYPE::REAL    },
      { P_ID::BOX_HEIGHT,          false, "height",        P_TYPE::SPATIUM },
      { P_ID::BOX_WIDTH,           false, "width",         P_TYPE::SPATIUM },
      { P_ID::TOP_GAP,             false, "topGap",        P_TYPE::SP_REAL },
      { P_ID::BOTTOM_GAP,          false, "bottomGap",     P_TYPE::SP_REAL },
      { P_ID::LEFT_MARGIN,         false, "leftMargin",    P_TYPE::REAL    },

      { P_ID::RIGHT_MARGIN,        false, "rightMargin",   P_TYPE::REAL    },
      { P_ID::TOP_MARGIN,          false, "topMargin",     P_TYPE::REAL    },
      { P_ID::BOTTOM_MARGIN,       false, "bottomMargin",  P_TYPE::REAL    },
      { P_ID::LAYOUT_BREAK,        false, "subtype",       P_TYPE::LAYOUT_BREAK },
      { P_ID::AUTOSCALE,           false, "autoScale",     P_TYPE::BOOL   },
      { P_ID::SIZE,                false, "size",            P_TYPE::SIZE },
      { P_ID::SCALE,               false, 0,                 P_TYPE::SCALE  },
      { P_ID::LOCK_ASPECT_RATIO,   false, "lockAspectRatio", P_TYPE::BOOL },
      { P_ID::SIZE_IS_SPATIUM,     false, "sizeIsSpatium",   P_TYPE::BOOL },
      { P_ID::TEXT_STYLE,          false, "textStyle",       P_TYPE::TEXT_STYLE  },

      { P_ID::TEXT_STYLE_TYPE,     false, "textStyleType", P_TYPE::INT  },
      { P_ID::TEXT,                false, 0,               P_TYPE::STRING },
      { P_ID::HTML_TEXT,           false, 0,               P_TYPE::STRING },
      { P_ID::USER_MODIFIED,       false, 0,               P_TYPE::BOOL   },
      { P_ID::BEAM_POS,            false, 0,               P_TYPE::POINT  },
      { P_ID::BEAM_MODE,           true, "BeamMode",       P_TYPE::BEAM_MODE  },
      { P_ID::BEAM_NO_SLOPE,       true, "noSlope",        P_TYPE::BOOL   },
      { P_ID::USER_LEN,            false, "",              P_TYPE::REAL   },
      { P_ID::SPACE,               false, "space",         P_TYPE::SP_REAL},
      { P_ID::TEMPO,               false, "tempo",         P_TYPE::TEMPO  },

      { P_ID::TEMPO_FOLLOW_TEXT,   false, "followText",    P_TYPE::BOOL   },
      { P_ID::ACCIDENTAL_BRACKET,  false, "bracket",       P_TYPE::BOOL   },
      { P_ID::NUMERATOR_STRING,    false, "textN",         P_TYPE::STRING },
      { P_ID::DENOMINATOR_STRING,  false, "textD",         P_TYPE::STRING },
      { P_ID::BREAK_HINT,          false, "",              P_TYPE::BOOL   },
      { P_ID::FBPREFIX,            false, "prefix",        P_TYPE::INT    },
      { P_ID::FBDIGIT,             false, "digit",         P_TYPE::INT    },
      { P_ID::FBSUFFIX,            false, "suffix",        P_TYPE::INT    },
      { P_ID::FBCONTINUATIONLINE,  false, "continuationLine", P_TYPE::INT },
      { P_ID::FBPARENTHESIS1,      false, "",              P_TYPE::INT    },

      { P_ID::FBPARENTHESIS2,      false, "",              P_TYPE::INT    },
      { P_ID::FBPARENTHESIS3,      false, "",              P_TYPE::INT    },
      { P_ID::FBPARENTHESIS4,      false, "",              P_TYPE::INT    },
      { P_ID::FBPARENTHESIS5,      false, "",              P_TYPE::INT    },
      { P_ID::VOLTA_TYPE,          false, "",              P_TYPE::INT    },
      { P_ID::OTTAVA_TYPE,         false, "",              P_TYPE::INT    },
      { P_ID::NUMBERS_ONLY,        false, "numbersOnly",   P_TYPE::BOOL   },
      { P_ID::TRILL_TYPE,          false, "",              P_TYPE::INT    },
      { P_ID::HAIRPIN_CIRCLEDTIP,  false, "hairpinCircledTip", P_TYPE::BOOL     },
      { P_ID::HAIRPIN_TYPE,        false, "",              P_TYPE::INT     },

      { P_ID::HAIRPIN_HEIGHT,      false, "hairpinHeight",     P_TYPE::SPATIUM },
      { P_ID::HAIRPIN_CONT_HEIGHT, false, "hairpinContHeight", P_TYPE::SPATIUM },
      { P_ID::VELO_CHANGE,         false, "",              P_TYPE::INT    },
      { P_ID::DYNAMIC_RANGE,       false, "dynType",       P_TYPE::INT    },
      { P_ID::PLACEMENT,           false, "placement",     P_TYPE::PLACEMENT    },
      { P_ID::VELOCITY,            false, "velocity",      P_TYPE::INT    },
      { P_ID::JUMP_TO,             false, "jumpTo",        P_TYPE::STRING },
      { P_ID::PLAY_UNTIL,          false, "playUntil",     P_TYPE::STRING },
      { P_ID::CONTINUE_AT,         false, "continueAt",    P_TYPE::STRING },
      { P_ID::LABEL,               false, "label",         P_TYPE::STRING },

      { P_ID::MARKER_TYPE,         false, 0,               P_TYPE::INT    },
      { P_ID::ARP_USER_LEN1,       false, 0,               P_TYPE::REAL   },
      { P_ID::ARP_USER_LEN2,       false, 0,               P_TYPE::REAL   },
      { P_ID::REPEAT_FLAGS,        false,  0,              P_TYPE::INT    },
      { P_ID::END_BARLINE_TYPE,    false, 0,               P_TYPE::INT    },
      { P_ID::END_BARLINE_VISIBLE, false, 0,               P_TYPE::BOOL   },
      { P_ID::END_BARLINE_COLOR,   false, 0,               P_TYPE::COLOR  },
      { P_ID::MEASURE_NUMBER_MODE, false, 0,               P_TYPE::INT    },
      { P_ID::GLISS_TYPE,          false, 0,               P_TYPE::INT    },
      { P_ID::GLISS_TEXT,          false, 0,               P_TYPE::STRING },

      { P_ID::GLISS_SHOW_TEXT,     false, 0,               P_TYPE::BOOL   },
      { P_ID::DIAGONAL,            false, 0,               P_TYPE::BOOL     },
      { P_ID::GROUPS,              false, 0,               P_TYPE::GROUPS   },
      { P_ID::LINE_STYLE,          false, "lineStyle",     P_TYPE::INT      },
      { P_ID::LINE_COLOR,          false, 0,               P_TYPE::COLOR    },
      { P_ID::LINE_WIDTH,          false, 0,               P_TYPE::SPATIUM  },
      { P_ID::LASSO_POS,           false, 0,               P_TYPE::POINT_MM },
      { P_ID::LASSO_SIZE,          false, 0,               P_TYPE::SIZE_MM  },
      { P_ID::TIME_STRETCH,        false, 0,               P_TYPE::REAL     },
      { P_ID::TIMESIG,             false, 0,               P_TYPE::FRACTION },

      { P_ID::TIMESIG_GLOBAL,      false, 0,               P_TYPE::FRACTION },
      { P_ID::TIMESIG_TYPE,        true,  0,               P_TYPE::INT },
      { P_ID::SPANNER_TICK,        true,  "tick",          P_TYPE::INT      },
      { P_ID::SPANNER_TICK2,       true,  "tick2",         P_TYPE::INT      },
      { P_ID::SPANNER_TRACK2,      true,  "track2",          P_TYPE::INT      },
      { P_ID::USER_OFF2,           false, "userOff2",        P_TYPE::POINT   },
      { P_ID::BEGIN_TEXT_PLACE,    false, "beginTextPlace",  P_TYPE::INT     },
      { P_ID::CONTINUE_TEXT_PLACE, false, "continueTextPlace",  P_TYPE::INT     },
      { P_ID::END_TEXT_PLACE,      false, "endTextPlace",    P_TYPE::INT     },
      { P_ID::BEGIN_HOOK,          false, "beginHook",       P_TYPE::BOOL    },
      { P_ID::END_HOOK,            false, "endHook",         P_TYPE::BOOL    },

      { P_ID::BEGIN_HOOK_HEIGHT,   false, "beginHookHeight", P_TYPE::SPATIUM },
      { P_ID::END_HOOK_HEIGHT,     false, "endHookHeight",   P_TYPE::SPATIUM },
      { P_ID::BEGIN_HOOK_TYPE,     false, "beginHookType",   P_TYPE::INT     },
      { P_ID::END_HOOK_TYPE,       false, "endHookType",     P_TYPE::INT     },
      { P_ID::BEGIN_TEXT,          true,  "beginText",       P_TYPE::STRING  },
      { P_ID::CONTINUE_TEXT,       true,  "continueText",    P_TYPE::STRING  },
      { P_ID::END_TEXT,            true,  "endText",         P_TYPE::STRING  },
      { P_ID::BEGIN_TEXT_STYLE,    false, "beginTextStyle",    P_TYPE::TEXT_STYLE },
      { P_ID::CONTINUE_TEXT_STYLE, false, "continueTextStyle", P_TYPE::TEXT_STYLE },
      { P_ID::END_TEXT_STYLE,      false, "endTextStyle",      P_TYPE::TEXT_STYLE },

      { P_ID::BREAK_MMR,           false, "breakMultiMeasureRest", P_TYPE::BOOL },
      { P_ID::REPEAT_COUNT,        true,  "endRepeat",       P_TYPE::INT  },
      { P_ID::USER_STRETCH,        false, "stretch",         P_TYPE::REAL },
      { P_ID::NO_OFFSET,           false, "noOffset",        P_TYPE::INT  },
      { P_ID::IRREGULAR,           true,  "irregular",       P_TYPE::BOOL },

      { P_ID::ANCHOR,              false,  "anchor",       P_TYPE::INT },

      { P_ID::SLUR_UOFF1,          false,  "o1",             P_TYPE::POINT  },
      { P_ID::SLUR_UOFF2,          false,  "o2",             P_TYPE::POINT  },
      { P_ID::SLUR_UOFF3,          false,  "o3",             P_TYPE::POINT  },
      { P_ID::SLUR_UOFF4,          false,  "o4",             P_TYPE::POINT  },

      { P_ID::END,                 false, "",              P_TYPE::INT      }
      };

//---------------------------------------------------------
//   propertyType
//---------------------------------------------------------

P_TYPE propertyType(P_ID id)
      {
      return propertyList[int(id)].type;
      }

//---------------------------------------------------------
//   propertyLink
//---------------------------------------------------------

bool propertyLink(P_ID id)
      {
      return propertyList[int(id)].link;
      }

//---------------------------------------------------------
//   propertyName
//---------------------------------------------------------

const char* propertyName(P_ID id)
      {
      return propertyList[int(id)].name;
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
                        return QVariant(int(MScore::Direction::UP));
                  else if (value == "down")
                        return QVariant(int(MScore::Direction::DOWN));
                  else if (value == "auto")
                        return QVariant(int(MScore::Direction::AUTO));
                  }
                  break;
            case P_TYPE::DIRECTION_H:
                  {
                  QString value(e.readElementText());
                  if (value == "left" || value == "1")
                        return QVariant(int(MScore::DirectionH::LEFT));
                  else if (value == "right" || value == "2")
                        return QVariant(int(MScore::DirectionH::RIGHT));
                  else if (value == "auto")
                        return QVariant(int(MScore::DirectionH::AUTO));
                  }
                  break;
            case P_TYPE::LAYOUT_BREAK: {
                  QString value(e.readElementText());
                  if (value == "line")
                        return QVariant(int(LayoutBreak::Type::LINE));
                  if (value == "page")
                        return QVariant(int(LayoutBreak::Type::PAGE));
                  if (value == "section")
                        return QVariant(int(LayoutBreak::Type::SECTION));
                  qDebug("getProperty: invalid P_TYPE::LAYOUT_BREAK: <%s>", qPrintable(value));
                  }
                  break;
            case P_TYPE::VALUE_TYPE: {
                  QString value(e.readElementText());
                  if (value == "offset")
                        return QVariant(int(Note::ValueType::OFFSET_VAL));
                  else if (value == "user")
                        return QVariant(int(Note::ValueType::USER_VAL));
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

