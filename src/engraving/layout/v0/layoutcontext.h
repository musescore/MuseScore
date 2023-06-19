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

#include "style/style.h"
#include "iengravingfont.h"

namespace mu::engraving {
class EngravingItem;
class MeasureBase;
class Page;
class Score;
class Spanner;
class System;
class Staff;
class Measure;

class UndoCommand;
class EditData;
}

namespace mu::engraving::compat {
class DummyElement;
}

namespace mu::engraving::layout::v0 {
class LayoutContext
{
public:
    LayoutContext(Score* s);
    LayoutContext(const LayoutContext&) = delete;
    LayoutContext& operator=(const LayoutContext&) = delete;
    ~LayoutContext();

    // Context
    Score* score() const { return m_score; }

    bool isPaletteMode() const;
    bool printingMode() const;
    bool lineMode() const;

    double spatium() const;
    double point(const Spatium sp) const;

    const MStyle& style() const;
    double noteHeadWidth() const;
    bool showInvisible() const;

    IEngravingFontPtr engravingFont() const;

    const std::vector<System*>& systems() const;
    size_t nstaves() const;
    const Staff* staff(staff_idx_t idx) const;
    size_t ntracks() const;
    const Measure* tick2measure(const Fraction& tick) const;

    // Create/Remove
    compat::DummyElement* dummyParent() const;
    void undoRemoveElement(EngravingItem* item);
    void undo(UndoCommand* cmd, EditData* ed = nullptr) const;
    void addUnmanagedSpanner(Spanner* s);

    // Mark
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e);
    void addRefresh(const mu::RectF& r);

    // State
    bool startWithLongNames = true;
    bool firstSystem = true;
    bool firstSystemIndent = true;
    Page* page = nullptr;
    page_idx_t curPage = 0; // index in Score->page()s
    Fraction tick{ 0, 1 };

    std::vector<System*> systemList; // reusable systems
    std::set<Spanner*> processedSpanners;

    System* prevSystem = nullptr; // used during page layout
    System* curSystem = nullptr;

    MeasureBase* systemOldMeasure = nullptr;
    MeasureBase* pageOldMeasure = nullptr;
    bool rangeDone = false;

    MeasureBase* prevMeasure = nullptr;
    MeasureBase* curMeasure = nullptr;
    MeasureBase* nextMeasure = nullptr;
    int measureNo = 0;
    Fraction startTick;
    Fraction endTick;

    double totalBracketsWidth = -1.0;

private:
    Score* m_score = nullptr;
};
}

#endif // MU_ENGRAVING_LAYOUTCONTEXT_H
