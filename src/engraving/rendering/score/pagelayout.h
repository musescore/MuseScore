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
#ifndef MU_ENGRAVING_PAGELAYOUT_DEV_H
#define MU_ENGRAVING_PAGELAYOUT_DEV_H

#include "layoutcontext.h"

namespace mu::engraving {
class Page;
class System;
}

namespace mu::engraving::rendering::score {
class PageLayout
{
public:

    static void removeEmptySystems(LayoutContext& ctx);
    static void getNextPage(LayoutContext& ctx);
    static void collectPage(LayoutContext& ctx);

private:
    static void layoutPage(LayoutContext& ctx, Page* page, double restHeight, double footerPadding);
    static void checkDivider(LayoutContext& ctx, bool left, System* s, double yOffset, bool remove = false);
    static void distributeStaves(LayoutContext& ctx, Page* page, double footerPadding);

    static void layoutCrossStaffElements(LayoutContext& ctx, Page* page);
    static void layoutCrossStaffSlurs(LayoutContext& ctx, System* system);
    static void layoutArticAndFingeringOnCrossStaffBeams(LayoutContext& ctx, System* system);
};
}

#endif // MU_ENGRAVING_PAGELAYOUT_DEV_H
