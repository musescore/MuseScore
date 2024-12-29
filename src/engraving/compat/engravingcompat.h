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

#ifndef MU_ENGRAVING_COMPAT_ENGRAVINGCOMPAT_H
#define MU_ENGRAVING_COMPAT_ENGRAVINGCOMPAT_H

namespace mu::engraving {
class MasterScore;
}

namespace mu::engraving::compat {
class EngravingCompat
{
public:
    static void doPreLayoutCompatIfNeeded(MasterScore* score);
    static void doPostLayoutCompatIfNeeded(MasterScore* score);

private:
    static void correctPedalEndPoints(MasterScore* score);
    static void undoStaffTextExcludeFromPart(MasterScore* masterScore);
    static void migrateDynamicPosOnVocalStaves(MasterScore* masterScore);
    static void resetMarkerLeftFontSize(MasterScore* masterScore);
    static void replaceEmptyCRSegmentsWithTimeTick(MasterScore* masterScore);

    static bool relayoutUserModifiedCrossStaffBeams(MasterScore* score);
};
} // namespace mu::engraving::compat

#endif // MU_ENGRAVING_COMPAT_ENGRAVINGCOMPAT_H
