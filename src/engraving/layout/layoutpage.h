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
#ifndef MU_ENGRAVING_LAYOUTPAGE_H
#define MU_ENGRAVING_LAYOUTPAGE_H

#include "layoutoptions.h"
#include "layoutcontext.h"

namespace Ms {
class Page;
class System;
}

namespace mu::engraving {
class LayoutPage
{
public:

    static void getNextPage(const LayoutOptions& options, LayoutContext& lc);
    static void collectPage(const LayoutOptions& options, LayoutContext& lc);

private:
    static void layoutPage(Ms::Page* page, qreal restHeight);
    static void checkDivider(bool left, Ms::System* s, qreal yOffset, bool remove = false);
    static void distributeStaves(Ms::Page* page);
};
}

#endif // MU_ENGRAVING_LAYOUTPAGE_H
