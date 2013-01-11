//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

//---------------------------------------------------------
//   PropertyData
//---------------------------------------------------------

struct PropertyData {
      P_ID id;
      const char* name;       // xml name of property
      P_TYPE type;
      };

//
// always: property[subtype].id == subtype
//
//
static const PropertyData propertyList[] = {
      { P_SUBTYPE,             "subtype",       T_INT   },
      { P_SELECTED,            "selected",      T_BOOL  },
      { P_COLOR,               "color",         T_COLOR },
      { P_VISIBLE,             "visible",       T_BOOL  },
      { P_SMALL,               "small",         T_BOOL  },
      { P_SHOW_COURTESY,       "",              T_INT   },
      { P_LINE_TYPE,           "",              T_INT   },
      { P_PITCH,               "pitch",         T_INT },
      { P_TPC,                 "tpc",           T_INT },
      { P_HEAD_TYPE,           "headType",      T_INT },

      { P_HEAD_GROUP,          "head",          T_INT },
      { P_VELO_TYPE,           "veloType",      T_VALUE_TYPE },
      { P_VELO_OFFSET,         "velocity",      T_INT },
      { P_ARTICULATION_ANCHOR, "",              T_INT },
      { P_DIRECTION,           "direction",     T_DIRECTION },
      { P_STEM_DIRECTION,      "StemDirection", T_DIRECTION },
      { P_NO_STEM,             "",              T_INT },
      { P_SLUR_DIRECTION,      "",              T_INT },
      { P_LEADING_SPACE,       "",              T_INT },
      { P_TRAILING_SPACE,      "",              T_INT },

      { P_DISTRIBUTE,          "distribute",    T_BOOL },
      { P_MIRROR_HEAD,         "mirror",        T_DIRECTION_H },
      { P_DOT_POSITION,        "dotPosition",   T_DIRECTION },
      { P_ONTIME_OFFSET,       "onTimeOffset",  T_INT },
      { P_OFFTIME_OFFSET,      "offTimeOffset", T_INT },
      { P_TUNING,              "tuning",        T_REAL },
      { P_PAUSE,               "pause",         T_REAL },
      { P_BARLINE_SPAN,        "",              T_INT },
      { P_USER_OFF,            0,               T_POINT },
      { P_FRET,                "fret",          T_INT   },

      { P_STRING,              "string",        T_INT   },
      { P_GHOST,               "ghost",         T_BOOL  },
      { P_TIMESIG_NOMINAL,     0,               T_FRACTION },
      { P_TIMESIG_ACTUAL,      0,               T_FRACTION },
      { P_NUMBER_TYPE,         "numberType",    T_INT   },
      { P_BRACKET_TYPE,        "bracketType",   T_INT   },
      { P_NORMAL_NOTES,        "normalNotes",   T_INT   },
      { P_ACTUAL_NOTES,        "actualNotes",   T_INT   },
      { P_P1,                  "p1",            T_POINT },
      { P_P2,                  "p2",            T_POINT },

      { P_GROW_LEFT,           "growLeft",      T_REAL  },
      { P_GROW_RIGHT,          "growRight",     T_REAL  },
      { P_BOX_HEIGHT,          "height",        T_REAL  },
      { P_BOX_WIDTH,           "width",         T_REAL  },
      { P_TOP_GAP,             "topGap",        T_SREAL },
      { P_BOTTOM_GAP,          "bottomGap",     T_SREAL },
      { P_LEFT_MARGIN,         "leftMargin",    T_REAL  },
      { P_RIGHT_MARGIN,        "rightMargin",   T_REAL  },
      { P_TOP_MARGIN,          "topMargin",     T_REAL  },
      { P_BOTTOM_MARGIN,       "bottomMargin",  T_REAL  },

      { P_LAYOUT_BREAK,        "subtype",       T_LAYOUT_BREAK },
      { P_AUTOSCALE,           "autoScale",     T_BOOL   },
      { P_SIZE,                "size",            T_SIZE },
      { P_SCALE,               0,                 T_SCALE  },
      { P_LOCK_ASPECT_RATIO,   "lockAspectRatio", T_BOOL },
      { P_SIZE_IS_SPATIUM,     "sizeIsSpatium",   T_BOOL },
      { P_TEXT_STYLE,          "textStyle",       T_INT  },
      { P_USER_MODIFIED,       0,               T_BOOL   },
      { P_BEAM_POS,            0,               T_POINT  },
      { P_BEAM_MODE,           "BeamMode",      T_BEAM_MODE  },

      { P_USER_LEN,            "",              T_REAL   },
      { P_SPACE,               "space",         T_REAL   },
      { P_TEMPO,               "tempo",         T_REAL   },
      { P_TEMPO_FOLLOW_TEXT,   "followText",    T_BOOL   },
      { P_ACCIDENTAL_BRACKET,  "bracket",       T_BOOL   },
      { P_NUMERATOR_STRING,    "textN",         T_STRING },
      { P_DENOMINATOR_STRING,  "textD",         T_STRING },
      { P_SHOW_NATURALS,       "showNaturals",  T_BOOL   },
      { P_BREAK_HINT,          "",              T_BOOL   },
      { P_FBPREFIX,            "prefix",        T_INT    },

      { P_FBDIGIT,             "digit",         T_INT    },
      { P_FBSUFFIX,            "suffix",        T_INT    },
      { P_FBCONTINUATIONLINE,  "continuationLine", T_BOOL},
      { P_FBPARENTHESIS1,      "",              T_INT    },
      { P_FBPARENTHESIS2,      "",              T_INT    },
      { P_FBPARENTHESIS3,      "",              T_INT    },
      { P_FBPARENTHESIS4,      "",              T_INT    },
      { P_FBPARENTHESIS5,      "",              T_INT    },
      { P_VOLTA_TYPE,          "",              T_INT    },
      { P_OTTAVA_TYPE,         "",              T_INT    },

      { P_TRILL_TYPE,          "",              T_INT    },
      { P_HAIRPIN_TYPE,        "",              T_INT    },
      { P_VELO_CHANGE,         "",              T_INT    },
      { P_DYNAMIC_RANGE,       "dynType",       T_INT    },
      { P_PLACEMENT,           "placement",     T_PLACEMENT    },
      { P_VELOCITY,            "velocity",      T_INT    },

      { P_END,                 "",              T_INT    }
      };

//---------------------------------------------------------
//   propertyType
//---------------------------------------------------------

P_TYPE propertyType(P_ID id)
      {
      return propertyList[id].type;
      }

//---------------------------------------------------------
//   propertyName
//---------------------------------------------------------

const char* propertyName(P_ID id)
      {
      return propertyList[id].name;
      }

//---------------------------------------------------------
// getProperty
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
            case T_SREAL:
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
                        return QVariant(int(LAYOUT_BREAK_LINE));
                  if (value == "page")
                        return QVariant(int(LAYOUT_BREAK_PAGE));
                  if (value == "section")
                        return QVariant(int(LAYOUT_BREAK_SECTION));
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
            }
      return QVariant();
      }

