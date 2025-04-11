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
#include "pagelayout.h"

#include "realfn.h"
#include "defer.h"

#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/box.h"
#include "dom/bracketItem.h"
#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/durationelement.h"
#include "dom/factory.h"
#include "dom/fingering.h"
#include "dom/measure.h"
#include "dom/measurebase.h"
#include "dom/note.h"
#include "dom/page.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/slur.h"
#include "dom/spacer.h"
#include "dom/spannermap.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/systemdivider.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tuplet.h"

#include "arpeggiolayout.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "masklayout.h"
#include "measurelayout.h"
#include "slurtielayout.h"
#include "systemlayout.h"
#include "tlayout.h"
#include "tupletlayout.h"
#include "verticalgapdata.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

//---------------------------------------------------------
//   getNextPage
//---------------------------------------------------------

void PageLayout::getNextPage(LayoutContext& ctx)
{
    LayoutState& state = ctx.mutState();
    DomAccessor& dom = ctx.mutDom();

    if (!state.page() || state.pageIdx() >= dom.npages()) {
        state.setPage(Factory::createPage(dom.rootItem()));
        dom.pages().push_back(state.page());
        state.setPrevSystem(nullptr);
        state.setPageOldMeasure(nullptr);
    } else {
        state.setPage(dom.pages()[state.pageIdx()]);
        std::vector<System*>& systems = state.page()->systems();
        state.setPageOldMeasure(systems.empty() || systems.back()->measures().empty() ? nullptr : systems.back()->measures().back());
        const system_idx_t i = muse::indexOf(systems, state.curSystem());
        if ((i < systems.size()) && i > 0 && systems[i - 1]->page() == state.page()) {
            // Current and previous systems are on the current page.
            // Erase only the current and the following systems
            // as the previous one will not participate in layout.
            systems.erase(systems.begin() + i, systems.end());
        } else { // system is not on the current page (or will be the first one)
            systems.clear();
        }
        state.setPrevSystem(systems.empty() ? nullptr : systems.back());
    }
    state.page()->mutldata()->setBbox(0.0, 0.0, ctx.conf().loWidth(), ctx.conf().loHeight());
    state.page()->setNo(state.pageIdx());
    double x = 0.0;
    double y = 0.0;
    if (state.pageIdx()) {
        const Page* prevPage = dom.pages().at(state.pageIdx() - 1);
        if (MScore::verticalOrientation()) {
            y = prevPage->pos().y() + state.page()->height() + MScore::verticalPageGap;
        } else {
            double gap = (state.pageIdx() + ctx.conf().pageNumberOffset())
                         & 1 ? MScore::horizontalPageGapOdd : MScore::horizontalPageGapEven;
            x = prevPage->pos().x() + state.page()->width() + gap;
        }
    }
    state.setPageIdx(state.pageIdx() + 1);
    state.page()->setPos(x, y);
}

//---------------------------------------------------------
//   collectPage
//---------------------------------------------------------

void PageLayout::collectPage(LayoutContext& ctx)
{
    TRACEFUNC;

    Page* page = ctx.mutState().page();
    const LayoutConfiguration& conf = ctx.conf();

    LAYOUT_CALL() << "page->no: " << page->no();

    const double slb = conf.styleMM(Sid::staffLowerBorder);
    bool breakPages = conf.viewMode() != LayoutMode::SYSTEM;
    double footerExtension = page->footerExtension();
    double headerExtension = page->headerExtension();
    double headerFooterPadding = conf.styleMM(Sid::staffHeaderFooterPadding);
    double endY = page->height() - page->bm();
    double y = 0.0;

    System* nextSystem = 0;
    int systemIdx = -1;

    // re-calculate positions for systems before current
    // (they may have been filled on previous layout)
    size_t pSystems = page->systems().size();
    if (pSystems > 0) {
        SystemLayout::restoreLayout2(page->system(0), ctx);
        y = page->system(0)->y() + page->system(0)->height();
    } else {
        y = page->tm();
    }

    for (size_t i = 1; i < pSystems; ++i) {
        System* cs = page->system(i);
        System* ps = page->system(i - 1);
        double distance = SystemLayout::minDistance(ps, cs, ctx);
        y += distance;
        cs->setPos(page->lm(), y);
        SystemLayout::restoreLayout2(cs, ctx);
        y += cs->height();
    }

    for (;;) {
        //
        // calculate distance to previous system
        //
        double distance;
        if (ctx.state().prevSystem()) {
            distance = SystemLayout::minDistance(ctx.state().prevSystem(), ctx.state().curSystem(), ctx);
        } else {
            // this is the first system on page
            if (ctx.state().curSystem()->vbox()) {
                // if the header exists and there is a frame, move the frame downwards
                // to avoid collisions
                distance = headerExtension ? headerExtension + headerFooterPadding : 0.0;
            } else {
                distance = ctx.conf().styleMM(Sid::staffUpperBorder);
                bool fixedDistance = false;
                for (MeasureBase* mb : ctx.mutState().curSystem()->measures()) {
                    if (mb->isMeasure()) {
                        Measure* m = toMeasure(mb);
                        Spacer* sp = m->vspacerUp(0);
                        if (sp) {
                            if (sp->spacerType() == SpacerType::FIXED) {
                                distance = sp->gap();
                                fixedDistance = true;
                                break;
                            } else {
                                distance = std::max(distance, sp->gap().val());
                            }
                        }
                    }
                }
                if (!fixedDistance) {
                    double top = ctx.state().curSystem()->minTop();
                    // ensure it doesn't collide with header
                    if (headerExtension > 0.0) {
                        top += headerExtension + headerFooterPadding;
                    }
                    distance = std::max(distance, top);
                }
            }
        }

        y += distance;
        ctx.mutState().curSystem()->setPos(ctx.state().page()->lm(), y);
        SystemLayout::restoreLayout2(ctx.mutState().curSystem(), ctx);
        ctx.mutState().page()->appendSystem(ctx.mutState().curSystem());
        y += ctx.state().curSystem()->height();

        //
        //  check for page break or if next system will fit on page
        //
        bool collected = false;
        if (ctx.state().rangeDone()) {
            // take next system unchanged
            if (systemIdx > 0) {
                nextSystem = muse::value(ctx.mutDom().systems(), systemIdx++);
                if (!nextSystem) {
                    // TODO: handle next movement
                }
            } else {
                nextSystem = ctx.state().systemList().empty() ? 0 : muse::takeFirst(ctx.mutState().systemList());
                if (nextSystem) {
                    ctx.mutDom().systems().push_back(nextSystem);
                }
            }
        } else {
            nextSystem = SystemLayout::collectSystem(ctx);
            if (nextSystem) {
                collected = true;
            }
        }

        if (ctx.dom().lastSegment()) {
            float curPercent = static_cast<float>(ctx.state().measureNo()) / ctx.dom().lastSegment()->measure()->index();
            page->score()->layoutProgressChannel().send(curPercent);
        }

        ctx.mutState().setPrevSystem(ctx.mutState().curSystem());
        assert(ctx.state().curSystem() != nextSystem);
        ctx.mutState().setCurSystem(nextSystem);

        bool isPageBreak = !ctx.state().curSystem() || (breakPages && ctx.state().prevSystem()->pageBreak());

        if (!isPageBreak) {
            double dist = SystemLayout::minDistance(ctx.state().prevSystem(), ctx.state().curSystem(), ctx)
                          + ctx.state().curSystem()->height();
            Box* vbox = ctx.state().curSystem()->vbox();
            if (vbox) {
                if (footerExtension > 0) {
                    dist += footerExtension;
                }
            } else if (!ctx.state().prevSystem()->hasFixedDownDistance()) {
                double margin = std::max(ctx.state().curSystem()->minBottom(), ctx.state().curSystem()->spacerDistance(false));
                // ensure it doesn't collide with footer
                if (footerExtension > 0) {
                    margin += footerExtension + headerFooterPadding;
                }
                dist += std::max(margin, slb);
            }
            isPageBreak = (y + dist) >= endY && breakPages;
        }
        if (isPageBreak) {
            double dist = std::max(ctx.state().prevSystem()->minBottom(), ctx.state().prevSystem()->spacerDistance(false));
            double footerPadding = 0.0;
            // ensure it doesn't collide with footer
            if (footerExtension > 0) {
                footerPadding = footerExtension + headerFooterPadding;
                dist += footerPadding;
            }
            dist = std::max(dist, slb);
            layoutPage(ctx, page, endY - (y + dist), footerPadding);
            // if we collected a system we cannot fit onto this page,
            // we need to collect next page in order to correctly set system positions
            if (collected) {
                ctx.mutState().setPageOldMeasure(nullptr);
            }
            break;
        }
    }

    // All staves have been hidden, so correct last system's header
    System* lastSys = ctx.mutState().page()->systems().back();
    Measure* lastSysMeasure = lastSys->firstMeasure();
    if (lastSysMeasure) {
        MeasureLayout::addSystemHeader(lastSysMeasure, false, ctx);
    }

    Fraction stick2 = Fraction(-1, 1);
    for (System* s : page->systems()) {
        for (MeasureBase* mb : s->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            if (stick2 == Fraction(-1, 1)) {
                stick2 = m->tick();
            }

            for (size_t track = 0; track < ctx.dom().ntracks(); ++track) {
                for (Segment* segment = m->first(); segment; segment = segment->next()) {
                    EngravingItem* e2 = segment->element(track);
                    if (!e2) {
                        continue;
                    }
                    if (e2->isChordRest()) {
                        if (!ctx.dom().staff(track2staff(track))->show()) {
                            continue;
                        }
                        ChordRest* cr = toChordRest(e2);
                        if (BeamLayout::isStartOfCrossBeam(cr)) {                           // layout cross staff beams
                            TLayout::layoutBeam(cr->beam(), ctx);
                            BeamLayout::checkCrossPosAndStemConsistency(cr->beam(), ctx);
                        }
                        if (TupletLayout::notTopTuplet(cr)) {
                            // fix layout of tuplets
                            DurationElement* de = cr;
                            while (de->tuplet() && de->tuplet()->elements().front() == de) {
                                Tuplet* t = de->tuplet();
                                TLayout::layoutTuplet(t, ctx);
                                de = t;
                            }
                        }

                        if (cr->isChord()) {
                            Chord* c = toChord(cr);
                            for (Chord* cc : c->graceNotes()) {
                                if (cc->beam() && cc->beam()->elements().front() == cc) {
                                    TLayout::layoutBeam(cc->beam(), ctx);
                                    BeamLayout::checkCrossPosAndStemConsistency(cc->beam(), ctx);
                                }
                                ChordLayout::layoutSpanners(cc, ctx);
                                for (EngravingItem* element : cc->el()) {
                                    if (element->isSlur()) {
                                        TLayout::layoutItem(element, ctx);
                                    }
                                }
                            }
                            ArpeggioLayout::layoutArpeggio2(c->arpeggio(), ctx);
                            ChordLayout::layoutSpanners(c, ctx);
                            if (c->tremoloTwoChord()) {
                                TremoloTwoChord* t = c->tremoloTwoChord();
                                Chord* c1 = t->chord1();
                                Chord* c2 = t->chord2();
                                if (c1 && c2 && (c1->staffMove() || c2->staffMove())) {
                                    // For cross-staff tremolo, vertical justification may have changed
                                    // staff distances, and therefore also stem directions, so re-compute them
                                    ChordLayout::computeUp(c1, ctx);     // This will also call a tremolo layout
                                    ChordLayout::computeUp(c2, ctx);
                                }
                            }
                        }
                    } else if (e2->isBarLine()) {
                        TLayout::layoutBarLine2(toBarLine(e2), ctx);
                    }
                }
            }
            MeasureLayout::layout2(m, ctx);
        }
        SystemLayout::layoutSystemLockIndicators(s, ctx);
    }

    // If this is the last page we layout, we must also relayout the first barlines of the
    // next page, because they may have been altered while collecting the systems.
    MeasureBase* lastOfThisPage = ctx.mutState().page()->systems().back()->measures().back();
    MeasureBase* firstOfNextPage = lastOfThisPage ? lastOfThisPage->next() : nullptr;
    if (firstOfNextPage && firstOfNextPage->isMeasure() && firstOfNextPage->tick() > ctx.state().endTick()) {
        for (Segment& segment : toMeasure(firstOfNextPage)->segments()) {
            if (!segment.isType(SegmentType::BarLineType)) {
                continue;
            }
            for (EngravingItem* item : segment.elist()) {
                if (item && item->isBarLine()) {
                    TLayout::layoutBarLine2(toBarLine(item), ctx);
                }
            }
        }
    }

    if (ctx.conf().isMode(LayoutMode::SYSTEM)) {
        const System* s = page->systems().back();
        double height = s ? s->pos().y() + s->height() + s->minBottom() : ctx.state().page()->tm();
        page->mutldata()->setBbox(0.0, 0.0, ctx.conf().loWidth(), height + ctx.state().page()->bm());
    }

    layoutCrossStaffElements(ctx, page);

    for (const System* system : page->systems()) {
        SystemLayout::centerElementsBetweenStaves(system);
    }

    if (ctx.conf().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() == TimeSigPlacement::ACROSS_STAVES
        && ctx.conf().styleB(Sid::timeSigCenterAcrossStaveGroup)) {
        for (const System* system : page->systems()) {
            SystemLayout::centerBigTimeSigsAcrossStaves(system);
        }
    }

    MaskLayout::computeMasks(ctx, page);

    page->invalidateBspTree();
}

void PageLayout::layoutCrossStaffElements(LayoutContext& ctx, Page* page)
{
    for (System* system : page->systems()) {
        if (!system->firstMeasure()) {
            continue;
        }

        Fraction stick = system->firstMeasure()->tick();
        Fraction etick = system->endTick();

        IF_ASSERT_FAILED(stick < etick) {
            continue;
        }

        if (etick <= ctx.state().startTick() || etick > ctx.state().tick()) {
            continue;
        }

        layoutCrossStaffSlurs(ctx, system);

        layoutArticAndFingeringOnCrossStaffBeams(ctx, system);
    }
}

void PageLayout::layoutCrossStaffSlurs(LayoutContext& ctx, System* system)
{
    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->endTick();

    auto spanners = ctx.dom().spannerMap().findOverlapping(stick.ticks(), etick.ticks());
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (!sp->isSlur() || sp->tick() == system->endTick()) {
            continue;
        }
        if (toSlur(sp)->isCrossStaff()) {
            SlurTieLayout::layoutSystem(toSlur(sp), system, ctx);
        }
    }
}

void PageLayout::layoutArticAndFingeringOnCrossStaffBeams(LayoutContext& ctx, System* system)
{
    for (const MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (const Segment& segment : toMeasure(mb)->segments()) {
            if (!segment.isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : segment.elist()) {
                if (!item || !item->isChord()) {
                    continue;
                }

                Chord* c = toChord(item);
                bool hasCrossBeam = c->beam() && (c->beam()->cross() || c->staffMove() != 0);
                if (!hasCrossBeam) {
                    continue;
                }

                ChordLayout::layoutArticulations(c, ctx);
                ChordLayout::layoutArticulations2(c, ctx, true);

                for (Note* note : c->notes()) {
                    for (EngravingItem* e : note->el()) {
                        if (!e || !e->isFingering()) {
                            continue;
                        }
                        Fingering* fingering = toFingering(e);
                        if (fingering->isOnCrossBeamSide()) {
                            TLayout::layoutFingering(fingering, fingering->mutldata());
                            if (fingering->addToSkyline()) {
                                const Note* n = fingering->note();
                                const RectF r = fingering->ldata()->bbox().translated(
                                    fingering->pos() + n->pos() + n->chord()->pos() + segment.pos()
                                    + segment.measure()->pos());
                                system->staff(fingering->note()->chord()->vStaffIdx())->skyline().add(r, fingering);
                            }
                        }
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutPage
//    restHeight - vertical space which has to be distributed
//                 between systems
//    The algorithm tries to produce most equally spaced
//    systems.
//---------------------------------------------------------

void PageLayout::layoutPage(LayoutContext& ctx, Page* page, double restHeight, double footerPadding)
{
    TRACEFUNC;
    if (restHeight < 0.0) {
        LOGN("restHeight < 0.0: %f\n", restHeight);
        restHeight = 0;
    }

    int gaps = static_cast<int>(page->systems().size()) - 1;

    std::vector<System*> sList;

    // build list of systems (excluding last)
    // set initial distance for each to the unstretched minimum distance to next
    for (int i = 0; i < gaps; ++i) {
        System* s1 = page->systems().at(i);
        System* s2 = page->systems().at(i + 1);
        s1->setDistance(s2->y() - s1->y());
        if (s1->vbox() || s2->vbox() || s1->hasFixedDownDistance()) {
            if (s2->vbox()) {
                checkDivider(ctx, true, s1, 0.0, true);              // remove
                checkDivider(ctx, false, s1, 0.0, true);             // remove
                checkDivider(ctx, true, s2, 0.0, true);              // remove
                checkDivider(ctx, false, s2, 0.0, true);             // remove
            }
            continue;
        }
        sList.push_back(s1);
    }

    // last system needs no divider
    System* lastSystem = page->systems().back();
    checkDivider(ctx, true, lastSystem, 0.0, true);        // remove
    checkDivider(ctx, false, lastSystem, 0.0, true);       // remove

    if (sList.empty() || MScore::noVerticalStretch || ctx.conf().isVerticalSpreadEnabled()
        || ctx.conf().viewMode() == LayoutMode::SYSTEM) {
        if (ctx.conf().viewMode() == LayoutMode::FLOAT) {
            for (System* system : page->systems()) {
                system->move(PointF(0.0, 0.0));
            }
        } else if ((ctx.conf().viewMode() != LayoutMode::SYSTEM) && ctx.conf().isVerticalSpreadEnabled()) {
            distributeStaves(ctx, page, footerPadding);
        }

        // system dividers
        for (int i = 0; i < gaps; ++i) {
            System* s1 = page->systems().at(i);
            System* s2 = page->systems().at(i + 1);
            if (!(s1->vbox() || s2->vbox())) {
                double yOffset = s1->height() + (s1->distance() - s1->height()) * .5;
                checkDivider(ctx, true,  s1, yOffset);
                checkDivider(ctx, false, s1, yOffset);
            }
        }
        return;
    }

    double maxDist = ctx.conf().maxSystemDistance();

    // allocate space as needed to normalize system distance (bottom of one system to top of next)
    std::sort(sList.begin(), sList.end(), [](System* a, System* b) { return a->distance() - a->height() < b->distance() - b->height(); });
    System* s0 = sList[0];
    double dist = s0->distance() - s0->height();             // distance for shortest system
    for (size_t i = 1; i < sList.size(); ++i) {
        System* si = sList[i];
        double ndist = si->distance() - si->height();        // next taller system
        double fill  = ndist - dist;                         // amount by which this system distance exceeds next shorter
        if (fill > 0.0) {
            double totalFill = fill * static_cast<double>(i); // space required to add this amount to all shorter systems
            if (totalFill > restHeight) {
                totalFill = restHeight;                     // too much; adjust amount
                fill = restHeight / static_cast<double>(i);
            }
            for (size_t k = 0; k < i; ++k) {                   // add amount to all shorter systems
                System* s = sList[k];
                double d = s->distance() + fill;
                if ((d - s->height()) > maxDist) {          // but don't exceed max system distance
                    d = std::max(maxDist + s->height(), s->distance());
                }
                s->setDistance(d);
            }
            restHeight -= totalFill;                        // reduce available space for next iteration
            if (restHeight <= 0) {
                break;                                      // no space left
            }
        }
        dist = ndist;                                       // set up for next iteration
    }

    if (restHeight > 0.0) {                                 // space left?
        double fill = restHeight / static_cast<double>(sList.size());
        for (System* s : sList) {                           // allocate it to systems equally
            double d = s->distance() + fill;
            if ((d - s->height()) > maxDist) {              // but don't exceed max system distance
                d = std::max(maxDist + s->height(), s->distance());
            }
            s->setDistance(d);
        }
    }

    double y = page->systems().at(0)->y();
    for (int i = 0; i < gaps; ++i) {
        System* s1  = page->systems().at(i);
        System* s2  = page->systems().at(i + 1);
        s1->mutldata()->setPosY(y);
        y          += s1->distance();

        if (!(s1->vbox() || s2->vbox())) {
            double yOffset = s1->height() + (s1->distance() - s1->height()) * .5;
            checkDivider(ctx, true,  s1, yOffset);
            checkDivider(ctx, false, s1, yOffset);
        }
    }
    page->systems().back()->mutldata()->setPosY(y);
}

void PageLayout::checkDivider(LayoutContext& ctx, bool left, System* s, double yOffset, bool remove)
{
    SystemDivider* divider = left ? s->systemDividerLeft() : s->systemDividerRight();
    SystemDivider::LayoutData* dividerLdata = nullptr;
    if ((ctx.conf().styleB(left ? Sid::dividerLeft : Sid::dividerRight)) && !remove) {
        if (!divider) {
            divider = new SystemDivider(s);
            divider->setDividerType(left ? SystemDivider::Type::LEFT : SystemDivider::Type::RIGHT);
            divider->setGenerated(true);
            s->add(divider);
        }
        dividerLdata = divider->mutldata();
        TLayout::layoutSystemDivider(divider, divider->mutldata(), ctx);
        dividerLdata->setPosY(divider->height() * .5 + yOffset);
        if (left) {
            dividerLdata->moveY(ctx.conf().styleD(Sid::dividerLeftY) * SPATIUM20);
            dividerLdata->setPosX(ctx.conf().styleD(Sid::dividerLeftX) * SPATIUM20);
        } else {
            dividerLdata->moveY(ctx.conf().styleD(Sid::dividerRightY) * SPATIUM20);
            dividerLdata->setPosX(ctx.conf().styleD(Sid::pagePrintableWidth) * DPI - divider->width());
            dividerLdata->moveX(ctx.conf().styleD(Sid::dividerRightX) * SPATIUM20);
        }
    } else if (divider) {
        if (divider->generated()) {
            s->remove(divider);
            delete divider;
        } else {
            ctx.mutDom().undoRemoveElement(divider);
        }
    }
}

void PageLayout::distributeStaves(LayoutContext& ctx, Page* page, double footerPadding)
{
    VerticalGapDataList vgdl;

    // Find and classify all gaps between staves.
    int ngaps { 0 };
    double prevYBottom  { page->tm() };
    double yBottom      { 0.0 };
    double spacerOffset { 0.0 };
    bool vbox          { false };
    Spacer* nextSpacer { nullptr };
    bool transferNormalBracket { false };
    bool transferCurlyBracket  { false };
    for (System* system : page->systems()) {
        if (system->vbox()) {
            VerticalGapData* vgd = new VerticalGapData(&ctx.conf().style(), !ngaps++, system, nullptr, nullptr, nullptr, prevYBottom);
            vgd->addSpaceAroundVBox(true);
            prevYBottom = system->y();
            yBottom     = system->y() + system->height();
            vbox        = true;
            vgdl.push_back(vgd);
            transferNormalBracket = false;
            transferCurlyBracket  = false;
        } else {
            bool newSystem       { true };
            bool addSpaceAroundNormalBracket { false };
            bool addSpaceAroundCurlyBracket  { false };
            int endNormalBracket { -1 };
            int endCurlyBracket  { -1 };
            int staffNr { -1 };
            for (SysStaff* sysStaff : system->staves()) {
                const Staff* staff = ctx.dom().staff(++staffNr);
                IF_ASSERT_FAILED(staff) {
                    break;
                }
                addSpaceAroundNormalBracket |= endNormalBracket == staffNr;
                addSpaceAroundCurlyBracket  |= endCurlyBracket == staffNr;
                for (const BracketItem* bi : staff->brackets()) {
                    if (bi->bracketType() == BracketType::NORMAL) {
                        addSpaceAroundNormalBracket |= int(staff->idx()) > (endNormalBracket - 1);
                        endNormalBracket = std::max(endNormalBracket, int(staff->idx() + bi->bracketSpan()));
                    } else if (bi->bracketType() == BracketType::BRACE) {
                        addSpaceAroundCurlyBracket |= int(staff->idx()) > (endCurlyBracket - 1);
                        endCurlyBracket = std::max(endCurlyBracket, int(staff->idx() + bi->bracketSpan()));
                    }
                }

                if (!sysStaff->show()) {
                    continue;
                }

                VerticalGapData* vgd
                    = new VerticalGapData(&ctx.conf().style(), !ngaps++, system, staff, sysStaff, nextSpacer, prevYBottom);
                nextSpacer = system->downSpacer(staff->idx());

                if (newSystem) {
                    vgd->addSpaceBetweenSections();
                    newSystem = false;
                }
                if (addSpaceAroundNormalBracket || transferNormalBracket) {
                    vgd->addSpaceAroundNormalBracket();
                    addSpaceAroundNormalBracket = false;
                    transferNormalBracket = false;
                }
                if (addSpaceAroundCurlyBracket || transferCurlyBracket) {
                    vgd->addSpaceAroundCurlyBracket();
                    addSpaceAroundCurlyBracket = false;
                    transferCurlyBracket = false;
                } else if (staffNr < endCurlyBracket) {
                    vgd->insideCurlyBracket();
                }

                if (vbox) {
                    vgd->addSpaceAroundVBox(false);
                    vbox = false;
                }

                prevYBottom  = system->y() + sysStaff->bbox().bottom();
                yBottom      = system->y() + sysStaff->y() + sysStaff->skyline().south().max();
                spacerOffset = sysStaff->skyline().south().max() - sysStaff->bbox().height();
                vgdl.push_back(vgd);
            }
            transferNormalBracket = endNormalBracket >= 0;
            transferCurlyBracket  = endCurlyBracket >= 0;
        }
    }
    --ngaps;
    const double staffLowerBorder = ctx.conf().styleMM(Sid::staffLowerBorder);
    const double combinedBottomMargin = page->bm() + footerPadding;
    const double marginToStaff = page->bm() + staffLowerBorder;
    double spaceRemaining{ std::min(page->height() - combinedBottomMargin - yBottom, page->height() - marginToStaff - prevYBottom) };

    if (nextSpacer) {
        spaceRemaining -= std::max(0.0, nextSpacer->gap() - spacerOffset - staffLowerBorder);
    }
    if (spaceRemaining <= 0.0) {
        return;
    }

    // Try to make the gaps equal, taking the spread factors and maximum spacing into account.
    static const int maxPasses { 20 };     // Saveguard to prevent endless loops.
    int pass { 0 };
    while (!muse::RealIsNull(spaceRemaining) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        double smallest     { vgdl.smallest() };
        double nextSmallest { vgdl.smallest(smallest) };
        if (muse::RealIsNull(smallest) || muse::RealIsNull(nextSmallest)) {
            break;
        }

        if ((nextSmallest - smallest) * vgdl.sumStretchFactor() > spaceRemaining) {
            nextSmallest = smallest + spaceRemaining / vgdl.sumStretchFactor();
        }

        double addedSpace { 0.0 };
        VerticalGapDataList modified;
        for (VerticalGapData* vgd : vgdl) {
            if (!muse::RealIsNull(vgd->spacing() - smallest)) {
                continue;
            }
            double step { nextSmallest - vgd->spacing() };
            if (step < 0.0) {
                continue;
            }
            step = vgd->addSpacing(step);
            if (!muse::RealIsNull(step)) {
                addedSpace += step * vgd->factor();
                modified.push_back(vgd);
                ++ngaps;
            }
            if ((spaceRemaining - addedSpace) <= 0.0) {
                break;
            }
        }
        if ((spaceRemaining - addedSpace) <= 0.0) {
            for (VerticalGapData* vgd : modified) {
                vgd->undoLastAddSpacing();
            }
            ngaps = 0;
        } else {
            spaceRemaining -= addedSpace;
        }
    }

    // If there is still space left, distribute the space of the staves.
    // However, there is a limit on how much space is added per gap.
    const double maxPageFill = ctx.conf().styleMM(Sid::maxPageFillSpread);
    spaceRemaining = std::min(maxPageFill * static_cast<double>(vgdl.size()), spaceRemaining);
    pass = 0;
    ngaps = 1;
    while (!muse::RealIsNull(spaceRemaining) && !muse::RealIsNull(maxPageFill) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        double addedSpace { 0.0 };
        double step { spaceRemaining / vgdl.sumStretchFactor() };
        for (VerticalGapData* vgd : vgdl) {
            double res { vgd->addFillSpacing(step, maxPageFill) };
            if (!muse::RealIsNull(res)) {
                addedSpace += res * vgd->factor();
                ++ngaps;
            }
        }
        spaceRemaining -= addedSpace;
    }

    std::set<System*> systems;
    double systemShift { 0.0 };
    double staffShift  { 0.0 };
    System* prvSystem { nullptr };
    for (VerticalGapData* vgd : vgdl) {
        if (vgd->sysStaff) {
            systems.insert(vgd->system);
        }
        systemShift += vgd->actualAddedSpace();
        if (prvSystem == vgd->system) {
            staffShift += vgd->actualAddedSpace();
        } else {
            vgd->system->mutldata()->moveY(systemShift);
            if (prvSystem) {
                prvSystem->setDistance(vgd->system->y() - prvSystem->y());
                prvSystem->setHeight(prvSystem->height() + staffShift);
            }
            staffShift = 0.0;
        }

        if (vgd->sysStaff) {
            vgd->sysStaff->bbox().translate(0.0, staffShift);
        }

        prvSystem = vgd->system;
    }
    if (prvSystem) {
        prvSystem->setHeight(prvSystem->height() + staffShift);
    }

    for (System* system : systems) {
        SystemLayout::setMeasureHeight(system, system->height(), ctx);
        SystemLayout::layoutBracketsVertical(system, ctx);
        SystemLayout::layoutInstrumentNames(system, ctx);
    }
    vgdl.deleteAll();
}
