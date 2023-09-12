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
#include "scorehorizontalviewlayout.h"

#include "containers.h"

#include "dom/durationelement.h"
#include "dom/factory.h"
#include "dom/score.h"
#include "dom/system.h"
#include "dom/spanner.h"
#include "dom/page.h"
#include "dom/durationelement.h"
#include "dom/staff.h"
#include "dom/chordrest.h"
#include "dom/chord.h"
#include "dom/tuplet.h"
#include "dom/tremolo.h"
#include "dom/part.h"
#include "dom/box.h"

#include "passresetlayoutdata.h"

#include "tlayout.h"
#include "systemlayout.h"
#include "beamlayout.h"
#include "tupletlayout.h"
#include "chordlayout.h"
#include "arpeggiolayout.h"
#include "measurelayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

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
        page->mutLayoutData()->setBbox(0.0, 0.0, ctx.conf().loWidth(), ctx.conf().loHeight());
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
                    m->layoutSegmentsInPracticeMode(visibleParts);
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
                                    cr->beam()->mutLayoutData()->move(p);
                                }
                            }
                        }
                    }
                }
            }
            m->setPos(pos);
            MeasureLayout::layoutStaffLines(m, ctx);
        } else if (ctx.state().curMeasure()->isHBox()) {
            MeasureBase* curM = ctx.mutState().curMeasure();
            curM->setPos(pos + PointF(toHBox(curM)->topGap(), 0.0));
            TLayout::layoutMeasureBase(curM, curM->mutLayoutData(), ctx);
            ww = curM->width();
        }
        pos.rx() += ww;

        MeasureLayout::getNextMeasure(ctx);
    }

    system->setWidth(pos.x());
}
