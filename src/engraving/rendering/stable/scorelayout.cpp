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
#include "scorelayout.h"

#include "containers.h"

#include "dom/score.h"
#include "dom/masterscore.h"
#include "dom/system.h"
#include "dom/layoutbreak.h"
#include "dom/bracket.h"
#include "dom/page.h"
#include "dom/factory.h"
#include "dom/spanner.h"
#include "dom/part.h"
#include "dom/chordrest.h"
#include "dom/box.h"
#include "dom/beam.h"
#include "dom/staff.h"
#include "dom/tuplet.h"
#include "dom/chord.h"
#include "dom/tremolo.h"
#include "dom/note.h"

#include "tlayout.h"
#include "pagelayout.h"
#include "measurelayout.h"
#include "systemlayout.h"
#include "beamlayout.h"
#include "tupletlayout.h"
#include "arpeggiolayout.h"
#include "chordlayout.h"

#include "../dev/horizontalspacing.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::stable;
using namespace mu::engraving::rendering::dev;

class CmdStateLocker
{
    Score* m_score = nullptr;
public:
    CmdStateLocker(Score* s)
        : m_score(s) { m_score->cmdState().lock(); }
    ~CmdStateLocker() { m_score->cmdState().unlock(); }
};

static void resetLayoutData(EngravingItem* item)
{
    if (item->ldata()) {
        item->mutldata()->reset();
    }
    for (EngravingItem* ch : item->childrenItems()) {
        resetLayoutData(ch);
    }
}

void ScoreLayout::layoutRange(Score* score, const Fraction& st, const Fraction& et)
{
    CmdStateLocker cmdStateLocker(score);
    LayoutContext ctx(score);

    Fraction stick(st);
    Fraction etick(et);
    assert(!(stick == Fraction(-1, 1) && etick == Fraction(-1, 1)));

    if (!score->last() || (ctx.conf().isLinearMode() && !score->firstMeasure())) {
        LOGD("empty score");
        DeleteAll(score->systems());
        score->systems().clear();
        DeleteAll(score->pages());
        score->pages().clear();
        PageLayout::getNextPage(ctx);
        return;
    }

    bool isLayoutAll = stick <= Fraction(0, 1) && (etick < Fraction(0, 1) || etick >= score->masterScore()->last()->endTick());
    if (stick < Fraction(0, 1)) {
        stick = Fraction(0, 1);
    }
    if (etick < Fraction(0, 1)) {
        etick = score->last()->endTick();
    }

    ctx.mutState().setEndTick(etick);

    if (isLayoutAll) {
        RootItem* rootItem = score->rootItem();
        resetLayoutData(rootItem);
    }

    if (score->cmdState().layoutFlags & LayoutFlag::REBUILD_MIDI_MAPPING) {
        if (score->isMaster()) {
            score->masterScore()->rebuildMidiMapping();
        }
    }
    if (score->cmdState().layoutFlags & LayoutFlag::FIX_PITCH_VELO) {
        score->updateVelo();
    }

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

    if (ctx.conf().isLinearMode()) {
        ctx.mutState().setPrevMeasure(nullptr);
        ctx.mutState().setNextMeasure(m);         //_showVBox ? first() : firstMeasure();
        ctx.mutState().setStartTick(m->tick());
        layoutLinear(ctx, isLayoutAll);
        return;
    }

    if (!isLayoutAll && m->system()) {
        System* system = m->system();
        system_idx_t systemIndex = mu::indexOf(score->systems(), system);
        ctx.mutState().setPage(system->page());
        ctx.mutState().setPageIdx(score->pageIdx(ctx.state().page()));
        if (ctx.state().pageIdx() == mu::nidx) {
            ctx.mutState().setPageIdx(0);
        }
        ctx.mutState().setCurSystem(system);
        ctx.mutState().setSystemList(mu::mid(score->systems(), systemIndex));

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
        DeleteAll(score->systems());
        score->systems().clear();

        DeleteAll(score->pages());
        score->pages().clear();

        ctx.mutState().setNextMeasure(ctx.conf().isShowVBox() ? score->first() : score->firstMeasure());
    }

    ctx.mutState().setPrevMeasure(nullptr);

    MeasureLayout::getNextMeasure(ctx);
    ctx.mutState().setCurSystem(SystemLayout::collectSystem(ctx));

    doLayout(ctx);
}

void ScoreLayout::doLayout(LayoutContext& ctx)
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
        DeleteAll(ctx.mutState().systemList());
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

void ScoreLayout::layoutLinear(LayoutContext& ctx, bool layoutAll)
{
    resetSystems(ctx, layoutAll);

    collectLinearSystem(ctx);

    layoutLinear(ctx);
}

//  in linear mode there is only one page
//  which contains one system
void ScoreLayout::resetSystems(LayoutContext& ctx, bool layoutAll)
{
    DomAccessor& mutDom = ctx.mutDom();
    Page* page = 0;
    if (layoutAll) {
        for (System* s : mutDom.systems()) {
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->resetExplicitParent();
            }
        }
        DeleteAll(mutDom.systems());
        mutDom.systems().clear();
        DeleteAll(mutDom.pages());
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

// Append all measures to System. VBox is not included to System
void ScoreLayout::collectLinearSystem(LayoutContext& ctx)
{
    std::vector<int> visibleParts;
    for (size_t partIdx = 0; partIdx < ctx.dom().parts().size(); partIdx++) {
        if (ctx.dom().parts().at(partIdx)->show()) {
            visibleParts.push_back(static_cast<int>(partIdx));
        }
    }

    System* system = ctx.mutDom().systems().front();
    SystemLayout::setInstrumentNames(system, ctx, /* longNames */ true);

    PointF pos;
    bool firstMeasure = true;       //lc.startTick.isZero();

    //set first measure to lc.nextMeasures for following
    //utilizing in getNextMeasure()
    ctx.mutState().setNextMeasure(ctx.mutDom().first());
    ctx.mutState().setTick(Fraction(0, 1));
    MeasureLayout::getNextMeasure(ctx);

    static constexpr Fraction minTicks = Fraction(1, 16);
    static constexpr Fraction maxTicks = Fraction(4, 4);
    // CAUTION: In continuous view, we cannot look fot the shortest (or longest) note
    // of the system (as we do in page view), because the whole music is a single big system. Therefore,
    // we simply assume a shortest note of 1/16 and longest of 4/4. This ensures perfect spacing consistency,
    // even if the actual values may be be different.

    while (ctx.state().curMeasure()) {
        double ww = 0.0;
        if (ctx.state().curMeasure()->isVBox() || ctx.state().curMeasure()->isTBox()) {
            ctx.mutState().curMeasure()->resetExplicitParent();
            MeasureLayout::getNextMeasure(ctx);
            continue;
        }
        system->appendMeasure(ctx.mutState().curMeasure());
        if (ctx.state().curMeasure()->isMeasure()) {
            Measure* m = toMeasure(ctx.mutState().curMeasure());
            if (m->mmRest()) {
                m->mmRest()->resetExplicitParent();
            }
            if (firstMeasure) {
                SystemLayout::layoutSystem(system, ctx, pos.rx());
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                MeasureLayout::addSystemHeader(m, true, ctx);
                pos.rx() += system->leftMargin();
                firstMeasure = false;
            } else if (m->header()) {
                MeasureLayout::removeSystemHeader(m);
            }
            if (m->trailer()) {
                MeasureLayout::removeSystemTrailer(m, ctx);
            }
            if (m->tick() >= ctx.state().startTick() && m->tick() <= ctx.state().endTick()) {
                // for measures in range, do full layout
                if (ctx.conf().isMode(LayoutMode::HORIZONTAL_FIXED)) {
                    MeasureLayout::createEndBarLines(m, true, ctx);
                    layoutSegmentsWithDuration(m, visibleParts);
                    ww = m->width();
                    MeasureLayout::stretchMeasureInPracticeMode(m, ww, ctx);
                } else {
                    MeasureLayout::createEndBarLines(m, false, ctx);
                    MeasureLayout::computeWidth(m, ctx, minTicks, maxTicks, 1);
                    ww = m->width();
                    MeasureLayout::layoutMeasureElements(m, ctx);
                }
            } else {
                // for measures not in range, use existing layout
                ww = m->width();
                if (m->pos() != pos) {
                    // fix beam positions
                    // other elements with system as parent are processed in layoutSystemElements()
                    // but full beam processing is expensive and not needed if we adjust position here
                    PointF p = pos - m->pos();
                    for (const Segment& s : m->segments()) {
                        if (!s.isChordRestType()) {
                            continue;
                        }
                        for (size_t track = 0; track < ctx.dom().ntracks(); ++track) {
                            EngravingItem* e = s.element(static_cast<track_idx_t>(track));
                            if (e) {
                                ChordRest* cr = toChordRest(e);
                                if (cr->beam() && cr->beam()->elements().front() == cr) {
                                    cr->beam()->mutldata()->move(p);
                                }
                            }
                        }
                    }
                }
            }
            m->setPos(pos);
            MeasureLayout::layoutStaffLines(m, ctx);
        } else if (ctx.state().curMeasure()->isHBox()) {
            ctx.mutState().curMeasure()->setPos(pos + PointF(toHBox(ctx.state().curMeasure())->topGap(), 0.0));
            TLayout::layoutMeasureBase(ctx.mutState().curMeasure(), ctx);
            ww = ctx.state().curMeasure()->width();
        }
        pos.rx() += ww;

        MeasureLayout::getNextMeasure(ctx);
    }

    system->setWidth(pos.x());
}

void ScoreLayout::layoutLinear(LayoutContext& ctx)
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
                    if (BeamLayout::notTopBeam(cr)) {                           // layout cross staff beams
                        TLayout::layout(cr->beam(), ctx);
                    }
                    if (TupletLayout::notTopTuplet(cr)) {
                        // fix layout of tuplets
                        DurationElement* de = cr;
                        while (de->tuplet() && de->tuplet()->elements().front() == de) {
                            Tuplet* t = de->tuplet();
                            TLayout::layout(t, ctx);
                            de = de->tuplet();
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
                    }
                } else if (e->isBarLine()) {
                    TLayout::layout2(toBarLine(e), ctx);
                }
            }
        }
        MeasureLayout::layout2(m, ctx);
    }

    const double lm = ctx.state().page()->lm();
    const double tm = ctx.state().page()->tm() + ctx.conf().styleMM(Sid::staffUpperBorder);
    const double rm = ctx.state().page()->rm();
    const double bm = ctx.state().page()->bm() + ctx.conf().styleMM(Sid::staffLowerBorder);

    ctx.mutState().page()->setPos(0, 0);
    system->setPos(lm, tm);
    ctx.mutState().page()->setWidth(lm + system->width() + rm);
    ctx.mutState().page()->setHeight(tm + system->height() + bm);
    ctx.mutState().page()->invalidateBspTree();
}

static Segment* findFirstEnabledSegment(Measure* measure)
{
    Segment* current = measure->first();
    while (current && !current->enabled()) {
        current = current->next();
    }

    return current;
}

void ScoreLayout::layoutSegmentsWithDuration(Measure* m, const std::vector<int>& visibleParts)
{
    double currentXPos = 0;

    Segment* current = findFirstEnabledSegment(m);

    auto [spacing, width] = computeCellWidth(current, visibleParts);
    currentXPos = HorizontalSpacing::computeFirstSegmentXPosition(m, current, 1.0);
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

std::pair<double, double> ScoreLayout::computeCellWidth(const Segment* s, const std::vector<int>& visibleParts)
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
        return { 0, HorizontalSpacing::minHorizontalDistance(s, nextSeg, false, 1.0) };
    }

    return { 0, s->minRight() };
}

ChordRest* ScoreLayout::chordRestWithMinDuration(const Segment* seg, const std::vector<int>& visibleParts)
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

Fraction ScoreLayout::calculateQuantumCell(const Measure* m, const std::vector<int>& visibleParts)
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
