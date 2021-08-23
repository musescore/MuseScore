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
#include "layoutpage.h"

#include "realfn.h"

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/systemdivider.h"
#include "libmscore/bracketItem.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/spanner.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/chordrest.h"
#include "libmscore/duration.h"
#include "libmscore/spacer.h"
#include "libmscore/box.h"
#include "libmscore/tuplet.h"
#include "libmscore/chord.h"
#include "libmscore/tremolo.h"
#include "libmscore/barline.h"

#include "layoutsystem.h"
#include "layoutbeams.h"
#include "layouttuplets.h"
#include "verticalgapdata.h"

using namespace mu::engraving;
using namespace Ms;

//---------------------------------------------------------
//   getNextPage
//---------------------------------------------------------

void LayoutPage::getNextPage(const LayoutOptions& options, LayoutContext& lc)
{
    if (!lc.page || lc.curPage >= lc.score->npages()) {
        lc.page = new Page(lc.score);
        lc.score->pages().push_back(lc.page);
        lc.prevSystem = nullptr;
        lc.pageOldMeasure = nullptr;
    } else {
        lc.page = lc.score->pages()[lc.curPage];
        QList<System*>& systems = lc.page->systems();
        lc.pageOldMeasure = systems.isEmpty() ? nullptr : systems.back()->measures().back();
        const int i = systems.indexOf(lc.curSystem);
        if (i > 0 && systems[i - 1]->page() == lc.page) {
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
    qreal x = 0.0;
    qreal y = 0.0;
    if (lc.curPage) {
        Page* prevPage = lc.score->pages()[lc.curPage - 1];
        if (MScore::verticalOrientation()) {
            y = prevPage->pos().y() + lc.page->height() + MScore::verticalPageGap;
        } else {
            qreal gap = (lc.curPage + lc.score->pageNumberOffset()) & 1 ? MScore::horizontalPageGapOdd : MScore::horizontalPageGapEven;
            x = prevPage->pos().x() + lc.page->width() + gap;
        }
    }
    ++lc.curPage;
    lc.page->setPos(x, y);
}

//---------------------------------------------------------
//   collectPage
//---------------------------------------------------------

void LayoutPage::collectPage(const LayoutOptions& options, LayoutContext& lc)
{
    const qreal slb = lc.score->styleP(Sid::staffLowerBorder);
    bool breakPages = lc.score->layoutMode() != LayoutMode::SYSTEM;
    qreal ey        = lc.page->height() - lc.page->bm();
    qreal y         = 0.0;

    System* nextSystem = 0;
    int systemIdx = -1;

    // re-calculate positions for systems before current
    // (they may have been filled on previous layout)
    int pSystems = lc.page->systems().size();
    if (pSystems > 0) {
        lc.page->system(0)->restoreLayout2();
        y = lc.page->system(0)->y() + lc.page->system(0)->height();
    } else {
        y = lc.page->tm();
    }
    for (int i = 1; i < pSystems; ++i) {
        System* cs = lc.page->system(i);
        System* ps = lc.page->system(i - 1);
        qreal distance = ps->minDistance(cs);
        y += distance;
        cs->setPos(lc.page->lm(), y);
        cs->restoreLayout2();
        y += cs->height();
    }

    for (int k = 0;; ++k) {
        //
        // calculate distance to previous system
        //
        qreal distance;
        if (lc.prevSystem) {
            distance = lc.prevSystem->minDistance(lc.curSystem);
        } else {
            // this is the first system on page
            if (lc.curSystem->vbox()) {
                distance = 0.0;
            } else {
                distance = lc.score->styleP(Sid::staffUpperBorder);
                bool fixedDistance = false;
                for (MeasureBase* mb : lc.curSystem->measures()) {
                    if (mb->isMeasure()) {
                        Measure* m = toMeasure(mb);
                        Spacer* sp = m->vspacerUp(0);
                        if (sp) {
                            if (sp->spacerType() == SpacerType::FIXED) {
                                distance = sp->gap();
                                fixedDistance = true;
                                break;
                            } else {
                                distance = qMax(distance, sp->gap());
                            }
                        }
                    }
                }
                if (!fixedDistance) {
                    distance = qMax(distance, lc.curSystem->minTop());
                }
            }
        }

        y += distance;
        lc.curSystem->setPos(lc.page->lm(), y);
        lc.curSystem->restoreLayout2();
        lc.page->appendSystem(lc.curSystem);
        y += lc.curSystem->height();

        //
        //  check for page break or if next system will fit on page
        //
        bool collected = false;
        if (lc.rangeDone) {
            // take next system unchanged
            if (systemIdx > 0) {
                nextSystem = lc.score->systems().value(systemIdx++);
                if (!nextSystem) {
                    // TODO: handle next movement
                }
            } else {
                nextSystem = lc.systemList.empty() ? 0 : lc.systemList.takeFirst();
                if (nextSystem) {
                    lc.score->systems().append(nextSystem);
                }
            }
        } else {
            nextSystem = LayoutSystem::collectSystem(options, lc, lc.score);
            if (nextSystem) {
                collected = true;
            }
        }
        lc.prevSystem = lc.curSystem;
        Q_ASSERT(lc.curSystem != nextSystem);
        lc.curSystem  = nextSystem;

        bool breakPage = !lc.curSystem || (breakPages && lc.prevSystem->pageBreak());

        if (!breakPage) {
            qreal dist = lc.prevSystem->minDistance(lc.curSystem) + lc.curSystem->height();
            Box* vbox = lc.curSystem->vbox();
            if (vbox) {
                dist += vbox->bottomGap();
            } else if (!lc.prevSystem->hasFixedDownDistance()) {
                qreal margin = qMax(lc.curSystem->minBottom(), lc.curSystem->spacerDistance(false));
                dist += qMax(margin, slb);
            }
            breakPage = (y + dist) >= ey && breakPages;
        }
        if (breakPage) {
            qreal dist = qMax(lc.prevSystem->minBottom(), lc.prevSystem->spacerDistance(false));
            dist = qMax(dist, slb);
            layoutPage(lc.page, ey - (y + dist));
            // if we collected a system we cannot fit onto this page,
            // we need to collect next page in order to correctly set system positions
            if (collected) {
                lc.pageOldMeasure = nullptr;
            }
            break;
        }
    }

    Fraction stick = Fraction(-1, 1);
    for (System* s : lc.page->systems()) {
        Score* currentScore = s->score();
        for (MeasureBase* mb : s->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            if (stick == Fraction(-1, 1)) {
                stick = m->tick();
            }

            for (int track = 0; track < currentScore->ntracks(); ++track) {
                for (Segment* segment = m->first(); segment; segment = segment->next()) {
                    Element* e = segment->element(track);
                    if (!e) {
                        continue;
                    }
                    if (e->isChordRest()) {
                        if (!currentScore->staff(track2staff(track))->show()) {
                            continue;
                        }
                        ChordRest* cr = toChordRest(e);
                        if (LayoutBeams::notTopBeam(cr)) {                           // layout cross staff beams
                            cr->beam()->layout();
                        }
                        if (LayoutTuplets::notTopTuplet(cr)) {
                            // fix layout of tuplets
                            DurationElement* de = cr;
                            while (de->tuplet() && de->tuplet()->elements().front() == de) {
                                Tuplet* t = de->tuplet();
                                t->layout();
                                de = t;
                            }
                        }

                        if (cr->isChord()) {
                            Chord* c = toChord(cr);
                            for (Chord* cc : c->graceNotes()) {
                                if (cc->beam() && cc->beam()->elements().front() == cc) {
                                    cc->beam()->layout();
                                }
                                cc->layoutSpanners();
                                for (Element* element : cc->el()) {
                                    if (element->isSlur()) {
                                        element->layout();
                                    }
                                }
                            }
                            c->layoutArpeggio2();
                            c->layoutSpanners();
                            if (c->tremolo()) {
                                Tremolo* t = c->tremolo();
                                Chord* c1 = t->chord1();
                                Chord* c2 = t->chord2();
                                if (t->twoNotes() && c1 && c2 && (c1->staffMove() || c2->staffMove())) {
                                    t->layout();
                                }
                            }
                        }
                    } else if (e->isBarLine()) {
                        toBarLine(e)->layout2();
                    }
                }
            }
            m->layout2();
        }
    }

    if (options.isMode(LayoutMode::SYSTEM)) {
        System* s = lc.page->systems().last();
        qreal height = s ? s->pos().y() + s->height() + s->minBottom() : lc.page->tm();
        lc.page->bbox().setRect(0.0, 0.0, options.loWidth, height + lc.page->bm());
    }

    lc.page->invalidateBspTree();
}

//---------------------------------------------------------
//   layoutPage
//    restHeight - vertical space which has to be distributed
//                 between systems
//    The algorithm tries to produce most equally spaced
//    systems.
//---------------------------------------------------------

void LayoutPage::layoutPage(Page* page, qreal restHeight)
{
    if (restHeight < 0.0) {
        qDebug("restHeight < 0.0: %f\n", restHeight);
        restHeight = 0;
    }

    Score* score = page->score();
    int gaps     = page->systems().size() - 1;

    QList<System*> sList;

    // build list of systems (excluding last)
    // set initial distance for each to the unstretched minimum distance to next
    for (int i = 0; i < gaps; ++i) {
        System* s1 = page->systems().at(i);
        System* s2 = page->systems().at(i + 1);
        s1->setDistance(s2->y() - s1->y());
        if (s1->vbox() || s2->vbox() || s1->hasFixedDownDistance()) {
            if (s2->vbox()) {
                checkDivider(true, s1, 0.0, true);              // remove
                checkDivider(false, s1, 0.0, true);             // remove
                checkDivider(true, s2, 0.0, true);              // remove
                checkDivider(false, s2, 0.0, true);             // remove
            }
            continue;
        }
        sList.push_back(s1);
    }

    // last system needs no divider
    System* lastSystem = page->systems().back();
    checkDivider(true, lastSystem, 0.0, true);        // remove
    checkDivider(false, lastSystem, 0.0, true);       // remove

    if (sList.empty() || MScore::noVerticalStretch || score->enableVerticalSpread() || score->layoutMode() == LayoutMode::SYSTEM) {
        if (score->layoutMode() == LayoutMode::FLOAT) {
            qreal y = restHeight * .5;
            for (System* system : page->systems()) {
                system->move(PointF(0.0, y));
            }
        } else if ((score->layoutMode() != LayoutMode::SYSTEM) && score->enableVerticalSpread()) {
            distributeStaves(page);
        }

        // system dividers
        for (int i = 0; i < gaps; ++i) {
            System* s1 = page->systems().at(i);
            System* s2 = page->systems().at(i + 1);
            if (!(s1->vbox() || s2->vbox())) {
                qreal yOffset = s1->height() + (s1->distance() - s1->height()) * .5;
                checkDivider(true,  s1, yOffset);
                checkDivider(false, s1, yOffset);
            }
        }
        return;
    }

    qreal maxDist = score->maxSystemDistance();

    // allocate space as needed to normalize system distance (bottom of one system to top of next)
    std::sort(sList.begin(), sList.end(), [](System* a, System* b) { return a->distance() - a->height() < b->distance() - b->height(); });
    System* s0 = sList[0];
    qreal dist = s0->distance() - s0->height();             // distance for shortest system
    for (int i = 1; i < sList.size(); ++i) {
        System* si = sList[i];
        qreal ndist = si->distance() - si->height();        // next taller system
        qreal fill  = ndist - dist;                         // amount by which this system distance exceeds next shorter
        if (fill > 0.0) {
            qreal totalFill = fill * i;                     // space required to add this amount to all shorter systems
            if (totalFill > restHeight) {
                totalFill = restHeight;                     // too much; adjust amount
                fill = restHeight / i;
            }
            for (int k = 0; k < i; ++k) {                   // add amount to all shorter systems
                System* s = sList[k];
                qreal d = s->distance() + fill;
                if ((d - s->height()) > maxDist) {          // but don't exceed max system distance
                    d = qMax(maxDist + s->height(), s->distance());
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
        qreal fill = restHeight / sList.size();
        for (System* s : qAsConst(sList)) {                           // allocate it to systems equally
            qreal d = s->distance() + fill;
            if ((d - s->height()) > maxDist) {              // but don't exceed max system distance
                d = qMax(maxDist + s->height(), s->distance());
            }
            s->setDistance(d);
        }
    }

    qreal y = page->systems().at(0)->y();
    for (int i = 0; i < gaps; ++i) {
        System* s1  = page->systems().at(i);
        System* s2  = page->systems().at(i + 1);
        s1->rypos() = y;
        y          += s1->distance();

        if (!(s1->vbox() || s2->vbox())) {
            qreal yOffset = s1->height() + (s1->distance() - s1->height()) * .5;
            checkDivider(true,  s1, yOffset);
            checkDivider(false, s1, yOffset);
        }
    }
    page->systems().back()->rypos() = y;
}

void LayoutPage::checkDivider(bool left, System* s, qreal yOffset, bool remove)
{
    SystemDivider* divider = left ? s->systemDividerLeft() : s->systemDividerRight();
    if ((s->score()->styleB(left ? Sid::dividerLeft : Sid::dividerRight)) && !remove) {
        if (!divider) {
            divider = new SystemDivider(s->score());
            divider->setDividerType(left ? SystemDivider::Type::LEFT : SystemDivider::Type::RIGHT);
            divider->setGenerated(true);
            s->add(divider);
        }
        divider->layout();
        divider->rypos() = divider->height() * .5 + yOffset;
        if (left) {
            divider->rypos() += s->score()->styleD(Sid::dividerLeftY) * SPATIUM20;
            divider->rxpos() =  s->score()->styleD(Sid::dividerLeftX) * SPATIUM20;
        } else {
            divider->rypos() += s->score()->styleD(Sid::dividerRightY) * SPATIUM20;
            divider->rxpos() =  s->score()->styleD(Sid::pagePrintableWidth) * DPI - divider->width();
            divider->rxpos() += s->score()->styleD(Sid::dividerRightX) * SPATIUM20;
        }
    } else if (divider) {
        if (divider->generated()) {
            s->remove(divider);
            delete divider;
        } else {
            s->score()->undoRemoveElement(divider);
        }
    }
}

void LayoutPage::distributeStaves(Page* page)
{
    Score* score { page->score() };
    VerticalGapDataList vgdl;

    // Find and classify all gaps between staves.
    int ngaps { 0 };
    qreal prevYBottom  { page->tm() };
    qreal yBottom      { 0.0 };
    qreal spacerOffset { 0.0 };
    bool vbox          { false };
    Spacer* nextSpacer { nullptr };
    bool transferNormalBracket { false };
    bool transferCurlyBracket  { false };
    for (System* system : page->systems()) {
        if (system->vbox()) {
            VerticalGapData* vgd = new VerticalGapData(!ngaps++, system, nullptr, nullptr, nullptr, prevYBottom);
            vgd->addSpaceAroundVBox(true);
            prevYBottom = system->y();
            yBottom     = system->y() + system->height();
            vbox        = true;
            vgdl.append(vgd);
            transferNormalBracket = false;
            transferCurlyBracket  = false;
        } else {
            bool newSystem       { true };
            bool addSpaceAroundNormalBracket { false };
            bool addSpaceAroundCurlyBracket  { false };
            int endNormalBracket { -1 };
            int endCurlyBracket  { -1 };
            int staffNr { -1 };
            for (SysStaff* sysStaff : *system->staves()) {
                Staff* staff { score->staff(++staffNr) };
                addSpaceAroundNormalBracket |= endNormalBracket == staffNr;
                addSpaceAroundCurlyBracket  |= endCurlyBracket == staffNr;
                for (const BracketItem* bi : staff->brackets()) {
                    if (bi->bracketType() == BracketType::NORMAL) {
                        addSpaceAroundNormalBracket |= staff->idx() > (endNormalBracket - 1);
                        endNormalBracket = qMax(endNormalBracket, staff->idx() + bi->bracketSpan());
                    } else if (bi->bracketType() == BracketType::BRACE) {
                        addSpaceAroundCurlyBracket |= staff->idx() > (endCurlyBracket - 1);
                        endCurlyBracket = qMax(endCurlyBracket, staff->idx() + bi->bracketSpan());
                    }
                }

                if (!sysStaff->show()) {
                    continue;
                }

                VerticalGapData* vgd = new VerticalGapData(!ngaps++, system, staff, sysStaff, nextSpacer, prevYBottom);
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
                vgdl.append(vgd);
            }
            transferNormalBracket = endNormalBracket >= 0;
            transferCurlyBracket  = endCurlyBracket >= 0;
        }
    }
    --ngaps;

    qreal spaceLeft { page->height() - page->bm() - score->styleP(Sid::staffLowerBorder) - yBottom };
    if (nextSpacer) {
        spaceLeft -= qMax(0.0, nextSpacer->gap() - spacerOffset - score->styleP(Sid::staffLowerBorder));
    }
    if (spaceLeft <= 0.0) {
        return;
    }

    // Try to make the gaps equal, taking the spread factors and maximum spacing into account.
    static const int maxPasses { 20 };     // Saveguard to prevent endless loops.
    int pass { 0 };
    while (!RealIsNull(spaceLeft) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        qreal smallest     { vgdl.smallest() };
        qreal nextSmallest { vgdl.smallest(smallest) };
        if (RealIsNull(smallest) || RealIsNull(nextSmallest)) {
            break;
        }

        if ((nextSmallest - smallest) * vgdl.sumStretchFactor() > spaceLeft) {
            nextSmallest = smallest + spaceLeft / vgdl.sumStretchFactor();
        }

        qreal addedSpace { 0.0 };
        VerticalGapDataList modified;
        for (VerticalGapData* vgd : vgdl) {
            if (!RealIsNull(vgd->spacing() - smallest)) {
                continue;
            }
            qreal step { nextSmallest - vgd->spacing() };
            if (step < 0.0) {
                continue;
            }
            step = vgd->addSpacing(step);
            if (!RealIsNull(step)) {
                addedSpace += step * vgd->factor();
                modified.append(vgd);
                ++ngaps;
            }
            if ((spaceLeft - addedSpace) <= 0.0) {
                break;
            }
        }
        if ((spaceLeft - addedSpace) <= 0.0) {
            for (VerticalGapData* vgd : modified) {
                vgd->undoLastAddSpacing();
            }
            ngaps = 0;
        } else {
            spaceLeft -= addedSpace;
        }
    }

    // If there is still space left, distribute the space of the staves.
    // However, there is a limit on how much space is added per gap.
    const qreal maxPageFill { score->styleP(Sid::maxPageFillSpread) };
    spaceLeft = qMin(maxPageFill * vgdl.length(), spaceLeft);
    pass = 0;
    ngaps = 1;
    while (!RealIsNull(spaceLeft) && !RealIsNull(maxPageFill) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        qreal addedSpace { 0.0 };
        qreal step { spaceLeft / vgdl.sumStretchFactor() };
        for (VerticalGapData* vgd : vgdl) {
            qreal res { vgd->addFillSpacing(step, maxPageFill) };
            if (!RealIsNull(res)) {
                addedSpace += res * vgd->factor();
                ++ngaps;
            }
        }
        spaceLeft -= addedSpace;
    }

    QSet<System*> systems;
    qreal systemShift { 0.0 };
    qreal staffShift  { 0.0 };
    System* prvSystem { nullptr };
    for (VerticalGapData* vgd : vgdl) {
        if (vgd->sysStaff) {
            systems.insert(vgd->system);
        }
        systemShift += vgd->actualAddedSpace();
        if (prvSystem == vgd->system) {
            staffShift += vgd->actualAddedSpace();
        } else {
            vgd->system->rypos() += systemShift;
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
