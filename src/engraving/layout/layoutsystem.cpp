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
#include "layoutsystem.h"

#include "libmscore/factory.h"
#include "libmscore/barline.h"
#include "libmscore/box.h"
#include "libmscore/chord.h"
#include "libmscore/dynamic.h"
#include "libmscore/fingering.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/mmrestrange.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/stafflines.h"
#include "libmscore/tie.h"
#include "libmscore/tremolo.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/system.h"

#include "layoutbeams.h"
#include "layoutharmonies.h"
#include "layoutlyrics.h"
#include "layoutmeasure.h"
#include "layouttuplets.h"

using namespace mu::engraving;
using namespace Ms;

//---------------------------------------------------------
//   collectSystem
//---------------------------------------------------------

System* LayoutSystem::collectSystem(const LayoutOptions& options, LayoutContext& ctx, Ms::Score* score)
{
    if (!ctx.curMeasure) {
        return nullptr;
    }

    const MeasureBase* measure  = score->systems().empty() ? 0 : score->systems().back()->measures().back();
    if (measure) {
        measure = measure->findPotentialSectionBreak();
    }

    if (measure) {
        const LayoutBreak* layoutBreak = measure->sectionBreakElement();
        ctx.firstSystem        = measure->sectionBreak() && !options.isMode(LayoutMode::FLOAT);
        ctx.firstSystemIndent  = ctx.firstSystem && options.firstSystemIndent && layoutBreak->firstSystemIdentation();
        ctx.startWithLongNames = ctx.firstSystem && layoutBreak->startWithLongNames();
    }

    System* system = getNextSystem(ctx);
    Fraction lcmTick = ctx.curMeasure->tick();
    system->setInstrumentNames(ctx, ctx.startWithLongNames, lcmTick);

    qreal curSysWidth    = 0;
    qreal layoutSystemMinWidth = 0;
    bool firstMeasure = true;
    bool createHeader = false;
    qreal systemWidth = score->styleD(Sid::pagePrintableWidth) * DPI;
    system->setWidth(systemWidth);

    // save state of measure
    bool curHeader = ctx.curMeasure->header();
    bool curTrailer = ctx.curMeasure->trailer();
    MeasureBase* breakMeasure = nullptr;

    Fraction minTicks = Fraction::max(); // Initializing this variable at an arbitrary high value.
    // In principle, it just needs to be longer than any possible note.
    Fraction prevMinTicks = Fraction(1, 1);
    bool changeMinSysTicks = false;
    qreal oldStretch = 1;

    while (ctx.curMeasure) {      // collect measure for system
        System* oldSystem = ctx.curMeasure->system();
        system->appendMeasure(ctx.curMeasure);

        qreal ww  = 0;          // width of current measure

        // After appending a new measure, the shortest note in the system may change, in which case
        // we need to recompute the layout of the previous measures. When updating the width of these
        // measures, curSysWidth must be updated accordingly.
        if (ctx.curMeasure->isMeasure()) {
            if (toMeasure(ctx.curMeasure)->computeTicks() < minTicks) {
                prevMinTicks = minTicks; // We save the previous value in case we need to restore it (see later)
                minTicks = toMeasure(ctx.curMeasure)->computeTicks();
                changeMinSysTicks = true;
                for (MeasureBase* mb : system->measures()) {
                    if (mb == ctx.curMeasure) {
                        break; // Cause I want to change only previous measures, not current one
                    }
                    if (mb->isMeasure()) {
                        qreal prevWidth = toMeasure(mb)->width();
                        toMeasure(mb)->computeWidth(minTicks, 1);
                        qreal newWidth = toMeasure(mb)->width();
                        curSysWidth += newWidth - prevWidth;
                    }
                }
            } else {
                changeMinSysTicks = false;
            }
        }

        if (ctx.curMeasure->isMeasure()) {
            Measure* m = toMeasure(ctx.curMeasure);
            if (firstMeasure) {
                layoutSystemMinWidth = curSysWidth;
                system->layoutSystem(ctx, curSysWidth, ctx.firstSystem, ctx.firstSystemIndent);
                curSysWidth += system->leftMargin();
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                m->addSystemHeader(ctx.firstSystem);
                firstMeasure = false;
                createHeader = false;
            } else {
                if (createHeader) {
                    m->addSystemHeader(false);
                    createHeader = false;
                } else if (m->header()) {
                    m->removeSystemHeader();
                }
            }

            m->createEndBarLines(true);
            // measures with nobreak cannot end a system
            // thus they will not contain a trailer
            if (m->noBreak()) {
                m->removeSystemTrailer();
            } else {
                m->addSystemTrailer(m->nextMeasure());
            }
            m->computeWidth(minTicks, 1);
            ww = m->width();
        } else if (ctx.curMeasure->isHBox()) {
            ctx.curMeasure->computeMinWidth();
            ww = ctx.curMeasure->width();
            createHeader = toHBox(ctx.curMeasure)->createSystemHeader();
        } else {
            // vbox:
            LayoutMeasure::getNextMeasure(options, ctx);
            system->layout2(ctx);         // compute staff distances
            return system;
        }
        // check if lc.curMeasure fits, remove if not
        // collect at least one measure and the break

        double acceptanceRange = system->isSqueezable() ? 1.025 : 1; // A value slightly larger than 1 allows systems
        // to be initially slightly larger than the target width and be justified by tightening rather than stretching.
        // However, we must first make sure that the system *can* be tightened. isSqueezable() checks for that.
        bool doBreak = (system->measures().size() > 1) && ((curSysWidth + ww) > systemWidth * acceptanceRange);
        if (doBreak) {
            breakMeasure = ctx.curMeasure;
            system->removeLastMeasure();
            ctx.curMeasure->setParent(oldSystem);
            while (ctx.prevMeasure && ctx.prevMeasure->noBreak() && system->measures().size() > 1) {
                // remove however many measures are grouped with nobreak, working backwards
                // but if too many are grouped, stop before we get 0 measures left on system
                // TODO: intelligently break group into smaller groups instead
                ctx.tick -= ctx.curMeasure->ticks();
                ctx.measureNo = ctx.prevMeasure->no();

                ctx.nextMeasure = ctx.curMeasure;
                ctx.curMeasure  = ctx.prevMeasure;
                ctx.prevMeasure = ctx.curMeasure->prevMeasure();

                curSysWidth -= system->lastMeasure()->width();
                system->removeLastMeasure();
                ctx.curMeasure->setParent(oldSystem);
            }
            // If the last appended measure caused a re-layout of the previous measures, now that we are
            // removing it we need to re-layout the previous measures again.
            if (changeMinSysTicks) {
                minTicks = prevMinTicks; // If the last measure caused it to change, now we need to restore it!
                for (MeasureBase* mb : system->measures()) {
                    if (mb->isMeasure()) {
                        qreal prevWidth = toMeasure(mb)->width();
                        toMeasure(mb)->computeWidth(minTicks, 1);
                        qreal newWidth = toMeasure(mb)->width();
                        curSysWidth += newWidth - prevWidth;
                    }
                }
            }
            break;
        }

        if (ctx.prevMeasure && ctx.prevMeasure->isMeasure() && ctx.prevMeasure->system() == system) {
            //
            // now we know that the previous measure is not the last
            // measure in the system and we finally can create the end barline for it

            Measure* m = toMeasure(ctx.prevMeasure);
            // TODO: if lc.curMeasure is a frame, removing the trailer may be premature
            // but merely skipping this code isn't good enough,
            // we need to find the right time to re-enable the trailer,
            // since it seems to be disabled somewhere else
            if (m->trailer()) {
                qreal ow = m->width();
                m->removeSystemTrailer();
                curSysWidth += m->width() - ow;
            }
            // if the prev measure is an end repeat and the cur measure
            // is an repeat, the createEndBarLines() created an start-end repeat barline
            // and we can remove the start repeat barline of the current barline

            if (ctx.curMeasure->isMeasure()) {
                Measure* m1 = toMeasure(ctx.curMeasure);
                if (m1->repeatStart()) {
                    Segment* s = m1->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                        m1->computeWidth(minTicks, 1);
                        ww = m1->width();
                    }
                }
            }
            // TODO: we actually still don't know for sure
            // if this will be the last true measure of the system or not
            // since the lc.curMeasure may be a frame
            // but at this point we have no choice but to assume it isn't
            // since we don't know yet if another true measure will fit
            // worst that happens is we don't get the automatic double bar before a courtesy key signature
            curSysWidth += m->createEndBarLines(false);          // create final barLine
        }

        MeasureBase* mb = ctx.curMeasure;
        bool lineBreak  = false;
        switch (options.mode) {
        case LayoutMode::PAGE:
        case LayoutMode::SYSTEM:
            lineBreak = mb->pageBreak() || mb->lineBreak() || mb->sectionBreak();
            break;
        case LayoutMode::FLOAT:
        case LayoutMode::LINE:
        case LayoutMode::HORIZONTAL_FIXED:
            lineBreak = false;
            break;
        }

        // preserve state of next measure (which is about to become current measure)
        if (ctx.nextMeasure) {
            MeasureBase* nmb = ctx.nextMeasure;
            if (nmb->isMeasure() && score->styleB(Sid::createMultiMeasureRests)) {
                Measure* nm = toMeasure(nmb);
                if (nm->hasMMRest()) {
                    nmb = nm->mmRest();
                }
            }
            if (nmb->isMeasure()) {
                oldStretch = toMeasure(nmb)->layoutStretch();
            }
            if (!ctx.curMeasure->noBreak()) {
                // current measure is not a nobreak,
                // so next measure could possibly start a system
                curHeader = nmb->header();
            }
            if (!nmb->noBreak()) {
                // next measure is not a nobreak
                // so it could possibly end a system
                curTrailer = nmb->trailer();
            }
        }

        LayoutMeasure::getNextMeasure(options, ctx);

        curSysWidth += ww;

        // ElementType nt = lc.curMeasure ? lc.curMeasure->type() : ElementType::INVALID;
        mb = ctx.curMeasure;
        bool tooWide = false;     // curSysWidth + minMeasureWidth > systemWidth;  // TODO: noBreak
        if (lineBreak || !mb || mb->isVBox() || mb->isTBox() || mb->isFBox() || tooWide) {
            break;
        }
    }

    if (ctx.endTick < ctx.prevMeasure->tick()) {
        // we've processed the entire range
        // but we need to continue layout until we reach a system whose last measure is the same as previous layout
        if (ctx.prevMeasure == ctx.systemOldMeasure) {
            // this system ends in the same place as the previous layout
            // ok to stop
            if (ctx.curMeasure && ctx.curMeasure->isMeasure()) {
                // we may have previously processed first measure(s) of next system
                // so now we must restore to original state
                Measure* m = toMeasure(ctx.curMeasure);
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                const MeasureBase* pbmb = ctx.prevMeasure->findPotentialSectionBreak();
                bool localFirstSystem = pbmb->sectionBreak() && !options.isMode(LayoutMode::FLOAT);
                MeasureBase* nm = breakMeasure ? breakMeasure : m;
                if (curHeader) {
                    m->addSystemHeader(localFirstSystem);
                } else {
                    m->removeSystemHeader();
                }
                for (;;) {
                    // TODO: what if the nobreak group takes the entire system - is this correct?
                    if (curTrailer && !m->noBreak()) {
                        m->addSystemTrailer(m->nextMeasure());
                    } else {
                        m->removeSystemTrailer();
                    }
                    m->computeWidth(m->system()->minSysTicks(), oldStretch);
                    m->layoutMeasureElements();
                    LayoutBeams::restoreBeams(m);
                    if (m == nm || !m->noBreak()) {
                        break;
                    }
                    m = m->nextMeasure();
                }
            }
            ctx.rangeDone = true;
        }
    }

    //
    // now we have a complete set of measures for this system
    //
    // prevMeasure is the last measure in the system
    if (ctx.prevMeasure && ctx.prevMeasure->isMeasure()) {
        LayoutBeams::breakCrossMeasureBeams(ctx, toMeasure(ctx.prevMeasure));
        qreal w = toMeasure(ctx.prevMeasure)->createEndBarLines(true);
        curSysWidth += w;
    }

    hideEmptyStaves(score, system, ctx.firstSystem);
    // Relayout system decorations to reuse space properly for
    // hidden staves' instrument names or other hidden elements.
    curSysWidth -= system->leftMargin();
    system->layoutSystem(ctx, layoutSystemMinWidth, ctx.firstSystem, ctx.firstSystemIndent);
    curSysWidth += system->leftMargin();

    //-------------------------------------------------------
    //    add system trailer if needed
    //    (cautionary time/key signatures etc)
    //-------------------------------------------------------

    Measure* lm  = system->lastMeasure();
    if (lm) {
        Measure* nm = lm->nextMeasure();
        if (nm) {
            qreal w = lm->width();
            lm->addSystemTrailer(nm);
            if (lm->trailer()) {
                lm->computeWidth(minTicks, 1);
            }
            curSysWidth += lm->width() - w;
        }
    }

    // BRING THE WIDTH OF THE SYSTEM TO THE DESIRED VALUE
    // Gradient descent method: calls the computeWidth() function with a stretch parameter
    // proportional to the difference between the current length and the target length.
    // After few iterations, the length will converge to the target length.
    qreal newRest = systemWidth - curSysWidth;
    if ((ctx.curMeasure == 0 || (lm && lm->sectionBreak()))
        && ((curSysWidth / systemWidth) <= score->styleD(Sid::lastSystemFillLimit))) {
        // We do not stretch last system of a section (or the last of the piece) if curSysWidth is <= lastSystemFillLimit
        newRest = 0;
    }
    if (MScore::noHorizontalStretch) { // Debug feature
        newRest = 0;
    }
    qreal stretchCoeff = 1;
    qreal prevWidth = 0;
    int iter = 0;
    double epsilon = score->spatium() * 0.05; // For reference: this is smaller than the width of a note stem
    static constexpr float multiplier = 1.4f; // Empirically optimized value which allows the fastest convergence of the following algorithm.
    static constexpr int maxIter = 100;
    // Different systems need different numbers of iterations of the following loop to reach the target width.
    // The average is less than 3 iterations, and the maximum I've ever seen (very rare) is 30-40 iterations.
    // maxIter just serves as a safety exit to not get stuck in the loop in case a system can't be justified
    // (which can only happen if errors are made before getting here). It's set to a very high value to make
    // sure that the system really can't be justified, and it isn't just a "tricky" one needing more iterations.
    while (abs(newRest) > epsilon && iter < maxIter) {
        stretchCoeff *= (1 + multiplier * newRest / curSysWidth);
        for (MeasureBase* mb : system->measures()) {
            if (mb->isMeasure()) {
                Measure* m = toMeasure(mb);
                if (!(m->isWidthLocked() && stretchCoeff < m->layoutStretch())) { // It would be pointless to re-compute the layout of a measure
                    prevWidth = m->width();                                       // that is already widthLocked to a larger value.
                    m->computeWidth(minTicks, stretchCoeff);
                    curSysWidth += m->width() - prevWidth;
                }
            }
        }
        newRest = systemWidth - curSysWidth;
        iter++;
    }

    // LAYOUT MEASURES
    PointF pos;
    firstMeasure = true;
    bool createBrackets = false;
    for (MeasureBase* mb : system->measures()) {
        qreal ww = mb->width();
        if (mb->isMeasure()) {
            if (firstMeasure) {
                pos.rx() += system->leftMargin();
                firstMeasure = false;
            }
            mb->setPos(pos);
            Measure* m = toMeasure(mb);
            m->layoutMeasureElements();
            m->layoutStaffLines();
            if (createBrackets) {
                system->addBrackets(ctx, toMeasure(mb));
                createBrackets = false;
            }
        } else if (mb->isHBox()) {
            mb->setPos(pos + PointF(toHBox(mb)->topGap(), 0.0));
            mb->layout();
            createBrackets = toHBox(mb)->createSystemHeader();
        } else if (mb->isVBox()) {
            mb->setPos(pos);
        }
        pos.rx() += ww;
    }
    system->setWidth(pos.x());

    layoutSystemElements(options, ctx, score, system);
    system->layout2(ctx);     // compute staff distances
    // TODO: now that the code at the top of this function does this same backwards search,
    // we might be able to eliminate this block
    // but, lc might be used elsewhere so we need to be careful
    measure = system->measures().back();

    if (measure) {
        measure = measure->findPotentialSectionBreak();
    }

    if (measure) {
        const LayoutBreak* layoutBreak = measure->sectionBreakElement();
        ctx.firstSystem        = measure->sectionBreak() && !options.isMode(LayoutMode::FLOAT);
        ctx.firstSystemIndent  = ctx.firstSystem && options.firstSystemIndent && layoutBreak->firstSystemIdentation();
        ctx.startWithLongNames = ctx.firstSystem && layoutBreak->startWithLongNames();
    }

    return system;
}

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* LayoutSystem::getNextSystem(LayoutContext& ctx)
{
    Ms::Score* score = ctx.score();
    bool isVBox = ctx.curMeasure->isVBox();
    System* system;
    if (ctx.systemList.empty()) {
        system = Factory::createSystem(score->dummy()->page());
        ctx.systemOldMeasure = 0;
    } else {
        system = ctx.systemList.takeFirst();
        ctx.systemOldMeasure = system->measures().empty() ? 0 : system->measures().back();
        system->clear();       // remove measures from system
    }
    score->systems().append(system);
    if (!isVBox) {
        int nstaves = score->Score::nstaves();
        system->adjustStavesNumber(nstaves);
        for (int i = 0; i < nstaves; ++i) {
            system->staff(i)->setShow(score->staff(i)->show());
        }
    }
    return system;
}

void LayoutSystem::hideEmptyStaves(Score* score, System* system, bool isFirstSystem)
{
    int staves   = score->nstaves();
    int staffIdx = 0;
    bool systemIsEmpty = true;

    for (Staff* staff : qAsConst(score->staves())) {
        SysStaff* ss  = system->staff(staffIdx);

        Staff::HideMode hideMode = staff->hideWhenEmpty();

        if (hideMode == Staff::HideMode::ALWAYS
            || (score->styleB(Sid::hideEmptyStaves)
                && (staves > 1)
                && !(isFirstSystem && score->styleB(Sid::dontHideStavesInFirstSystem))
                && hideMode != Staff::HideMode::NEVER)) {
            bool hideStaff = true;
            for (MeasureBase* m : system->measures()) {
                if (!m->isMeasure()) {
                    continue;
                }
                Measure* measure = toMeasure(m);
                if (!measure->isEmpty(staffIdx)) {
                    hideStaff = false;
                    break;
                }
            }
            // check if notes moved into this staff
            Part* part = staff->part();
            int n = part->nstaves();
            if (hideStaff && (n > 1)) {
                int idx = part->staves()->front()->idx();
                for (int i = 0; i < part->nstaves(); ++i) {
                    int st = idx + i;

                    for (MeasureBase* mb : system->measures()) {
                        if (!mb->isMeasure()) {
                            continue;
                        }
                        Measure* m = toMeasure(mb);
                        if (staff->hideWhenEmpty() == Staff::HideMode::INSTRUMENT && !m->isEmpty(st)) {
                            hideStaff = false;
                            break;
                        }
                        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                            for (int voice = 0; voice < VOICES; ++voice) {
                                ChordRest* cr = s->cr(st * VOICES + voice);
                                if (cr == 0 || cr->isRest()) {
                                    continue;
                                }
                                int staffMove = cr->staffMove();
                                if (staffIdx == st + staffMove) {
                                    hideStaff = false;
                                    break;
                                }
                            }
                        }
                        if (!hideStaff) {
                            break;
                        }
                    }
                    if (!hideStaff) {
                        break;
                    }
                }
            }
            ss->setShow(hideStaff ? false : staff->show());
            if (ss->show()) {
                systemIsEmpty = false;
            }
        } else if (!staff->show()) {
            // TODO: OK to check this first and not bother with checking if empty?
            ss->setShow(false);
        } else {
            systemIsEmpty = false;
            ss->setShow(true);
        }

        ++staffIdx;
    }
    Staff* firstVisible = nullptr;
    if (systemIsEmpty) {
        for (Staff* staff : qAsConst(score->staves())) {
            SysStaff* ss  = system->staff(staff->idx());
            if (staff->showIfEmpty() && !ss->show()) {
                ss->setShow(true);
                systemIsEmpty = false;
            } else if (!firstVisible && staff->show()) {
                firstVisible = staff;
            }
        }
    }
    // don’t allow a complete empty system
    if (systemIsEmpty && !score->staves().isEmpty()) {
        Staff* staff = firstVisible ? firstVisible : score->staves().front();
        SysStaff* ss = system->staff(staff->idx());
        ss->setShow(true);
    }
}

void LayoutSystem::layoutSystemElements(const LayoutOptions& options, LayoutContext& lc, Score* score, System* system)
{
    if (score->noStaves()) {
        return;
    }

    //-------------------------------------------------------------
    //    create cr segment list to speed up computations
    //-------------------------------------------------------------

    std::vector<Segment*> sl;
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        m->layoutMeasureNumber();
        m->layoutMMRestRange();

        // in continuous view, entire score is one system
        // but we only need to process the range
        if (options.isLinearMode() && (m->tick() < lc.startTick || m->tick() > lc.endTick)) {
            continue;
        }
        for (Segment* s = m->first(); s; s = s->next()) {
            if (s->isChordRestType() || !s->annotations().empty()) {
                sl.push_back(s);
            }
        }
    }

    //-------------------------------------------------------------
    // layout beams
    //  Needs to be done before creating skylines as stem lengths
    //  may change.
    //-------------------------------------------------------------

    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->staff(e->staffIdx())->show()) {
                // the beam and its system may still be referenced when selecting all,
                // even if the staff is invisible. The old system is invalid and does cause problems in #284012
                if (e && e->isChordRest() && !score->staff(e->staffIdx())->show() && toChordRest(e)->beam()) {
                    toChordRest(e)->beam()->resetExplicitParent();
                }
                continue;
            }
            ChordRest* cr = toChordRest(e);

            // layout beam
            if (LayoutBeams::isTopBeam(cr)) {
                Beam* b = cr->beam();
                b->layout();
            }
        }
    }

    //-------------------------------------------------------------
    //    create skylines
    //-------------------------------------------------------------

    for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        SysStaff* ss = system->staff(staffIdx);
        Skyline& skyline = ss->skyline();
        skyline.clear();
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            MeasureNumber* mno = m->noText(staffIdx);
            MMRestRange* mmrr  = m->mmRangeText(staffIdx);
            // no need to build skyline outside of range in continuous view
            if (options.isLinearMode() && (m->tick() < lc.startTick || m->tick() > lc.endTick)) {
                continue;
            }
            if (mno && mno->addToSkyline()) {
                ss->skyline().add(mno->bbox().translated(m->pos() + mno->pos()));
            }
            if (mmrr && mmrr->addToSkyline()) {
                ss->skyline().add(mmrr->bbox().translated(m->pos() + mmrr->pos()));
            }
            if (m->staffLines(staffIdx)->addToSkyline()) {
                ss->skyline().add(m->staffLines(staffIdx)->bbox().translated(m->pos()));
            }
            for (Segment& s : m->segments()) {
                if (!s.enabled() || s.isTimeSigType()) {             // hack: ignore time signatures
                    continue;
                }
                PointF p(s.pos() + m->pos());
                if (s.segmentType()
                    & (SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine)) {
                    BarLine* bl = toBarLine(s.element(staffIdx * VOICES));
                    if (bl && bl->addToSkyline()) {
                        RectF r = bl->layoutRect();
                        skyline.add(r.translated(bl->pos() + p));
                    }
                } else {
                    int strack = staffIdx * VOICES;
                    int etrack = strack + VOICES;
                    for (EngravingItem* e : s.elist()) {
                        if (!e) {
                            continue;
                        }
                        int effectiveTrack = e->vStaffIdx() * VOICES + e->voice();
                        if (effectiveTrack < strack || effectiveTrack >= etrack) {
                            continue;
                        }

                        // clear layout for chord-based fingerings
                        // do this before adding chord to skyline
                        if (e->isChord()) {
                            Chord* c = toChord(e);
                            std::list<Note*> notes;
                            for (auto gc : c->graceNotes()) {
                                for (auto n : gc->notes()) {
                                    notes.push_back(n);
                                }
                            }
                            for (auto n : c->notes()) {
                                notes.push_back(n);
                            }
                            for (Note* note : notes) {
                                for (EngravingItem* en : note->el()) {
                                    if (en->isFingering()) {
                                        Fingering* f = toFingering(en);
                                        if (f->layoutType() == ElementType::CHORD) {
                                            f->setPos(PointF());
                                            f->setbbox(RectF());
                                        }
                                    }
                                }
                            }
                        }

                        // add element to skyline
                        if (e->addToSkyline()) {
                            skyline.add(e->shape().translated(e->pos() + p));
                        }

                        // add tremolo to skyline
                        if (e->isChord() && toChord(e)->tremolo()) {
                            Tremolo* t = toChord(e)->tremolo();
                            Chord* c1 = t->chord1();
                            Chord* c2 = t->chord2();
                            if (!t->twoNotes() || (c1 && !c1->staffMove() && c2 && !c2->staffMove())) {
                                if (t->chord() == e && t->addToSkyline()) {
                                    skyline.add(t->shape().translated(t->pos() + e->pos() + p));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------
    // layout fingerings, add beams to skylines
    //-------------------------------------------------------------

    for (Segment* s : sl) {
        std::set<int> recreateShapes;
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->staff(e->staffIdx())->show()) {
                continue;
            }
            ChordRest* cr = toChordRest(e);

            // add beam to skyline
            if (LayoutBeams::isTopBeam(cr)) {
                Beam* b = cr->beam();
                b->addSkyline(system->staff(b->staffIdx())->skyline());
            }

            // layout chord-based fingerings
            if (e->isChord()) {
                Chord* c = toChord(e);
                std::list<Note*> notes;
                for (auto gc : c->graceNotes()) {
                    for (auto n : gc->notes()) {
                        notes.push_back(n);
                    }
                }
                for (auto n : c->notes()) {
                    notes.push_back(n);
                }
                std::list<Fingering*> fingerings;
                for (Note* note : notes) {
                    for (EngravingItem* el : note->el()) {
                        if (el->isFingering()) {
                            Fingering* f = toFingering(el);
                            if (f->layoutType() == ElementType::CHORD) {
                                if (f->placeAbove()) {
                                    fingerings.push_back(f);
                                } else {
                                    fingerings.push_front(f);
                                }
                            }
                        }
                    }
                }
                for (Fingering* f : fingerings) {
                    f->layout();
                    if (f->addToSkyline()) {
                        Note* n = f->note();
                        RectF r = f->bbox().translated(f->pos() + n->pos() + n->chord()->pos() + s->pos() + s->measure()->pos());
                        system->staff(f->note()->chord()->vStaffIdx())->skyline().add(r);
                    }
                    recreateShapes.insert(f->staffIdx());
                }
            }
        }
        for (auto staffIdx : recreateShapes) {
            s->createShape(staffIdx);
        }
    }

    //-------------------------------------------------------------
    // layout articulations
    //-------------------------------------------------------------

    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->staff(e->staffIdx())->show()) {
                continue;
            }
            ChordRest* cr = toChordRest(e);
            // articulations
            if (cr->isChord()) {
                Chord* c = toChord(cr);
                c->layoutArticulations();
                c->layoutArticulations2();
            }
        }
    }

    //-------------------------------------------------------------
    // layout tuplets
    //-------------------------------------------------------------

    std::map<int, Fraction> skipTo;
    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->staff(e->staffIdx())->show()) {
                continue;
            }
            int track = e->track();
            if (skipTo.count(track) && e->tick() < skipTo[track]) {
                continue; // don't lay out tuplets for this voice that have already been done
            }
            // find the top tuplet for this segment
            DurationElement* de = toChordRest(e);
            if (!de->tuplet()) {
                continue;
            }
            while (de->tuplet()) {
                de = de->tuplet();
            }
            LayoutTuplets::layout(de); // recursively lay out all tuplets covered by this tuplet

            // don't layout any tuplets covered by this top level tuplet for this voice--
            // they've already been laid out by layoutTuplet().
            skipTo[track] = de->tick() + de->ticks();
            break;
        }
    }

    //-------------------------------------------------------------
    // Drumline sticking
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isSticking()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // layout slurs
    //-------------------------------------------------------------

    bool useRange = false;    // TODO: lineMode();
    Fraction stick = useRange ? lc.startTick : system->measures().front()->tick();
    Fraction etick = useRange ? lc.endTick : system->measures().back()->endTick();
    auto spanners = score->spannerMap().findOverlapping(stick.ticks(), etick.ticks());

    std::vector<Spanner*> spanner;
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        sp->computeStartElement();
        sp->computeEndElement();
        lc.processedSpanners.insert(sp);
        if (sp->tick() < etick && sp->tick2() >= stick) {
            if (sp->isSlur()) {
                // skip cross-staff slurs
                ChordRest* scr = sp->startCR();
                ChordRest* ecr = sp->endCR();
                int idx = sp->vStaffIdx();
                if (scr && ecr && (scr->vStaffIdx() != idx || ecr->vStaffIdx() != idx)) {
                    continue;
                }
                spanner.push_back(sp);
            }
        }
    }
    processLines(system, spanner, false);
    for (auto s : spanner) {
        Slur* slur = toSlur(s);
        ChordRest* scr = s->startCR();
        ChordRest* ecr = s->endCR();
        if (scr && scr->isChord()) {
            toChord(scr)->layoutArticulations3(slur);
        }
        if (ecr && ecr->isChord()) {
            toChord(ecr)->layoutArticulations3(slur);
        }
    }

    std::vector<Dynamic*> dynamics;
    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e) {
                continue;
            }
            if (e->isChord()) {
                Chord* c = toChord(e);
                for (Chord* ch : c->graceNotes()) {
                    layoutTies(ch, system, stick);
                }
                layoutTies(c, system, stick);
            }
        }
        for (EngravingItem* e : s->annotations()) {
            if (e->isDynamic()) {
                Dynamic* d = toDynamic(e);
                d->layout();

                if (d->autoplace()) {
                    d->autoplaceSegmentElement(false);
                    dynamics.push_back(d);
                }
            } else if (e->isFiguredBass()) {
                e->layout();
                e->autoplaceSegmentElement();
            }
        }
    }

    // add dynamics shape to skyline

    for (Dynamic* d : dynamics) {
        if (!d->addToSkyline()) {
            continue;
        }
        int si = d->staffIdx();
        Segment* s = d->segment();
        Measure* m = s->measure();
        system->staff(si)->skyline().add(d->shape().translated(d->pos() + s->pos() + m->pos()));
    }

    //-------------------------------------------------------------
    // layout SpannerSegments for current system
    // ottavas, pedals, voltas are collected here, but layouted later
    //-------------------------------------------------------------

    spanner.clear();
    std::vector<Spanner*> hairpins;
    std::vector<Spanner*> ottavas;
    std::vector<Spanner*> pedal;
    std::vector<Spanner*> voltas;

    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (sp->tick() < etick && sp->tick2() > stick) {
            if (sp->isOttava()) {
                ottavas.push_back(sp);
            } else if (sp->isPedal()) {
                pedal.push_back(sp);
            } else if (sp->isVolta()) {
                voltas.push_back(sp);
            } else if (sp->isHairpin()) {
                hairpins.push_back(sp);
            } else if (!sp->isSlur() && !sp->isVolta()) {      // slurs are already
                spanner.push_back(sp);
            }
        }
    }
    processLines(system, hairpins, false);
    processLines(system, spanner, false);

    //-------------------------------------------------------------
    // Fermata, TremoloBar
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isFermata() || e->isTremoloBar()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // Ottava, Pedal
    //-------------------------------------------------------------

    processLines(system, ottavas, false);
    processLines(system, pedal,   true);

    //-------------------------------------------------------------
    // Lyric
    //-------------------------------------------------------------

    LayoutLyrics::layoutLyrics(options, score, system);

    // here are lyrics dashes and melisma
    for (Spanner* sp : score->unmanagedSpanners()) {
        if (sp->tick() >= etick || sp->tick2() <= stick) {
            continue;
        }
        sp->layoutSystem(system);
    }

    //
    // We need to known if we have FretDiagrams in the system to decide when to layout the Harmonies
    //

    bool hasFretDiagram = false;
    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isFretDiagram()) {
                hasFretDiagram = true;
                break;
            }
        }

        if (hasFretDiagram) {
            break;
        }
    }

    //-------------------------------------------------------------
    // Harmony, 1st place
    // If we have FretDiagrams, we want the Harmony above this and
    // above the volta, therefore we delay the layout.
    //-------------------------------------------------------------

    if (!hasFretDiagram) {
        LayoutHarmonies::layoutHarmonies(sl);
        LayoutHarmonies::alignHarmonies(system, sl, true, options.maxFretShiftAbove, options.maxFretShiftBelow);
    }

    //-------------------------------------------------------------
    // StaffText, InstrumentChange
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isPlayTechAnnotation() || e->isStaffText() || e->isSystemText() || e->isInstrumentChange()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // Jump, Marker
    //-------------------------------------------------------------

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (EngravingItem* e : m->el()) {
            if (e->isJump() || e->isMarker()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // TempoText
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isTempoText()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // layout Voltas for current system
    //-------------------------------------------------------------

    processLines(system, voltas, false);

    //
    // vertical align volta segments
    //
    for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        std::vector<SpannerSegment*> voltaSegments;
        for (SpannerSegment* ss : system->spannerSegments()) {
            if (ss->isVoltaSegment() && ss->staffIdx() == staffIdx) {
                voltaSegments.push_back(ss);
            }
        }
        while (!voltaSegments.empty()) {
            // we assume voltas are sorted left to right (by tick values)
            qreal y = 0;
            int idx = 0;
            Volta* prevVolta = 0;
            for (SpannerSegment* ss : voltaSegments) {
                Volta* volta = toVolta(ss->spanner());
                if (prevVolta && prevVolta != volta) {
                    // check if volta is adjacent to prevVolta
                    if (prevVolta->tick2() != volta->tick()) {
                        break;
                    }
                }
                y = qMin(y, ss->rypos());
                ++idx;
                prevVolta = volta;
            }

            for (int i = 0; i < idx; ++i) {
                SpannerSegment* ss = voltaSegments[i];
                if (ss->autoplace() && ss->isStyled(Pid::OFFSET)) {
                    ss->rypos() = y;
                }
                if (ss->addToSkyline()) {
                    system->staff(staffIdx)->skyline().add(ss->shape().translated(ss->pos()));
                }
            }

            voltaSegments.erase(voltaSegments.begin(), voltaSegments.begin() + idx);
        }
    }

    //-------------------------------------------------------------
    // FretDiagram
    //-------------------------------------------------------------

    if (hasFretDiagram) {
        for (const Segment* s : sl) {
            for (EngravingItem* e : s->annotations()) {
                if (e->isFretDiagram()) {
                    e->layout();
                }
            }
        }

        //-------------------------------------------------------------
        // Harmony, 2nd place
        // We have FretDiagrams, we want the Harmony above this and
        // above the volta.
        //-------------------------------------------------------------

        LayoutHarmonies::layoutHarmonies(sl);
        LayoutHarmonies::alignHarmonies(system, sl, false, options.maxFretShiftAbove, options.maxFretShiftBelow);
    }

    //-------------------------------------------------------------
    // RehearsalMark
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isRehearsalMark()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // Image
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isImage()) {
                e->layout();
            }
        }
    }
}

void LayoutSystem::processLines(System* system, std::vector<Spanner*> lines, bool align)
{
    std::vector<SpannerSegment*> segments;
    for (Spanner* sp : lines) {
        SpannerSegment* ss = sp->layoutSystem(system);         // create/layout spanner segment for this system
        if (ss->autoplace()) {
            segments.push_back(ss);
        }
    }

    if (align && segments.size() > 1) {
        const int nstaves = system->staves()->size();
        constexpr qreal minY = -1000000.0;
        const qreal defaultY = segments[0]->rypos();
        std::vector<qreal> y(nstaves, minY);

        for (SpannerSegment* ss : segments) {
            if (ss->visible()) {
                qreal& staffY = y[ss->staffIdx()];
                staffY = qMax(staffY, ss->rypos());
            }
        }
        for (SpannerSegment* ss : segments) {
            if (!ss->isStyled(Pid::OFFSET)) {
                continue;
            }
            const qreal staffY = y[ss->staffIdx()];
            if (staffY > minY) {
                ss->rypos() = staffY;
            } else {
                ss->rypos() = defaultY;
            }
        }
    }

    //
    // add shapes to skyline
    //
    for (SpannerSegment* ss : segments) {
        if (ss->addToSkyline()) {
            system->staff(ss->staffIdx())->skyline().add(ss->shape().translated(ss->pos()));
        }
    }
}

void LayoutSystem::layoutTies(Chord* ch, System* system, const Fraction& stick)
{
    SysStaff* staff = system->staff(ch->staffIdx());
    if (!staff->show()) {
        return;
    }
    for (Note* note : ch->notes()) {
        Tie* t = note->tieFor();
        if (t) {
            TieSegment* ts = t->layoutFor(system);
            if (ts && ts->addToSkyline()) {
                staff->skyline().add(ts->shape().translated(ts->pos()));
            }
        }
        t = note->tieBack();
        if (t) {
            if (t->startNote()->tick() < stick) {
                TieSegment* ts = t->layoutBack(system);
                if (ts && ts->addToSkyline()) {
                    staff->skyline().add(ts->shape().translated(ts->pos()));
                }
            }
        }
    }
}
