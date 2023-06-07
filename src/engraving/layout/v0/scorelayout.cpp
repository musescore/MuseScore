#include "scorelayout.h"

#include "containers.h"

#include "libmscore/score.h"
#include "libmscore/masterscore.h"
#include "libmscore/system.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/bracket.h"
#include "libmscore/page.h"
#include "libmscore/factory.h"
#include "libmscore/spanner.h"
#include "libmscore/part.h"
#include "libmscore/chordrest.h"
#include "libmscore/box.h"
#include "libmscore/beam.h"
#include "libmscore/staff.h"
#include "libmscore/tuplet.h"
#include "libmscore/chord.h"
#include "libmscore/tremolo.h"

#include "tlayout.h"
#include "pagelayout.h"
#include "measurelayout.h"
#include "systemlayout.h"
#include "beamlayout.h"
#include "tupletlayout.h"
#include "arpeggiolayout.h"
#include "chordlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

class CmdStateLocker
{
    Score* m_score = nullptr;
public:
    CmdStateLocker(Score* s)
        : m_score(s) { m_score->cmdState().lock(); }
    ~CmdStateLocker() { m_score->cmdState().unlock(); }
};

void ScoreLayout::layoutRange(Score* score, const LayoutOptions& options, const Fraction& st, const Fraction& et)
{
    CmdStateLocker cmdStateLocker(score);
    LayoutContext ctx(score);

    Fraction stick(st);
    Fraction etick(et);
    assert(!(stick == Fraction(-1, 1) && etick == Fraction(-1, 1)));

    if (!score->last() || (options.isLinearMode() && !score->firstMeasure())) {
        LOGD("empty score");
        DeleteAll(score->_systems);
        score->_systems.clear();
        DeleteAll(score->pages());
        score->pages().clear();
        PageLayout::getNextPage(options, ctx);
        return;
    }

    bool layoutAll = stick <= Fraction(0, 1) && (etick < Fraction(0, 1) || etick >= score->masterScore()->last()->endTick());
    if (stick < Fraction(0, 1)) {
        stick = Fraction(0, 1);
    }
    if (etick < Fraction(0, 1)) {
        etick = score->last()->endTick();
    }

    ctx.endTick = etick;

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

    if (options.isLinearMode()) {
        ctx.prevMeasure = 0;
        ctx.nextMeasure = m;         //_showVBox ? first() : firstMeasure();
        ctx.startTick   = m->tick();
        layoutLinear(layoutAll, options, ctx);
        return;
    }

    if (!layoutAll && m->system()) {
        System* system = m->system();
        system_idx_t systemIndex = mu::indexOf(score->_systems, system);
        ctx.page = system->page();
        ctx.curPage = score->pageIdx(ctx.page);
        if (ctx.curPage == mu::nidx) {
            ctx.curPage = 0;
        }
        ctx.curSystem   = system;
        ctx.systemList  = mu::mid(score->_systems, systemIndex);

        if (systemIndex == 0) {
            ctx.nextMeasure = options.showVBox ? score->first() : score->firstMeasure();
        } else {
            System* prevSystem = score->_systems[systemIndex - 1];
            ctx.nextMeasure = prevSystem->measures().back()->next();
        }

        score->_systems.erase(score->_systems.begin() + systemIndex, score->_systems.end());
        if (!ctx.nextMeasure->prevMeasure()) {
            ctx.measureNo = 0;
            ctx.tick      = Fraction(0, 1);
        } else {
            const MeasureBase* mb = ctx.nextMeasure->prev();
            if (mb) {
                mb = mb->findPotentialSectionBreak();
            }

            const LayoutBreak* layoutBreak = mb->sectionBreakElement();
            // TODO: also use mb in else clause here?
            // probably not, only actual measures have meaningful numbers
            if (layoutBreak && layoutBreak->startWithMeasureOne()) {
                ctx.measureNo = 0;
            } else {
                ctx.measureNo = ctx.nextMeasure->prevMeasure()->no()                             // will be adjusted later with respect
                                + (ctx.nextMeasure->prevMeasure()->irregular() ? 0 : 1);        // to the user-defined offset.
            }
            ctx.tick = ctx.nextMeasure->tick();
        }
    } else {
        for (System* s : score->_systems) {
            for (Bracket* b : s->brackets()) {
                if (b->selected()) {
                    bool selected = b->selected();
                    score->_selection.remove(b);
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
        DeleteAll(score->_systems);
        score->_systems.clear();

        DeleteAll(score->pages());
        score->pages().clear();

        ctx.nextMeasure = options.showVBox ? score->first() : score->firstMeasure();
    }

    ctx.prevMeasure = 0;

    MeasureLayout::getNextMeasure(options, ctx);
    ctx.curSystem = SystemLayout::collectSystem(options, ctx, score);

    doLayout(options, ctx);
}

void ScoreLayout::doLayout(const LayoutOptions& options, LayoutContext& lc)
{
    MeasureBase* lmb;
    do {
        PageLayout::getNextPage(options, lc);
        PageLayout::collectPage(options, lc);

        if (lc.page && !lc.page->systems().empty()) {
            lmb = lc.page->systems().back()->measures().back();
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
    } while (lc.curSystem && !(lc.rangeDone && lmb == lc.pageOldMeasure));
    // && page->system(0)->measures().back()->tick() > endTick // FIXME: perhaps the first measure was meant? Or last system?

    if (!lc.curSystem) {
        // The end of the score. The remaining systems are not needed...
        DeleteAll(lc.systemList);
        lc.systemList.clear();
        // ...and the remaining pages too
        while (lc.score()->npages() > lc.curPage) {
            Page* p = lc.score()->pages().back();
            lc.score()->pages().pop_back();
            delete p;
        }
    } else {
        Page* p = lc.curSystem->page();
        if (p && (p != lc.page)) {
            p->invalidateBspTree();
        }
    }
    lc.score()->systems().insert(lc.score()->systems().end(), lc.systemList.begin(), lc.systemList.end());
}

void ScoreLayout::layoutLinear(bool layoutAll, const LayoutOptions& options, LayoutContext& ctx)
{
    resetSystems(layoutAll, options, ctx);

    collectLinearSystem(options, ctx);

    layoutLinear(options, ctx);
}

//  in linear mode there is only one page
//  which contains one system
void ScoreLayout::resetSystems(bool layoutAll, const LayoutOptions& options, LayoutContext& ctx)
{
    Page* page = 0;
    if (layoutAll) {
        for (System* s : ctx.score()->_systems) {
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->resetExplicitParent();
            }
        }
        DeleteAll(ctx.score()->_systems);
        ctx.score()->_systems.clear();
        DeleteAll(ctx.score()->pages());
        ctx.score()->pages().clear();
        if (!ctx.score()->firstMeasure()) {
            LOGD("no measures");
            return;
        }

        for (MeasureBase* mb = ctx.score()->first(); mb; mb = mb->next()) {
            mb->resetExplicitParent();
        }

        page = Factory::createPage(ctx.score()->rootItem());
        ctx.score()->pages().push_back(page);
        page->bbox().setRect(0.0, 0.0, options.loWidth, options.loHeight);
        page->setNo(0);

        System* system = Factory::createSystem(page);
        ctx.score()->_systems.push_back(system);
        page->appendSystem(system);
        system->adjustStavesNumber(static_cast<int>(ctx.score()->nstaves()));
    } else {
        if (ctx.score()->pages().empty()) {
            return;
        }
        page = ctx.score()->pages().front();
        System* system = ctx.score()->systems().front();
        system->clear();
        system->adjustStavesNumber(static_cast<int>(ctx.score()->nstaves()));
    }
    ctx.page = page;
}

// Append all measures to System. VBox is not included to System
void ScoreLayout::collectLinearSystem(const LayoutOptions& options, LayoutContext& ctx)
{
    std::vector<int> visibleParts;
    for (size_t partIdx = 0; partIdx < ctx.score()->parts().size(); partIdx++) {
        if (ctx.score()->parts().at(partIdx)->show()) {
            visibleParts.push_back(static_cast<int>(partIdx));
        }
    }

    System* system = ctx.score()->systems().front();
    system->setInstrumentNames(ctx, /* longNames */ true);

    PointF pos;
    bool firstMeasure = true;       //lc.startTick.isZero();

    //set first measure to lc.nextMeasures for following
    //utilizing in getNextMeasure()
    ctx.nextMeasure = ctx.score()->_measures.first();
    ctx.tick = Fraction(0, 1);
    MeasureLayout::getNextMeasure(options, ctx);

    static constexpr Fraction minTicks = Fraction(1, 16);
    static constexpr Fraction maxTicks = Fraction(4, 4);
    // CAUTION: In continuous view, we cannot look fot the shortest (or longest) note
    // of the system (as we do in page view), because the whole music is a single big system. Therefore,
    // we simply assume a shortest note of 1/16 and longest of 4/4. This ensures perfect spacing consistency,
    // even if the actual values may be be different.

    while (ctx.curMeasure) {
        double ww = 0.0;
        if (ctx.curMeasure->isVBox() || ctx.curMeasure->isTBox()) {
            ctx.curMeasure->resetExplicitParent();
            MeasureLayout::getNextMeasure(options, ctx);
            continue;
        }
        system->appendMeasure(ctx.curMeasure);
        if (ctx.curMeasure->isMeasure()) {
            Measure* m = toMeasure(ctx.curMeasure);
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
                m->removeSystemHeader();
            }
            if (m->trailer()) {
                m->removeSystemTrailer();
            }
            if (m->tick() >= ctx.startTick && m->tick() <= ctx.endTick) {
                // for measures in range, do full layout
                if (options.isMode(LayoutMode::HORIZONTAL_FIXED)) {
                    MeasureLayout::createEndBarLines(m, true, ctx);
                    m->layoutSegmentsInPracticeMode(visibleParts);
                    ww = m->width();
                    MeasureLayout::stretchMeasureInPracticeMode(m, ww, ctx);
                } else {
                    MeasureLayout::createEndBarLines(m, false, ctx);
                    m->computeWidth(minTicks, maxTicks, 1);
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
                        for (size_t track = 0; track < ctx.score()->ntracks(); ++track) {
                            EngravingItem* e = s.element(static_cast<int>(track));
                            if (e) {
                                ChordRest* cr = toChordRest(e);
                                if (cr->beam() && cr->beam()->elements().front() == cr) {
                                    cr->beam()->movePos(p);
                                }
                            }
                        }
                    }
                }
            }
            m->setPos(pos);
            MeasureLayout::layoutStaffLines(m, ctx);
        } else if (ctx.curMeasure->isHBox()) {
            ctx.curMeasure->setPos(pos + PointF(toHBox(ctx.curMeasure)->topGap(), 0.0));
            TLayout::layoutMeasureBase(ctx.curMeasure, ctx);
            ww = ctx.curMeasure->width();
        }
        pos.rx() += ww;

        MeasureLayout::getNextMeasure(options, ctx);
    }

    system->setWidth(pos.x());
}

void ScoreLayout::layoutLinear(const LayoutOptions& options, LayoutContext& ctx)
{
    System* system = ctx.score()->systems().front();

    SystemLayout::layoutSystemElements(options, ctx, ctx.score(), system);

    system->layout2(ctx);     // compute staff distances

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);

        for (size_t track = 0; track < ctx.score()->ntracks(); ++track) {
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                EngravingItem* e = segment->element(static_cast<int>(track));
                if (!e) {
                    continue;
                }
                if (e->isChordRest()) {
                    if (m->tick() < ctx.startTick || m->tick() > ctx.endTick) {
                        continue;
                    }
                    if (!ctx.score()->staff(track2staff(static_cast<int>(track)))->show()) {
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

    const double lm = ctx.page->lm();
    const double tm = ctx.page->tm() + ctx.score()->styleMM(Sid::staffUpperBorder);
    const double rm = ctx.page->rm();
    const double bm = ctx.page->bm() + ctx.score()->styleMM(Sid::staffLowerBorder);

    ctx.page->setPos(0, 0);
    system->setPos(lm, tm);
    ctx.page->setWidth(lm + system->width() + rm);
    ctx.page->setHeight(tm + system->height() + bm);
    ctx.page->invalidateBspTree();
}
