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
#ifndef MU_ENGRAVING_TREMOLOLAYOUT_H
#define MU_ENGRAVING_TREMOLOLAYOUT_H

#include <utility>

#include "layoutcontext.h"

namespace mu::engraving {
class Tremolo;
}

namespace mu::engraving::layout::v0 {
class TremoloLayout
{
public:

    static void layout(Tremolo* item, LayoutContext& ctx);

    static std::pair<double, double> extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, double stemLen1, double stemLen2);

private:
    static void layoutOneNoteTremolo(Tremolo* item, LayoutContext& ctx, double x, double y, double h, double spatium);
    static void layoutTwoNotesTremolo(Tremolo* item, LayoutContext& ctx, double x, double y, double h, double spatium);
};
}

#endif // MU_ENGRAVING_TREMOLOLAYOUT_H
