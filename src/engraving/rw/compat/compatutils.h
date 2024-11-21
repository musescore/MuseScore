/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_COMPATUTILS_H
#define MU_ENGRAVING_COMPATUTILS_H

#include <vector>
#include <set>

#include "dom/articulation.h"

namespace mu::engraving {
enum class SymId;
class Score;
class MasterScore;
class Excerpt;
class Dynamic;
enum class DynamicType : char;
}

namespace mu::engraving::compat {
class CompatUtils
{
public:
    static void assignInitialPartToExcerpts(const std::vector<Excerpt*>& excerpts);
    static void doCompatibilityConversions(MasterScore* masterScore);
    static ArticulationAnchor translateToNewArticulationAnchor(int anchor);
    static const std::set<SymId> ORNAMENT_IDS;

private:
    static void replaceStaffTextWithPlayTechniqueAnnotation(MasterScore* score);
    static void replaceOldWithNewOrnaments(MasterScore* score);
    static void replaceOldWithNewExpressions(MasterScore* score);
    static void reconstructTypeOfCustomDynamics(MasterScore* score);
    static void splitArticulations(MasterScore* score);
    static DynamicType reconstructDynamicTypeFromString(Dynamic* dynamic);
    static void resetRestVerticalOffset(MasterScore* masterScore);
    static void resetArticulationOffsets(MasterScore* masterScore);
    static void resetStemLengthsForTwoNoteTrems(MasterScore* masterScore);
    static void replaceStaffTextWithCapo(MasterScore* masterScore);
    static void addMissingInitKeyForTransposingInstrument(MasterScore* score);
    static void resetFramesExclusionFromParts(MasterScore* masterScore);
    static void mapHeaderFooterStyles(MasterScore* masterScore);
    static NoteLine* createNoteLineFromTextLine(TextLine* textLine);
    static void convertTextLineToNoteAnchoredLine(MasterScore* masterScore);
    static void convertLaissezVibArticToTie(MasterScore* masterScore);
};
}
#endif // MU_ENGRAVING_COMPATUTILS_H
