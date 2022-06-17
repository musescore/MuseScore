/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 \file
 Implementation of classes Note and ShadowNote.
*/

#include "note.h"

#include <assert.h>
#include <QtMath>

#include "translation.h"
#include "draw/brush.h"
#include "rw/xml.h"
#include "accessibility/accessiblenote.h"
#include "types/typesconv.h"

#include "accidental.h"
#include "actionicon.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bagpembell.h"
#include "bend.h"
#include "chord.h"
#include "clef.h"
#include "drumset.h"
#include "factory.h"
#include "fingering.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "image.h"
#include "linkedobjects.h"
#include "measure.h"
#include "mscoreview.h"
#include "notedot.h"
#include "page.h"
#include "part.h"
#include "pitchspelling.h"
#include "score.h"
#include "scorefont.h"
#include "segment.h"
#include "slur.h"
#include "spanner.h"
#include "staff.h"
#include "stafftype.h"
#include "stringdata.h"
#include "system.h"
#include "text.h"
#include "textline.h"
#include "tie.h"
#include "tremolo.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "hook.h"

#include "config.h"
#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   noteHeads
//    notehead groups
//---------------------------------------------------------

//int(NoteHeadGroup::HEAD_GROUPS) - 1: "-1" is needed to prevent building CUSTOM_GROUP noteheads set, since it is built by users and keep a specific set of existing noteheads
static const SymId noteHeads[2][int(NoteHeadGroup::HEAD_GROUPS) - 1][int(NoteHeadType::HEAD_TYPES)] = {
    {    // down stem
        { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,
          SymId::noteheadDoubleWhole },
        { SymId::noteheadXWhole,              SymId::noteheadXHalf,               SymId::noteheadXBlack,
          SymId::noteheadXDoubleWhole },
        { SymId::noteheadPlusWhole,           SymId::noteheadPlusHalf,            SymId::noteheadPlusBlack,
          SymId::noteheadPlusDoubleWhole },
        { SymId::noteheadCircleXWhole,        SymId::noteheadCircleXHalf,         SymId::noteheadCircleX,
          SymId::noteheadCircleXDoubleWhole },
        { SymId::noteheadWholeWithX,          SymId::noteheadHalfWithX,           SymId::noteheadVoidWithX,
          SymId::noteheadDoubleWholeWithX },
        { SymId::noteheadTriangleUpWhole,     SymId::noteheadTriangleUpHalf,      SymId::noteheadTriangleUpBlack,
          SymId::noteheadTriangleUpDoubleWhole },
        { SymId::noteheadTriangleDownWhole,   SymId::noteheadTriangleDownHalf,    SymId::noteheadTriangleDownBlack,
          SymId::noteheadTriangleDownDoubleWhole },
        { SymId::noteheadSlashedWhole1,       SymId::noteheadSlashedHalf1,        SymId::noteheadSlashedBlack1,
          SymId::noteheadSlashedDoubleWhole1 },
        { SymId::noteheadSlashedWhole2,       SymId::noteheadSlashedHalf2,        SymId::noteheadSlashedBlack2,
          SymId::noteheadSlashedDoubleWhole2 },
        { SymId::noteheadDiamondWhole,        SymId::noteheadDiamondHalf,         SymId::noteheadDiamondBlack,
          SymId::noteheadDiamondDoubleWhole },
        { SymId::noteheadDiamondWholeOld,     SymId::noteheadDiamondHalfOld,      SymId::noteheadDiamondBlackOld,
          SymId::noteheadDiamondDoubleWholeOld },
        { SymId::noteheadCircledWhole,        SymId::noteheadCircledHalf,         SymId::noteheadCircledBlack,
          SymId::noteheadCircledDoubleWhole },
        { SymId::noteheadCircledWholeLarge,   SymId::noteheadCircledHalfLarge,    SymId::noteheadCircledBlackLarge,
          SymId::noteheadCircledDoubleWholeLarge },
        { SymId::noteheadLargeArrowUpWhole,   SymId::noteheadLargeArrowUpHalf,    SymId::noteheadLargeArrowUpBlack,
          SymId::noteheadLargeArrowUpDoubleWhole },
        { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,
          SymId::noteheadDoubleWholeSquare },

        { SymId::noteheadSlashWhiteWhole,     SymId::noteheadSlashWhiteHalf,      SymId::noteheadSlashHorizontalEnds,
          SymId::noteheadSlashWhiteWhole },

        { SymId::noteShapeRoundWhite,         SymId::noteShapeRoundWhite,         SymId::noteShapeRoundBlack,
          SymId::noteShapeRoundDoubleWhole },
        { SymId::noteShapeSquareWhite,        SymId::noteShapeSquareWhite,        SymId::noteShapeSquareBlack,
          SymId::noteShapeSquareDoubleWhole },
        { SymId::noteShapeTriangleRightWhite, SymId::noteShapeTriangleRightWhite, SymId::noteShapeTriangleRightBlack,
          SymId::noteShapeTriangleRightDoubleWhole },
        { SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondBlack,
          SymId::noteShapeDiamondDoubleWhole },
        { SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpBlack,
          SymId::noteShapeTriangleUpDoubleWhole },
        { SymId::noteShapeMoonWhite,          SymId::noteShapeMoonWhite,          SymId::noteShapeMoonBlack,
          SymId::noteShapeMoonDoubleWhole },
        { SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundBlack,
          SymId::noteShapeTriangleRoundDoubleWhole },

        { SymId::noteheadHeavyX,              SymId::noteheadHeavyX,              SymId::noteheadHeavyX,
          SymId::noteheadHeavyX },
        { SymId::noteheadHeavyXHat,           SymId::noteheadHeavyXHat,           SymId::noteheadHeavyXHat,
          SymId::noteheadHeavyXHat },

        { SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneBlack,
          SymId::noteShapeKeystoneDoubleWhole },
        { SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonBlack,
          SymId::noteShapeQuarterMoonDoubleWhole },
        { SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleBlack,
          SymId::noteShapeIsoscelesTriangleDoubleWhole },
        { SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftBlack,
          SymId::noteShapeMoonLeftDoubleWhole },
        { SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftBlack,
          SymId::noteShapeArrowheadLeftDoubleWhole },
        { SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftBlack,
          SymId::noteShapeTriangleRoundLeftDoubleWhole },

        { SymId::noteDoWhole,  SymId::noteDoHalf,  SymId::noteDoBlack,  SymId::noSym },
        { SymId::noteDiWhole,  SymId::noteDiHalf,  SymId::noteDiBlack,  SymId::noSym },
        { SymId::noteRaWhole,  SymId::noteRaHalf,  SymId::noteRaBlack,  SymId::noSym },
        { SymId::noteReWhole,  SymId::noteReHalf,  SymId::noteReBlack,  SymId::noSym },
        { SymId::noteRiWhole,  SymId::noteRiHalf,  SymId::noteRiBlack,  SymId::noSym },
        { SymId::noteMeWhole,  SymId::noteMeHalf,  SymId::noteMeBlack,  SymId::noSym },
        { SymId::noteMiWhole,  SymId::noteMiHalf,  SymId::noteMiBlack,  SymId::noSym },
        { SymId::noteFaWhole,  SymId::noteFaHalf,  SymId::noteFaBlack,  SymId::noSym },
        { SymId::noteFiWhole,  SymId::noteFiHalf,  SymId::noteFiBlack,  SymId::noSym },
        { SymId::noteSeWhole,  SymId::noteSeHalf,  SymId::noteSeBlack,  SymId::noSym },
        { SymId::noteSoWhole,  SymId::noteSoHalf,  SymId::noteSoBlack,  SymId::noSym },
        { SymId::noteLeWhole,  SymId::noteLeHalf,  SymId::noteLeBlack,  SymId::noSym },
        { SymId::noteLaWhole,  SymId::noteLaHalf,  SymId::noteLaBlack,  SymId::noSym },
        { SymId::noteLiWhole,  SymId::noteLiHalf,  SymId::noteLiBlack,  SymId::noSym },
        { SymId::noteTeWhole,  SymId::noteTeHalf,  SymId::noteTeBlack,  SymId::noSym },
        { SymId::noteTiWhole,  SymId::noteTiHalf,  SymId::noteTiBlack,  SymId::noSym },
        { SymId::noteSiWhole,  SymId::noteSiHalf,  SymId::noteSiBlack,  SymId::noSym },

        { SymId::noteASharpWhole,  SymId::noteASharpHalf,  SymId::noteASharpBlack,  SymId::noSym },
        { SymId::noteAWhole,       SymId::noteAHalf,       SymId::noteABlack,       SymId::noSym },
        { SymId::noteAFlatWhole,   SymId::noteAFlatHalf,   SymId::noteAFlatBlack,   SymId::noSym },
        { SymId::noteBSharpWhole,  SymId::noteBSharpHalf,  SymId::noteBSharpBlack,  SymId::noSym },
        { SymId::noteBWhole,       SymId::noteBHalf,       SymId::noteBBlack,       SymId::noSym },
        { SymId::noteBFlatWhole,   SymId::noteBFlatHalf,   SymId::noteBFlatBlack,   SymId::noSym },
        { SymId::noteCSharpWhole,  SymId::noteCSharpHalf,  SymId::noteCSharpBlack,  SymId::noSym },
        { SymId::noteCWhole,       SymId::noteCHalf,       SymId::noteCBlack,       SymId::noSym },
        { SymId::noteCFlatWhole,   SymId::noteCFlatHalf,   SymId::noteCFlatBlack,   SymId::noSym },
        { SymId::noteDSharpWhole,  SymId::noteDSharpHalf,  SymId::noteDSharpBlack,  SymId::noSym },
        { SymId::noteDWhole,       SymId::noteDHalf,       SymId::noteDBlack,       SymId::noSym },
        { SymId::noteDFlatWhole,   SymId::noteDFlatHalf,   SymId::noteDFlatBlack,   SymId::noSym },
        { SymId::noteESharpWhole,  SymId::noteESharpHalf,  SymId::noteESharpBlack,  SymId::noSym },
        { SymId::noteEWhole,       SymId::noteEHalf,       SymId::noteEBlack,       SymId::noSym },
        { SymId::noteEFlatWhole,   SymId::noteEFlatHalf,   SymId::noteEFlatBlack,   SymId::noSym },
        { SymId::noteFSharpWhole,  SymId::noteFSharpHalf,  SymId::noteFSharpBlack,  SymId::noSym },
        { SymId::noteFWhole,       SymId::noteFHalf,       SymId::noteFBlack,       SymId::noSym },
        { SymId::noteFFlatWhole,   SymId::noteFFlatHalf,   SymId::noteFFlatBlack,   SymId::noSym },
        { SymId::noteGSharpWhole,  SymId::noteGSharpHalf,  SymId::noteGSharpBlack,  SymId::noSym },
        { SymId::noteGWhole,       SymId::noteGHalf,       SymId::noteGBlack,       SymId::noSym },
        { SymId::noteGFlatWhole,   SymId::noteGFlatHalf,   SymId::noteGFlatBlack,   SymId::noSym },
        { SymId::noteHWhole,       SymId::noteHHalf,       SymId::noteHBlack,       SymId::noSym },
        { SymId::noteHSharpWhole,  SymId::noteHSharpHalf,  SymId::noteHSharpBlack,  SymId::noSym }
    },
    {    // up stem
        { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,
          SymId::noteheadDoubleWhole },
        { SymId::noteheadXWhole,              SymId::noteheadXHalf,               SymId::noteheadXBlack,
          SymId::noteheadXDoubleWhole },
        { SymId::noteheadPlusWhole,           SymId::noteheadPlusHalf,            SymId::noteheadPlusBlack,
          SymId::noteheadPlusDoubleWhole },
        { SymId::noteheadCircleXWhole,        SymId::noteheadCircleXHalf,         SymId::noteheadCircleX,
          SymId::noteheadCircleXDoubleWhole },
        { SymId::noteheadWholeWithX,          SymId::noteheadHalfWithX,           SymId::noteheadVoidWithX,
          SymId::noteheadDoubleWholeWithX },
        { SymId::noteheadTriangleUpWhole,     SymId::noteheadTriangleUpHalf,      SymId::noteheadTriangleUpBlack,
          SymId::noteheadTriangleUpDoubleWhole },
        { SymId::noteheadTriangleDownWhole,   SymId::noteheadTriangleDownHalf,    SymId::noteheadTriangleDownBlack,
          SymId::noteheadTriangleDownDoubleWhole },
        { SymId::noteheadSlashedWhole1,       SymId::noteheadSlashedHalf1,        SymId::noteheadSlashedBlack1,
          SymId::noteheadSlashedDoubleWhole1 },
        { SymId::noteheadSlashedWhole2,       SymId::noteheadSlashedHalf2,        SymId::noteheadSlashedBlack2,
          SymId::noteheadSlashedDoubleWhole2 },
        { SymId::noteheadDiamondWhole,        SymId::noteheadDiamondHalf,         SymId::noteheadDiamondBlack,
          SymId::noteheadDiamondDoubleWhole },
        { SymId::noteheadDiamondWholeOld,     SymId::noteheadDiamondHalfOld,      SymId::noteheadDiamondBlackOld,
          SymId::noteheadDiamondDoubleWholeOld },
        { SymId::noteheadCircledWhole,        SymId::noteheadCircledHalf,         SymId::noteheadCircledBlack,
          SymId::noteheadCircledDoubleWhole },
        { SymId::noteheadCircledWholeLarge,   SymId::noteheadCircledHalfLarge,    SymId::noteheadCircledBlackLarge,
          SymId::noteheadCircledDoubleWholeLarge },
        // different from down, find source?
        { SymId::noteheadLargeArrowDownWhole, SymId::noteheadLargeArrowDownHalf,  SymId::noteheadLargeArrowDownBlack,
          SymId::noteheadLargeArrowDownDoubleWhole },
        { SymId::noteheadWhole,               SymId::noteheadHalf,                SymId::noteheadBlack,
          SymId::noteheadDoubleWholeSquare },

        { SymId::noteheadSlashWhiteWhole,     SymId::noteheadSlashWhiteHalf,      SymId::noteheadSlashHorizontalEnds,
          SymId::noteheadSlashWhiteDoubleWhole },

        { SymId::noteShapeRoundWhite,         SymId::noteShapeRoundWhite,         SymId::noteShapeRoundBlack,
          SymId::noteShapeRoundDoubleWhole },
        { SymId::noteShapeSquareWhite,        SymId::noteShapeSquareWhite,        SymId::noteShapeSquareBlack,
          SymId::noteShapeSquareDoubleWhole },
        // different from down
        { SymId::noteShapeTriangleLeftWhite,  SymId::noteShapeTriangleLeftWhite,  SymId::noteShapeTriangleLeftBlack,
          SymId::noteShapeTriangleLeftDoubleWhole },
        { SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondWhite,       SymId::noteShapeDiamondBlack,
          SymId::noteShapeDiamondDoubleWhole },
        { SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpWhite,    SymId::noteShapeTriangleUpBlack,
          SymId::noteShapeTriangleUpDoubleWhole },
        { SymId::noteShapeMoonWhite,          SymId::noteShapeMoonWhite,          SymId::noteShapeMoonBlack,
          SymId::noteShapeMoonDoubleWhole },
        { SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundWhite, SymId::noteShapeTriangleRoundBlack,
          SymId::noteShapeTriangleRoundDoubleWhole },

        { SymId::noteheadHeavyX,              SymId::noteheadHeavyX,              SymId::noteheadHeavyX,
          SymId::noteheadHeavyX },
        { SymId::noteheadHeavyXHat,           SymId::noteheadHeavyXHat,           SymId::noteheadHeavyXHat,
          SymId::noteheadHeavyXHat },

        { SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneWhite,          SymId::noteShapeKeystoneBlack,
          SymId::noteShapeKeystoneDoubleWhole },
        { SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonWhite,       SymId::noteShapeQuarterMoonBlack,
          SymId::noteShapeQuarterMoonDoubleWhole },
        { SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleWhite, SymId::noteShapeIsoscelesTriangleBlack,
          SymId::noteShapeIsoscelesTriangleDoubleWhole },
        { SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftWhite,          SymId::noteShapeMoonLeftBlack,
          SymId::noteShapeMoonLeftDoubleWhole },
        { SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftWhite,     SymId::noteShapeArrowheadLeftBlack,
          SymId::noteShapeArrowheadLeftDoubleWhole },
        { SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftWhite, SymId::noteShapeTriangleRoundLeftBlack,
          SymId::noteShapeTriangleRoundLeftDoubleWhole },

        { SymId::noteDoWhole,  SymId::noteDoHalf,  SymId::noteDoBlack,  SymId::noSym },
        { SymId::noteDiWhole,  SymId::noteDiHalf,  SymId::noteDiBlack,  SymId::noSym },
        { SymId::noteRaWhole,  SymId::noteRaHalf,  SymId::noteRaBlack,  SymId::noSym },
        { SymId::noteReWhole,  SymId::noteReHalf,  SymId::noteReBlack,  SymId::noSym },
        { SymId::noteRiWhole,  SymId::noteRiHalf,  SymId::noteRiBlack,  SymId::noSym },
        { SymId::noteMeWhole,  SymId::noteMeHalf,  SymId::noteMeBlack,  SymId::noSym },
        { SymId::noteMiWhole,  SymId::noteMiHalf,  SymId::noteMiBlack,  SymId::noSym },
        { SymId::noteFaWhole,  SymId::noteFaHalf,  SymId::noteFaBlack,  SymId::noSym },
        { SymId::noteFiWhole,  SymId::noteFiHalf,  SymId::noteFiBlack,  SymId::noSym },
        { SymId::noteSeWhole,  SymId::noteSeHalf,  SymId::noteSeBlack,  SymId::noSym },
        { SymId::noteSoWhole,  SymId::noteSoHalf,  SymId::noteSoBlack,  SymId::noSym },
        { SymId::noteLeWhole,  SymId::noteLeHalf,  SymId::noteLeBlack,  SymId::noSym },
        { SymId::noteLaWhole,  SymId::noteLaHalf,  SymId::noteLaBlack,  SymId::noSym },
        { SymId::noteLiWhole,  SymId::noteLiHalf,  SymId::noteLiBlack,  SymId::noSym },
        { SymId::noteTeWhole,  SymId::noteTeHalf,  SymId::noteTeBlack,  SymId::noSym },
        { SymId::noteTiWhole,  SymId::noteTiHalf,  SymId::noteTiBlack,  SymId::noSym },
        { SymId::noteSiWhole,  SymId::noteSiHalf,  SymId::noteSiBlack,  SymId::noSym },

        { SymId::noteASharpWhole,  SymId::noteASharpHalf,  SymId::noteASharpBlack,  SymId::noSym },
        { SymId::noteAWhole,       SymId::noteAHalf,       SymId::noteABlack,       SymId::noSym },
        { SymId::noteAFlatWhole,   SymId::noteAFlatHalf,   SymId::noteAFlatBlack,   SymId::noSym },
        { SymId::noteBSharpWhole,  SymId::noteBSharpHalf,  SymId::noteBSharpBlack,  SymId::noSym },
        { SymId::noteBWhole,       SymId::noteBHalf,       SymId::noteBBlack,       SymId::noSym },
        { SymId::noteBFlatWhole,   SymId::noteBFlatHalf,   SymId::noteBFlatBlack,   SymId::noSym },
        { SymId::noteCSharpWhole,  SymId::noteCSharpHalf,  SymId::noteCSharpBlack,  SymId::noSym },
        { SymId::noteCWhole,       SymId::noteCHalf,       SymId::noteCBlack,       SymId::noSym },
        { SymId::noteCFlatWhole,   SymId::noteCFlatHalf,   SymId::noteCFlatBlack,   SymId::noSym },
        { SymId::noteDSharpWhole,  SymId::noteDSharpHalf,  SymId::noteDSharpBlack,  SymId::noSym },
        { SymId::noteDWhole,       SymId::noteDHalf,       SymId::noteDBlack,       SymId::noSym },
        { SymId::noteDFlatWhole,   SymId::noteDFlatHalf,   SymId::noteDFlatBlack,   SymId::noSym },
        { SymId::noteESharpWhole,  SymId::noteESharpHalf,  SymId::noteESharpBlack,  SymId::noSym },
        { SymId::noteEWhole,       SymId::noteEHalf,       SymId::noteEBlack,       SymId::noSym },
        { SymId::noteEFlatWhole,   SymId::noteEFlatHalf,   SymId::noteEFlatBlack,   SymId::noSym },
        { SymId::noteFSharpWhole,  SymId::noteFSharpHalf,  SymId::noteFSharpBlack,  SymId::noSym },
        { SymId::noteFWhole,       SymId::noteFHalf,       SymId::noteFBlack,       SymId::noSym },
        { SymId::noteFFlatWhole,   SymId::noteFFlatHalf,   SymId::noteFFlatBlack,   SymId::noSym },
        { SymId::noteGSharpWhole,  SymId::noteGSharpHalf,  SymId::noteGSharpBlack,  SymId::noSym },
        { SymId::noteGWhole,       SymId::noteGHalf,       SymId::noteGBlack,       SymId::noSym },
        { SymId::noteGFlatWhole,   SymId::noteGFlatHalf,   SymId::noteGFlatBlack,   SymId::noSym },
        { SymId::noteHWhole,       SymId::noteHHalf,       SymId::noteHBlack,       SymId::noSym },
        { SymId::noteHSharpWhole,  SymId::noteHSharpHalf,  SymId::noteHSharpBlack,  SymId::noSym }
    }
};

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Note::noteHead(int direction, NoteHeadGroup group, NoteHeadType t)
{
    return noteHeads[direction][int(group)][int(t)];
}

SymId Note::noteHead(int direction, NoteHeadGroup group, NoteHeadType t, int tpc, Key key, NoteHeadScheme scheme)
{
    // shortcut
    if (scheme == NoteHeadScheme::HEAD_NORMAL) {
        return noteHeads[direction][int(group)][int(t)];
    }
    // other schemes
    if (scheme == NoteHeadScheme::HEAD_PITCHNAME || scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN) {
        if (tpc == Tpc::TPC_A) {
            group = NoteHeadGroup::HEAD_A;
        } else if (tpc == Tpc::TPC_B) {
            if (scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN) {
                group = NoteHeadGroup::HEAD_H;
            } else {
                group = NoteHeadGroup::HEAD_B;
            }
        } else if (tpc == Tpc::TPC_C) {
            group = NoteHeadGroup::HEAD_C;
        } else if (tpc == Tpc::TPC_D) {
            group = NoteHeadGroup::HEAD_D;
        } else if (tpc == Tpc::TPC_E) {
            group = NoteHeadGroup::HEAD_E;
        } else if (tpc == Tpc::TPC_F) {
            group = NoteHeadGroup::HEAD_F;
        } else if (tpc == Tpc::TPC_G) {
            group = NoteHeadGroup::HEAD_G;
        } else if (tpc == Tpc::TPC_A_S) {
            group = NoteHeadGroup::HEAD_A_SHARP;
        } else if (tpc == Tpc::TPC_B_S) {
            if (scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN) {
                group = NoteHeadGroup::HEAD_H_SHARP;
            } else {
                group = NoteHeadGroup::HEAD_B_SHARP;
            }
        } else if (tpc == Tpc::TPC_C_S) {
            group = NoteHeadGroup::HEAD_C_SHARP;
        } else if (tpc == Tpc::TPC_D_S) {
            group = NoteHeadGroup::HEAD_D_SHARP;
        } else if (tpc == Tpc::TPC_E_S) {
            group = NoteHeadGroup::HEAD_E_SHARP;
        } else if (tpc == Tpc::TPC_F_S) {
            group = NoteHeadGroup::HEAD_F_SHARP;
        } else if (tpc == Tpc::TPC_G_S) {
            group = NoteHeadGroup::HEAD_G_SHARP;
        } else if (tpc == Tpc::TPC_A_B) {
            group = NoteHeadGroup::HEAD_A_FLAT;
        } else if (tpc == Tpc::TPC_B_B) {
            if (scheme == NoteHeadScheme::HEAD_PITCHNAME_GERMAN) {
                group = NoteHeadGroup::HEAD_B;
            } else {
                group = NoteHeadGroup::HEAD_B_FLAT;
            }
        } else if (tpc == Tpc::TPC_C_B) {
            group = NoteHeadGroup::HEAD_C_FLAT;
        } else if (tpc == Tpc::TPC_D_B) {
            group = NoteHeadGroup::HEAD_D_FLAT;
        } else if (tpc == Tpc::TPC_E_B) {
            group = NoteHeadGroup::HEAD_E_FLAT;
        } else if (tpc == Tpc::TPC_F_B) {
            group = NoteHeadGroup::HEAD_F_FLAT;
        } else if (tpc == Tpc::TPC_G_B) {
            group = NoteHeadGroup::HEAD_G_FLAT;
        }
    } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_4) {
        int degree = tpc2degree(tpc, key);
        switch (degree) {
        case 0:
        case 3:
            group = NoteHeadGroup::HEAD_FA;
            break;
        case 1:
        case 4:
            group = NoteHeadGroup::HEAD_SOL;
            break;
        case 2:
        case 5:
            group = NoteHeadGroup::HEAD_LA;
            break;
        case 6:
            group = NoteHeadGroup::HEAD_MI;
            break;
        }
    } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN
               || scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK
               || scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER) {
        int degree = tpc2degree(tpc, key);
        switch (degree) {
        case 0:
            if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN) {
                group = NoteHeadGroup::HEAD_DO;
            } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK) {
                group = NoteHeadGroup::HEAD_DO_FUNK;
            } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER) {
                group = NoteHeadGroup::HEAD_DO_WALKER;
            }
            break;
        case 1:
            if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN) {
                group = NoteHeadGroup::HEAD_RE;
            } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK) {
                group = NoteHeadGroup::HEAD_RE_FUNK;
            } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER) {
                group = NoteHeadGroup::HEAD_RE_WALKER;
            }
            break;
        case 2:
            group = NoteHeadGroup::HEAD_MI;
            break;
        case 3:
            group = NoteHeadGroup::HEAD_FA;
            break;
        case 4:
            group = NoteHeadGroup::HEAD_SOL;
            break;
        case 5:
            group = NoteHeadGroup::HEAD_LA;
            break;
        case 6:
            if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN) {
                group = NoteHeadGroup::HEAD_TI;
            } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK) {
                group = NoteHeadGroup::HEAD_TI_FUNK;
            } else if (scheme == NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER) {
                group = NoteHeadGroup::HEAD_TI_WALKER;
            }
            break;
        }
    } else if (scheme == NoteHeadScheme::HEAD_SOLFEGE) {
        int degree = tpc2degree(tpc, key);
        int alteration = tpc2alterByKey(tpc, key);
        if (degree == 0 && alteration == 0) {
            group = NoteHeadGroup::HEAD_DO_NAME;
        } else if (degree == 0 && alteration == 1) {
            group = NoteHeadGroup::HEAD_DI_NAME;
        } else if (degree == 1 && alteration == -1) {
            group = NoteHeadGroup::HEAD_RA_NAME;
        } else if (degree == 1 && alteration == 0) {
            group = NoteHeadGroup::HEAD_RE_NAME;
        } else if (degree == 1 && alteration == 1) {
            group = NoteHeadGroup::HEAD_RI_NAME;
        } else if (degree == 2 && alteration == -1) {
            group = NoteHeadGroup::HEAD_ME_NAME;
        } else if (degree == 2 && alteration == 0) {
            group = NoteHeadGroup::HEAD_MI_NAME;
        } else if (degree == 3 && alteration == 0) {
            group = NoteHeadGroup::HEAD_FA_NAME;
        } else if (degree == 3 && alteration == 1) {
            group = NoteHeadGroup::HEAD_FI_NAME;
        } else if (degree == 4 && alteration == -1) {
            group = NoteHeadGroup::HEAD_SE_NAME;
        } else if (degree == 4 && alteration == 0) {
            group = NoteHeadGroup::HEAD_SOL_NAME;
        } else if (degree == 4 && alteration == 1) {
            group = NoteHeadGroup::HEAD_SI_NAME;
        } else if (degree == 5 && alteration == -1) {
            group = NoteHeadGroup::HEAD_LE_NAME;
        } else if (degree == 5 && alteration == 0) {
            group = NoteHeadGroup::HEAD_LA_NAME;
        } else if (degree == 5 && alteration == 1) {
            group = NoteHeadGroup::HEAD_LI_NAME;
        } else if (degree == 6 && alteration == -1) {
            group = NoteHeadGroup::HEAD_TE_NAME;
        } else if (degree == 6 && alteration == 0) {
            group = NoteHeadGroup::HEAD_TI_NAME;
        }
    } else if (scheme == NoteHeadScheme::HEAD_SOLFEGE_FIXED) {
        if (tpc == Tpc::TPC_C) {
            group = NoteHeadGroup::HEAD_DO_NAME;
        } else if (tpc == Tpc::TPC_C_S) {
            group = NoteHeadGroup::HEAD_DI_NAME;
        } else if (tpc == Tpc::TPC_D_B) {
            group = NoteHeadGroup::HEAD_RA_NAME;
        } else if (tpc == Tpc::TPC_D) {
            group = NoteHeadGroup::HEAD_RE_NAME;
        } else if (tpc == Tpc::TPC_D_S) {
            group = NoteHeadGroup::HEAD_RI_NAME;
        } else if (tpc == Tpc::TPC_E_B) {
            group = NoteHeadGroup::HEAD_ME_NAME;
        } else if (tpc == Tpc::TPC_E) {
            group = NoteHeadGroup::HEAD_MI_NAME;
        } else if (tpc == Tpc::TPC_F) {
            group = NoteHeadGroup::HEAD_FA_NAME;
        } else if (tpc == Tpc::TPC_F_S) {
            group = NoteHeadGroup::HEAD_FI_NAME;
        } else if (tpc == Tpc::TPC_G_B) {
            group = NoteHeadGroup::HEAD_SE_NAME;
        } else if (tpc == Tpc::TPC_G) {
            group = NoteHeadGroup::HEAD_SOL_NAME;
        } else if (tpc == Tpc::TPC_G_S) {
            group = NoteHeadGroup::HEAD_SI_NAME;
        } else if (tpc == Tpc::TPC_A_B) {
            group = NoteHeadGroup::HEAD_LE_NAME;
        } else if (tpc == Tpc::TPC_A) {
            group = NoteHeadGroup::HEAD_LA_NAME;
        } else if (tpc == Tpc::TPC_A_S) {
            group = NoteHeadGroup::HEAD_LI_NAME;
        } else if (tpc == Tpc::TPC_B_B) {
            group = NoteHeadGroup::HEAD_TE_NAME;
        } else if (tpc == Tpc::TPC_B) {
            group = NoteHeadGroup::HEAD_TI_NAME;
        }
    }
    return noteHeads[direction][int(group)][int(t)];
}

NoteHead::NoteHead(Note* parent)
    : Symbol(ElementType::NOTEHEAD, parent) {}

//---------------------------------------------------------
//   headGroup
//   used only when dropping a notehead from the palette
//   they are either half note, either double whole
//---------------------------------------------------------

NoteHeadGroup NoteHead::headGroup() const
{
    NoteHeadGroup group = NoteHeadGroup::HEAD_INVALID;
    for (int i = 0; i < int(NoteHeadGroup::HEAD_DO_WALKER); ++i) {
        if (noteHeads[0][i][1] == _sym || noteHeads[0][i][3] == _sym) {
            group = (NoteHeadGroup)i;
            break;
        }
    }
    return group;
}

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Chord* ch)
    : EngravingItem(ElementType::NOTE, ch, ElementFlag::MOVABLE)
{
    _playEvents.push_back(NoteEvent());      // add default play event
    _cachedNoteheadSym = SymId::noSym;
    _cachedSymNull = SymId::noSym;
}

Note::~Note()
{
    delete _accidental;
    qDeleteAll(_el);
    delete _tieFor;
    qDeleteAll(_dots);
    _leftParenthesis = nullptr;
    _rightParenthesis = nullptr;
}

Note::Note(const Note& n, bool link)
    : EngravingItem(n)
{
    if (link) {
        score()->undo(new Link(this, const_cast<Note*>(&n)));
    }
    _subchannel        = n._subchannel;
    _line              = n._line;
    _fret              = n._fret;
    m_harmonicFret     = n.m_harmonicFret;
    m_displayFret      = n.m_displayFret;
    _string            = n._string;
    _fretConflict      = n._fretConflict;
    _ghost             = n._ghost;
    _deadNote          = n._deadNote;
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
    _headScheme        = n._headScheme;
    _headGroup         = n._headGroup;
    _headType          = n._headType;
    _mirror            = n._mirror;
    _userMirror        = n._userMirror;
    m_isSmall          = n.m_isSmall;
    _userDotPosition   = n._userDotPosition;
    _fixed             = n._fixed;
    _fixedLine         = n._fixedLine;
    _accidental        = 0;
    _harmonic          = n._harmonic;
    _cachedNoteheadSym = n._cachedNoteheadSym;
    _cachedSymNull     = n._cachedSymNull;

    if (n._accidental) {
        add(new Accidental(*(n._accidental)));
    }

    // types in _el: SYMBOL, IMAGE, FINGERING, TEXT, BEND
    const Staff* stf = staff();
    bool tabFingering = stf->staffTypeForElement(this)->showTabFingering();
    for (EngravingItem* e : n._el) {
        if (e->isFingering() && staff()->isTabStaff(tick()) && !tabFingering) {      // tablature has no fingering
            continue;
        }
        EngravingItem* ce = e->clone();
        add(ce);
        if (link) {
            score()->undo(new Link(ce, const_cast<EngravingItem*>(e)));
        }
    }

    _playEvents = n._playEvents;

    if (n._tieFor) {
        _tieFor = Factory::copyTie(*n._tieFor);
        _tieFor->setStartNote(this);
        _tieFor->setTick(_tieFor->startNote()->tick());
        _tieFor->setEndNote(0);
    } else {
        _tieFor = 0;
    }
    _tieBack  = 0;
    for (NoteDot* dot : n._dots) {
        add(Factory::copyNoteDot(*dot));
    }
    _mark      = n._mark;

    setDropTarget(false);
}

void Note::setParent(Chord* ch)
{
    EngravingItem::setParent(ch);
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
    Q_ASSERT(pitchIsValid(val));
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
//   tpc1default
//---------------------------------------------------------

int Note::tpc1default(int p) const
{
    Key key = Key::C;
    if (staff() && chord()) {
        Fraction tick = chord()->tick();
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
        Fraction tick = chord()->tick();
        key = staff()->key(tick);
        if (concertPitch()) {
            Interval interval = part()->instrument(tick)->transpose();
            if (!interval.isZero()) {
                key = transposeKey(key, interval);
            }
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
    Fraction tick = chord() ? chord()->tick() : Fraction(-1, 1);
    Interval v = staff() ? part()->instrument(tick)->transpose() : Interval();
    Key key = (staff() && chord()) ? staff()->key(chord()->tick()) : Key::C;
    // convert key to concert pitch
    if (!concertPitch() && !v.isZero()) {
        key = transposeKey(key, v);
    }
    // set concert pitch tpc
    _tpc[0] = pitch2tpc(_pitch, key, Prefer::NEAREST);
    // set transposed tpc
    if (v.isZero()) {
        _tpc[1] = _tpc[0];
    } else {
        v.flip();
        _tpc[1] = mu::engraving::transposeTpc(_tpc[0], v, true);
    }
    Q_ASSERT(tpcIsValid(_tpc[0]));
    Q_ASSERT(tpcIsValid(_tpc[1]));
}

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

void Note::setTpc(int v)
{
    IF_ASSERT_FAILED(tpcIsValid(v)) {
        return;
    }
    _tpc[concertPitchIdx()] = v;
}

//---------------------------------------------------------
//   undoSetTpc
//    change the current tpc
//---------------------------------------------------------

void Note::undoSetTpc(int v)
{
    if (concertPitch()) {
        if (v != tpc1()) {
            undoChangeProperty(Pid::TPC1, v);
        }
    } else {
        if (v != tpc2()) {
            undoChangeProperty(Pid::TPC2, v);
        }
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

String Note::tpcUserName(const int tpc, const int pitch, const bool explicitAccidental)
{
    const String pitchStr = mtrc("engraving",
                                 tpc2name(tpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, explicitAccidental)
                                 .replace(u"b", u"♭").replace(u"#", u"♯").toUtf8().constChar());
    const String octaveStr = String::number(((pitch - static_cast<int>(tpc2alter(tpc))) / PITCH_DELTA_OCTAVE) - 1);

    return pitchStr + (explicitAccidental ? u" " : u"") + octaveStr;
}

//---------------------------------------------------------
//   tpcUserName
//---------------------------------------------------------

String Note::tpcUserName(const bool explicitAccidental) const
{
    String pitchName = tpcUserName(tpc(), epitch() + ottaveCapoFret(), explicitAccidental);

    if (fixed() && headGroup() == NoteHeadGroup::HEAD_SLASH) {
        // see Note::accessibleInfo(), but we return what we have
        return pitchName;
    }
    if (staff()->isDrumStaff(tick()) && part()->instrument()->drumset()) {
        // see Note::accessibleInfo(), but we return what we have
        return pitchName;
    }
    if (staff()->isTabStaff(tick())) {
        // no further translation
        return pitchName;
    }

    String pitchOffset;
    if (tuning() != 0) {
        pitchOffset = QString::asprintf("%+.3f", tuning());
    }

    if (!concertPitch() && transposition()) {
        String soundingPitch = tpcUserName(tpc1(), ppitch(), explicitAccidental);
        return mtrc("engraving", "%1 (sounding as %2%3)").arg(pitchName, soundingPitch, pitchOffset);
    }
    return pitchName + pitchOffset;
}

//---------------------------------------------------------
//   transposeTpc
//    return transposed tpc
//    If in concertPitch mode return tpc for transposed view
//    else return tpc for concert pitch view.
//---------------------------------------------------------

int Note::transposeTpc(int tpc) const
{
    Fraction tick = chord() ? chord()->tick() : Fraction(-1, 1);
    Interval v = part()->instrument(tick)->transpose();
    if (v.isZero()) {
        return tpc;
    }
    if (concertPitch()) {
        v.flip();
        return mu::engraving::transposeTpc(tpc, v, true);
    } else {
        return mu::engraving::transposeTpc(tpc, v, true);
    }
}

int Note::playingTpc() const
{
    if (!concertPitch() && transposition()) {
        return transposeTpc(tpc());
    }

    return tpc();
}

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Note::noteHead() const
{
    int up;
    NoteHeadType ht;
    if (chord()) {
        up = chord()->up();
        ht = chord()->durationType().headType();
    } else {
        up = 1;
        ht = NoteHeadType::HEAD_QUARTER;
    }
    if (_headType != NoteHeadType::HEAD_AUTO) {
        ht = _headType;
    }

    const Staff* st = chord() ? chord()->staff() : nullptr;

    if (_headGroup == NoteHeadGroup::HEAD_CUSTOM) {
        if (st) {
            if (st->staffTypeForElement(chord())->isDrumStaff()) {
                Fraction t = chord()->tick();
                Instrument* inst = st->part()->instrument(t);
                Drumset* d = inst->drumset();
                if (d) {
                    return d->noteHeads(_pitch, ht);
                } else {
                    LOGD("no drumset");
                    return noteHead(up, NoteHeadGroup::HEAD_NORMAL, ht);
                }
            }
        } else {
            return _cachedNoteheadSym;
        }
    }

    Key key = Key::C;
    NoteHeadScheme scheme = _headScheme;
    if (st) {
        Fraction tick = chord()->tick();
        if (tick >= Fraction(0, 1)) {
            key    = st->key(tick);
            if (scheme == NoteHeadScheme::HEAD_AUTO) {
                scheme = st->staffTypeForElement(chord())->noteHeadScheme();
            }
        }
    }
    if (scheme == NoteHeadScheme::HEAD_AUTO) {
        scheme = NoteHeadScheme::HEAD_NORMAL;
    }
    SymId t = noteHead(up, _headGroup, ht, tpc(), key, scheme);
    if (t == SymId::noSym) {
        LOGD("invalid notehead %d/%d", int(_headGroup), int(ht));
        t = noteHead(up, NoteHeadGroup::HEAD_NORMAL, ht);
    }
    return t;
}

//---------------------------------------------------------
//   headWidth
//
//    returns the x of the symbol bbox. It is different from headWidth() because zero point could be different from leftmost bbox position.
//---------------------------------------------------------
qreal Note::bboxRightPos() const
{
    const auto& bbox = score()->scoreFont()->bbox(noteHead(), magS());
    return bbox.right();
}

//---------------------------------------------------------
//   headBodyWidth
//
//    returns the width of the notehead "body". It is actual for slashed noteheads like -O-, where O is body.
//---------------------------------------------------------
qreal Note::headBodyWidth() const
{
    return headWidth() + 2 * bboxXShift();
}

//---------------------------------------------------------
//   outsideTieAttachX
//
//    returns the X-position for tie attachment for this particular notehead
//---------------------------------------------------------
qreal Note::outsideTieAttachX(bool up) const
{
    qreal xo = 0;

    // tab staff notes just use center of bounding box
    if (staffType()->isTabStaff()) {
        return x() + ((width() / 2) * mag());
    }
    // special cases:
    if (_headGroup == NoteHeadGroup::HEAD_SLASH) {
        // the anchors are really close to the stem attach points
        xo = up ? symSmuflAnchor(noteHead(), SmuflAnchorId::stemUpSE).x() : symSmuflAnchor(noteHead(), SmuflAnchorId::stemDownNW).x();
        xo += spatium() * 0.13 * (chord()->up() ? mag() : -mag());
        return x() + xo;
    }
    if (_headGroup == NoteHeadGroup::HEAD_SLASHED1 || _headGroup == NoteHeadGroup::HEAD_SLASHED2) {
        // just use the very center of the notehead
        return x() + ((headBodyWidth() / 2) * mag());
    }
    /* Noteheads do not have optical centers at this time, but here's the code
       to future-proof
    xo = symSmuflAnchor(noteHead(), SmuflAnchorId::opticalCenter).x() * mag();
    if (xo > 0) {
        return x() + xo;
    }
    */
    // try for average of cutouts
    if (up) {
        qreal xNE = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutNE).x();
        qreal xNW = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutNW).x();
        xo = ((xNE + xNW) / 2) * mag();
        if (xNE < xNW) {
            // musejazz is busted
            xo = 0;
        }
    } else {
        qreal xSE = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutSE).x();
        qreal xSW = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutSW).x();
        xo = ((xSE + xSW) / 2) * mag();
        if (xSE < xSW) {
            xo = 0;
        }
    }
    if (xo > 0) {
        return x() + xo;
    }
    // no cutout, not a slash head, default to middle of notehead
    return x() + ((headWidth() / 2) * mag());
}

void Note::updateHeadGroup(const NoteHeadGroup headGroup)
{
    NoteHeadGroup group = headGroup;

    if (group == NoteHeadGroup::HEAD_INVALID) {
        LOGD("unknown notehead");
        group = NoteHeadGroup::HEAD_NORMAL;
    }

    if (group == _headGroup) {
        return;
    }

    if (links()) {
        for (EngravingObject* scoreElement : *links()) {
            scoreElement->undoChangeProperty(Pid::HEAD_GROUP, static_cast<int>(group));

            Note* note = toNote(scoreElement);

            if (note->staff() && note->staff()->isTabStaff(chord()->tick()) && group == NoteHeadGroup::HEAD_CROSS) {
                scoreElement->undoChangeProperty(Pid::GHOST, true);
            }
        }
    } else {
        undoChangeProperty(Pid::HEAD_GROUP, int(group));
    }
}

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the symbol bbox
//    or the width of the string representation of the fret mark
//---------------------------------------------------------

qreal Note::headWidth() const
{
    return symWidth(noteHead());
}

//---------------------------------------------------------
//   bboxXShift
//
//    returns the x shift of the notehead bounding box
//---------------------------------------------------------
qreal Note::bboxXShift() const
{
    const auto& bbox = score()->scoreFont()->bbox(noteHead(), magS());
    return bbox.bottomLeft().x();
}

//---------------------------------------------------------
//   noteheadCenterX
//
//    returns the x coordinate of the notehead center related to the basepoint of the notehead bbox
//---------------------------------------------------------
qreal Note::noteheadCenterX() const
{
    return score()->scoreFont()->width(noteHead(), magS()) / 2 + bboxXShift();
}

//---------------------------------------------------------
//   tabHeadWidth
//---------------------------------------------------------

qreal Note::tabHeadWidth(const StaffType* tab) const
{
    qreal val;
    if (tab && tab->isTabStaff() && _fret != INVALID_FRET_INDEX && _string != INVALID_STRING_INDEX) {
        mu::draw::Font f    = tab->fretFont();
        f.setPointSizeF(tab->fretFontSize());
        val  = mu::draw::FontMetrics::width(f, _fretString) * magS();
    } else {
        val = headWidth();
    }
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

qreal Note::tabHeadHeight(const StaffType* tab) const
{
    if (tab && _fret != INVALID_FRET_INDEX && _string != INVALID_STRING_INDEX) {
        return tab->fretBoxH() * magS();
    }
    return headHeight();
}

//---------------------------------------------------------
//   stemDownNW
//---------------------------------------------------------

PointF Note::stemDownNW() const
{
    return symSmuflAnchor(noteHead(), SmuflAnchorId::stemDownNW);
}

//---------------------------------------------------------
//   stemUpSE
//---------------------------------------------------------

PointF Note::stemUpSE() const
{
    return symSmuflAnchor(noteHead(), SmuflAnchorId::stemUpSE);
}

//---------------------------------------------------------
//   playTicks
///   Return total tick len of tied notes
//---------------------------------------------------------

int Note::playTicks() const
{
    return playTicksFraction().ticks();
}

//---------------------------------------------------------
//   playTicksFraction
///   Return total tick len of tied notes
//---------------------------------------------------------

Fraction Note::playTicksFraction() const
{
    Fraction stick = firstTiedNote()->chord()->tick();
    const Note* note = lastTiedNote();
    return note->chord()->tick() + note->chord()->actualTicks() - stick;
}

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Note::addSpanner(Spanner* l)
{
    EngravingItem* e = l->endElement();
    if (e && e->isNote()) {
        Note* note = toNote(e);
        note->addSpannerBack(l);
        if (l->isGlissando()) {
            note->chord()->setEndsGlissando(true);
        }
    }
    addSpannerFor(l);
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Note::removeSpanner(Spanner* l)
{
    Note* e = toNote(l->endElement());
    if (e && e->isNote()) {
        if (!e->removeSpannerBack(l)) {
            LOGD("Note::removeSpanner(%p): cannot remove spannerBack %s %p", this, l->typeName(), l);
            // abort();
        }
        if (l->isGlissando()) {
            e->chord()->updateEndsGlissando();
        }
    }
    if (!removeSpannerFor(l)) {
        LOGD("Note(%p): cannot remove spannerFor %s %p", this, l->typeName(), l);
        // abort();
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(EngravingItem* e)
{
    if (e->explicitParent() != this) {
        e->setParent(this);
    }
    e->setTrack(track());

    switch (e->type()) {
    case ElementType::NOTEDOT:
        _dots.push_back(toNoteDot(e));
        break;
    case ElementType::FINGERING:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::BEND:
        _el.push_back(e);
        break;
    case ElementType::TIE: {
        Tie* tie = toTie(e);
        tie->setStartNote(this);
        tie->setTick(tie->startNote()->tick());
        tie->setTrack(track());
        setTieFor(tie);
        if (tie->endNote()) {
            tie->endNote()->setTieBack(tie);
        }
    }
    break;
    case ElementType::ACCIDENTAL:
        _accidental = toAccidental(e);
        break;
    case ElementType::TEXTLINE:
    case ElementType::GLISSANDO:
        addSpanner(toSpanner(e));
        break;
    default:
        LOGD("Note::add() not impl. %s", e->typeName());
        break;
    }
    triggerLayout();

    e->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Note::remove(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::NOTEDOT:
        _dots.pop_back();
        break;

    case ElementType::TEXT:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
    case ElementType::BEND:
        if (!_el.remove(e)) {
            LOGD("Note::remove(): cannot find %s", e->typeName());
        }
        break;
    case ElementType::TIE: {
        Tie* tie = toTie(e);
        setTieFor(0);
        if (tie->endNote()) {
            tie->endNote()->setTieBack(0);
        }
    }
    break;

    case ElementType::ACCIDENTAL:
        _accidental = 0;
        break;

    case ElementType::TEXTLINE:
    case ElementType::GLISSANDO:
        removeSpanner(toSpanner(e));
        break;

    default:
        LOGD("Note::remove() not impl. %s", e->typeName());
        break;
    }
    triggerLayout();
    e->removed();
}

//---------------------------------------------------------
//   isNoteName
//---------------------------------------------------------

bool Note::isNoteName() const
{
    if (chord() && chord()->staff()) {
        const Staff* st = staff();
        NoteHeadScheme s = _headScheme;
        if (s == NoteHeadScheme::HEAD_AUTO) {
            s = st->staffTypeForElement(this)->noteHeadScheme();
        }
        return s == NoteHeadScheme::HEAD_PITCHNAME || s == NoteHeadScheme::HEAD_PITCHNAME_GERMAN
               || s == NoteHeadScheme::HEAD_SOLFEGE || s == NoteHeadScheme::HEAD_SOLFEGE_FIXED;
    }
    return false;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (_hidden) {
        return;
    }

    Color c(curColor());
    painter->setPen(c);
    bool tablature = staff() && staff()->isTabStaff(chord()->tick());

    // tablature
    if (tablature) {
        if (m_displayFret == DisplayFretOption::Hide) {
            return;
        }
        const Staff* st = staff();
        const StaffType* tab = st->staffTypeForElement(this);
        if (tieBack() && !tab->showBackTied()) {
            if (chord()->measure() == tieBack()->startNote()->chord()->measure() && el().empty()) {
                // fret should be hidden, so return without drawing it
                return;
            }
        }
        // draw background, if required (to hide a segment of string line or to show a fretting conflict)
        if (!tab->linesThrough() || fretConflict()) {
            qreal d  = spatium() * .1;
            RectF bb = RectF(bbox().x() - d, tab->fretMaskY() * magS(), bbox().width() + 2 * d, tab->fretMaskH() * magS());
            // we do not know which viewer did this draw() call
            // so update all:
            if (!score()->getViewer().empty()) {
                for (MuseScoreView* view : score()->getViewer()) {
                    view->drawBackground(painter, bb);
                }
            } else {
                painter->fillRect(bb, mu::draw::Color::white);
            }

            if (fretConflict() && !score()->printing() && score()->showUnprintable()) {                //on fret conflict, draw on red background
                painter->save();
                painter->setPen(engravingConfiguration()->criticalColor());
                painter->setBrush(engravingConfiguration()->criticalColor());
                painter->drawRect(bb);
                painter->restore();
            }
        }
        mu::draw::Font f(tab->fretFont());
        f.setPointSizeF(f.pointSizeF() * magS() * MScore::pixelRatio);
        painter->setFont(f);
        painter->setPen(c);
        painter->drawText(PointF(bbox().x(), tab->fretFontYOffset()), _fretString);
    }
    // NOT tablature
    else {
        // skip drawing, if second note of a cross-measure value
        if (chord() && chord()->crossMeasure() == CrossMeasure::SECOND) {
            return;
        }
        // warn if pitch extends usable range of instrument
        // by coloring the notehead
        if (chord() && chord()->segment() && staff()
            && !score()->printing() && MScore::warnPitchRange && !staff()->isDrumStaff(chord()->tick())) {
            const Instrument* in = part()->instrument(chord()->tick());
            int i = ppitch();
            if (i < in->minPitchP() || i > in->maxPitchP()) {
                painter->setPen(selected() ? engravingConfiguration()->criticalSelectedColor() : engravingConfiguration()->criticalColor());
            } else if (i < in->minPitchA() || i > in->maxPitchA()) {
                painter->setPen(selected() ? engravingConfiguration()->warningSelectedColor() : engravingConfiguration()->warningColor());
            }
        }
        // draw blank notehead to avoid staff and ledger lines
        if (_cachedSymNull != SymId::noSym) {
            painter->save();
            painter->setPen(mu::draw::Color::white);
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
    xml.startElement(this);
    EngravingItem::writeProperties(xml);

    if (_accidental) {
        _accidental->write(xml);
    }
    _el.write(xml);
    bool write_dots = false;
    for (NoteDot* dot : _dots) {
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != engravingConfiguration()->defaultColor()
            || dot->visible() != visible()) {
            write_dots = true;
            break;
        }
    }
    if (write_dots) {
        for (NoteDot* dot : _dots) {
            dot->write(xml);
        }
    }
    if (_tieFor) {
        _tieFor->writeSpannerStart(xml, this, track());
    }
    if (_tieBack) {
        _tieBack->writeSpannerEnd(xml, this, track());
    }
    if ((chord() == 0 || chord()->playEventType() != PlayEventType::Auto) && !_playEvents.empty()) {
        xml.startElement("Events");
        for (const NoteEvent& e : _playEvents) {
            e.write(xml);
        }
        xml.endElement();
    }
    for (Pid id : { Pid::PITCH, Pid::TPC1, Pid::TPC2, Pid::SMALL, Pid::MIRROR_HEAD, Pid::DOT_POSITION,
                    Pid::HEAD_SCHEME, Pid::HEAD_GROUP, Pid::VELO_OFFSET, Pid::PLAY, Pid::TUNING, Pid::FRET, Pid::STRING,
                    Pid::GHOST, Pid::HEAD_TYPE, Pid::VELO_TYPE, Pid::FIXED, Pid::FIXED_LINE }) {
        writeProperty(xml, id);
    }

    for (Spanner* e : _spannerFor) {
        e->writeSpannerStart(xml, this, track());
    }
    for (Spanner* e : _spannerBack) {
        e->writeSpannerEnd(xml, this, track());
    }

    xml.endElement();
}

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(XmlReader& e)
{
    setTpc1(Tpc::TPC_INVALID);
    setTpc2(Tpc::TPC_INVALID);

    while (e.readNextStartElement()) {
        if (readProperties(e)) {
        } else {
            e.unknown();
        }
    }
    // ensure sane values:
    _pitch = limit(_pitch, 0, 127);

    if (!tpcIsValid(_tpc[0]) && !tpcIsValid(_tpc[1])) {
        Key key = (staff() && chord()) ? staff()->key(chord()->tick()) : Key::C;
        int tpc = pitch2tpc(_pitch, key, Prefer::NEAREST);
        if (concertPitch()) {
            _tpc[0] = tpc;
        } else {
            _tpc[1] = tpc;
        }
    }
    if (!(tpcIsValid(_tpc[0]) && tpcIsValid(_tpc[1]))) {
        Fraction tick = chord() ? chord()->tick() : Fraction(-1, 1);
        Interval v = staff() ? part()->instrument(tick)->transpose() : Interval();
        if (tpcIsValid(_tpc[0])) {
            v.flip();
            if (v.isZero()) {
                _tpc[1] = _tpc[0];
            } else {
                _tpc[1] = mu::engraving::transposeTpc(_tpc[0], v, true);
            }
        } else {
            if (v.isZero()) {
                _tpc[0] = _tpc[1];
            } else {
                _tpc[0] = mu::engraving::transposeTpc(_tpc[1], v, true);
            }
        }
    }

    // check consistency of pitch, tpc1, tpc2, and transposition
    // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
    // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
    // including perhaps some we don't know about yet,
    // we will attempt to fix some problems here regardless of version

    if (staff() && !staff()->isDrumStaff(e.context()->tick()) && !e.context()->pasteMode() && !MScore::testMode) {
        int tpc1Pitch = (tpc2pitch(_tpc[0]) + 12) % 12;
        int tpc2Pitch = (tpc2pitch(_tpc[1]) + 12) % 12;
        int soundingPitch = _pitch % 12;
        if (tpc1Pitch != soundingPitch) {
            LOGD("bad tpc1 - soundingPitch = %d, tpc1 = %d", soundingPitch, tpc1Pitch);
            _pitch += tpc1Pitch - soundingPitch;
        }
        if (staff()) {
            Interval v = staff()->part()->instrument(e.context()->tick())->transpose();
            int writtenPitch = (_pitch - v.chromatic) % 12;
            if (tpc2Pitch != writtenPitch) {
                LOGD("bad tpc2 - writtenPitch = %d, tpc2 = %d", writtenPitch, tpc2Pitch);
                if (concertPitch()) {
                    // assume we want to keep sounding pitch
                    // so fix written pitch (tpc only)
                    v.flip();
                    _tpc[1] = mu::engraving::transposeTpc(_tpc[0], v, true);
                } else {
                    // assume we want to keep written pitch
                    // so fix sounding pitch (both tpc and pitch)
                    _tpc[0] = mu::engraving::transposeTpc(_tpc[1], v, true);
                    _pitch += tpc2Pitch - writtenPitch;
                }
            }
        }
    }

    for (EngravingItem* item : _el) {
        if (!item->isSymbol()) {
            continue;
        }

        Symbol* symbol = toSymbol(item);
        SymId symbolId = symbol->sym();

        if (symbolId == SymId::noteheadParenthesisLeft) {
            _leftParenthesis = symbol;
        } else if (symbolId == SymId::noteheadParenthesisRight) {
            _rightParenthesis = symbol;
        }
    }

    if (_leftParenthesis && _rightParenthesis) {
        _hasHeadParentheses = true;
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Note::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());

    if (tag == "pitch") {
        _pitch = e.readInt();
    } else if (tag == "tpc") {
        _tpc[0] = e.readInt();
        _tpc[1] = _tpc[0];
    } else if (tag == "track") {          // for performance
        setTrack(e.readInt());
    } else if (tag == "Accidental") {
        Accidental* a = Factory::createAccidental(this);
        a->setTrack(track());
        a->read(e);
        add(a);
    } else if (tag == "Spanner") {
        Spanner::readSpanner(e, this, track());
    } else if (tag == "tpc2") {
        _tpc[1] = e.readInt();
    } else if (tag == "small") {
        setSmall(e.readInt());
    } else if (tag == "mirror") {
        readProperty(e, Pid::MIRROR_HEAD);
    } else if (tag == "dotPosition") {
        readProperty(e, Pid::DOT_POSITION);
    } else if (tag == "fixed") {
        setFixed(e.readBool());
    } else if (tag == "fixedLine") {
        setFixedLine(e.readInt());
    } else if (tag == "headScheme") {
        readProperty(e, Pid::HEAD_SCHEME);
    } else if (tag == "head") {
        readProperty(e, Pid::HEAD_GROUP);
    } else if (tag == "velocity") {
        setVeloOffset(e.readInt());
    } else if (tag == "play") {
        setPlay(e.readInt());
    } else if (tag == "tuning") {
        setTuning(e.readDouble());
    } else if (tag == "fret") {
        setFret(e.readInt());
    } else if (tag == "string") {
        setString(e.readInt());
    } else if (tag == "ghost") {
        setGhost(e.readInt());
    } else if (tag == "headType") {
        readProperty(e, Pid::HEAD_TYPE);
    } else if (tag == "veloType") {
        readProperty(e, Pid::VELO_TYPE);
    } else if (tag == "line") {
        setLine(e.readInt());
    } else if (tag == "Fingering") {
        Fingering* f = Factory::createFingering(this);
        f->setTrack(track());
        f->read(e);
        add(f);
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(this);
        s->setTrack(track());
        s->read(e);
        add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(this);
            image->setTrack(track());
            image->read(e);
            add(image);
        }
    } else if (tag == "Bend") {
        Bend* b = Factory::createBend(this);
        b->setTrack(track());
        b->read(e);
        add(b);
    } else if (tag == "NoteDot") {
        NoteDot* dot = Factory::createNoteDot(this);
        dot->read(e);
        add(dot);
    } else if (tag == "Events") {
        _playEvents.clear();        // remove default event
        while (e.readNextStartElement()) {
            const AsciiStringView t(e.name());
            if (t == "Event") {
                NoteEvent ne;
                ne.read(e);
                _playEvents.push_back(ne);
            } else {
                e.unknown();
            }
        }
        if (chord()) {
            chord()->setPlayEventType(PlayEventType::User);
        }
    } else if (tag == "offset") {
        EngravingItem::readProperties(e);
    } else if (EngravingItem::readProperties(e)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   Note::readAddConnector
//---------------------------------------------------------

void Note::readAddConnector(ConnectorInfoReader* info, bool pasteMode)
{
    const ElementType type = info->type();
    const Location& l = info->location();
    switch (type) {
    case ElementType::TIE:
    case ElementType::TEXTLINE:
    case ElementType::GLISSANDO:
    {
        Spanner* sp = toSpanner(info->connector());
        if (info->isStart()) {
            sp->setTrack(l.track());
            sp->setTick(tick());
            if (sp->isTie()) {
                Note* n = this;
                while (n->tieFor()) {
                    n = n->tieFor()->endNote();
                }
                Tie* tie = toTie(sp);
                tie->setParent(n);
                tie->setStartNote(n);
                n->_tieFor = tie;
            } else {
                sp->setAnchor(Spanner::Anchor::NOTE);
                sp->setStartElement(this);
                addSpannerFor(sp);
                sp->setParent(this);
            }
        } else if (info->isEnd()) {
            sp->setTrack2(l.track());
            sp->setTick2(tick());
            sp->setEndElement(this);
            if (sp->isTie()) {
                _tieBack = toTie(sp);
            } else {
                if (sp->isGlissando() && explicitParent() && explicitParent()->isChord()) {
                    toChord(explicitParent())->setEndsGlissando(true);
                }
                addSpannerBack(sp);
            }

            // As spanners get added after being fully read, they
            // do not get cloned with the note when pasting to
            // linked staves. So add this spanner explicitly.
            if (pasteMode) {
                score()->undoAddElement(sp);
            }
        }
    }
    default:
        break;
    }
}

//---------------------------------------------------------
//   transposition
//---------------------------------------------------------

int Note::transposition() const
{
    Fraction tick = chord() ? chord()->tick() : Fraction(-1, 1);
    return staff() ? part()->instrument(tick)->transpose().chromatic : 0;
}

//---------------------------------------------------------
//   NoteEditData
//---------------------------------------------------------

class NoteEditData : public ElementEditData
{
public:
    enum EditMode {
        EditMode_ChangePitch = 0,
        EditMode_AddSpacing,
        EditMode_Undefined,
    };

    int line = 0;
    int string = 0;
    EditMode mode = EditMode_Undefined;
    PointF delta;

    virtual EditDataType type() override { return EditDataType::NoteEditData; }

    static constexpr double MODE_TRANSITION_LIMIT_DEGREES = 15.0;

    static inline EditMode editModeByDragDirection(const qreal& deltaX, const qreal& deltaY)
    {
        qreal x = qAbs(deltaX);
        qreal y = qAbs(deltaY);

        mu::PointF normalizedVector(x, y);

        normalizedVector.normalize();

        float radians = PointF::dotProduct(normalizedVector, PointF(1, 0));

        qreal degrees = (qAcos(radians) * 180.0) / M_PI;

        LOGD() << "NOTE DRAG DEGREES " << degrees;

        if (degrees >= MODE_TRANSITION_LIMIT_DEGREES) {
            return NoteEditData::EditMode_ChangePitch;
        } else {
            return NoteEditData::EditMode_AddSpacing;
        }
    }
};

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;
    ElementType type = e->type();
    if (type == ElementType::GLISSANDO) {
        for (auto ee : _spannerFor) {
            if (ee->isGlissando()) {
                return false;
            }
        }
        return true;
    }
    const Staff* st   = staff();
    bool isTablature  = st->isTabStaff(tick());
    bool tabFingering = st->staffTypeForElement(this)->showTabFingering();
    return type == ElementType::ARTICULATION
           || type == ElementType::FERMATA
           || type == ElementType::CHORDLINE
           || type == ElementType::TEXT
           || type == ElementType::REHEARSAL_MARK
           || (type == ElementType::FINGERING && (!isTablature || tabFingering))
           || type == ElementType::ACCIDENTAL
           || type == ElementType::BREATH
           || type == ElementType::ARPEGGIO
           || type == ElementType::NOTEHEAD
           || type == ElementType::NOTE
           || type == ElementType::TREMOLO
           || type == ElementType::STAFF_STATE
           || type == ElementType::INSTRUMENT_CHANGE
           || type == ElementType::IMAGE
           || type == ElementType::CHORD
           || type == ElementType::HARMONY
           || type == ElementType::DYNAMIC
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::ACCIACCATURA)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::APPOGGIATURA)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::GRACE4)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::GRACE16)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::GRACE32)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::GRACE8_AFTER)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::GRACE16_AFTER)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::GRACE32_AFTER)
           || (noteType() == NoteType::NORMAL && type == ElementType::BAGPIPE_EMBELLISHMENT)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_AUTO)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_NONE)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BREAK_LEFT)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BREAK_INNER_8TH)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BREAK_INNER_16TH)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_JOIN)
           || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::PARENTHESES)
           || (type == ElementType::SYMBOL)
           || (type == ElementType::CLEF)
           || (type == ElementType::KEYSIG)
           || (type == ElementType::TIMESIG)
           || (type == ElementType::BAR_LINE)
           || (type == ElementType::STAFF_TEXT)
           || (type == ElementType::PLAYTECH_ANNOTATION)
           || (type == ElementType::SYSTEM_TEXT)
           || (type == ElementType::TRIPLET_FEEL)
           || (type == ElementType::STICKING)
           || (type == ElementType::TEMPO_TEXT)
           || (type == ElementType::BEND)
           || (type == ElementType::TREMOLOBAR)
           || (type == ElementType::FRET_DIAGRAM)
           || (type == ElementType::FIGURED_BASS)
           || (type == ElementType::LYRICS)
           || (type != ElementType::TIE && e->isSpanner());
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Note::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;

    const Staff* st = staff();
    bool isTablature = st->isTabStaff(tick());
    bool tabFingering = st->staffTypeForElement(this)->showTabFingering();
    Chord* ch = chord();

    switch (e->type()) {
    case ElementType::REHEARSAL_MARK:
        return ch->drop(data);

    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        e->setParent(this);
        score()->undoAddElement(e);
        return e;

    case ElementType::FINGERING:
        if (!isTablature || tabFingering) {
            e->setParent(this);
            score()->undoAddElement(e);
            return e;
        } else {
            delete e;
        }
        return 0;

    case ElementType::LYRICS:
        e->setParent(ch);
        e->setTrack(track());
        score()->undoAddElement(e);
        return e;

    case ElementType::ACCIDENTAL:
        score()->changeAccidental(this, toAccidental(e)->accidentalType());
        break;

    case ElementType::BEND:
        e->setParent(this);
        e->setTrack(track());
        score()->undoAddElement(e);
        return e;

    case ElementType::NOTEHEAD:
    {
        NoteHead* s = toNoteHead(e);
        NoteHeadGroup group = s->headGroup();
        if (group == NoteHeadGroup::HEAD_INVALID) {
            LOGD("unknown notehead");
            group = NoteHeadGroup::HEAD_NORMAL;
        }
        delete s;

        if (group != _headGroup) {
            if (links()) {
                for (EngravingObject* se : *links()) {
                    se->undoChangeProperty(Pid::HEAD_GROUP, int(group));
                    Note* note = toNote(se);
                    if (note->staff() && note->staff()->isTabStaff(ch->tick()) && group == NoteHeadGroup::HEAD_CROSS) {
                        se->undoChangeProperty(Pid::GHOST, true);
                    }
                }
            } else {
                undoChangeProperty(Pid::HEAD_GROUP, int(group));
            }
        }
    }
    break;

    case ElementType::ACTION_ICON:
    {
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::ACCIACCATURA:
            score()->setGraceNote(ch, pitch(), NoteType::ACCIACCATURA, Constants::division / 2);
            break;
        case ActionIconType::APPOGGIATURA:
            score()->setGraceNote(ch, pitch(), NoteType::APPOGGIATURA, Constants::division / 2);
            break;
        case ActionIconType::GRACE4:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE4, Constants::division);
            break;
        case ActionIconType::GRACE16:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE16,  Constants::division / 4);
            break;
        case ActionIconType::GRACE32:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE32, Constants::division / 8);
            break;
        case ActionIconType::GRACE8_AFTER:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE8_AFTER, Constants::division / 2);
            break;
        case ActionIconType::GRACE16_AFTER:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE16_AFTER, Constants::division / 4);
            break;
        case ActionIconType::GRACE32_AFTER:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE32_AFTER, Constants::division / 8);
            break;
        case ActionIconType::BEAM_AUTO:
        case ActionIconType::BEAM_NONE:
        case ActionIconType::BEAM_BREAK_LEFT:
        case ActionIconType::BEAM_BREAK_INNER_8TH:
        case ActionIconType::BEAM_BREAK_INNER_16TH:
        case ActionIconType::BEAM_JOIN:
            return ch->drop(data);
            break;
        case ActionIconType::PARENTHESES:
            setHeadHasParentheses(true);
            break;
        default:
            break;
        }
    }
        delete e;
        break;

    case ElementType::BAGPIPE_EMBELLISHMENT:
    {
        BagpipeEmbellishment* b = toBagpipeEmbellishment(e);
        noteList nl = b->getNoteList();
        // add grace notes in reverse order, as setGraceNote adds a grace note
        // before the current note
        for (int i = static_cast<int>(nl.size()) - 1; i >= 0; --i) {
            int p = BagpipeEmbellishment::BagpipeNoteInfoList[nl.at(i)].pitch;
            score()->setGraceNote(ch, p, NoteType::GRACE32, Constants::division / 8);
        }
    }
        delete e;
        break;

    case ElementType::NOTE:
    {
        // calculate correct transposed tpc
        Note* n = toNote(e);
        Interval v = part()->instrument(ch->tick())->transpose();
        v.flip();
        n->setTpc2(mu::engraving::transposeTpc(n->tpc1(), v, true));
        // replace this note with new note
        n->setParent(ch);
        if (this->tieBack()) {
            n->setTieBack(this->tieBack());
            n->tieBack()->setEndNote(n);
            this->setTieBack(nullptr);
        }
        score()->undoRemoveElement(this);
        score()->undoAddElement(n);
    }
    break;

    case ElementType::GLISSANDO:
    {
        for (auto ee : qAsConst(_spannerFor)) {
            if (ee->type() == ElementType::GLISSANDO) {
                LOGD("there is already a glissando");
                delete e;
                return 0;
            }
        }

        // this is the glissando initial note, look for a suitable final note
        Note* finalNote = Glissando::guessFinalNote(chord());
        if (finalNote) {
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
            if (staff()->isTabStaff(finalNote->chord()->tick())) {
                gliss->setGlissandoType(GlissandoType::STRAIGHT);
                gliss->setShowText(false);
            }
            gliss->setParent(this);
            score()->undoAddElement(e);
        } else {
            LOGD("no segment for second note of glissando found");
            delete e;
            return 0;
        }
    }
    break;

    case ElementType::CHORD:
    {
        Chord* c      = toChord(e);
        Note* n       = c->upNote();
        DirectionV dir = c->stemDirection();
        track_idx_t t = track(); // (staff2track(staffIdx()) + n->voice());
        score()->select(0, SelectType::SINGLE, 0);
        NoteVal nval;
        nval.pitch = n->pitch();
        nval.headGroup = n->headGroup();
        ChordRest* cr = nullptr;
        if (data.modifiers & Qt::ShiftModifier) {
            // add note to chord
            score()->addNote(ch, nval);
        } else {
            // replace current chord
            Segment* seg = score()->setNoteRest(ch->segment(), t, nval,
                                                score()->inputState().duration().fraction(), dir);
            cr = seg ? toChordRest(seg->element(t)) : nullptr;
        }
        if (cr) {
            score()->nextInputPos(cr, false);
        }
        delete e;
    }
    break;

    default:
        Spanner* spanner;
        if (e->isSpanner() && (spanner = toSpanner(e))->anchor() == Spanner::Anchor::NOTE) {
            spanner->setParent(this);
            spanner->setStartElement(this);
            spanner->setTick(tick());
            spanner->setTrack(track());
            spanner->setTrack2(track());
            spanner->computeEndElement();
            score()->undoAddElement(spanner);
            return e;
        }
        return ch->drop(data);
    }
    return 0;
}

void Note::setHeadHasParentheses(bool hasParentheses)
{
    if (hasParentheses == _hasHeadParentheses) {
        return;
    }

    _hasHeadParentheses = hasParentheses;

    if (hasParentheses) {
        if (!_leftParenthesis) {
            _leftParenthesis = new Symbol(this);
            _leftParenthesis->setSym(SymId::noteheadParenthesisLeft);
            _leftParenthesis->setParent(this);
        }

        if (!_rightParenthesis) {
            _rightParenthesis = new Symbol(this);
            _rightParenthesis->setSym(SymId::noteheadParenthesisRight);
            _rightParenthesis->setParent(this);
        }

        score()->undoAddElement(_leftParenthesis);
        score()->undoAddElement(_rightParenthesis);
    } else {
        score()->undoRemoveElement(_leftParenthesis);
        score()->undoRemoveElement(_rightParenthesis);
    }
}

//---------------------------------------------------------
//   setDotY
//---------------------------------------------------------

void Note::setDotY(DirectionV pos)
{
    bool onLine = false;
    qreal y = 0;

    if (staff()->isTabStaff(chord()->tick())) {
        // with TAB's, dotPosX is not set:
        // get dot X from width of fret text and use TAB default spacing
        const Staff* st = staff();
        const StaffType* tab = st->staffTypeForElement(this);
        if (tab->stemThrough()) {
            // if fret mark on lines, use standard processing
            if (tab->onLines()) {
                onLine = true;
            } else {
                // if fret marks above lines, raise the dots by half line distance
                y = -0.5;
            }
        }
        // if stems beside staff, do nothing
        else {
            return;
        }
    } else {
        onLine = !(line() & 1);
    }

    bool oddVoice = voice() & 1;
    if (onLine) {
        // displace dots by half spatium up or down according to voice
        if (pos == DirectionV::AUTO) {
            y = oddVoice ? 0.5 : -0.5;
        } else if (pos == DirectionV::UP) {
            y = -0.5;
        } else {
            y = 0.5;
        }
    } else {
        if (pos == DirectionV::UP && !oddVoice) {
            y -= 1.0;
        } else if (pos == DirectionV::DOWN && oddVoice) {
            y += 1.0;
        }
    }
    y *= spatium() * staff()->lineDistance(tick());

    // apply to dots

    int cdots = static_cast<int>(chord()->dots());
    int ndots = static_cast<int>(_dots.size());

    int n = cdots - ndots;
    for (int i = 0; i < n; ++i) {
        NoteDot* dot = Factory::createNoteDot(this);
        dot->setParent(this);
        dot->setTrack(track());      // needed to know the staff it belongs to (and detect tablature)
        dot->setVisible(visible());
        score()->undoAddElement(dot);
    }
    if (n < 0) {
        for (int i = 0; i < -n; ++i) {
            score()->undoRemoveElement(_dots.back());
        }
    }
    for (NoteDot* dot : qAsConst(_dots)) {
        dot->layout();
        dot->rypos() = y;
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Note::layout()
{
    bool useTablature = staff() && staff()->isTabStaff(chord()->tick());
    if (useTablature) {
        if (m_displayFret == DisplayFretOption::Hide) {
            return;
        }

        const Staff* st = staff();
        const StaffType* tab = st->staffTypeForElement(this);
        qreal mags = magS();
        bool parenthesis = false;
        if (tieBack()) {
            if (chord()->measure() != tieBack()->startNote()->chord()->measure() || !el().empty()) {
                parenthesis = true;
            }
        }
        if (_ghost) {
            parenthesis = true;
        }
        // not complete but we need systems to be laid out to add parenthesis
        if (_fixed) {
            _fretString = "/";
        } else {
            _fretString = tab->fretString(_fret, _string, _deadNote);
            if (m_displayFret == DisplayFretOption::ArtificialHarmonic) {
                _fretString = String("%1 <%2>").arg(_fretString, String::number(m_harmonicFret));
            } else if (m_displayFret == DisplayFretOption::NaturalHarmonic) {
                _fretString = String("<%1>").arg(String::number(m_harmonicFret));
            }
        }
        if (parenthesis) {
            _fretString = String("(%1)").arg(_fretString);
        }
        qreal w = tabHeadWidth(tab);     // !! use _fretString
        bbox().setRect(0.0, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags);
    } else {
        if (_deadNote) {
            setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        } else if (_harmonic) {
            setHeadGroup(NoteHeadGroup::HEAD_DIAMOND);
        }
        SymId nh = noteHead();
        _cachedNoteheadSym = nh;
        if (isNoteName()) {
            _cachedSymNull = SymId::noteEmptyBlack;
            NoteHeadType ht = _headType == NoteHeadType::HEAD_AUTO ? chord()->durationType().headType() : _headType;
            if (ht == NoteHeadType::HEAD_WHOLE) {
                _cachedSymNull = SymId::noteEmptyWhole;
            } else if (ht == NoteHeadType::HEAD_HALF) {
                _cachedSymNull = SymId::noteEmptyHalf;
            }
        } else {
            _cachedSymNull = SymId::noSym;
        }
        setbbox(symBbox(nh));
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

    int dots = chord()->dots();
    if (dots) {
        qreal d  = score()->point(score()->styleS(Sid::dotNoteDistance)) * mag();
        qreal dd = score()->point(score()->styleS(Sid::dotDotDistance)) * mag();
        qreal x  = chord()->dotPosX() - pos().x() - chord()->pos().x();
        // adjust dot distance for hooks
        if (chord()->hook() && chord()->up()) {
            qreal hookRight = chord()->hook()->width() + chord()->hook()->x() + chord()->pos().x();
            qreal hookBottom = chord()->hook()->height() + chord()->hook()->y() + chord()->pos().y() + (0.25 * spatium());
            // the top dot in the chord, not the dot for this particular note:
            qreal dotY = chord()->notes().back()->y() + chord()->notes().back()->dots().front()->pos().y();
            if (chord()->dotPosX() < hookRight && dotY < hookBottom) {
                d = chord()->hook()->width();
            }
        }
        // if TAB and stems through staff
        if (staff()->isTabStaff(chord()->tick())) {
            const Staff* st = staff();
            const StaffType* tab = st->staffTypeForElement(this);
            if (tab->stemThrough()) {
                // with TAB's, dot Y is not calculated during layoutChords3(),
                // as layoutChords3() is not even called for TAB's;
                // setDotY() actually also manages creation/deletion of NoteDot's
                setDotY(DirectionV::AUTO);

                // use TAB default note-to-dot spacing
                dd = STAFFTYPE_TAB_DEFAULTDOTDIST_X * spatium();
                d = dd * 0.5;
            }
        }
        // apply to dots
        qreal xx = x + d;
        for (NoteDot* dot : qAsConst(_dots)) {
            dot->rxpos() = xx;
            xx += dd;
        }
    }

    // layout elements attached to note
    for (EngravingItem* e : _el) {
        if (!score()->tagIsValid(e->tag())) {
            continue;
        }
        if (e->isSymbol()) {
            e->setMag(mag());
            qreal w = headWidth();
            Symbol* sym = toSymbol(e);
            e->layout();
            if (sym->sym() == SymId::noteheadParenthesisRight) {
                if (staff()->isTabStaff(chord()->tick())) {
                    const Staff* st = staff();
                    const StaffType* tab = st->staffTypeForElement(this);
                    w = tabHeadWidth(tab);
                }
                e->rxpos() += w;
            } else if (sym->sym() == SymId::noteheadParenthesisLeft) {
                e->rxpos() -= symWidth(SymId::noteheadParenthesisLeft);
            }
        } else if (e->isFingering()) {
            // don't set mag; fingerings should not scale with note
            Fingering* f = toFingering(e);
            if (f->propertyFlags(Pid::PLACEMENT) == PropertyFlags::STYLED) {
                f->setPlacement(f->calculatePlacement());
            }
            // layout fingerings that are placed relative to notehead
            // fingerings placed relative to chord will be laid out later
            if (f->layoutType() == ElementType::NOTE) {
                f->layout();
            }
        } else {
            e->setMag(mag());
            e->layout();
        }
    }
}

//---------------------------------------------------------
//   dotIsUp
//---------------------------------------------------------

bool Note::dotIsUp() const
{
    if (_dots.empty()) {
        return true;
    }
    if (_userDotPosition == DirectionV::AUTO) {
        return _dots[0]->y() < spatium() * .1;
    } else {
        return _userDotPosition == DirectionV::UP;
    }
}

static bool hasAlteredUnison(Note* note)
{
    const auto& chordNotes = note->chord()->notes();
    AccidentalVal accVal = tpc2alter(note->tpc());
    int relLine = absStep(note->tpc(), note->epitch());
    return std::find_if(chordNotes.begin(), chordNotes.end(), [note, accVal, relLine](Note* n) {
        return n != note && !n->hidden() && absStep(n->tpc(), n->epitch()) == relLine && tpc2alter(n->tpc()) != accVal;
    }) != chordNotes.end();
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
        int eRelLine = absStep(tpc(), epitch() + ottaveCapoFret());
        AccidentalVal relLineAccVal = as->accidentalVal(eRelLine, error);
        if (error) {
            LOGD("error accidentalVal()");
            return;
        }
        if ((accVal != relLineAccVal) || hidden() || as->tieContext(eRelLine)) {
            as->setAccidentalVal(eRelLine, accVal, _tieBack != 0 && _accidental == 0);
            acci = Accidental::value2subtype(accVal);
            // if previous tied note has same tpc, don't show accidental
            if (_tieBack && _tieBack->startNote()->tpc1() == tpc1()) {
                acci = AccidentalType::NONE;
            } else if (acci == AccidentalType::NONE) {
                acci = AccidentalType::NATURAL;
            }
        } else if (hasAlteredUnison(this)) {
            if ((acci = Accidental::value2subtype(accVal)) == AccidentalType::NONE) {
                acci = AccidentalType::NATURAL;
            }
        }
        if (acci != AccidentalType::NONE && !_hidden) {
            if (_accidental == 0) {
                Accidental* a = Factory::createAccidental(this);
                a->setParent(this);
                a->setAccidentalType(acci);
                score()->undoAddElement(a);
            } else if (_accidental->accidentalType() != acci) {
                Accidental* a = _accidental->clone();
                a->setParent(this);
                a->setAccidentalType(acci);
                score()->undoChangeElement(_accidental, a);
            }
        } else {
            if (_accidental) {
                // remove this if it was AUTO:
                if (_accidental->role() == AccidentalRole::AUTO) {
                    score()->undoRemoveElement(_accidental);
                } else {
                    // keep it, but update type if needed
                    acci = Accidental::value2subtype(accVal);
                    if (acci == AccidentalType::NONE) {
                        acci = AccidentalType::NATURAL;
                    }
                    if (_accidental->accidentalType() != acci) {
                        Accidental* a = _accidental->clone();
                        a->setParent(this);
                        a->setAccidentalType(acci);
                        score()->undoChangeElement(_accidental, a);
                    }
                }
            }
        }
    } else {
        // microtonal accidentals playback as naturals
        // in 1.X, they had no effect on accidental state of measure
        // ultimately, they should probably get their own state
        // for now, at least change state to natural, so subsequent notes playback as might be expected
        // this is an incompatible change, but better to break it for 2.0 than wait until later
        AccidentalVal accVal = Accidental::subtype2value(_accidental->accidentalType());
        as->setAccidentalVal(relLine, accVal, _tieBack != 0 && _accidental == 0);
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

String Note::noteTypeUserName() const
{
    switch (noteType()) {
    case NoteType::ACCIACCATURA:
        return QObject::tr("Acciaccatura");
    case NoteType::APPOGGIATURA:
        return QObject::tr("Appoggiatura");
    case NoteType::GRACE8_AFTER:
    case NoteType::GRACE16_AFTER:
    case NoteType::GRACE32_AFTER:
        return QObject::tr("Grace note after");
    case NoteType::GRACE4:
    case NoteType::GRACE16:
    case NoteType::GRACE32:
        return QObject::tr("Grace note before");
    default:
        return QObject::tr("Note");
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Note::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    func(data, this);
    // tie segments are collected from System
    //      if (_tieFor && !staff()->isTabStaff(chord->tick()))  // no ties in tablature
    //            _tieFor->scanElements(data, func, all);
    for (EngravingItem* e : _el) {
        if (score()->tagIsValid(e->tag())) {
            e->scanElements(data, func, all);
        }
    }
    for (Spanner* sp : _spannerFor) {
        sp->scanElements(data, func, all);
    }

    if (!dragMode && _accidental) {
        func(data, _accidental);
    }
    for (NoteDot* dot : _dots) {
        func(data, dot);
    }

    // see above - tie segments are still collected from System!
    // if (_tieFor && !_tieFor->spannerSegments().empty())
    //      _tieFor->spannerSegments().front()->scanElements(data, func, all);
    // if (_tieBack && _tieBack->spannerSegments().size() > 1)
    //      _tieBack->spannerSegments().back()->scanElements(data, func, all);
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Note::setTrack(track_idx_t val)
{
    EngravingItem::setTrack(val);
    if (_tieFor) {
        _tieFor->setTrack(val);
        for (SpannerSegment* seg : _tieFor->spannerSegments()) {
            seg->setTrack(val);
        }
    }
    for (Spanner* s : _spannerFor) {
        s->setTrack(val);
    }
    for (Spanner* s : _spannerBack) {
        s->setTrack2(val);
    }
    for (EngravingItem* e : _el) {
        e->setTrack(val);
    }
    if (_accidental) {
        _accidental->setTrack(val);
    }
    if (!chord()) {     // if note is dragged with shift+ctrl
        return;
    }
    for (NoteDot* dot : _dots) {
        dot->setTrack(val);
    }
}

//---------------------------------------------------------
//    reset
//---------------------------------------------------------

void Note::reset()
{
    undoChangeProperty(Pid::OFFSET, PointF());
    chord()->undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(PointF()));
    chord()->undoChangeProperty(Pid::STEM_DIRECTION, PropertyValue::fromValue<DirectionV>(DirectionV::AUTO));
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Note::mag() const
{
    qreal m = chord() ? chord()->mag() : 1.0;
    if (m_isSmall) {
        m *= score()->styleD(Sid::smallNoteMag);
    }
    return m;
}

//---------------------------------------------------------
//   elementBase
//---------------------------------------------------------
EngravingItem* Note::elementBase() const
{
    return parentItem();
}

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Note::setSmall(bool val)
{
    m_isSmall = val;
}

//---------------------------------------------------------
//   line
//---------------------------------------------------------

int Note::line() const
{
    return fixed() ? _fixedLine : _line;
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
//   setHeadScheme
//---------------------------------------------------------

void Note::setHeadScheme(NoteHeadScheme val)
{
    IF_ASSERT_FAILED(int(val) >= -1 && int(val) < int(NoteHeadScheme::HEAD_SCHEMES)) {
        val = NoteHeadScheme::HEAD_AUTO;
    }
    _headScheme = val;
}

//---------------------------------------------------------
//   setHeadGroup
//---------------------------------------------------------

void Note::setHeadGroup(NoteHeadGroup val)
{
    Q_ASSERT(int(val) >= 0 && int(val) < int(NoteHeadGroup::HEAD_GROUPS));
    _headGroup = val;
}

//---------------------------------------------------------
//   ottaveCapoFret
//    offset added by Ottava's and Capo Fret.
//---------------------------------------------------------

int Note::ottaveCapoFret() const
{
    Chord* ch = chord();
    int capoFretId = staff()->capo(ch->segment()->tick());
    if (capoFretId != 0) {
        capoFretId -= 1;
    }

    return staff()->pitchOffset(ch->segment()->tick()) + capoFretId;
}

//---------------------------------------------------------
//   ppitch
//    playback pitch
//---------------------------------------------------------

int Note::ppitch() const
{
    Chord* ch = chord();
    // if staff is drum
    // match tremolo and articulation between variants and chord
    if (play() && ch && ch->staff() && ch->staff()->isDrumStaff(ch->tick())) {
        const Drumset* ds = ch->staff()->part()->instrument(ch->tick())->drumset();
        if (ds) {
            DrumInstrumentVariant div = ds->findVariant(_pitch, ch->articulations(), ch->tremolo());
            if (div.pitch != INVALID_PITCH) {
                return div.pitch;
            }
        }
    }

    return _pitch + ottaveCapoFret();
}

//---------------------------------------------------------
//   epitch
//    effective pitch, i.e. a pitch which is visible in the
//    currently used written notation.
//    honours transposing instruments
//---------------------------------------------------------

int Note::epitch() const
{
    return _pitch - (concertPitch() ? 0 : transposition());
}

//---------------------------------------------------------
//   octave
//    compute octave number from note information
//---------------------------------------------------------

int Note::octave() const
{
    return ((epitch() + ottaveCapoFret() - static_cast<int>(tpc2alter(tpc()))) / 12) - 1;
}

int Note::playingOctave() const
{
    return ((ppitch() - static_cast<int>(tpc2alter(tpc1()))) / PITCH_DELTA_OCTAVE) - 1;
}

//---------------------------------------------------------
//   customizeVelocity
//    Input is the global velocity determined by dynamic
//    signs and crescendo/decrescendo etc.
//    Returns the actual play velocity for this note
//    modified by veloOffset
//---------------------------------------------------------

int Note::customizeVelocity(int velo) const
{
    if (veloType() == VeloType::OFFSET_VAL) {
        velo = velo + (velo * veloOffset()) / 100;
    } else if (veloType() == VeloType::USER_VAL) {
        velo = veloOffset();
    }
    return limit(velo, 1, 127);
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Note::startDrag(EditData& ed)
{
    std::shared_ptr<NoteEditData> ned = std::make_shared<NoteEditData>();
    ned->e      = this;
    ned->line   = _line;
    ned->string = _string;
    ned->pushProperty(Pid::PITCH);
    ned->pushProperty(Pid::TPC1);
    ned->pushProperty(Pid::TPC2);
    ned->pushProperty(Pid::FRET);
    ned->pushProperty(Pid::STRING);

    ed.addData(ned);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF Note::drag(EditData& ed)
{
    NoteEditData* noteEditData = static_cast<NoteEditData*>(ed.getData(this).get());
    IF_ASSERT_FAILED(noteEditData) {
        return RectF();
    }

    PointF delta = ed.evtDelta;
    noteEditData->delta = delta;

    if (noteEditData->mode == NoteEditData::EditMode_Undefined) {
        noteEditData->mode = NoteEditData::editModeByDragDirection(delta.x(), delta.y());
    }

    if (noteEditData->mode == NoteEditData::EditMode_AddSpacing) {
        horizontalDrag(ed);
    } else if (noteEditData->mode == NoteEditData::EditMode_ChangePitch) {
        verticalDrag(ed);
    }

    return RectF();
}

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Note::endDrag(EditData& ed)
{
    NoteEditData* ned = static_cast<NoteEditData*>(ed.getData(this).get());
    IF_ASSERT_FAILED(ned) {
        return;
    }
    for (Note* nn : tiedNotes()) {
        for (const PropertyData& pd : ned->propertyData) {
            setPropertyFlags(pd.id, pd.f);       // reset initial property flags state
            score()->undoPropertyChanged(nn, pd.id, pd.data);
        }
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Note::editDrag(EditData& editData)
{
    Chord* ch = chord();
    Segment* seg = ch->segment();

    if (editData.modifiers & Qt::ShiftModifier) {
        const Spatium deltaSp = Spatium(editData.delta.x() / spatium());
        seg->undoChangeProperty(Pid::LEADING_SPACE, seg->extraLeadingSpace() + deltaSp);
    } else if (ch->notes().size() == 1) {
        // if the chord contains only this note, then move the whole chord
        // including stem, flag etc.
        ch->undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(ch->offset() + offset() + editData.evtDelta));
        setOffset(PointF());
    } else {
        setOffset(offset() + editData.evtDelta);
    }

    triggerLayout();
}

//---------------------------------------------------------
//   verticalDrag
//---------------------------------------------------------

void Note::verticalDrag(EditData& ed)
{
    Fraction _tick      = chord()->tick();
    const Staff* stf    = staff();
    const StaffType* st = stf->staffType(_tick);
    const Instrument* instr = part()->instrument(_tick);

    if (instr->useDrumset()) {
        return;
    }

    NoteEditData* ned   = static_cast<NoteEditData*>(ed.getData(this).get());

    qreal _spatium      = spatium();
    bool tab            = st->isTabStaff();
    qreal step          = _spatium * (tab ? st->lineDistance().val() : 0.5);
    int lineOffset      = lrint(ed.moveDelta.y() / step);

    if (tab) {
        const StringData* strData = staff()->part()->instrument(_tick)->stringData();
        int nString = ned->string + (st->upsideDown() ? -lineOffset : lineOffset);
        int nFret   = strData->fret(_pitch, nString, staff());

        if (nFret >= 0) {                        // no fret?
            if (fret() != nFret || string() != nString) {
                for (Note* nn : tiedNotes()) {
                    nn->setFret(nFret);
                    nn->setString(nString);
                    strData->fretChords(nn->chord());
                    nn->triggerLayout();
                }
            }
        }
    } else {
        Key key = staff()->key(_tick);
        staff_idx_t idx = chord()->vStaffIdx();
        int newPitch = line2pitch(ned->line + lineOffset, score()->staff(idx)->clef(_tick), key);

        if (!concertPitch()) {
            Interval interval = staff()->part()->instrument(_tick)->transpose();
            newPitch += interval.chromatic;
        }

        int newTpc1 = pitch2tpc(newPitch, key, Prefer::NEAREST);
        int newTpc2 = pitch2tpc(newPitch - transposition(), key, Prefer::NEAREST);
        for (Note* nn : tiedNotes()) {
            nn->setPitch(newPitch, newTpc1, newTpc2);
            nn->triggerLayout();
        }
    }
}

//---------------------------------------------------------
//   normalizeLeftDragDelta
//---------------------------------------------------------

void Note::normalizeLeftDragDelta(Segment* seg, EditData& ed, NoteEditData* ned)
{
    Segment* previous = seg->prev();

    if (previous) {
        qreal minDist = previous->minHorizontalCollidingDistance(seg);

        qreal diff = (ed.pos.x()) - (previous->pageX() + minDist);

        qreal distanceBetweenSegments = (previous->pageX() + minDist) - seg->pageX();

        if (diff < 0) {
            ned->delta.setX(distanceBetweenSegments);
        }
    } else {
        Measure* measure = seg->measure();

        qreal minDist = score()->styleMM(Sid::barNoteDistance);

        qreal diff = (ed.pos.x()) - (measure->pageX() + minDist);

        qreal distanceBetweenSegments = (measure->pageX() + minDist) - seg->pageX();

        if (diff < 0) {
            ned->delta.setX(distanceBetweenSegments);
        }
    }
}

//---------------------------------------------------------
//   horizontalDrag
//---------------------------------------------------------

void Note::horizontalDrag(EditData& ed)
{
    Chord* ch = chord();
    Segment* seg = ch->segment();

    NoteEditData* ned = static_cast<NoteEditData*>(ed.getData(this).get());

    if (ed.moveDelta.x() < 0) {
        normalizeLeftDragDelta(seg, ed, ned);
    }

    const Spatium deltaSp = Spatium(ned->delta.x() / spatium());

    if (seg->extraLeadingSpace() + deltaSp < Spatium(0)) {
        return;
    }

    seg->undoChangeProperty(Pid::LEADING_SPACE, seg->extraLeadingSpace() + deltaSp);

    triggerLayout();
}

//---------------------------------------------------------
//   updateRelLine
//    calculate the real note line depending on clef,
//    _line is the absolute line
//---------------------------------------------------------

void Note::updateRelLine(int relLine, bool undoable)
{
    if (!staff()) {
        return;
    }
    // int idx      = staffIdx() + chord()->staffMove();
    Q_ASSERT(staffIdx() == chord()->staffIdx());
    staff_idx_t idx      = chord()->vStaffIdx();

    const Staff* staff  = score()->staff(idx);
    const StaffType* st = staff->staffTypeForElement(this);

    if (chord()->staffMove()) {
        // check that destination staff makes sense (might have been deleted)
        staff_idx_t minStaff = part()->startTrack() / VOICES;
        staff_idx_t maxStaff = part()->endTrack() / VOICES;
        const Staff* stf = this->staff();
        if (idx < minStaff || idx >= maxStaff || st->group() != stf->staffTypeForElement(this)->group()) {
            LOGD("staffMove out of scope %zu + %d min %zu max %zu",
                 staffIdx(), chord()->staffMove(), minStaff, maxStaff);
            chord()->undoChangeProperty(Pid::STAFF_MOVE, 0);
        }
    }

    ClefType clef = staff->clef(chord()->tick());
    int line      = relStep(relLine, clef);

    if (undoable && (_line != INVALID_LINE) && (line != _line)) {
        undoChangeProperty(Pid::LINE, line);
    } else {
        setLine(line);
    }

    int off  = st->stepOffset();
    qreal ld = st->lineDistance().val();
    rypos()  = (_line + off * 2.0) * spatium() * .5 * ld;
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

void Note::setNval(const NoteVal& nval, Fraction tick)
{
    setPitch(nval.pitch);
    _fret   = nval.fret;
    _string = nval.string;

    _tpc[0] = nval.tpc1;
    _tpc[1] = nval.tpc2;

    if (tick == Fraction(-1, 1) && chord()) {
        tick = chord()->tick();
    }
    Interval v = part()->instrument(tick)->transpose();
    if (nval.tpc1 == Tpc::TPC_INVALID) {
        Key key = staff()->key(tick);
        if (!concertPitch() && !v.isZero()) {
            key = transposeKey(key, v);
        }
        _tpc[0] = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
    }
    if (nval.tpc2 == Tpc::TPC_INVALID) {
        if (v.isZero()) {
            _tpc[1] = _tpc[0];
        } else {
            v.flip();
            _tpc[1] = mu::engraving::transposeTpc(_tpc[0], v, true);
        }
    }

    _headGroup = NoteHeadGroup(nval.headGroup);
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Note::localSpatiumChanged(qreal oldValue, qreal newValue)
{
    EngravingItem::localSpatiumChanged(oldValue, newValue);
    for (EngravingItem* e : dots()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    for (EngravingItem* e : el()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    for (Spanner* spanner : spannerBack()) {
        for (auto k : spanner->spannerSegments()) {
            k->localSpatiumChanged(oldValue, newValue);
        }
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Note::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PITCH:
        return pitch();
    case Pid::TPC1:
        return _tpc[0];
    case Pid::TPC2:
        return _tpc[1];
    case Pid::SMALL:
        return isSmall();
    case Pid::MIRROR_HEAD:
        return userMirror();
    case Pid::HEAD_HAS_PARENTHESES:
        return _hasHeadParentheses;
    case Pid::DOT_POSITION:
        return PropertyValue::fromValue<DirectionV>(userDotPosition());
    case Pid::HEAD_SCHEME:
        return int(headScheme());
    case Pid::HEAD_GROUP:
        return int(headGroup());
    case Pid::VELO_OFFSET:
        return veloOffset();
    case Pid::TUNING:
        return tuning();
    case Pid::FRET:
        return fret();
    case Pid::STRING:
        return string();
    case Pid::GHOST:
        return ghost();
    case Pid::HEAD_TYPE:
        return headType();
    case Pid::VELO_TYPE:
        return veloType();
    case Pid::PLAY:
        return play();
    case Pid::LINE:
        return _line;
    case Pid::FIXED:
        return fixed();
    case Pid::FIXED_LINE:
        return fixedLine();
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Note::setProperty(Pid propertyId, const PropertyValue& v)
{
    Measure* m = chord() ? chord()->measure() : nullptr;
    switch (propertyId) {
    case Pid::PITCH:
        setPitch(v.toInt());
        score()->setPlaylistDirty();
        break;
    case Pid::TPC1:
        _tpc[0] = v.toInt();
        break;
    case Pid::TPC2:
        _tpc[1] = v.toInt();
        break;
    case Pid::LINE:
        setLine(v.toInt());
        break;
    case Pid::SMALL:
        setSmall(v.toBool());
        break;
    case Pid::MIRROR_HEAD:
        setUserMirror(v.value<DirectionH>());
        break;
    case Pid::HEAD_HAS_PARENTHESES:
        setHeadHasParentheses(v.toBool());
        break;
    case Pid::DOT_POSITION:
        setUserDotPosition(v.value<DirectionV>());
        triggerLayout();
        return true;
    case Pid::HEAD_SCHEME:
        setHeadScheme(v.value<NoteHeadScheme>());
        break;
    case Pid::HEAD_GROUP:
        setHeadGroup(v.value<NoteHeadGroup>());
        break;
    case Pid::VELO_OFFSET:
        setVeloOffset(v.toInt());
        score()->setPlaylistDirty();
        break;
    case Pid::TUNING:
        setTuning(v.toDouble());
        score()->setPlaylistDirty();
        break;
    case Pid::FRET:
        setFret(v.toInt());
        break;
    case Pid::STRING:
        setString(v.toInt());
        break;
    case Pid::GHOST:
        setGhost(v.toBool());
        break;
    case Pid::HEAD_TYPE:
        setHeadType(v.value<NoteHeadType>());
        break;
    case Pid::VELO_TYPE:
        setVeloType(v.value<VeloType>());
        score()->setPlaylistDirty();
        break;
    case Pid::VISIBLE: {
        setVisible(v.toBool());
        if (m) {
            m->checkMultiVoices(chord()->staffIdx());
        }
        break;
    }
    case Pid::PLAY:
        setPlay(v.toBool());
        score()->setPlaylistDirty();
        break;
    case Pid::FIXED:
        setFixed(v.toBool());
        break;
    case Pid::FIXED_LINE:
        setFixedLine(v.toInt());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Note::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GHOST:
    case Pid::SMALL:
        return false;
    case Pid::MIRROR_HEAD:
        return DirectionH::AUTO;
    case Pid::HEAD_HAS_PARENTHESES:
        return false;
    case Pid::DOT_POSITION:
        return DirectionV::AUTO;
    case Pid::HEAD_SCHEME:
        return NoteHeadScheme::HEAD_AUTO;
    case Pid::HEAD_GROUP:
        return NoteHeadGroup::HEAD_NORMAL;
    case Pid::VELO_OFFSET:
        return 0;
    case Pid::TUNING:
        return 0.0;
    case Pid::FRET:
    case Pid::STRING:
        return -1;
    case Pid::HEAD_TYPE:
        return NoteHeadType::HEAD_AUTO;
    case Pid::VELO_TYPE:
        return VeloType::OFFSET_VAL;
    case Pid::PLAY:
        return true;
    case Pid::FIXED:
        return false;
    case Pid::FIXED_LINE:
        return 0;
    case Pid::TPC2:
        return getProperty(Pid::TPC1);
    case Pid::PITCH:
    case Pid::TPC1:
        return PropertyValue();
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   setHeadType
//---------------------------------------------------------

void Note::setHeadType(NoteHeadType t)
{
    _headType = t;
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
    EngravingItem::setScore(s);
    if (_tieFor) {
        _tieFor->setScore(s);
    }
    if (_accidental) {
        _accidental->setScore(s);
    }
    for (NoteDot* dot : qAsConst(_dots)) {
        dot->setScore(s);
    }
    for (EngravingItem* el : _el) {
        el->setScore(s);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Note::accessibleInfo() const
{
    if (!chord() || !staff()) {
        return String();
    }

    String duration = chord()->durationUserName();
    String voice = mtrc("engraving", "Voice: %1").arg(track() % VOICES + 1);
    String pitchName;
    String onofftime;
    if (!_playEvents.empty()) {
        int on = _playEvents[0].ontime();
        int off = _playEvents[0].offtime();
        if (on != 0 || off != NoteEvent::NOTE_LENGTH) {
            onofftime = mtrc("engraving", " (on %1‰ off %2‰)").arg(on, off);
        }
    }

    const Drumset* drumset = part()->instrument(chord()->tick())->drumset();
    if (fixed() && headGroup() == NoteHeadGroup::HEAD_SLASH) {
        pitchName = chord()->noStem() ? mtrc("engraving", "Beat slash") : mtrc("engraving", "Rhythm slash");
    } else if (staff()->isDrumStaff(tick()) && drumset) {
        pitchName = mtrc("engraving", drumset->name(pitch()));
    } else if (staff()->isTabStaff(tick())) {
        pitchName = mtrc("engraving", "%1; String: %2; Fret: %3")
                    .arg(tpcUserName(false), String::number(string() + 1), String::number(fret()));
    } else {
        pitchName = tpcUserName(false);
    }

    return mtrc("engraving", "%1; Pitch: %2; Duration: %3%4%5")
           .arg(noteTypeUserName(), pitchName, duration, onofftime, (chord()->isGrace() ? u"" : String("; %1").arg(voice)));
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String Note::screenReaderInfo() const
{
    String duration = chord()->durationUserName();
    Measure* m = chord()->measure();
    bool voices = m ? m->hasVoices(staffIdx()) : false;
    String voice = voices ? mtrc("engraving", "Voice: %1").arg(track() % VOICES + 1) : u"";
    String pitchName;
    const Drumset* drumset = part()->instrument(chord()->tick())->drumset();
    if (fixed() && headGroup() == NoteHeadGroup::HEAD_SLASH) {
        pitchName = chord()->noStem() ? mtrc("engraving", "Beat Slash") : mtrc("engraving", "Rhythm Slash");
    } else if (staff()->isDrumStaff(tick()) && drumset) {
        pitchName = mtrc("engraving", drumset->name(pitch()));
    } else if (staff()->isTabStaff(tick())) {
        pitchName = mtrc("engraving", "%1; String: %2; Fret: %3")
                    .arg(tpcUserName(true), String::number(string() + 1), String::number(fret()));
    } else {
        pitchName = tpcUserName(true);
    }
    return String("%1 %2 %3%4").arg(noteTypeUserName(), pitchName, duration, (chord()->isGrace() ? u"" : String("; %1").arg(voice)));
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String Note::accessibleExtraInfo() const
{
    String rez;
    if (accidental()) {
        rez = String("%1 %2").arg(rez, accidental()->screenReaderInfo());
    }
    if (!el().empty()) {
        for (EngravingItem* e : el()) {
            if (!score()->selectionFilter().canSelect(e)) {
                continue;
            }
            rez = String("%1 %2").arg(rez, e->screenReaderInfo());
        }
    }
    if (tieFor()) {
        rez = mtrc("engraving", "%1 Start of %2").arg(rez, tieFor()->screenReaderInfo());
    }

    if (tieBack()) {
        rez = mtrc("engraving", "%1 End of %2").arg(rez, tieBack()->screenReaderInfo());
    }

    if (!spannerFor().empty()) {
        for (Spanner* s : spannerFor()) {
            if (!score()->selectionFilter().canSelect(s)) {
                continue;
            }
            rez = mtrc("engraving", "%1 Start of %2").arg(rez, s->screenReaderInfo());
        }
    }
    if (!spannerBack().empty()) {
        for (Spanner* s : spannerBack()) {
            if (!score()->selectionFilter().canSelect(s)) {
                continue;
            }
            rez = mtrc("engraving", "%1 End of %2").arg(rez, s->screenReaderInfo());
        }
    }

    // only read extra information for top note of chord
    // (it is reached directly on next/previous element)
    if (this == chord()->upNote()) {
        rez = String("%1 %2").arg(rez, chord()->accessibleExtraInfo());
    }

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
    return static_cast<int>(_dots.size());
}

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

String Note::subtypeName() const
{
    return TConv::toUserName(_headGroup);
}

//---------------------------------------------------------
//   nextInEl
//   returns next element in _el
//---------------------------------------------------------

EngravingItem* Note::nextInEl(EngravingItem* e)
{
    if (e == _el.back()) {
        return nullptr;
    }
    auto i = std::find(_el.begin(), _el.end(), e);
    if (i == _el.end()) {
        return nullptr;
    }
    return *(i + 1);
}

//---------------------------------------------------------
//   prevInEl
//   returns prev element in _el
//---------------------------------------------------------

EngravingItem* Note::prevInEl(EngravingItem* e)
{
    if (e == _el.front()) {
        return nullptr;
    }
    auto i = std::find(_el.begin(), _el.end(), e);
    if (i == _el.end()) {
        return nullptr;
    }
    return *(i - 1);
}

static bool tieValid(Tie* tie)
{
    return tie && !tie->segmentsEmpty();
}

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

EngravingItem* Note::nextElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }
    if (!e) {
        return nullptr;
    }
    switch (e->type()) {
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
    case ElementType::TEXT:
    case ElementType::BEND: {
        EngravingItem* next = nextInEl(e);           // return next element in _el
        if (next) {
            return next;
        } else if (tieValid(_tieFor)) {
            return _tieFor->frontSegment();
        } else if (!_spannerFor.empty()) {
            for (auto i : qAsConst(_spannerFor)) {
                if (i->type() == ElementType::GLISSANDO) {
                    return i->spannerSegments().front();
                }
            }
        }
        return nullptr;
    }

    case ElementType::TIE_SEGMENT:
        if (!_spannerFor.empty()) {
            for (auto i : qAsConst(_spannerFor)) {
                if (i->type() == ElementType::GLISSANDO) {
                    return i->spannerSegments().front();
                }
            }
        }
        return chord()->nextElement();

    case ElementType::GLISSANDO_SEGMENT:
        return chord()->nextElement();

    case ElementType::ACCIDENTAL:
        if (!_el.empty()) {
            return _el[0];
        }
        if (tieValid(_tieFor)) {
            return _tieFor->frontSegment();
        }
        if (!_spannerFor.empty()) {
            for (auto i : qAsConst(_spannerFor)) {
                if (i->isGlissando()) {
                    return i->spannerSegments().front();
                }
            }
        }
        return nullptr;

    case ElementType::NOTE:
        if (!_el.empty()) {
            return _el[0];
        }
        if (tieValid(_tieFor)) {
            return _tieFor->frontSegment();
        }
        if (!_spannerFor.empty()) {
            for (auto i : qAsConst(_spannerFor)) {
                if (i->isGlissando()) {
                    return i->spannerSegments().front();
                }
            }
        }
        return nullptr;

    default:
        return nullptr;
    }
}

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

EngravingItem* Note::prevElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().back();
    }
    if (!e) {
        return nullptr;
    }
    switch (e->type()) {
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
    case ElementType::TEXT:
    case ElementType::BEND: {
        EngravingItem* prev = prevInEl(e);           // return prev element in _el
        if (prev) {
            return prev;
        }
    }
        return this;
    case ElementType::TIE_SEGMENT:
        if (!_el.empty()) {
            return _el.back();
        }
        return this;
    case ElementType::GLISSANDO_SEGMENT:
        if (tieValid(_tieFor)) {
            return _tieFor->frontSegment();
        } else if (!_el.empty()) {
            return _el.back();
        }
        return this;
    case ElementType::ACCIDENTAL:
        return this;
    default:
        return nullptr;
    }
}

//---------------------------------------------------------
//   lastElementBeforeSegment
//---------------------------------------------------------

EngravingItem* Note::lastElementBeforeSegment()
{
    if (!_spannerFor.empty()) {
        for (auto i : qAsConst(_spannerFor)) {
            if (i->type() == ElementType::GLISSANDO) {
                return i->spannerSegments().front();
            }
        }
    }
    if (tieValid(_tieFor)) {
        return _tieFor->frontSegment();
    }
    if (!_el.empty()) {
        return _el.back();
    }
    return this;
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Note::nextSegmentElement()
{
    if (chord()->isGrace()) {
        return EngravingItem::nextSegmentElement();
    }

    const std::vector<Note*>& notes = chord()->notes();
    if (this == notes.front()) {
        return chord()->nextSegmentElement();
    }
    auto i = std::find(notes.begin(), notes.end(), this);
    return *(i - 1);
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Note::prevSegmentElement()
{
    if (chord()->isGrace()) {
        return EngravingItem::prevSegmentElement();
    }

    const std::vector<Note*>& notes = chord()->notes();
    if (this == notes.back()) {
        return chord()->prevSegmentElement();
    }
    auto i = std::find(notes.begin(), notes.end(), this);
    return *++i;
}

//---------------------------------------------------------
//   lastTiedNote
//---------------------------------------------------------

const Note* Note::lastTiedNote() const
{
    std::vector<const Note*> notes;
    const Note* note = this;
    notes.push_back(note);
    while (note->tieFor()) {
        if (std::find(notes.begin(), notes.end(), note->tieFor()->endNote()) != notes.end()) {
            break;
        }
        if (!note->tieFor()->endNote()) {
            break;
        }
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
        if (std::find(notes.begin(), notes.end(), note->tieBack()->startNote()) != notes.end()) {
            break;
        }
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
        Note* endNote = note->tieFor()->endNote();
        if (!endNote || std::find(notes.begin(), notes.end(), endNote) != notes.end()) {
            break;
        }
        note = endNote;
        notes.push_back(note);
    }
    return notes;
}

//---------------------------------------------------------
//   unisonIndex
//---------------------------------------------------------

int Note::unisonIndex() const
{
    int index = 0;
    for (Note* n : chord()->notes()) {
        if (n->pitch() == pitch()) {
            if (n == this) {
                return index;
            } else {
                ++index;
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   disconnectTiedNotes
//---------------------------------------------------------

void Note::disconnectTiedNotes()
{
    if (tieBack() && tieBack()->startNote()) {
        tieBack()->startNote()->remove(tieBack());
    }
    if (tieFor() && tieFor()->endNote()) {
        tieFor()->endNote()->setTieBack(0);
    }
}

//---------------------------------------------------------
//   connectTiedNotes
//---------------------------------------------------------

void Note::connectTiedNotes()
{
    if (tieBack()) {
        tieBack()->setEndNote(this);
        if (tieBack()->startNote()) {
            tieBack()->startNote()->add(tieBack());
        }
    }
    if (tieFor() && tieFor()->endNote()) {
        tieFor()->endNote()->setTieBack(tieFor());
    }
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
    if (score()) {
        score()->changeAccidental(this, type);
    }
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Note::shape() const
{
    RectF r(bbox());

    Shape shape(r, this);
    for (NoteDot* dot : _dots) {
        shape.add(symBbox(SymId::augmentationDot).translated(dot->pos()), dot);
    }
    if (_accidental && _accidental->addToSkyline()) {
        shape.add(_accidental->bbox().translated(_accidental->pos()), _accidental);
    }
    for (auto e : _el) {
        if (e->addToSkyline()) {
            if (e->isFingering() && toFingering(e)->layoutType() != ElementType::NOTE) {
                continue;
            }
            shape.add(e->bbox().translated(e->pos()), e);
        }
    }
    return shape;
}

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void Note::undoUnlink()
{
    EngravingItem::undoUnlink();
    for (EngravingItem* e : _el) {
        e->undoUnlink();
    }
}

//---------------------------------------------------------
//   slides
//---------------------------------------------------------

bool Note::isSlideToNote() const
{
    if (!_attachedSlide.isValid()) {
        return false;
    }

    if (_attachedSlide.is(Note::SlideType::Lift)
        || _attachedSlide.is(Note::SlideType::Plop)) {
        return true;
    }

    return false;
}

bool Note::isSlideOutNote() const
{
    if (!_attachedSlide.isValid()) {
        return false;
    }

    if (_attachedSlide.is(Note::SlideType::Doit)
        || _attachedSlide.is(Note::SlideType::Fall)) {
        return true;
    }

    return false;
}

bool Note::isSlideStart() const
{
    return _attachedSlide.isValid() && _attachedSlide.startNote == this;
}

bool Note::isSlideEnd() const
{
    return (_attachedSlide.isValid() && _attachedSlide.endNote == this)
           || (_relatedSlide && _relatedSlide->endNote == this);
}

double Note::computePadding(const EngravingItem* nextItem) const
{
    double scaling = (mag() + nextItem->mag()) / 2;
    double padding = score()->paddingTable().at(type()).at(nextItem->type());

    if ((nextItem->isNote() || nextItem->isStem()) && track() == nextItem->track()
        && (shape().translated(pos())).intersects(nextItem->shape().translated(nextItem->pos()))) {
        padding = std::max(padding, static_cast<double>(score()->styleMM(Sid::minNoteDistance)));
    }

    padding *= scaling;

    // Grace-to-grace
    if (isGrace() && nextItem->isNote() && toNote(nextItem)->isGrace()) {
        padding = std::max(padding, static_cast<double>(score()->styleMM(Sid::graceToGraceNoteDist)));
    }
    // Grace-to-main
    if (isGrace() && (nextItem->isRest() || (nextItem->isNote() && !toNote(nextItem)->isGrace()))) {
        padding = std::max(padding, static_cast<double>(score()->styleMM(Sid::graceToMainNoteDist)));
    }
    // Main-to-grace
    if (!isGrace() && nextItem->isNote() && toNote(nextItem)->isGrace()) {
        qDebug() << nextItem->typeName();
        padding = std::max(padding, static_cast<double>(score()->styleMM(Sid::graceToMainNoteDist)));
    }

    return padding;
}
}
