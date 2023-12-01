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
#include "scorepageviewlayout.h"

#include "containers.h"

#include "dom/score.h"
#include "dom/system.h"
#include "dom/bracket.h"
#include "dom/layoutbreak.h"
#include "dom/page.h"

#include "passresetlayoutdata.h"
#include "passlayoutindependentitems.h"

#include "measurelayout.h"
#include "systemlayout.h"
#include "pagelayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void ScorePageViewLayout::initLayoutContext(const Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick)
{
    LayoutState& state = ctx.mutState();

    state.setEndTick(etick);

    MeasureBase* m = nullptr;
    {
        m = score->tick2measure(stick);
        if (m == nullptr) {
            m = score->first();
        }

        // start layout one measure earlier to handle clefs and cautionary elements
        if (m->prevMeasureMM()) {
            m = m->prevMeasureMM();
        } else if (m->prev()) {
            m = m->prev();
        }

        while (!m->isMeasure() && m->prev()) {
            m = m->prev();
        }

        // if the first measure of the score is part of a multi measure rest
        // m->system() will return a nullptr. We need to find the multi measure
        // rest which replaces the measure range
        if (!m->system() && m->isMeasure() && toMeasure(m)->hasMMRest()) {
            LOGD("  don’t start with mmrest");
            m = toMeasure(m)->mmRest();
        }
    }

    if (!state.isLayoutAll() && m->system()) {
        System* system = m->system();
        system_idx_t systemIndex = mu::indexOf(score->systems(), system);

        // set current system
        state.setCurSystem(system);
        state.setSystemList(mu::mid(score->systems(), systemIndex));

        // set current page
        state.setPage(system->page());
        page_idx_t pageIdx = score->pageIdx(state.page());
        if (pageIdx == mu::nidx) {
            pageIdx = 0;
        }
        state.setPageIdx(pageIdx);

        MeasureBase* nextMeasure = nullptr;
        if (systemIndex == 0) {
            nextMeasure = ctx.conf().isShowVBox() ? score->first() : score->firstMeasure();
        } else {
            System* prevSystem = score->systems().at(systemIndex - 1);
            nextMeasure = prevSystem->measures().back()->next();
        }
        state.setNextMeasure(nextMeasure);

        if (!state.nextMeasure()->prevMeasure()) {
            state.setMeasureNo(0);
            state.setTick(Fraction(0, 1));
        } else {
            const MeasureBase* mb = state.nextMeasure()->prev();
            if (mb) {
                mb = mb->findPotentialSectionBreak();
            }

            const LayoutBreak* layoutBreak = mb->sectionBreakElement();
            // TODO: also use mb in else clause here?
            // probably not, only actual measures have meaningful numbers
            if (layoutBreak && layoutBreak->startWithMeasureOne()) {
                state.setMeasureNo(0);
            } else {
                state.setMeasureNo(state.nextMeasure()->prevMeasure()->no()                             // will be adjusted later with respect
                                   + (state.nextMeasure()->prevMeasure()->irregular() ? 0 : 1));          // to the user-defined offset.
            }
            state.setTick(ctx.state().nextMeasure()->tick());
        }
    } else {
        state.setNextMeasure(ctx.conf().isShowVBox() ? score->first() : score->firstMeasure());
    }

    state.setPrevMeasure(nullptr);
}

void ScorePageViewLayout::prepareScore(Score* score, const LayoutContext& ctx)
{
    if (!ctx.state().isLayoutAll() && ctx.state().curSystem()) {
        system_idx_t systemIndex = mu::indexOf(score->systems(), ctx.state().curSystem());
        score->systems().erase(score->systems().begin() + systemIndex, score->systems().end());
    } else {
        for (System* s : score->systems()) {
            for (Bracket* b : s->brackets()) {
                if (b->selected()) {
                    bool selected = b->selected();
                    score->selection().remove(b);
                    BracketItem* item = b->bracketItem();
                    item->setSelected(selected);
                    score->setSelectionChanged(true);
                }
            }
            s->resetExplicitParent();
        }

        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            mb->resetExplicitParent();
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                toMeasure(mb)->mmRest()->moveToDummy();
            }
        }
        DeleteAll(score->systems());
        score->systems().clear();

        DeleteAll(score->pages());
        score->pages().clear();
    }
}

void ScorePageViewLayout::layoutPageView(Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick)
{
    TRACEFUNC;

    initLayoutContext(score, ctx, stick, etick);

    prepareScore(score, ctx);

    //! NOTE Reset pass need anyway
//#ifdef MUE_ENABLE_ENGRAVING_LD_PASSES
    PassResetLayoutData resetPass;
    resetPass.run(score, ctx);
//#endif

#ifdef MUE_ENABLE_ENGRAVING_LD_PASSES
    if (ctx.state().isLayoutAll()) {
        PassLayoutIndependentItems independentPass;
        independentPass.run(score, ctx);
    }
#endif

    doLayout(ctx);

    layoutFinished(score, ctx);
}

void ScorePageViewLayout::doLayout(LayoutContext& ctx)
{
    MeasureLayout::getNextMeasure(ctx);
    ctx.mutState().setCurSystem(SystemLayout::collectSystem(ctx));

    const MeasureBase* lmb = nullptr;
    do {
        PageLayout::getNextPage(ctx);
        PageLayout::collectPage(ctx);

        if (ctx.state().page() && !ctx.state().page()->systems().empty()) {
            lmb = ctx.state().page()->systems().back()->measures().back();
        } else {
            lmb = nullptr;
        }

        // we can stop collecting pages when:
        // 1) we reach the end of score (curSystem is nullptr)
        // or
        // 2) we have fully processed the range and reached a point of stability:
        //    a) we have completed layout for the range (rangeDone is true)
        //    b) we haven't collected a system that will need to go on the next page
        //    c) this page ends with the same measure as the previous layout
        //    pageOldMeasure will be last measure from previous layout if range was completed on or before this page
        //    it will be nullptr if this page was never laid out or if we collected a system for next page
    } while (ctx.state().curSystem() && !(ctx.state().rangeDone() && lmb == ctx.state().pageOldMeasure()));
    // && page->system(0)->measures().back()->tick() > endTick // FIXME: perhaps the first measure was meant? Or last system?
}

void ScorePageViewLayout::layoutFinished(Score* score, LayoutContext& ctx)
{
    LayoutState& state = ctx.mutState();

    if (!state.curSystem()) {
        // The end of the score. The remaining systems are not needed...
        DeleteAll(state.systemList());
        state.systemList().clear();

        // ...and the remaining pages too
        while (score->npages() > state.pageIdx()) {
            Page* p = score->pages().back();
            score->pages().pop_back();
            delete p;
        }
    } else {
        Page* p = state.curSystem()->page();
        if (p && (p != state.page())) {
            p->invalidateBspTree();
        }
    }

    score->systems().insert(score->systems().end(), state.systemList().begin(), state.systemList().end());
}
