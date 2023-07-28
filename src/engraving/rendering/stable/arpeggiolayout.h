/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_ARPEGGIOLAYOUT_STABLE_H
#define MU_ENGRAVING_ARPEGGIOLAYOUT_STABLE_H

#include "layoutcontext.h"

namespace mu::engraving {
class Arpeggio;
}

namespace mu::engraving::rendering::stable {
class ArpeggioLayout
{
public:

    static void layout(Arpeggio* item, LayoutContext& ctx);

    static void layoutArpeggio2(Arpeggio* item, LayoutContext& ctx);
    static void computeHeight(Arpeggio* item, bool includeCrossStaffHeight = false);

    static void layoutOnEditDrag(Arpeggio* item, LayoutContext& ctx);
    static void layoutOnEdit(Arpeggio* item, LayoutContext& ctx);

private:
    static void symbolLine(Arpeggio* item, LayoutContext& ctx, SymId end, SymId fill);
    static double calcTop(Arpeggio* item, LayoutContext& ctx);
    static double calcBottom(Arpeggio* item, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_ARPEGGIOLAYOUT_STABLE_H
