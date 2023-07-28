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

#include "draw/types/brush.h"

#include "translation.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"
#include "iengravingfont.h"

#include "accidental.h"
#include "actionicon.h"
#include "articulation.h"

#include "bagpembell.h"
#include "beam.h"
#include "bend.h"

#include "connector.h"
#include "chord.h"
#include "chordline.h"

#include "drumset.h"
#include "factory.h"
#include "fingering.h"
#include "glissando.h"
#include "hook.h"
#include "image.h"
#include "linkedobjects.h"
#include "measure.h"
#include "mscoreview.h"
#include "notedot.h"
#include "part.h"
#include "pitchspelling.h"
#include "score.h"
#include "segment.h"
#include "spanner.h"
#include "staff.h"
#include "stafftype.h"
#include "stretchedbend.h"
#include "stringdata.h"
#include "tie.h"
#include "tremolo.h"
#include "undo.h"
#include "utils.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#include "accessibility/accessibleroot.h"
#endif

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
        { SymId::noteheadSlashDiamondWhite,   SymId::noteheadSlashDiamondWhite,   SymId::noteheadSlashHorizontalEnds,
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
        { SymId::noteHSharpWhole,  SymId::noteHSharpHalf,  SymId::noteHSharpBlack,  SymId::noSym },

        { SymId::noSym, SymId::swissRudimentsNoteheadHalfFlam,   SymId::swissRudimentsNoteheadBlackFlam,   SymId::noSym },
        { SymId::noSym, SymId::swissRudimentsNoteheadHalfDouble, SymId::swissRudimentsNoteheadBlackDouble, SymId::noSym }
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
        { SymId::noteheadSlashDiamondWhite,   SymId::noteheadSlashDiamondWhite,   SymId::noteheadSlashHorizontalEnds,
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
        { SymId::noteHSharpWhole,  SymId::noteHSharpHalf,  SymId::noteHSharpBlack,  SymId::noSym },

        { SymId::noSym, SymId::swissRudimentsNoteheadHalfFlam,   SymId::swissRudimentsNoteheadBlackFlam,   SymId::noSym },
        { SymId::noSym, SymId::swissRudimentsNoteheadHalfDouble, SymId::swissRudimentsNoteheadBlackDouble, SymId::noSym }
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
    m_playEvents.push_back(NoteEvent());      // add default play event
    m_cachedNoteheadSym = SymId::noSym;
    m_cachedSymNull = SymId::noSym;
}

Note::~Note()
{
    delete m_accidental;
    DeleteAll(m_el);

    if (m_tieFor && m_tieFor->parent() == this) {
        delete m_tieFor;
    }

    DeleteAll(m_dots);
    m_leftParenthesis = nullptr;
    m_rightParenthesis = nullptr;
}

std::vector<const Note*> Note::compoundNotes() const
{
    std::vector<const Note*> elements;
    if (const Note* note = firstTiedNote()) {
        elements.push_back(note);
    }

    if (const Note* note = lastTiedNote()) {
        elements.push_back(note);
    }

    for (Spanner* e : m_spannerFor) {
        elements.push_back(toNote(e->endElement()));
    }
    for (Spanner* e : m_spannerBack) {
        elements.push_back(toNote(e->startElement()));
    }

    Beam* beam = chord()->beam();
    if (beam) {
        for (EngravingItem* e : beam->elements()) {
            if (e && e->isNote() && e != this) {
                elements.push_back(toNote(e));
            }
        }
    }

    return elements;
}

Note::Note(const Note& n, bool link)
    : EngravingItem(n)
{
    if (link) {
        score()->undo(new Link(this, const_cast<Note*>(&n)));
    }
    m_subchannel        = n.m_subchannel;
    m_line              = n.m_line;
    m_fret              = n.m_fret;
    m_harmonicFret     = n.m_harmonicFret;
    m_displayFret      = n.m_displayFret;
    m_string            = n.m_string;
    m_fretConflict      = n.m_fretConflict;
    m_ghost             = n.m_ghost;
    m_deadNote          = n.m_deadNote;
    m_dragMode           = n.m_dragMode;
    m_pitch             = n.m_pitch;
    m_tpc[0]            = n.m_tpc[0];
    m_tpc[1]            = n.m_tpc[1];
    m_dotsHidden        = n.m_dotsHidden;
    m_hidden            = n.m_hidden;
    m_play              = n.m_play;
    m_tuning            = n.m_tuning;
    m_veloType          = n.m_veloType;
    m_userVelocity      = n.m_userVelocity;
    m_headScheme        = n.m_headScheme;
    m_headGroup         = n.m_headGroup;
    m_headType          = n.m_headType;
    m_mirror            = n.m_mirror;
    m_userMirror        = n.m_userMirror;
    m_isSmall          = n.m_isSmall;
    m_userDotPosition   = n.m_userDotPosition;
    m_fixed             = n.m_fixed;
    m_fixedLine         = n.m_fixedLine;
    m_accidental        = 0;
    m_harmonic          = n.m_harmonic;
    m_cachedNoteheadSym = n.m_cachedNoteheadSym;
    m_cachedSymNull     = n.m_cachedSymNull;

    if (n.m_accidental) {
        add(new Accidental(*(n.m_accidental)));
    }

    // types in _el: SYMBOL, IMAGE, FINGERING, TEXT, BEND
    const Staff* stf = staff();
    bool tabFingering = stf ? stf->staffTypeForElement(this)->showTabFingering() : false;
    for (EngravingItem* e : n.m_el) {
        if (e->isFingering() && staff()->isTabStaff(tick()) && !tabFingering) {      // tablature has no fingering
            continue;
        }
        EngravingItem* ce = e->clone();
        add(ce);
        if (link) {
            score()->undo(new Link(ce, const_cast<EngravingItem*>(e)));
        }
    }

    m_playEvents = n.m_playEvents;

    if (n.m_tieFor) {
        m_tieFor = Factory::copyTie(*n.m_tieFor);
        m_tieFor->setStartNote(this);
        m_tieFor->setTick(m_tieFor->startNote()->tick());
        m_tieFor->setEndNote(0);
    } else {
        m_tieFor = 0;
    }
    m_tieBack  = 0;
    for (NoteDot* dot : n.m_dots) {
        add(Factory::copyNoteDot(*dot));
    }
    m_mark      = n.m_mark;

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

void Note::setPitch(int val, bool notifyAboutChanged)
{
    assert(pitchIsValid(val));
    if (m_pitch != val) {
        m_pitch = val;

        if (notifyAboutChanged) {
            score()->setPlaylistDirty();

#ifndef ENGRAVING_NO_ACCESSIBILITY
            notifyAboutNameChanged();
#endif
        }
    }
}

void Note::setPitch(int pitch, int tpc1, int tpc2)
{
    assert(tpcIsValid(tpc1));
    assert(tpcIsValid(tpc2));
    m_tpc[0] = tpc1;
    m_tpc[1] = tpc2;
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
        key = staff()->concertKey(tick);
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
                interval.flip();
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
    Interval v = staff() ? staff()->transpose(tick) : Interval();
    Key cKey = (staff() && chord()) ? staff()->concertKey(chord()->tick()) : Key::C;
    // set concert pitch tpc
    m_tpc[0] = pitch2tpc(m_pitch, cKey, Prefer::NEAREST);
    // set transposed tpc
    if (v.isZero()) {
        m_tpc[1] = m_tpc[0];
    } else {
        v.flip();
        m_tpc[1] = mu::engraving::transposeTpc(m_tpc[0], v, true);
    }
    assert(tpcIsValid(m_tpc[0]));
    assert(tpcIsValid(m_tpc[1]));
}

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

void Note::setTpc(int v)
{
    IF_ASSERT_FAILED(tpcIsValid(v)) {
        return;
    }
    m_tpc[concertPitchIdx()] = v;
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
    return m_tpc[concertPitchIdx()];
}

//---------------------------------------------------------
//   tpcUserName
//---------------------------------------------------------

String Note::tpcUserName(int tpc, int pitch, bool explicitAccidental, bool full)
{
    String pitchStr = tpc2name(tpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, explicitAccidental, full);
    if (!explicitAccidental) {
        pitchStr.replace(u"b", u"♭");
        pitchStr.replace(u"#", u"♯");
    }

    pitchStr = mtrc("engraving", pitchStr);

    const String octaveStr = String::number(((pitch - static_cast<int>(tpc2alter(tpc))) / PITCH_DELTA_OCTAVE) - 1);

    return pitchStr + (explicitAccidental ? u" " : u"") + octaveStr;
}

//---------------------------------------------------------
//   tpcUserName
//---------------------------------------------------------

String Note::tpcUserName(const bool explicitAccidental, bool full) const
{
    String pitchName = tpcUserName(tpc(), epitch() + ottaveCapoFret(), explicitAccidental, full);

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
        static constexpr size_t bufferSize = 50;
        char buffer[bufferSize];
        snprintf(buffer, bufferSize, "%+.3f", tuning());
        pitchOffset = String::fromAscii(buffer);
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
    Interval v = staff()->transpose(tick);
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
    int result = tpc();

    if (!concertPitch() && transposition()) {
        int tpc1 = this->tpc1();
        if (tpc1 == Tpc::TPC_INVALID) {
            result = transposeTpc(result);
        } else {
            result = tpc1;
        }
    }

    int steps = ottaveCapoFret();
    if (steps != 0) {
        result = mu::engraving::transposeTpc(result, Interval(steps), true);
    }

    return result;
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
    if (m_headType != NoteHeadType::HEAD_AUTO) {
        ht = m_headType;
    }

    const Staff* st = chord() ? chord()->staff() : nullptr;

    if (m_headGroup == NoteHeadGroup::HEAD_CUSTOM) {
        if (st) {
            if (st->staffTypeForElement(chord())->isDrumStaff()) {
                Fraction t = chord()->tick();
                Instrument* inst = st->part()->instrument(t);
                Drumset* d = inst->drumset();
                if (d) {
                    return d->noteHeads(m_pitch, ht);
                } else {
                    LOGD("no drumset");
                    return noteHead(up, NoteHeadGroup::HEAD_NORMAL, ht);
                }
            }
        } else {
            return m_cachedNoteheadSym;
        }
    }

    Key key = Key::C;
    NoteHeadScheme scheme = m_headScheme;
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
    SymId t = noteHead(up, m_headGroup, ht, tpc(), key, scheme);
    if (t == SymId::noSym) {
        LOGD("invalid notehead %d/%d", int(m_headGroup), int(ht));
        t = noteHead(up, NoteHeadGroup::HEAD_NORMAL, ht);
    }
    return t;
}

//---------------------------------------------------------
//   headWidth
//
//    returns the x of the symbol bbox. It is different from headWidth() because zero point could be different from leftmost bbox position.
//---------------------------------------------------------
double Note::bboxRightPos() const
{
    const auto& bbox = score()->engravingFont()->bbox(noteHead(), magS());
    return bbox.right();
}

//---------------------------------------------------------
//   headBodyWidth
//
//    returns the width of the notehead "body". It is actual for slashed noteheads like -O-, where O is body.
//---------------------------------------------------------
double Note::headBodyWidth() const
{
    return headWidth() + 2 * bboxXShift();
}

//---------------------------------------------------------
//   outsideTieAttachX
//
//    returns the X-position for tie attachment for this particular notehead
//---------------------------------------------------------
double Note::outsideTieAttachX(bool up) const
{
    double xo = 0;

    // tab staff notes just use center of bounding box
    if (staffType()->isTabStaff()) {
        return x() + ((width() / 2) * mag());
    }
    // special cases:
    if (m_headGroup == NoteHeadGroup::HEAD_SLASH) {
        xo = (up ? headWidth() * 0.75 : headWidth() * 0.25);
        if (chord()->durationType().hasStem()) {
            // for quarters and halves, we can safely move a little bit outwards
            xo += spatium() * 0.13 * (chord()->up() ? -mag() : mag());
        }
        return x() + xo;
    }
    if (m_headGroup == NoteHeadGroup::HEAD_SLASHED1 || m_headGroup == NoteHeadGroup::HEAD_SLASHED2) {
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
        double xNE = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutNE).x();
        double xNW = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutNW).x();
        xo = ((xNE + xNW) / 2);
        if (xNE < xNW) {
            // musejazz is busted
            xo = 0;
        }
    } else {
        double xSE = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutSE).x();
        double xSW = symSmuflAnchor(noteHead(), SmuflAnchorId::cutOutSW).x();
        xo = ((xSE + xSW) / 2);
        if (xSE < xSW) {
            xo = 0;
        }
    }
    if (xo > 0) {
        return x() + xo;
    }
    // no cutout, not a slash head, default to middle of notehead
    return x() + (headWidth() / 2);
}

void Note::updateHeadGroup(const NoteHeadGroup headGroup)
{
    NoteHeadGroup group = headGroup;

    if (group == NoteHeadGroup::HEAD_INVALID) {
        LOGD("unknown notehead");
        group = NoteHeadGroup::HEAD_NORMAL;
    }

    if (group == m_headGroup) {
        return;
    }

    if (links()) {
        for (EngravingObject* scoreElement : *links()) {
            scoreElement->undoChangeProperty(Pid::HEAD_GROUP, static_cast<int>(group));
            Note* note = toNote(scoreElement);

            if (note->staff() && !note->staff()->isDrumStaff(chord()->tick()) && group == NoteHeadGroup::HEAD_CROSS) {
                scoreElement->undoChangeProperty(Pid::DEAD, true);
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

double Note::headWidth() const
{
    return symWidth(noteHead());
}

//---------------------------------------------------------
//   bboxXShift
//
//    returns the x shift of the notehead bounding box
//---------------------------------------------------------
double Note::bboxXShift() const
{
    const auto& bbox = score()->engravingFont()->bbox(noteHead(), magS());
    return bbox.bottomLeft().x();
}

//---------------------------------------------------------
//   noteheadCenterX
//
//    returns the x coordinate of the notehead center related to the basepoint of the notehead bbox
//---------------------------------------------------------
double Note::noteheadCenterX() const
{
    return score()->engravingFont()->width(noteHead(), magS()) / 2 + bboxXShift();
}

//---------------------------------------------------------
//   tabHeadWidth
//---------------------------------------------------------

double Note::tabHeadWidth(const StaffType* tab) const
{
    double val;
    if (tab && tab->isTabStaff() && m_fret != INVALID_FRET_INDEX && m_string != INVALID_STRING_INDEX) {
        mu::draw::Font f    = tab->fretFont();
        f.setPointSizeF(tab->fretFontSize());
        val  = mu::draw::FontMetrics::width(f, m_fretString) * magS();
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

double Note::headHeight() const
{
    return symHeight(noteHead());
}

//---------------------------------------------------------
//   tabHeadHeight
//---------------------------------------------------------

double Note::tabHeadHeight(const StaffType* tab) const
{
    if (tab && m_fret != INVALID_FRET_INDEX && m_string != INVALID_STRING_INDEX) {
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
    if (!m_tieBack && !m_tieFor && chord()) {
        return chord()->actualTicks();
    }

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
        m_dots.push_back(toNoteDot(e));
        break;
    case ElementType::BEND:
    case ElementType::STRETCHED_BEND:
        m_bend = toBend(e);
    // fallthrough
    case ElementType::FINGERING:
    case ElementType::IMAGE:
    case ElementType::TEXT:
        m_el.push_back(e);
        break;
    case ElementType::SYMBOL: {
        Symbol* s = toSymbol(e);
        SymId symbolId = toSymbol(e)->sym();

        if (symbolId == SymId::noteheadParenthesisLeft) {
            m_leftParenthesis = s;
        } else if (symbolId == SymId::noteheadParenthesisRight) {
            m_rightParenthesis = s;
        }
        m_hasHeadParentheses = m_leftParenthesis && m_rightParenthesis;
        m_el.push_back(e);
    } break;
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
        m_accidental = toAccidental(e);
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
        m_dots.pop_back();
        break;

    case ElementType::BEND:
    case ElementType::STRETCHED_BEND:
        m_bend = nullptr;
    // fallthrough
    case ElementType::TEXT:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
        if (!m_el.remove(e)) {
            LOGD("Note::remove(): cannot find %s", e->typeName());
        }
        break;
    case ElementType::SYMBOL:
        if (e == m_leftParenthesis) {
            m_leftParenthesis = nullptr;
        }
        if (e == m_rightParenthesis) {
            m_rightParenthesis = nullptr;
        }
        m_hasHeadParentheses = m_leftParenthesis && m_rightParenthesis;

        if (!m_el.remove(e)) {
            LOGD("Note::remove(): cannot find %s", e->typeName());
        }
        break;

    case ElementType::TIE: {
        Tie* tie = toTie(e);
        assert(tie->startNote() == this);
        setTieFor(0);
        if (tie->endNote()) {
            tie->endNote()->setTieBack(0);
        }
    }
    break;

    case ElementType::ACCIDENTAL:
        m_accidental = 0;
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
        NoteHeadScheme s = m_headScheme;
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
    TRACE_ITEM_DRAW;
    if (m_hidden) {
        return;
    }

    bool negativeFret = negativeFretUsed() && staff()->isTabStaff(tick());

    Color c(negativeFret ? engravingConfiguration()->criticalColor() : curColor());
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
            if (chord()->measure()->system() == tieBack()->startNote()->chord()->measure()->system() && el().empty()) {
                // fret should be hidden, so return without drawing it
                return;
            }
        }
        // draw background, if required (to hide a segment of string line or to show a fretting conflict)
        if (!tab->linesThrough() || fretConflict()) {
            double d  = spatium() * .1;
            RectF bb = RectF(bbox().x() - d, tab->fretMaskY() * magS(), bbox().width() + 2 * d, tab->fretMaskH() * magS());
            // we do not know which viewer did this draw() call
            // so update all:
            if (!score()->getViewer().empty()) {
                for (MuseScoreView* view : score()->getViewer()) {
                    view->drawBackground(painter, bb);
                }
            } else {
                painter->fillRect(bb, engravingConfiguration()->noteBackgroundColor());
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
        double startPosX = bbox().x();
        if (m_ghost && engravingConfiguration()->tablatureParenthesesZIndexWorkaround()) {
            startPosX += symWidth(SymId::noteheadParenthesisLeft);
        }

        painter->drawText(PointF(startPosX, tab->fretFontYOffset() * magS()), m_fretString);
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
        // Warn if notes are unplayable based on previous harp diagram setting
        if (chord() && chord()->segment() && staff() && !score()->printing() && !staff()->isDrumStaff(chord()->tick())) {
            HarpPedalDiagram* prevDiagram = part()->currentHarpDiagram(chord()->segment()->tick());
            if (prevDiagram && !prevDiagram->isPitchPlayable(ppitch())) {
                painter->setPen(selected() ? engravingConfiguration()->criticalSelectedColor() : engravingConfiguration()->criticalColor());
            }
        }
        // draw blank notehead to avoid staff and ledger lines
        if (m_cachedSymNull != SymId::noSym) {
            painter->save();
            painter->setPen(engravingConfiguration()->noteBackgroundColor());
            drawSymbol(m_cachedSymNull, painter);
            painter->restore();
        }
        drawSymbol(m_cachedNoteheadSym, painter);
    }
}

void Note::setupAfterRead(const Fraction& ctxTick, bool pasteMode)
{
    // ensure sane values:
    m_pitch = std::clamp(m_pitch, 0, 127);

    if (!tpcIsValid(m_tpc[0]) && !tpcIsValid(m_tpc[1])) {
        Key key = (staff() && chord()) ? staff()->key(chord()->tick()) : Key::C;
        int tpc = pitch2tpc(m_pitch, key, Prefer::NEAREST);
        if (concertPitch()) {
            m_tpc[0] = tpc;
        } else {
            m_tpc[1] = tpc;
        }
    }
    if (!(tpcIsValid(m_tpc[0]) && tpcIsValid(m_tpc[1]))) {
        Fraction tick = chord() ? chord()->tick() : Fraction(-1, 1);
        Interval v = staff() ? staff()->transpose(tick) : Interval();
        if (tpcIsValid(m_tpc[0])) {
            v.flip();
            if (v.isZero()) {
                m_tpc[1] = m_tpc[0];
            } else {
                m_tpc[1] = mu::engraving::transposeTpc(m_tpc[0], v, true);
            }
        } else {
            if (v.isZero()) {
                m_tpc[0] = m_tpc[1];
            } else {
                m_tpc[0] = mu::engraving::transposeTpc(m_tpc[1], v, true);
            }
        }
    }

    // check consistency of pitch, tpc1, tpc2, and transposition
    // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
    // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
    // including perhaps some we don't know about yet,
    // we will attempt to fix some problems here regardless of version

    if (staff() && !staff()->isDrumStaff(ctxTick) && !pasteMode && !MScore::testMode) {
        int tpc1Pitch = (tpc2pitch(m_tpc[0]) + 12) % 12;
        int tpc2Pitch = (tpc2pitch(m_tpc[1]) + 12) % 12;
        int soundingPitch = m_pitch % 12;
        if (tpc1Pitch != soundingPitch) {
            LOGD("bad tpc1 - soundingPitch = %d, tpc1 = %d", soundingPitch, tpc1Pitch);
            m_pitch += tpc1Pitch - soundingPitch;
        }
        if (staff()) {
            Interval v = staff()->transpose(ctxTick);
            int writtenPitch = (m_pitch - v.chromatic) % 12;
            if (tpc2Pitch != writtenPitch) {
                LOGD("bad tpc2 - writtenPitch = %d, tpc2 = %d", writtenPitch, tpc2Pitch);
                if (concertPitch()) {
                    // assume we want to keep sounding pitch
                    // so fix written pitch (tpc only)
                    v.flip();
                    m_tpc[1] = mu::engraving::transposeTpc(m_tpc[0], v, true);
                } else {
                    // assume we want to keep written pitch
                    // so fix sounding pitch (both tpc and pitch)
                    m_tpc[0] = mu::engraving::transposeTpc(m_tpc[1], v, true);
                    m_pitch += tpc2Pitch - writtenPitch;
                }
            }
        }
    }

    for (EngravingItem* item : m_el) {
        if (!item->isSymbol()) {
            continue;
        }

        Symbol* symbol = toSymbol(item);
        SymId symbolId = symbol->sym();

        if (symbolId == SymId::noteheadParenthesisLeft) {
            m_leftParenthesis = symbol;
        } else if (symbolId == SymId::noteheadParenthesisRight) {
            m_rightParenthesis = symbol;
        }
    }

    if (m_leftParenthesis && m_rightParenthesis) {
        m_hasHeadParentheses = true;
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
    OBJECT_ALLOCATOR(engraving, NoteEditData)
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

    static inline EditMode editModeByDragDirection(const double& deltaX, const double& deltaY)
    {
        double x = std::abs(deltaX);
        double y = std::abs(deltaY);

        mu::PointF normalizedVector(x, y);

        normalizedVector.normalize();

        float radians = PointF::dotProduct(normalizedVector, PointF(1, 0));

        double degrees = (std::acos(radians) * 180.0) / M_PI;

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
        for (auto ee : m_spannerFor) {
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
           || type == ElementType::ORNAMENT
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
           || type == ElementType::EXPRESSION
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
           || (type == ElementType::CAPO)
           || (type == ElementType::SYSTEM_TEXT)
           || (type == ElementType::TRIPLET_FEEL)
           || (type == ElementType::STICKING)
           || (type == ElementType::TEMPO_TEXT)
           || (type == ElementType::BEND)
           || (type == ElementType::TREMOLOBAR)
           || (type == ElementType::FRET_DIAGRAM)
           || (type == ElementType::FIGURED_BASS)
           || (type == ElementType::LYRICS)
           || (type == ElementType::HARP_DIAGRAM)
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

        if (group != m_headGroup) {
            if (links()) {
                for (EngravingObject* se : *links()) {
                    se->undoChangeProperty(Pid::HEAD_GROUP, int(group));
                    Note* note = toNote(se);
                    if (note->staff() && !note->staff()->isDrumStaff(ch->tick())) {
                        se->undoChangeProperty(Pid::DEAD, group == NoteHeadGroup::HEAD_CROSS);
                    }
                }
            } else {
                undoChangeProperty(Pid::HEAD_GROUP, int(group));
                if (!staff()->isDrumStaff(ch->tick())) {
                    undoChangeProperty(Pid::DEAD, group == NoteHeadGroup::HEAD_CROSS);
                }
            }
        }
    }
    break;

    case ElementType::ACTION_ICON:
    {
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::ACCIACCATURA:
            score()->setGraceNote(ch, pitch(), NoteType::ACCIACCATURA, Constants::DIVISION / 2);
            break;
        case ActionIconType::APPOGGIATURA:
            score()->setGraceNote(ch, pitch(), NoteType::APPOGGIATURA, Constants::DIVISION / 2);
            break;
        case ActionIconType::GRACE4:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE4, Constants::DIVISION);
            break;
        case ActionIconType::GRACE16:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE16,  Constants::DIVISION / 4);
            break;
        case ActionIconType::GRACE32:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE32, Constants::DIVISION / 8);
            break;
        case ActionIconType::GRACE8_AFTER:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE8_AFTER, Constants::DIVISION / 2);
            break;
        case ActionIconType::GRACE16_AFTER:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE16_AFTER, Constants::DIVISION / 4);
            break;
        case ActionIconType::GRACE32_AFTER:
            score()->setGraceNote(ch, pitch(), NoteType::GRACE32_AFTER, Constants::DIVISION / 8);
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
        BagpipeNoteList nl = b->resolveNoteList();
        // add grace notes in reverse order, as setGraceNote adds a grace note
        // before the current note
        for (int i = static_cast<int>(nl.size()) - 1; i >= 0; --i) {
            int p = BagpipeEmbellishment::BAGPIPE_NOTEINFO_LIST[nl.at(i)].pitch;
            score()->setGraceNote(ch, p, NoteType::GRACE32, Constants::DIVISION / 8);
        }
    }
        delete e;
        break;

    case ElementType::NOTE:
    {
        // calculate correct transposed tpc
        Note* n = toNote(e);
        Interval v = staff()->transpose(ch->tick());
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
        for (auto ee : m_spannerFor) {
            if (ee->type() == ElementType::GLISSANDO) {
                LOGD("there is already a glissando");
                delete e;
                return 0;
            }
        }

        // this is the glissando initial note, look for a suitable final note
        Note* finalNote = Glissando::guessFinalNote(chord(), this);
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
        if (data.modifiers & ShiftModifier) {
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

    case ElementType::CHORDLINE:
        toChordLine(e)->setNote(this);
        return ch->drop(data);

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
    if (hasParentheses == m_hasHeadParentheses) {
        return;
    }

    m_hasHeadParentheses = hasParentheses;

    if (hasParentheses) {
        if (!m_leftParenthesis) {
            m_leftParenthesis = new Symbol(this);
            m_leftParenthesis->setSym(SymId::noteheadParenthesisLeft);
            m_leftParenthesis->setParent(this);
            score()->undoAddElement(m_leftParenthesis);
        }

        if (!m_rightParenthesis) {
            m_rightParenthesis = new Symbol(this);
            m_rightParenthesis->setSym(SymId::noteheadParenthesisRight);
            m_rightParenthesis->setParent(this);
            score()->undoAddElement(m_rightParenthesis);
        }
    } else {
        score()->undoRemoveElement(m_leftParenthesis);
        score()->undoRemoveElement(m_rightParenthesis);
        assert(m_leftParenthesis == nullptr);
        assert(m_rightParenthesis == nullptr);
    }
}

//---------------------------------------------------------
//   setDotY
//    dotMove is number of staff spaces/lines to move from the note's
//    space or line
//---------------------------------------------------------

void Note::setDotRelativeLine(int dotMove)
{
    double y = dotMove / 2.0;
    if (staff()->isTabStaff(chord()->tick())) {
        // with TAB's, dotPosX is not set:
        // get dot X from width of fret text and use TAB default spacing
        const Staff* st = staff();
        const StaffType* tab = st->staffTypeForElement(this);
        if (tab->stemThrough()) {
            // if fret mark on lines, use standard processing
            if (!tab->onLines()) {
                // if fret marks above lines, raise the dots by half line distance
                y = -0.5;
            }
            if (dotMove == 0) {
                bool oddVoice = voice() & 1;
                y = oddVoice ? 0.5 : -0.5;
            } else {
                y = 0.5;
            }
        }
        // if stems beside staff, do nothing
        else {
            return;
        }
    }
    y *= spatium() * staff()->lineDistance(tick());

    // apply to dots

    int cdots = static_cast<int>(chord()->dots());
    int ndots = static_cast<int>(m_dots.size());

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
            score()->undoRemoveElement(m_dots.back());
        }
    }

    for (NoteDot* dot : m_dots) {
        rendering()->layoutItem(dot);
        dot->setPosY(y);
    }
}

//---------------------------------------------------------
//   dotIsUp
//---------------------------------------------------------

bool Note::dotIsUp() const
{
    if (m_dots.empty()) {
        return true;
    }
    if (m_userDotPosition == DirectionV::AUTO) {
        return m_dots[0]->y() < spatium() * .1;
    } else {
        return m_userDotPosition == DirectionV::UP;
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
    if (!(m_accidental && Accidental::isMicrotonal(m_accidental->accidentalType()))) {
        // calculate accidental
        AccidentalType acci = AccidentalType::NONE;

        AccidentalVal accVal = tpc2alter(tpc());
        bool error = false;
        int eRelLine = absStep(tpc(), epitch());
        AccidentalVal relLineAccVal = as->accidentalVal(eRelLine, error);
        if (error) {
            LOGD("error accidentalVal()");
            return;
        }
        if ((accVal != relLineAccVal) || hidden() || as->tieContext(eRelLine) || as->forceRestateAccidental(eRelLine)) {
            as->setAccidentalVal(eRelLine, accVal, m_tieBack != 0 && m_accidental == 0);
            acci = Accidental::value2subtype(accVal);
            // if previous tied note has same tpc, don't show accidental
            if (m_tieBack && m_tieBack->startNote()->tpc1() == tpc1()) {
                acci = AccidentalType::NONE;
            } else if (acci == AccidentalType::NONE) {
                acci = AccidentalType::NATURAL;
            }
        } else if (hasAlteredUnison(this)) {
            if ((acci = Accidental::value2subtype(accVal)) == AccidentalType::NONE) {
                acci = AccidentalType::NATURAL;
            }
        }
        if (acci != AccidentalType::NONE && !m_hidden) {
            if (m_accidental == 0) {
                Accidental* a = Factory::createAccidental(this);
                a->setParent(this);
                a->setAccidentalType(acci);
                score()->undoAddElement(a);
            } else if (m_accidental->accidentalType() != acci) {
                Accidental* a = m_accidental->clone();
                a->setParent(this);
                a->setAccidentalType(acci);
                score()->undoChangeElement(m_accidental, a);
            }
        } else {
            if (m_accidental) {
                // remove this if it was AUTO:
                if (m_accidental->role() == AccidentalRole::AUTO) {
                    score()->undoRemoveElement(m_accidental);
                } else {
                    // keep it, but update type if needed
                    acci = Accidental::value2subtype(accVal);
                    if (acci == AccidentalType::NONE) {
                        acci = AccidentalType::NATURAL;
                    }
                    if (m_accidental->accidentalType() != acci) {
                        Accidental* a = m_accidental->clone();
                        a->setParent(this);
                        a->setAccidentalType(acci);
                        score()->undoChangeElement(m_accidental, a);
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
        AccidentalVal accVal = Accidental::subtype2value(m_accidental->accidentalType());
        as->setAccidentalVal(relLine, accVal, m_tieBack != 0 && m_accidental == 0);
    }

    as->setForceRestateAccidental(relLine, false);
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
        return mtrc("engraving", "Acciaccatura");
    case NoteType::APPOGGIATURA:
        return mtrc("engraving", "Appoggiatura");
    case NoteType::GRACE8_AFTER:
    case NoteType::GRACE16_AFTER:
    case NoteType::GRACE32_AFTER:
        return mtrc("engraving", "Grace note after");
    case NoteType::GRACE4:
    case NoteType::GRACE16:
    case NoteType::GRACE32:
        return mtrc("engraving", "Grace note before");
    default:
        return mtrc("engraving", "Note");
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
    for (EngravingItem* e : m_el) {
        e->scanElements(data, func, all);
    }
    for (Spanner* sp : m_spannerFor) {
        sp->scanElements(data, func, all);
    }

    if (!m_dragMode && m_accidental) {
        func(data, m_accidental);
    }
    for (NoteDot* dot : m_dots) {
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
    if (m_tieFor) {
        m_tieFor->setTrack(val);
        for (SpannerSegment* seg : m_tieFor->spannerSegments()) {
            seg->setTrack(val);
        }
    }
    for (Spanner* s : m_spannerFor) {
        s->setTrack(val);
    }
    for (Spanner* s : m_spannerBack) {
        s->setTrack2(val);
    }
    for (EngravingItem* e : m_el) {
        e->setTrack(val);
    }
    if (m_accidental) {
        m_accidental->setTrack(val);
    }
    if (!chord()) {     // if note is dragged with shift+ctrl
        return;
    }
    for (NoteDot* dot : m_dots) {
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

float Note::userVelocityFraction() const
{
    return m_userVelocity / 127.f;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Note::mag() const
{
    double m = chord() ? chord()->mag() : 1.0;
    if (m_isSmall) {
        m *= style().styleD(Sid::smallNoteMag);
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
    return fixed() ? m_fixedLine : m_line;
}

//---------------------------------------------------------
//   setString
//---------------------------------------------------------

void Note::setString(int val)
{
    m_string = val;
    setPosY(m_string * spatium() * 1.5);
}

//---------------------------------------------------------
//   setHeadScheme
//---------------------------------------------------------

void Note::setHeadScheme(NoteHeadScheme val)
{
    IF_ASSERT_FAILED(int(val) >= -1 && int(val) < int(NoteHeadScheme::HEAD_SCHEMES)) {
        val = NoteHeadScheme::HEAD_AUTO;
    }
    m_headScheme = val;
}

//---------------------------------------------------------
//   setHeadGroup
//---------------------------------------------------------

void Note::setHeadGroup(NoteHeadGroup val)
{
    assert(int(val) >= 0 && int(val) < int(NoteHeadGroup::HEAD_GROUPS));
    m_headGroup = val;
}

//---------------------------------------------------------
//   ottaveCapoFret
//    offset added by Ottava's and Capo Fret.
//---------------------------------------------------------

int Note::ottaveCapoFret() const
{
    const Chord* ch = chord();
    Fraction segmentTick = ch->segment()->tick();

    const CapoParams& capo = staff()->capo(segmentTick);
    int capoFret = 0;

    if (capo.active) {
        if (capo.ignoredStrings.empty() || !mu::contains(capo.ignoredStrings, static_cast<string_idx_t>(m_string))) {
            capoFret = capo.fretPosition;
        }
    }

    return staff()->pitchOffset(segmentTick) + capoFret;
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
            DrumInstrumentVariant div = ds->findVariant(m_pitch, ch->articulations(), ch->tremolo());
            if (div.pitch != INVALID_PITCH) {
                return div.pitch;
            }
        }
    }

    return m_pitch + ottaveCapoFret();
}

//---------------------------------------------------------
//   epitch
//    effective pitch, i.e. a pitch which is visible in the
//    currently used written notation.
//    honours transposing instruments
//---------------------------------------------------------

int Note::epitch() const
{
    return m_pitch - (concertPitch() ? 0 : transposition());
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
    return mu::engraving::playingOctave(ppitch(), tpc1());
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
    if (m_veloType == VeloType::OFFSET_VAL) {
        velo = velo + (velo * userVelocity()) / 100;
    } else if (m_veloType == VeloType::USER_VAL) {
        velo = userVelocity();
    }
    return std::clamp(velo, 1, 127);
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Note::startDrag(EditData& ed)
{
    std::shared_ptr<NoteEditData> ned = std::make_shared<NoteEditData>();
    ned->e      = this;
    ned->line   = m_line;
    ned->string = m_string;
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

    if (editData.modifiers & ShiftModifier) {
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

    double _spatium      = spatium();
    bool tab            = st->isTabStaff();
    double step          = _spatium * (tab ? st->lineDistance().val() : 0.5);
    int lineOffset      = lrint(ed.moveDelta.y() / step);

    if (tab) {
        const StringData* strData = staff()->part()->instrument(_tick)->stringData();
        int nString = ned->string + (st->upsideDown() ? -lineOffset : lineOffset);
        int nFret   = strData->fret(m_pitch, nString, staff());

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
        Key cKey = staff()->concertKey(_tick);
        staff_idx_t idx = chord()->vStaffIdx();
        Interval interval = staff()->part()->instrument(_tick)->transpose();
        int newPitch = line2pitch(ned->line + lineOffset, score()->staff(idx)->clef(_tick), key);

        if (!concertPitch()) {
            newPitch += interval.chromatic;
        } else {
            interval.flip();
            key = transposeKey(cKey, interval, staff()->part()->preferSharpFlat());
        }

        int newTpc1 = pitch2tpc(newPitch, cKey, Prefer::NEAREST);
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
        double minDist = previous->minHorizontalCollidingDistance(seg);

        double diff = (ed.pos.x()) - (previous->pageX() + minDist);

        double distanceBetweenSegments = (previous->pageX() + minDist) - seg->pageX();

        if (diff < 0) {
            ned->delta.setX(distanceBetweenSegments);
        }
    } else {
        Measure* measure = seg->measure();

        double minDist = style().styleMM(Sid::barNoteDistance);

        double diff = (ed.pos.x()) - (measure->pageX() + minDist);

        double distanceBetweenSegments = (measure->pageX() + minDist) - seg->pageX();

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
    assert(staffIdx() == chord()->staffIdx());
    staff_idx_t idx      = chord()->vStaffIdx();

    const Staff* staff  = score()->staff(idx);
    const StaffType* st = staff->staffTypeForElement(this);
    if (st->isTabStaff()) {
        // tab staff is already correct, and the following relStep method doesn't apply whatsoever to tab staves
        return;
    }
    ClefType clef = staff->clef(chord()->tick());
    int line      = relStep(relLine, clef);

    if (undoable && (m_line != INVALID_LINE) && (line != m_line)) {
        undoChangeProperty(Pid::LINE, line);
    } else {
        setLine(line);
    }

    int off  = st->stepOffset();
    double ld = st->lineDistance().val();
    setPosY((m_line + off * 2.0) * spatium() * .5 * ld);
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
    m_fret   = nval.fret;
    m_string = nval.string;

    m_tpc[0] = nval.tpc1;
    m_tpc[1] = nval.tpc2;

    if (tick == Fraction(-1, 1) && chord()) {
        tick = chord()->tick();
    }
    Interval v = staff()->transpose(tick);
    if (nval.tpc1 == Tpc::TPC_INVALID) {
        Key key = staff()->concertKey(tick);
        m_tpc[0] = pitch2tpc(nval.pitch, key, Prefer::NEAREST);
    }
    if (nval.tpc2 == Tpc::TPC_INVALID) {
        if (v.isZero()) {
            m_tpc[1] = m_tpc[0];
        } else {
            v.flip();
            m_tpc[1] = mu::engraving::transposeTpc(m_tpc[0], v, true);
        }
    }

    m_headGroup = NoteHeadGroup(nval.headGroup);
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Note::localSpatiumChanged(double oldValue, double newValue)
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
        return m_tpc[0];
    case Pid::TPC2:
        return m_tpc[1];
    case Pid::SMALL:
        return isSmall();
    case Pid::MIRROR_HEAD:
        return userMirror();
    case Pid::HEAD_HAS_PARENTHESES:
        return m_hasHeadParentheses;
    case Pid::DOT_POSITION:
        return PropertyValue::fromValue<DirectionV>(userDotPosition());
    case Pid::HEAD_SCHEME:
        return int(headScheme());
    case Pid::HEAD_GROUP:
        return int(headGroup());
    case Pid::USER_VELOCITY:
        return userVelocity();
    case Pid::TUNING:
        return tuning();
    case Pid::FRET:
        return fret();
    case Pid::STRING:
        return string();
    case Pid::GHOST:
        return ghost();
    case Pid::DEAD:
        return deadNote();
    case Pid::HEAD_TYPE:
        return headType();
    case Pid::VELO_TYPE:
        return m_veloType;
    case Pid::PLAY:
        return play();
    case Pid::LINE:
        return m_line;
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
        m_tpc[0] = v.toInt();
        break;
    case Pid::TPC2:
        m_tpc[1] = v.toInt();
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
        if (links()) {
            for (EngravingObject* scoreElement : *links()) {
                Note* note = toNote(scoreElement);
                note->setDeadNote(staff() && !staff()->isDrumStaff(tick()) && m_headGroup == NoteHeadGroup::HEAD_CROSS);
                note->setHeadGroup(m_headGroup);
            }
        } else {
            setDeadNote(staff() && !staff()->isDrumStaff(tick()) && m_headGroup == NoteHeadGroup::HEAD_CROSS);
            setHeadGroup(m_headGroup);
        }
        break;
    case Pid::USER_VELOCITY:
        setUserVelocity(v.toInt());
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
    case Pid::DEAD:
        setDeadNote(v.toBool());
        break;
    case Pid::HEAD_TYPE:
        setHeadType(v.value<NoteHeadType>());
        break;
    case Pid::VELO_TYPE:
        m_veloType = v.value<VeloType>();
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
    case Pid::DEAD:
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
    case Pid::USER_VELOCITY:
        return 0;
    case Pid::TUNING:
        return 0.0;
    case Pid::FRET:
    case Pid::STRING:
        return -1;
    case Pid::HEAD_TYPE:
        return NoteHeadType::HEAD_AUTO;
    case Pid::VELO_TYPE:
        return VeloType::USER_VAL;
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
    m_headType = t;
}

//---------------------------------------------------------
//   setOnTimeOffset
//---------------------------------------------------------

void Note::setOnTimeOffset(int val)
{
    m_playEvents[0].setOntime(val);
    chord()->setPlayEventType(PlayEventType::User);
}

//---------------------------------------------------------
//   setOffTimeOffset
//---------------------------------------------------------

void Note::setOffTimeOffset(int val)
{
    m_playEvents[0].setLen(val - m_playEvents[0].ontime());
    chord()->setPlayEventType(PlayEventType::User);
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Note::setScore(Score* s)
{
    EngravingItem::setScore(s);
    if (m_tieFor) {
        m_tieFor->setScore(s);
    }
    if (m_accidental) {
        m_accidental->setScore(s);
    }
    for (NoteDot* dot : m_dots) {
        dot->setScore(s);
    }
    for (EngravingItem* el : m_el) {
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
    if (!m_playEvents.empty()) {
        int on = m_playEvents[0].ontime();
        int off = m_playEvents[0].offtime();
        if (on != 0 || off != NoteEvent::NOTE_LENGTH) {
            //: Note-on and note-off times relative to note duration, expressed in thousandths (per mille)
            onofftime = u" " + mtrc("engraving", "(on %1‰ off %2‰)").arg(on, off);
        }
    }

    const Drumset* drumset = part()->instrument(chord()->tick())->drumset();
    if (fixed() && headGroup() == NoteHeadGroup::HEAD_SLASH) {
        pitchName = chord()->noStem() ? mtrc("engraving", "Beat slash") : mtrc("engraving", "Rhythm slash");
    } else if (staff()->isDrumStaff(tick()) && drumset) {
        pitchName = drumset->translatedName(pitch());
    } else if (staff()->isTabStaff(tick())) {
        pitchName = mtrc("engraving", "%1; String: %2; Fret: %3")
                    .arg(tpcUserName(false), String::number(string() + 1), String::number(fret()));
    } else {
        pitchName = tpcUserName(false);
    }

    return mtrc("engraving", "%1; Pitch: %2; Duration: %3%4%5")
           .arg(noteTypeUserName(), pitchName, duration, onofftime, (chord()->isGrace() ? u"" : String(u"; %1").arg(voice)));
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String Note::screenReaderInfo() const
{
    const Instrument* instrument = part()->instrument(chord()->tick());
    String duration = chord()->durationUserName();
    Measure* m = chord()->measure();
    bool voices = m ? m->hasVoices(staffIdx()) : false;
    String voice = voices ? mtrc("engraving", "Voice: %1").arg(track() % VOICES + 1) : u"";
    String pitchName;
    String pitchOutOfRangeWarning;
    const Drumset* drumset = instrument->drumset();
    if (fixed() && headGroup() == NoteHeadGroup::HEAD_SLASH) {
        pitchName = chord()->noStem() ? mtrc("engraving", "Beat slash") : mtrc("engraving", "Rhythm slash");
    } else if (staff()->isDrumStaff(tick()) && drumset) {
        pitchName = drumset->translatedName(pitch());
    } else if (staff()->isTabStaff(tick())) {
        pitchName = mtrc("engraving", "%1; String: %2; Fret: %3")
                    .arg(tpcUserName(true, true), String::number(string() + 1), String::number(fret()));
    } else {
        pitchName = m_headGroup == NoteHeadGroup::HEAD_NORMAL
                    ? tpcUserName(true, true)
                    //: head as in note head. %1 is head type (circle, cross, etc.). %2 is pitch (e.g. Db4).
                    : mtrc("engraving", "%1 head %2").arg(translatedSubtypeUserName()).arg(tpcUserName(true));
        if (chord()->staffMove() < 0) {
            duration += u"; " + mtrc("engraving", "Cross-staff above");
        } else if (chord()->staffMove() > 0) {
            duration += u"; " + mtrc("engraving", "Cross-staff below");
        }

        if (pitch() < instrument->minPitchP()) {
            pitchOutOfRangeWarning = u" " + mtrc("engraving", "too low");
        } else if (pitch() > instrument->maxPitchP()) {
            pitchOutOfRangeWarning = u" " + mtrc("engraving", "too high");
        } else if (pitch() < instrument->minPitchA()) {
            pitchOutOfRangeWarning = u" " + mtrc("engraving", "too low for amateurs");
        } else if (pitch() > instrument->maxPitchA()) {
            pitchOutOfRangeWarning = u" " + mtrc("engraving", "too high for amateurs");
        }
    }
    return String(u"%1 %2 %3%4%5").arg(noteTypeUserName(), pitchName, duration, pitchOutOfRangeWarning,
                                       (chord()->isGrace() ? u"" : String(u"; %1").arg(voice)));
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String Note::accessibleExtraInfo() const
{
    String rez;
    if (accidental()) {
        rez = String(u"%1 %2").arg(rez, accidental()->screenReaderInfo());
    }
    if (!el().empty()) {
        for (EngravingItem* e : el()) {
            if (!score()->selectionFilter().canSelect(e)) {
                continue;
            }
            rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
        }
    }
    if (tieFor()) {
        rez += u" " + mtrc("engraving", "Start of %1").arg(tieFor()->screenReaderInfo());
    }

    if (tieBack()) {
        rez += u" " + mtrc("engraving", "End of %1").arg(tieBack()->screenReaderInfo());
    }

    if (!spannerFor().empty()) {
        for (Spanner* s : spannerFor()) {
            if (!score()->selectionFilter().canSelect(s)) {
                continue;
            }
            rez += u" " + mtrc("engraving", "Start of %1").arg(s->screenReaderInfo());
        }
    }
    if (!spannerBack().empty()) {
        for (Spanner* s : spannerBack()) {
            if (!score()->selectionFilter().canSelect(s)) {
                continue;
            }
            rez += u" " + mtrc("engraving", "End of %1").arg(s->screenReaderInfo());
        }
    }

    // only read extra information for top note of chord
    // (it is reached directly on next/previous element)
    if (this == chord()->upNote()) {
        rez = String(u"%1 %2").arg(rez, chord()->accessibleExtraInfo());
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
    return static_cast<int>(m_dots.size());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

TranslatableString Note::subtypeUserName() const
{
    return TConv::userName(m_headGroup);
}

//---------------------------------------------------------
//   nextInEl
//   returns next element in _el
//---------------------------------------------------------

EngravingItem* Note::nextInEl(EngravingItem* e)
{
    if (e == m_el.back()) {
        return nullptr;
    }
    auto i = std::find(m_el.begin(), m_el.end(), e);
    if (i == m_el.end()) {
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
    if (e == m_el.front()) {
        return nullptr;
    }
    auto i = std::find(m_el.begin(), m_el.end(), e);
    if (i == m_el.end()) {
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
        } else if (tieValid(m_tieFor)) {
            return m_tieFor->frontSegment();
        } else if (!m_spannerFor.empty()) {
            for (auto i : m_spannerFor) {
                if (i->type() == ElementType::GLISSANDO) {
                    return i->spannerSegments().front();
                }
            }
        }
        return nullptr;
    }

    case ElementType::TIE_SEGMENT:
        if (!m_spannerFor.empty()) {
            for (auto i : m_spannerFor) {
                if (i->type() == ElementType::GLISSANDO) {
                    return i->spannerSegments().front();
                }
            }
        }
        return chord()->nextElement();

    case ElementType::GLISSANDO_SEGMENT:
        return chord()->nextElement();

    case ElementType::ACCIDENTAL:
        if (!m_el.empty()) {
            return m_el[0];
        }
        if (tieValid(m_tieFor)) {
            return m_tieFor->frontSegment();
        }
        if (!m_spannerFor.empty()) {
            for (auto i : m_spannerFor) {
                if (i->isGlissando()) {
                    return i->spannerSegments().front();
                }
            }
        }
        return nullptr;

    case ElementType::NOTE:
        if (!m_el.empty()) {
            return m_el[0];
        }
        if (tieValid(m_tieFor)) {
            return m_tieFor->frontSegment();
        }
        if (!m_spannerFor.empty()) {
            for (auto i : m_spannerFor) {
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
        if (!m_el.empty()) {
            return m_el.back();
        }
        return this;
    case ElementType::GLISSANDO_SEGMENT:
        if (tieValid(m_tieFor)) {
            return m_tieFor->frontSegment();
        } else if (!m_el.empty()) {
            return m_el.back();
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
    if (!m_spannerFor.empty()) {
        for (auto i : m_spannerFor) {
            if (i->type() == ElementType::GLISSANDO) {
                return i->spannerSegments().front();
            }
        }
    }
    if (tieValid(m_tieFor)) {
        return m_tieFor->frontSegment();
    }
    if (!m_el.empty()) {
        return m_el.back();
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
    return m_accidental ? m_accidental->accidentalType() : AccidentalType::NONE;
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
    for (NoteDot* dot : m_dots) {
        shape.add(symBbox(SymId::augmentationDot).translated(dot->pos()), dot);
    }
    if (m_accidental && m_accidental->addToSkyline()) {
        shape.add(m_accidental->bbox().translated(m_accidental->pos()), m_accidental);
    }
    for (auto e : m_el) {
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
    for (EngravingItem* e : m_el) {
        e->undoUnlink();
    }
}

//---------------------------------------------------------
//   slides
//---------------------------------------------------------

static bool isSlideTo(Note::SlideType slideType)
{
    return slideType == Note::SlideType::UpToNote || slideType == Note::SlideType::DownToNote;
}

static bool isSlideFrom(Note::SlideType slideType)
{
    return slideType == Note::SlideType::UpFromNote || slideType == Note::SlideType::DownFromNote;
}

void Note::attachSlide(SlideType slideType)
{
    if (isSlideTo(slideType)) {
        m_slideToType = slideType;
    } else if (isSlideFrom(slideType)) {
        m_slideFromType = slideType;
    }
}

bool Note::hasSlideToNote() const
{
    return m_slideToType != SlideType::Undefined;
}

bool Note::hasSlideFromNote() const
{
    return m_slideFromType != SlideType::Undefined;
}

mu::PointF Note::posInStaffCoordinates()
{
    double X = x() + chord()->x() + chord()->segment()->x() + chord()->measure()->x() + headWidth() / 2;
    return mu::PointF(X, y());
}

void Note::addLineAttachPoint(PointF point, EngravingItem* line)
{
    // IMPORTANT: the point is expected in *staff* coordinates
    // We transform into note coordinates by subtracting the note position in staff coordinates
    point -= posInStaffCoordinates();
    m_lineAttachPoints.push_back(LineAttachPoint(line, point.x(), point.y()));
}

bool Note::negativeFretUsed() const
{
    return engravingConfiguration()->negativeFretsAllowed() && m_fret < 0;
}
}
