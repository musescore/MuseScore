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
#ifndef MU_ENGRAVING_ARPEGGIOLAYOUT_DEV_H
#define MU_ENGRAVING_ARPEGGIOLAYOUT_DEV_H

#include <climits>

#include "layoutcontext.h"
#include "dom/arpeggio.h"

namespace mu::engraving::rendering::score {
class ArpeggioLayout
{
    static constexpr int ARBITRARY_ARPEGGIO_LENGTH = INT_MAX;
public:

    static void layoutArpeggio2(Arpeggio* item, LayoutContext& ctx);

    static double insetDistance(const Arpeggio* item, const LayoutContext& ctx, double mag_, const Chord* _chord);
    static double insetDistance(const Arpeggio* item, const LayoutContext& ctx, double mag_, const Chord* _chord,
                                const std::vector<Accidental*>& accidentals);
    static double insetTop(const Arpeggio* item, const Chord* c);
    static double insetBottom(const Arpeggio* item, const Chord* c);
    static double insetWidth(const Arpeggio* item);
    static void clearAccidentals(Arpeggio* item, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_ARPEGGIOLAYOUT_DEV_H
