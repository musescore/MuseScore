/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#pragma once

#include "layoutcontext.h"

#include "dom/instrumentname.h"
#include "dom/sharedpart.h"

namespace mu::engraving {
class Bracket;
class BracketItem;
class InstrumentName;
class System;
class SysStaff;
}

namespace mu::engraving::rendering::score {
class SystemHeaderLayout
{
public:
    static void updateSystemHeaderWidth(System* system, LayoutContext& ctx);

    static double layoutBrackets(System* system, LayoutContext& ctx);
    static void addBrackets(System* system, Measure* measure, LayoutContext& ctx);
    static double totalBracketOffset(LayoutContext& ctx);
    static void setBracketsXPosition(System* system, double xPosition);
    static void layoutBracketsVertical(System* system, LayoutContext& ctx);

    static void computeInstrumentNameOffset(System* system, LayoutContext& ctx);
    static void computeInstrumentNamesWidth(System* system, LayoutContext& ctx);
    static void setInstrumentNamesVerticalPos(System* system, LayoutContext& ctx);
    static void setInstrumentNamesHorizontalPos(System* system);
    static void setGroupBracketsHorizontalPos(System* system);
    static void setInstrumentNames(System* system, LayoutContext& ctx);

    static String formatSharedVoiceLabel(const std::vector<Instrument*>& instruments, bool trailingDotSingle, bool trailingDotMultiple,
                                         int hyphenLimit);

private:
    static Bracket* createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                  std::vector<Bracket*>& bl, Measure* measure);
    static void computeGroupBracketsWidths(System* system, LayoutContext& ctx);
    static void computeStackedBracketsWidth(Bracket* first, const std::vector<Bracket*>& allGroupBracketsOrderedByColumn, double& width,
                                            std::vector<Bracket*>& stack);

    static void setSharedPartNames(SharedPart* sharedPart, staff_idx_t startStaffIdx, System* system, LayoutContext& ctx, bool longName,
                                   const Fraction& tick = { 0, 1 });

    static void updateGroupNames(System* system, LayoutContext& ctx, const Fraction& tick);
    static bool useGroupNames(const String& instrumentGroup, LayoutContext& ctx);
    static InstrumentName* updateName(System* system, staff_idx_t staffIdx, LayoutContext& ctx, const String& name, InstrumentNameType type,
                                      InstrumentNameRole role);
    static String formattedInstrumentName(System* system, Part* part, const Fraction& tick);
    static String formattedGroupName(System* system, Part* part, const Fraction& tick);

    static String formattedSharedStaffLabel(staff_idx_t staffIdx, const SharedTrackMap& trackMap, const std::vector<Part*>& originParts);
    static String formatVerticalSharedLabel(const std::vector<Instrument*>& instruments, bool trailingDotSingle);

    static String& resolveTokens(String& str, const String& name, const String& transposition, const String& number);
    static bool showNames(LayoutContext& ctx);
    static double nameWidthIncludingGroupBrackets(InstrumentName* name, System* system);

    static bool stackLabelsVertically(System* system);
};
}
