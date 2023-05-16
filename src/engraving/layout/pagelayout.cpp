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
#include "pagelayout.h"

#include "realfn.h"

#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/box.h"
#include "libmscore/bracketItem.h"
#include "libmscore/chord.h"
#include "libmscore/chordrest.h"
#include "libmscore/durationelement.h"
#include "libmscore/factory.h"
#include "libmscore/fingering.h"
#include "libmscore/measure.h"
#include "libmscore/measurebase.h"
#include "libmscore/note.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/spacer.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/systemdivider.h"
#include "libmscore/tremolo.h"
#include "libmscore/tuplet.h"

#include "tlayout.h"
#include "systemlayout.h"
#include "chordlayout.h"
#include "beamlayout.h"
#include "measurelayout.h"
#include "tupletlayout.h"
#include "verticalgapdata.h"
#include "arpeggiolayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::v0;

//---------------------------------------------------------
//   getNextPage
//---------------------------------------------------------

void PageLayout::getNextPage(const LayoutOptions& options, LayoutContext& lc)
{
    if (!lc.page || lc.curPage >= lc.score()->npages()) {
        lc.page = Factory::createPage(lc.score()->rootItem());
        lc.score()->pages().push_back(lc.page);
        lc.prevSystem = nullptr;
        lc.pageOldMeasure = nullptr;
    } else {
        lc.page = lc.score()->pages()[lc.curPage];
        std::vector<System*>& systems = lc.page->systems();
        lc.pageOldMeasure = systems.empty() ? nullptr : systems.back()->measures().back();
        const system_idx_t i = mu::indexOf(systems, lc.curSystem);
        if (i < systems.size() && i > 0 && systems[i - 1]->page() == lc.page) {
            // Current and previous systems are on the current page.
            // Erase only the current and the following systems
            // as the previous one will not participate in layout.
            systems.erase(systems.begin() + i, systems.end());
        } else { // system is not on the current page (or will be the first one)
            systems.clear();
        }
        lc.prevSystem = systems.empty() ? nullptr : systems.back();
    }
    lc.page->bbox().setRect(0.0, 0.0, options.loWidth, options.loHeight);
    lc.page->setNo(lc.curPage);
    double x = 0.0;
    double y = 0.0;
    if (lc.curPage) {
        Page* prevPage = lc.score()->pages()[lc.curPage - 1];
        if (MScore::verticalOrientation()) {
            y = prevPage->pos().y() + lc.page->height() + MScore::verticalPageGap;
        } else {
            double gap = (lc.curPage + lc.score()->pageNumberOffset()) & 1 ? MScore::horizontalPageGapOdd : MScore::horizontalPageGapEven;
            x = prevPage->pos().x() + lc.page->width() + gap;
        }
    }
    ++lc.curPage;
    lc.page->setPos(x, y);
}

//---------------------------------------------------------
//   collectPage
//---------------------------------------------------------

void PageLayout::collectPage(const LayoutOptions& options, LayoutContext& ctx)
{
    TRACEFUNC;

    const double slb = ctx.score()->styleMM(Sid::staffLowerBorder);
    bool breakPages = ctx.score()->layoutMode() != LayoutMode::SYSTEM;
    double footerExtension = ctx.page->footerExtension();
    double headerExtension = ctx.page->headerExtension();
    double headerFooterPadding = ctx.score()->styleMM(Sid::staffHeaderFooterPadding);
    double endY = ctx.page->height() - ctx.page->bm();
    double y = 0.0;

    System* nextSystem = 0;
    int systemIdx = -1;

    // re-calculate positions for systems before current
    // (they may have been filled on previous layout)
    size_t pSystems = ctx.page->systems().size();
    if (pSystems > 0) {
        ctx.page->system(0)->restoreLayout2();
        y = ctx.page->system(0)->y() + ctx.page->system(0)->height();
    } else {
        y = ctx.page->tm();
    }
    for (int i = 1; i < static_cast<int>(pSystems); ++i) {
        System* cs = ctx.page->system(i);
        System* ps = ctx.page->system(i - 1);
        double distance = ps->minDistance(cs);
        y += distance;
        cs->setPos(ctx.page->lm(), y);
        cs->restoreLayout2();
        y += cs->height();
    }

    for (;;) {
        //
        // calculate distance to previous system
        //
        double distance;
        if (ctx.prevSystem) {
            distance = ctx.prevSystem->minDistance(ctx.curSystem);
        } else {
            // this is the first system on page
            if (ctx.curSystem->vbox()) {
                // if the header exists and there is a frame, move the frame downwards
                // to avoid collisions
                distance = headerExtension ? headerExtension + headerFooterPadding : 0.0;
            } else {
                distance = ctx.score()->styleMM(Sid::staffUpperBorder);
                bool fixedDistance = false;
                for (MeasureBase* mb : ctx.curSystem->measures()) {
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
                    double top = ctx.curSystem->minTop();
                    // ensure it doesn't collide with header
                    if (headerExtension > 0.0) {
                        top += headerExtension + headerFooterPadding;
                    }
                    distance = std::max(distance, top);
                }
            }
        }

        y += distance;
        ctx.curSystem->setPos(ctx.page->lm(), y);
        ctx.curSystem->restoreLayout2();
        ctx.page->appendSystem(ctx.curSystem);
        y += ctx.curSystem->height();

        //
        //  check for page break or if next system will fit on page
        //
        bool collected = false;
        if (ctx.rangeDone) {
            // take next system unchanged
            if (systemIdx > 0) {
                nextSystem = mu::value(ctx.score()->systems(), systemIdx++);
                if (!nextSystem) {
                    // TODO: handle next movement
                }
            } else {
                nextSystem = ctx.systemList.empty() ? 0 : mu::takeFirst(ctx.systemList);
                if (nextSystem) {
                    ctx.score()->systems().push_back(nextSystem);
                }
            }
        } else {
            nextSystem = SystemLayout::collectSystem(options, ctx, ctx.score());
            if (nextSystem) {
                collected = true;
            }
        }
        ctx.prevSystem = ctx.curSystem;
        assert(ctx.curSystem != nextSystem);
        ctx.curSystem  = nextSystem;

        bool breakPage = !ctx.curSystem || (breakPages && ctx.prevSystem->pageBreak());

        if (!breakPage) {
            double dist = ctx.prevSystem->minDistance(ctx.curSystem) + ctx.curSystem->height();
            Box* vbox = ctx.curSystem->vbox();
            if (vbox) {
                dist += vbox->bottomGap();
                if (footerExtension > 0) {
                    dist += footerExtension;
                }
            } else if (!ctx.prevSystem->hasFixedDownDistance()) {
                double margin = std::max(ctx.curSystem->minBottom(), ctx.curSystem->spacerDistance(false));
                // ensure it doesn't collide with footer
                if (footerExtension > 0) {
                    margin += footerExtension + headerFooterPadding;
                }
                dist += std::max(margin, slb);
            }
            breakPage = (y + dist) >= endY && breakPages;
        }
        if (breakPage) {
            double dist = std::max(ctx.prevSystem->minBottom(), ctx.prevSystem->spacerDistance(false));
            double footerPadding = 0.0;
            // ensure it doesn't collide with footer
            if (footerExtension > 0) {
                footerPadding = footerExtension + headerFooterPadding;
                dist += footerPadding;
            }
            dist = std::max(dist, slb);
            layoutPage(ctx, ctx.page, endY - (y + dist), footerPadding);
            // if we collected a system we cannot fit onto this page,
            // we need to collect next page in order to correctly set system positions
            if (collected) {
                ctx.pageOldMeasure = nullptr;
            }
            break;
        }
    }

    Fraction stick = Fraction(-1, 1);
    for (System* s : ctx.page->systems()) {
        Score* currentScore = ctx.score();
        for (MeasureBase* mb : s->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            if (stick == Fraction(-1, 1)) {
                stick = m->tick();
            }

            for (size_t track = 0; track < currentScore->ntracks(); ++track) {
                for (Segment* segment = m->first(); segment; segment = segment->next()) {
                    EngravingItem* e = segment->element(static_cast<int>(track));
                    if (!e) {
                        continue;
                    }
                    if (e->isChordRest()) {
                        if (!currentScore->staff(track2staff(static_cast<int>(track)))->show()) {
                            continue;
                        }
                        ChordRest* cr = toChordRest(e);
                        if (BeamLayout::notTopBeam(cr)) {                           // layout cross staff beams
                            TLayout::layout(cr->beam(), ctx);
                        }
                        if (TupletLayout::notTopTuplet(cr)) {
                            // fix layout of tuplets
                            DurationElement* de = cr;
                            while (de->tuplet() && de->tuplet()->elements().front() == de) {
                                Tuplet* t = de->tuplet();
                                TLayout::layout(t, ctx);
                                de = t;
                            }
                        }

                        if (cr->isChord()) {
                            Chord* c = toChord(cr);
                            for (Chord* cc : c->graceNotes()) {
                                if (cc->beam() && cc->beam()->elements().front() == cc) {
                                    TLayout::layout(cc->beam(), ctx);
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
                            if (c->tremolo()) {
                                Tremolo* t = c->tremolo();
                                Chord* c1 = t->chord1();
                                Chord* c2 = t->chord2();
                                if (t->twoNotes() && c1 && c2 && (c1->staffMove() || c2->staffMove())) {
                                    TLayout::layout(t, ctx);
                                }
                            }
                            // Fingering and articulations on top of cross-staff beams must be laid out here
                            if (c->beam() && (c->beam()->cross() || c->staffMove() != 0)) {
                                ChordLayout::layoutArticulations(c, ctx);
                                ChordLayout::layoutArticulations2(c, ctx, true);

                                for (Note* note : c->notes()) {
                                    for (EngravingItem* e : note->el()) {
                                        if (!e || !e->isFingering()) {
                                            continue;
                                        }
                                        Fingering* fingering = toFingering(e);
                                        if (fingering->isOnCrossBeamSide()) {
                                            TLayout::layout(fingering, ctx);
                                        }
                                    }
                                }
                            }
                        }
                    } else if (e->isBarLine()) {
                        toBarLine(e)->layout2();
                    }
                }
            }
            MeasureLayout::layout2(m, ctx);
        }
    }

    if (options.isMode(LayoutMode::SYSTEM)) {
        System* s = ctx.page->systems().back();
        double height = s ? s->pos().y() + s->height() + s->minBottom() : ctx.page->tm();
        ctx.page->bbox().setRect(0.0, 0.0, options.loWidth, height + ctx.page->bm());
    }

    // HACK: we relayout here cross-staff slurs because only now the information
    // about staff distances is fully available.
    for (System* system : ctx.page->systems()) {
        long int stick = 0;
        long int etick = 0;
        if (system->firstMeasure()) {
            stick = system->firstMeasure()->tick().ticks();
        }
        etick = system->endTick().ticks();
        if (stick == 0 && etick == 0) {
            continue;
        }
        auto spanners = ctx.score()->spannerMap().findOverlapping(stick, etick);
        for (auto interval : spanners) {
            Spanner* sp = interval.value;
            if (!sp->isSlur()) {
                continue;
            }
            if (toSlur(sp)->isCrossStaff()) {
                TLayout::layout(toSlur(sp), ctx);
            }
        }
    }

    ctx.page->invalidateBspTree();
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
    if (restHeight < 0.0) {
        LOGN("restHeight < 0.0: %f\n", restHeight);
        restHeight = 0;
    }

    Score* score = ctx.score();
    int gaps     = static_cast<int>(page->systems().size()) - 1;

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

    if (sList.empty() || MScore::noVerticalStretch || score->enableVerticalSpread() || score->layoutMode() == LayoutMode::SYSTEM) {
        if (score->layoutMode() == LayoutMode::FLOAT) {
            double y = restHeight * .5;
            for (System* system : page->systems()) {
                system->move(PointF(0.0, y));
            }
        } else if ((score->layoutMode() != LayoutMode::SYSTEM) && score->enableVerticalSpread()) {
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

    double maxDist = score->maxSystemDistance();

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
        s1->setPosY(y);
        y          += s1->distance();

        if (!(s1->vbox() || s2->vbox())) {
            double yOffset = s1->height() + (s1->distance() - s1->height()) * .5;
            checkDivider(ctx, true,  s1, yOffset);
            checkDivider(ctx, false, s1, yOffset);
        }
    }
    page->systems().back()->setPosY(y);
}

void PageLayout::checkDivider(LayoutContext& ctx, bool left, System* s, double yOffset, bool remove)
{
    SystemDivider* divider = left ? s->systemDividerLeft() : s->systemDividerRight();
    if ((ctx.score()->styleB(left ? Sid::dividerLeft : Sid::dividerRight)) && !remove) {
        if (!divider) {
            divider = new SystemDivider(s);
            divider->setDividerType(left ? SystemDivider::Type::LEFT : SystemDivider::Type::RIGHT);
            divider->setGenerated(true);
            s->add(divider);
        }
        TLayout::layout(divider, ctx);
        divider->setPosY(divider->height() * .5 + yOffset);
        if (left) {
            divider->movePosY(ctx.score()->styleD(Sid::dividerLeftY) * SPATIUM20);
            divider->setPosX(ctx.score()->styleD(Sid::dividerLeftX) * SPATIUM20);
        } else {
            divider->movePosY(ctx.score()->styleD(Sid::dividerRightY) * SPATIUM20);
            divider->setPosX(ctx.score()->styleD(Sid::pagePrintableWidth) * DPI - divider->width());
            divider->movePosX(ctx.score()->styleD(Sid::dividerRightX) * SPATIUM20);
        }
    } else if (divider) {
        if (divider->generated()) {
            s->remove(divider);
            delete divider;
        } else {
            ctx.score()->undoRemoveElement(divider);
        }
    }
}

void PageLayout::distributeStaves(const LayoutContext& ctx, Page* page, double footerPadding)
{
    Score* score = ctx.score();
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
            VerticalGapData* vgd = new VerticalGapData(&ctx.score()->style(), !ngaps++, system, nullptr, nullptr, nullptr, prevYBottom);
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
                Staff* staff { score->staff(++staffNr) };
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
                    = new VerticalGapData(&ctx.score()->style(), !ngaps++, system, staff, sysStaff, nextSpacer, prevYBottom);
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

                prevYBottom  = system->y() + sysStaff->y() + sysStaff->bbox().height();
                yBottom      = system->y() + sysStaff->y() + sysStaff->skyline().south().max();
                spacerOffset = sysStaff->skyline().south().max() - sysStaff->bbox().height();
                vgdl.push_back(vgd);
            }
            transferNormalBracket = endNormalBracket >= 0;
            transferCurlyBracket  = endCurlyBracket >= 0;
        }
    }
    --ngaps;
    const double staffLowerBorder = score->styleMM(Sid::staffLowerBorder);
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
    while (!RealIsNull(spaceRemaining) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        double smallest     { vgdl.smallest() };
        double nextSmallest { vgdl.smallest(smallest) };
        if (RealIsNull(smallest) || RealIsNull(nextSmallest)) {
            break;
        }

        if ((nextSmallest - smallest) * vgdl.sumStretchFactor() > spaceRemaining) {
            nextSmallest = smallest + spaceRemaining / vgdl.sumStretchFactor();
        }

        double addedSpace { 0.0 };
        VerticalGapDataList modified;
        for (VerticalGapData* vgd : vgdl) {
            if (!RealIsNull(vgd->spacing() - smallest)) {
                continue;
            }
            double step { nextSmallest - vgd->spacing() };
            if (step < 0.0) {
                continue;
            }
            step = vgd->addSpacing(step);
            if (!RealIsNull(step)) {
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
    const double maxPageFill { score->styleMM(Sid::maxPageFillSpread) };
    spaceRemaining = std::min(maxPageFill * static_cast<double>(vgdl.size()), spaceRemaining);
    pass = 0;
    ngaps = 1;
    while (!RealIsNull(spaceRemaining) && !RealIsNull(maxPageFill) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        double addedSpace { 0.0 };
        double step { spaceRemaining / vgdl.sumStretchFactor() };
        for (VerticalGapData* vgd : vgdl) {
            double res { vgd->addFillSpacing(step, maxPageFill) };
            if (!RealIsNull(res)) {
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
            vgd->system->movePosY(systemShift);
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
        system->setMeasureHeight(system->height());
        system->layoutBracketsVertical();
        system->layoutInstrumentNames();
    }
    vgdl.deleteAll();
}
