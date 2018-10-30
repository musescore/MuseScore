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
#include "barline.h"
#include "style.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   PropertyMetaData
//---------------------------------------------------------

struct PropertyMetaData {
      Pid id;
      const char* qml;        // qml name of property
      bool link;              // link this property for linked elements
      const char* name;       // xml name of property
      P_TYPE type;
      const char* userName;   // user-visible name of property
      };

//
// always: propertyList[subtype].id == subtype
//
//
static constexpr PropertyMetaData propertyList[] = {
      { Pid::SUBTYPE,                 "subtype",                 false, "subtype",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::SELECTED,                "selected",                false, "selected",              P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "selected")         },
      { Pid::GENERATED,               "generated",               false,  "generated",            P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "generated")        },
      { Pid::COLOR,                   "color",                   false, "color",                 P_TYPE::COLOR,               QT_TRANSLATE_NOOP("propertyName", "color")            },
      { Pid::VISIBLE,                 "visible",                 false, "visible",               P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "visible")          },
      { Pid::Z,                       "z",                       false, "z",                     P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "z")                },
      { Pid::SMALL,                   "small",                   false, "small",                 P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "small")            },
      { Pid::SHOW_COURTESY,           "show_courtesy",           false, "showCourtesy",          P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "show courtesy")    },
      { Pid::LINE_TYPE,               "line_type",               false, "lineType",              P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "line type")        },
      { Pid::PITCH,                   "pitch",                   true,  "pitch",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "pitch")            },

      { Pid::TPC1,                    "tpc1",                    true,  "tpc",                   P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "tonal pitch class") },
      { Pid::TPC2,                    "tpc2",                    true,  "tpc2",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "tonal pitch class") },
      { Pid::LINE,                    "line",                    false, "line",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "line")             },
      { Pid::FIXED,                   "fixed",                   false, "fixed",                 P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "fixed")            },
      { Pid::FIXED_LINE,              "fixed_line",              false, "fixedLine",             P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "fixed line")       },
      { Pid::HEAD_TYPE,               "head_type",               false, "headType",              P_TYPE::HEAD_TYPE,           QT_TRANSLATE_NOOP("propertyName", "head type")        },
      { Pid::HEAD_GROUP,              "head_group",              false, "head",                  P_TYPE::HEAD_GROUP,          QT_TRANSLATE_NOOP("propertyName", "head")             },
      { Pid::VELO_TYPE,               "velo_type",               false, "veloType",              P_TYPE::VALUE_TYPE,          QT_TRANSLATE_NOOP("propertyName", "velocity type")    },
      { Pid::VELO_OFFSET,             "velo_offset",             false, "velocity",              P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "velocity")         },
      { Pid::ARTICULATION_ANCHOR,     "articulation_anchor",     false, "anchor",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "anchor")           },

      { Pid::DIRECTION,               "direction",               false, "direction",             P_TYPE::DIRECTION,           QT_TRANSLATE_NOOP("propertyName", "direction")        },
      { Pid::STEM_DIRECTION,          "stem_direction",          false, "StemDirection",         P_TYPE::DIRECTION,           QT_TRANSLATE_NOOP("propertyName", "stem direction")   },
      { Pid::NO_STEM,                 "no_stem",                 false, "noStem",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "no stem")          },
      { Pid::SLUR_DIRECTION,          "slur_direction",          false, "up",                    P_TYPE::DIRECTION,           QT_TRANSLATE_NOOP("propertyName", "up")               },
      { Pid::LEADING_SPACE,           "leading_space",           false, "leadingSpace",          P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "leading space")    },
      { Pid::DISTRIBUTE,              "distribute",              false, "distribute",            P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "distributed")      },
      { Pid::MIRROR_HEAD,             "mirror_head",             false, "mirror",                P_TYPE::DIRECTION_H,         QT_TRANSLATE_NOOP("propertyName", "mirror")           },
      { Pid::DOT_POSITION,            "dot_position",            false, "dotPosition",           P_TYPE::DIRECTION,           QT_TRANSLATE_NOOP("propertyName", "dot position")     },
      { Pid::TUNING,                  "tuning",                  false, "tuning",                P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "tuning")           },
      { Pid::PAUSE,                   "pause",                   true,  "pause",                 P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "pause")            },

      { Pid::BARLINE_TYPE,            "barline_type",            false, "subtype",               P_TYPE::BARLINE_TYPE,        QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::BARLINE_SPAN,            "barline_span",            false, "span",                  P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "span")             },
      { Pid::BARLINE_SPAN_FROM,       "barline_span_from",       false, "spanFromOffset",        P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "span from")        },
      { Pid::BARLINE_SPAN_TO,         "barline_span_to",         false, "spanToOffset",          P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "span to")          },
      { Pid::OFFSET,                  "offset",                  false, "offset",                P_TYPE::POINT_SP_MM,         QT_TRANSLATE_NOOP("propertyName", "offset")           },
      { Pid::FRET,                    "fret",                    true,  "fret",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "fret")             },
      { Pid::STRING,                  "string",                  true,  "string",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "string")           },
      { Pid::GHOST,                   "ghost",                   true,  "ghost",                 P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "ghost")            },
      { Pid::PLAY,                    "play",                    false, "play",                  P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "played")           },
      { Pid::TIMESIG_NOMINAL,         "timesig_nominal",         false, 0,                       P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "nominal time signature") },

      { Pid::TIMESIG_ACTUAL,          "timesig_actual",          true,  0,                       P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "actual time signature") },
      { Pid::NUMBER_TYPE,             "number_type",             false, "numberType",            P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "number type")      },
      { Pid::BRACKET_TYPE,            "bracket_type",            false, "bracketType",           P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "bracket type")     },
      { Pid::NORMAL_NOTES,            "normal_notes",            false, "normalNotes",           P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "normal notes")     },
      { Pid::ACTUAL_NOTES,            "actual_notes",            false, "actualNotes",           P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "actual notes")     },
      { Pid::P1,                      "p1",                      false, "p1",                    P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "p1")               },
      { Pid::P2,                      "p2",                      false, "p2",                    P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "p2")               },
      { Pid::GROW_LEFT,               "grow_left",               false, "growLeft",              P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "grow left")        },
      { Pid::GROW_RIGHT,              "grow_right",              false, "growRight",             P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "grow right")       },
      { Pid::BOX_HEIGHT,              "box_height",              false, "height",                P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "height")           },

      { Pid::BOX_WIDTH,               "box_width",               false, "width",                 P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "width")            },
      { Pid::TOP_GAP,                 "top_gap",                 false, "topGap",                P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "top gap")          },
      { Pid::BOTTOM_GAP,              "bottom_gap",              false, "bottomGap",             P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "bottom gap")       },
      { Pid::LEFT_MARGIN,             "left_margin",             false, "leftMargin",            P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "left margin")      },
      { Pid::RIGHT_MARGIN,            "right_margin",            false, "rightMargin",           P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "right margin")     },
      { Pid::TOP_MARGIN,              "top_margin",              false, "topMargin",             P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "top margin")       },
      { Pid::BOTTOM_MARGIN,           "bottom_margin",           false, "bottomMargin",          P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "bottom margin")    },
      { Pid::LAYOUT_BREAK,            "layout_break",            false, "subtype",               P_TYPE::LAYOUT_BREAK,        QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::AUTOSCALE,               "autoscale",               false, "autoScale",             P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "autoscale")        },
      { Pid::SIZE,                    "size",                    false, "size",                  P_TYPE::SIZE,                QT_TRANSLATE_NOOP("propertyName", "size")             },

      { Pid::SCALE,                   "scale",                   false, "scale",                 P_TYPE::SCALE,               QT_TRANSLATE_NOOP("propertyName", "scale")            },
      { Pid::LOCK_ASPECT_RATIO,       "lock_aspect_ratio",       false, "lockAspectRatio",       P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "aspect ratio locked") },
      { Pid::SIZE_IS_SPATIUM,         "size_is_spatium",         false, "sizeIsSpatium",         P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "size is spatium")  },
      { Pid::TEXT,                    "text",                    true,  0,                       P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "text")             },
      { Pid::HTML_TEXT,               "html_text",               false, 0,                       P_TYPE::STRING,              ""                                                    },
      { Pid::USER_MODIFIED,           "user_modified",           false, 0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::BEAM_POS,                "beam_pos",                false, 0,                       P_TYPE::POINT,               QT_TRANSLATE_NOOP("propertyName", "beam position")    },
      { Pid::BEAM_MODE,               "beam_mode",               true, "BeamMode",               P_TYPE::BEAM_MODE,           QT_TRANSLATE_NOOP("propertyName", "beam mode")        },
      { Pid::BEAM_NO_SLOPE,           "beam_no_slope",           true, "noSlope",                P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "without slope")    },
      { Pid::USER_LEN,                "user_len",                false, "userLen",               P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "length")           },

      { Pid::SPACE,                   "space",                   false, "space",                 P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "space")            },
      { Pid::TEMPO,                   "tempo",                   true,  "tempo",                 P_TYPE::TEMPO,               QT_TRANSLATE_NOOP("propertyName", "tempo")            },
      { Pid::TEMPO_FOLLOW_TEXT,       "tempo_follow_text",       true,  "followText",            P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "following text")   },
      { Pid::ACCIDENTAL_BRACKET,      "accidental_bracket",      false, "bracket",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "bracket")          },
      { Pid::NUMERATOR_STRING,        "numerator_string",        false, "textN",                 P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "numerator string") },
      { Pid::DENOMINATOR_STRING,      "denominator_string",      false, "textD",                 P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "denominator string") },
      { Pid::FBPREFIX,                "fbprefix",                false, "prefix",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "prefix")           },
      { Pid::FBDIGIT,                 "fbdigit",                 false, "digit",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "digit")            },
      { Pid::FBSUFFIX,                "fbsuffix",                false, "suffix",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "suffix")           },
      { Pid::FBCONTINUATIONLINE,      "fbcontinuationline",      false, "continuationLine",      P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "continuation line") },

      { Pid::FBPARENTHESIS1,          "fbparenthesis1",          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS2,          "fbparenthesis2",          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS3,          "fbparenthesis3",          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS4,          "fbparenthesis4",          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::FBPARENTHESIS5,          "fbparenthesis5",          false, "",                      P_TYPE::INT,                 ""                                                    },
      { Pid::OTTAVA_TYPE,             "ottava_type",             true,  "",                      P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "ottava type")      },
      { Pid::NUMBERS_ONLY,            "numbers_only",            false, "numbersOnly",           P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "numbers only")     },
      { Pid::TRILL_TYPE,              "trill_type",              false, "",                      P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "trill type")       },
      { Pid::VIBRATO_TYPE,            "vibrato_type",            false, "",                      P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "vibrato type")     },
      { Pid::HAIRPIN_CIRCLEDTIP,      "hairpin_circledtip",      false, "hairpinCircledTip",     P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "hairpin with circled tip") },

      { Pid::HAIRPIN_TYPE,            "hairpin_type",            true,  "",                      P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "hairpin type")     },
      { Pid::HAIRPIN_HEIGHT,          "hairpin_height",          false, "hairpinHeight",         P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "hairpin height")   },
      { Pid::HAIRPIN_CONT_HEIGHT,     "hairpin_cont_height",     false, "hairpinContHeight",     P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "hairpin cont height") },
      { Pid::VELO_CHANGE,             "velo_change",             true,  "veloChange",            P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "velocity change")  },
      { Pid::DYNAMIC_RANGE,           "dynamic_range",           true,  "dynType",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "dynamic range")    },
      { Pid::PLACEMENT,               "placement",               false, "placement",             P_TYPE::PLACEMENT,           QT_TRANSLATE_NOOP("propertyName", "placement")        },
      { Pid::VELOCITY,                "velocity",                false, "velocity",              P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "velocity")         },
      { Pid::JUMP_TO,                 "jump_to",                 true,  "jumpTo",                P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "jump to")          },
      { Pid::PLAY_UNTIL,              "play_until",              true,  "playUntil",             P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "play until")       },
      { Pid::CONTINUE_AT,             "continue_at",             true,  "continueAt",            P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "continue at")       },
//100
      { Pid::LABEL,                   "label",                   true,  "label",                 P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "label")            },
      { Pid::MARKER_TYPE,             "marker_type",             true,  0,                       P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "marker type")      },
      { Pid::ARP_USER_LEN1,           "arp_user_len1",           false, 0,                       P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "length 1")         },
      { Pid::ARP_USER_LEN2,           "arp_user_len2",           false, 0,                       P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "length 2")         },
      { Pid::REPEAT_END,              "repeat_end",              true,  0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::REPEAT_START,            "repeat_start",            true,  0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::REPEAT_JUMP,             "repeat_jump",             true,  0,                       P_TYPE::BOOL,                ""                                                    },
      { Pid::MEASURE_NUMBER_MODE,     "measure_number_mode",     false, "measureNumberMode",     P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "measure number mode") },
      { Pid::GLISS_TYPE,              "gliss_type",              false, "subtype",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::GLISS_TEXT,              "gliss_text",              false, 0,                       P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "text")             },

      { Pid::GLISS_SHOW_TEXT,         "gliss_show_text",         false, 0,                       P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "showing text")     },
      { Pid::DIAGONAL,                "diagonal",                false, 0,                       P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "diagonal")         },
      { Pid::GROUPS,                  "groups",                  false, 0,                       P_TYPE::GROUPS,              QT_TRANSLATE_NOOP("propertyName", "groups")           },
      { Pid::LINE_STYLE,              "line_style",              false, "lineStyle",             P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "line style")       },
      { Pid::LINE_COLOR,              "line_color",              false, 0,                       P_TYPE::COLOR,               QT_TRANSLATE_NOOP("propertyName", "line color")       },
      { Pid::LINE_WIDTH,              "line_width",              false, "lineWidth",             P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "line width")       },
      { Pid::LASSO_POS,               "lasso_pos",               false, 0,                       P_TYPE::POINT_MM,            QT_TRANSLATE_NOOP("propertyName", "lasso position")   },
      { Pid::LASSO_SIZE,              "lasso_size",              false, 0,                       P_TYPE::SIZE_MM,             QT_TRANSLATE_NOOP("propertyName", "lasso size")       },
      { Pid::TIME_STRETCH,            "time_stretch",            true,  "timeStretch",           P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "time stretch")     },
      { Pid::ORNAMENT_STYLE,          "ornament_style",          true,  "ornamentStyle",         P_TYPE::ORNAMENT_STYLE,      QT_TRANSLATE_NOOP("propertyName", "ornament style")   },

      { Pid::TIMESIG,                 "timesig",                 false, 0,                       P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "time signature")   },
      { Pid::TIMESIG_GLOBAL,          "timesig_global",          false, 0,                       P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "global time signature") },
      { Pid::TIMESIG_STRETCH,         "timesig_stretch",         false, 0,                       P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "time signature stretch") },
      { Pid::TIMESIG_TYPE,            "timesig_type",            true,  "subtype",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "subtype")          },
      { Pid::SPANNER_TICK,            "spanner_tick",            true,  "tick",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "tick")             },
      { Pid::SPANNER_TICKS,           "spanner_ticks",           true,  "ticks",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "ticks")            },
      { Pid::SPANNER_TRACK2,          "spanner_track2",          true,  "track2",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "track2")           },
      { Pid::OFFSET2,                 "user_off2",               false, "userOff2",              P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "offset2")         },
      { Pid::BREAK_MMR,               "break_mmr",               false, "breakMultiMeasureRest", P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "breaking multimeasure rest")},
      { Pid::REPEAT_COUNT,            "repeat_count",            true,  "endRepeat",             P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "end repeat")       },

      { Pid::USER_STRETCH,            "user_stretch",            false, "stretch",               P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "stretch")          },
      { Pid::NO_OFFSET,               "no_offset",               false, "noOffset",              P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "numbering offset") },
      { Pid::IRREGULAR,               "irregular",               true,  "irregular",             P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "irregular")        },
      { Pid::ANCHOR,                  "anchor",                  false,  "anchor",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "anchor")           },
      { Pid::SLUR_UOFF1,              "slur_uoff1",              false,  "o1",                   P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "o1")               },
      { Pid::SLUR_UOFF2,              "slur_uoff2",              false,  "o2",                   P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "o2")               },
      { Pid::SLUR_UOFF3,              "slur_uoff3",              false,  "o3",                   P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "o3")               },
      { Pid::SLUR_UOFF4,              "slur_uoff4",              false,  "o4",                   P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "o4")               },
      { Pid::STAFF_MOVE,              "staff_move",              true,  "staffMove",             P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "staff move")       },
      { Pid::VERSE,                   "verse",                   true,  "no",                    P_TYPE::ZERO_INT,            QT_TRANSLATE_NOOP("propertyName", "verse")            },

      { Pid::SYLLABIC,                "syllabic",                true,  "syllabic",              P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "syllabic")         },
      { Pid::LYRIC_TICKS,             "lyric_ticks",             true,  "ticks",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "ticks")            },
      { Pid::VOLTA_ENDING,            "volta_ending",            true,  "endings",               P_TYPE::INT_LIST,            QT_TRANSLATE_NOOP("propertyName", "endings")          },
      { Pid::LINE_VISIBLE,            "line_visible",            true,  "lineVisible",           P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "visible line")     },
      { Pid::MAG,                     "mag",                     false, "mag",                   P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "mag")              },
      { Pid::USE_DRUMSET,             "use_drumset",             false, "useDrumset",            P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "using drumset")    },
      { Pid::PART_VOLUME,             "part_volume",             false, "volume",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "volume")           },
      { Pid::PART_MUTE,               "part_mute",               false, "mute",                  P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "mute")             },
      { Pid::PART_PAN,                "part_pan",                false, "pan",                   P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "pan")              },
      { Pid::PART_REVERB,             "part_reverb",             false, "reverb",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "reverb")           },

      { Pid::PART_CHORUS,             "part_chorus",             false, "chorus",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "chorus")           },
      { Pid::DURATION,                "duration",                false, 0,                       P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "duration")         },
      { Pid::DURATION_TYPE,           "duration_type",           false, 0,                       P_TYPE::TDURATION,           QT_TRANSLATE_NOOP("propertyName", "duration type")    },
      { Pid::ROLE,                    "role",                    false, "role",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "role")             },
      { Pid::TRACK,                   "track",                   false, 0,                       P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "track")            },
      { Pid::GLISSANDO_STYLE,         "glissando_style",         true,  "glissandoStyle",        P_TYPE::GLISSANDO_STYLE,     QT_TRANSLATE_NOOP("propertyName", "glissando style")  },
      { Pid::FRET_STRINGS,            "fret_strings",            false, "strings",               P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "strings")          },
      { Pid::FRET_FRETS,              "fret_frets",              false, "frets",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "frets")            },
      { Pid::FRET_BARRE,              "fret_barre",              false, "barre",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "barre")            },
      { Pid::FRET_OFFSET,             "fret_offset",             false, "fretOffset",            P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "fret offset")      },

      { Pid::FRET_NUM_POS,            "fret_num_pos",            false, "fretNumPos",            P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "fret number position") },
      { Pid::SYSTEM_BRACKET,          "system_bracket",          false, "type",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "type")             },
      { Pid::GAP,                     "gap",                     false, 0,                       P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "gap")              },
      { Pid::AUTOPLACE,               "autoplace",               false, "autoplace",             P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "autoplace")        },
      { Pid::DASH_LINE_LEN,           "dash_line_len",           false, "dashLineLength",        P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "dash line length") },
      { Pid::DASH_GAP_LEN,            "dash_gap_len",            false, "dashGapLength",         P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "dash gap length")  },
      { Pid::TICK,                    "tick",                    false, 0,                       P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "tick")             },
      { Pid::PLAYBACK_VOICE1,         "playback_voice1",         false, "playbackVoice1",        P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "playback voice 1") },
      { Pid::PLAYBACK_VOICE2,         "playback_voice2",         false, "playbackVoice2",        P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "playback voice 2") },
      { Pid::PLAYBACK_VOICE3,         "playback_voice3",         false, "playbackVoice3",        P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "playback voice 3") },

      { Pid::PLAYBACK_VOICE4,         "playback_voice4",         false, "playbackVoice4",        P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "playback voice 4") },
      { Pid::SYMBOL,                  "symbol",                  true,  "symbol",                P_TYPE::SYMID,               QT_TRANSLATE_NOOP("propertyName", "symbol")           },
      { Pid::PLAY_REPEATS,            "play_repeats",            true,  "playRepeats",           P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "playing repeats")  },
      { Pid::CREATE_SYSTEM_HEADER,    "create_system_header",    false, "createSystemHeader",    P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "creating system header") },
      { Pid::STAFF_LINES,             "staff_lines",             true,  "lines",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "lines")            },
      { Pid::LINE_DISTANCE,           "line_distance",           true,  "lineDistance",          P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "line distance")    },
      { Pid::STEP_OFFSET,             "step_offset",             true,  "stepOffset",            P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "step offset")      },
      { Pid::STAFF_SHOW_BARLINES,     "staff_show_barlines",     false, "",                      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "showing barlines") },
      { Pid::STAFF_SHOW_LEDGERLINES,  "staff_show_ledgerlines",  false, "",                      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "showing ledgerlines") },
      { Pid::STAFF_SLASH_STYLE,       "staff_slash_style",       false, "",                      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "slash style")      },

      { Pid::STAFF_NOTEHEAD_SCHEME,   "staff_notehead_scheme",   false, "",                      P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "notehead scheme")  },
      { Pid::STAFF_GEN_CLEF,          "staff_gen_clef",          false, "",                      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "generating clefs") },
      { Pid::STAFF_GEN_TIMESIG,       "staff_gen_timesig",       false, "",                      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "generating time signature") },
      { Pid::STAFF_GEN_KEYSIG,        "staff_gen_keysig",        false, "",                      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "generating key signature")  },
      { Pid::STAFF_YOFFSET,           "staff_yoffset",           false, "",                      P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "y-offset")                 },
      { Pid::STAFF_USERDIST,          "staff_userdist",          false, "distOffset",            P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "distance offset")  },
      { Pid::STAFF_BARLINE_SPAN,      "staff_barline_span",      false, "barLineSpan",           P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "barline span")     },
      { Pid::STAFF_BARLINE_SPAN_FROM, "staff_barline_span_from", false, "barLineSpanFrom",       P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "barline span from") },
      { Pid::STAFF_BARLINE_SPAN_TO,   "staff_barline_span_to",   false, "barLineSpanTo",         P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "barline span to")  },
      { Pid::BRACKET_SPAN,            "bracket_span",            false, "bracketSpan",           P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "bracket span")     },

      { Pid::BRACKET_COLUMN,          "bracket_column",          false, "level",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "level")            },
      { Pid::INAME_LAYOUT_POSITION,   "iname_layout_position",   false, "layoutPosition",        P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "layout position")  },
      { Pid::SUB_STYLE,               "sub_style",               false, "style",                 P_TYPE::SUB_STYLE,           QT_TRANSLATE_NOOP("propertyName", "style")            },
      { Pid::FONT_FACE,               "font_face",               false, "family",                P_TYPE::FONT,                QT_TRANSLATE_NOOP("propertyName", "family")           },
      { Pid::FONT_SIZE,               "font_size",               false, "size",                  P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "size")             },
      { Pid::FONT_BOLD,               "font_bold",               false, "bold",                  P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "bold")             },
      { Pid::FONT_ITALIC,             "font_italic",             false, "italic",                P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "italic")           },
      { Pid::FONT_UNDERLINE,          "font_underline",          false, "underline",             P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "underlined")       },
      { Pid::FRAME_TYPE,              "frame_type",              false, "frameType",             P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "frame type")       },
      { Pid::FRAME_WIDTH,             "frame_width",             false, "frameWidth",            P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "frame width")      },
//200
      { Pid::FRAME_PADDING,           "frame_padding",           false, "framePadding",          P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "frame padding")    },
      { Pid::FRAME_ROUND,             "frame_round",             false, "frameRound",            P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "frame round")      },
      { Pid::FRAME_FG_COLOR,          "frame_fg_color",          false, "frameFgColor",          P_TYPE::COLOR,               QT_TRANSLATE_NOOP("propertyName", "frame foreground color") },
      { Pid::FRAME_BG_COLOR,          "frame_bg_color",          false, "frameBgColor",          P_TYPE::COLOR,               QT_TRANSLATE_NOOP("propertyName", "frame background color") },
      { Pid::SIZE_SPATIUM_DEPENDENT,  "size_spatium_dependent",  false, "sizeIsSpatiumDependent",P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "spatium dependent font") },
      { Pid::ALIGN,                   "align",                   false, "align",                 P_TYPE::ALIGN,               QT_TRANSLATE_NOOP("propertyName", "align")            },
      { Pid::SYSTEM_FLAG,             "system_flag",             false, "systemFlag",            P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "system flag")      },
      { Pid::BEGIN_TEXT,              "begin_text",              false, "beginText",             P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "begin text")       },

      { Pid::BEGIN_TEXT_ALIGN,        "begin_text_align",        false, "beginTextAlign",        P_TYPE::ALIGN,               QT_TRANSLATE_NOOP("propertyName", "begin text align") },
      { Pid::BEGIN_TEXT_PLACE,        "begin_text_place",        false, "beginTextPlace",        P_TYPE::TEXT_PLACE,          QT_TRANSLATE_NOOP("propertyName", "begin text place") },
      { Pid::BEGIN_HOOK_TYPE,         "begin_hook_type",         false, "beginHookType",         P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "begin hook type")  },
      { Pid::BEGIN_HOOK_HEIGHT,       "begin_hook_height",       false, "beginHookHeight",       P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "begin hook height") },
      { Pid::BEGIN_FONT_FACE,         "begin_font_face",         false, "beginFontFace",         P_TYPE::FONT,                QT_TRANSLATE_NOOP("propertyName", "begin font face")  },
      { Pid::BEGIN_FONT_SIZE,         "begin_font_size",         false, "beginFontSize",         P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "begin font size")  },
      { Pid::BEGIN_FONT_BOLD,         "begin_font_bold",         false, "beginFontBold",         P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "begin font bold")  },
      { Pid::BEGIN_FONT_ITALIC,       "begin_font_italic",       false, "beginFontItalic",       P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "begin font italic") },
      { Pid::BEGIN_FONT_UNDERLINE,    "begin_font_underline",    false, "beginFontUnderline",    P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "begin font underlined") },
      { Pid::BEGIN_TEXT_OFFSET,       "begin_text_offset",       false, "beginTextOffset",       P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "begin text offset")  },

      { Pid::CONTINUE_TEXT,           "continue_text",           false, "continueText",          P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "continue text")    },
      { Pid::CONTINUE_TEXT_ALIGN,     "continue_text_align",     false, "continueTextAlign",     P_TYPE::ALIGN,               QT_TRANSLATE_NOOP("propertyName", "continue text align") },
      { Pid::CONTINUE_TEXT_PLACE,     "continue_text_place",     false, "continueTextPlace",     P_TYPE::TEXT_PLACE,          QT_TRANSLATE_NOOP("propertyName", "continue text place") },
      { Pid::CONTINUE_FONT_FACE,      "continue_font_face",      false, "continueFontFace",      P_TYPE::FONT,                QT_TRANSLATE_NOOP("propertyName", "continue font face") },
      { Pid::CONTINUE_FONT_SIZE,      "continue_font_size",      false, "continueFontSize",      P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "continue font size") },
      { Pid::CONTINUE_FONT_BOLD,      "continue_font_bold",      false, "continueFontBold",      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "continue font bold") },
      { Pid::CONTINUE_FONT_ITALIC,    "continue_font_italic",    false, "continueFontItalic",    P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "continue font italic") },
      { Pid::CONTINUE_FONT_UNDERLINE, "continue_font_underline", false, "continueFontUnderline", P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "continue font underlined") },
      { Pid::CONTINUE_TEXT_OFFSET,    "continue_text_offset",    false, "continueTextOffset",    P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "continue text offset") },
      { Pid::END_TEXT,                "end_text",                false, "endText",               P_TYPE::STRING,              QT_TRANSLATE_NOOP("propertyName", "end text")         },

      { Pid::END_TEXT_ALIGN,          "end_text_align",          false, "endTextAlign",          P_TYPE::ALIGN,               QT_TRANSLATE_NOOP("propertyName", "end text align")   },
      { Pid::END_TEXT_PLACE,          "end_text_place",          false, "endTextPlace",          P_TYPE::TEXT_PLACE,          QT_TRANSLATE_NOOP("propertyName", "end text place")   },
      { Pid::END_HOOK_TYPE,           "end_hook_type",           false, "endHookType",           P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "end hook type")    },
      { Pid::END_HOOK_HEIGHT,         "end_hook_height",         false, "endHookHeight",         P_TYPE::SPATIUM,             QT_TRANSLATE_NOOP("propertyName", "end hook height")  },
      { Pid::END_FONT_FACE,           "end_font_face",           false, "endFontFace",           P_TYPE::FONT,                QT_TRANSLATE_NOOP("propertyName", "end font face")    },
      { Pid::END_FONT_SIZE,           "end_font_size",           false, "endFontSize",           P_TYPE::REAL,                QT_TRANSLATE_NOOP("propertyName", "end font size")    },
      { Pid::END_FONT_BOLD,           "end_font_bold",           false, "endFontBold",           P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "end font bold")    },
      { Pid::END_FONT_ITALIC,         "end_font_italic",         false, "endFontItalic",         P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "end font italic")  },
      { Pid::END_FONT_UNDERLINE,      "end_font_underline",      false, "endFontUnderline",      P_TYPE::BOOL,                QT_TRANSLATE_NOOP("propertyName", "end font underlined") },
      { Pid::END_TEXT_OFFSET,         "end_text_offset",         false, "endTextOffset",         P_TYPE::POINT_SP,            QT_TRANSLATE_NOOP("propertyName", "end text offset")  },

      { Pid::POS_ABOVE,               "pos_above",               false, "posAbove",              P_TYPE::SP_REAL,             QT_TRANSLATE_NOOP("propertyName", "position above")   },

      { Pid::LOCATION_STAVES,         0,                         false, "staves",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "staves distance")  },
      { Pid::LOCATION_VOICES,         0,                         false, "voices",                P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "voices distance")  },
      { Pid::LOCATION_MEASURES,       0,                         false, "measures",              P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "measures distance") },
      { Pid::LOCATION_FRACTIONS,      0,                         false, "fractions",             P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "position distance") },
      { Pid::LOCATION_GRACE,          0,                         false, "grace",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "grace note index") },
      { Pid::LOCATION_NOTE,           0,                         false, "note",                  P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "note index")       },

      { Pid::VOICE,                   0,                         false, "voice",                 P_TYPE::INT,                 QT_TRANSLATE_NOOP("propertyName", "voice")            },
      { Pid::POSITION,                0,                         false, "position",              P_TYPE::FRACTION,            QT_TRANSLATE_NOOP("propertyName", "position")         },

      { Pid::END, "++end++", false, "++end++", P_TYPE::INT, QT_TRANSLATE_NOOP("propertyName", "<invalid property>") }
      };

//---------------------------------------------------------
//   propertyIdQml
//---------------------------------------------------------

Pid propertyIdQml(const QStringRef& s)
      {
      for (const PropertyMetaData& pd : propertyList) {
            if (pd.qml == s)
                  return pd.id;
            }
      return Pid::END;
      }

//---------------------------------------------------------
//   propertyIdQml
//---------------------------------------------------------

Pid propertyIdQml(const QString& s)
      {
      return propertyIdQml(QStringRef(&s));
      }

//---------------------------------------------------------
//   propertyIdName
//---------------------------------------------------------

Pid propertyIdName(const QStringRef& s)
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

Pid propertyIdName(const QString& s)
      {
      return propertyIdName(QStringRef(&s));
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

const char* propertyQmlName(Pid id)
      {
      Q_ASSERT( propertyList[int(id)].id == id);
      return propertyList[int(id)].qml;
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
            case P_TYPE::GLISSANDO_STYLE: {
                  QString value(e.readElementText());
                  if ( value == "whitekeys")
                        return QVariant(int(GlissandoStyle::WHITE_KEYS));
                  else if ( value == "blackkeys")
                        return QVariant(int(GlissandoStyle::BLACK_KEYS));
                  else if ( value == "diatonic")
                        return QVariant(int(GlissandoStyle::DIATONIC));
                  else // e.g., normally "Chromatic"
                        return QVariant(int(GlissandoStyle::CHROMATIC));
                  }
                  break;
            case P_TYPE::ORNAMENT_STYLE: {
                  QString value(e.readElementText());
                  if ( value == "baroque")
                        return QVariant(int(MScore::OrnamentStyle::BAROQUE));
                  return QVariant(int(MScore::OrnamentStyle::DEFAULT));
                  }

            case P_TYPE::DIRECTION:
                  return QVariant::fromValue<Direction>(toDirection(e.readElementText()));

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
                        return QVariant(int(Placement::ABOVE));
                  else if (value == "below")
                        return QVariant(int(Placement::BELOW));
                  }
                  break;
            case P_TYPE::TEXT_PLACE: {
                  QString value(e.readElementText());
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
                  const QString& val(e.readElementText());
                  int ct = val.toInt(&ok);
                  if (ok)
                        return QVariant(ct);
                  else {
                        BarLineType t = BarLine::barLineType(val);
                        return QVariant::fromValue(t);
                        }
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
            case P_TYPE::SYMID:
                  return QVariant::fromValue(Sym::name2id(e.readElementText()));
                  break;
            case P_TYPE::HEAD_GROUP: {
                  QString s = e.readElementText();
                  return QVariant::fromValue(NoteHead::name2group(s));
                  }
            case P_TYPE::HEAD_TYPE:
                  return QVariant::fromValue(NoteHead::name2type(e.readElementText()));
            case P_TYPE::POINT_MM:              // not supported
            case P_TYPE::TDURATION:
            case P_TYPE::SIZE_MM:
            case P_TYPE::INT_LIST:
                  return QVariant();
            case P_TYPE::SUB_STYLE:
                  return int(textStyleFromName(e.readElementText()));
            case P_TYPE::ALIGN: {
                  QString s = e.readElementText();
                  QStringList sl = s.split(',');
                  if (sl.size() != 2) {
                        qDebug("bad align text <%s>", qPrintable(s));
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
                  return  int(align);
                  }
            default:
                  qFatal("unhandled PID type");
                  break;
            }
      return QVariant();
      }

}

