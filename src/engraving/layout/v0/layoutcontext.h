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

#include "../layoutoptions.h"

namespace mu::engraving {
class EngravingItem;
class RootItem;
class MeasureBase;
class Part;
class Page;
class Score;
class Spanner;
class SpannerMap;
class System;
class Staff;
class Measure;
class ChordRest;

class UndoCommand;
class EditData;
}

namespace mu::engraving::compat {
class DummyElement;
}

namespace mu::engraving::layout::v0 {
class DomAccessor
{
public:

    DomAccessor(Score* s);

    // Const access
    const std::vector<Part*>& parts() const;

    size_t npages() const;
    const std::vector<Page*>& pages() const;

    const std::vector<System*>& systems() const;

    size_t nstaves() const;
    const std::vector<Staff*>& staves() const;
    const Staff* staff(staff_idx_t idx) const;

    size_t ntracks() const;

    const Measure* tick2measure(const Fraction& tick) const;
    const Measure* firstMeasure() const;

    const SpannerMap& spannerMap() const;

    const ChordRest* findCR(Fraction tick, track_idx_t track) const;

    // Mutable access
    std::vector<Page*>& pages();
    std::vector<System*>& systems();

    MeasureBase* first();
    Measure* firstMeasure();

    ChordRest* findCR(Fraction tick, track_idx_t track);

    // Create/Remove
    RootItem* rootItem() const;
    compat::DummyElement* dummyParent() const;
    void undoAddElement(EngravingItem* item, bool addToLinkedStaves = true, bool ctrlModifier = false);
    void undoRemoveElement(EngravingItem* item);
    void undo(UndoCommand* cmd, EditData* ed = nullptr) const;
    void addElement(EngravingItem* item);
    void removeElement(EngravingItem* item);

    void addUnmanagedSpanner(Spanner* s);
    const std::set<Spanner*> unmanagedSpanners();

private:
    Score* m_score = nullptr;
};

class LayoutState
{
public:
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
};

class LayoutContext
{
public:
    LayoutContext(Score* s);
    ~LayoutContext();

    LayoutContext(const LayoutContext&) = delete;
    LayoutContext& operator=(const LayoutContext&) = delete;

    bool isValid() const;

    // Context
    bool isPaletteMode() const;
    bool printingMode() const;
    LayoutMode layoutMode() const;
    bool lineMode() const;
    bool linearMode() const;
    bool floatMode() const;

    double spatium() const;
    double point(const Spatium sp) const;

    const MStyle& style() const;
    double noteHeadWidth() const;
    bool showInvisible() const;
    int pageNumberOffset() const;
    bool enableVerticalSpread() const;
    double maxSystemDistance() const;

    IEngravingFontPtr engravingFont() const;

    // Dom access
    const DomAccessor& dom() const;
    DomAccessor& mutDom();

    // State
    const LayoutState& state() const;
    LayoutState& mutState();

    // Mark
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e);
    void addRefresh(const mu::RectF& r);

private:
    Score* m_score = nullptr;
    DomAccessor m_dom;
    LayoutState m_state;
};
}

#endif // MU_ENGRAVING_LAYOUTCONTEXT_H
