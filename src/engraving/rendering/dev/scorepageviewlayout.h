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
#ifndef MU_ENGRAVING_SCOREPAGEVIEWLAYOUT_DEV_H
#define MU_ENGRAVING_SCOREPAGEVIEWLAYOUT_DEV_H

#include "layoutcontext.h"

namespace mu::engraving::rendering::dev {
class ScorePageViewLayout
{
public:

    static void layoutPageView(Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick);

private:

    static void initLayoutContext(const Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick);
    static void prepareScore(Score* score, const LayoutContext& ctx);

    static void doLayout(LayoutContext& ctx);

    static void layoutFinished(Score* score, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_SCOREPAGEVIEWLAYOUT_DEV_H
