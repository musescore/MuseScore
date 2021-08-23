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

namespace Ms {
class Score;
class Fraction;
class System;
class Tremolo;
}

namespace mu::engraving {
class LayoutContext;
class Layout
{
public:
    Layout(Ms::Score* score);

    void doLayoutRange(const Ms::Fraction&, const Ms::Fraction&);

private:

    void layoutLinear(LayoutContext& lc);
    void layoutLinear(bool layoutAll, LayoutContext& lc);
    void resetSystems(bool layoutAll, LayoutContext& lc);
    void collectLinearSystem(LayoutContext& lc);

    void doLayout(LayoutContext& lc);

    Ms::Score* m_score = nullptr;
};
}

#endif // MU_ENGRAVING_LAYOUT_H
