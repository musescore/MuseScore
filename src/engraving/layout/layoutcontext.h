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
#ifndef MU_ENGRAVING_LAYOUTCONTEXT_H
#define MU_ENGRAVING_LAYOUTCONTEXT_H

#include <vector>
#include <set>

#include "types/fraction.h"
#include "types/types.h"

namespace mu::engraving {
class Score;
class Page;
class System;
class Spanner;
class MeasureBase;
}

namespace mu::engraving {
class LayoutContext
{
public:
    LayoutContext(mu::engraving::Score* s);
    LayoutContext(const LayoutContext&) = delete;
    LayoutContext& operator=(const LayoutContext&) = delete;
    ~LayoutContext();

    mu::engraving::Score* score() const { return m_score; }

    bool startWithLongNames = true;
    bool firstSystem = true;
    bool firstSystemIndent = true;
    mu::engraving::Page* page = nullptr;
    page_idx_t curPage = 0; // index in Score->page()s
    mu::engraving::Fraction tick{ 0, 1 };

    std::vector<mu::engraving::System*> systemList; // reusable systems
    std::set<mu::engraving::Spanner*> processedSpanners;

    mu::engraving::System* prevSystem = nullptr; // used during page layout
    mu::engraving::System* curSystem = nullptr;

    mu::engraving::MeasureBase* systemOldMeasure = nullptr;
    mu::engraving::MeasureBase* pageOldMeasure = nullptr;
    bool rangeDone = false;

    mu::engraving::MeasureBase* prevMeasure = nullptr;
    mu::engraving::MeasureBase* curMeasure = nullptr;
    mu::engraving::MeasureBase* nextMeasure = nullptr;
    int measureNo = 0;
    mu::engraving::Fraction startTick;
    mu::engraving::Fraction endTick;

private:
    mu::engraving::Score* m_score = nullptr;
};
}

#endif // MU_ENGRAVING_LAYOUTCONTEXT_H
