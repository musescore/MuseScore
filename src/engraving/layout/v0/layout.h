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
#ifndef MU_ENGRAVING_LAYOUT_H
#define MU_ENGRAVING_LAYOUT_H

#include "layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Score;
}

namespace mu::engraving::layout::v0  {
class Layout
{
public:
    Layout(Score* score);

    void doLayoutRange(const LayoutOptions& options, const Fraction&, const Fraction&);

private:

    void layoutLinear(const LayoutOptions& options, LayoutContext& ctx);
    void layoutLinear(bool layoutAll, const LayoutOptions& options, LayoutContext& lc);
    void resetSystems(bool layoutAll, const LayoutOptions& options, LayoutContext& lc);
    void collectLinearSystem(const LayoutOptions& options, LayoutContext& ctx);

    void doLayout(const LayoutOptions& options, LayoutContext& lc);

    Score* m_score = nullptr;
};
}

#endif // MU_ENGRAVING_LAYOUT_H
