//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of classes Note and ShadowNote.
*/

#include <assert.h>

#include "note.h"
#include "score.h"
#include "chord.h"
#include "sym.h"
#include "xml.h"
#include "slur.h"
#include "tie.h"
#include "text.h"
#include "clef.h"
#include "staff.h"
#include "pitchspelling.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "utils.h"
#include "image.h"
#include "system.h"
#include "tuplet.h"
#include "articulation.h"
#include "drumset.h"
#include "segment.h"
#include "measure.h"
#include "undo.h"
#include "part.h"
#include "stafftype.h"
#include "stringdata.h"
#include "fret.h"
#include "harmony.h"
#include "fingering.h"
#include "bend.h"
#include "accidental.h"
#include "page.h"
#include "icon.h"
#include "notedot.h"
#include "spanner.h"
#include "glissando.h"
#include "bagpembell.h"
#include "hairpin.h"
#include "textline.h"

namespace Ms {

//---------------------------------------------------------
//   noteHeads
//    notehead groups
//---------------------------------------------------------

static const SymId noteHeads[2][int(NoteHead::Group::HEAD_GROUPS)][int(NoteHead::Type::HEAD_TYPES)] = {
   {     // down stem
      { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,               SymId::noteheadDoubleWhole  },
      { SymId::noteheadXWhole,              SymId::noteheadXHalf,               SymId::noteheadXBlack,              SymId::noteheadXDoubleWhole  },
      { SymId::noteheadPlusWhole,           SymId::noteheadPlusHalf,            SymId::noteheadPlusBlack,           SymId::noteheadPlusDoubleWhole  },
      { SymId::noteheadCircleXWhole,        SymId::noteheadCircleXHalf,         SymId::noteheadCircleX,             SymId::noteheadCircleXDoubleWhole},
      { SymId::noteheadWholeWithX,          SymId::noteheadHalfWithX,           SymId::noteheadVoidWithX,           SymId::noteheadDoubleWholeWithX},
      { SymId::noteheadTriangleUpWhole,     SymId::noteheadTriangleUpHalf,      SymId::noteheadTriangleUpBlack,     SymId::noteheadTriangleUpDoubleWhole },
      { SymId::noteheadTriangleDownWhole,   SymId::noteheadTriangleDownHalf,    SymId::noteheadTriangleDownBlack,   SymId::noteheadTriangleDownDoubleWhole },
      { SymId::noteheadSlashedWhole1,       SymId::noteheadSlashedHalf1,        SymId::noteheadSlashedBlack1,       SymId::noteheadSlashedDoubleWhole1 },
      { SymId::noteheadSlashedWhole2,       SymId::noteheadSlashedHalf2,        SymId::noteheadSlashedBlack2,       SymId::noteheadSlashedDoubleWhole2 },
      { SymId::noteheadDiamondWhole,        SymId::noteheadDiamondHalf,         SymId::noteheadDiamondBlack,        SymId::noteheadDiamondDoubleWhole  },
      { SymId::noteheadDiamondWholeOld,     SymId::noteheadDiamondHalfOld,      SymId::noteheadDiamondBlackOld,     SymId::noteheadDiamondDoubleWholeOld  },
      { SymId::noteheadCircledWhole,        SymId::noteheadCircledHalf,         SymId::noteheadCircledBlack,        SymId::noteheadCircledDoubleWhole  },
      { SymId::noteheadCircledWholeLarge,   SymId::noteheadCircledHalfLarge,    SymId::noteheadCircledBlackLarge,   SymId::noteheadCircledDoubleWholeLarge  },
      { SymId::noteheadLargeArrowUpWhole,   SymId::noteheadLargeArrowUpHalf,    SymId::noteheadLargeArrowUpBlack,   SymId::noteheadLargeArrowUpDoubleWhole  },
      { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,               SymId::noteheadDoubleWholeSquare   },

      { SymId::noteheadSlashWhiteWhole,     SymId::noteheadSlashWhiteHalf,      SymId::noteheadSlashHorizontalEnds, SymId::noteheadSlashWhiteWhole},

      { SymId::noteShapeRoundWhite,         SymId::noteShapeRoundWhite,         SymId::noteShapeRoundBlack,         SymId::noteShapeRoundDoubleWhole            },
      { SymId::noteShapeSquareWhite,        SymId::noteShapeSquareWhite,        SymId::noteShapeSquareBlack,        SymId::noteShapeSquareDoubleWhole           },
      { SymId::noteShapeTriangleRightWhite, SymId::noteShapeTriangleRightWhite, SymId::noteShapeTriangleRightBlack, SymId::noteShapeTriangleRightDoubleWhole    },
      { SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondBlack,       SymId::noteShapeDiamondDoubleWhole          },
      { SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpBlack,    SymId::noteShapeTriangleUpDoubleWhole       },
      { SymId::noteShapeMoonWhite,          SymId::noteShapeMoonWhite,          SymId::noteShapeMoonBlack,          SymId::noteShapeMoonDoubleWhole            },
      { SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundBlack, SymId::noteShapeTriangleRoundDoubleWhole    },

      { SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneBlack,          SymId::noteShapeKeystoneDoubleWhole    },
      { SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonBlack,       SymId::noteShapeQuarterMoonDoubleWhole },
      { SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleBlack, SymId::noteShapeIsoscelesTriangleDoubleWhole   },
      { SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftBlack,          SymId::noteShapeMoonLeftDoubleWhole    },
      { SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftBlack,     SymId::noteShapeArrowheadLeftDoubleWhole       },
      { SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftBlack, SymId::noteShapeTriangleRoundLeftDoubleWhole   },

      { SymId::noteDoWhole,  SymId::noteDoHalf,  SymId::noteDoBlack,  SymId::noSym            },
      { SymId::noteReWhole,  SymId::noteReHalf,  SymId::noteReBlack,  SymId::noSym            },
      { SymId::noteMiWhole,  SymId::noteMiHalf,  SymId::noteMiBlack,  SymId::noSym            },
      { SymId::noteFaWhole,  SymId::noteFaHalf,  SymId::noteFaBlack,  SymId::noSym            },
      { SymId::noteSoWhole,  SymId::noteSoHalf,  SymId::noteSoBlack,  SymId::noSym            },
      { SymId::noteLaWhole,  SymId::noteLaHalf,  SymId::noteLaBlack,  SymId::noSym            },
      { SymId::noteTiWhole,  SymId::noteTiHalf,  SymId::noteTiBlack,  SymId::noSym            },
      { SymId::noteSiWhole,  SymId::noteSiHalf,  SymId::noteSiBlack,  SymId::noSym            },

      { SymId::noteASharpWhole,  SymId::noteASharpHalf,  SymId::noteASharpBlack,  SymId::noSym            },
      { SymId::noteAWhole,       SymId::noteAHalf,       SymId::noteABlack,       SymId::noSym            },
      { SymId::noteAFlatWhole,   SymId::noteAFlatHalf,   SymId::noteAFlatBlack,   SymId::noSym            },
      { SymId::noteBSharpWhole,  SymId::noteBSharpHalf,  SymId::noteBSharpBlack,  SymId::noSym            },
      { SymId::noteBWhole,       SymId::noteBHalf,       SymId::noteBBlack,       SymId::noSym            },
      { SymId::noteBFlatWhole,   SymId::noteBFlatHalf,   SymId::noteBFlatBlack,   SymId::noSym            },
      { SymId::noteCSharpWhole,  SymId::noteCSharpHalf,  SymId::noteCSharpBlack,  SymId::noSym            },
      { SymId::noteCWhole,       SymId::noteCHalf,       SymId::noteCBlack,       SymId::noSym            },
      { SymId::noteCFlatWhole,   SymId::noteCFlatHalf,   SymId::noteCFlatBlack,   SymId::noSym            },
      { SymId::noteDSharpWhole,  SymId::noteDSharpHalf,  SymId::noteDSharpBlack,  SymId::noSym            },
      { SymId::noteDWhole,       SymId::noteDHalf,       SymId::noteDBlack,       SymId::noSym            },
      { SymId::noteDFlatWhole,   SymId::noteDFlatHalf,   SymId::noteDFlatBlack,   SymId::noSym            },
      { SymId::noteESharpWhole,  SymId::noteESharpHalf,  SymId::noteESharpBlack,  SymId::noSym            },
      { SymId::noteEWhole,       SymId::noteEHalf,       SymId::noteEBlack,       SymId::noSym            },
      { SymId::noteEFlatWhole,   SymId::noteEFlatHalf,   SymId::noteEFlatBlack,   SymId::noSym            },
      { SymId::noteFSharpWhole,  SymId::noteFSharpHalf,  SymId::noteFSharpBlack,  SymId::noSym            },
      { SymId::noteFWhole,       SymId::noteFHalf,       SymId::noteFBlack,       SymId::noSym            },
      { SymId::noteFFlatWhole,   SymId::noteFFlatHalf,   SymId::noteFFlatBlack,   SymId::noSym            },
      { SymId::noteGSharpWhole,  SymId::noteGSharpHalf,  SymId::noteGSharpBlack,  SymId::noSym            },
      { SymId::noteGWhole,       SymId::noteGHalf,       SymId::noteGBlack,       SymId::noSym            },
      { SymId::noteGFlatWhole,   SymId::noteGFlatHalf,   SymId::noteGFlatBlack,   SymId::noSym            },
      { SymId::noteHWhole,       SymId::noteHHalf,       SymId::noteHBlack,       SymId::noSym            },
      { SymId::noteHSharpWhole,  SymId::noteHSharpHalf,  SymId::noteHSharpBlack,  SymId::noSym            }

   },
   {     // up stem
      { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,               SymId::noteheadDoubleWhole  },
      { SymId::noteheadXWhole,              SymId::noteheadXHalf,               SymId::noteheadXBlack,              SymId::noteheadXDoubleWhole       },
      { SymId::noteheadPlusWhole,           SymId::noteheadPlusHalf,            SymId::noteheadPlusBlack,           SymId::noteheadPlusDoubleWhole  },
      { SymId::noteheadCircleXWhole,        SymId::noteheadCircleXHalf,         SymId::noteheadCircleX,             SymId::noteheadCircleXDoubleWhole},
      { SymId::noteheadWholeWithX,          SymId::noteheadHalfWithX,           SymId::noteheadVoidWithX,           SymId::noteheadDoubleWholeWithX},
      { SymId::noteheadTriangleUpWhole,     SymId::noteheadTriangleUpHalf,      SymId::noteheadTriangleUpBlack,     SymId::noteheadTriangleUpDoubleWhole },
      { SymId::noteheadTriangleDownWhole,   SymId::noteheadTriangleDownHalf,    SymId::noteheadTriangleDownBlack,   SymId::noteheadTriangleDownDoubleWhole },
      { SymId::noteheadSlashedWhole1,       SymId::noteheadSlashedHalf1,        SymId::noteheadSlashedBlack1,       SymId::noteheadSlashedDoubleWhole1 },
      { SymId::noteheadSlashedWhole2,       SymId::noteheadSlashedHalf2,        SymId::noteheadSlashedBlack2,       SymId::noteheadSlashedDoubleWhole2 },
      { SymId::noteheadDiamondWhole,        SymId::noteheadDiamondHalf,         SymId::noteheadDiamondBlack,        SymId::noteheadDiamondDoubleWhole  },
      { SymId::noteheadDiamondWholeOld,     SymId::noteheadDiamondHalfOld,      SymId::noteheadDiamondBlackOld,     SymId::noteheadDiamondDoubleWholeOld  },
      { SymId::noteheadCircledWhole,        SymId::noteheadCircledHalf,         SymId::noteheadCircledBlack,        SymId::noteheadCircledDoubleWhole  },
      { SymId::noteheadCircledWholeLarge,   SymId::noteheadCircledHalfLarge,    SymId::noteheadCircledBlackLarge,   SymId::noteheadCircledDoubleWholeLarge  },
      // different from down, find source?
      { SymId::noteheadLargeArrowDownWhole, SymId::noteheadLargeArrowDownHalf,  SymId::noteheadLargeArrowDownBlack, SymId::noteheadLargeArrowDownDoubleWhole  },
      { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,               SymId::noteheadDoubleWholeSquare   },

      { SymId::noteheadSlashWhiteWhole,     SymId::noteheadSlashWhiteHalf,      SymId::noteheadSlashHorizontalEnds, SymId::noteheadSlashWhiteDoubleWhole},

      { SymId::noteShapeRoundWhite,         SymId::noteShapeRoundWhite,         SymId::noteShapeRoundBlack,         SymId::noteShapeRoundDoubleWhole       },
      { SymId::noteShapeSquareWhite,        SymId::noteShapeSquareWhite,        SymId::noteShapeSquareBlack,        SymId::noteShapeSquareDoubleWhole      },
      // different from down
      { SymId::noteShapeTriangleLeftWhite,  SymId::noteShapeTriangleLeftWhite,  SymId::noteShapeTriangleLeftBlack,  SymId::noteShapeTriangleLeftDoubleWhole },
      { SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondBlack,       SymId::noteShapeDiamondDoubleWhole      },
      { SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpBlack,    SymId::noteShapeTriangleUpDoubleWhole   },
      { SymId::noteShapeMoonWhite,          SymId::noteShapeMoonWhite,          SymId::noteShapeMoonBlack,          SymId::noteShapeMoonDoubleWhole         },
      { SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundBlack, SymId::noteShapeTriangleRoundDoubleWhole },

      { SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneBlack,          SymId::noteShapeKeystoneDoubleWhole },
      { SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonBlack,       SymId::noteShapeQuarterMoonDoubleWhole },
      { SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleBlack, SymId::noteShapeIsoscelesTriangleDoubleWhole },
      { SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftBlack,          SymId::noteShapeMoonLeftDoubleWhole          },
      { SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftBlack,     SymId::noteShapeArrowheadLeftDoubleWhole     },
      { SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftBlack, SymId::noteShapeTriangleRoundLeftDoubleWhole },

      { SymId::noteDoWhole,  SymId::noteDoHalf,  SymId::noteDoBlack,  SymId::noSym            },
      { SymId::noteReWhole,  SymId::noteReHalf,  SymId::noteReBlack,  SymId::noSym            },
      { SymId::noteMiWhole,  SymId::noteMiHalf,  SymId::noteMiBlack,  SymId::noSym            },
      { SymId::noteFaWhole,  SymId::noteFaHalf,  SymId::noteFaBlack,  SymId::noSym            },
      { SymId::noteSoWhole,  SymId::noteSoHalf,  SymId::noteSoBlack,  SymId::noSym            },
      { SymId::noteLaWhole,  SymId::noteLaHalf,  SymId::noteLaBlack,  SymId::noSym            },
      { SymId::noteTiWhole,  SymId::noteTiHalf,  SymId::noteTiBlack,  SymId::noSym            },
      { SymId::noteSiWhole,  SymId::noteSiHalf,  SymId::noteSiBlack,  SymId::noSym            },

      { SymId::noteASharpWhole,  SymId::noteASharpHalf,  SymId::noteASharpBlack,  SymId::noSym            },
      { SymId::noteAWhole,       SymId::noteAHalf,       SymId::noteABlack,       SymId::noSym            },
      { SymId::noteAFlatWhole,   SymId::noteAFlatHalf,   SymId::noteAFlatBlack,   SymId::noSym            },
      { SymId::noteBSharpWhole,  SymId::noteBSharpHalf,  SymId::noteBSharpBlack,  SymId::noSym            },
      { SymId::noteBWhole,       SymId::noteBHalf,       SymId::noteBBlack,       SymId::noSym            },
      { SymId::noteBFlatWhole,   SymId::noteBFlatHalf,   SymId::noteBFlatBlack,   SymId::noSym            },
      { SymId::noteCSharpWhole,  SymId::noteCSharpHalf,  SymId::noteCSharpBlack,  SymId::noSym            },
      { SymId::noteCWhole,       SymId::noteCHalf,       SymId::noteCBlack,       SymId::noSym            },
      { SymId::noteCFlatWhole,   SymId::noteCFlatHalf,   SymId::noteCFlatBlack,   SymId::noSym            },
      { SymId::noteDSharpWhole,  SymId::noteDSharpHalf,  SymId::noteDSharpBlack,  SymId::noSym            },
      { SymId::noteDWhole,       SymId::noteDHalf,       SymId::noteDBlack,       SymId::noSym            },
      { SymId::noteDFlatWhole,   SymId::noteDFlatHalf,   SymId::noteDFlatBlack,   SymId::noSym            },
      { SymId::noteESharpWhole,  SymId::noteESharpHalf,  SymId::noteESharpBlack,  SymId::noSym            },
      { SymId::noteEWhole,       SymId::noteEHalf,       SymId::noteEBlack,       SymId::noSym            },
      { SymId::noteEFlatWhole,   SymId::noteEFlatHalf,   SymId::noteEFlatBlack,   SymId::noSym            },
      { SymId::noteFSharpWhole,  SymId::noteFSharpHalf,  SymId::noteFSharpBlack,  SymId::noSym            },
      { SymId::noteFWhole,       SymId::noteFHalf,       SymId::noteFBlack,       SymId::noSym            },
      { SymId::noteFFlatWhole,   SymId::noteFFlatHalf,   SymId::noteFFlatBlack,   SymId::noSym            },
      { SymId::noteGSharpWhole,  SymId::noteGSharpHalf,  SymId::noteGSharpBlack,  SymId::noSym            },
      { SymId::noteGWhole,       SymId::noteGHalf,       SymId::noteGBlack,       SymId::noSym            },
      { SymId::noteGFlatWhole,   SymId::noteGFlatHalf,   SymId::noteGFlatBlack,   SymId::noSym            },
      { SymId::noteHWhole,       SymId::noteHHalf,       SymId::noteHBlack,       SymId::noSym            },
      { SymId::noteHSharpWhole,  SymId::noteHSharpHalf,  SymId::noteHSharpBlack,  SymId::noSym            }

   }
};

struct NoteHeadName {
   const char* name;
   const char* username;
};

// same order as NoteHead::Group
static NoteHeadName noteHeadGroupNames[] = {
      {"normal",         QT_TRANSLATE_NOOP("noteheadnames", "Normal") },
      {"cross",          QT_TRANSLATE_NOOP("noteheadnames", "Cross") },
      {"plus",           QT_TRANSLATE_NOOP("noteheadnames", "Plus") },
      {"xcircle",        QT_TRANSLATE_NOOP("noteheadnames", "XCircle") },
      {"withx",          QT_TRANSLATE_NOOP("noteheadnames", "With X") },
      {"triangle-up",    QT_TRANSLATE_NOOP("noteheadnames", "Triangle Up") },
      {"triangle-down",  QT_TRANSLATE_NOOP("noteheadnames", "Triangle Down") },
      {"slashed1",       QT_TRANSLATE_NOOP("noteheadnames", "Slashed bottom left to top right") },
      {"slashed2",       QT_TRANSLATE_NOOP("noteheadnames", "Slashed top left to bottom right") },
      {"diamond",        QT_TRANSLATE_NOOP("noteheadnames", "Diamond") },
      {"diamond-old",    QT_TRANSLATE_NOOP("noteheadnames", "Diamond (Old)") },
      {"circled",        QT_TRANSLATE_NOOP("noteheadnames", "Circled") },
      {"circled-large",  QT_TRANSLATE_NOOP("noteheadnames", "Circled Large") },
      {"large-arrow",    QT_TRANSLATE_NOOP("noteheadnames", "Large Arrow") },
      {"altbrevis",      QT_TRANSLATE_NOOP("noteheadnames", "Alt. Brevis") },

      {"slash",     QT_TRANSLATE_NOOP("noteheadnames", "Slash") },

      // shape notes
      {"sol",       QT_TRANSLATE_NOOP("noteheadnames", "Sol") },
      {"la",        QT_TRANSLATE_NOOP("noteheadnames", "La") },
      {"fa",        QT_TRANSLATE_NOOP("noteheadnames", "Fa") },
      {"mi",        QT_TRANSLATE_NOOP("noteheadnames", "Mi") },
      {"do",        QT_TRANSLATE_NOOP("noteheadnames", "Do") },
      {"re",        QT_TRANSLATE_NOOP("noteheadnames", "Re") },
      {"ti",        QT_TRANSLATE_NOOP("noteheadnames", "Ti") },

      // not exposed
      {"do-walker", QT_TRANSLATE_NOOP("noteheadnames", "Do (Walker)") },
      {"re-walker", QT_TRANSLATE_NOOP("noteheadnames", "Re (Walker)") },
      {"ti-walker", QT_TRANSLATE_NOOP("noteheadnames", "Ti (Walker)") },
      {"do-funk",   QT_TRANSLATE_NOOP("noteheadnames", "Do (Funk)") },
      {"re-funk",   QT_TRANSLATE_NOOP("noteheadnames", "Re (Funk)") },
      {"ti-funk",   QT_TRANSLATE_NOOP("noteheadnames", "Ti (Funk)") },

      // note name
      {"do-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Do (Name)") },
      {"re-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Re (Name)") },
      {"mi-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Mi (Name)") },
      {"fa-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Fa (Name)") },
      {"sol-name", QT_TRANSLATE_NOOP("noteheadnames",  "Sol (Name)") },
      {"la-name",  QT_TRANSLATE_NOOP("noteheadnames",  "La (Name)") },
      {"ti-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Ti (Name)") },
      {"si-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Si (Name)") },


      {"a-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "A Sharp (Name)") },
      {"a-name",       QT_TRANSLATE_NOOP("noteheadnames",  "A (Name)") },
      {"a-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "A Flat (Name)") },
      {"b-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "B Sharp (Name)") },
      {"b-name",       QT_TRANSLATE_NOOP("noteheadnames",  "B (Name)") },
      {"b-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "B Flat (Name)") },
      {"c-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "C Sharp (Name)") },
      {"c-name",       QT_TRANSLATE_NOOP("noteheadnames",  "C (Name)") },
      {"c-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "C Flat (Name)") },
      {"d-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "D Sharp (Name)") },
      {"d-name",       QT_TRANSLATE_NOOP("noteheadnames",  "D (Name)") },
      {"d-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "D Flat (Name)") },
      {"e-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "E Sharp (Name)") },
      {"e-name",       QT_TRANSLATE_NOOP("noteheadnames",  "E (Name)") },
      {"e-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "E Flat (Name)") },
      {"f-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "F Sharp (Name)") },
      {"f-name",       QT_TRANSLATE_NOOP("noteheadnames",  "F (Name)") },
      {"f-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "F Flat (Name)") },
      {"g-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "G Sharp (Name)") },
      {"g-name",       QT_TRANSLATE_NOOP("noteheadnames",  "G (Name)") },
      {"g-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "G Flat (Name)") },
      {"h-name",       QT_TRANSLATE_NOOP("noteheadnames",  "H (Name)") },
      {"h-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "H Sharp (Name)") }
      };

// same order as NoteHead::Type
static NoteHeadName noteHeadTypeNames[] = {
      {"auto",    QT_TRANSLATE_NOOP("noteheadnames", "Auto") },
      {"whole",   QT_TRANSLATE_NOOP("noteheadnames", "Whole") },
      {"half",    QT_TRANSLATE_NOOP("noteheadnames", "Half") },
      {"quarter", QT_TRANSLATE_NOOP("noteheadnames", "Quarter") },
      {"breve",   QT_TRANSLATE_NOOP("noteheadnames", "Breve") },
      };

//---------------------------------------------------------
//   group2userName
//---------------------------------------------------------

QString NoteHead::group2userName(NoteHead::Group group)
      {
      return qApp->translate("noteheadnames", noteHeadGroupNames[int(group)].username);
      }

//---------------------------------------------------------
//   type2userName
//---------------------------------------------------------

QString NoteHead::type2userName(NoteHead::Type type)
      {
      return qApp->translate("noteheadnames", noteHeadTypeNames[int(type) + 1].username);
      }

//---------------------------------------------------------
//   group2name
//---------------------------------------------------------

QString NoteHead::group2name(NoteHead::Group group)
      {
      return noteHeadGroupNames[int(group)].name;
      }

//---------------------------------------------------------
//   type2name
//---------------------------------------------------------

QString NoteHead::type2name(NoteHead::Type type)
      {
      return noteHeadTypeNames[int(type) + 1].name;
      }

//---------------------------------------------------------
//   name2group
//---------------------------------------------------------

NoteHead::Group NoteHead::name2group(QString s)
      {
      for (int i = 0; i < int(NoteHead::Group::HEAD_GROUPS); ++i) {
            if (noteHeadGroupNames[i].name == s)
                  return NoteHead::Group(i);
            }
      return NoteHead::Group::HEAD_NORMAL;
      }

//---------------------------------------------------------
//   name2type
//---------------------------------------------------------

NoteHead::Type NoteHead::name2type(QString s)
      {
      for (int i = 0; i <= int(NoteHead::Type::HEAD_TYPES); ++i) {
            if (noteHeadTypeNames[i].name == s)
                  return NoteHead::Type(i - 1);
            }
      return NoteHead::Type::HEAD_AUTO;
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Note::noteHead(int direction, NoteHead::Group group, NoteHead::Type t)
      {
      return noteHeads[direction][int(group)][int(t)];
      }

SymId Note::noteHead(int direction, NoteHead::Group group, NoteHead::Type t, int tpc, Key key, NoteHeadScheme scheme)
      {
      // shortcut
      if (scheme == NoteHeadScheme::HEAD_NORMAL)
            return noteHeads[direction][int(group)][int(t)];
      // other schemes
      if (scheme == NoteHeadScheme::HEAD_PITCHNAME || scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN) {
            if (tpc == Tpc::TPC_A)
                  group = NoteHead::Group::HEAD_A;
            else if (tpc == Tpc::TPC_B) {
                  if (scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN)
                        group = NoteHead::Group::HEAD_H;
                  else
                        group = NoteHead::Group::HEAD_B;
                  }
            else if (tpc == Tpc::TPC_C)
                  group = NoteHead::Group::HEAD_C;
            else if (tpc == Tpc::TPC_D)
                  group = NoteHead::Group::HEAD_D;
            else if (tpc == Tpc::TPC_E)
                  group = NoteHead::Group::HEAD_E;
            else if (tpc == Tpc::TPC_F)
                  group = NoteHead::Group::HEAD_F;
            else if (tpc == Tpc::TPC_G)
                  group = NoteHead::Group::HEAD_G;
            else if (tpc == Tpc::TPC_A_S)
                  group = NoteHead::Group::HEAD_A_SHARP;
            else if (tpc == Tpc::TPC_B_S)
                  if (scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN)
                        group = NoteHead::Group::HEAD_H_SHARP;
                  else
                        group = NoteHead::Group::HEAD_B_SHARP;
            else if (tpc == Tpc::TPC_C_S)
                  group = NoteHead::Group::HEAD_C_SHARP;
            else if (tpc == Tpc::TPC_D_S)
                  group = NoteHead::Group::HEAD_D_SHARP;
            else if (tpc == Tpc::TPC_E_S)
                  group = NoteHead::Group::HEAD_E_SHARP;
            else if (tpc == Tpc::TPC_F_S)
                  group = NoteHead::Group::HEAD_F_SHARP;
            else if (tpc == Tpc::TPC_G_S)
                  group = NoteHead::Group::HEAD_G_SHARP;
            else if (tpc == Tpc::TPC_A_B)
                  group = NoteHead::Group::HEAD_A_FLAT;
            else if (tpc == Tpc::TPC_B_B)
                  if (scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN)
                        group = NoteHead::Group::HEAD_B;
                  else
                        group = NoteHead::Group::HEAD_B_FLAT;
            else if (tpc == Tpc::TPC_C_B)
                  group = NoteHead::Group::HEAD_C_FLAT;
            else if (tpc == Tpc::TPC_D_B)
                  group = NoteHead::Group::HEAD_D_FLAT;
            else if (tpc == Tpc::TPC_E_B)
                  group = NoteHead::Group::HEAD_E_FLAT;
            else if (tpc == Tpc::TPC_F_B)
                  group = NoteHead::Group::HEAD_F_FLAT;
            else if (tpc == Tpc::TPC_G_B)
                  group = NoteHead::Group::HEAD_G_FLAT;
            }
      else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_4) {
            int degree = tpc2degree(tpc, key);
            switch (degree) {
                  case 0:
                  case 3:
                        group = NoteHead::Group::HEAD_FA; break;
                  case 1:
                  case 4:
                        group = NoteHead::Group::HEAD_SOL; break;
                  case 2:
                  case 5:
                        group = NoteHead::Group::HEAD_LA; break;
                  case 6:
                        group = NoteHead::Group::HEAD_MI; break;
                  }
            }
      else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN
         || scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK
         || scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER) {
            int degree = tpc2degree(tpc, key);
            switch (degree) {
                  case 0:
                        if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN)
                              group = NoteHead::Group::HEAD_DO;
                        else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK)
                              group = NoteHead::Group::HEAD_DO_FUNK;
                        else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER)
                              group = NoteHead::Group::HEAD_DO_WALKER;
                        break;
                  case 1:
                        if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN)
                              group = NoteHead::Group::HEAD_RE;
                        else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK)
                              group = NoteHead::Group::HEAD_RE_FUNK;
                        else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER)
                              group = NoteHead::Group::HEAD_RE_WALKER;
                        break;
                  case 2:
                        group = NoteHead::Group::HEAD_MI; break;
                  case 3:
                        group = NoteHead::Group::HEAD_FA; break;
                  case 4:
                        group = NoteHead::Group::HEAD_SOL; break;
                  case 5:
                        group = NoteHead::Group::HEAD_LA; break;
                  case 6:
                        if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN)
                              group = NoteHead::Group::HEAD_TI;
                        else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK)
                              group = NoteHead::Group::HEAD_TI_FUNK;
                        else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER)
                              group = NoteHead::Group::HEAD_TI_WALKER;
                        break;
                  }
            }
      else if (scheme == NoteHeadScheme::HEAD_SOLFEGE) {
            int degree = tpc2degree(tpc, key);
            switch (degree) {
                  case 0:
                        group = NoteHead::Group::HEAD_DO_NAME; break;
                  case 1:
                        group = NoteHead::Group::HEAD_RE_NAME; break;
                  case 2:
                        group = NoteHead::Group::HEAD_MI_NAME; break;
                  case 3:
                        group = NoteHead::Group::HEAD_FA_NAME; break;
                  case 4:
                        group = NoteHead::Group::HEAD_SOL_NAME; break;
                  case 5:
                        group = NoteHead::Group::HEAD_LA_NAME; break;
                  case 6:
                        group = NoteHead::Group::HEAD_TI_NAME; break;
                  }
            }
      else if (scheme == NoteHeadScheme::HEAD_SOLFEGE_FIXED) {
            QString stepName = tpc2stepName(tpc);
            if (stepName == "C")
                  group = NoteHead::Group::HEAD_DO_NAME;
            else if (stepName == "D")
                  group = NoteHead::Group::HEAD_RE_NAME;
            else if (stepName == "E")
                  group = NoteHead::Group::HEAD_MI_NAME;
            else if (stepName == "F")
                  group = NoteHead::Group::HEAD_FA_NAME;
            else if (stepName == "G")
                  group = NoteHead::Group::HEAD_SOL_NAME;
            else if (stepName == "A")
                  group = NoteHead::Group::HEAD_LA_NAME;
            else if (stepName == "B")
                  group = NoteHead::Group::HEAD_SI_NAME;
            }
      return noteHeads[direction][int(group)][int(t)];
      };

//---------------------------------------------------------
//   headGroup
//   used only when dropping a notehead from the palette
//   they are either half note, either double whole
//---------------------------------------------------------

NoteHead::Group NoteHead::headGroup() const
      {
      Group group = Group::HEAD_INVALID;
      for (int i = 0; i < int(Group::HEAD_DO_WALKER); ++i) {
            if (noteHeads[0][i][1] == _sym || noteHeads[0][i][3] == _sym) {
                  group = (Group)i;
                        break;
                  }
            }
      return group;
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      _playEvents.append(NoteEvent());    // add default play event
      _cachedNoteheadSym = SymId::noSym;
      _cachedSymNull = SymId::noSym;
      }

Note::~Note()
      {
      delete _accidental;
      qDeleteAll(_el);
      delete _tieFor;
      qDeleteAll(_dots);
      }

Note::Note(const Note& n, bool link)
   : Element(n)
      {
      if (link)
            score()->undo(new Link(const_cast<Note*>(&n), this));
      _subchannel        = n._subchannel;
      _line              = n._line;
      _fret              = n._fret;
      _string            = n._string;
      _fretConflict      = n._fretConflict;
      _ghost             = n._ghost;
      dragMode           = n.dragMode;
      _pitch             = n._pitch;
      _tpc[0]            = n._tpc[0];
      _tpc[1]            = n._tpc[1];
      _dotsHidden        = n._dotsHidden;
      _hidden            = n._hidden;
      _play              = n._play;
      _tuning            = n._tuning;
      _veloType          = n._veloType;
      _veloOffset        = n._veloOffset;
      _headGroup         = n._headGroup;
      _headType          = n._headType;
      _mirror            = n._mirror;
      _userMirror        = n._userMirror;
      _small             = n._small;
      _userDotPosition   = n._userDotPosition;
      _fixed             = n._fixed;
      _fixedLine         = n._fixedLine;
      _accidental        = 0;

      if (n._accidental)
            add(new Accidental(*(n._accidental)));

      // types in _el: SYMBOL, IMAGE, FINGERING, TEXT, BEND
      for (Element* e : n._el) {
            Element* ce = e->clone();
            add(ce);
            if (link)
                  score()->undo(new Link(const_cast<Element*>(e), ce));
            }

      _playEvents = n._playEvents;

      if (n._tieFor) {
            _tieFor = new Tie(*n._tieFor);
            _tieFor->setStartNote(this);
            _tieFor->setEndNote(0);
            }
      else
            _tieFor = 0;
      _tieBack  = 0;
      for (NoteDot* dot : n._dots)
            add(new NoteDot(*dot));
      _lineOffset = n._lineOffset;
      _mark      = n._mark;
      }

//---------------------------------------------------------
//   concertPitchIdx
//---------------------------------------------------------

inline int Note::concertPitchIdx() const
      {
      return concertPitch() ? 0 : 1;
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void Note::setPitch(int val)
      {
      Q_ASSERT(val >= 0 && val <= 127);
      if (_pitch != val) {
            _pitch = val;
            score()->setPlaylistDirty();
            }
      }

void Note::setPitch(int pitch, int tpc1, int tpc2)
      {
      Q_ASSERT(tpcIsValid(tpc1));
      Q_ASSERT(tpcIsValid(tpc2));
      _tpc[0] = tpc1;
      _tpc[1] = tpc2;
      setPitch(pitch);
      }

//---------------------------------------------------------
//   undoSetPitch
//---------------------------------------------------------

void Note::undoSetPitch(int p)
      {
      undoChangeProperty(P_ID::PITCH, p);
      }

//---------------------------------------------------------
//   tpc1default
//---------------------------------------------------------

int Note::tpc1default(int p) const
      {
      Key key = Key::C;
      if (staff() && chord()) {
            int tick = chord()->tick();
            key = staff()->key(tick);
            if (!concertPitch()) {
                  Interval interval = part()->instrument(tick)->transpose();
                  if (!interval.isZero()) {
                        interval.flip();
                        key = transposeKey(key, interval);
                        }
                  }
            }
      return pitch2tpc(p, key, Prefer::NEAREST);
      }

//---------------------------------------------------------
//   tpc2default
//---------------------------------------------------------

int Note::tpc2default(int p) const
      {
      Key key = Key::C;
      if (staff() && chord()) {
            int tick = chord()->tick();
            key = staff()->key(tick);
            if (concertPitch()) {
                  Interval interval = part()->instrument(tick)->transpose();
                  if (!interval.isZero())
                        key = transposeKey(key, interval);
                  }
            }
      return pitch2tpc(p - transposition(), key, Prefer::NEAREST);
      }

//---------------------------------------------------------
//   setTpcFromPitch
//---------------------------------------------------------

void Note::setTpcFromPitch()
      {
      // works best if note is already added to score, otherwise we can't determine transposition or key
      int tick = chord() ? chord()->tick() : -1;
      Interval v = staff() ? part()->instrument(tick)->transpose() : Interval();
      Key key = (staff() && chord()) ? staff()->key(chord()->tick()) : Key::C;
      // convert key to concert pitch
      if (!concertPitch() && !v.isZero())
            key = transposeKey(key, v);
      // set concert pitch tpc
      _tpc[0] = pitch2tpc(_pitch, key, Prefer::NEAREST);
      // set transposed tpc
      if (v.isZero())
            _tpc[1] = _tpc[0];
      else {
            v.flip();
            _tpc[1] = Ms::transposeTpc(_tpc[0], v, true);
            }
      Q_ASSERT(tpcIsValid(_tpc[0]));
      Q_ASSERT(tpcIsValid(_tpc[1]));
      }

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

void Note::setTpc(int v)
      {
      if (!tpcIsValid(v))
            qFatal("Note::setTpc: bad tpc %d", v);
      _tpc[concertPitchIdx()] = v;
      }

//---------------------------------------------------------
//   undoSetTpc
//    change the current tpc
//---------------------------------------------------------

void Note::undoSetTpc(int v)
      {
      if (concertPitch()) {
            if (v != tpc1())
                  undoChangeProperty(P_ID::TPC1, v);
            }
      else {
            if (v != tpc2())
                  undoChangeProperty(P_ID::TPC2, v);
            }
      }

//---------------------------------------------------------
//   tpc
//---------------------------------------------------------

int Note::tpc() const
      {
      return _tpc[concertPitchIdx()];
      }

//---------------------------------------------------------
//   tpcUserName
//---------------------------------------------------------

QString Note::tpcUserName(bool explicitAccidental) const
      {
      QString pitchName = tpc2name(tpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, explicitAccidental);
      QString octaveName = QString::number((pitch() / 12) - 1);
      return pitchName + (explicitAccidental ? " " : "") + octaveName;
      }

//---------------------------------------------------------
//   transposeTpc
//    return transposed tpc
//    If in concertPitch mode return tpc for transposed view
//    else return tpc for concert pitch view.
//---------------------------------------------------------

int Note::transposeTpc(int tpc)
      {
      int tick = chord() ? chord()->tick() : -1;
      Interval v = part()->instrument(tick)->transpose();
      if (v.isZero())
            return tpc;
      if (concertPitch()) {
            v.flip();
            return Ms::transposeTpc(tpc, v, true);
            }
      else
            return Ms::transposeTpc(tpc, v, true);
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Note::noteHead() const
      {
      int up;
      NoteHead::Type ht;
      if (chord()) {
            up = chord()->up();
            ht = chord()->durationType().headType();
            }
      else {
            up = 1;
            ht = NoteHead::Type::HEAD_QUARTER;
            }
      if (_headType != NoteHead::Type::HEAD_AUTO)
            ht = _headType;
      Key key = Key::C;
      NoteHeadScheme scheme = NoteHeadScheme::HEAD_NORMAL;
      if (chord() && chord()->staff() && chord()->tick() >= 0){
            key = chord()->staff()->key(chord()->tick());
            scheme = chord()->staff()->staffType()->noteHeadScheme();
            }
      SymId t = noteHead(up, _headGroup, ht, tpc(), key, scheme);
      if (t == SymId::noSym) {
            qDebug("invalid notehead %d/%d", int(_headGroup), int(ht));
            t = noteHead(up, NoteHead::Group::HEAD_NORMAL, ht);
            }
      return t;
      }

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the notehead symbol
//    or the width of the string representation of the fret mark
//---------------------------------------------------------

qreal Note::headWidth() const
      {
      return symWidth(noteHead());
      }

//---------------------------------------------------------
//   tabHeadWidth
//---------------------------------------------------------

qreal Note::tabHeadWidth(StaffType* tab) const
      {
      qreal val;
      if (tab && _fret != FRET_NONE && _string != STRING_NONE) {
            QFont f    = tab->fretFont();
            f.setPointSizeF(tab->fretFontSize());
            QFontMetricsF fm(f, MScore::paintDevice());
            QString s;
            if (fixed())
                s = "/";
            else
                s = tab->fretString(_fret, _string, _ghost);
            val  = fm.width(s) * magS();
            }
      else
            val = headWidth();
      return val;
      }

//---------------------------------------------------------
//   headHeight
//
//    returns the height of the notehead symbol
//    or the height of the string representation of the fret mark
//---------------------------------------------------------

qreal Note::headHeight() const
      {
      return symHeight(noteHead());
      }

//---------------------------------------------------------
//   tabHeadHeight
//---------------------------------------------------------

qreal Note::tabHeadHeight(StaffType* tab) const
      {
      if (tab && _fret != FRET_NONE && _string != STRING_NONE)
            return tab->fretBoxH() * magS();
      return headHeight();
      }

//---------------------------------------------------------
//   stemDownNW
//---------------------------------------------------------

QPointF Note::stemDownNW() const
      {
      return symStemDownNW(noteHead());
      }

//---------------------------------------------------------
//   stemUpSE
//---------------------------------------------------------

QPointF Note::stemUpSE() const
      {
      return symStemUpSE(noteHead());
      }

//---------------------------------------------------------
//   playTicks
///   Return total tick len of tied notes
//---------------------------------------------------------

int Note::playTicks() const
      {
      int stick = firstTiedNote()->chord()->tick();
      const Note* note = lastTiedNote();
      return note->chord()->tick() + note->chord()->actualTicks() - stick;
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Note::addSpanner(Spanner* l)
      {
      Element* e = l->endElement();
      if (e && e->isNote()) {
            Note* note = toNote(e);
            note->addSpannerBack(l);
            if (l->isGlissando())
                 note->chord()->setEndsGlissando(true);
            }
      addSpannerFor(l);
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Note::removeSpanner(Spanner* l)
      {
      Note* e = static_cast<Note*>(l->endElement());
      if (e && e->isNote()) {
            if (!e->removeSpannerBack(l)) {
                  qDebug("Note::removeSpanner(%p): cannot remove spannerBack %s %p", this, l->name(), l);
                  // abort();
                  }
            if (l->isGlissando())
                 e->chord()->updateEndsGlissando();
            }
      if (!removeSpannerFor(l)) {
            qDebug("Note(%p): cannot remove spannerFor %s %p", this, l->name(), l);
            // abort();
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());

      switch(e->type()) {
            case Element::Type::NOTEDOT:
                  _dots.append(toNoteDot(e));
                  break;
            case Element::Type::SYMBOL:
            case Element::Type::IMAGE:
            case Element::Type::FINGERING:
            case Element::Type::TEXT:
            case Element::Type::BEND:
                  _el.push_back(e);
                  break;
            case Element::Type::TIE:
                  {
                  Tie* tie = toTie(e);
                  tie->setStartNote(this);
                  tie->setTrack(track());
                  setTieFor(tie);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(tie);
                  int n = tie->spannerSegments().size();
                  for (int i = 0; i < n; ++i) {
                        SpannerSegment* ss = tie->spannerSegments().at(i);
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;
            case Element::Type::ACCIDENTAL:
                  _accidental = toAccidental(e);
                  break;
            case Element::Type::TEXTLINE:
            case Element::Type::GLISSANDO:
                  addSpanner(static_cast<Spanner*>(e));
                  break;
            default:
                  qDebug("Note::add() not impl. %s", e->name());
                  break;
            }
      score()->setLayout(tick());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Note::remove(Element* e)
      {
      switch(e->type()) {
            case Element::Type::NOTEDOT:
                  _dots.takeLast();
                  break;

            case Element::Type::TEXT:
            case Element::Type::SYMBOL:
            case Element::Type::IMAGE:
            case Element::Type::FINGERING:
            case Element::Type::BEND:
                  if (!_el.remove(e))
                        qDebug("Note::remove(): cannot find %s", e->name());
                  break;
            case Element::Type::TIE:
                  {
                  Tie* tie = toTie(e);
                  setTieFor(0);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(0);
                  for (SpannerSegment* ss : tie->spannerSegments()) {
                        Q_ASSERT(ss->spanner() == tie);
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
                  break;

            case Element::Type::ACCIDENTAL:
                  _accidental = 0;
                  break;

            case Element::Type::TEXTLINE:
            case Element::Type::GLISSANDO:
                  removeSpanner(static_cast<Spanner*>(e));
                  break;

            default:
                  qDebug("Note::remove() not impl. %s", e->name());
                  break;
            }
      score()->setLayout(tick());
      }

//---------------------------------------------------------
//   isNoteName
//---------------------------------------------------------

bool Note::isNoteName() const
      {
      if (chord()) {
            NoteHeadScheme s = chord()->staff()->staffType()->noteHeadScheme();
            return s == NoteHeadScheme::HEAD_PITCHNAME || s == NoteHeadScheme::HEAD_PITCHNAME_GERMAN || s == NoteHeadScheme::HEAD_SOLFEGE || s == NoteHeadScheme::HEAD_SOLFEGE_FIXED;
            }
      return false;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(QPainter* painter) const
      {
      if (_hidden)
            return;

      QColor c(curColor());
      painter->setPen(c);
      bool tablature = staff() && staff()->isTabStaff();

      // tablature

      if (tablature) {
            StaffType* tab = staff()->staffType();
            if (tieBack() && !tab->showBackTied())    // skip back-tied notes if not shown
                  return;
            QString s;
            if (fixed())
                  s = "/";
            else
                  s = tab->fretString(_fret, _string, _ghost);

            // draw background, if required (to hide a segment of string line or to show a fretting conflict)
            if (!tab->linesThrough() || fretConflict()) {
                  qreal d  = spatium() * .1;
                  QRectF bb = QRectF(bbox().x()-d, tab->fretMaskY()*magS(), bbox().width() + 2*d, tab->fretMaskH()*magS());
                  // we do not know which viewer did this draw() call
                  // so update all:
                  for (MuseScoreView* view : score()->getViewer())
                        view->drawBackground(painter, bb);

                  if (fretConflict() && !score()->printing()) {          //on fret conflict, draw on red background
                        painter->save();
                        painter->setPen(Qt::red);
                        painter->setBrush(QBrush(QColor(Qt::red)));
                        painter->drawRect(bb);
                        painter->restore();
                        }
                  }
            QFont f(tab->fretFont());
            f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(f);
            painter->setPen(c);
            painter->drawText(QPointF(bbox().x(), tab->fretFontYOffset()), s);
            }

      // NOT tablature

      else {
            // skip drawing, if second note of a cross-measure value
            if (chord()->crossMeasure() == CrossMeasure::SECOND)
                  return;
            // warn if pitch extends usable range of instrument
            // by coloring the notehead
            if (chord() && chord()->segment() && staff() && !selected()
               && !score()->printing() && MScore::warnPitchRange) {
                  const Instrument* in = part()->instrument(chord()->tick());
                  int i = ppitch();
                  if (i < in->minPitchP() || i > in->maxPitchP())
                        painter->setPen(Qt::red);
                  else if (i < in->minPitchA() || i > in->maxPitchA())
                        painter->setPen(Qt::darkYellow);
                  }
            // draw blank notehead to avoid staff and ledger lines
            if (_cachedSymNull != SymId::noSym) {
                  painter->save();
                  painter->setPen(Qt::white);
                  drawSymbol(_cachedSymNull, painter);
                  painter->restore();
                  }
            drawSymbol(_cachedNoteheadSym, painter);
            }
      }

//--------------------------------------------------
//   Note::write
//---------------------------------------------------------

void Note::write(XmlWriter& xml) const
      {
      xml.stag("Note");
      Element::writeProperties(xml);

      if (_accidental)
            _accidental->write(xml);
      _el.write(xml);
      for (NoteDot* dot : _dots) {
            if (!dot->userOff().isNull() || !dot->visible() || dot->color() != Qt::black || dot->visible() != visible()) {
                  dot->write(xml);
                  break;
                  }
            }
      if (_tieFor)
            _tieFor->write(xml);
      if (_tieBack) {
            int id = xml.spannerId(_tieBack);
            xml.tagE(QString("endSpanner id=\"%1\"").arg(id));
            }
      if ((chord() == 0 || chord()->playEventType() != PlayEventType::Auto) && !_playEvents.empty()) {
            xml.stag("Events");
            for (const NoteEvent& e : _playEvents)
                  e.write(xml);
            xml.etag();
            }
      writeProperty(xml, P_ID::PITCH);
      // write tpc1 before tpc2 !
      writeProperty(xml, P_ID::TPC1);
      if (_tpc[1] != _tpc[0])
            writeProperty(xml, P_ID::TPC2);
      writeProperty(xml, P_ID::SMALL);
      writeProperty(xml, P_ID::MIRROR_HEAD);
      writeProperty(xml, P_ID::DOT_POSITION);
      writeProperty(xml, P_ID::HEAD_GROUP);
      writeProperty(xml, P_ID::VELO_OFFSET);
      writeProperty(xml, P_ID::PLAY);
      writeProperty(xml, P_ID::TUNING);
      writeProperty(xml, P_ID::FRET);
      writeProperty(xml, P_ID::STRING);
      writeProperty(xml, P_ID::GHOST);
      writeProperty(xml, P_ID::HEAD_TYPE);
      writeProperty(xml, P_ID::VELO_TYPE);
      writeProperty(xml, P_ID::FIXED);
      writeProperty(xml, P_ID::FIXED_LINE);

      for (Spanner* e : _spannerFor)
            e->write(xml);
      for (Spanner* e : _spannerBack)
            xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(e)));

      xml.etag();
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(XmlReader& e)
      {
      setTpc1(Tpc::TPC_INVALID);
      setTpc2(Tpc::TPC_INVALID);

      while (e.readNextStartElement()) {
            if (readProperties(e))
                  ;
            else
                  e.unknown();
            }
      // ensure sane values:
      _pitch = limit(_pitch, 0, 127);

      if (!tpcIsValid(_tpc[0]) && !tpcIsValid(_tpc[1])) {
            Key key = (staff() && chord()) ? staff()->key(chord()->tick()) : Key::C;
            int tpc = pitch2tpc(_pitch, key, Prefer::NEAREST);
            if (concertPitch())
                  _tpc[0] = tpc;
            else
                  _tpc[1] = tpc;
            }
      if (!(tpcIsValid(_tpc[0]) && tpcIsValid(_tpc[1]))) {
            int tick = chord() ? chord()->tick() : -1;
            Interval v = staff() ? part()->instrument(tick)->transpose() : Interval();
            if (tpcIsValid(_tpc[0])) {
                  v.flip();
                  if (v.isZero())
                        _tpc[1] = _tpc[0];
                  else
                        _tpc[1] = Ms::transposeTpc(_tpc[0], v, true);
                  }
            else {
                  if (v.isZero())
                        _tpc[0] = _tpc[1];
                  else
                        _tpc[0] = Ms::transposeTpc(_tpc[1], v, true);
                  }
            }

      // check consistency of pitch, tpc1, tpc2, and transposition
      // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
      // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
      // including perhaps some we don't know about yet,
      // we will attempt to fix some problems here regardless of version

      if (!e.pasteMode() && !MScore::testMode) {
            int tpc1Pitch = (tpc2pitch(_tpc[0]) + 12) % 12;
            int tpc2Pitch = (tpc2pitch(_tpc[1]) + 12) % 12;
            int concertPitch = _pitch % 12;
            if (tpc1Pitch != concertPitch) {
                  qDebug("bad tpc1 - concertPitch = %d, tpc1 = %d", concertPitch, tpc1Pitch);
                  _pitch += tpc1Pitch - concertPitch;
                  }
            Interval v = staff()->part()->instrument(e.tick())->transpose();
            int transposedPitch = (_pitch - v.chromatic) % 12;
            if (tpc2Pitch != transposedPitch) {
                  qDebug("bad tpc2 - transposedPitch = %d, tpc2 = %d", transposedPitch, tpc2Pitch);
                  }
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Note::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "pitch")
            _pitch = e.readInt();
      else if (tag == "tpc") {
            _tpc[0] = e.readInt();
            _tpc[1] = _tpc[0];
            }
      else if (tag == "track")            // for performance
            setTrack(e.readInt());
      else if (tag == "Accidental") {
            Accidental* a = new Accidental(score());
            a->setTrack(track());
            a->read(e);
            add(a);
            }
      else if (tag == "Tie") {
            Tie* tie = new Tie(score());
            tie->setParent(this);
            tie->setTrack(track());
            tie->read(e);
            tie->setStartNote(this);
            _tieFor = tie;
            }
      else if (tag == "tpc2")
            _tpc[1] = e.readInt();
      else if (tag == "small")
            setSmall(e.readInt());
      else if (tag == "mirror")
            setProperty(P_ID::MIRROR_HEAD, Ms::getProperty(P_ID::MIRROR_HEAD, e));
      else if (tag == "dotPosition")
            setProperty(P_ID::DOT_POSITION, Ms::getProperty(P_ID::DOT_POSITION, e));
      else if (tag == "fixed")
            setFixed(e.readBool());
      else if (tag == "fixedLine")
            setFixedLine(e.readInt());
      else if (tag == "head")
            setProperty(P_ID::HEAD_GROUP, Ms::getProperty(P_ID::HEAD_GROUP, e));
      else if (tag == "velocity")
            setVeloOffset(e.readInt());
      else if (tag == "play")
            setPlay(e.readInt());
      else if (tag == "tuning")
            setTuning(e.readDouble());
      else if (tag == "fret")
            setFret(e.readInt());
      else if (tag == "string")
            setString(e.readInt());
      else if (tag == "ghost")
            setGhost(e.readInt());
      else if (tag == "headType")
            setProperty(P_ID::HEAD_TYPE, Ms::getProperty(P_ID::HEAD_TYPE, e));
      else if (tag == "veloType")
            setProperty(P_ID::VELO_TYPE, Ms::getProperty(P_ID::VELO_TYPE, e));
      else if (tag == "line")
            _line = e.readInt();
      else if (tag == "Fingering") {
            Fingering* f = new Fingering(score());
            f->setTextStyleType(TextStyleType::FINGERING);
            f->read(e);
            add(f);
            }
      else if (tag == "Symbol") {
            Symbol* s = new Symbol(score());
            s->setTrack(track());
            s->read(e);
            add(s);
            }
      else if (tag == "Image") {
            if (MScore::noImages)
                  e.skipCurrentElement();
            else {
                  Image* image = new Image(score());
                  image->setTrack(track());
                  image->read(e);
                  add(image);
                  }
            }
      else if (tag == "Bend") {
            Bend* b = new Bend(score());
            b->setTrack(track());
            b->read(e);
            add(b);
            }
      else if (tag == "NoteDot") {
            NoteDot* dot = new NoteDot(score());
            dot->read(e);
            add(dot);
            }
      else if (tag == "Events") {
            _playEvents.clear();    // remove default event
            while (e.readNextStartElement()) {
                  const QStringRef& tag(e.name());
                  if (tag == "Event") {
                        NoteEvent ne;
                        ne.read(e);
                        _playEvents.append(ne);
                        }
                  else
                        e.unknown();
                  }
            if (chord())
                  chord()->setPlayEventType(PlayEventType::User);
            }
      else if (tag == "endSpanner") {
            int id = e.intAttribute("id");
            Spanner* sp = e.findSpanner(id);
            if (sp) {
                  sp->setEndElement(this);
                  if (sp->isTie())
                        _tieBack = toTie(sp);
                  else {
                        if (sp->isGlissando() && parent() && parent()->isChord())
                              toChord(parent())->setEndsGlissando(true);
                        addSpannerBack(sp);
                        }
                  e.removeSpanner(sp);
                  }
            else {
                  // End of a spanner whose start element will appear later;
                  // may happen for cross-staff spanner from a lower to a higher staff
                  // (for instance a glissando from bass to treble staff of piano).
                  // Create a place-holder spanner with end data
                  // (a TextLine is used only because both Spanner or SLine are abstract,
                  // the actual class does not matter, as long as it is derived from Spanner)
                  int id = e.intAttribute("id", -1);
                  if (id != -1 &&
                              // DISABLE if pasting into a staff with linked staves
                              // because the glissando is not properly cloned into the linked staves
                              (!e.pasteMode() || !staff()->linkedStaves() || staff()->linkedStaves()->empty())) {
                        Spanner* placeholder = new TextLine(score());
                        placeholder->setAnchor(Spanner::Anchor::NOTE);
                        placeholder->setEndElement(this);
                        placeholder->setTrack2(track());
                        placeholder->setTick(0);
                        placeholder->setTick2(e.tick());
                        e.addSpanner(id, placeholder);
                        }
                  }
            e.readNext();
            }
      else if (tag == "TextLine"
            || tag == "Glissando") {
            Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
            // check this is not a lower-to-higher cross-staff spanner we already got
            int id = e.intAttribute("id");
            Spanner* placeholder = e.findSpanner(id);
            if (placeholder) {
                  // if it is, fill end data from place-holder
                  sp->setAnchor(Spanner::Anchor::NOTE);           // make sure we can set a Note as end element
                  sp->setEndElement(placeholder->endElement());
                  sp->setTrack2(placeholder->track2());
                  sp->setTick(e.tick());                          // make sure tick2 will be correct
                  sp->setTick2(placeholder->tick2());
                  static_cast<Note*>(placeholder->endElement())->addSpannerBack(sp);
                  // remove no longer needed place-holder before reading the new spanner,
                  // as reading it also adds it to XML reader list of spanners,
                  // which would overwrite the place-holder
                  e.removeSpanner(placeholder);
                  delete placeholder;
                  }
            sp->setTrack(track());
            sp->read(e);
            // DISABLE pasting of glissandi into staves with other lionked staves
            // because the glissando is not properly cloned into the linked staves
            if (e.pasteMode() && staff()->linkedStaves() && !staff()->linkedStaves()->empty()) {
                  e.removeSpanner(sp);    // read() added the element to the XMLReader: remove it
                  delete sp;
                  }
            else {
                  sp->setAnchor(Spanner::Anchor::NOTE);
                  sp->setStartElement(this);
                  sp->setTick(e.tick());
                  addSpannerFor(sp);
                  sp->setParent(this);
                  }
            }
      else if (tag == "offset")
            Element::readProperties(e);
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Note::drag(EditData* data)
      {
      if (staff()->isDrumStaff())
            return QRect();
      dragMode = true;
      QRectF bb(chord()->bbox());

      qreal _spatium = spatium();
      bool tab       = staff()->isTabStaff();
      qreal step     = _spatium * (tab ? staff()->staffType()->lineDistance().val() : 0.5);
      _lineOffset    = lrint(data->delta.y() / step);
      triggerLayout();
      return bb.translated(chord()->pagePos());
      }

//---------------------------------------------------------
//   transposition
//---------------------------------------------------------

int Note::transposition() const
      {
      int tick = chord() ? chord()->tick() : -1;
      return staff() ? part()->instrument(tick)->transpose().chromatic : 0;
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Note::endDrag()
      {
      dragMode = false;
      if (_lineOffset == 0)
            return;

      int staffIdx = chord()->vStaffIdx();
      Staff* staff = score()->staff(staffIdx);
      int tick     = chord()->tick();

      if (staff->isTabStaff()) {
            // on TABLATURE staves, dragging a note keeps same pitch on a different string (if possible)
            // determine new string of dragged note (if tablature is upside down, invert _lineOffset)
            // and fret for the same pitch on the new string
            const StringData* strData = staff->part()->instrument()->stringData();
            int nString = _string + (staff->staffType()->upsideDown() ? -_lineOffset : _lineOffset);
            int nFret   = strData->fret(_pitch, nString, staff, tick);
            if (nFret < 0)                      // no fret?
                  return;                       // no party!
            // move the note together with all notes tied to it
            for (Note* nn : tiedNotes()) {
                  bool refret = false;
                  if (nn->fret() != nFret) {
                        nn->undoChangeProperty(P_ID::FRET, nFret);
                        refret = true;
                        }
                  if (nn->string() != nString) {
                        nn->undoChangeProperty(P_ID::STRING, nString);
                        refret = true;
                        }
                  if (refret)
                        strData->fretChords(nn->chord());
                  }
            }
      else {
            // on PITCHED / PERCUSSION staves, dragging a note changes the note pitch
            int nLine   = _line + _lineOffset;
            // get note context
            ClefType clef = staff->clef(tick);
            Key key       = staff->key(tick);
            // determine new pitch of dragged note
            int nPitch = line2pitch(nLine, clef, key);
            if (!concertPitch()) {
                  Interval interval = staff->part()->instrument(tick)->transpose();
                  nPitch += interval.chromatic;
                  }
            int tpc1 = pitch2tpc(nPitch, key, Prefer::NEAREST);
            int tpc2 = pitch2tpc(nPitch - transposition(), key, Prefer::NEAREST);
            // undefined for non-tablature staves
            for (Note* nn : tiedNotes()) {
                  // score()->undoChangePitch(nn, nPitch, tpc1, tpc2);
                  nn->undoChangeProperty(P_ID::PITCH, nPitch);
                  nn->undoChangeProperty(P_ID::TPC1, tpc1);
                  nn->undoChangeProperty(P_ID::TPC2, tpc2);
                  }
            }
      _lineOffset = 0;
      score()->select(this, SelectType::SINGLE, 0);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(const DropData& data) const
      {
      Element* e = data.element;
      Element::Type type = e->type();
      if (type == Element::Type::GLISSANDO) {
            for (auto e : _spannerFor)
                  if (e->type() == Element::Type::GLISSANDO) {
                        return false;
                  }
            return true;
            }
      return (type == Element::Type::ARTICULATION
         || type == Element::Type::CHORDLINE
         || type == Element::Type::TEXT
         || type == Element::Type::REHEARSAL_MARK
         || type == Element::Type::FINGERING
         || type == Element::Type::ACCIDENTAL
         || type == Element::Type::BREATH
         || type == Element::Type::ARPEGGIO
         || type == Element::Type::NOTEHEAD
         || type == Element::Type::NOTE
         || type == Element::Type::TREMOLO
         || type == Element::Type::STAFF_STATE
         || type == Element::Type::INSTRUMENT_CHANGE
         || type == Element::Type::IMAGE
         || type == Element::Type::CHORD
         || type == Element::Type::HARMONY
         || type == Element::Type::DYNAMIC
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::ACCIACCATURA)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::APPOGGIATURA)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::GRACE4)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::GRACE16)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::GRACE32)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::GRACE8_AFTER)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::GRACE16_AFTER)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::GRACE32_AFTER)
         || (noteType() == NoteType::NORMAL && type == Element::Type::BAGPIPE_EMBELLISHMENT)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::SBEAM)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::MBEAM)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::NBEAM)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::BEAM32)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::BEAM64)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::AUTOBEAM)
         || (type == Element::Type::ICON && toIcon(e)->iconType() == IconType::BRACKETS)
         || (type == Element::Type::SYMBOL)
         || (type == Element::Type::CLEF)
         || (type == Element::Type::KEYSIG)
         || (type == Element::Type::TIMESIG)
         || (type == Element::Type::BAR_LINE)
         || (type == Element::Type::SLUR)
         || (type == Element::Type::HAIRPIN)
         || (type == Element::Type::STAFF_TEXT)
         || (type == Element::Type::TEMPO_TEXT)
         || (type == Element::Type::BEND)
         || (type == Element::Type::TREMOLOBAR)
         || (type == Element::Type::FRET_DIAGRAM)
         || (type == Element::Type::FIGURED_BASS)
         || (type == Element::Type::LYRICS));
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Note::drop(const DropData& data)
      {
      Element* e = data.element;
      bool fromPalette = (e->track() == -1);

      Chord* ch = chord();
      switch(e->type()) {
            case Element::Type::REHEARSAL_MARK:
                  return ch->drop(data);

            case Element::Type::SYMBOL:
            case Element::Type::IMAGE:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::FINGERING:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  {
                  // set style
                  Fingering* f = toFingering(e);
                  TextStyleType st = f->textStyleType();
                  //f->setTextStyleType(st);
                  if (st >= TextStyleType::DEFAULT && fromPalette)
                        f->textStyle().restyle(MScore::baseStyle()->textStyle(st), score()->textStyle(st));
                  }
                  return e;

            case Element::Type::SLUR:
                  delete e;
                  data.view->cmdAddSlur(this, 0);
                  return 0;

            case Element::Type::HAIRPIN:
                  {
                  Hairpin* hairpin = toHairpin(e);
                  data.view->cmdAddHairpin(hairpin->hairpinType());
                  delete e;
                  }
                  return 0;

            case Element::Type::LYRICS:
                  e->setParent(ch);
                  e->setTrack(track());
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::ACCIDENTAL:
                  score()->changeAccidental(this, static_cast<Accidental*>(e)->accidentalType());
                  break;

            case Element::Type::BEND:
                  e->setParent(this);
                  e->setTrack(track());
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::NOTEHEAD:
                  {
                  NoteHead* s = toNoteHead(e);
                  NoteHead::Group group = s->headGroup();
                  if (group == NoteHead::Group::HEAD_INVALID) {
                        qDebug("unknown notehead");
                        group = NoteHead::Group::HEAD_NORMAL;
                        }
                  delete s;

                  if (group != _headGroup) {
                        if (links()) {
                              for (ScoreElement* e : *links()) {
                                    e->undoChangeProperty(P_ID::HEAD_GROUP, int(group));
                                    Note* note = static_cast<Note*>(e);
                                    if (note->staff() && note->staff()->isTabStaff() && group == NoteHead::Group::HEAD_CROSS)
                                          e->undoChangeProperty(P_ID::GHOST, true);
                                    }
                              }
                        else {
                              undoChangeProperty(P_ID::HEAD_GROUP, int(group));
                              }
                        }
                  }
                  break;

            case Element::Type::ICON:
                  {
                  switch (toIcon(e)->iconType()) {
                        case IconType::ACCIACCATURA:
                              score()->setGraceNote(ch, pitch(), NoteType::ACCIACCATURA, MScore::division/2);
                              break;
                        case IconType::APPOGGIATURA:
                              score()->setGraceNote(ch, pitch(), NoteType::APPOGGIATURA, MScore::division/2);
                              break;
                        case IconType::GRACE4:
                              score()->setGraceNote(ch, pitch(), NoteType::GRACE4, MScore::division);
                              break;
                        case IconType::GRACE16:
                              score()->setGraceNote(ch, pitch(), NoteType::GRACE16,  MScore::division/4);
                              break;
                        case IconType::GRACE32:
                              score()->setGraceNote(ch, pitch(), NoteType::GRACE32, MScore::division/8);
                              break;
                        case IconType::GRACE8_AFTER:
                              score()->setGraceNote(ch, pitch(), NoteType::GRACE8_AFTER, MScore::division/2);
                              break;
                        case IconType::GRACE16_AFTER:
                              score()->setGraceNote(ch, pitch(), NoteType::GRACE16_AFTER, MScore::division/4);
                              break;
                        case IconType::GRACE32_AFTER:
                              score()->setGraceNote(ch, pitch(), NoteType::GRACE32_AFTER, MScore::division/8);
                              break;
                        case IconType::SBEAM:
                        case IconType::MBEAM:
                        case IconType::NBEAM:
                        case IconType::BEAM32:
                        case IconType::BEAM64:
                        case IconType::AUTOBEAM:
                              return ch->drop(data);
                              break;
                        case IconType::BRACKETS:
                              addBracket();
                              break;
                        default:
                              break;
                        }
                  }
                  delete e;
                  break;

            case Element::Type::BAGPIPE_EMBELLISHMENT:
                  {
                  BagpipeEmbellishment* b = static_cast<BagpipeEmbellishment*>(e);
                  noteList nl = b->getNoteList();
                  // add grace notes in reverse order, as setGraceNote adds a grace note
                  // before the current note
                  for (int i = nl.size() - 1; i >= 0; --i) {
                        int p = BagpipeEmbellishment::BagpipeNoteInfoList[nl.at(i)].pitch;
                        score()->setGraceNote(ch, p, NoteType::GRACE32, MScore::division/8);
                        }
                  }
                  delete e;
                  break;

            case Element::Type::NOTE:
                  {
                  Chord* ch = chord();
                  if (ch->noteType() != NoteType::NORMAL) {
                        delete e;
                        return 0;
                        }
                  // calculate correct transposed tpc
                  Note* n = toNote(e);
                  Interval v = part()->instrument(ch->tick())->transpose();
                  v.flip();
                  n->setTpc2(Ms::transposeTpc(n->tpc1(), v, true));
                  // replace this note with new note
                  n->setParent(ch);
                  score()->undoRemoveElement(this);
                  score()->undoAddElement(n);
                  }
                  break;

            case Element::Type::GLISSANDO:
                  {
                  for (auto e : _spannerFor) {
                        if (e->type() == Element::Type::GLISSANDO) {
                              qDebug("there is already a glissando");
                              delete e;
                              return 0;
                              }
                        }

                  // this is the glissando initial note, look for a suitable final note
                  Note* finalNote = Glissando::guessFinalNote(chord());
                  if (finalNote != nullptr) {
                        // init glissando data
                        Glissando* gliss = toGlissando(e);
                        gliss->setAnchor(Spanner::Anchor::NOTE);
                        gliss->setStartElement(this);
                        gliss->setEndElement(finalNote);
                        gliss->setTick(ch->tick());
                        gliss->setTick2(finalNote->chord()->tick());
                        gliss->setTrack(track());
                        gliss->setTrack2(finalNote->track());
                        // in TAB, use straight line with no text
                        if (staff()->isTabStaff()) {
                              gliss->setGlissandoType(Glissando::Type::STRAIGHT);
                              gliss->setShowText(false);
                              }
                        gliss->setParent(this);
                        score()->undoAddElement(e);
                        }
                  else {
                        qDebug("no segment for second note of glissando found");
                        delete e;
                        return 0;
                        }
                  }
                  break;

            case Element::Type::CHORD:
                  {
                  Chord* c      = toChord(e);
                  Note* n       = c->upNote();
                  Direction dir = c->stemDirection();
                  int t         = (staff2track(staffIdx()) + n->voice());
                  score()->select(0, SelectType::SINGLE, 0);
                  NoteVal nval;
                  nval.pitch = n->pitch();
                  nval.headGroup = n->headGroup();
                  Segment* seg = score()->setNoteRest(chord()->segment(), t, nval,
                     score()->inputState().duration().fraction(), dir);
                  ChordRest* cr = toChordRest(seg->element(t));
                  if (cr)
                        score()->nextInputPos(cr, true);
                  delete e;
                  }
                  break;

            default:
                  return ch->drop(data);
            }
      return 0;
      }

//---------------------------------------------------------
//   addBracket
//---------------------------------------------------------

void Note::addBracket()
      {
      Symbol* s = new Symbol(score());
      s->setSym(SymId::noteheadParenthesisLeft);
      s->setParent(this);
      score()->undoAddElement(s);
      s = new Symbol(score());
      s->setSym(SymId::noteheadParenthesisRight);
      s->setParent(this);
      score()->undoAddElement(s);
      }

//---------------------------------------------------------
//   setDotY
//---------------------------------------------------------

void Note::setDotY(Direction pos)
      {
      bool onLine = false;
      qreal y = 0;

      if (staff()->isTabStaff()) {
            // with TAB's, dotPosX is not set:
            // get dot X from width of fret text and use TAB default spacing
            StaffType* tab = staff()->staffType();
            if (tab->stemThrough() ) {
                  // if fret mark on lines, use standard processing
                  if (tab->onLines())
                        onLine = true;
                  else
                        // if fret marks above lines, raise the dots by half line distance
                        y = -0.5;
                  }
            // if stems beside staff, do nothing
            else
                  return;
            }
      else
            onLine = !(line() & 1);

      bool oddVoice = voice() & 1;
      if (onLine) {
            // displace dots by half spatium up or down according to voice
            if (pos == Direction::AUTO)
                  y = oddVoice ? 0.5 : -0.5;
            else if (pos == Direction::UP)
                  y = -0.5;
            else
                  y = 0.5;
            }
      else {
            if (pos == Direction::UP && !oddVoice)
                  y -= 1.0;
            else if (pos == Direction::DOWN && oddVoice)
                  y += 1.0;
            }
      y *= spatium() * staff()->lineDistance();

      // apply to dots

      int cdots = chord()->dots();
      int ndots = _dots.size();

      int n = cdots - ndots;
      for (int i = 0; i < n; ++i) {
            NoteDot* dot = new NoteDot(score());
            dot->setParent(this);
            dot->setTrack(track());  // needed to know the staff it belongs to (and detect tablature)
            dot->setVisible(visible());
            score()->undoAddElement(dot);
            }
      if (n < 0) {
            for (int i = 0; i < -n; ++i)
                  score()->undoRemoveElement(_dots.back());
            }
      for (NoteDot* dot : _dots) {
            dot->layout();
            dot->rypos() = y;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Note::layout()
      {
      bool useTablature = staff() && staff()->isTabStaff();
      if (useTablature) {
            StaffType* tab = staff()->staffType();
            qreal mags = magS();
            qreal w = tabHeadWidth(tab);
            bbox().setRect(0.0, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags);
            }
      else {
            SymId nh = noteHead();
            _cachedNoteheadSym = nh;
            if (isNoteName()) {
                  _cachedSymNull = SymId::noteEmptyBlack;
                  NoteHead::Type ht = _headType == NoteHead::Type::HEAD_AUTO ? chord()->durationType().headType() : _headType;
                  if (ht == NoteHead::Type::HEAD_WHOLE)
                        _cachedSymNull = SymId::noteEmptyWhole;
                  else if (ht == NoteHead::Type::HEAD_HALF)
                        _cachedSymNull = SymId::noteEmptyHalf;
                  }
            else
                  _cachedSymNull = SymId::noSym;
            setbbox(symBbox(nh));
            if (parent() == 0)
                  return;
            }
      }

//---------------------------------------------------------
//   layout2
//    called after final position of note is set
//---------------------------------------------------------

void Note::layout2()
      {
      // for standard staves this is done in Score::layoutChords3()
      // so that the results are available there
      if (staff()->isTabStaff())
            adjustReadPos();

      int dots = chord()->dots();
      if (dots) {
            qreal d  = score()->point(score()->styleS(StyleIdx::dotNoteDistance)) * mag();
            qreal dd = score()->point(score()->styleS(StyleIdx::dotDotDistance)) * mag();
            qreal x  = chord()->dotPosX() - pos().x() - chord()->pos().x();
            // if TAB and stems through staff
            if (staff()->isTabStaff()) {
                  StaffType* tab = staff()->staffType();
                  if (tab->stemThrough()) {
                        // with TAB's, dot Y is not calculated during layoutChords3(),
                        // as layoutChords3() is not even called for TAB's;
                        // setDotY() actually also manages creation/deletion of NoteDot's
                        setDotY(Direction::AUTO);

                        // use TAB default note-to-dot spacing
                        dd = STAFFTYPE_TAB_DEFAULTDOTDIST_X * spatium();
                        d = dd * 0.5;
                        }
                  }
            // apply to dots
            qreal xx = x + d;
            for (NoteDot* dot : _dots) {
                  dot->rxpos() = xx;
                  dot->adjustReadPos();
                  xx += dd;
                  }
            }

      // layout elements attached to note
      for (Element* e : _el) {
            if (!score()->tagIsValid(e->tag()))
                  continue;
            e->setMag(mag());
            if (e->isSymbol()) {
                  qreal w = headWidth();
                  Symbol* sym = toSymbol(e);
                  QPointF rp = e->readPos();
                  e->layout();
                  if (sym->sym() == SymId::noteheadParenthesisRight) {
                        if (staff()->isTabStaff()) {
                              StaffType* tab = staff()->staffType();
                              w = tabHeadWidth(tab);
                              }
                        e->rxpos() += w;
                        }
                  else if (sym->sym() == SymId::noteheadParenthesisLeft) {
                        e->rxpos() -= symWidth(SymId::noteheadParenthesisLeft);
                        }
                  if (sym->sym() == SymId::noteheadParenthesisLeft || sym->sym() == SymId::noteheadParenthesisRight) {
                        // adjustReadPos() was called too early in layout(), adjust:
                        if (!rp.isNull()) {
                              e->setUserOff(QPointF());
                              e->setReadPos(rp);
                              e->adjustReadPos();
                              }
                        }
                  }
            else
                  e->layout();
            }
      }

//---------------------------------------------------------
//   dotIsUp
//---------------------------------------------------------

bool Note::dotIsUp() const
      {
      if (_dots.empty())
            return true;
      if (_userDotPosition == Direction::AUTO)
            return _dots[0]->y() < spatium() * .1;
      else
            return (_userDotPosition == Direction::UP);
      }

//---------------------------------------------------------
//   updateAccidental
//    set _accidental and _line depending on tpc
//---------------------------------------------------------

void Note::updateAccidental(AccidentalState* as)
      {
      int relLine = absStep(tpc(), epitch());

      // don't touch accidentals that don't concern tpc such as
      // quarter tones
      if (!(_accidental && Accidental::isMicrotonal(_accidental->accidentalType()))) {
            // calculate accidental
            AccidentalType acci = AccidentalType::NONE;

            AccidentalVal accVal = tpc2alter(tpc());
            bool error = false;
            AccidentalVal relLineAccVal = as->accidentalVal(relLine, error);
            if (error)
                  return;
            if ((accVal != relLineAccVal) || hidden() || as->tieContext(relLine)) {
                  as->setAccidentalVal(relLine, accVal, _tieBack != 0);
                  acci = Accidental::value2subtype(accVal);
                  // if previous tied note has same tpc, don't show accidental
                  if (_tieBack && _tieBack->startNote()->tpc1() == tpc1())
                        acci = AccidentalType::NONE;
                  else if (acci == AccidentalType::NONE)
                        acci = AccidentalType::NATURAL;
                  }
            if (acci != AccidentalType::NONE && !_hidden) {
                  if (_accidental == 0) {
                        Accidental* a = new Accidental(score());
                        a->setParent(this);
                        a->setAccidentalType(acci);
                        score()->undoAddElement(a);
                        }
                  else if (_accidental->accidentalType() != acci) {
                        Accidental* a = _accidental->clone();
                        a->setParent(this);
                        a->setAccidentalType(acci);
                        score()->undoChangeElement(_accidental, a);
                        }
                  }
            else {
                  if (_accidental) {
                        // remove this if it was AUTO:
                        if (_accidental->role() == AccidentalRole::AUTO)
                              score()->undoRemoveElement(_accidental);
                        else {
                              // keep it, but update type if needed
                              acci = Accidental::value2subtype(accVal);
                              if (acci == AccidentalType::NONE)
                                    acci = AccidentalType::NATURAL;
                              if (_accidental->accidentalType() != acci) {
                                    Accidental* a = _accidental->clone();
                                    a->setParent(this);
                                    a->setAccidentalType(acci);
                                    score()->undoChangeElement(_accidental, a);
                                    }
                              }
                        }
                  }
            }

      else {
            // microtonal accidentals playback as naturals
            // in 1.X, they had no effect on accidental state of measure
            // ultimetely, they should probably get their own state
            // for now, at least change state to natural, so subsequent notes playback as might be expected
            // this is an incompatible change, but better to break it for 2.0 than wait until later
            as->setAccidentalVal(relLine, AccidentalVal::NATURAL, _tieBack != 0);
            }

      updateRelLine(relLine, true);
      }

//---------------------------------------------------------
//   noteType
//---------------------------------------------------------

NoteType Note::noteType() const
      {
      return chord()->noteType();
      }

//---------------------------------------------------------
//   noteTypeUserName
//---------------------------------------------------------

QString Note::noteTypeUserName() const
      {
      switch (noteType()) {
            case NoteType::ACCIACCATURA:
                  return tr("Acciaccatura");
            case NoteType::APPOGGIATURA:
                  return tr("Appoggiatura");
            case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                  return tr("Grace note after");
            case NoteType::GRACE4:
            case NoteType::GRACE16:
            case NoteType::GRACE32:
                  return tr("Grace note before");
            default:
                  return tr("Note");
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Note::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      // tie segments are collected from System
      //      if (_tieFor && !staff()->isTabStaff())  // no ties in tablature
      //            _tieFor->scanElements(data, func, all);
      for (Element* e : _el) {
            if (score()->tagIsValid(e->tag()))
                  e->scanElements(data, func, all);
            }
      for (Spanner* sp : _spannerFor)
            sp->scanElements(data, func, all);

      if (!dragMode && _accidental)
            func(data, _accidental);
      for (NoteDot* dot : _dots)
            func(data, dot);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Note::setTrack(int val)
      {
      Element::setTrack(val);
      if (_tieFor) {
            _tieFor->setTrack(val);
            for (SpannerSegment* seg : _tieFor->spannerSegments())
                  seg->setTrack(val);
            }
      for (Spanner* s : _spannerFor) {
            s->setTrack(val);
            }
      for (Spanner* s : _spannerBack) {
            s->setTrack2(val);
            }
      for (Element* e : _el)
            e->setTrack(val);
      if (_accidental)
            _accidental->setTrack(val);
      if (!chord())     // if note is dragged with shift+ctrl
            return;
      for (NoteDot* dot : _dots)
            dot->setTrack(val);
      }

//---------------------------------------------------------
//    reset
//---------------------------------------------------------

void Note::reset()
      {
      undoChangeProperty(P_ID::USER_OFF, QPointF());
      chord()->undoChangeProperty(P_ID::USER_OFF, QPointF());
      chord()->undoChangeProperty(P_ID::STEM_DIRECTION, Direction(Direction::AUTO));
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Note::mag() const
      {
      qreal m = chord()->mag();
      if (_small)
            m *= score()->styleD(StyleIdx::smallNoteMag);
      return m;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Note::setSmall(bool val)
      {
      _small = val;
      }

//---------------------------------------------------------
//   line
//---------------------------------------------------------

int Note::line() const
      {
      if (_fixed)
            return _fixedLine;
      else
            return _line + _lineOffset;
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void Note::setLine(int n)
      {
      _line = n;
      int off = staff() ? staff()->staffType()->stepOffset() : 0;
      rypos() = (_line + off) * spatium() * .5;
      }

//---------------------------------------------------------
//   physicalLine
//---------------------------------------------------------

int Note::physicalLine() const
      {
      int l = line();
      Staff *st = staff();
      if (st && !st->scaleNotesToLines())
            return l * (st->logicalLineDistance() / st->lineDistance());
      else
            return l;
      }

//---------------------------------------------------------
//   setString
//---------------------------------------------------------

void Note::setString(int val)
      {
      _string = val;
      rypos() = _string * spatium() * 1.5;
      }

//---------------------------------------------------------
//   setHeadGroup
//---------------------------------------------------------

void Note::setHeadGroup(NoteHead::Group val)
      {
      Q_ASSERT(int(val) >= 0 && int(val) < int(NoteHead::Group::HEAD_GROUPS));
      _headGroup = val;
      }

//---------------------------------------------------------
//   ppitch
//    playback pitch
//---------------------------------------------------------

int Note::ppitch() const
      {
      return _pitch + staff()->pitchOffset(chord()->segment()->tick());
      }

//---------------------------------------------------------
//   epitch
//    effective pitch
//    honours transposing instruments
//---------------------------------------------------------

int Note::epitch() const
      {
      return _pitch - (concertPitch() ? 0 : transposition());
      }

//---------------------------------------------------------
//   customizeVelocity
//    Input is the global velocity determined by dynamic
//    signs and crescende/decrescendo etc.
//    Returns the actual play velocity for this note
//    modified by veloOffset
//---------------------------------------------------------

int Note::customizeVelocity(int velo) const
      {
      if (veloType() == ValueType::OFFSET_VAL)
            velo = velo + (velo * veloOffset()) / 100;
      else if (veloType() == ValueType::USER_VAL)
            velo = veloOffset();
      return limit(velo, 1, 127);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Note::endEdit()
      {
      Chord* ch = chord();
      if (ch->notes().size() == 1) {
            ch->undoChangeProperty(P_ID::USER_OFF, ch->userOff() + userOff());
            setUserOff(QPointF());
            triggerLayout();
            }
      }

//---------------------------------------------------------
//   updateRelLine
//    calculate the real note line depending on clef,
//    _line is the absolute line
//---------------------------------------------------------

void Note::updateRelLine(int relLine, bool undoable)
      {
      if (staff() && chord()->staffMove()) {
            // check that destination staff makes sense (might have been deleted)
            int idx = staffIdx() + chord()->staffMove();
            int minStaff = part()->startTrack() / VOICES;
            int maxStaff = part()->endTrack() / VOICES;
            if (idx < minStaff || idx >= maxStaff || score()->staff(idx)->staffGroup() != staff()->staffGroup())
                  chord()->undoChangeProperty(P_ID::STAFF_MOVE, 0);
            }

      Staff* s      = score()->staff(staffIdx() + chord()->staffMove());
      ClefType clef = s->clef(chord()->tick());
      int line      = relStep(relLine, clef);

      if (line != _line) {
            if (undoable && _line != INVALID_LINE)
                  undoChangeProperty(P_ID::LINE, line);
            else
                  setLine(line);
            }
      }

//---------------------------------------------------------
//   updateLine
//---------------------------------------------------------

void Note::updateLine()
      {
      int relLine = absStep(tpc(), epitch());
      updateRelLine(relLine, false);
      }

//---------------------------------------------------------
//   setNval
//    set note properties from NoteVal
//---------------------------------------------------------

void Note::setNval(const NoteVal& nval, int tick)
      {
      setPitch(nval.pitch);
      _fret   = nval.fret;
      _string = nval.string;

      _tpc[0] = nval.tpc1;
      _tpc[1] = nval.tpc2;

      if (tick == -1 && chord())
            tick = chord()->tick();
      Interval v = part()->instrument(tick)->transpose();
      if (nval.tpc1 == Tpc::TPC_INVALID) {
            Key key = staff()->key(tick);
            if (!concertPitch() && !v.isZero())
                  key = transposeKey(key, v);
            _tpc[0] = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
            }
      if (nval.tpc2 == Tpc::TPC_INVALID) {
            if (v.isZero())
                  _tpc[1] = _tpc[0];
            else {
                  v.flip();
                  _tpc[1] = Ms::transposeTpc(_tpc[0], v, true);
                  }
            }

      _headGroup = NoteHead::Group(nval.headGroup);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Note::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::PITCH:
                  return pitch();
            case P_ID::TPC1:
                  return _tpc[0];
            case P_ID::TPC2:
                  return _tpc[1];
            case P_ID::SMALL:
                  return small();
            case P_ID::MIRROR_HEAD:
                  return int(userMirror());
            case P_ID::DOT_POSITION:
                  return userDotPosition();
            case P_ID::HEAD_GROUP:
                  return int(headGroup());
            case P_ID::VELO_OFFSET:
                  return veloOffset();
            case P_ID::TUNING:
                  return tuning();
            case P_ID::FRET:
                  return fret();
            case P_ID::STRING:
                  return string();
            case P_ID::GHOST:
                  return ghost();
            case P_ID::HEAD_TYPE:
                  return int(headType());
            case P_ID::VELO_TYPE:
                  return int(veloType());
            case P_ID::PLAY:
                  return play();
            case P_ID::LINE:
                  return _line;
            case P_ID::FIXED:
                  return fixed();
            case P_ID::FIXED_LINE:
                  return fixedLine();
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Note::setProperty(P_ID propertyId, const QVariant& v)
      {
      Measure* m = chord() ? chord()->measure() : nullptr;
      switch(propertyId) {
            case P_ID::PITCH:
                  setPitch(v.toInt());
                  score()->setPlaylistDirty();
                  break;
            case P_ID::TPC1:
                  _tpc[0] = v.toInt();
                  break;
            case P_ID::TPC2:
                  _tpc[1] = v.toInt();
                  break;
            case P_ID::LINE:
                  setLine(v.toInt());
                  break;
            case P_ID::SMALL:
                  setSmall(v.toBool());
                  break;
            case P_ID::MIRROR_HEAD:
                  setUserMirror(MScore::DirectionH(v.toInt()));
                  break;
            case P_ID::DOT_POSITION:
                  setUserDotPosition(v.value<Direction>());
                  score()->setLayout(tick());
                  return true;
            case P_ID::HEAD_GROUP:
                  setHeadGroup(NoteHead::Group(v.toInt()));
                  break;
            case P_ID::VELO_OFFSET:
                  setVeloOffset(v.toInt());
                  score()->setPlaylistDirty();
                  break;
            case P_ID::TUNING:
                  setTuning(v.toDouble());
                  score()->setPlaylistDirty();
                  break;
            case P_ID::FRET:
                  setFret(v.toInt());
                  break;
            case P_ID::STRING:
                  setString(v.toInt());
                  break;
            case P_ID::GHOST:
                  setGhost(v.toBool());
                  break;
            case P_ID::HEAD_TYPE:
                  setHeadType(NoteHead::Type(v.toInt()));
                  break;
            case P_ID::VELO_TYPE:
                  setVeloType(ValueType(v.toInt()));
                  score()->setPlaylistDirty();
                  break;
            case P_ID::VISIBLE: {                     // P_ID::VISIBLE requires reflecting property on dots
                  setVisible(v.toBool());
                  int dots = chord()->dots();
                  for (int i = 0; i < dots; ++i) {
                        if (_dots[i])
                              _dots[i]->setVisible(visible());
                        }
                  if (m)
                        m->checkMultiVoices(chord()->staffIdx());
                  break;
                  }
            case P_ID::PLAY:
                  setPlay(v.toBool());
                  score()->setPlaylistDirty();
                  break;
            case P_ID::FIXED:
                  setFixed(v.toBool());
                  break;
            case P_ID::FIXED_LINE:
                  setFixedLine(v.toInt());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayout(tick());
      return true;
      }

//---------------------------------------------------------
//   undoSetFret
//---------------------------------------------------------

void Note::undoSetFret(int val)
      {
      undoChangeProperty(P_ID::FRET, val);
      }

//---------------------------------------------------------
//   undoSetString
//---------------------------------------------------------

void Note::undoSetString(int val)
      {
      undoChangeProperty(P_ID::STRING, val);
      }

//---------------------------------------------------------
//   undoSetGhost
//---------------------------------------------------------

void Note::undoSetGhost(bool val)
      {
      undoChangeProperty(P_ID::GHOST, val);
      }

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void Note::undoSetSmall(bool val)
      {
      undoChangeProperty(P_ID::SMALL, val);
      }

//---------------------------------------------------------
//   undoSetPlay
//---------------------------------------------------------

void Note::undoSetPlay(bool val)
      {
      undoChangeProperty(P_ID::PLAY, val);
      }

//---------------------------------------------------------
//   undoSetTuning
//---------------------------------------------------------

void Note::undoSetTuning(qreal val)
      {
      undoChangeProperty(P_ID::TUNING, val);
      }

//---------------------------------------------------------
//   undoSetVeloType
//---------------------------------------------------------

void Note::undoSetVeloType(ValueType val)
      {
      undoChangeProperty(P_ID::VELO_TYPE, int(val));
      }

//---------------------------------------------------------
//   undoSetVeloOffset
//---------------------------------------------------------

void Note::undoSetVeloOffset(int val)
      {
      undoChangeProperty(P_ID::VELO_OFFSET, val);
      }

//---------------------------------------------------------
//   undoSetUserMirror
//---------------------------------------------------------

void Note::undoSetUserMirror(MScore::DirectionH val)
      {
      undoChangeProperty(P_ID::MIRROR_HEAD, int(val));
      }

//---------------------------------------------------------
//   undoSetUserDotPosition
//---------------------------------------------------------

void Note::undoSetUserDotPosition(Direction val)
      {
      undoChangeProperty(P_ID::DOT_POSITION, val);
      }

//---------------------------------------------------------
//   undoSetHeadGroup
//---------------------------------------------------------

void Note::undoSetHeadGroup(NoteHead::Group val)
      {
      undoChangeProperty(P_ID::HEAD_GROUP, int(val));
      }

//---------------------------------------------------------
//   setHeadType
//---------------------------------------------------------

void Note::setHeadType(NoteHead::Type t)
      {
      _headType = t;
      }

//---------------------------------------------------------
//   undoSetHeadType
//---------------------------------------------------------

void Note::undoSetHeadType(NoteHead::Type val)
      {
      undoChangeProperty(P_ID::HEAD_TYPE, int(val));
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Note::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::GHOST:
            case P_ID::SMALL:
                  return false;
            case P_ID::MIRROR_HEAD:
                  return int(MScore::DirectionH::AUTO);
            case P_ID::DOT_POSITION:
                  return Direction(Direction::AUTO);
            case P_ID::HEAD_GROUP:
                  return int(NoteHead::Group::HEAD_NORMAL);
            case P_ID::VELO_OFFSET:
                  return 0;
            case P_ID::TUNING:
                  return 0.0;
            case P_ID::FRET:
            case P_ID::STRING:
                  return -1;
            case P_ID::HEAD_TYPE:
                  return int(NoteHead::Type::HEAD_AUTO);
            case P_ID::VELO_TYPE:
                  return int (ValueType::OFFSET_VAL);
            case P_ID::PLAY:
                  return true;
            case P_ID::FIXED:
                  return false;
            case P_ID::FIXED_LINE:
                  return 0;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   setOnTimeOffset
//---------------------------------------------------------

void Note::setOnTimeOffset(int val)
      {
      _playEvents[0].setOntime(val);
      chord()->setPlayEventType(PlayEventType::User);
      }

//---------------------------------------------------------
//   setOffTimeOffset
//---------------------------------------------------------

void Note::setOffTimeOffset(int val)
      {
      _playEvents[0].setLen(val - _playEvents[0].ontime());
      chord()->setPlayEventType(PlayEventType::User);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Note::setScore(Score* s)
      {
      Element::setScore(s);
      if (_tieFor)
            _tieFor->setScore(s);
      if (_accidental)
            _accidental->setScore(s);
      for (Element* el : _el)
            el->setScore(s);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Note::accessibleInfo() const
      {
      QString duration = chord()->durationUserName();
      QString voice = tr("Voice: %1").arg(QString::number(track() % VOICES + 1));
      QString pitchName;
      const Drumset* drumset = part()->instrument()->drumset();
      if (fixed() && headGroup() == NoteHead::Group::HEAD_SLASH)
            pitchName = chord()->noStem() ? tr("Beat Slash") : tr("Rhythm Slash");
      else if (staff()->isDrumStaff() && drumset)
            pitchName = qApp->translate("drumset", drumset->name(pitch()).toUtf8().constData());
      else
            pitchName = tpcUserName(false);
      return tr("%1; Pitch: %2; Duration: %3%4").arg(noteTypeUserName()).arg(pitchName).arg(duration).arg((chord()->isGrace() ? "" : QString("; %1").arg(voice)));
      }

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString Note::screenReaderInfo() const
      {
      QString duration = chord()->durationUserName();
      QString voice = tr("Voice: %1").arg(QString::number(track() % VOICES + 1));
      QString pitchName;
      const Drumset* drumset = part()->instrument()->drumset();
      if (fixed() && headGroup() == NoteHead::Group::HEAD_SLASH)
            pitchName = chord()->noStem() ? tr("Beat Slash") : tr("Rhythm Slash");
      else if (staff()->isDrumStaff() && drumset)
            pitchName = qApp->translate("drumset", drumset->name(pitch()).toUtf8().constData());
      else
            pitchName = tpcUserName(true);
      return QString("%1 %2 %3%4").arg(noteTypeUserName()).arg(pitchName).arg(duration).arg((chord()->isGrace() ? "" : QString("; %1").arg(voice)));
      }

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString Note::accessibleExtraInfo() const
      {
      QString rez = "";
      if (accidental()) {
            rez = QString("%1 %2").arg(rez).arg(accidental()->screenReaderInfo());
            }
      if (!el().empty()) {
            for (Element* e : el()) {
                  if (!score()->selectionFilter().canSelect(e)) continue;
                  rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                  }
            }
      if (tieFor())
            rez = tr("%1 Start of %2").arg(rez).arg(tieFor()->screenReaderInfo());

      if (tieBack())
            rez = tr("%1 End of %2").arg(rez).arg(tieBack()->screenReaderInfo());

      if (!spannerFor().empty()) {
            for (Spanner* s : spannerFor()) {
                  if (!score()->selectionFilter().canSelect(s))
                        continue;
                  rez = tr("%1 Start of %2").arg(rez).arg(s->screenReaderInfo());
                  }
            }
      if (!spannerBack().empty()) {
            for (Spanner* s : spannerBack()) {
                  if (!score()->selectionFilter().canSelect(s))
                        continue;
                  rez = tr("%1 End of %2").arg(rez).arg(s->screenReaderInfo());
                  }
            }

      rez = QString("%1 %2").arg(rez).arg(chord()->accessibleExtraInfo());
      return rez;
      }

//---------------------------------------------------------
//   noteVal
//---------------------------------------------------------

NoteVal Note::noteVal() const
      {
      NoteVal nval;
      nval.pitch     = pitch();
      nval.tpc1      = tpc1();
      nval.tpc2      = tpc2();
      nval.fret      = fret();
      nval.string    = string();
      nval.headGroup = headGroup();
      return nval;
      }

//---------------------------------------------------------
//   qmlDotsCount
//    returns number of dots for plugins
//---------------------------------------------------------

int Note::qmlDotsCount()
      {
      return _dots.size();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Note::subtypeName() const
      {
      return NoteHead::group2userName(_headGroup);
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Note::nextElement()
      {
      if (chord()->isGrace())
            return Element::nextElement();

      const std::vector<Note*>& notes = chord()->notes();
      if (this == notes.front())
            return chord()->nextElement();
      auto i = std::find(notes.begin(), notes.end(), this);
      return *(i-1);
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Note::prevElement()
      {
      if (chord()->isGrace())
            return Element::prevElement();

      const std::vector<Note*>& notes = chord()->notes();
      if (this == notes.back())
            return chord()->prevElement();
      auto i = std::find(notes.begin(), notes.end(), this);
      return *++i;
      }

//---------------------------------------------------------
//   lastTiedNote
//---------------------------------------------------------

Note* Note::lastTiedNote() const
      {
      std::vector<Note*> notes;
      Note* note = const_cast<Note*>(this);
      notes.push_back(note);
      while (note->tieFor()) {
            if (std::find(notes.begin(), notes.end(), note->tieFor()->endNote()) != notes.end())
                  break;
            note = note->tieFor()->endNote();
            notes.push_back(note);
            }
      return note;
      }

//---------------------------------------------------------
//   firstTiedNote
//    if note has ties, return last note in chain
//    - handle recursion in connected notes
//---------------------------------------------------------

Note* Note::firstTiedNote() const
      {
      std::vector<const Note*> notes;
      const Note* note = this;
      notes.push_back(note);
      while (note->tieBack()) {
            if (std::find(notes.begin(), notes.end(), note->tieBack()->startNote()) != notes.end())
                  break;
            note = note->tieBack()->startNote();
            notes.push_back(note);
            }
      return const_cast<Note*>(note);
      }

//---------------------------------------------------------
//   tiedNotes
//---------------------------------------------------------

std::vector<Note*> Note::tiedNotes() const
      {
      std::vector<Note*> notes;
      Note* note = firstTiedNote();

      notes.push_back(note);
      while (note->tieFor()) {
            if (std::find(notes.begin(), notes.end(), note->tieFor()->endNote()) != notes.end())
                  break;
            note = note->tieFor()->endNote();
            notes.push_back(note);
            }
      return notes;
      }

//---------------------------------------------------------
//   accidentalType
//---------------------------------------------------------

AccidentalType Note::accidentalType() const
      {
      return _accidental ? _accidental->accidentalType() : AccidentalType::NONE;
      }

//---------------------------------------------------------
//   setAccidentalType
//---------------------------------------------------------

void Note::setAccidentalType(AccidentalType type)
      {
      if (score())
      	score()->changeAccidental(this, type);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Note::shape() const
      {
      Shape shape;
      shape.add(symBbox(noteHead()));
      for (NoteDot* dot : _dots)
            shape.add(symBbox(SymId::augmentationDot).translated(dot->pos()));
      if (_accidental)
            shape.add(_accidental->bbox().translated(_accidental->pos()));
      return shape.translated(pos());
      }

}
