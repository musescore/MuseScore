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
#include "accidental.h"
#include "bracket.h"
#include "clef.h"
#include "dynamic.h"
#include "mscore.h"
#include "ottava.h"
#include "tremolo.h"
#include "trill.h"
#include "vibrato.h"
#include "layoutbreak.h"
#include "groups.h"
#include "xml.h"
#include "note.h"
#include "barline.h"
#include "style.h"
#include "sym.h"
#include "changeMap.h"
#include "fret.h"

namespace Ms {

//---------------------------------------------------------
//   PropertyMetaData
//---------------------------------------------------------

struct PropertyMetaData {
      Pid id;                 // associated Pid
      bool link;              // link this property for linked elements
      const char* name;       // xml name of property
      P_TYPE type;            // associated P_TYPE
      const char* userName;   // user-visible name of property
      };

//
// always: propertyList[subtype].id == subtype
//
//

//keep this properties untranslatable for now until we put the same strings to all UI elements
#define DUMMY_QT_TRANSLATE_NOOP(x, y) y
static constexpr PropertyMetaData propertyList[] = {
      { Pid::SUBTYPE,                 false, "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::SELECTED,                false, "selected",              P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "selected")         },
      { Pid::GENERATED,               false, "generated",             P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "generated")        },
      { Pid::COLOR,                   false, "color",                 P_TYPE::COLOR,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "color")            },
      { Pid::VISIBLE,                 false, "visible",               P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "visible")          },
      { Pid::Z,                       false, "z",                     P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "z")                },
      { Pid::SMALL,                   false, "small",                 P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "small")            },
      { Pid::SHOW_COURTESY,           false, "showCourtesySig",       P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "show courtesy")    },
      { Pid::KEYSIG_MODE,             false, "keysig_mode",           P_TYPE::KEYMODE,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "show courtesy")    },
      { Pid::LINE_TYPE,               false, "lineType",              P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "line type")        },
      { Pid::PITCH,                   true,  "pitch",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "pitch")            },

      { Pid::TPC1,                    true,  "tpc",                   P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "tonal pitch class") },
      { Pid::TPC2,                    true,  "tpc2",                  P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "tonal pitch class") },
      { Pid::LINE,                    false, "line",                  P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "line")             },
      { Pid::FIXED,                   false, "fixed",                 P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "fixed")            },
      { Pid::FIXED_LINE,              false, "fixedLine",             P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "fixed line")       },
      { Pid::HEAD_TYPE,               false, "headType",              P_TYPE::HEAD_TYPE,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "head type")        },
      { Pid::HEAD_GROUP,              false, "head",                  P_TYPE::HEAD_GROUP,          DUMMY_QT_TRANSLATE_NOOP("propertyName", "head")             },
      { Pid::VELO_TYPE,               false, "veloType",              P_TYPE::VALUE_TYPE,          DUMMY_QT_TRANSLATE_NOOP("propertyName", "velocity type")    },
      { Pid::VELO_OFFSET,             false, "velocity",              P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "velocity")         },
      { Pid::ARTICULATION_ANCHOR,     false, "anchor",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "anchor")           },

      { Pid::DIRECTION,               false, "direction",             P_TYPE::DIRECTION,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "direction")        },
      { Pid::STEM_DIRECTION,          false, "StemDirection",         P_TYPE::DIRECTION,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "stem direction")   },
      { Pid::NO_STEM,                 false, "noStem",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "no stem")          },
      { Pid::SLUR_DIRECTION,          false, "up",                    P_TYPE::DIRECTION,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "up")               },
      { Pid::LEADING_SPACE,           false, "leadingSpace",          P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "leading space")    },
      { Pid::DISTRIBUTE,              false, "distribute",            P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "distributed")      },
      { Pid::MIRROR_HEAD,             false, "mirror",                P_TYPE::DIRECTION_H,         DUMMY_QT_TRANSLATE_NOOP("propertyName", "mirror")           },
      { Pid::DOT_POSITION,            false, "dotPosition",           P_TYPE::DIRECTION,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "dot position")     },
      { Pid::TUNING,                  false, "tuning",                P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "tuning")           },
      { Pid::PAUSE,                   true,  "pause",                 P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "pause")            },

      { Pid::BARLINE_TYPE,            false, "subtype",               P_TYPE::BARLINE_TYPE,        DUMMY_QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::BARLINE_SPAN,            false, "span",                  P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "span")             },
      { Pid::BARLINE_SPAN_FROM,       false, "spanFromOffset",        P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "span from")        },
      { Pid::BARLINE_SPAN_TO,         false, "spanToOffset",          P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "span to")          },
      { Pid::OFFSET,                  false, "offset",                P_TYPE::POINT_SP_MM,         DUMMY_QT_TRANSLATE_NOOP("propertyName", "offset")           },
      { Pid::FRET,                    true,  "fret",                  P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "fret")             },
      { Pid::STRING,                  true,  "string",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "string")           },
      { Pid::GHOST,                   true,  "ghost",                 P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "ghost")            },
      { Pid::PLAY,                    false, "play",                  P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "played")           },
      { Pid::TIMESIG_NOMINAL,         false, 0,                       P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "nominal time signature") },

      { Pid::TIMESIG_ACTUAL,          true,  0,                       P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "actual time signature") },
      { Pid::NUMBER_TYPE,             false, "numberType",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "number type")      },
      { Pid::BRACKET_TYPE,            false, "bracketType",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "bracket type")     },
      { Pid::NORMAL_NOTES,            false, "normalNotes",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "normal notes")     },
      { Pid::ACTUAL_NOTES,            false, "actualNotes",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "actual notes")     },
      { Pid::P1,                      false, "p1",                    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "p1")               },
      { Pid::P2,                      false, "p2",                    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "p2")               },
      { Pid::GROW_LEFT,               false, "growLeft",              P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "grow left")        },
      { Pid::GROW_RIGHT,              false, "growRight",             P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "grow right")       },
      { Pid::BOX_HEIGHT,              false, "height",                P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "height")           },

      { Pid::BOX_WIDTH,               false, "width",                 P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "width")            },
      { Pid::BOX_AUTOSIZE,            false, "boxAutoSize",           P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "autosize frame")   },
      { Pid::TOP_GAP,                 false, "topGap",                P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "top gap")          },
      { Pid::BOTTOM_GAP,              false, "bottomGap",             P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "bottom gap")       },
      { Pid::LEFT_MARGIN,             false, "leftMargin",            P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "left margin")      },
      { Pid::RIGHT_MARGIN,            false, "rightMargin",           P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "right margin")     },
      { Pid::TOP_MARGIN,              false, "topMargin",             P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "top margin")       },
      { Pid::BOTTOM_MARGIN,           false, "bottomMargin",          P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "bottom margin")    },
      { Pid::LAYOUT_BREAK,            false, "subtype",               P_TYPE::LAYOUT_BREAK,        DUMMY_QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::AUTOSCALE,               false, "autoScale",             P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "autoscale")        },
      { Pid::SIZE,                    false, "size",                  P_TYPE::SIZE,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "size")             },

      { Pid::SCALE,                   false, "scale",                 P_TYPE::SCALE,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "scale")            },
      { Pid::LOCK_ASPECT_RATIO,       false, "lockAspectRatio",       P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "aspect ratio locked") },
      { Pid::SIZE_IS_SPATIUM,         false, "sizeIsSpatium",         P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "size is spatium")  },
      { Pid::TEXT,                    true,  "text",                  P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "text")             },
      { Pid::HTML_TEXT,               false, 0,                       P_TYPE::STRING,              ""                                                    },
      { Pid::USER_MODIFIED,           false, 0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::BEAM_POS,                false, 0,                       P_TYPE::POINT,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "beam position")    },
      { Pid::BEAM_MODE,               true, "BeamMode",               P_TYPE::BEAM_MODE,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "beam mode")        },
      { Pid::BEAM_NO_SLOPE,           true, "noSlope",                P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "without slope")    },
      { Pid::USER_LEN,                false, "userLen",               P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "length")           },

      { Pid::SPACE,                   false, "space",                 P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "space")            },
      { Pid::TEMPO,                   true,  "tempo",                 P_TYPE::TEMPO,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "tempo")            },
      { Pid::TEMPO_FOLLOW_TEXT,       true,  "followText",            P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "following text")   },
      { Pid::ACCIDENTAL_BRACKET,      false, "bracket",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "bracket")          },
      { Pid::ACCIDENTAL_TYPE,         true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "type")             },
      { Pid::NUMERATOR_STRING,        false, "textN",                 P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "numerator string") },
      { Pid::DENOMINATOR_STRING,      false, "textD",                 P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "denominator string") },
      { Pid::FBPREFIX,                false, "prefix",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "prefix")           },
      { Pid::FBDIGIT,                 false, "digit",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "digit")            },
      { Pid::FBSUFFIX,                false, "suffix",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "suffix")           },
      { Pid::FBCONTINUATIONLINE,      false, "continuationLine",      P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "continuation line") },

      { Pid::FBPARENTHESIS1,          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS2,          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS3,          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS4,          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS5,          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::OTTAVA_TYPE,             true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "ottava type")      },
      { Pid::NUMBERS_ONLY,            false, "numbersOnly",           P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "numbers only")     },
      { Pid::TRILL_TYPE,              false, "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "trill type")       },
      { Pid::VIBRATO_TYPE,            false, "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "vibrato type")     },
      { Pid::HAIRPIN_CIRCLEDTIP,      false, "hairpinCircledTip",     P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "hairpin with circled tip") },

      { Pid::HAIRPIN_TYPE,            true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "hairpin type")     },
      { Pid::HAIRPIN_HEIGHT,          false, "hairpinHeight",         P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "hairpin height")   },
      { Pid::HAIRPIN_CONT_HEIGHT,     false, "hairpinContHeight",     P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "hairpin cont height") },
      { Pid::VELO_CHANGE,             true,  "veloChange",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "velocity change")  },
      { Pid::VELO_CHANGE_METHOD,      true,  "veloChangeMethod",      P_TYPE::CHANGE_METHOD,       DUMMY_QT_TRANSLATE_NOOP("propertyName", "velocity change method")   },     // left as a compatability property - we need to be able to read it correctly
      { Pid::VELO_CHANGE_SPEED,       true,  "veloChangeSpeed",       P_TYPE::CHANGE_SPEED,        DUMMY_QT_TRANSLATE_NOOP("propertyName", "velocity change speed")  },
      { Pid::DYNAMIC_TYPE,            true,  "subtype",               P_TYPE::DYNAMIC_TYPE,        DUMMY_QT_TRANSLATE_NOOP("propertyName", "dynamic type")     },
      { Pid::DYNAMIC_RANGE,           true,  "dynType",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "dynamic range")    },
//100
      { Pid::SINGLE_NOTE_DYNAMICS,    true,  "singleNoteDynamics",    P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "single note dynamics")   },
      { Pid::CHANGE_METHOD,           true,  "changeMethod",          P_TYPE::CHANGE_METHOD,       DUMMY_QT_TRANSLATE_NOOP("propertyName", "change method")   },        // the new, more general version of VELO_CHANGE_METHOD
      { Pid::PLACEMENT,               false, "placement",             P_TYPE::PLACEMENT,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "placement")        },
      { Pid::HPLACEMENT,              false, "hplacement",            P_TYPE::HPLACEMENT,          DUMMY_QT_TRANSLATE_NOOP("propertyName", "horizontal placement")   },
      { Pid::MMREST_RANGE_BRACKET_TYPE, false, "mmrestRangeBracketType", P_TYPE::INT,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "multimeasure rest range bracket type")   },
      { Pid::VELOCITY,                false, "velocity",              P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "velocity")         },
      { Pid::JUMP_TO,                 true,  "jumpTo",                P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "jump to")          },
      { Pid::PLAY_UNTIL,              true,  "playUntil",             P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "play until")       },
      { Pid::CONTINUE_AT,             true,  "continueAt",            P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue at")       },
      { Pid::LABEL,                   true,  "label",                 P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "label")            },
      { Pid::MARKER_TYPE,             true,  0,                       P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "marker type")      },
      { Pid::ARP_USER_LEN1,           false, 0,                       P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "length 1")         },
      { Pid::ARP_USER_LEN2,           false, 0,                       P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "length 2")         },
      { Pid::REPEAT_END,              true,  0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::REPEAT_START,            true,  0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::REPEAT_JUMP,             true,  0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::MEASURE_NUMBER_MODE,     false, "measureNumberMode",     P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "measure number mode") },

      { Pid::GLISS_TYPE,              false, "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::GLISS_TEXT,              false, 0,                       P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "text")             },
      { Pid::GLISS_SHOW_TEXT,         false, 0,                       P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "showing text")     },
      { Pid::GLISS_STYLE,             true,  "glissandoStyle",        P_TYPE::GLISS_STYLE,         DUMMY_QT_TRANSLATE_NOOP("propertyName", "glissando style") },
      { Pid::GLISS_EASEIN,            false, "easeInSpin",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "ease in")          },
      { Pid::GLISS_EASEOUT,           false, "easeOutSpin",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "ease out")         },
      { Pid::DIAGONAL,                false, 0,                       P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "diagonal")         },
      { Pid::GROUPS,                  false, 0,                       P_TYPE::GROUPS,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "groups")           },
      { Pid::LINE_STYLE,              false, "lineStyle",             P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "line style")       },
      { Pid::LINE_WIDTH,              false, "lineWidth",             P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "line width")       },
      { Pid::LINE_WIDTH_SPATIUM,      false, "lineWidth",             P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "line width (spatium)") },
      { Pid::LASSO_POS,               false, 0,                       P_TYPE::POINT_MM,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "lasso position")   },
      { Pid::LASSO_SIZE,              false, 0,                       P_TYPE::SIZE_MM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "lasso size")       },
      { Pid::TIME_STRETCH,            true,  "timeStretch",           P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "time stretch")     },
      { Pid::ORNAMENT_STYLE,          true,  "ornamentStyle",         P_TYPE::ORNAMENT_STYLE,      DUMMY_QT_TRANSLATE_NOOP("propertyName", "ornament style")   },

      { Pid::TIMESIG,                 false, "timesig",               P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "time signature")   },
      { Pid::TIMESIG_GLOBAL,          false, 0,                       P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "global time signature") },
      { Pid::TIMESIG_STRETCH,         false, 0,                       P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "time signature stretch") },
      { Pid::TIMESIG_TYPE,            true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::SPANNER_TICK,            true,  "tick",                  P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "tick")             },
      { Pid::SPANNER_TICKS,           true,  "ticks",                 P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "ticks")            },
      { Pid::SPANNER_TRACK2,          false, "track2",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "track2")           },
      { Pid::OFFSET2,                 false, "userOff2",              P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "offset2")         },
      { Pid::BREAK_MMR,               false, "breakMultiMeasureRest", P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "breaking multimeasure rest")},
      { Pid::MMREST_NUMBER_POS,       false, "mmRestNumberPos",       P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "vertical position of multimeasure rest number")},
      { Pid::REPEAT_COUNT,            true,  "endRepeat",             P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "end repeat")       },

      { Pid::USER_STRETCH,            false, "stretch",               P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "stretch")          },
      { Pid::NO_OFFSET,               true,  "noOffset",              P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "numbering offset") },
      { Pid::IRREGULAR,               true,  "irregular",             P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "irregular")        },
      { Pid::ANCHOR,                  false, "anchor",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "anchor")           },
      { Pid::SLUR_UOFF1,              false, "o1",                    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "o1")               },
      { Pid::SLUR_UOFF2,              false, "o2",                    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "o2")               },
      { Pid::SLUR_UOFF3,              false, "o3",                    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "o3")               },
      { Pid::SLUR_UOFF4,              false, "o4",                    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "o4")               },
      { Pid::STAFF_MOVE,              true,  "staffMove",             P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "staff move")       },
      { Pid::VERSE,                   true,  "no",                    P_TYPE::ZERO_INT,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "verse")            },

      { Pid::SYLLABIC,                true,  "syllabic",              P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "syllabic")         },
      { Pid::LYRIC_TICKS,             true,  "ticks_f",               P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "ticks")            },
      { Pid::VOLTA_ENDING,            true,  "endings",               P_TYPE::INT_LIST,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "endings")          },
      { Pid::LINE_VISIBLE,            true,  "lineVisible",           P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "visible line")     },
      { Pid::MAG,                     false, "mag",                   P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "mag")              },
      { Pid::USE_DRUMSET,             false, "useDrumset",            P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "using drumset")    },
      { Pid::DURATION,                true,  0,                       P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "duration")         },
      { Pid::DURATION_TYPE,           true,  0,                       P_TYPE::TDURATION,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "duration type")    },
      { Pid::ROLE,                    false, "role",                  P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "role")             },
      { Pid::TRACK,                   false, 0,                       P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "track")            },

      { Pid::FRET_STRINGS,            true,  "strings",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "strings")          },
      { Pid::FRET_FRETS,              true,  "frets",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "frets")            },
      { Pid::FRET_NUT,                true,  "showNut",               P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "show nut")         },
      { Pid::FRET_OFFSET,             true,  "fretOffset",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "fret offset")      },
      { Pid::FRET_NUM_POS,            true,  "fretNumPos",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "fret number position") },
      { Pid::ORIENTATION,             true,  "orientation",           P_TYPE::ORIENTATION,         DUMMY_QT_TRANSLATE_NOOP("propertyName", "orientation")      },

      { Pid::HARMONY_VOICE_LITERAL,   true,  "harmonyVoiceLiteral",   P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "harmony voice literal") },
      { Pid::HARMONY_VOICING,         true,  "harmonyVoicing",        P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "harmony voicing") },
      { Pid::HARMONY_DURATION,        true,  "harmonyDuration",       P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "harmony duration") },

      { Pid::SYSTEM_BRACKET,          false, "type",                  P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "type")             },
      { Pid::GAP,                     false, 0,                       P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "gap")              },
      { Pid::AUTOPLACE,               false, "autoplace",             P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "autoplace")        },
      { Pid::DASH_LINE_LEN,           false, "dashLineLength",        P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "dash line length") },
      { Pid::DASH_GAP_LEN,            false, "dashGapLength",         P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "dash gap length")  },
      { Pid::TICK,                    false, 0,                       P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "tick")             },
      { Pid::PLAYBACK_VOICE1,         false, "playbackVoice1",        P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "playback voice 1") },
      { Pid::PLAYBACK_VOICE2,         false, "playbackVoice2",        P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "playback voice 2") },
      { Pid::PLAYBACK_VOICE3,         false, "playbackVoice3",        P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "playback voice 3") },

      { Pid::PLAYBACK_VOICE4,         false, "playbackVoice4",        P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "playback voice 4") },
      { Pid::SYMBOL,                  true,  "symbol",                P_TYPE::SYMID,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "symbol")           },
      { Pid::PLAY_REPEATS,            true,  "playRepeats",           P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "playing repeats")  },
      { Pid::CREATE_SYSTEM_HEADER,    false, "createSystemHeader",    P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "creating system header") },
      { Pid::STAFF_LINES,             true,  "lines",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "lines")            },
      { Pid::LINE_DISTANCE,           true,  "lineDistance",          P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "line distance")    },
      { Pid::STEP_OFFSET,             true,  "stepOffset",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "step offset")      },
      { Pid::STAFF_SHOW_BARLINES,     false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "showing barlines") },
      { Pid::STAFF_SHOW_LEDGERLINES,  false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "showing ledgerlines") },
      { Pid::STAFF_STEMLESS,          false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "stemless")         },
      { Pid::STAFF_INVISIBLE,         false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "invisible")        },
      { Pid::STAFF_COLOR,             false, "color",                 P_TYPE::COLOR,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "color")            },

      { Pid::HEAD_SCHEME,             false, "headScheme",            P_TYPE::HEAD_SCHEME,         DUMMY_QT_TRANSLATE_NOOP("propertyName", "notehead scheme")  },
      { Pid::STAFF_GEN_CLEF,          false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "generating clefs") },
      { Pid::STAFF_GEN_TIMESIG,       false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "generating time signature") },
      { Pid::STAFF_GEN_KEYSIG,        false, "",                      P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "generating key signature")  },
      { Pid::STAFF_YOFFSET,           false, "",                      P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "y-offset")                 },
      { Pid::STAFF_USERDIST,          false, "distOffset",            P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "distance offset")  },
      { Pid::STAFF_BARLINE_SPAN,      false, "barLineSpan",           P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "barline span")     },
      { Pid::STAFF_BARLINE_SPAN_FROM, false, "barLineSpanFrom",       P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "barline span from") },
      { Pid::STAFF_BARLINE_SPAN_TO,   false, "barLineSpanTo",         P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "barline span to")  },
      { Pid::BRACKET_SPAN,            false, "bracketSpan",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "bracket span")     },

      { Pid::BRACKET_COLUMN,          false, "level",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "level")            },
      { Pid::INAME_LAYOUT_POSITION,   false, "layoutPosition",        P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "layout position")  },
//200
      { Pid::SUB_STYLE,               false, "style",                 P_TYPE::SUB_STYLE,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "style")            },
      { Pid::FONT_FACE,               false, "family",                P_TYPE::FONT,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "family")           },
      { Pid::FONT_SIZE,               false, "size",                  P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "size")             },
      { Pid::FONT_STYLE,              false, "fontStyle",             P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "font style")       },
      { Pid::TEXT_LINE_SPACING,       false, "textLineSpacing",       P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "user line distancing") },

      { Pid::FRAME_TYPE,              false, "frameType",             P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "frame type")       },
      { Pid::FRAME_WIDTH,             false, "frameWidth",            P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "frame width")      },
      { Pid::FRAME_PADDING,           false, "framePadding",          P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "frame padding")    },
      { Pid::FRAME_ROUND,             false, "frameRound",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "frame round")      },
      { Pid::FRAME_FG_COLOR,          false, "frameFgColor",          P_TYPE::COLOR,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "frame foreground color") },
      { Pid::FRAME_BG_COLOR,          false, "frameBgColor",          P_TYPE::COLOR,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "frame background color") },
      { Pid::SIZE_SPATIUM_DEPENDENT,  false, "sizeIsSpatiumDependent",P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "spatium dependent font") },
      { Pid::ALIGN,                   false, "align",                 P_TYPE::ALIGN,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "align")            },
      { Pid::SYSTEM_FLAG,             false, "systemFlag",            P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "system flag")      },
      { Pid::BEGIN_TEXT,              true,  "beginText",             P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin text")       },

      { Pid::BEGIN_TEXT_ALIGN,        false, "beginTextAlign",        P_TYPE::ALIGN,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin text align") },
      { Pid::BEGIN_TEXT_PLACE,        false, "beginTextPlace",        P_TYPE::TEXT_PLACE,          DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin text place") },
      { Pid::BEGIN_HOOK_TYPE,         false, "beginHookType",         P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin hook type")  },
      { Pid::BEGIN_HOOK_HEIGHT,       false, "beginHookHeight",       P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin hook height") },
      { Pid::BEGIN_FONT_FACE,         false, "beginFontFace",         P_TYPE::FONT,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin font face")  },
      { Pid::BEGIN_FONT_SIZE,         false, "beginFontSize",         P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin font size")  },
      { Pid::BEGIN_FONT_STYLE,        false, "beginFontStyle",        P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin font style")  },
      { Pid::BEGIN_TEXT_OFFSET,       false, "beginTextOffset",       P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "begin text offset")  },

      { Pid::CONTINUE_TEXT,           true,  "continueText",          P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue text")    },
      { Pid::CONTINUE_TEXT_ALIGN,     false, "continueTextAlign",     P_TYPE::ALIGN,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue text align") },
      { Pid::CONTINUE_TEXT_PLACE,     false, "continueTextPlace",     P_TYPE::TEXT_PLACE,          DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue text place") },
      { Pid::CONTINUE_FONT_FACE,      false, "continueFontFace",      P_TYPE::FONT,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue font face") },
      { Pid::CONTINUE_FONT_SIZE,      false, "continueFontSize",      P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue font size") },
      { Pid::CONTINUE_FONT_STYLE,     false, "continueFontStyle",     P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue font style") },
      { Pid::CONTINUE_TEXT_OFFSET,    false, "continueTextOffset",    P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "continue text offset") },
      { Pid::END_TEXT,                true,  "endText",               P_TYPE::STRING,              DUMMY_QT_TRANSLATE_NOOP("propertyName", "end text")         },

      { Pid::END_TEXT_ALIGN,          false, "endTextAlign",          P_TYPE::ALIGN,               DUMMY_QT_TRANSLATE_NOOP("propertyName", "end text align")   },
      { Pid::END_TEXT_PLACE,          false, "endTextPlace",          P_TYPE::TEXT_PLACE,          DUMMY_QT_TRANSLATE_NOOP("propertyName", "end text place")   },
      { Pid::END_HOOK_TYPE,           false, "endHookType",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "end hook type")    },
      { Pid::END_HOOK_HEIGHT,         false, "endHookHeight",         P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "end hook height")  },
      { Pid::END_FONT_FACE,           false, "endFontFace",           P_TYPE::FONT,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "end font face")    },
      { Pid::END_FONT_SIZE,           false, "endFontSize",           P_TYPE::REAL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "end font size")    },
      { Pid::END_FONT_STYLE,          false, "endFontStyle",          P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName",  "end font style")  },
      { Pid::END_TEXT_OFFSET,         false, "endTextOffset",         P_TYPE::POINT_SP,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "end text offset")  },

      { Pid::POS_ABOVE,               false, "posAbove",              P_TYPE::SP_REAL,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "position above")   },

      { Pid::LOCATION_STAVES,         false, "staves",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "staves distance")  },
      { Pid::LOCATION_VOICES,         false, "voices",                P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "voices distance")  },
      { Pid::LOCATION_MEASURES,       false, "measures",              P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "measures distance") },
      { Pid::LOCATION_FRACTIONS,      false, "fractions",             P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "position distance") },
      { Pid::LOCATION_GRACE,          false, "grace",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "grace note index") },
      { Pid::LOCATION_NOTE,           false, "note",                  P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "note index")       },

      { Pid::VOICE,                   false, "voice",                 P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "voice")            },
      { Pid::POSITION,                false, "position",              P_TYPE::FRACTION,            DUMMY_QT_TRANSLATE_NOOP("propertyName", "position")         },

      { Pid::CLEF_TYPE_CONCERT,       true,  "concertClefType",       P_TYPE::CLEF_TYPE,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "concert clef type") },
      { Pid::CLEF_TYPE_TRANSPOSING,   true,  "transposingClefType",   P_TYPE::CLEF_TYPE,           DUMMY_QT_TRANSLATE_NOOP("propertyName", "transposing clef type") },
      { Pid::KEY,                     true,  "accidental",            P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "key")              },
      { Pid::ACTION,                  false, "action",                P_TYPE::STRING,              0                                                           },
      { Pid::MIN_DISTANCE,            false, "minDistance",           P_TYPE::SPATIUM,             DUMMY_QT_TRANSLATE_NOOP("propertyName", "autoplace minimum distance") },

      { Pid::ARPEGGIO_TYPE,           true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "arpeggio type")    },
      { Pid::CHORD_LINE_TYPE,         true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "chord line type")  },
      { Pid::CHORD_LINE_STRAIGHT,     true,  "straight",              P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "straight chord line") },
      { Pid::TREMOLO_TYPE,            true,  "subtype",               P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "tremolo type")     },
      { Pid::TREMOLO_STYLE,           true,  "strokeStyle",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "tremolo style") },
      { Pid::HARMONY_TYPE,            true,  "harmonyType",           P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "harmony type") },

      { Pid::START_WITH_LONG_NAMES,   false, "startWithLongNames",    P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "start with long names")  },
      { Pid::START_WITH_MEASURE_ONE,  true,  "startWithMeasureOne",   P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "start with measure one") },
      { Pid::FIRST_SYSTEM_INDENTATION,true,  "firstSystemIndentation",P_TYPE::BOOL,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "first system indentation") },

      { Pid::PATH,                    false, "path",                  P_TYPE::PATH,                DUMMY_QT_TRANSLATE_NOOP("propertyName", "path") },

      { Pid::PREFER_SHARP_FLAT,       true,  "preferSharpFlat",       P_TYPE::INT,                 DUMMY_QT_TRANSLATE_NOOP("propertyName", "prefer sharps or flats") },

      { Pid::END, false, "++end++", P_TYPE::INT, DUMMY_QT_TRANSLATE_NOOP("propertyName", "<invalid property>") }
      };

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid propertyId(const QStringRef& s)
      {
      for (const PropertyMetaData& pd : propertyList) {
            if (pd.name == s)
                  return pd.id;
            }
      return Pid::END;
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid propertyId(const QString& s)
      {
      return propertyId(QStringRef(&s));
      }

//---------------------------------------------------------
//   propertyType
//---------------------------------------------------------

P_TYPE propertyType(Pid id)
      {
      Q_ASSERT( propertyList[int(id)].id == id);
      return propertyList[int(id)].type;
      }

//---------------------------------------------------------
//   propertyLink
//---------------------------------------------------------

bool propertyLink(Pid id)
      {
      Q_ASSERT( propertyList[int(id)].id == id);
      return propertyList[int(id)].link;
      }

//---------------------------------------------------------
//   propertyName
//---------------------------------------------------------

const char* propertyName(Pid id)
      {
      Q_ASSERT( propertyList[int(id)].id == id);
      return propertyList[int(id)].name;
      }

//---------------------------------------------------------
//   propertyUserName
//---------------------------------------------------------

QString propertyUserName(Pid id)
      {
      Q_ASSERT(propertyList[int(id)].id == id);
      return QObject::tr(propertyList[int(id)].userName, "propertyName");
      }

//---------------------------------------------------------
//    propertyFromString
//---------------------------------------------------------

QVariant propertyFromString(Pid id, QString value)
      {
      switch (propertyType(id)) {
            case P_TYPE::BOOL:
                  return QVariant(bool(value.toInt()));
            case P_TYPE::ZERO_INT:
            case P_TYPE::INT:
                  return QVariant(value.toInt());
            case P_TYPE::REAL:
            case P_TYPE::SPATIUM:
            case P_TYPE::SP_REAL:
            case P_TYPE::TEMPO:
                  return QVariant(value.toDouble());
            case P_TYPE::FRACTION:
                  return Fraction::fromString(value);
            case P_TYPE::COLOR:
                  // not used by MSCX
                  return QColor(value);
            case P_TYPE::POINT:
            case P_TYPE::POINT_SP:
            case P_TYPE::POINT_SP_MM: {
                  // not used by MSCX
                  const int i = value.indexOf(';');
                  return QPointF(value.leftRef(i).toDouble(), value.midRef(i+1).toDouble());
                  }
            case P_TYPE::SCALE:
            case P_TYPE::SIZE: {
                  // not used by MSCX
                  const int i = value.indexOf('x');
                  return QSizeF(value.leftRef(i).toDouble(), value.midRef(i+1).toDouble());
                  }
            case P_TYPE::FONT:
            case P_TYPE::STRING:
                  return value;
            case P_TYPE::GLISS_STYLE: {
                  if ( value == "whitekeys")
                        return QVariant(int(GlissandoStyle::WHITE_KEYS));
                  else if ( value == "blackkeys")
                        return QVariant(int(GlissandoStyle::BLACK_KEYS));
                  else if ( value == "diatonic")
                        return QVariant(int(GlissandoStyle::DIATONIC));
                  else if ( value == "portamento")
                        return QVariant(int(GlissandoStyle::PORTAMENTO));
                  else // e.g., normally "Chromatic"
                        return QVariant(int(GlissandoStyle::CHROMATIC));
                  }
                  break;
            case P_TYPE::ORNAMENT_STYLE: {
                  if ( value == "baroque")
                        return QVariant(int(MScore::OrnamentStyle::BAROQUE));
                  return QVariant(int(MScore::OrnamentStyle::DEFAULT));
                  }

            case P_TYPE::DIRECTION:
                  return QVariant::fromValue<Direction>(toDirection(value));

            case P_TYPE::DIRECTION_H:
                  {
                  if (value == "left" || value == "1")
                        return QVariant(int(MScore::DirectionH::LEFT));
                  else if (value == "right" || value == "2")
                        return QVariant(int(MScore::DirectionH::RIGHT));
                  else if (value == "auto")
                        return QVariant(int(MScore::DirectionH::AUTO));
                  }
                  break;
            case P_TYPE::LAYOUT_BREAK: {
                  if (value == "line")
                        return QVariant(int(LayoutBreak::LINE));
                  if (value == "page")
                        return QVariant(int(LayoutBreak::PAGE));
                  if (value == "section")
                        return QVariant(int(LayoutBreak::SECTION));
                  if (value == "nobreak")
                        return QVariant(int(LayoutBreak::NOBREAK));
                  qDebug("getProperty: invalid P_TYPE::LAYOUT_BREAK: <%s>", qPrintable(value));
                  }
                  break;
            case P_TYPE::VALUE_TYPE: {
                  if (value == "offset")
                        return QVariant(int(Note::ValueType::OFFSET_VAL));
                  else if (value == "user")
                        return QVariant(int(Note::ValueType::USER_VAL));
                  }
                  break;
            case P_TYPE::PLACEMENT: {
                  if (value == "above")
                        return QVariant(int(Placement::ABOVE));
                  else if (value == "below")
                        return QVariant(int(Placement::BELOW));
                  }
                  break;
            case P_TYPE::HPLACEMENT: {
                  if (value == "left")
                        return QVariant(int(HPlacement::LEFT));
                  else if (value == "center")
                        return QVariant(int(HPlacement::CENTER));
                  else if (value == "right")
                        return QVariant(int(HPlacement::RIGHT));
                  }
                  break;
            case P_TYPE::TEXT_PLACE: {
                  if (value == "auto")
                        return QVariant(int(PlaceText::AUTO));
                  else if (value == "above")
                        return QVariant(int(PlaceText::ABOVE));
                  else if (value == "below")
                        return QVariant(int(PlaceText::BELOW));
                  else if (value == "left")
                        return QVariant(int(PlaceText::LEFT));
                  }
                  break;
            case P_TYPE::BARLINE_TYPE: {
                  bool ok;
                  const int ct = value.toInt(&ok);
                  if (ok)
                        return QVariant(ct);
                  else {
                        BarLineType t = BarLine::barLineType(value);
                        return QVariant::fromValue(t);
                        }
                  }
                  break;
            case P_TYPE::BEAM_MODE:             // TODO
                  return QVariant(int(0));

            case P_TYPE::GROUPS:
                  // unsupported
                  return QVariant();
            case P_TYPE::SYMID:
                  return QVariant::fromValue(Sym::name2id(value));
            case P_TYPE::HEAD_SCHEME:
                  return QVariant::fromValue(NoteHead::name2scheme(value));
            case P_TYPE::HEAD_GROUP:
                  return QVariant::fromValue(NoteHead::name2group(value));
            case P_TYPE::HEAD_TYPE:
                  return QVariant::fromValue(NoteHead::name2type(value));
            case P_TYPE::POINT_MM:              // not supported
            case P_TYPE::TDURATION:
            case P_TYPE::SIZE_MM:
            case P_TYPE::INT_LIST:
                  return QVariant();
            case P_TYPE::SUB_STYLE:
                  return int(textStyleFromName(value));
            case P_TYPE::ALIGN: {
                  QStringList sl = value.split(',');
                  if (sl.size() != 2) {
                        qDebug("bad align text <%s>", qPrintable(value));
                        return QVariant();
                        }
                  Align align = Align::LEFT;
                  if (sl[0] == "center")
                        align = align | Align::HCENTER;
                  else if (sl[0] == "right")
                        align = align | Align::RIGHT;
                  else if (sl[0] == "left")
                        ;
                  else {
                        qDebug("bad align text <%s>", qPrintable(sl[0]));
                        return QVariant();
                        }
                  if (sl[1] == "center")
                        align = align | Align::VCENTER;
                  else if (sl[1] == "bottom")
                        align = align | Align::BOTTOM;
                  else if (sl[1] == "baseline")
                        align = align | Align::BASELINE;
                  else if (sl[1] == "top")
                        ;
                  else {
                        qDebug("bad align text <%s>", qPrintable(sl[1]));
                        return QVariant();
                        }
                  return int(align);
                  }
            case P_TYPE::CHANGE_METHOD:
                  return QVariant(int(ChangeMap::nameToChangeMethod(value)));
            case P_TYPE::ORIENTATION:
                  if (value == "vertical")
                        return QVariant(int(Orientation::VERTICAL));
                  else if (value == "horizontal")
                        return QVariant(int(Orientation::HORIZONTAL));
                  break;
            default:
                  break;
            }
      return QVariant();
      }

//---------------------------------------------------------
//    readProperty
//---------------------------------------------------------

QVariant readProperty(Pid id, XmlReader& e)
      {
      switch (propertyType(id)) {
            case P_TYPE::BOOL:
                  return QVariant(bool(e.readInt()));
            case P_TYPE::ZERO_INT:
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
            case P_TYPE::POINT_SP:
            case P_TYPE::POINT_SP_MM:
                  return QVariant(e.readPoint());
            case P_TYPE::SCALE:
            case P_TYPE::SIZE:
                  return QVariant(e.readSize());
            case P_TYPE::FONT:
            case P_TYPE::STRING:
                  return QVariant(e.readElementText());
            case P_TYPE::GLISS_STYLE:
            case P_TYPE::ORNAMENT_STYLE:
            case P_TYPE::DIRECTION:
            case P_TYPE::DIRECTION_H:
            case P_TYPE::LAYOUT_BREAK:
            case P_TYPE::VALUE_TYPE:
            case P_TYPE::PLACEMENT:
            case P_TYPE::HPLACEMENT:
            case P_TYPE::TEXT_PLACE:
            case P_TYPE::BARLINE_TYPE:
            case P_TYPE::SYMID:
            case P_TYPE::HEAD_SCHEME:
            case P_TYPE::HEAD_GROUP:
            case P_TYPE::HEAD_TYPE:
            case P_TYPE::SUB_STYLE:
            case P_TYPE::ALIGN:
            case P_TYPE::ORIENTATION:
                  return propertyFromString(id, e.readElementText());

            case P_TYPE::BEAM_MODE:             // TODO
                  return QVariant(int(0));

            case P_TYPE::GROUPS:
                  {
                  Groups g;
                  g.read(e);
                  return QVariant::fromValue(g);
                  }
            case P_TYPE::POINT_MM:              // not supported
            case P_TYPE::TDURATION:
            case P_TYPE::SIZE_MM:
            case P_TYPE::INT_LIST:
                  return QVariant();
            default:
                  qFatal("unhandled PID type");
                  break;
            }
      return QVariant();
      }

//---------------------------------------------------------
//   propertyToString
//    Originally extracted from XmlWriter
//---------------------------------------------------------

QString propertyToString(Pid id, QVariant value, bool mscx)
      {
      if (value == QVariant())
            return QString();

      switch(id) {
            case Pid::SYSTEM_BRACKET: // system bracket type
                  return Bracket::bracketTypeName(BracketType(value.toInt()));
            case Pid::ACCIDENTAL_TYPE:
                  return Accidental::subtype2name(AccidentalType(value.toInt()));
            case Pid::OTTAVA_TYPE:
                  return Ottava::ottavaTypeName(OttavaType(value.toInt()));
            case Pid::TREMOLO_TYPE:
                  return Tremolo::type2name(TremoloType(value.toInt()));
            case Pid::TRILL_TYPE:
                  return Trill::type2name(Trill::Type(value.toInt()));
            case Pid::VIBRATO_TYPE:
                  return Vibrato::type2name(Vibrato::Type(value.toInt()));
            default:
                  break;
            }

      switch (propertyType(id)) {
            case P_TYPE::BOOL:
            case P_TYPE::INT:
            case P_TYPE::ZERO_INT:
                  return QString::number(value.toInt());
            case P_TYPE::REAL:
                  return QString::number(value.value<double>());
            case P_TYPE::SPATIUM:
                  return QString::number(value.value<Spatium>().val());
            case P_TYPE::DIRECTION:
                  return toString(value.value<Direction>());
            case P_TYPE::STRING:
            case P_TYPE::FRACTION:
                  return value.toString();
            case P_TYPE::KEYMODE:
                  switch (KeyMode(value.toInt())) {
                        case KeyMode::NONE:
                              return "none";
                        case KeyMode::MAJOR:
                              return "major";
                        case KeyMode::MINOR:
                              return "minor";
                        case KeyMode::DORIAN:
                              return "dorian";
                        case KeyMode::PHRYGIAN:
                              return "phrygian";
                        case KeyMode::LYDIAN:
                              return "lydian";
                        case KeyMode::MIXOLYDIAN:
                              return "mixolydian";
                        case KeyMode::AEOLIAN:
                              return "aeolian";
                        case KeyMode::IONIAN:
                              return "ionian";
                        case KeyMode::LOCRIAN:
                              return "locrian";
                        default:
                              return "unknown";
                        }
            case P_TYPE::ORNAMENT_STYLE:
                  switch (MScore::OrnamentStyle(value.toInt())) {
                        case MScore::OrnamentStyle::BAROQUE:
                              return "baroque";
                        case MScore::OrnamentStyle::DEFAULT:
                              return "default";
                        }
                  break;
            case P_TYPE::GLISS_STYLE:
                  switch (GlissandoStyle(value.toInt())) {
                        case GlissandoStyle::BLACK_KEYS:
                              return "blackkeys";
                        case GlissandoStyle::WHITE_KEYS:
                              return "whitekeys";
                        case GlissandoStyle::DIATONIC:
                              return "diatonic";
                        case GlissandoStyle::PORTAMENTO:
                              return "portamento";
                        case GlissandoStyle::CHROMATIC:
                              return "Chromatic";
                        }
                  break;
            case P_TYPE::DIRECTION_H:
                  switch (MScore::DirectionH(value.toInt())) {
                        case MScore::DirectionH::LEFT:
                              return "left";
                        case MScore::DirectionH::RIGHT:
                              return "right";
                        case MScore::DirectionH::AUTO:
                              return "auto";
                        }
                  break;
            case P_TYPE::LAYOUT_BREAK:
                  switch (LayoutBreak::Type(value.toInt())) {
                        case LayoutBreak::LINE:
                              return "line";
                        case LayoutBreak::PAGE:
                              return "page";
                        case LayoutBreak::SECTION:
                              return "section";
                        case LayoutBreak::NOBREAK:
                              return "nobreak";
                        }
                  break;
            case P_TYPE::VALUE_TYPE:
                  switch (Note::ValueType(value.toInt())) {
                        case Note::ValueType::OFFSET_VAL:
                              return "offset";
                        case Note::ValueType::USER_VAL:
                              return "user";
                        }
                  break;
            case P_TYPE::PLACEMENT:
                  switch (Placement(value.toInt())) {
                        case Placement::ABOVE:
                              return "above";
                        case Placement::BELOW:
                              return "below";
                        }
                  break;
            case P_TYPE::HPLACEMENT:
                  switch (HPlacement(value.toInt())) {
                        case HPlacement::LEFT:
                              return "left";
                        case HPlacement::CENTER:
                              return "center";
                        case HPlacement::RIGHT:
                              return "right";
                        }
                  break;
            case P_TYPE::TEXT_PLACE:
                  switch (PlaceText(value.toInt())) {
                        case PlaceText::AUTO:
                              return "auto";
                        case PlaceText::ABOVE:
                              return "above";
                        case PlaceText::BELOW:
                              return "below";
                        case PlaceText::LEFT:
                              return "left";
                        }
                  break;
            case P_TYPE::SYMID:
                  return Sym::id2name(SymId(value.toInt()));
            case P_TYPE::BARLINE_TYPE:
                  return BarLine::barLineTypeName(BarLineType(value.toInt()));
            case P_TYPE::HEAD_SCHEME:
                  return NoteHead::scheme2name(NoteHead::Scheme(value.toInt()));
            case P_TYPE::HEAD_GROUP:
                  return NoteHead::group2name(NoteHead::Group(value.toInt()));
            case P_TYPE::HEAD_TYPE:
                  return NoteHead::type2name(NoteHead::Type(value.toInt()));
            case P_TYPE::SUB_STYLE:
                  return textStyleName(Tid(value.toInt()));
            case P_TYPE::CHANGE_SPEED:
                  return Dynamic::speedToName(Dynamic::Speed(value.toInt()));
            case P_TYPE::CHANGE_METHOD:
                  return ChangeMap::changeMethodToName(ChangeMethod(value.toInt()));
            case P_TYPE::CLEF_TYPE:
                  return ClefInfo::tag(ClefType(value.toInt()));
            case P_TYPE::DYNAMIC_TYPE:
                  return Dynamic::dynamicTypeName(value.value<Dynamic::Type>());
            case P_TYPE::ALIGN: {
                  const Align a = Align(value.toInt());
                  const char* h;
                  if (a & Align::HCENTER)
                        h = "center";
                  else if (a & Align::RIGHT)
                        h = "right";
                  else
                        h = "left";
                  const char* v;
                  if (a & Align::BOTTOM)
                        v = "bottom";
                  else if (a & Align::VCENTER)
                        v = "center";
                  else if (a & Align::BASELINE)
                        v = "baseline";
                  else
                        v = "top";
                  return QString("%1,%2").arg(h, v);
                  }
            case P_TYPE::ORIENTATION: {
                  const Orientation o = Orientation(value.toInt());
                  if (o == Orientation::VERTICAL)
                        return "vertical";
                  else if (o == Orientation::HORIZONTAL)
                        return "horizontal";
                  break;
                  }
            case P_TYPE::POINT_MM:
                  qFatal("unknown: POINT_MM");
            case P_TYPE::SIZE_MM:
                  qFatal("unknown: SIZE_MM");
            case P_TYPE::TDURATION:
                  qFatal("unknown: TDURATION");
            case P_TYPE::BEAM_MODE:
                  qFatal("unknown: BEAM_MODE");
            case P_TYPE::TEMPO:
                  qFatal("unknown: TEMPO");
            case P_TYPE::GROUPS:
                  qFatal("unknown: GROUPS");
            case P_TYPE::INT_LIST:
                  qFatal("unknown: INT_LIST");

            default: {
                  switch(value.type()) {
                        case QVariant::Bool:
                        case QVariant::Char:
                        case QVariant::Int:
                        case QVariant::UInt:
                              return QString::number(value.toInt());
                        case QVariant::Double:
                              return QString::number(value.value<double>());
                        default:
                              break;
                        }
                  }
            }

      if (!mscx) {
            // String representation for properties that are written
            // to MSCX in other way (e.g. as XML tag properties).
            switch(value.type()) {
                  case QVariant::PointF: {
                        const QPointF p(value.value<QPointF>());
                        return QString("%1;%2").arg(QString::number(p.x()), QString::number(p.y()));
                        }
                  case QVariant::SizeF: {
                        const QSizeF s(value.value<QSizeF>());
                        return QString("%1x%2").arg(QString::number(s.width()), QString::number(s.height()));
                        }
                  // TODO: support QVariant::Rect and QVariant::QRectF?
                  default:
                        break;
                  }

            if (value.canConvert<QString>())
                  return value.toString();
            }

      return QString();
      }
}

