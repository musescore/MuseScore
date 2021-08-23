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

#include "libmscore/fraction.h"

namespace Ms {
class Score;
class Page;
class System;
class Spanner;
class MeasureBase;
}

namespace mu::engraving {
struct LayoutContext
{
    Ms::Score* score = nullptr;
    bool startWithLongNames = true;
    bool firstSystem = true;
    bool firstSystemIndent = true;
    Ms::Page* page = nullptr;
    int curPage = 0; // index in Score->page()s
    Ms::Fraction tick{ 0, 1 };

    QList<Ms::System*> systemList; // reusable systems
    std::set<Ms::Spanner*> processedSpanners;

    Ms::System* prevSystem = nullptr; // used during page layout
    Ms::System* curSystem = nullptr;

    Ms::MeasureBase* systemOldMeasure = nullptr;
    Ms::MeasureBase* pageOldMeasure = nullptr;
    bool rangeDone = false;

    Ms::MeasureBase* prevMeasure = nullptr;
    Ms::MeasureBase* curMeasure = nullptr;
    Ms::MeasureBase* nextMeasure = nullptr;
    int measureNo = 0;
    Ms::Fraction startTick;
    Ms::Fraction endTick;

    LayoutContext(Ms::Score* s);
    LayoutContext(const LayoutContext&) = delete;
    LayoutContext& operator=(const LayoutContext&) = delete;
    ~LayoutContext();
};
}

#endif // MU_ENGRAVING_LAYOUTCONTEXT_H
