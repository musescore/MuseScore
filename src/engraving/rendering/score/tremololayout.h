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
#ifndef MU_ENGRAVING_TREMOLOLAYOUT_DEV_H
#define MU_ENGRAVING_TREMOLOLAYOUT_DEV_H

#include <utility>

#include "layoutcontext.h"

namespace mu::engraving {
class TremoloTwoChord;
class TremoloSingleChord;
}

namespace mu::engraving::rendering::score {
class TremoloLayout
{
public:

    static void layout(TremoloTwoChord* item, const LayoutContext& ctx);
    static void layout(TremoloSingleChord* item, const LayoutContext& ctx);

    static std::pair<double, double> extendedStemLenWithTwoNoteTremolo(TremoloTwoChord* tremolo, double stemLen1, double stemLen2);

    static void createBeamSegments(TremoloTwoChord* item, const LayoutContext& ctx);
private:
    static void layoutOneNoteTremolo(TremoloSingleChord* item, const LayoutContext& ctx, double x, double y, double h, double spatium);
    static void layoutTwoNotesTremolo(TremoloTwoChord* item, const LayoutContext& ctx, double x, double y, double h, double spatium);
    static void calcIsUp(TremoloTwoChord* item);
};
}

#endif // MU_ENGRAVING_TREMOLOLAYOUT_DEV_H
