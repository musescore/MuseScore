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
#include <cfloat>

#include "systemlayout.h"

#include "realfn.h"
#include "defer.h"

#include "style/defaultstyle.h"

#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/box.h"
#include "dom/bracket.h"
#include "dom/bracketItem.h"
#include "dom/chord.h"
#include "dom/dynamic.h"
#include "dom/factory.h"
#include "dom/guitarbend.h"
#include "dom/instrumentname.h"
#include "dom/layoutbreak.h"
#include "dom/lyrics.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/mmrestrange.h"
#include "dom/note.h"
#include "dom/ornament.h"
#include "dom/part.h"
#include "dom/parenthesis.h"
#include "dom/pedal.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/slur.h"
#include "dom/spacer.h"
#include "dom/staff.h"
#include "dom/stafflines.h"
#include "dom/system.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tuplet.h"
#include "dom/volta.h"

#include "tlayout.h"
#include "alignmentlayout.h"
#include "autoplace.h"
#include "beamlayout.h"
#include "beamtremololayout.h"
#include "chordlayout.h"
#include "harmonylayout.h"
#include "lyricslayout.h"
#include "measurelayout.h"
#include "tupletlayout.h"
#include "slurtielayout.h"
#include "horizontalspacing.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

//---------------------------------------------------------
//   collectSystem
//---------------------------------------------------------

System* SystemLayout::collectSystem(LayoutContext& ctx)
{
    TRACEFUNC;

    if (!ctx.state().curMeasure()) {
        return nullptr;
    }

    const MeasureBase* measure = ctx.dom().systems().empty() ? 0 : ctx.dom().systems().back()->measures().back();
    if (measure) {
        measure = measure->findPotentialSectionBreak();
    }

    bool firstSysLongName = ctx.conf().styleV(Sid::firstSystemInstNameVisibility).value<InstrumentLabelVisibility>()
                            == InstrumentLabelVisibility::LONG;
    bool subsSysLongName = ctx.conf().styleV(Sid::subsSystemInstNameVisibility).value<InstrumentLabelVisibility>()
                           == InstrumentLabelVisibility::LONG;
    if (measure) {
        const LayoutBreak* layoutBreak = measure->sectionBreakElement();
        ctx.mutState().setFirstSystem(measure->sectionBreak() && !ctx.conf().isFloatMode());
        ctx.mutState().setFirstSystemIndent(ctx.state().firstSystem()
                                            && ctx.conf().firstSystemIndent()
                                            && layoutBreak->firstSystemIndentation());
        ctx.mutState().setStartWithLongNames(ctx.state().firstSystem() && firstSysLongName && layoutBreak->startWithLongNames());
    } else {
        ctx.mutState().setStartWithLongNames(ctx.state().firstSystem() && firstSysLongName);
    }

    System* system = getNextSystem(ctx);

    LAYOUT_CALL() << LAYOUT_ITEM_INFO(system);

    Fraction lcmTick = ctx.state().curMeasure()->tick();
    bool longNames = ctx.mutState().firstSystem() ? ctx.mutState().startWithLongNames() : subsSysLongName;
    SystemLayout::setInstrumentNames(system, ctx, longNames, lcmTick);

    double curSysWidth = 0.0;
    double layoutSystemMinWidth = 0.0;
    double targetSystemWidth = ctx.conf().styleD(Sid::pagePrintableWidth) * DPI;
    system->setWidth(targetSystemWidth);

    // save state of measure
    MeasureBase* breakMeasure = nullptr;

    System* oldSystem = nullptr;

    MeasureState prevMeasureState;
    prevMeasureState.curHeader = ctx.state().curMeasure()->header();
    prevMeasureState.curTrailer = ctx.state().curMeasure()->trailer();

    const SystemLock* systemLock = ctx.dom().systemLocks()->lockStartingAt(ctx.state().curMeasure());

    while (ctx.state().curMeasure()) {      // collect measure for system
        oldSystem = ctx.mutState().curMeasure()->system();
        system->appendMeasure(ctx.mutState().curMeasure());

        if (ctx.state().curMeasure()->isMeasure()) {
            Measure* m = toMeasure(ctx.mutState().curMeasure());
            if (!(oldSystem && oldSystem->page() && oldSystem->page() != ctx.state().page())) {
                MeasureLayout::computePreSpacingItems(m, ctx);
            }

            if (measureHasCrossStuffOrModifiedBeams(m)) {
                updateCrossBeams(system, ctx);
            }

            if (m->isFirstInSystem()) {
                layoutSystemMinWidth = curSysWidth;
                SystemLayout::layoutSystem(system, ctx, curSysWidth, ctx.state().firstSystem(), ctx.state().firstSystemIndent());
                MeasureLayout::addSystemHeader(m, ctx.state().firstSystem(), ctx);
            } else {
                bool createHeader = ctx.state().prevMeasure()->isHBox() && toHBox(ctx.state().prevMeasure())->createSystemHeader();
                if (createHeader) {
                    MeasureLayout::addSystemHeader(m, false, ctx);
                } else if (m->header()) {
                    MeasureLayout::removeSystemHeader(m);
                }
            }

            MeasureLayout::createEndBarLines(m, true, ctx);

            if (m->noBreak()) {
                MeasureLayout::removeSystemTrailer(m);
            } else {
                MeasureLayout::addSystemTrailer(m, m->nextMeasure(), ctx);
            }

            MeasureLayout::setRepeatCourtesiesAndParens(m, ctx);

            MeasureLayout::updateGraceNotes(m, ctx);

            curSysWidth = HorizontalSpacing::updateSpacingForLastAddedMeasure(system);
        } else if (ctx.state().curMeasure()->isHBox()) {
            curSysWidth = HorizontalSpacing::updateSpacingForLastAddedMeasure(system);
        } else {
            // vbox:
            MeasureLayout::getNextMeasure(ctx);
            SystemLayout::layout2(system, ctx);         // compute staff distances
            return system;
        }

        bool doBreak = !systemLock && system->measures().size() > 1 && curSysWidth > targetSystemWidth
                       && !ctx.state().prevMeasure()->noBreak();
        if (doBreak) {
            breakMeasure = ctx.mutState().curMeasure();
            system->removeLastMeasure();
            ctx.mutState().curMeasure()->setParent(oldSystem);
            while (ctx.state().prevMeasure() && ctx.state().prevMeasure()->noBreak() && system->measures().size() > 1) {
                ctx.mutState().setTick(ctx.state().tick() - ctx.state().curMeasure()->ticks());
                ctx.mutState().setMeasureNo(ctx.state().curMeasure()->no());

                ctx.mutState().setNextMeasure(ctx.mutState().curMeasure());
                ctx.mutState().setCurMeasure(ctx.mutState().prevMeasure());
                ctx.mutState().setPrevMeasure(ctx.mutState().curMeasure()->prev());

                system->removeLastMeasure();
                ctx.mutState().curMeasure()->setParent(oldSystem);
            }

            break;
        }

        if (oldSystem && system != oldSystem && muse::contains(ctx.state().systemList(), oldSystem)) {
            oldSystem->clear();
        }

        if (ctx.state().prevMeasure() && ctx.state().prevMeasure()->isMeasure() && ctx.state().prevMeasure()->system() == system) {
            Measure* m = toMeasure(ctx.mutState().prevMeasure());

            MeasureLayout::createEndBarLines(m, false, ctx);

            if (m->trailer()) {
                MeasureLayout::removeSystemTrailer(m);
            }

            MeasureLayout::setRepeatCourtesiesAndParens(m, ctx);

            MeasureLayout::updateGraceNotes(m, ctx);

            curSysWidth = HorizontalSpacing::updateSpacingForLastAddedMeasure(system);
        }

        const MeasureBase* mb = ctx.state().curMeasure();
        bool lineBreak  = false;
        switch (ctx.conf().viewMode()) {
        case LayoutMode::PAGE:
        case LayoutMode::SYSTEM:
            lineBreak = mb->pageBreak() || mb->lineBreak() || mb->sectionBreak() || mb->isEndOfSystemLock()
                        || (ctx.state().nextMeasure() && ctx.state().nextMeasure()->isStartOfSystemLock());
            break;
        case LayoutMode::FLOAT:
        case LayoutMode::LINE:
        case LayoutMode::HORIZONTAL_FIXED:
            lineBreak = false;
            break;
        }

        // preserve state of next measure (which is about to become current measure)
        if (ctx.state().nextMeasure()) {
            MeasureBase* nmb = ctx.mutState().nextMeasure();
            if (nmb->isMeasure() && ctx.conf().styleB(Sid::createMultiMeasureRests)) {
                Measure* nm = toMeasure(nmb);
                if (nm->hasMMRest()) {
                    nmb = nm->mmRest();
                }
            }
            if (nmb->isMeasure()) {
                prevMeasureState.clear();
                prevMeasureState.measure = toMeasure(nmb);
                prevMeasureState.measurePos = nmb->x();
                prevMeasureState.measureWidth = nmb->width();
                for (Segment& seg : toMeasure(nmb)->segments()) {
                    prevMeasureState.segmentsPos.emplace_back(&seg, seg.x());
                }
            }
            if (!ctx.state().curMeasure()->noBreak()) {
                // current measure is not a nobreak,
                // so next measure could possibly start a system
                prevMeasureState.curHeader = nmb->header();
            }
            if (!nmb->noBreak()) {
                // next measure is not a nobreak
                // so it could possibly end a system
                prevMeasureState.curTrailer = nmb->trailer();
            }
        }

        MeasureLayout::getNextMeasure(ctx);

        // ElementType nt = lc.curMeasure ? lc.curMeasure->type() : ElementType::INVALID;
        mb = ctx.state().curMeasure();
        if (lineBreak || !mb || mb->isVBox() || mb->isTBox() || mb->isFBox()) {
            break;
        }
    }

    assert(ctx.state().prevMeasure());

    if (ctx.state().endTick() < ctx.state().prevMeasure()->tick()) {
        // we've processed the entire range
        // but we need to continue layout until we reach a system whose last measure is the same as previous layout
        MeasureBase* curMB = ctx.mutState().curMeasure();
        Measure* m = curMB && curMB->isMeasure() ? toMeasure(curMB) : nullptr;
        bool curMeasureMayHaveJoinedBeams = m && BeamLayout::measureMayHaveBeamsJoinedIntoNext(m);
        if (ctx.state().prevMeasure() == ctx.state().systemOldMeasure() && !curMeasureMayHaveJoinedBeams) {
            // If current measure has possible beams joining to the next, we need to continue layout. This needs a better solution in future. [M.S.]
            // this system ends in the same place as the previous layout
            // ok to stop
            if (m) {
                // we may have previously processed first measure(s) of next system
                // so now we must restore to original state
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                const MeasureBase* pbmb = ctx.state().prevMeasure()->findPotentialSectionBreak();
                bool localFirstSystem = pbmb->sectionBreak() && !ctx.conf().isMode(LayoutMode::FLOAT);
                MeasureBase* nm = breakMeasure ? breakMeasure : m;
                if (prevMeasureState.curHeader) {
                    MeasureLayout::addSystemHeader(m, localFirstSystem, ctx);
                } else {
                    MeasureLayout::removeSystemHeader(m);
                }
                for (;;) {
                    // TODO: what if the nobreak group takes the entire system - is this correct?
                    if (prevMeasureState.curTrailer && !m->noBreak()) {
                        MeasureLayout::addSystemTrailer(m, m->nextMeasure(), ctx);
                    } else {
                        MeasureLayout::removeSystemTrailer(m);
                    }

                    MeasureLayout::setRepeatCourtesiesAndParens(m, ctx);

                    prevMeasureState.restoreMeasure();
                    MeasureLayout::layoutMeasureElements(m, ctx);
                    BeamLayout::restoreBeams(m, ctx);
                    if (m == nm || !m->noBreak()) {
                        break;
                    }
                    m = m->nextMeasure();
                }
            }
            ctx.mutState().setRangeDone(true);
        }
    }

    if (system->staves().empty()) {
        // Edge case. Can only happen if all instruments have been deleted.
        return system;
    }

    /*************************************************************
     * SYSTEM NOW HAS A COMPLETE SET OF MEASURES
     * Now perform all operation to finalize system.
     * **********************************************************/

    // Brake cross-measure beams
    if (ctx.state().prevMeasure() && ctx.state().prevMeasure()->isMeasure()) {
        Measure* pm = toMeasure(ctx.mutState().prevMeasure());
        BeamLayout::breakCrossMeasureBeams(pm, ctx);
    }

    // hide empty staves
    hideEmptyStaves(system, ctx, ctx.state().firstSystem());

    // Relayout system to account for newly hidden/unhidden staves
    curSysWidth -= system->leftMargin();
    SystemLayout::layoutSystem(system, ctx, layoutSystemMinWidth, ctx.state().firstSystem(), ctx.state().firstSystemIndent());
    curSysWidth += system->leftMargin();

    // Create end barlines and system trailer if needed (cautionary time/key signatures etc)
    Measure* lm  = system->lastMeasure();
    if (lm) {
        MeasureLayout::createEndBarLines(lm, true, ctx);
        Measure* nm = lm->nextMeasure();
        if (nm) {
            MeasureLayout::addSystemTrailer(lm, nm, ctx);
        }
    }

    updateBigTimeSigIfNeeded(system, ctx);

    // Recompute spacing to account for the last changes (barlines, hidden staves, etc)
    curSysWidth = HorizontalSpacing::computeSpacingForFullSystem(system);

    if (curSysWidth > targetSystemWidth) {
        HorizontalSpacing::squeezeSystemToFit(system, curSysWidth, targetSystemWidth);
    }

    if (shouldBeJustified(system, curSysWidth, targetSystemWidth, ctx)) {
        HorizontalSpacing::justifySystem(system, curSysWidth, targetSystemWidth);
    }

    // LAYOUT MEASURES
    bool createBrackets = false;
    for (MeasureBase* mb : system->measures()) {
        if (mb->isMeasure()) {
            mb->setParent(system);
            Measure* m = toMeasure(mb);
            MeasureLayout::layoutMeasureElements(m, ctx);
            MeasureLayout::layoutStaffLines(m, ctx);
            if (createBrackets) {
                SystemLayout::addBrackets(system, toMeasure(mb), ctx);
                createBrackets = false;
            }
        } else if (mb->isHBox()) {
            HBox* curHBox = toHBox(mb);
            TLayout::layoutMeasureBase(curHBox, ctx);
            createBrackets = curHBox->createSystemHeader();
        }
    }

    layoutSystemElements(system, ctx);
    SystemLayout::layout2(system, ctx);     // compute staff distances

    if (oldSystem && !oldSystem->measures().empty() && oldSystem->measures().front()->tick() >= system->endTick()
        && !(oldSystem->page() && oldSystem->page() != ctx.state().page())) {
        // We may have previously processed the ties of the next system (in LayoutChords::updateLineAttachPoints()).
        // We need to restore them to the correct state.
        SystemLayout::restoreTiesAndBends(oldSystem, ctx);
    }

    return system;
}

bool SystemLayout::shouldBeJustified(System* system, double curSysWidth, double targetSystemWidth, LayoutContext& ctx)
{
    bool shouldJustify = true;

    MeasureBase* lm = system->measures().back();
    if ((curSysWidth / targetSystemWidth) < ctx.conf().styleD(Sid::lastSystemFillLimit)) {
        shouldJustify = false;
        const MeasureBase* lastMb = ctx.state().curMeasure();

        // For systems with a section break, don't justify
        if (lm && lm->sectionBreak()) {
            lastMb = nullptr;
        }

        // Justify if system is followed by a measure or HBox
        while (lastMb) {
            if (lastMb->isMeasure() || lastMb->isHBox()) {
                shouldJustify = true;
                break;
            }
            // Frames can contain section breaks too, account for that here
            if (lastMb->sectionBreak()) {
                shouldJustify = false;
                break;
            }
            lastMb = lastMb->nextMeasure();
        }
    }

    return shouldJustify && !MScore::noHorizontalStretch;
}

void SystemLayout::layoutSystemLockIndicators(System* system, LayoutContext& ctx)
{
    UNUSED(ctx);

    const std::vector<SystemLockIndicator*> lockIndicators = system->lockIndicators();
    // In PAGE view, at most ONE lock indicator can exist per system.
    assert(lockIndicators.size() <= 1);
    system->deleteLockIndicators();

    const SystemLock* lock = system->systemLock();
    if (!lock) {
        return;
    }

    SystemLockIndicator* lockIndicator = new SystemLockIndicator(system, lock);
    lockIndicator->setParent(system);
    system->addLockIndicator(lockIndicator);

    TLayout::layoutSystemLockIndicator(lockIndicator, lockIndicator->mutldata());
}

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* SystemLayout::getNextSystem(LayoutContext& ctx)
{
    bool isVBox = ctx.state().curMeasure()->isVBox();
    System* system = nullptr;
    if (ctx.state().systemList().empty()) {
        system = Factory::createSystem(ctx.mutDom().dummyParent()->page());
        ctx.mutState().setSystemOldMeasure(nullptr);
    } else {
        system = muse::takeFirst(ctx.mutState().systemList());
        ctx.mutState().setSystemOldMeasure(system->measures().empty() ? 0 : system->measures().back());
        system->clear();       // remove measures from system
    }
    ctx.mutDom().systems().push_back(system);
    if (!isVBox) {
        size_t nstaves = ctx.dom().nstaves();
        system->adjustStavesNumber(nstaves);
        for (staff_idx_t i = 0; i < nstaves; ++i) {
            system->staff(i)->setShow(ctx.dom().staff(i)->show());
        }
    }
    return system;
}

void SystemLayout::hideEmptyStaves(System* system, LayoutContext& ctx, bool isFirstSystem)
{
    size_t staves = ctx.dom().nstaves();
    staff_idx_t staffIdx = 0;
    bool systemIsEmpty = true;

    Fraction stick = system->measures().front()->tick();
    Fraction etick = system->measures().back()->endTick();
    auto& spanners = ctx.dom().spannerMap().findOverlapping(stick.ticks(), etick.ticks() - 1);

    for (const Staff* staff : ctx.dom().staves()) {
        SysStaff* ss  = system->staff(staffIdx);

        Staff::HideMode hideMode = staff->hideWhenEmpty();

        if (hideMode == Staff::HideMode::ALWAYS
            || (ctx.conf().styleB(Sid::hideEmptyStaves)
                && (staves > 1)
                && !(isFirstSystem && ctx.conf().styleB(Sid::dontHideStavesInFirstSystem))
                && hideMode != Staff::HideMode::NEVER)) {
            bool hideStaff = true;
            for (auto& spanner : spanners) {
                if (spanner.value->staff() == staff
                    && !spanner.value->systemFlag()
                    && !(spanner.stop == stick.ticks() && !spanner.value->isSlur())) {
                    hideStaff = false;
                    break;
                }
            }
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
                                int staffMove = cr ? cr->staffMove() : 0;
                                if (!cr || cr->isRest() || cr->staffMove() == 0) {
                                    // The case staffMove == 0 has already been checked by measure->isEmpty()
                                    continue;
                                }
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
    const Staff* firstVisible = nullptr;
    if (systemIsEmpty) {
        for (const Staff* staff : ctx.dom().staves()) {
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
    if (systemIsEmpty && !ctx.dom().staves().empty()) {
        const Staff* staff = firstVisible ? firstVisible : ctx.dom().staves().front();
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

void SystemLayout::updateBigTimeSigIfNeeded(System* system, LayoutContext& ctx)
{
    if (ctx.conf().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() != TimeSigPlacement::ABOVE_STAVES) {
        return;
    }

    staff_idx_t nstaves = ctx.dom().nstaves();
    bool centerOnBarline = ctx.conf().styleB(Sid::timeSigCenterOnBarline);

    for (Measure* measure = system->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment& seg : measure->segments()) {
            if (!seg.isType(SegmentType::TimeSigType)) {
                continue;
            }

            std::set<TimeSig*> timeSigToKeep;
            for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                TimeSig* timeSig = toTimeSig(seg.element(staff2track(staffIdx)));
                if (!timeSig || !timeSig->showOnThisStaff()) {
                    continue;
                }

                timeSigToKeep.insert(timeSig);
                if (system->staff(staffIdx)->show()) {
                    continue;
                }

                staff_idx_t nextVisStaff = system->nextVisibleStaff(staffIdx);
                if (nextVisStaff == muse::nidx) {
                    continue;
                }

                TimeSig* nextVisTimeSig = toTimeSig(seg.element(staff2track(nextVisStaff)));
                if (nextVisTimeSig) {
                    timeSigToKeep.insert(nextVisTimeSig);
                }
            }

            Segment* prevBarlineSeg = nullptr;
            Segment* prevRepeatAnnounceTimeSigSeg = nullptr;
            if (centerOnBarline) {
                for (Segment* prevSeg = seg.prev1(); prevSeg && prevSeg->tick() == seg.tick(); prevSeg = prevSeg->prev1()) {
                    if (prevSeg->isEndBarLineType()) {
                        prevBarlineSeg = prevSeg;
                    } else if (prevSeg->isTimeSigRepeatAnnounceType()) {
                        prevRepeatAnnounceTimeSigSeg = prevSeg;
                    }
                }
            }

            for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                TimeSig* timeSig = toTimeSig(seg.element(staff2track(staffIdx)));
                if (!timeSig) {
                    continue;
                }
                std::vector<EngravingItem*> parens = seg.findAnnotations(ElementType::PARENTHESIS, timeSig->track(), timeSig->track());
                if (!muse::contains(timeSigToKeep, timeSig)) {
                    timeSig->mutldata()->reset(); // Eliminates the shape
                    for (EngravingItem* paren : parens) {
                        paren->mutldata()->reset();
                    }
                    continue;
                }

                if (prevBarlineSeg && prevBarlineSeg->system() == system && !prevRepeatAnnounceTimeSigSeg) {
                    // Center timeSig on its segment
                    RectF bbox = timeSig->ldata()->bbox();
                    double newXPos = -0.5 * (bbox.right() + bbox.left());
                    double xPosDiff = timeSig->pos().x() - newXPos;
                    timeSig->mutldata()->setPosX(newXPos);

                    for (EngravingItem* el : parens) {
                        el->mutldata()->moveX(-xPosDiff);
                    }
                } else if (!seg.isTimeSigRepeatAnnounceType()) {
                    // Left-align to parenthesis if present
                    double xLeftParens = DBL_MAX;
                    for (EngravingItem* paren : parens) {
                        if (toParenthesis(paren)->direction() == DirectionH::LEFT) {
                            xLeftParens = paren->x() + paren->ldata()->bbox().left();
                        }
                    }
                    if (xLeftParens != DBL_MAX) {
                        timeSig->mutldata()->moveX(-xLeftParens);
                        for (EngravingItem* paren : parens) {
                            paren->mutldata()->moveX(-xLeftParens);
                        }
                    }
                } else {
                    // TimeSigRepeatAnnounce: right-align to segment
                    double xRight = -DBL_MAX;
                    xRight = std::max(xRight, timeSig->shape().right() + timeSig->x());
                    for (EngravingItem* paren : parens) {
                        xRight = std::max(xRight, paren->ldata()->bbox().right() + paren->x());
                    }
                    if (xRight != -DBL_MAX) {
                        timeSig->mutldata()->moveX(-xRight);
                        for (EngravingItem* paren : parens) {
                            paren->mutldata()->moveX(-xRight);
                        }
                    }
                }
            }

            seg.createShapes();
        }
    }
}

void SystemLayout::layoutSystemElements(System* system, LayoutContext& ctx)
{
    if (ctx.dom().nstaves() == 0) {
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
        MeasureLayout::layoutMeasureNumber(m, ctx);
        MeasureLayout::layoutMMRestRange(m, ctx);
        MeasureLayout::layoutTimeTickAnchors(m, ctx);

        // in continuous view, entire score is one system
        // but we only need to process the range
        if (ctx.conf().isLinearMode() && (m->tick() < ctx.state().startTick() || m->tick() > ctx.state().endTick())) {
            continue;
        }
        for (Segment* s = m->first(); s; s = s->next()) {
            if (s->isChordRestType() || !s->annotations().empty()) {
                sl.push_back(s);
            }
        }
    }

    if (sl.empty()) {
        return;
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
        BeamLayout::layoutNonCrossBeams(s, ctx);
        // Must recreate the shapes because stem lengths may have been changed!
        s->createShapes();
    }

    for (Segment* s : sl) {
        for (EngravingItem* item : s->elist()) {
            if (!item || !item->isRest()) {
                continue;
            }
            Rest* rest = toRest(item);
            Beam* beam = rest->beam();
            if (beam && !beam->cross() && !beam->fullCross()) {
                BeamLayout::verticalAdjustBeamedRests(rest, beam, ctx);
            }
        }
    }

    //-------------------------------------------------------------
    //    create skylines
    //-------------------------------------------------------------

    std::vector<MeasureNumber*> measureNumbers;
    std::vector<MMRestRange*> mmrRanges;

    for (size_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
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
            if (ctx.conf().isLinearMode() && (m->tick() < ctx.state().startTick() || m->tick() > ctx.state().endTick())) {
                continue;
            }
            if (mno && mno->addToSkyline()) {
                measureNumbers.push_back(mno);
            }
            if (mmrr && mmrr->addToSkyline()) {
                mmrRanges.push_back(mmrr);
            }
            if (m->staffLines(staffIdx)->addToSkyline()) {
                ss->skyline().add(m->staffLines(staffIdx)->ldata()->bbox().translated(m->pos()), m->staffLines(staffIdx));
            }
            for (Segment& s : m->segments()) {
                if (!s.enabled()) {
                    continue;
                }
                PointF p(s.pos() + m->pos());
                if (s.segmentType()
                    & (SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine)) {
                    BarLine* bl = toBarLine(s.element(staffIdx * VOICES));
                    if (bl && bl->addToSkyline()) {
                        RectF r = TLayout::layoutRect(bl, ctx);
                        skyline.add(r.translated(bl->pos() + p + bl->staffOffset()), bl);
                    }
                } else if (s.isType(SegmentType::TimeSigType)) {
                    TimeSig* ts = toTimeSig(s.element(staffIdx * VOICES));
                    if (ts && ts->addToSkyline() && ts->showOnThisStaff()) {
                        TimeSigPlacement timeSigPlacement = ts->style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>();
                        if (timeSigPlacement != TimeSigPlacement::ACROSS_STAVES) {
                            skyline.add(ts->shape().translate(ts->pos() + p + ts->staffOffset()));
                        }
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

                        // add element to skyline
                        if (e->addToSkyline()) {
                            const PointF offset = e->staffOffset();
                            skyline.add(e->shape().translate(e->pos() + p + offset));
                            // add grace notes to skyline
                            if (e->isChord()) {
                                GraceNotesGroup& graceBefore = toChord(e)->graceNotesBefore();
                                GraceNotesGroup& graceAfter = toChord(e)->graceNotesAfter();
                                TLayout::layoutGraceNotesGroup2(&graceBefore, graceBefore.mutldata());
                                TLayout::layoutGraceNotesGroup2(&graceAfter, graceAfter.mutldata());
                                if (!graceBefore.empty()) {
                                    skyline.add(graceBefore.shape().translate(graceBefore.pos() + p + offset));
                                }
                                if (!graceAfter.empty()) {
                                    skyline.add(graceAfter.shape().translate(graceAfter.pos() + p + offset));
                                }
                            }
                            // If present, add ornament cue note to skyline
                            if (e->isChord()) {
                                Ornament* ornament = toChord(e)->findOrnament();
                                if (ornament) {
                                    Chord* cue = ornament->cueNoteChord();
                                    if (cue && cue->upNote()->visible()) {
                                        skyline.add(cue->shape().translate(cue->pos() + p + cue->staffOffset()));
                                    }
                                }
                            }
                        }

                        // add tremolo to skyline
                        if (e->isChord()) {
                            Chord* ch = item_cast<Chord*>(e);
                            if (ch->tremoloSingleChord()) {
                                TremoloSingleChord* t = ch->tremoloSingleChord();
                                if (t->addToSkyline()) {
                                    skyline.add(t->shape().translate(t->pos() + e->pos() + p));
                                }
                            } else if (ch->tremoloTwoChord()) {
                                TremoloTwoChord* t = ch->tremoloTwoChord();
                                Chord* c1 = t->chord1();
                                Chord* c2 = t->chord2();
                                if (c1 && !c1->staffMove() && c2 && !c2->staffMove()) {
                                    if (t->chord() == e && t->addToSkyline()) {
                                        skyline.add(t->shape().translate(t->pos() + e->pos() + p));
                                    }
                                }
                            }
                        }

                        // add beams to skline
                        if (e->isChordRest()) {
                            ChordRest* cr = toChordRest(e);
                            if (BeamLayout::isTopBeam(cr)) {
                                Beam* b = cr->beam();
                                b->addSkyline(skyline);
                            }
                        }
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------
    // layout ties and guitar bends
    //-------------------------------------------------------------

    bool useRange = false;    // TODO: lineMode();
    Fraction stick = useRange ? ctx.state().startTick() : system->measures().front()->tick();
    Fraction etick = useRange ? ctx.state().endTick() : system->measures().back()->endTick();
    auto spanners = ctx.dom().spannerMap().findOverlapping(stick.ticks(), etick.ticks());
    std::sort(spanners.begin(), spanners.end(), [](const auto& sp1, const auto& sp2) {
        return sp1.value->tick() < sp2.value->tick();
    });

    // ties
    if (ctx.conf().isLinearMode()) {
        doLayoutNoteSpannersLinear(system, ctx);
    } else {
        doLayoutTies(system, sl, stick, etick, ctx);
    }
    // guitar bends
    layoutGuitarBends(sl, ctx);

    //-------------------------------------------------------------
    // layout articulations, fingering and stretched bends
    //-------------------------------------------------------------

    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChord() || !ctx.dom().staff(e->staffIdx())->show()) {
                continue;
            }
            Chord* c = toChord(e);
            ChordLayout::layoutArticulations(c, ctx);
            ChordLayout::layoutArticulations2(c, ctx);
            ChordLayout::layoutChordBaseFingering(c, system, ctx);
        }
    }

    //-------------------------------------------------------------
    // layout tuplets
    //-------------------------------------------------------------

    std::map<track_idx_t, Fraction> skipTo;
    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChordRest() || !ctx.dom().staff(e->staffIdx())->show()) {
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
            TupletLayout::layout(de, ctx); // recursively lay out all tuplets covered by this tuplet

            // don't layout any tuplets covered by this top level tuplet for this voice--
            // they've already been laid out by layoutTuplet().
            skipTo[track] = de->tick() + de->actualTicks();
        }
    }

    //-------------------------------------------------------------
    // layout slurs
    //-------------------------------------------------------------

    // slurs
    std::vector<Spanner*> spanner;
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (sp->staff() && !sp->staff()->show()) {
            continue;
        }

        sp->computeStartElement();
        sp->computeEndElement();
        ctx.mutState().processedSpanners().insert(sp);
        if (sp->tick() < etick && sp->tick2() >= stick) {
            if (sp->isSlur() && !toSlur(sp)->isCrossStaff()) {
                // skip cross-staff slurs, will be done after page layout
                spanner.push_back(sp);
            }
        }
    }
    processLines(system, ctx, spanner);
    for (auto s : spanner) {
        Slur* slur = toSlur(s);
        ChordRest* scr = s->startCR();
        ChordRest* ecr = s->endCR();
        if (scr && scr->isChord()) {
            ChordLayout::layoutArticulations3(toChord(scr), slur, ctx);
        }
        if (ecr && ecr->isChord()) {
            ChordLayout::layoutArticulations3(toChord(ecr), slur, ctx);
        }
    }

    //-------------------------------------------------------------
    // Trills
    //-------------------------------------------------------------
    std::vector<Spanner*> trills;
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (sp->staff() && !sp->staff()->show()) {
            continue;
        }
        if (sp->tick() < etick && sp->tick2() > stick && sp->isTrill()) {
            trills.push_back(sp);
        }
    }
    processLines(system, ctx, trills);

    //-------------------------------------------------------------
    // Drumline sticking
    //-------------------------------------------------------------
    struct StaffStickingGroups {
        std::vector<EngravingItem*> stickingsAbove;
        std::vector<EngravingItem*> stickingsBelow;
    };
    std::map<staff_idx_t, StaffStickingGroups> staffStickings;
    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isSticking()) {
                TLayout::layoutItem(e, ctx);
                if (e->addToSkyline()) {
                    e->placeAbove() ? staffStickings[e->staffIdx()].stickingsAbove.push_back(e) : staffStickings[e->staffIdx()].
                    stickingsBelow.push_back(e);
                }
            }
        }
    }
    for (const auto& staffSticking : staffStickings) {
        AlignmentLayout::alignItemsGroup(staffSticking.second.stickingsAbove, system);
        AlignmentLayout::alignItemsGroup(staffSticking.second.stickingsBelow, system);
    }

    //-------------------------------------------------------------
    // Fermata, TremoloBar
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isFermata() || e->isTremoloBar()) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }

    std::vector<EngravingItem*> dynamicsExprAndHairpinsToAlign;

    //-------------------------------------------------------------
    // Dynamics and figured bass
    //-------------------------------------------------------------

    std::vector<EngravingItem*> dynamicsAndFigBass;
    for (Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isDynamic() || e->isFiguredBass()) {
                TLayout::layoutItem(e, ctx);
                if (e->autoplace()) {
                    if (e->isDynamic()) {
                        toDynamic(e)->manageBarlineCollisions();
                        dynamicsExprAndHairpinsToAlign.push_back(e);
                    }
                    Autoplace::autoplaceSegmentElement(e, e->mutldata(), false);
                    dynamicsAndFigBass.push_back(e);
                }
            }
        }
    }

    // add dynamics shape to skyline
    for (EngravingItem* e : dynamicsAndFigBass) {
        if (!e->addToSkyline()) {
            continue;
        }
        EngravingItem* parent = e->parentItem(true);
        IF_ASSERT_FAILED(parent && parent->isSegment()) {
            continue;
        }
        staff_idx_t si = e->staffIdx();
        Segment* s = toSegment(parent);
        Measure* m = s->measure();
        system->staff(si)->skyline().add(e->shape().translate(e->pos() + s->pos() + m->pos() + e->staffOffset()));
    }

    //-------------------------------------------------------------
    // Expressions
    // Must be done after dynamics. Remember that expressions may
    // also snap into alignment with dynamics.
    //-------------------------------------------------------------
    for (Segment* s : sl) {
        Measure* m = s->measure();
        for (EngravingItem* e : s->annotations()) {
            if (e->isExpression()) {
                TLayout::layoutItem(e, ctx);
                if (e->addToSkyline()) {
                    dynamicsExprAndHairpinsToAlign.push_back(e);
                    system->staff(e->staffIdx())->skyline().add(e->shape().translate(e->pos() + s->pos() + m->pos()));
                }
            }
        }
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
    std::vector<Spanner*> partialLyricsLines;

    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (!sp->systemFlag() && sp->staff() && !sp->staff()->show()) {
            continue;
        }

        const Measure* startMeas = sp->findStartMeasure();
        const Measure* endMeas = sp->findEndMeasure();
        if (!sp->visible() && ((startMeas && startMeas->isMMRest()) || (endMeas && endMeas->isMMRest()))
            && ctx.conf().styleB(Sid::createMultiMeasureRests)) {
            continue;
        }
        if (sp->tick2() == stick && sp->isPedal() && toPedal(sp)->connect45HookToNext()) {
            pedal.push_back(sp);
        }

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
            } else if (sp->isPartialLyricsLine()) {
                partialLyricsLines.push_back(sp);
            } else if (!sp->isSlur() && !sp->isVolta() && !sp->isTrill()) {      // slurs are already
                spanner.push_back(sp);
            }
        }
    }
    processLines(system, ctx, hairpins);

    for (SpannerSegment* spannerSegment : system->spannerSegments()) {
        if (spannerSegment->isHairpinSegment()) {
            dynamicsExprAndHairpinsToAlign.push_back(spannerSegment);
        }
    }

    AlignmentLayout::alignItemsWithTheirSnappingChain(dynamicsExprAndHairpinsToAlign, system);

    processLines(system, ctx, spanner);
    for (MeasureNumber* mno : measureNumbers) {
        Autoplace::autoplaceMeasureElement(mno, mno->mutldata());
        system->staff(mno->staffIdx())->skyline().add(mno->ldata()->bbox().translated(mno->measure()->pos() + mno->pos()
                                                                                      + mno->staffOffset()), mno);
    }

    for (MMRestRange* mmrr : mmrRanges) {
        Autoplace::autoplaceMeasureElement(mmrr, mmrr->mutldata());
        system->staff(mmrr->staffIdx())->skyline().add(mmrr->ldata()->bbox().translated(mmrr->measure()->pos() + mmrr->pos()), mmrr);
    }

    processLines(system, ctx, ottavas);
    processLines(system, ctx, pedal, /*align=*/ true);

    for (Spanner* sp : partialLyricsLines) {
        TLayout::layoutSystem(sp, system, ctx);
    }

    //-------------------------------------------------------------
    // Lyric
    //-------------------------------------------------------------
    // Layout lyrics dashes and melisma
    // NOTE: loop on a *copy* of unmanagedSpanners because in some cases
    // the underlying operation may invalidate some of the iterators.
    bool dashOnFirstNoteSyllable = ctx.conf().style().styleB(Sid::lyricsShowDashIfSyllableOnFirstNote);
    std::set<Spanner*> unmanagedSpanners = ctx.dom().unmanagedSpanners();
    for (Spanner* sp : unmanagedSpanners) {
        if (!sp->systemFlag() && sp->staff() && !sp->staff()->show()) {
            continue;
        }
        bool dashOnFirst = dashOnFirstNoteSyllable && !toLyricsLine(sp)->isEndMelisma();
        if (sp->tick() >= etick || sp->tick2() < stick || (sp->tick2() == stick && !dashOnFirst)) {
            continue;
        }
        TLayout::layoutSystem(sp, system, ctx);
    }
    LyricsLayout::computeVerticalPositions(system, ctx);

    //-------------------------------------------------------------
    // Harp pedal diagrams
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isHarpPedalDiagram()) {
                TLayout::layoutItem(e, ctx);
            }
        }
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
        HarmonyLayout::autoplaceHarmonies(sl, ctx);
        HarmonyLayout::alignHarmonies(system, sl, true, ctx.conf().maxChordShiftAbove(), ctx.conf().maxChordShiftBelow());
    }

    //-------------------------------------------------------------
    // StaffText
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isStaffText()) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }

    //-------------------------------------------------------------
    // InstrumentChange
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isInstrumentChange()) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }

    //-------------------------------------------------------------
    // SystemText
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isPlayTechAnnotation() || e->isCapo() || e->isStringTunings() || e->isSystemText() || e->isTripletFeel()) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }

    //-------------------------------------------------------------
    // FretDiagram
    //-------------------------------------------------------------

    if (hasFretDiagram) {
        for (const Segment* s : sl) {
            for (EngravingItem* e : s->annotations()) {
                if (e->isFretDiagram()) {
                    Autoplace::autoplaceSegmentElement(e, e->mutldata());
                    if (Harmony* harmony = toFretDiagram(e)->harmony()) {
                        SkylineLine& skl = system->staff(e->staffIdx())->skyline().north();
                        Shape harmShape = harmony->ldata()->shape().translated(harmony->pos() + e->pos() + s->pos() + s->measure()->pos());
                        skl.add(harmShape);
                    }
                }
            }
        }

        //-------------------------------------------------------------
        // Harmony, 2nd place
        //-------------------------------------------------------------

        HarmonyLayout::autoplaceHarmonies(sl, ctx);
        HarmonyLayout::alignHarmonies(system, sl, false, ctx.conf().maxFretShiftAbove(), ctx.conf().maxFretShiftBelow());
    }

    //-------------------------------------------------------------
    // layout Voltas for current system
    //-------------------------------------------------------------

    processLines(system, ctx, voltas);

    //
    // vertical align volta segments
    //
    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
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
            Volta* prevVolta = nullptr;
            for (SpannerSegment* ss : voltaSegments) {
                Volta* volta = toVolta(ss->spanner());
                if (prevVolta && prevVolta != volta) {
                    // check if volta is adjacent to prevVolta
                    if (prevVolta->tick2() != volta->tick()) {
                        break;
                    }
                }
                if (ss->addToSkyline()) {
                    y = std::min(y, ss->ldata()->pos().y());
                }
                ++idx;
                prevVolta = volta;
            }

            for (int i = 0; i < idx; ++i) {
                SpannerSegment* ss = voltaSegments[i];
                if (ss->autoplace() && ss->isStyled(Pid::OFFSET)) {
                    ss->mutldata()->setPosY(y);
                }
                if (ss->addToSkyline()) {
                    system->staff(staffIdx)->skyline().add(ss->shape().translate(ss->pos()));
                }
            }

            voltaSegments.erase(voltaSegments.begin(), voltaSegments.begin() + idx);
        }
    }

    //-------------------------------------------------------------
    // RehearsalMark
    //-------------------------------------------------------------
    // Layout before tempo text but autoplace after
    std::vector<RehearsalMark*> rehearsMarks;
    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isRehearsalMark()) {
                TLayout::layoutItem(e, ctx);
                rehearsMarks.push_back(toRehearsalMark(e));
            }
        }
    }

    //-------------------------------------------------------------
    // TempoText, tempo change lines
    //-------------------------------------------------------------

    std::vector<EngravingItem*> tempoElementsToAlign;

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isTempoText()) {
                TLayout::layoutItem(e, ctx);
                tempoElementsToAlign.push_back(e);
            }
        }
    }

    processLines(system, ctx, tempoChangeLines);
    for (SpannerSegment* spannerSeg : system->spannerSegments()) {
        if (spannerSeg->isGradualTempoChangeSegment()) {
            tempoElementsToAlign.push_back(spannerSeg);
        }
    }

    AlignmentLayout::alignItemsWithTheirSnappingChain(tempoElementsToAlign, system);

    for (RehearsalMark* rehearsMark : rehearsMarks) {
        Autoplace::autoplaceSegmentElement(rehearsMark, rehearsMark->mutldata());
    }

    //-------------------------------------------------------------
    // Marker and Jump
    //-------------------------------------------------------------

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (EngravingItem* e : m->el()) {
            if (e->isMarker() || e->isJump()) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }

    //-------------------------------------------------------------
    // Image
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isImage()) {
                TLayout::layoutItem(e, ctx);
            }
        }
    }

    //-------------------------------------------------------------
    // Parenthesis
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (EngravingItem* e : s->annotations()) {
            if (!e->isParenthesis() || !e->addToSkyline()) {
                continue;
            }

            if (s->isType(SegmentType::TimeSigType)) {
                EngravingItem* el = s->element(e->track());
                TimeSig* timeSig = el ? toTimeSig(el) : nullptr;
                if (!timeSig) {
                    continue;
                }
                TimeSigPlacement timeSigPlacement = timeSig->style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>();
                if (timeSigPlacement == TimeSigPlacement::ACROSS_STAVES) {
                    if (!timeSig->showOnThisStaff()) {
                        e->mutldata()->reset();
                    }
                    continue;
                }
            }

            staff_idx_t si = e->staffIdx();
            Measure* m = s->measure();
            system->staff(si)->skyline().add(e->shape().translate(e->pos() + s->pos() + m->pos() + e->staffOffset()));
        }
    }

    //-------------------------------------------------------------
    // TimeSig above staff
    //-------------------------------------------------------------

    if (system->style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() == TimeSigPlacement::ABOVE_STAVES) {
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            for (Segment& s : toMeasure(mb)->segments()) {
                if (!s.isType(SegmentType::TimeSigType)) {
                    continue;
                }
                for (EngravingItem* timeSig : s.elist()) {
                    if (!timeSig || !toTimeSig(timeSig)->showOnThisStaff()) {
                        continue;
                    }
                    const double yBefore = timeSig->pos().y();
                    Autoplace::autoplaceSegmentElement(timeSig, timeSig->mutldata());
                    const double yAfter = timeSig->pos().y();
                    const double yPosDiff = yAfter - yBefore;
                    std::vector<EngravingItem*> parens = s.findAnnotations(ElementType::PARENTHESIS,
                                                                           timeSig->track(), timeSig->track());
                    for (EngravingItem* el : parens) {
                        el->mutldata()->moveY(yPosDiff);
                    }
                }
            }
        }
    }
}

void SystemLayout::doLayoutTies(System* system, std::vector<Segment*> sl, const Fraction& stick, const Fraction& etick, LayoutContext& ctx)
{
    UNUSED(etick);

    for (Segment* s : sl) {
        for (EngravingItem* e : s->elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Chord* ch : c->graceNotes()) {
                layoutTies(ch, system, stick, ctx);
            }
            layoutTies(c, system, stick, ctx);
        }
    }
}

void SystemLayout::layoutNoteAnchoredSpanners(System* system, Chord* chord)
{
    // Add all spanners attached to notes, otherwise these will be removed if outside of the layout range
    for (Note* note : chord->notes()) {
        for (Spanner* spanner : note->spannerFor()) {
            for (SpannerSegment* spannerSeg : spanner->spannerSegments()) {
                spannerSeg->setSystem(system);
            }
        }
    }
}

void SystemLayout::doLayoutNoteSpannersLinear(System* system, LayoutContext& ctx)
{
    constexpr Fraction start = Fraction(0, 1);
    for (Measure* measure = system->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (!segment->isChordRestType()) {
                continue;
            }
            for (EngravingItem* e : segment->elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* c = toChord(e);
                for (Chord* ch : c->graceNotes()) {
                    layoutTies(ch, system, start, ctx);
                    layoutNoteAnchoredSpanners(system, ch);
                }
                layoutTies(c, system, start, ctx);
                layoutNoteAnchoredSpanners(system, c);
            }
        }
    }
}

void SystemLayout::layoutGuitarBends(const std::vector<Segment*>& sl, LayoutContext& ctx)
{
    auto doLayoutGuitarBends = [&] (Chord* chord) {
        for (Note* note : chord->notes()) {
            GuitarBend* bendBack = note->bendBack();
            if (bendBack) {
                TLayout::layoutGuitarBend(bendBack, ctx);
            }

            Note* startOfTie = note->firstTiedNote();
            if (startOfTie != note) {
                GuitarBend* bendBack2 = startOfTie->bendBack();
                if (bendBack2) {
                    TLayout::layoutGuitarBend(bendBack2, ctx);
                }
            }

            GuitarBend* bendFor = note->bendFor();
            if (bendFor && bendFor->type() == GuitarBendType::SLIGHT_BEND) {
                TLayout::layoutGuitarBend(bendFor, ctx);
            }
        }
    };

    for (Segment* seg : sl) {
        for (EngravingItem* el : seg->elist()) {
            if (!el || !el->isChord()) {
                continue;
            }
            Chord* chord = toChord(el);
            for (Chord* grace : chord->graceNotesBefore()) {
                doLayoutGuitarBends(grace);
            }
            doLayoutGuitarBends(chord);
            for (Chord* grace : chord->graceNotesAfter()) {
                doLayoutGuitarBends(grace);
            }
        }
    }
}

void SystemLayout::processLines(System* system, LayoutContext& ctx, std::vector<Spanner*> lines, bool align)
{
    std::vector<SpannerSegment*> segments;
    for (Spanner* sp : lines) {
        SpannerSegment* ss = TLayout::layoutSystem(sp, system, ctx);        // create/layout spanner segment for this system
        if (ss->autoplace()) {
            segments.push_back(ss);
        }
    }

    if (align && segments.size() > 1) {
        const size_t nstaves = system->staves().size();
        const double defaultY = segments[0]->ldata()->pos().y();
        std::vector<double> yAbove(nstaves, -DBL_MAX);
        std::vector<double> yBelow(nstaves, -DBL_MAX);

        for (SpannerSegment* ss : segments) {
            if (ss->visible()) {
                double& staffY = ss->spanner() && ss->spanner()->placeAbove() ? yAbove[ss->staffIdx()] : yBelow[ss->staffIdx()];
                staffY = std::max(staffY, ss->ldata()->pos().y());
            }
        }
        for (SpannerSegment* ss : segments) {
            if (!ss->isStyled(Pid::OFFSET)) {
                continue;
            }
            const double& staffY = ss->spanner() && ss->spanner()->placeAbove() ? yAbove[ss->staffIdx()] : yBelow[ss->staffIdx()];
            if (staffY > -DBL_MAX) {
                ss->mutldata()->setPosY(staffY);
            } else {
                ss->mutldata()->setPosY(defaultY);
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
                        SlurTieLayout::computeBezier(slur1);
                        SlurTieLayout::computeBezier(slur2);
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
                        SlurTieLayout::computeBezier(slur1);
                    }
                }
                // END POINT
                if (compare(slur1->ups(Grip::END).p.x(), slurTie2->ups(Grip::END).p.x())) {
                    // slurs have the same endpoint
                    if (slur1->ups(Grip::START).p.x() < slurTie2->ups(Grip::START).p.x() || slurTie2->isTieSegment()) {
                        // slur1 is the "outside" slur
                        slur1->ups(Grip::END).p.ry() += slurCollisionVertOffset * (slur1->slur()->up() ? -1 : 1);
                        SlurTieLayout::computeBezier(slur1);
                    }
                }
            }
        }
    }
    //
    // Fix harmonic marks and vibrato overlaps
    //
    SpannerSegment* prevSegment = nullptr;
    bool fixed = false;

    for (SpannerSegment* ss : segments) {
        if (fixed) {
            fixed = false;
            prevSegment = ss;
            continue;
        }
        if (prevSegment) {
            if (prevSegment->visible()
                && ss->visible()
                && prevSegment->isHarmonicMarkSegment()
                && ss->isVibratoSegment()
                && muse::RealIsEqual(prevSegment->x(), ss->x())) {
                double diff = ss->ldata()->bbox().bottom() - prevSegment->ldata()->bbox().bottom()
                              + prevSegment->ldata()->bbox().top();
                prevSegment->mutldata()->moveY(diff);
                fixed = true;
            }
            if (prevSegment->visible()
                && ss->visible()
                && prevSegment->isVibratoSegment()
                && ss->isHarmonicMarkSegment()
                && muse::RealIsEqual(prevSegment->x(), ss->x())) {
                double diff = prevSegment->ldata()->bbox().bottom() - ss->ldata()->bbox().bottom()
                              + ss->ldata()->bbox().top();
                ss->mutldata()->moveY(diff);
                fixed = true;
            }
        }

        prevSegment = ss;
    }

    //
    // add shapes to skyline
    //
    for (SpannerSegment* ss : segments) {
        if (ss->addToSkyline()) {
            staff_idx_t stfIdx = ss->effectiveStaffIdx();
            if (stfIdx == muse::nidx) {
                continue;
            }
            system->staff(stfIdx)->skyline().add(ss->shape().translate(ss->pos()));
        }
    }
}

void SystemLayout::layoutTies(Chord* ch, System* system, const Fraction& stick, LayoutContext& ctx)
{
    SysStaff* staff = system->staff(ch->staffIdx());
    if (!staff->show()) {
        return;
    }
    std::vector<TieSegment*> stackedForwardTies;
    std::vector<TieSegment*> stackedBackwardTies;
    for (Note* note : ch->notes()) {
        Tie* t = note->tieFor();
        if (t && !t->isLaissezVib()) {
            TieSegment* ts = SlurTieLayout::layoutTieFor(t, system);
            if (ts && ts->addToSkyline()) {
                staff->skyline().add(ts->shape().translate(ts->pos()));
                stackedForwardTies.push_back(ts);
            }
        }
        t = note->tieBack();
        if (t) {
            if (note->incomingPartialTie() || t->startNote()->tick() < stick) {
                TieSegment* ts = SlurTieLayout::layoutTieBack(t, system, ctx);
                if (ts && ts->addToSkyline()) {
                    staff->skyline().add(ts->shape().translate(ts->pos()));
                    stackedBackwardTies.push_back(ts);
                }
            }
        }
    }

    SlurTieLayout::layoutLaissezVibChord(ch, ctx);

    if (!ch->staffType()->isTabStaff()) {
        SlurTieLayout::resolveVerticalTieCollisions(stackedForwardTies);
        SlurTieLayout::resolveVerticalTieCollisions(stackedBackwardTies);
    }
}

bool SystemLayout::measureHasCrossStuffOrModifiedBeams(const Measure* measure)
{
    for (const Segment& seg : measure->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (const EngravingItem* e : seg.elist()) {
            if (!e || !e->isChordRest()) {
                continue;
            }
            const Beam* beam = toChordRest(e)->beam();
            if (beam && (beam->cross() || beam->userModified())) {
                return true;
            }
            const Chord* c = e->isChord() ? toChord(e) : nullptr;
            if (c && c->tremoloTwoChord()) {
                const TremoloTwoChord* trem = c->tremoloTwoChord();
                const Chord* c1 = trem->chord1();
                const Chord* c2 = trem->chord2();
                if (trem->userModified() || c1->staffMove() != c2->staffMove()) {
                    return true;
                }
            }
            if (e->isChord() && !toChord(e)->graceNotes().empty()) {
                for (const Chord* grace : toChord(e)->graceNotes()) {
                    if (grace->beam() && (grace->beam()->cross() || grace->beam()->userModified())) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void SystemLayout::updateCrossBeams(System* system, LayoutContext& ctx)
{
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(system);

    SystemLayout::layout2(system, ctx); // Computes staff distances, essential for the rest of the calculations
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
                        ChordLayout::computeUp(grace, ctx);
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
        bool somethingChanged = false;
        for (Segment& seg : toMeasure(mb)->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* e : seg.elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* chord = toChord(e);
                if (chord->beam() && (chord->beam()->cross() || chord->beam()->userModified())) {
                    bool prevUp = chord->up();
                    Stem* stem = chord->stem();
                    double prevStemLength = stem ? stem->length() : 0.0;
                    ChordLayout::computeUp(chord, ctx);
                    if (chord->up() != prevUp || (stem && stem->length() != prevStemLength)) {
                        // If the chord has changed direction needs to be re-laid out
                        ChordLayout::layoutChords1(ctx, &seg, chord->vStaffIdx());
                        somethingChanged = true;
                    }
                } else if (chord->tremoloTwoChord()) {
                    TremoloTwoChord* t = chord->tremoloTwoChord();
                    Chord* c1 = t->chord1();
                    Chord* c2 = t->chord2();
                    if (t->userModified() || (c1->staffMove() != 0 || c2->staffMove() != 0)) {
                        bool prevUp = chord->up();
                        ChordLayout::computeUp(chord, ctx);
                        if (chord->up() != prevUp) {
                            ChordLayout::layoutChords1(ctx, &seg, chord->vStaffIdx());
                            somethingChanged = true;
                        }
                    }
                }
            }
            if (somethingChanged) {
                seg.createShapes();
            }
        }
    }
}

void SystemLayout::restoreTiesAndBends(System* system, LayoutContext& ctx)
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
    doLayoutTies(system, segList, stick, etick, ctx);
    layoutGuitarBends(segList, ctx);
}

void SystemLayout::layoutSystem(System* system, LayoutContext& ctx, double xo1, const bool isFirstSystem, bool firstSystemIndent)
{
    if (system->staves().empty()) {                 // ignore vbox
        return;
    }

    // Get standard instrument name distance
    double instrumentNameOffset = ctx.conf().styleMM(Sid::instrumentNameOffset);
    // Now scale it depending on the text size (which also may not follow staff scaling)
    double textSizeScaling = 1.0;
    double actualSize = 0.0;
    double defaultSize = 0.0;
    bool followStaffSize = true;
    if (ctx.state().startWithLongNames()) {
        actualSize = ctx.conf().styleD(Sid::longInstrumentFontSize);
        defaultSize = DefaultStyle::defaultStyle().value(Sid::longInstrumentFontSize).toDouble();
        followStaffSize = ctx.conf().styleB(Sid::longInstrumentFontSpatiumDependent);
    } else {
        actualSize = ctx.conf().styleD(Sid::shortInstrumentFontSize);
        defaultSize = DefaultStyle::defaultStyle().value(Sid::shortInstrumentFontSize).toDouble();
        followStaffSize = ctx.conf().styleB(Sid::shortInstrumentFontSpatiumDependent);
    }
    textSizeScaling = actualSize / defaultSize;
    if (!followStaffSize) {
        textSizeScaling *= DefaultStyle::defaultStyle().value(Sid::spatium).toDouble() / ctx.conf().styleD(Sid::spatium);
    }
    textSizeScaling = std::max(textSizeScaling, 1.0);
    instrumentNameOffset *= textSizeScaling;

    size_t nstaves = system->staves().size();

    //---------------------------------------------------
    //  find x position of staves
    //---------------------------------------------------
    SystemLayout::layoutBrackets(system, ctx);
    double maxBracketsWidth = SystemLayout::totalBracketOffset(ctx);

    double maxNamesWidth = SystemLayout::instrumentNamesWidth(system, ctx, isFirstSystem);

    double indent = maxNamesWidth > 0 ? maxNamesWidth + instrumentNameOffset : 0.0;
    if (isFirstSystem && firstSystemIndent) {
        indent = std::max(indent, system->styleP(Sid::firstSystemIndentationValue) * system->mag() - maxBracketsWidth);
        maxNamesWidth = indent - instrumentNameOffset;
    }

    if (muse::RealIsNull(indent)) {
        if (ctx.conf().styleB(Sid::alignSystemToMargin)) {
            system->setLeftMargin(0.0);
        } else {
            system->setLeftMargin(maxBracketsWidth);
        }
    } else {
        system->setLeftMargin(indent + maxBracketsWidth);
    }

    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        SysStaff* s = system->staves().at(staffIdx);
        const Staff* staff = ctx.dom().staff(staffIdx);
        if (!staff->show() || !s->show()) {
            s->setbbox(RectF());
            continue;
        }

        double staffMag = staff->staffMag(Fraction(0, 1));         // ??? TODO
        int staffLines = staff->lines(Fraction(0, 1));
        if (staffLines <= 1) {
            double h = staff->lineDistance(Fraction(0, 1)) * staffMag * system->spatium();
            s->setbbox(system->leftMargin() + xo1, -h, 0.0, 2 * h);
        } else {
            double h = (staffLines - 1) * staff->lineDistance(Fraction(0, 1));
            h = h * staffMag * system->spatium();
            s->setbbox(system->leftMargin() + xo1, 0.0, 0.0, h);
        }
    }

    //---------------------------------------------------
    //  layout brackets
    //---------------------------------------------------

    system->setBracketsXPosition(xo1 + system->leftMargin());

    //---------------------------------------------------
    //  layout instrument names x position
    //     at this point it is not clear which staves will
    //     be hidden, so layout all instrument names
    //---------------------------------------------------

    for (const SysStaff* s : system->staves()) {
        for (InstrumentName* t : s->instrumentNames) {
            TLayout::layoutInstrumentName(t, t->mutldata());

            switch (t->align().horizontal) {
            case AlignH::LEFT:
                t->mutldata()->setPosX(0);
                break;
            case AlignH::HCENTER:
                t->mutldata()->setPosX(maxNamesWidth * .5);
                break;
            case AlignH::RIGHT:
                t->mutldata()->setPosX(maxNamesWidth);
                break;
            }
        }
    }

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        if (m == system->measures().front() || (m->prev() && m->prev()->isHBox())) {
            MeasureLayout::createSystemBeginBarLine(m, ctx);
        }
    }
}

double SystemLayout::instrumentNamesWidth(System* system, LayoutContext& ctx, bool isFirstSystem)
{
    double namesWidth = 0.0;

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        const SysStaff* staff = system->staff(staffIdx);
        if (!staff || (isFirstSystem && !staff->show())) {
            continue;
        }

        for (InstrumentName* name : staff->instrumentNames) {
            TLayout::layoutInstrumentName(name, name->mutldata());
            namesWidth = std::max(namesWidth, name->width());
        }
    }

    return namesWidth;
}

/// Calculates the total width of all brackets together that
/// would be visible when all staves are visible.
/// The logic in this method is closely related to the logic in
/// System::layoutBrackets and System::createBracket.
double SystemLayout::totalBracketOffset(LayoutContext& ctx)
{
    if (ctx.state().totalBracketsWidth() >= 0) {
        return ctx.state().totalBracketsWidth();
    }

    size_t columns = 0;
    for (const Staff* staff : ctx.dom().staves()) {
        for (const BracketItem* bi : staff->brackets()) {
            columns = std::max(columns, bi->column() + 1);
        }
    }

    size_t nstaves = ctx.dom().nstaves();
    std::vector < double > bracketWidth(nstaves, 0.0);
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* staff = ctx.dom().staff(staffIdx);
        for (auto bi : staff->brackets()) {
            if (bi->bracketType() == BracketType::NO_BRACKET || !bi->visible()) {
                continue;
            }

            //! This logic is partially copied from System::createBracket.
            //! Of course, we don't need to worry about invisible staves,
            //! but we do need to worry about brackets that span past the
            //! last staff.
            staff_idx_t firstStaff = staffIdx;
            staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
            if (lastStaff >= nstaves) {
                lastStaff = nstaves - 1;
            }

            for (; firstStaff <= lastStaff; ++firstStaff) {
                if (ctx.dom().staff(firstStaff)->show()) {
                    break;
                }
            }
            for (; lastStaff >= firstStaff; --lastStaff) {
                if (ctx.dom().staff(lastStaff)->show()) {
                    break;
                }
            }

            size_t span = lastStaff - firstStaff + 1;
            if (span > 1
                || (bi->bracketSpan() == span)
                || (span == 1 && ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden))) {
                Bracket* dummyBr = Factory::createBracket(ctx.mutDom().dummyParent(), /*isAccessibleEnabled=*/ false);
                dummyBr->setBracketItem(bi);
                dummyBr->setStaffSpan(firstStaff, lastStaff);
                dummyBr->mutldata()->bracketHeight.set_value(3.5 * dummyBr->spatium() * 2); // default
                TLayout::layoutBracket(dummyBr, dummyBr->mutldata(), ctx.conf());
                for (staff_idx_t stfIdx = firstStaff; stfIdx <= lastStaff; ++stfIdx) {
                    bracketWidth[stfIdx] += dummyBr->ldata()->bracketWidth();
                }
                delete dummyBr;
            }
        }
    }

    double totalBracketsWidth = 0.0;
    for (double w : bracketWidth) {
        totalBracketsWidth = std::max(totalBracketsWidth, w);
    }
    ctx.mutState().setTotalBracketsWidth(totalBracketsWidth);

    return totalBracketsWidth;
}

double SystemLayout::layoutBrackets(System* system, LayoutContext& ctx)
{
    size_t nstaves = system->staves().size();
    size_t columns = system->getBracketsColumnsCount();

    std::vector<double> bracketWidth(columns, 0.0);

    std::vector<Bracket*> bl;
    bl.swap(system->brackets());

    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* s = ctx.dom().staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                Bracket* b = SystemLayout::createBracket(system, ctx, bi, i, static_cast<int>(staffIdx), bl, system->firstMeasure());
                if (b != nullptr) {
                    b->mutldata()->bracketHeight.set_value(3.5 * b->spatium() * 2); // dummy
                    TLayout::layoutBracket(b, b->mutldata(), ctx.conf());
                    bracketWidth[i] = std::max(bracketWidth[i], b->ldata()->bracketWidth());
                }
            }
        }
    }

    for (Bracket* b : bl) {
        delete b;
    }

    double totalBracketWidth = 0.0;

    if (!system->brackets().empty()) {
        for (double w : bracketWidth) {
            totalBracketWidth += w;
        }
    }

    return totalBracketWidth;
}

void SystemLayout::addBrackets(System* system, Measure* measure, LayoutContext& ctx)
{
    if (system->staves().empty()) {                 // ignore vbox
        return;
    }

    size_t nstaves = system->staves().size();

    //---------------------------------------------------
    //  find x position of staves
    //    create brackets
    //---------------------------------------------------

    size_t columns = system->getBracketsColumnsCount();

    std::vector<Bracket*> bl;
    bl.swap(system->brackets());

    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* s = ctx.dom().staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                SystemLayout::createBracket(system, ctx, bi, i, staffIdx, bl, measure);
            }
        }
        if (!system->staff(staffIdx)->show()) {
            continue;
        }
    }

    //---------------------------------------------------
    //  layout brackets
    //---------------------------------------------------
    SystemLayout::layoutBracketsVertical(system, ctx);

    system->setBracketsXPosition(measure->x());

    muse::join(system->brackets(), bl);
}

//---------------------------------------------------------
//   createBracket
//---------------------------------------------------------

Bracket* SystemLayout::createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                     std::vector<Bracket*>& bl,
                                     Measure* measure)
{
    if (!measure) {
        return nullptr;
    }

    size_t nstaves = system->staves().size();
    staff_idx_t firstStaff = staffIdx;
    staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
    if (lastStaff >= nstaves) {
        lastStaff = nstaves - 1;
    }

    for (; firstStaff <= lastStaff; ++firstStaff) {
        if (system->staff(firstStaff)->show()) {
            break;
        }
    }
    for (; lastStaff >= firstStaff; --lastStaff) {
        if (system->staff(lastStaff)->show()) {
            break;
        }
    }
    size_t span = lastStaff - firstStaff + 1;
    //
    // do not show bracket if it only spans one
    // system due to some invisible staves
    //
    if (span > 1
        || (bi->bracketSpan() == span)
        || (span == 1 && ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() != BracketType::SQUARE)
        || (span == 1 && ctx.conf().styleB(Sid::alwaysShowSquareBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() == BracketType::SQUARE)) {
        //
        // this bracket is visible
        //
        Bracket* b = 0;
        track_idx_t track = staffIdx * VOICES;
        for (size_t k = 0; k < bl.size(); ++k) {
            if (bl[k]->track() == track && bl[k]->column() == column && bl[k]->bracketType() == bi->bracketType()
                && bl[k]->measure() == measure) {
                b = muse::takeAt(bl, k);
                break;
            }
        }
        if (b == 0) {
            b = Factory::createBracket(ctx.mutDom().dummyParent());
            b->setBracketItem(bi);
            b->setGenerated(true);
            b->setTrack(track);
            b->setMeasure(measure);
        }
        system->add(b);

        if (bi->selected()) {
            bool needSelect = true;

            std::vector<EngravingItem*> brackets = ctx.selection().elements(ElementType::BRACKET);
            for (const EngravingItem* element : brackets) {
                if (toBracket(element)->bracketItem() == bi) {
                    needSelect = false;
                    break;
                }
            }

            if (needSelect) {
                ctx.select(b, SelectType::ADD);
            }
        }

        b->setStaffSpan(firstStaff, lastStaff);

        return b;
    }

    return nullptr;
}

//---------------------------------------------------------
//   layout2
//    called after measure layout
//    adjusts staff distance
//---------------------------------------------------------

void SystemLayout::layout2(System* system, LayoutContext& ctx)
{
    TRACEFUNC;
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(system);

    Box* vb = system->vbox();
    if (vb) {
        TLayout::layoutBox(vb, vb->mutldata(), ctx);
        system->setbbox(vb->ldata()->bbox());
        return;
    }

    system->setPos(0.0, 0.0);
    std::list<std::pair<size_t, SysStaff*> > visibleStaves;

    for (size_t i = 0; i < system->staves().size(); ++i) {
        const Staff* s  = ctx.dom().staff(i);
        SysStaff* ss = system->staves().at(i);
        if (s->show() && ss->show()) {
            visibleStaves.push_back(std::pair<size_t, SysStaff*>(i, ss));
        } else {
            ss->setbbox(RectF());        // already done in layout() ?
        }
    }

    double _spatium            = system->spatium();
    double y                   = 0.0;
    double minVerticalDistance = ctx.conf().styleMM(Sid::minVerticalDistance);
    double staffDistance       = ctx.conf().styleMM(Sid::staffDistance);
    double akkoladeDistance    = ctx.conf().styleMM(Sid::akkoladeDistance);
    if (ctx.conf().isVerticalSpreadEnabled()) {
        staffDistance       = ctx.conf().styleMM(Sid::minStaffSpread);
        akkoladeDistance    = ctx.conf().styleMM(Sid::minStaffSpread);
    }

    if (visibleStaves.empty()) {
        return;
    }

    for (auto i = visibleStaves.begin();; ++i) {
        SysStaff* ss  = i->second;
        staff_idx_t si1 = i->first;
        const Staff* staff  = ctx.dom().staff(si1);
        auto ni = std::next(i);

        double dist = staff->staffHeight();
        double yOffset;
        double h;
        if (staff->lines(Fraction(0, 1)) == 1) {
            yOffset = _spatium * BARLINE_SPAN_1LINESTAFF_TO * 0.5;
            h = _spatium * (BARLINE_SPAN_1LINESTAFF_TO - BARLINE_SPAN_1LINESTAFF_FROM) * 0.5;
        } else {
            yOffset = 0.0;
            h = staff->staffHeight();
        }
        if (ni == visibleStaves.end()) {
            ss->setYOff(yOffset);
            ss->setbbox(system->leftMargin(), y - yOffset, system->width() - system->leftMargin(), h);
            ss->saveLayout();
            break;
        }

        staff_idx_t si2 = ni->first;
        const Staff* staff2  = ctx.dom().staff(si2);

        if (staff->part() == staff2->part()) {
            Measure* m = system->firstMeasure();
            double mag = m ? staff->staffMag(m->tick()) : 1.0;
            dist += akkoladeDistance * mag;
        } else {
            dist += staffDistance;
        }
        dist += staff2->userDist();
        bool fixedSpace = false;
        for (const MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            const Measure* m = toMeasure(mb);
            Spacer* sp = m->vspacerDown(si1);
            if (sp) {
                if (sp->spacerType() == SpacerType::FIXED) {
                    dist = staff->staffHeight() + sp->gap();
                    fixedSpace = true;
                    break;
                } else {
                    dist = std::max(dist, staff->staffHeight() + sp->gap());
                }
            }
            sp = m->vspacerUp(si2);
            if (sp) {
                dist = std::max(dist, sp->gap() + staff->staffHeight());
            }
        }
        if (!fixedSpace) {
            // check minimum distance to next staff
            // note that in continuous view, we normally only have a partial skyline for the system
            // a full one is only built when triggering a full layout
            // therefore, we don't know the value we get from minDistance will actually be enough
            // so we remember the value between layouts and increase it when necessary
            // (the first layout on switching to continuous view gives us good initial values)
            // the result is space is good to start and grows as needed
            // it does not, however, shrink when possible - only by trigger a full layout
            // (such as by toggling to page view and back)
            const double minHorizontalClearance = ctx.conf().styleMM(Sid::skylineMinHorizontalClearance);
            double d = ss->skyline().minDistance(system->System::staff(si2)->skyline(), minHorizontalClearance);
            if (ctx.conf().isLineMode()) {
                double previousDist = ss->continuousDist();
                if (d > previousDist) {
                    ss->setContinuousDist(d);
                } else {
                    d = previousDist;
                }
            }
            dist = std::max(dist, d + minVerticalDistance);
            dist = std::max(dist, minVertSpaceForCrossStaffBeams(system, si1, si2, ctx));
        }
        ss->setYOff(yOffset);
        ss->setbbox(system->leftMargin(), y - yOffset, system->width() - system->leftMargin(), h);
        ss->saveLayout();
        y += dist;
    }

    system->setSystemHeight(system->staff(visibleStaves.back().first)->bbox().bottom());
    system->setHeight(system->systemHeight());

    SystemLayout::setMeasureHeight(system, system->systemHeight(), ctx);

    //---------------------------------------------------
    //  layout brackets vertical position
    //---------------------------------------------------

    SystemLayout::layoutBracketsVertical(system, ctx);

    //---------------------------------------------------
    //  layout instrument names
    //---------------------------------------------------

    SystemLayout::layoutInstrumentNames(system, ctx);
}

double SystemLayout::minVertSpaceForCrossStaffBeams(System* system, staff_idx_t staffIdx1, staff_idx_t staffIdx2, LayoutContext& ctx)
{
    double minSpace = -DBL_MAX;
    track_idx_t startTrack = staffIdx1 * VOICES;
    track_idx_t endTrack = staffIdx2 * VOICES + VOICES;
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& segment : toMeasure(mb)->segments()) {
            if (!segment.isChordRestType()) {
                continue;
            }
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                EngravingItem* item = segment.elementAt(track);
                if (!item || !item->isChord()) {
                    continue;
                }
                Beam* beam = toChord(item)->beam();
                if (!beam || !beam->autoplace() || beam->elements().front() != item) {
                    continue;
                }
                if (beam->ldata()->crossStaffBeamPos != BeamBase::CrossStaffBeamPosition::BETWEEN) {
                    continue;
                }
                const Chord* limitingChordAbove = nullptr;
                const Chord* limitingChordBelow = nullptr;
                double limitFromAbove = -DBL_MAX;
                double limitFromBelow = DBL_MAX;
                for (const ChordRest* cr : beam->elements()) {
                    if (!cr->isChord()) {
                        continue;
                    }
                    const Chord* chord = toChord(cr);
                    double minStemLength = BeamTremoloLayout::minStemLength(chord, beam->ldata()) * (chord->spatium() / 4);
                    bool isUnderCrossBeam = cr->isBelowCrossBeam(beam);
                    if (isUnderCrossBeam && chord->vStaffIdx() == staffIdx2) {
                        const Note* topNote = chord->upNote();
                        double noteLimit = topNote->y() - minStemLength;
                        if (noteLimit < limitFromBelow) {
                            limitFromBelow = noteLimit;
                            limitingChordBelow = chord;
                        }
                        limitFromBelow = std::min(limitFromBelow, noteLimit);
                    } else if (!isUnderCrossBeam && chord->vStaffIdx() == staffIdx1) {
                        const Note* bottomNote = chord->downNote();
                        double noteLimit = bottomNote->y() + minStemLength;
                        if (noteLimit > limitFromAbove) {
                            limitFromAbove = noteLimit;
                            limitingChordAbove = chord;
                        }
                    }
                }
                if (limitingChordAbove && limitingChordBelow) {
                    double minSpaceRequired = limitFromAbove - limitFromBelow;
                    beam->mutldata()->limitingChordAbove = limitingChordAbove;
                    beam->mutldata()->limitingChordBelow = limitingChordBelow;

                    const BeamSegment* topBeamSegmentForChordAbove = beam->topLevelSegmentForElement(limitingChordAbove);
                    const BeamSegment* topBeamSegmentForChordBelow = beam->topLevelSegmentForElement(limitingChordBelow);
                    if (topBeamSegmentForChordAbove->above == topBeamSegmentForChordBelow->above) {
                        // In this case the two opposing stems overlap the beam height, so we must subtract it
                        int strokeCount = std::min(BeamTremoloLayout::strokeCount(beam->ldata(), limitingChordAbove),
                                                   BeamTremoloLayout::strokeCount(beam->ldata(), limitingChordBelow));
                        double beamHeight = ctx.conf().styleMM(Sid::beamWidth).val() + beam->beamDist() * (strokeCount - 1);
                        minSpaceRequired -= beamHeight;
                    }
                    minSpace = std::max(minSpace, minSpaceRequired);
                }
            }
        }
    }

    return minSpace;
}

void SystemLayout::restoreLayout2(System* system, LayoutContext& ctx)
{
    TRACEFUNC;
    if (system->vbox()) {
        return;
    }

    for (SysStaff* s : system->staves()) {
        s->restoreLayout();
    }

    system->setHeight(system->systemHeight());
    SystemLayout::setMeasureHeight(system, system->systemHeight(), ctx);
}

void SystemLayout::setMeasureHeight(System* system, double height, const LayoutContext& ctx)
{
    double spatium = system->spatium();
    for (MeasureBase* m : system->measures()) {
        MeasureBase::LayoutData* mldata = m->mutldata();
        if (m->isMeasure()) {
            // note that the factor 2 * _spatium must be corrected for when exporting
            // system distance in MusicXML (issue #24733)
            mldata->setBbox(0.0, -spatium, m->width(), height + 2.0 * spatium);
        } else if (m->isHBox()) {
            mldata->setBbox(0.0, 0.0, m->width(), height);
            TLayout::layoutHBox2(toHBox(m), ctx);
        } else if (m->isTBox()) {
            TLayout::layoutTBox(toTBox(m), toTBox(m)->mutldata(), ctx);
        } else {
            LOGD("unhandled measure type %s", m->typeName());
        }
    }
}

void SystemLayout::layoutBracketsVertical(System* system, LayoutContext& ctx)
{
    for (Bracket* b : system->brackets()) {
        int staffIdx1 = static_cast<int>(b->firstStaff());
        int staffIdx2 = static_cast<int>(b->lastStaff());
        double sy = 0;                           // assume bracket not visible
        double ey = 0;
        // if start staff not visible, try next staff
        while (staffIdx1 <= staffIdx2 && !system->staves().at(staffIdx1)->show()) {
            ++staffIdx1;
        }
        // if end staff not visible, try prev staff
        while (staffIdx1 <= staffIdx2 && !system->staves().at(staffIdx2)->show()) {
            --staffIdx2;
        }
        // if the score doesn't have "alwaysShowBracketsWhenEmptyStavesAreHidden" as true,
        // the bracket will be shown IF:
        // it spans at least 2 visible staves (staffIdx1 < staffIdx2) OR
        // it spans just one visible staff (staffIdx1 == staffIdx2) but it is required to do so
        // (the second case happens at least when the bracket is initially dropped)
        bool notHidden = ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)
                         ? (staffIdx1 <= staffIdx2) : (staffIdx1 < staffIdx2) || (b->span() == 1 && staffIdx1 == staffIdx2);
        if (notHidden) {                        // set vert. pos. and height to visible spanned staves
            sy = system->staves().at(staffIdx1)->bbox().top();
            ey = system->staves().at(staffIdx2)->bbox().bottom();
        }

        Bracket::LayoutData* bldata = b->mutldata();
        bldata->setPosY(sy);
        bldata->bracketHeight = ey - sy;
        TLayout::layoutBracket(b, bldata, ctx.conf());
    }
}

void SystemLayout::layoutInstrumentNames(System* system, LayoutContext& ctx)
{
    staff_idx_t staffIdx = 0;

    for (const Part* p : ctx.dom().parts()) {
        SysStaff* s = system->staff(staffIdx);
        SysStaff* s2;
        size_t nstaves = p->nstaves();

        staff_idx_t visible = system->firstVisibleSysStaffOfPart(p);
        if (visible != muse::nidx) {
            // The top staff might be invisible but this top staff contains the instrument names.
            // To make sure these instrument name are drawn, even when the top staff is invisible,
            // move the InstrumentName elements to the first visible staff of the part.
            if (visible != staffIdx) {
                SysStaff* vs = system->staff(visible);
                for (InstrumentName* t : s->instrumentNames) {
                    t->setTrack(visible * VOICES);
                    t->setSysStaff(vs);
                    vs->instrumentNames.push_back(t);
                }
                s->instrumentNames.clear();
                s = vs;
            }

            for (InstrumentName* t : s->instrumentNames) {
                //
                // override Text->layout()
                //
                double y1, y2;
                switch (t->layoutPos()) {
                default:
                case 0:                         // center at part
                    y1 = s->bbox().top();
                    s2 = system->staff(staffIdx);
                    for (int i = static_cast<int>(staffIdx + nstaves - 1); i > 0; --i) {
                        SysStaff* s3 = system->staff(i);
                        if (s3->show()) {
                            s2 = s3;
                            break;
                        }
                    }
                    y2 = s2->bbox().bottom();
                    break;
                case 1:                         // center at first staff
                    y1 = s->bbox().top();
                    y2 = s->bbox().bottom();
                    break;
                case 2:                         // center between first and second staff
                    y1 = s->bbox().top();
                    y2 = system->staff(staffIdx + 1)->bbox().bottom();
                    break;
                case 3:                         // center at second staff
                    y1 = system->staff(staffIdx + 1)->bbox().top();
                    y2 = system->staff(staffIdx + 1)->bbox().bottom();
                    break;
                case 4:                         // center between first and second staff
                    y1 = system->staff(staffIdx + 1)->bbox().top();
                    y2 = system->staff(staffIdx + 2)->bbox().bottom();
                    break;
                case 5:                         // center at third staff
                    y1 = system->staff(staffIdx + 2)->bbox().top();
                    y2 = system->staff(staffIdx + 2)->bbox().bottom();
                    break;
                }
                t->mutldata()->setPosY(y1 + (y2 - y1) * .5 + t->offset().y());
            }
        }
        staffIdx += nstaves;
    }
}

void SystemLayout::setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick)
{
    //
    // remark: add/remove instrument names is not undo/redoable
    //         as add/remove of systems is not undoable
    //
    if (system->vbox()) {                 // ignore vbox
        return;
    }
    if (!ctx.conf().isShowInstrumentNames()
        || (ctx.conf().styleB(Sid::hideInstrumentNameIfOneInstrument) && ctx.dom().visiblePartCount() <= 1)
        || (ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::firstSystemInstNameVisibility).value<InstrumentLabelVisibility>() == InstrumentLabelVisibility::HIDE)
        || (!ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::subsSystemInstNameVisibility).value<InstrumentLabelVisibility>()
            == InstrumentLabelVisibility::HIDE)) {
        for (SysStaff* staff : system->staves()) {
            for (InstrumentName* t : staff->instrumentNames) {
                ctx.mutDom().removeElement(t);
            }
        }
        return;
    }

    int staffIdx = 0;
    for (SysStaff* staff : system->staves()) {
        const Staff* s = ctx.dom().staff(staffIdx);
        Part* part = s->part();

        bool atLeastOneVisibleStaff = false;
        for (Staff* partStaff : part->staves()) {
            if (partStaff->show()) {
                atLeastOneVisibleStaff = true;
                break;
            }
        }

        bool showName = part->show() && atLeastOneVisibleStaff;
        if (!s->isTop() || !showName) {
            for (InstrumentName* t : staff->instrumentNames) {
                ctx.mutDom().removeElement(t);
            }
            ++staffIdx;
            continue;
        }

        const std::list<StaffName>& names = longName ? part->longNames(tick) : part->shortNames(tick);

        size_t idx = 0;
        for (const StaffName& sn : names) {
            InstrumentName* iname = muse::value(staff->instrumentNames, idx);
            if (iname == 0) {
                iname = new InstrumentName(system);
                iname->setGenerated(true);
                iname->setParent(system);
                iname->setSysStaff(staff);
                iname->setTrack(staffIdx * VOICES);
                iname->setInstrumentNameType(longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT);
                iname->setLayoutPos(sn.pos());
                ctx.mutDom().addElement(iname);
            }
            iname->setXmlText(sn.name());
            ++idx;
        }
        for (; idx < staff->instrumentNames.size(); ++idx) {
            ctx.mutDom().removeElement(staff->instrumentNames[idx]);
        }
        ++staffIdx;
    }
}

//---------------------------------------------------------
//   minDistance
//    Return the minimum distance between this system and s2
//    without any element collisions.
//
//    top - top system
//    bottom   - bottom system
//---------------------------------------------------------

double SystemLayout::minDistance(const System* top, const System* bottom, const LayoutContext& ctx)
{
    TRACEFUNC;

    const LayoutConfiguration& conf = ctx.conf();
    const DomAccessor& dom = ctx.dom();

    const Box* topVBox = top->vbox();
    const Box* bottomVBox = bottom->vbox();

    if (topVBox && !bottomVBox) {
        return std::max(topVBox->absoluteFromSpatium(topVBox->bottomGap()), bottom->minTop());
    } else if (!topVBox && bottomVBox) {
        return std::max(bottomVBox->absoluteFromSpatium(bottomVBox->topGap()), top->minBottom());
    } else if (topVBox && bottomVBox) {
        return bottomVBox->absoluteFromSpatium(bottomVBox->topGap()) + topVBox->absoluteFromSpatium(topVBox->bottomGap());
    }

    if (top->staves().empty() || bottom->staves().empty()) {
        return 0.0;
    }

    double minVerticalDistance = conf.styleMM(Sid::minVerticalDistance);
    double dist = conf.isVerticalSpreadEnabled() ? conf.styleMM(Sid::minSystemSpread) : conf.styleMM(Sid::minSystemDistance);
    size_t firstStaff = 0;
    size_t lastStaff = 0;

    for (firstStaff = 0; firstStaff < top->staves().size() - 1; ++firstStaff) {
        if (dom.staff(firstStaff)->show() && bottom->staff(firstStaff)->show()) {
            break;
        }
    }
    for (lastStaff = top->staves().size() - 1; lastStaff > 0; --lastStaff) {
        if (dom.staff(lastStaff)->show() && top->staff(lastStaff)->show()) {
            break;
        }
    }

    const Staff* staff = dom.staff(firstStaff);
    double userDist = staff ? staff->userDist() : 0.0;
    dist = std::max(dist, userDist);
    top->setFixedDownDistance(false);

    const SysStaff* sysStaff = top->staff(lastStaff);
    const double minHorizontalClearance = conf.styleMM(Sid::skylineMinHorizontalClearance);
    double sld = sysStaff ? sysStaff->skyline().minDistance(bottom->staff(firstStaff)->skyline(), minHorizontalClearance) : 0;
    sld -= sysStaff ? sysStaff->bbox().height() - minVerticalDistance : 0;

    if (conf.isFloatMode()) {
        return std::max(dist, sld);
    }

    for (const MeasureBase* mb1 : top->measures()) {
        if (mb1->isMeasure()) {
            const Measure* m = toMeasure(mb1);
            const Spacer* sp = m->vspacerDown(lastStaff);
            if (sp) {
                if (sp->spacerType() == SpacerType::FIXED) {
                    dist = sp->gap();
                    top->setFixedDownDistance(true);
                    break;
                } else {
                    dist = std::max(dist, sp->gap().val());
                }
            }
        }
    }
    if (!top->hasFixedDownDistance()) {
        for (const MeasureBase* mb2 : bottom->measures()) {
            if (mb2->isMeasure()) {
                const Measure* m = toMeasure(mb2);
                const Spacer* sp = m->vspacerUp(firstStaff);
                if (sp) {
                    dist = std::max(dist, sp->gap().val());
                }
            }
        }

        dist = std::max(dist, sld);
    }
    return dist;
}

void SystemLayout::updateSkylineForElement(EngravingItem* element, const System* system, double yMove)
{
    Skyline& skyline = system->staff(element->staffIdx())->skyline();
    SkylineLine& skylineLine = element->placeAbove() ? skyline.north() : skyline.south();
    for (ShapeElement& shapeEl : skylineLine.elements()) {
        const EngravingItem* itemInSkyline = shapeEl.item();
        if (itemInSkyline && itemInSkyline->isText() && itemInSkyline->explicitParent() && itemInSkyline->parent()->isSLineSegment()) {
            itemInSkyline = itemInSkyline->parentItem();
        }
        if (itemInSkyline == element) {
            shapeEl.translate(0.0, yMove);
        }
    }
}

void SystemLayout::centerElementsBetweenStaves(const System* system)
{
    std::vector<EngravingItem*> centeredItems;

    for (SpannerSegment* spannerSeg : system->spannerSegments()) {
        if (spannerSeg->isHairpinSegment() && elementShouldBeCenteredBetweenStaves(spannerSeg, system)) {
            centerElementBetweenStaves(spannerSeg, system);
            centeredItems.push_back(spannerSeg);
        }
    }

    for (const MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (const Segment& seg : toMeasure(mb)->segments()) {
            for (EngravingItem* item : seg.elist()) {
                if (item && item->isMMRest() && mmRestShouldBeCenteredBetweenStaves(toMMRest(item), system)) {
                    centerMMRestBetweenStaves(toMMRest(item), system);
                }
            }
            for (EngravingItem* item : seg.annotations()) {
                if ((item->isDynamic() || item->isExpression()) && elementShouldBeCenteredBetweenStaves(item, system)) {
                    centerElementBetweenStaves(item, system);
                    centeredItems.push_back(item);
                }
            }
        }
    }

    AlignmentLayout::alignStaffCenteredItems(centeredItems, system);
}

void SystemLayout::centerBigTimeSigsAcrossStaves(const System* system)
{
    staff_idx_t nstaves = system->score()->nstaves();
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& segment : toMeasure(mb)->segments()) {
            if (!segment.isType(SegmentType::TimeSigType)) {
                continue;
            }
            for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                TimeSig* timeSig = toTimeSig(segment.element(staff2track(staffIdx)));
                if (!timeSig || !timeSig->showOnThisStaff()) {
                    continue;
                }
                staff_idx_t thisStaffIdx = timeSig->effectiveStaffIdx();
                staff_idx_t nextStaffIdx = thisStaffIdx;
                for (staff_idx_t idx = thisStaffIdx + 1; idx < nstaves; ++idx) {
                    TimeSig* nextTimeSig = toTimeSig(segment.element(staff2track(idx)));
                    if (nextTimeSig && nextTimeSig->showOnThisStaff()) {
                        staff_idx_t nextTimeSigStave = nextTimeSig->effectiveStaffIdx();
                        nextStaffIdx = system->prevVisibleStaff(nextTimeSigStave);
                        break;
                    }
                    if (idx == nstaves - 1) {
                        nextStaffIdx = system->prevVisibleStaff(nstaves);
                        break;
                    }
                }
                double yTop = system->staff(thisStaffIdx)->y() + system->score()->staff(thisStaffIdx)->staffHeight(segment.tick());
                double yBottom = system->staff(nextStaffIdx)->y() + system->score()->staff(nextStaffIdx)->staffHeight(segment.tick());
                double newYPos = 0.5 * (yBottom - yTop);
                double yPosDiff = newYPos - timeSig->pos().y();
                timeSig->mutldata()->setPosY(newYPos);

                for (EngravingItem* el : segment.findAnnotations(ElementType::PARENTHESIS, timeSig->track(), timeSig->track())) {
                    if (!el || !el->isParenthesis()) {
                        continue;
                    }
                    el->mutldata()->moveY(yPosDiff);
                }
            }
        }
    }
}

bool SystemLayout::elementShouldBeCenteredBetweenStaves(const EngravingItem* item, const System* system)
{
    if (item->offset().y() != item->propertyDefault(Pid::OFFSET).value<PointF>().y()) {
        // NOTE: because of current limitations of the offset system, we can't center an element that's been manually moved.
        return false;
    }

    const Part* itemPart = item->part();
    bool centerStyle = item->style().styleB(Sid::dynamicsHairpinsAutoCenterOnGrandStaff);
    AutoOnOff centerProperty = item->getProperty(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>();
    if (itemPart->nstaves() <= 1 || centerProperty == AutoOnOff::OFF || (!centerStyle && centerProperty != AutoOnOff::ON)) {
        return false;
    }

    if (centerProperty != AutoOnOff::ON && !itemPart->instrument()->isNormallyMultiStaveInstrument()) {
        return false;
    }

    const Staff* thisStaff = item->staff();
    const std::vector<Staff*>& partStaves = itemPart->staves();
    IF_ASSERT_FAILED(partStaves.size() > 0) {
        return false;
    }

    if ((thisStaff == partStaves.front() && item->placeAbove()) || (thisStaff == partStaves.back() && item->placeBelow())) {
        return false;
    }

    staff_idx_t thisIdx = thisStaff->idx();
    if (item->placeAbove()) {
        IF_ASSERT_FAILED(thisIdx > 0) {
            return false;
        }
    }

    staff_idx_t nextIdx = item->placeAbove() ? system->prevVisibleStaff(thisIdx) : system->nextVisibleStaff(thisIdx);
    if (nextIdx == muse::nidx || !muse::contains(partStaves, item->score()->staff(nextIdx))) {
        return false;
    }

    return centerProperty == AutoOnOff::ON || item->appliesToAllVoicesInInstrument();
}

bool SystemLayout::mmRestShouldBeCenteredBetweenStaves(const MMRest* mmRest, const System* system)
{
    if (!mmRest->style().styleB(Sid::mmRestBetweenStaves)) {
        return false;
    }

    const Part* itemPart = mmRest->part();
    if (itemPart->nstaves() <= 1) {
        return false;
    }

    staff_idx_t thisStaffIdx = mmRest->staffIdx();
    staff_idx_t prevStaffIdx = system->prevVisibleStaff(thisStaffIdx);

    return prevStaffIdx != muse::nidx && mmRest->score()->staff(prevStaffIdx)->part() == itemPart;
}

bool SystemLayout::elementHasAnotherStackedOutside(const EngravingItem* element, const Shape& elementShape, const SkylineLine& skylineLine)
{
    double elemShapeLeft = -elementShape.left();
    double elemShapeRight = elementShape.right();
    double elemShapeTop = elementShape.top();
    double elemShapeBottom = elementShape.bottom();

    for (const ShapeElement& skylineElement : skylineLine.elements()) {
        const EngravingItem* skylineItem = skylineElement.item();
        if (!skylineItem || skylineItem == element || skylineItem->parent() == element
            || Autoplace::itemsShouldIgnoreEachOther(element, skylineItem)) {
            continue;
        }
        bool intersectHorizontally = elemShapeRight > skylineElement.left() && elemShapeLeft < skylineElement.right();
        if (!intersectHorizontally) {
            continue;
        }
        bool skylineElementIsStackedOnIt = skylineLine.isNorth() ? skylineElement.top() < elemShapeTop
                                           : skylineElement.bottom() > elemShapeBottom;
        if (skylineElementIsStackedOnIt) {
            return true;
        }
    }

    return false;
}

void SystemLayout::centerElementBetweenStaves(EngravingItem* element, const System* system)
{
    bool isAbove = element->placeAbove();
    staff_idx_t thisIdx = element->staffIdx();
    if (isAbove) {
        IF_ASSERT_FAILED(thisIdx > 0) {
            return;
        }
    }
    staff_idx_t nextIdx = isAbove ? system->prevVisibleStaff(thisIdx) : system->nextVisibleStaff(thisIdx);
    IF_ASSERT_FAILED(nextIdx != muse::nidx) {
        return;
    }

    SysStaff* thisStaff = system->staff(thisIdx);
    SysStaff* nextStaff = system->staff(nextIdx);

    IF_ASSERT_FAILED(thisStaff && nextStaff) {
        return;
    }

    double elementXinSystemCoord = element->pageX() - system->pageX();
    const double minHorizontalClearance = system->style().styleMM(Sid::skylineMinHorizontalClearance);

    Shape elementShape = element->ldata()->shape()
                         .translated(PointF(elementXinSystemCoord, element->y()))
                         .adjust(-minHorizontalClearance, 0.0, minHorizontalClearance, 0.0);
    elementShape.remove_if([](ShapeElement& shEl) { return shEl.ignoreForLayout(); });

    const SkylineLine& skylineOfThisStaff = isAbove ? thisStaff->skyline().north() : thisStaff->skyline().south();

    if (elementHasAnotherStackedOutside(element, elementShape, skylineOfThisStaff)) {
        return;
    }

    SkylineLine thisSkyline = skylineOfThisStaff.getFilteredCopy([element](const ShapeElement& shEl) {
        const EngravingItem* shapeItem = shEl.item();
        if (!shapeItem) {
            return false;
        }
        return shapeItem->isAccidental() || Autoplace::itemsShouldIgnoreEachOther(element, shapeItem);
    });

    double yStaffDiff = nextStaff->y() - thisStaff->y();
    SkylineLine nextSkyline = isAbove ? nextStaff->skyline().south() : nextStaff->skyline().north();
    nextSkyline.translateY(yStaffDiff);

    double elementMinDist = element->minDistance().toMM(element->spatium());
    double availSpaceAbove = (isAbove ? nextSkyline.verticalClaranceBelow(elementShape) : thisSkyline.verticalClaranceBelow(elementShape))
                             - elementMinDist;
    double availSpaceBelow = (isAbove ? thisSkyline.verticalClearanceAbove(elementShape) : nextSkyline.verticalClearanceAbove(elementShape))
                             - elementMinDist;

    double yMove = 0.5 * (availSpaceBelow - availSpaceAbove);

    element->mutldata()->moveY(yMove);

    availSpaceAbove += yMove;
    availSpaceBelow -= yMove;
    element->mutldata()->setStaffCenteringInfo(std::max(availSpaceAbove, 0.0), std::max(availSpaceBelow, 0.0));

    updateSkylineForElement(element, system, yMove);
}

void SystemLayout::centerMMRestBetweenStaves(MMRest* mmRest, const System* system)
{
    staff_idx_t thisIdx = mmRest->staffIdx();
    IF_ASSERT_FAILED(thisIdx > 0) {
        return;
    }

    staff_idx_t prevIdx = system->prevVisibleStaff(thisIdx);
    IF_ASSERT_FAILED(prevIdx != muse::nidx) {
        return;
    }

    SysStaff* thisStaff = system->staff(thisIdx);
    SysStaff* prevStaff = system->staff(prevIdx);
    double prevStaffHeight = system->score()->staff(prevIdx)->staffHeight(mmRest->tick());
    double yStaffDiff = prevStaff->y() + prevStaffHeight - thisStaff->y();

    PointF mmRestDefaultNumberPosition = mmRest->numberPos() - PointF(0.0, mmRest->spatium() * mmRest->numberOffset());
    RectF numberBbox = mmRest->numberRect().translated(mmRestDefaultNumberPosition + mmRest->pos());
    double yBaseLine = 0.5 * (yStaffDiff - numberBbox.height());
    double yDiff = yBaseLine - numberBbox.top();

    mmRest->mutldata()->yNumberPos += yDiff;
}
