/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "scoreverticalviewlayout.h"

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
using namespace mu::engraving::rendering::score;

void ScoreVerticalViewLayout::layoutVerticalView(Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick)
{
    ctx.mutState().setEndTick(etick);

    //---------------------------------------------------
    //    initialize layout context lc
    //---------------------------------------------------

    MeasureBase* m = score->tick2measure(stick);
    if (m == 0) {
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

    if (!ctx.state().isLayoutAll() && m->system()) {
        System* system = m->system();
        system_idx_t systemIndex = muse::indexOf(score->systems(), system);
        ctx.mutState().setPage(system->page());
        ctx.mutState().setPageIdx(score->pageIdx(ctx.state().page()));
        if (ctx.state().pageIdx() == muse::nidx) {
            ctx.mutState().setPageIdx(0);
        }
        ctx.mutState().setCurSystem(system);
        ctx.mutState().setSystemList(muse::mid(score->systems(), systemIndex));

        if (systemIndex == 0) {
            ctx.mutState().setNextMeasure(ctx.conf().isShowVBox() ? score->first() : score->firstMeasure());
        } else {
            System* prevSystem = score->systems()[systemIndex - 1];
            ctx.mutState().setNextMeasure(prevSystem->measures().back()->next());
        }

        score->systems().erase(score->systems().begin() + systemIndex, score->systems().end());
        if (!ctx.state().nextMeasure()->prevMeasure()) {
            ctx.mutState().setMeasureNo(0);
            ctx.mutState().setTick(Fraction(0, 1));
        } else {
            const MeasureBase* mb = ctx.state().nextMeasure()->prev();
            if (mb) {
                mb = mb->findPotentialSectionBreak();
            }

            const LayoutBreak* layoutBreak = mb->sectionBreakElement();
            // TODO: also use mb in else clause here?
            // probably not, only actual measures have meaningful numbers
            if (layoutBreak && layoutBreak->startWithMeasureOne()) {
                ctx.mutState().setMeasureNo(0);
            } else {
                ctx.mutState().setMeasureNo(ctx.state().nextMeasure()->prevMeasure()->no()                             // will be adjusted later with respect
                                            + (ctx.state().nextMeasure()->prevMeasure()->irregular() ? 0 : 1)); // to the user-defined offset.
            }
            ctx.mutState().setTick(ctx.state().nextMeasure()->tick());
        }
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
        muse::DeleteAll(score->systems());
        score->systems().clear();

        muse::DeleteAll(score->pages());
        score->pages().clear();

        ctx.mutState().setNextMeasure(ctx.conf().isShowVBox() ? score->first() : score->firstMeasure());
    }

    ctx.mutState().setPrevMeasure(nullptr);

    PassResetLayoutData resetPass;
    resetPass.run(score, ctx);

    MeasureLayout::getNextMeasure(ctx);
    ctx.mutState().setCurSystem(SystemLayout::collectSystem(ctx));

    if (ctx.state().isLayoutAll()) {
        PassLayoutIndependentItems independedPass;
        independedPass.run(score, ctx);
    }

    doLayout(ctx);
}

void ScoreVerticalViewLayout::doLayout(LayoutContext& ctx)
{
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

    if (!ctx.state().curSystem()) {
        // The end of the score. The remaining systems are not needed...
        muse::DeleteAll(ctx.mutState().systemList());
        ctx.mutState().systemList().clear();
        // ...and the remaining pages too
        while (ctx.dom().npages() > ctx.state().pageIdx()) {
            Page* p = ctx.mutDom().pages().back();
            ctx.mutDom().pages().pop_back();
            delete p;
        }
    } else {
        Page* p = ctx.mutState().curSystem()->page();
        if (p && (p != ctx.state().page())) {
            p->invalidateBspTree();
        }
    }
    ctx.mutDom().systems().insert(ctx.mutDom().systems().end(), ctx.state().systemList().begin(), ctx.state().systemList().end());
}
