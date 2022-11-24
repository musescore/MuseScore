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

#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/box.h"
#include "libmscore/chord.h"
#include "libmscore/dynamic.h"
#include "libmscore/factory.h"
#include "libmscore/fingering.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/mmrestrange.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafflines.h"
#include "libmscore/system.h"
#include "libmscore/tie.h"
#include "libmscore/tremolo.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"

#include "layoutbeams.h"
#include "layoutchords.h"
#include "layoutharmonies.h"
#include "layoutlyrics.h"
#include "layoutmeasure.h"
#include "layouttuplets.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   collectSystem
//---------------------------------------------------------

System* LayoutSystem::collectSystem(const LayoutOptions& options, LayoutContext& ctx, Score* score)
{
    TRACEFUNC;

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
        ctx.firstSystemIndent  = ctx.firstSystem && options.firstSystemIndent && layoutBreak->firstSystemIndentation();
        ctx.startWithLongNames = ctx.firstSystem && layoutBreak->startWithLongNames();
    }

    System* system = getNextSystem(ctx);
    Fraction lcmTick = ctx.curMeasure->tick();
    system->setInstrumentNames(ctx, ctx.startWithLongNames, lcmTick);

    double curSysWidth    = 0.0;
    double layoutSystemMinWidth = 0.0;
    bool firstMeasure = true;
    bool createHeader = false;
    double targetSystemWidth = score->styleD(Sid::pagePrintableWidth) * DPI;
    system->setWidth(targetSystemWidth);

    // save state of measure
    bool curHeader = ctx.curMeasure->header();
    bool curTrailer = ctx.curMeasure->trailer();
    MeasureBase* breakMeasure = nullptr;

    Fraction minTicks = Fraction::max(); // Initializing at highest possible value
    Fraction prevMinTicks = Fraction(1, 1);
    bool minSysTicksChanged = false;
    Fraction maxTicks = Fraction(0, 1); // Initializing at lowest possible value
    Fraction prevMaxTicks = Fraction(1, 1);
    bool maxSysTicksChanged = false;
    static constexpr double squeezability = 0.3; // We may consider exposing in Style settings (M.S.)
    double oldStretch = 1.0;
    double oldWidth = 0.0;
    System* oldSystem = nullptr;

    while (ctx.curMeasure) {      // collect measure for system
        oldSystem = ctx.curMeasure->system();
        system->appendMeasure(ctx.curMeasure);
        if (system->hasCrossStaffOrModifiedBeams()) {
            updateCrossBeams(system, ctx);
        }
        double ww  = 0.0; // width of current measure
        if (ctx.curMeasure->isMeasure()) {
            Measure* m = toMeasure(ctx.curMeasure);
            // Construct information that is needed before horizontal spacing
            LayoutMeasure::computePreSpacingItems(m);
            // After appending a new measure, the shortest note in the system may change, in which case
            // we need to recompute the layout of the previous measures. When updating the width of these
            // measures, curSysWidth must be updated accordingly.
            Fraction curMinTicks = m->shortestChordRest();
            Fraction curMaxTicks = m->maxTicks();
            if (curMinTicks < minTicks) {
                prevMinTicks = minTicks; // We save the previous value in case we need to restore it (see later)
                minTicks = curMinTicks;
                minSysTicksChanged = true;
            } else {
                minSysTicksChanged = false;
            }
            if (curMaxTicks > maxTicks) {
                prevMaxTicks = maxTicks;
                maxTicks = curMaxTicks;
                maxSysTicksChanged = true;
            } else {
                maxSysTicksChanged = false;
            }
            if (minSysTicksChanged || maxSysTicksChanged) {
                for (MeasureBase* mb : system->measures()) {
                    if (mb == m) {
                        break; // Cause I want to change only previous measures, not current one
                    }
                    if (mb->isMeasure()) {
                        Measure* mm = toMeasure(mb);
                        double prevWidth = mm->width();
                        mm->computeWidth(minTicks, maxTicks, 1);
                        double newWidth = mm->width();
                        curSysWidth += newWidth - prevWidth;
                    }
                }
            }

            if (firstMeasure) {
                layoutSystemMinWidth = curSysWidth;
                system->layoutSystem(ctx, curSysWidth, ctx.firstSystem, ctx.firstSystemIndent);
                if (system->hasCrossStaffOrModifiedBeams()) {
                    updateCrossBeams(system, ctx);
                }
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
            m->computeWidth(minTicks, maxTicks, 1);
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
        double acceptanceRange = squeezability * system->squeezableSpace();
        bool doBreak = (system->measures().size() > 1) && ((curSysWidth + ww) > targetSystemWidth + acceptanceRange);
        /* acceptanceRange allows some systems to be initially slightly larger than the margins and be
         * justified by squeezing instead of stretching. Allows to make much better choices of how many
         * measures to fit per system. */
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
                ctx.prevMeasure = ctx.curMeasure->prev();

                curSysWidth -= system->lastMeasure()->width();
                system->removeLastMeasure();
                ctx.curMeasure->setParent(oldSystem);
            }
            // If the last appended measure caused a re-layout of the previous measures, now that we are
            // removing it we need to re-layout the previous measures again.
            if (minSysTicksChanged) {
                minTicks = prevMinTicks; // If the last measure caused it to change, now we need to restore it!
            }
            if (maxSysTicksChanged) {
                maxTicks = prevMaxTicks;
            }
            if (minSysTicksChanged || maxSysTicksChanged) {
                for (MeasureBase* mb : system->measures()) {
                    if (mb->isMeasure()) {
                        double prevWidth = toMeasure(mb)->width();
                        toMeasure(mb)->computeWidth(minTicks, maxTicks, 1);
                        double newWidth = toMeasure(mb)->width();
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
                double ow = m->width();
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
                        m1->computeWidth(minTicks, maxTicks, 1);
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
                oldWidth = toMeasure(nmb)->width();
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

    assert(ctx.prevMeasure);

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
                    m->computeWidth(m->system()->minSysTicks(), m->system()->maxSysTicks(), oldStretch);
                    m->stretchToTargetWidth(oldWidth);
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

    /*************************************************************
     * SYSTEM NOW HAS A COMPLETE SET OF MEASURES
     * Now perform all operation to finalize system.
     * **********************************************************/

    // Brake cross-measure beams
    // Create end barlines
    if (ctx.prevMeasure && ctx.prevMeasure->isMeasure()) {
        Measure* pm = toMeasure(ctx.prevMeasure);
        LayoutBeams::breakCrossMeasureBeams(ctx, pm);
        pm->createEndBarLines(true);
    }

    // hide empty staves
    hideEmptyStaves(score, system, ctx.firstSystem);
    // Relayout system to account for newly hidden/unhidden staves
    curSysWidth -= system->leftMargin();
    system->layoutSystem(ctx, layoutSystemMinWidth, ctx.firstSystem, ctx.firstSystemIndent);
    curSysWidth += system->leftMargin();

    // add system trailer if needed (cautionary time/key signatures etc)
    Measure* lm  = system->lastMeasure();
    if (lm) {
        Measure* nm = lm->nextMeasure();
        if (nm) {
            lm->addSystemTrailer(nm);
        }
    }

    // Recompute measure widths to account for the last changes (barlines, hidden staves, etc)
    // If system is currently larger than margin (because of acceptanceRange) compute width
    // with a reduced pre-stretch, because justifySystem expects curSysWidth < targetWidth
    double preStretch = targetSystemWidth > curSysWidth ? 1.0 : 1 - squeezability;
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        double oldWidth = m->width();
        m->computeWidth(minTicks, maxTicks, preStretch);
        curSysWidth += m->width() - oldWidth;
    }

    // JUSTIFY SYSTEM
    // Do not justify last system of a section if curSysWidth is < lastSystemFillLimit
    if (!((ctx.curMeasure == 0 || (lm && lm->sectionBreak()))
          && ((curSysWidth / targetSystemWidth) < score->styleD(Sid::lastSystemFillLimit)))
        && !MScore::noHorizontalStretch) { // debug feature
        justifySystem(system, curSysWidth, targetSystemWidth);
    }

    // LAYOUT MEASURES
    PointF pos;
    firstMeasure = true;
    bool createBrackets = false;
    for (MeasureBase* mb : system->measures()) {
        double ww = mb->width();
        if (mb->isMeasure()) {
            if (firstMeasure) {
                pos.rx() += system->leftMargin();
                firstMeasure = false;
            }
            mb->setPos(pos);
            mb->setParent(system);
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
    for (MeasureBase* mb : system->measures()) {
        mb->layoutCrossStaff();
    }
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
        ctx.firstSystemIndent  = ctx.firstSystem && options.firstSystemIndent && layoutBreak->firstSystemIndentation();
        ctx.startWithLongNames = ctx.firstSystem && layoutBreak->startWithLongNames();
    }

    if (oldSystem) {
        // We may have previously processed the ties of the next system (in LayoutChords::updateLineAttachPoints()).
        // We need to restore them to the correct state.
        LayoutSystem::restoreTies(oldSystem);
    }

    return system;
}

void LayoutSystem::justifySystem(System* system, double curSysWidth, double targetSystemWidth)
{
    double rest = targetSystemWidth - curSysWidth;
    if (RealIsNull(rest)) {
        return;
    }
    if (rest < 0) {
        LOGE("*** System justification error ***");
        return;
    }

    std::vector<Spring> springs;

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& s : toMeasure(mb)->segments()) {
            if (s.isChordRestType() && s.visible() && s.enabled() && !s.allElementsInvisible()) {
                double springConst = 1 / s.stretch();
                double width = s.width() - s.widthOffset();
                double preTension = width * springConst;
                springs.push_back(Spring(springConst, width, preTension, &s));
            }
        }
    }

    Segment::stretchSegmentsToWidth(springs, rest);

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        m->respaceSegments();
    }
}

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* LayoutSystem::getNextSystem(LayoutContext& ctx)
{
    Score* score = ctx.score();
    bool isVBox = ctx.curMeasure->isVBox();
    System* system = nullptr;
    if (ctx.systemList.empty()) {
        system = Factory::createSystem(score->dummy()->page());
        ctx.systemOldMeasure = 0;
    } else {
        system = mu::takeFirst(ctx.systemList);
        ctx.systemOldMeasure = system->measures().empty() ? 0 : system->measures().back();
        system->clear();       // remove measures from system
    }
    score->systems().push_back(system);
    if (!isVBox) {
        size_t nstaves = score->Score::nstaves();
        system->adjustStavesNumber(nstaves);
        for (staff_idx_t i = 0; i < nstaves; ++i) {
            system->staff(i)->setShow(score->staff(i)->show());
        }
    }
    return system;
}

void LayoutSystem::hideEmptyStaves(Score* score, System* system, bool isFirstSystem)
{
    size_t staves = score->nstaves();
    staff_idx_t staffIdx = 0;
    bool systemIsEmpty = true;

    for (Staff* staff : score->staves()) {
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
            const size_t n = part->nstaves();
            if (hideStaff && (n > 1)) {
                staff_idx_t idx = part->staves().front()->idx();
                for (staff_idx_t i = 0; i < n; ++i) {
                    staff_idx_t st = idx + i;

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
                            for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
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
        for (Staff* staff : score->staves()) {
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
    if (systemIsEmpty && !score->staves().empty()) {
        Staff* staff = firstVisible ? firstVisible : score->staves().front();
        SysStaff* ss = system->staff(staff->idx());
        ss->setShow(true);
    }
    // Re-create the shapes to account for newly hidden or un-hidden staves
    for (auto mb : system->measures()) {
        if (mb->isMeasure()) {
            for (auto& seg : toMeasure(mb)->segments()) {
                seg.createShapes();
            }
        }
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
        if (!s->isChordRestType()) {
            continue;
        }
        LayoutBeams::layoutNonCrossBeams(s);
        // Must recreate the shapes because stem lengths may have been changed!
        s->createShapes();
    }

    //-------------------------------------------------------------
    //    create skylines
    //-------------------------------------------------------------

    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
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
                    track_idx_t strack = staffIdx * VOICES;
                    track_idx_t etrack = strack + VOICES;
                    for (EngravingItem* e : s.elist()) {
                        if (!e) {
                            continue;
                        }
                        track_idx_t effectiveTrack = e->vStaffIdx() * VOICES + e->voice();
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
        std::set<staff_idx_t> recreateShapes;
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
        for (staff_idx_t staffIdx : recreateShapes) {
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

    std::map<track_idx_t, Fraction> skipTo;
    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->staff(e->staffIdx())->show()) {
                continue;
            }
            track_idx_t track = e->track();
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

    // ties
    doLayoutTies(system, sl, stick, etick);

    // slurs
    std::vector<Spanner*> spanner;
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        sp->computeStartElement();
        sp->computeEndElement();
        lc.processedSpanners.insert(sp);
        if (sp->tick() < etick && sp->tick2() >= stick) {
            if (sp->isSlur() && !toSlur(sp)->isCrossStaff()) {
                // skip cross-staff slurs, will be done after page layout
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
    // Dynamics
    //-------------------------------------------------------------

    std::vector<Dynamic*> dynamics;
    for (Segment* s : sl) {
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
        staff_idx_t si = d->staffIdx();
        Segment* s = d->segment();
        Measure* m = s->measure();
        system->staff(si)->skyline().add(d->shape().translated(d->pos() + s->pos() + m->pos()));
    }

    //-------------------------------------------------------------
    // layout SpannerSegments for current system
    // voltas and tempo change lines are collected here, but laid out later
    //-------------------------------------------------------------

    spanner.clear();
    std::vector<Spanner*> hairpins;
    std::vector<Spanner*> ottavas;
    std::vector<Spanner*> pedal;
    std::vector<Spanner*> voltas;
    std::vector<Spanner*> tempoChangeLines;

    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (sp->tick() < etick && sp->tick2() > stick) {
            if (sp->isOttava()) {
                if (sp->staff()->staffType()->isTabStaff()) {
                    continue;
                }

                ottavas.push_back(sp);
            } else if (sp->isPedal()) {
                pedal.push_back(sp);
            } else if (sp->isVolta()) {
                voltas.push_back(sp);
            } else if (sp->isHairpin()) {
                hairpins.push_back(sp);
            } else if (sp->isGradualTempoChange()) {
                tempoChangeLines.push_back(sp);
            } else if (!sp->isSlur() && !sp->isVolta()) {      // slurs are already
                spanner.push_back(sp);
            }
        }
    }
    processLines(system, hairpins, false);
    processLines(system, spanner, false);
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
        LayoutHarmonies::alignHarmonies(system, sl, true, options.maxChordShiftAbove, options.maxChordShiftBelow);
    }

    //-------------------------------------------------------------
    // StaffText
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isStaffText()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // InstrumentChange
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isInstrumentChange()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // SystemText
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isPlayTechAnnotation() || e->isSystemText() || e->isTripletFeel()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // Jump
    //-------------------------------------------------------------

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (EngravingItem* e : m->el()) {
            if (e->isJump()) {
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
    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        std::vector<SpannerSegment*> voltaSegments;
        for (SpannerSegment* ss : system->spannerSegments()) {
            if (ss->isVoltaSegment() && ss->staffIdx() == staffIdx) {
                voltaSegments.push_back(ss);
            }
        }
        while (!voltaSegments.empty()) {
            // we assume voltas are sorted left to right (by tick values)
            double y = 0;
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
                y = std::min(y, ss->ypos());
                ++idx;
                prevVolta = volta;
            }

            for (int i = 0; i < idx; ++i) {
                SpannerSegment* ss = voltaSegments[i];
                if (ss->autoplace() && ss->isStyled(Pid::OFFSET)) {
                    ss->setPosY(y);
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
    // TempoText, tempo change lines
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isTempoText()) {
                e->layout();
            }
        }
    }
    processLines(system, tempoChangeLines, false);

    //-------------------------------------------------------------
    // Marker
    //-------------------------------------------------------------

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (EngravingItem* e : m->el()) {
            if (e->isMarker()) {
                e->layout();
            }
        }
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

void LayoutSystem::doLayoutTies(System* system, std::vector<Segment*> sl, const Fraction& stick, const Fraction& etick)
{
    UNUSED(etick);

    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Chord* ch : c->graceNotes()) {
                layoutTies(ch, system, stick);
            }
            layoutTies(c, system, stick);
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
        const size_t nstaves = system->staves().size();
        constexpr double minY = -1000000.0;
        const double defaultY = segments[0]->ypos();
        std::vector<double> y(nstaves, minY);

        for (SpannerSegment* ss : segments) {
            if (ss->visible()) {
                double& staffY = y[ss->staffIdx()];
                staffY = std::max(staffY, ss->ypos());
            }
        }
        for (SpannerSegment* ss : segments) {
            if (!ss->isStyled(Pid::OFFSET)) {
                continue;
            }
            const double staffY = y[ss->staffIdx()];
            if (staffY > minY) {
                ss->setPosY(staffY);
            } else {
                ss->setPosY(defaultY);
            }
        }
    }

    if (segments.size() > 1) {
        //how far vertically an endpoint should adjust to avoid other slur endpoints:
        const double slurCollisionVertOffset = 0.65 * system->spatium();
        const double slurCollisionHorizOffset = 0.2 * system->spatium();
        const double fuzzyHorizCompare = 0.25 * system->spatium();
        auto compare = [fuzzyHorizCompare](double x1, double x2) { return std::abs(x1 - x2) < fuzzyHorizCompare; };
        for (SpannerSegment* seg1 : segments) {
            if (!seg1->isSlurSegment()) {
                continue;
            }
            SlurSegment* slur1 = toSlurSegment(seg1);
            for (SpannerSegment* seg2 : segments) {
                if (!seg2->isSlurTieSegment() || seg1 == seg2) {
                    continue;
                }
                if (seg2->isSlurSegment()) {
                    SlurSegment* slur2 = toSlurSegment(seg2);
                    if (slur1->slur()->endChord() == slur2->slur()->startChord()
                        && compare(slur1->ups(Grip::END).p.y(), slur2->ups(Grip::START).p.y())) {
                        slur1->ups(Grip::END).p.rx() -= slurCollisionHorizOffset;
                        slur2->ups(Grip::START).p.rx() += slurCollisionHorizOffset;
                        slur1->computeBezier();
                        slur2->computeBezier();
                        continue;
                    }
                }
                SlurTieSegment* slurTie2 = toSlurTieSegment(seg2);

                // slurs don't collide with themselves or slurs on other staves
                if (slur1->vStaffIdx() != slurTie2->vStaffIdx()) {
                    continue;
                }
                // slurs which don't overlap don't need to be checked
                if (slur1->ups(Grip::END).p.x() < slurTie2->ups(Grip::START).p.x()
                    || slurTie2->ups(Grip::END).p.x() < slur1->ups(Grip::START).p.x()
                    || slur1->slur()->up() != slurTie2->slurTie()->up()) {
                    continue;
                }
                // START POINT
                if (compare(slur1->ups(Grip::START).p.x(), slurTie2->ups(Grip::START).p.x())) {
                    if (slur1->ups(Grip::END).p.x() > slurTie2->ups(Grip::END).p.x() || slurTie2->isTieSegment()) {
                        // slur1 is the "outside" slur
                        slur1->ups(Grip::START).p.ry() += slurCollisionVertOffset * (slur1->slur()->up() ? -1 : 1);
                        slur1->computeBezier();
                    }
                }
                // END POINT
                if (compare(slur1->ups(Grip::END).p.x(), slurTie2->ups(Grip::END).p.x())) {
                    // slurs have the same endpoint
                    if (slur1->ups(Grip::START).p.x() < slurTie2->ups(Grip::START).p.x() || slurTie2->isTieSegment()) {
                        // slur1 is the "outside" slur
                        slur1->ups(Grip::END).p.ry() += slurCollisionVertOffset * (slur1->slur()->up() ? -1 : 1);
                        slur1->computeBezier();
                    }
                }
            }
        }
    }

    //
    // add shapes to skyline
    //
    for (SpannerSegment* ss : segments) {
        if (ss->addToSkyline()) {
            staff_idx_t stfIdx = ss->systemFlag() ? ss->staffIdxOrNextVisible() : ss->staffIdx();
            if (stfIdx == mu::nidx) {
                continue;
            }
            system->staff(stfIdx)->skyline().add(ss->shape().translated(ss->pos()));
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

/****************************************************************************
 * updateCrossBeams
 * Performs a pre-calculation of staff distances (final staff distances will
 * be calculated at the very end of layout) and updates the up() property
 * of cross-beam chords accordingly.
 * *************************************************************************/

void LayoutSystem::updateCrossBeams(System* system, const LayoutContext& ctx)
{
    system->layout2(ctx); // Computes staff distances, essential for the rest of the calculations
    // Update grace cross beams
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg : toMeasure(mb)->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* e : seg.elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                for (Chord* grace : toChord(e)->graceNotes()) {
                    if (grace->beam() && (grace->beam()->cross() || grace->beam()->userModified())) {
                        grace->computeUp();
                    }
                }
            }
        }
    }
    // Update normal chords cross beams and respective segments
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg : toMeasure(mb)->segments()) {
            for (EngravingItem* e : seg.elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* chord = toChord(e);
                if (chord->beam() && (chord->beam()->cross() || chord->beam()->userModified())) {
                    bool prevUp = chord->up();
                    chord->computeUp();
                    if (chord->up() != prevUp) {
                        // If the chord has changed direction needs to be re-laid out
                        LayoutChords::layoutChords1(chord->score(), &seg, chord->vStaffIdx());
                        seg.createShape(chord->vStaffIdx());
                    }
                }
            }
        }
    }
}

void LayoutSystem::restoreTies(System* system)
{
    std::vector<Segment*> segList;
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg : toMeasure(mb)->segments()) {
            if (seg.isChordRestType()) {
                segList.push_back(&seg);
            }
        }
    }
    Fraction stick = system->measures().front()->tick();
    Fraction etick = system->measures().back()->endTick();
    doLayoutTies(system, segList, stick, etick);
}
