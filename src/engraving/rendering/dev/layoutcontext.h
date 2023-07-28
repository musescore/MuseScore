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
#ifndef MU_ENGRAVING_LAYOUTCONTEXT_H
#define MU_ENGRAVING_LAYOUTCONTEXT_H

#include <vector>
#include <set>

#include "types/fraction.h"
#include "types/types.h"

#include "style/style.h"
#include "iengravingfont.h"
#include "libmscore/mscore.h"

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
class Segment;

class UndoCommand;
class EditData;

class Selection;
}

namespace mu::engraving::compat {
class DummyElement;
}

namespace mu::engraving::rendering::dev {
class IGetScoreInternal
{
private:
    friend class LayoutConfiguration;
    friend class DomAccessor;

    virtual Score* score() = 0;
};

class LayoutConfiguration
{
public:
    LayoutConfiguration(IGetScoreInternal* s);

    LayoutMode layoutMode() const { return options().mode; }
    bool isMode(LayoutMode m) const { return options().isMode(m); }
    bool isLineMode() const { return isMode(LayoutMode::LINE); }
    bool isLinearMode() const { return options().isLinearMode(); }
    bool isFloatMode() const { return isMode(LayoutMode::FLOAT); }
    bool isPaletteMode() const;
    bool isPrintingMode() const;

    bool isShowVBox() const { return options().isShowVBox; }
    double noteHeadWidth() const { return options().noteHeadWidth; }
    bool isShowInvisible() const;
    int pageNumberOffset() const;
    bool isVerticalSpreadEnabled() const;
    double maxSystemDistance() const;
    bool isShowInstrumentNames() const;

    const MStyle& style() const;

    const PropertyValue& styleV(Sid idx) const { return style().styleV(idx); }
    Spatium styleS(Sid idx) const { return style().styleS(idx); }
    Millimetre styleMM(Sid idx) const { return style().styleMM(idx); }
    String styleSt(Sid idx) const { return style().styleSt(idx); }
    bool styleB(Sid idx) const { return style().styleB(idx); }
    double styleD(Sid idx) const { return style().styleD(idx); }
    int styleI(Sid idx) const { return style().styleI(idx); }

    double spatium() const { return styleD(Sid::spatium); }
    double point(const Spatium sp) const { return sp.val() * spatium(); }
    double magS(double mag) const { return mag * (spatium() / SPATIUM20); }

    double loWidth() const { return styleD(Sid::pageWidth) * DPI; }
    double loHeight() const { return styleD(Sid::pageHeight) * DPI; }

    bool firstSystemIndent() const { return styleB(Sid::enableIndentationOnFirstSystem); }

    double maxChordShiftAbove() const { return styleMM(Sid::maxChordShiftAbove); }
    double maxChordShiftBelow() const { return styleMM(Sid::maxChordShiftBelow); }

    double maxFretShiftAbove() const { return styleMM(Sid::maxFretShiftAbove); }
    double maxFretShiftBelow() const { return styleMM(Sid::maxFretShiftBelow); }

    VerticalAlignRange verticalAlignRange() const { return style().value(Sid::autoplaceVerticalAlignRange).value<VerticalAlignRange>(); }

private:
    const Score* score() const;
    const LayoutOptions& options() const;

    IGetScoreInternal* m_getScore = nullptr;
};

class DomAccessor
{
public:

    DomAccessor(IGetScoreInternal* s);

    // Const access
    const std::vector<Part*>& parts() const;
    int visiblePartCount() const;

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

    const Segment* lastSegment() const;

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
    const std::set<Spanner*>& unmanagedSpanners() const;

private:
    const Score* score() const;
    Score* score();

    IGetScoreInternal* m_getScore = nullptr;
};

class LayoutState
{
public:

    // Const
    bool firstSystem() const { return m_firstSystem; }
    bool firstSystemIndent() const { return m_firstSystemIndent; }
    bool startWithLongNames() const { return m_startWithLongNames; }

    const Fraction& tick() const { return m_tick; }
    const Fraction& startTick() const { return m_startTick; }
    const Fraction& endTick() const { return m_endTick; }

    const Page* page() const { return m_page; }
    page_idx_t pageIdx() const { return m_pageIdx; }

    const std::vector<System*>& systemList() const { return m_systemList; }
    const System* prevSystem() const { return m_prevSystem; }
    const System* curSystem() const { return m_curSystem; }

    const MeasureBase* prevMeasure() const { return m_prevMeasure; }
    const MeasureBase* curMeasure() const { return m_curMeasure; }
    const MeasureBase* nextMeasure() const { return m_nextMeasure; }
    const MeasureBase* systemOldMeasure() const { return m_systemOldMeasure; }
    const MeasureBase* pageOldMeasure() const { return m_pageOldMeasure; }
    int measureNo() const { return m_measureNo; }

    bool rangeDone() const { return m_rangeDone; }

    double totalBracketsWidth() const { return m_totalBracketsWidth; }

    // Mutable
    void setFirstSystem(bool val) { m_firstSystem = val; }
    void setFirstSystemIndent(bool val) { m_firstSystemIndent = val; }
    void setStartWithLongNames(bool val) { m_startWithLongNames = val; }

    void setTick(const Fraction& t) { m_tick = t; }
    void setStartTick(const Fraction& t) { m_startTick = t; }
    void setEndTick(const Fraction& t) { m_endTick = t; }

    Page* page() { return m_page; }
    void setPage(Page* p) { m_page = p; }
    void setPageIdx(page_idx_t idx) { m_pageIdx = idx; }

    std::vector<System*>& systemList() { return m_systemList; }
    void setSystemList(const std::vector<System*>& l) { m_systemList = l; }
    System* prevSystem() { return m_prevSystem; }
    void setPrevSystem(System* s) { m_prevSystem = s; }
    System* curSystem() { return m_curSystem; }
    void setCurSystem(System* s) { m_curSystem = s; }

    MeasureBase* prevMeasure() { return m_prevMeasure; }
    void setPrevMeasure(MeasureBase* m) { m_prevMeasure = m; }
    MeasureBase* curMeasure() { return m_curMeasure; }
    void setCurMeasure(MeasureBase* m) { m_curMeasure = m; }
    MeasureBase* nextMeasure() { return m_nextMeasure; }
    void setNextMeasure(MeasureBase* m) { m_nextMeasure = m; }
    void setSystemOldMeasure(MeasureBase* m) { m_systemOldMeasure = m; }
    void setPageOldMeasure(MeasureBase* m) { m_pageOldMeasure = m; }
    void setMeasureNo(int no) { m_measureNo = no; }

    std::set<Spanner*>& processedSpanners() { return m_processedSpanners; }

    void setRangeDone(bool val) { m_rangeDone = val; }

    void setTotalBracketsWidth(double val) { m_totalBracketsWidth = val; }

private:

    bool m_firstSystem = true;
    bool m_firstSystemIndent = true;
    bool m_startWithLongNames = true;

    Fraction m_tick{ 0, 1 };
    Fraction m_startTick;
    Fraction m_endTick;

    Page* m_page = nullptr;
    page_idx_t m_pageIdx = 0;               // index in Score->page()s

    std::vector<System*> m_systemList;      // reusable systems
    System* m_prevSystem = nullptr;         // used during page layout
    System* m_curSystem = nullptr;

    MeasureBase* m_prevMeasure = nullptr;
    MeasureBase* m_curMeasure = nullptr;
    MeasureBase* m_nextMeasure = nullptr;
    MeasureBase* m_systemOldMeasure = nullptr;
    MeasureBase* m_pageOldMeasure = nullptr;
    int m_measureNo = 0;

    std::set<Spanner*> m_processedSpanners;

    bool m_rangeDone = false;

    // cache
    double m_totalBracketsWidth = -1.0;
};

class LayoutContext : public IGetScoreInternal
{
public:
    LayoutContext(Score* s);
    ~LayoutContext();

    LayoutContext(const LayoutContext&) = delete;
    LayoutContext& operator=(const LayoutContext&) = delete;

    bool isValid() const;

    // Conf
    IEngravingFontPtr engravingFont() const;
    const LayoutConfiguration& conf() const { return m_configuration; }

    // Dom access
    const DomAccessor& dom() const;
    DomAccessor& mutDom();

    // State
    const LayoutState& state() const;
    LayoutState& mutState();

    // Mark
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e);
    void addRefresh(const mu::RectF& r);

    // Other
    const Selection& selection() const;
    void select(EngravingItem* item, SelectType = SelectType::SINGLE, staff_idx_t staff = 0);
    void deselect(EngravingItem* el);

private:

    Score* score() override { return m_score; }

    Score* m_score = nullptr;

    LayoutConfiguration m_configuration;
    DomAccessor m_dom;
    LayoutState m_state;
};
}

#endif // MU_ENGRAVING_LAYOUTCONTEXT_H
