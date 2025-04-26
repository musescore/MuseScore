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
#include "scorehorizontalviewlayout.h"

#include "containers.h"

#include "dom/durationelement.h"
#include "dom/factory.h"
#include "dom/score.h"
#include "dom/masterscore.h"
#include "dom/system.h"
#include "dom/spanner.h"
#include "dom/page.h"
#include "dom/durationelement.h"
#include "dom/staff.h"
#include "dom/chordrest.h"
#include "dom/chord.h"
#include "dom/tuplet.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/part.h"
#include "dom/box.h"
#include "dom/slur.h"

#include "passresetlayoutdata.h"

#include "tlayout.h"
#include "systemlayout.h"
#include "beamlayout.h"
#include "tupletlayout.h"
#include "chordlayout.h"
#include "arpeggiolayout.h"
#include "measurelayout.h"
#include "horizontalspacing.h"
#include "tremololayout.h"
#include "slurtielayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void ScoreHorizontalViewLayout::layoutHorizontalView(Score* score, LayoutContext& ctx, const Fraction& stick, const Fraction& etick)
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

    ctx.mutState().setPrevMeasure(nullptr);
    ctx.mutState().setNextMeasure(m);             //_showVBox ? first() : firstMeasure();
    ctx.mutState().setStartTick(m->tick());

    PassResetLayoutData resetPass;
    resetPass.run(score, ctx);

    layoutLinear(ctx, ctx.state().isLayoutAll());
}

void ScoreHorizontalViewLayout::layoutLinear(LayoutContext& ctx, bool layoutAll)
{
    resetSystems(ctx, layoutAll);

    collectLinearSystem(ctx);

    layoutLinear(ctx);
}

//  in linear mode there is only one page
//  which contains one system
void ScoreHorizontalViewLayout::resetSystems(LayoutContext& ctx, bool layoutAll)
{
    DomAccessor& mutDom = ctx.mutDom();
    Page* page = 0;
    if (layoutAll) {
        for (System* s : mutDom.systems()) {
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->resetExplicitParent();
            }
        }
        muse::DeleteAll(mutDom.systems());
        mutDom.systems().clear();
        muse::DeleteAll(mutDom.pages());
        mutDom.pages().clear();
        if (!ctx.dom().firstMeasure()) {
            LOGD("no measures");
            return;
        }

        for (MeasureBase* mb = ctx.mutDom().first(); mb; mb = mb->next()) {
            mb->resetExplicitParent();
        }

        page = Factory::createPage(ctx.mutDom().rootItem());
        ctx.mutDom().pages().push_back(page);
        page->mutldata()->setBbox(0.0, 0.0, ctx.conf().loWidth(), ctx.conf().loHeight());
        page->setNo(0);

        System* system = Factory::createSystem(page);
        ctx.mutDom().systems().push_back(system);
        page->appendSystem(system);
        system->adjustStavesNumber(ctx.dom().nstaves());
    } else {
        if (ctx.dom().pages().empty()) {
            return;
        }
        page = ctx.mutDom().pages().front();
        System* system = ctx.mutDom().systems().front();
        system->clear();
        system->adjustStavesNumber(ctx.dom().nstaves());
    }
    ctx.mutState().setPage(page);
}

void ScoreHorizontalViewLayout::layoutLinear(LayoutContext& ctx)
{
    System* system = ctx.mutDom().systems().front();

    SystemLayout::layoutSystemElements(system, ctx);

    SystemLayout::layout2(system, ctx);     // compute staff distances

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);

        for (size_t track = 0; track < ctx.dom().ntracks(); ++track) {
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                EngravingItem* e = segment->element(track);
                if (!e) {
                    continue;
                }
                if (e->isChordRest()) {
                    if (m->tick() < ctx.state().startTick() || m->tick() > ctx.state().endTick()) {
                        continue;
                    }
                    if (!ctx.dom().staff(track2staff(track))->show()) {
                        continue;
                    }
                    ChordRest* cr = toChordRest(e);
                    if (BeamLayout::isStartOfCrossBeam(cr)) {                           // layout cross staff beams
                        TLayout::layoutBeam(cr->beam(), ctx);
                    }
                    if (TupletLayout::notTopTuplet(cr)) {
                        // fix layout of tuplets
                        DurationElement* de = cr;
                        while (de->tuplet() && de->tuplet()->elements().front() == de) {
                            Tuplet* t = de->tuplet();
                            TLayout::layoutTuplet(t, ctx);
                            de = de->tuplet();
                        }
                    }

                    if (cr->isChord()) {
                        Chord* c = toChord(cr);
                        for (Chord* cc : c->graceNotes()) {
                            if (cc->beam() && cc->beam()->elements().front() == cc) {
                                TLayout::layoutBeam(cc->beam(), ctx);
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
                        if (c->tremoloSingleChord()) {
                            TremoloLayout::layout(c->tremoloSingleChord(), ctx);
                        } else if (c->tremoloTwoChord()) {
                            TremoloTwoChord* t = c->tremoloTwoChord();
                            Chord* c1 = t->chord1();
                            Chord* c2 = t->chord2();
                            if (c1 && c2 && (c1->staffMove() || c2->staffMove())) {
                                TremoloLayout::layout(t, ctx);
                            }
                        }
                    }
                } else if (e->isBarLine()) {
                    TLayout::layoutBarLine2(toBarLine(e), ctx);
                }
            }
        }
        MeasureLayout::layout2(m, ctx);
    }

    auto spanners = ctx.dom().spannerMap().findOverlapping(system->tick().ticks(), system->endTick().ticks());
    for (auto interval : spanners) {
        Spanner* sp = interval.value;
        if (sp->isSlur() && toSlur(sp)->isCrossStaff()) {
            SlurTieLayout::layoutSystem(toSlur(sp), system, ctx);
        }
    }

    const double lm = ctx.state().page()->lm();
    const double tm = ctx.state().page()->tm() + ctx.conf().styleMM(Sid::staffUpperBorder);
    const double rm = ctx.state().page()->rm();
    const double bm = ctx.state().page()->bm() + ctx.conf().styleMM(Sid::staffLowerBorder);

    layoutSystemLockIndicators(system);

    ctx.mutState().page()->setPos(0, 0);
    system->setPos(lm, tm);
    ctx.mutState().page()->setWidth(lm + system->width() + rm);
    ctx.mutState().page()->setHeight(tm + system->height() + bm);
    ctx.mutState().page()->invalidateBspTree();
}

void ScoreHorizontalViewLayout::layoutSystemLockIndicators(System* system)
{
    system->deleteLockIndicators();

    std::vector<const SystemLock*> systemLocks = system->score()->systemLocks()->allLocks();
    for (const SystemLock* lock : systemLocks) {
        SystemLockIndicator* lockIndicator = Factory::createSystemLockIndicator(system, lock);
        lockIndicator->setParent(system);
        system->addLockIndicator(lockIndicator);
        TLayout::layoutSystemLockIndicator(lockIndicator, lockIndicator->mutldata());
    }
}

// Append all measures to System. VBox is not included to System
void ScoreHorizontalViewLayout::collectLinearSystem(LayoutContext& ctx)
{
    std::vector<int> visibleParts;
    for (size_t partIdx = 0; partIdx < ctx.dom().parts().size(); partIdx++) {
        if (ctx.dom().parts().at(partIdx)->show()) {
            visibleParts.push_back(static_cast<int>(partIdx));
        }
    }

    System* system = ctx.mutDom().systems().front();
    SystemLayout::setInstrumentNames(system, ctx, /* longNames */ true);

    double targetSystemWidth = ctx.dom().nmeasures() * ctx.conf().styleMM(Sid::minMeasureWidth).val();
    system->setWidth(targetSystemWidth);

    double curSystemWidth = 0.0;
    bool firstMeasureInScore = true;       //lc.startTick.isZero();
    bool firstMeasureInLayout = true;

    //set first measure to lc.nextMeasures for following
    //utilizing in getNextMeasure()
    ctx.mutState().setNextMeasure(ctx.mutDom().first());
    ctx.mutState().setTick(Fraction(0, 1));
    MeasureLayout::getNextMeasure(ctx);

    std::set<Measure*> measuresToLayout;

    while (ctx.state().curMeasure()) {
        if (ctx.state().curMeasure()->isVBox() || ctx.state().curMeasure()->isTBox()) {
            ctx.mutState().curMeasure()->resetExplicitParent();
            MeasureLayout::getNextMeasure(ctx);
            continue;
        }
        system->appendMeasure(ctx.mutState().curMeasure());
        bool createHeader = ctx.state().prevMeasure() && ctx.state().prevMeasure()->isHBox()
                            && toHBox(ctx.state().prevMeasure())->createSystemHeader();
        if (ctx.state().curMeasure()->isMeasure()) {
            Measure* m = toMeasure(ctx.mutState().curMeasure());
            if (m->mmRest()) {
                m->mmRest()->resetExplicitParent();
            }
            if (firstMeasureInScore) {
                SystemLayout::layoutSystem(system, ctx, curSystemWidth, true);
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                MeasureLayout::addSystemHeader(m, true, ctx);
                curSystemWidth += system->leftMargin();
                firstMeasureInScore = false;
            } else if (createHeader) {
                MeasureLayout::addSystemHeader(m, false, ctx);
            } else if (m->header()) {
                MeasureLayout::removeSystemHeader(m);
            }
            if (m->trailer()) {
                MeasureLayout::removeSystemTrailer(m);
            }

            MeasureLayout::updateGraceNotes(m, ctx);

            if (m->tick() >= ctx.state().startTick() && m->tick() <= ctx.state().endTick()) {
                // for measures in range, do full layout
                if (ctx.conf().isMode(LayoutMode::HORIZONTAL_FIXED)) {
                    MeasureLayout::createEndBarLines(m, true, ctx);
                    layoutSegmentsWithDuration(m, visibleParts);
                    double measureWidth = m->width();
                    MeasureLayout::stretchMeasureInPracticeMode(m, measureWidth, ctx);
                    m->setPos(curSystemWidth, m->y());
                    curSystemWidth += measureWidth;
                } else {
                    MeasureLayout::computePreSpacingItems(m, ctx);
                    MeasureLayout::createEndBarLines(m, false, ctx);
                    MeasureLayout::setRepeatCourtesiesAndParens(m, ctx);
                    curSystemWidth = HorizontalSpacing::updateSpacingForLastAddedMeasure(system, firstMeasureInLayout);
                    measuresToLayout.insert(m);
                    if (firstMeasureInLayout) {
                        firstMeasureInLayout = false;
                    }
                }
            } else {
                // for measures not in range, use existing layout
                double measureWidth = m->width();
                if (!muse::RealIsEqual(m->x(), curSystemWidth)) {
                    // fix beam positions
                    // other elements with system as parent are processed in layoutSystemElements()
                    // but full beam processing is expensive and not needed if we adjust position here
                    PointF p = PointF(curSystemWidth, 0.0) - m->pos();
                    for (const Segment& s : m->segments()) {
                        if (!s.isChordRestType()) {
                            continue;
                        }
                        for (size_t track = 0; track < ctx.dom().ntracks(); ++track) {
                            EngravingItem* e = s.element(static_cast<track_idx_t>(track));
                            if (!e) {
                                continue;
                            }
                            ChordRest* cr = toChordRest(e);
                            if (cr->beam() && cr->beam()->elements().front() == cr) {
                                cr->beam()->mutldata()->move(p);
                            }
                        }
                    }
                }
                m->setPos(curSystemWidth, m->y());
                curSystemWidth += measureWidth;
            }
        } else if (ctx.state().curMeasure()->isHBox()) {
            curSystemWidth = HorizontalSpacing::updateSpacingForLastAddedMeasure(system);
        }

        MeasureLayout::getNextMeasure(ctx);
    }

    for (Measure* m : measuresToLayout) {
        MeasureLayout::layoutMeasureElements(m, ctx);
    }

    for (MeasureBase* m : system->measures()) {
        if (!m->isMeasure()) {
            continue;
        }
        MeasureLayout::layoutStaffLines(toMeasure(m), ctx);
    }

    SystemLayout::hideEmptyStaves(system, ctx, true);

    system->setWidth(curSystemWidth);
}

static Segment* findFirstEnabledSegment(Measure* measure)
{
    Segment* current = measure->first();
    while (current && !current->enabled()) {
        current = current->next();
    }

    return current;
}

void ScoreHorizontalViewLayout::layoutSegmentsWithDuration(Measure* m, const std::vector<int>& visibleParts)
{
    double currentXPos = 0;

    Segment* current = findFirstEnabledSegment(m);

    auto [spacing, width] = computeCellWidth(current, visibleParts);
    currentXPos = m->style().styleMM(Sid::barNoteDistance);
    current->mutldata()->setPosX(currentXPos);
    current->setWidth(width);
    current->setSpacing(spacing);
    currentXPos += width;

    current = current->next();
    while (current) {
//        if (!current->enabled() || !current->visible()) {
//            current = current->next();
//            continue;
//        }

        auto [spacing2, width2] = computeCellWidth(current, visibleParts);
        current->setWidth(width2 + spacing2);
        current->setSpacing(spacing2);
        currentXPos += spacing2;
        current->mutldata()->setPosX(currentXPos);
        currentXPos += width2;
        current = current->next();
    }

    m->setWidth(currentXPos);
}

std::pair<double, double> ScoreHorizontalViewLayout::computeCellWidth(const Segment* s, const std::vector<int>& visibleParts)
{
    if (!s->enabled()) {
        return { 0, 0 };
    }

    Fraction quantum = calculateQuantumCell(s->measure(), visibleParts);

    auto calculateWidth = [quantum, sc = s->score()->masterScore()](ChordRest* cr) {
        return sc->widthOfSegmentCell()
               * sc->style().spatium()
               * cr->globalTicks().numerator() / cr->globalTicks().denominator()
               * quantum.denominator() / quantum.numerator();
    };

    if (s->isChordRestType()) {
        float width = 0.0;
        float spacing = 0.0;

        ChordRest* cr = nullptr;

        cr = chordRestWithMinDuration(s, visibleParts);

        if (cr) {
            width = calculateWidth(cr);

            if (cr->type() == ElementType::REST) {
                //spacing = 0; //!not necessary. It is to more clearly understanding code
            } else if (cr->type() == ElementType::CHORD) {
                Chord* ch = toChord(cr);

                //! check that gracenote exist. If exist add additional spacing
                //! to avoid colliding between grace note and previous chord
                if (!ch->graceNotes().empty()) {
                    Segment* prevSeg = s->prev();
                    if (prevSeg && prevSeg->segmentType() == SegmentType::ChordRest) {
                        ChordRest* prevCR = chordRestWithMinDuration(prevSeg, visibleParts);

                        if (prevCR && prevCR->globalTicks() < quantum) {
                            spacing = calculateWidth(prevCR);
                            return { spacing, width };
                        }
                    }
                }

                //! check that accidental exist in the chord. If exist add additional spacing
                //! to avoid colliding between grace note and previous chord
                for (auto note : ch->notes()) {
                    if (note->accidental()) {
                        Segment* prevSeg = s->prev();
                        if (prevSeg && prevSeg->segmentType() == SegmentType::ChordRest) {
                            ChordRest* prevCR = chordRestWithMinDuration(prevSeg, visibleParts);

                            if (prevCR && prevCR->globalTicks() < quantum) {
                                spacing = calculateWidth(prevCR);
                                return { spacing, width };
                            }
                        }
                    }
                }
            }
        }

        return { spacing, width };
    }

    Segment* nextSeg = s->nextActive();
    if (!nextSeg) {
        nextSeg = s->next(SegmentType::BarLineType);
    }

    if (nextSeg) {
        return { 0, HorizontalSpacing::minHorizontalDistance(s, nextSeg, 1.0) };
    }

    return { 0, s->minRight() };
}

ChordRest* ScoreHorizontalViewLayout::chordRestWithMinDuration(const Segment* seg, const std::vector<int>& visibleParts)
{
    ChordRest* chordRestWithMinDuration = nullptr;
    int minTicks = std::numeric_limits<int>::max();
    for (int partIdx : visibleParts) {
        for (const Staff* staff : seg->score()->parts().at(partIdx)->staves()) {
            staff_idx_t staffIdx = staff->idx();
            for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                if (auto element = seg->elist().at(staffIdx * VOICES + voice)) {
                    if (!element->isChordRest()) {
                        continue;
                    }

                    ChordRest* cr = toChordRest(element);
                    int chordTicks = cr->ticks().ticks();
                    if (chordTicks > minTicks) {
                        continue;
                    }
                    minTicks = chordTicks;
                    chordRestWithMinDuration = cr;
                }
            }
        }
    }

    return chordRestWithMinDuration;
}

Fraction ScoreHorizontalViewLayout::calculateQuantumCell(const Measure* m, const std::vector<int>& visibleParts)
{
    Fraction quantum = { 1, 16 };
    for (const Segment& s : m->segments()) {
        ChordRest* cr = chordRestWithMinDuration(&s, visibleParts);

        if (cr && cr->actualTicks() < quantum) {
            quantum = cr->actualTicks();
        }
    }

    return quantum;
}
