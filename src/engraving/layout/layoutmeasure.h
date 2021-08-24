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
#ifndef MU_ENGRAVING_LAYOUTMEASURE_H
#define MU_ENGRAVING_LAYOUTMEASURE_H

#include "layoutoptions.h"

namespace Ms {
class Score;
class Measure;
class Fraction;
class MeasureBase;
}

namespace mu::engraving {
struct LayoutContext;
class LayoutMeasure
{
public:
    LayoutMeasure() = default;

    static void getNextMeasure(const LayoutOptions& options, Ms::Score* score, LayoutContext& lc);

private:

    static void createMMRest(const LayoutOptions& options, Ms::Score* score, Ms::Measure* firstMeasure, Ms::Measure* lastMeasure,
                             const Ms::Fraction& len);

    static int adjustMeasureNo(LayoutContext& lc, Ms::MeasureBase* m);
};
}

#endif // MU_ENGRAVING_LAYOUTMEASURE_H
