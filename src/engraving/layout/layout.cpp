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
#include "layout.h"

#include "libmscore/score.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/scorefont.h"
#include "libmscore/bracket.h"
#include "libmscore/chordrest.h"
#include "libmscore/box.h"
#include "libmscore/marker.h"
#include "libmscore/barline.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "libmscore/keysig.h"
#include "libmscore/chord.h"
#include "libmscore/stem.h"
#include "libmscore/lyrics.h"
#include "libmscore/measurenumber.h"
#include "libmscore/fingering.h"
#include "libmscore/mmrestrange.h"
#include "libmscore/stafflines.h"
#include "libmscore/tuplet.h"
#include "libmscore/tie.h"
#include "libmscore/system.h"
#include "libmscore/page.h"

#include "layoutcontext.h"
#include "layoutpage.h"
#include "layoutmeasure.h"
#include "layoutsystem.h"
#include "layoutbeams.h"
#include "layouttuplets.h"

using namespace mu::engraving;
using namespace Ms;
//---------------------------------------------------------
//   CmdStateLocker
//---------------------------------------------------------

class CmdStateLocker
{
    Score* m_score = nullptr;
public:
    CmdStateLocker(Score* s)
        : m_score(s) { m_score->cmdState().lock(); }
    ~CmdStateLocker() { m_score->cmdState().unlock(); }
};

Layout::Layout(Ms::Score* score)
    : m_score(score)
{
}

void Layout::doLayoutRange(const LayoutOptions& options, const Fraction& st, const Fraction& et)
{
    CmdStateLocker cmdStateLocker(m_score);
    LayoutContext lc(m_score);

    Fraction stick(st);
    Fraction etick(et);
    Q_ASSERT(!(stick == Fraction(-1, 1) && etick == Fraction(-1, 1)));

    if (!m_score->last() || (options.isMode(LayoutMode::LINE) && !m_score->firstMeasure())) {
        qDebug("empty score");
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();
        qDeleteAll(m_score->pages());
        m_score->pages().clear();
        LayoutPage::getNextPage(options, lc);
        return;
    }

    bool layoutAll = stick <= Fraction(0, 1) && (etick < Fraction(0, 1) || etick >= m_score->masterScore()->last()->endTick());
    if (stick < Fraction(0, 1)) {
        stick = Fraction(0, 1);
    }
    if (etick < Fraction(0, 1)) {
        etick = m_score->last()->endTick();
    }

    lc.endTick = etick;

    if (m_score->cmdState().layoutFlags & LayoutFlag::REBUILD_MIDI_MAPPING) {
        if (m_score->isMaster()) {
            m_score->masterScore()->rebuildMidiMapping();
        }
    }
    if (m_score->cmdState().layoutFlags & LayoutFlag::FIX_PITCH_VELO) {
        m_score->updateVelo();
    }

    //---------------------------------------------------
    //    initialize layout context lc
    //---------------------------------------------------

    MeasureBase* m = m_score->tick2measure(stick);
    if (m == 0) {
        m = m_score->first();
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
        qDebug("  don’t start with mmrest");
        m = toMeasure(m)->mmRest();
    }

    if (options.isMode(LayoutMode::LINE)) {
        lc.prevMeasure = 0;
        lc.nextMeasure = m;         //_showVBox ? first() : firstMeasure();
        lc.startTick   = m->tick();
        layoutLinear(layoutAll, options, lc);
        return;
    }

    if (!layoutAll && m->system()) {
        System* system  = m->system();
        int systemIndex = m_score->_systems.indexOf(system);
        lc.page         = system->page();
        lc.curPage      = m_score->pageIdx(lc.page);
        if (lc.curPage == -1) {
            lc.curPage = 0;
        }
        lc.curSystem   = system;
        lc.systemList  = m_score->_systems.mid(systemIndex);

        if (systemIndex == 0) {
            lc.nextMeasure = options.showVBox ? m_score->first() : m_score->firstMeasure();
        } else {
            System* prevSystem = m_score->_systems[systemIndex - 1];
            lc.nextMeasure = prevSystem->measures().back()->next();
        }

        m_score->_systems.erase(m_score->_systems.begin() + systemIndex, m_score->_systems.end());
        if (!lc.nextMeasure->prevMeasure()) {
            lc.measureNo = 0;
            lc.tick      = Fraction(0, 1);
        } else {
            const MeasureBase* mb = lc.nextMeasure->prev();
            if (mb) {
                mb = mb->findPotentialSectionBreak();
            }
            LayoutBreak* sectionBreak = mb->sectionBreakElement();
            // TODO: also use mb in else clause here?
            // probably not, only actual measures have meaningful numbers
            if (sectionBreak && sectionBreak->startWithMeasureOne()) {
                lc.measureNo = 0;
            } else {
                lc.measureNo = lc.nextMeasure->prevMeasure()->no()                             // will be adjusted later with respect
                               + (lc.nextMeasure->prevMeasure()->irregular() ? 0 : 1);         // to the user-defined offset.
            }
            lc.tick = lc.nextMeasure->tick();
        }
    } else {
        for (System* s : qAsConst(m_score->_systems)) {
            for (Bracket* b : s->brackets()) {
                if (b->selected()) {
                    m_score->_selection.remove(b);
                    m_score->setSelectionChanged(true);
                }
            }
            s->setParent(nullptr);
        }
        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            mb->setSystem(0);
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                toMeasure(mb)->mmRest()->setSystem(0);
            }
        }
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();

        qDeleteAll(m_score->pages());
        m_score->pages().clear();

        lc.nextMeasure = options.showVBox ? m_score->first() : m_score->firstMeasure();
    }

    lc.prevMeasure = 0;

    LayoutMeasure::getNextMeasure(options, m_score, lc);
    lc.curSystem = LayoutSystem::collectSystem(options, lc, m_score);

    doLayout(options, lc);
}

void Layout::doLayout(const LayoutOptions& options, LayoutContext& lc)
{
    MeasureBase* lmb;
    do {
        LayoutPage::getNextPage(options, lc);
        LayoutPage::collectPage(options, lc);

        if (lc.page && !lc.page->systems().isEmpty()) {
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
        qDeleteAll(lc.systemList);
        lc.systemList.clear();
        // ...and the remaining pages too
        while (lc.score->npages() > lc.curPage) {
            delete lc.score->pages().takeLast();
        }
    } else {
        Page* p = lc.curSystem->page();
        if (p && (p != lc.page)) {
            p->invalidateBspTree();
        }
    }
    lc.score->systems().append(lc.systemList);
}

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Layout::layoutLinear(bool layoutAll, const LayoutOptions& options, LayoutContext& lc)
{
    lc.score = m_score;
    resetSystems(layoutAll, options, lc);

    collectLinearSystem(options, lc);

    layoutLinear(options, lc);
}

//---------------------------------------------------------
//   resetSystems
//    in linear mode there is only one page
//    which contains one system
//---------------------------------------------------------

void Layout::resetSystems(bool layoutAll, const LayoutOptions& options, LayoutContext& lc)
{
    Page* page = 0;
    if (layoutAll) {
        for (System* s : qAsConst(m_score->_systems)) {
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setParent(0);
            }
        }
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();
        qDeleteAll(m_score->pages());
        m_score->pages().clear();
        if (!m_score->firstMeasure()) {
            qDebug("no measures");
            return;
        }

        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            mb->setSystem(0);
        }

        page = new Page(m_score);
        m_score->pages().push_back(page);
        page->bbox().setRect(0.0, 0.0, options.loWidth, options.loHeight);
        page->setNo(0);

        System* system = new System(m_score);
        m_score->_systems.push_back(system);
        page->appendSystem(system);
        system->adjustStavesNumber(m_score->nstaves());
    } else {
        if (m_score->pages().isEmpty()) {
            return;
        }
        page = m_score->pages().front();
        System* system = m_score->systems().front();
        system->clear();
        system->adjustStavesNumber(m_score->nstaves());
    }
    lc.page = page;
}

//---------------------------------------------------------
//   collectLinearSystem
//   Append all measures to System. VBox is not included to System
//---------------------------------------------------------

void Layout::collectLinearSystem(const LayoutOptions& options, LayoutContext& lc)
{
    System* system = m_score->systems().front();
    system->setInstrumentNames(/* longNames */ true);

    PointF pos;
    bool firstMeasure = true;       //lc.startTick.isZero();

    //set first measure to lc.nextMeasures for following
    //utilizing in getNextMeasure()
    lc.nextMeasure = m_score->_measures.first();
    lc.tick = Fraction(0, 1);
    LayoutMeasure::getNextMeasure(options, m_score, lc);

    while (lc.curMeasure) {
        qreal ww = 0.0;
        if (lc.curMeasure->isVBox() || lc.curMeasure->isTBox()) {
            lc.curMeasure->setParent(nullptr);
            LayoutMeasure::getNextMeasure(options, m_score, lc);
            continue;
        }
        system->appendMeasure(lc.curMeasure);
        if (lc.curMeasure->isMeasure()) {
            Measure* m = toMeasure(lc.curMeasure);
            if (m->mmRest()) {
                m->mmRest()->setSystem(nullptr);
            }
            if (firstMeasure) {
                system->layoutSystem(pos.rx());
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                m->addSystemHeader(true);
                pos.rx() += system->leftMargin();
                firstMeasure = false;
            } else if (m->header()) {
                m->removeSystemHeader();
            }
            if (m->trailer()) {
                m->removeSystemTrailer();
            }
            if (m->tick() >= lc.startTick && m->tick() <= lc.endTick) {
                // for measures in range, do full layout
                m->createEndBarLines(false);
                m->computeMinWidth();
                ww = m->width();
                m->stretchMeasure(ww);
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
                        for (int track = 0; track < m_score->ntracks(); ++track) {
                            Element* e = s.element(track);
                            if (e) {
                                ChordRest* cr = toChordRest(e);
                                if (cr->beam() && cr->beam()->elements().front() == cr) {
                                    cr->beam()->rpos() += p;
                                }
                            }
                        }
                    }
                }
            }
            m->setPos(pos);
            m->layoutStaffLines();
        } else if (lc.curMeasure->isHBox()) {
            lc.curMeasure->setPos(pos + PointF(toHBox(lc.curMeasure)->topGap(), 0.0));
            lc.curMeasure->layout();
            ww = lc.curMeasure->width();
        }
        pos.rx() += ww;

        LayoutMeasure::getNextMeasure(options, m_score, lc);
    }

    system->setWidth(pos.x());
}

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Layout::layoutLinear(const LayoutOptions& options, LayoutContext& lc)
{
    System* system = lc.score->systems().front();

    LayoutSystem::layoutSystemElements(options, lc, lc.score, system);

    system->layout2();     // compute staff distances

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);

        for (int track = 0; track < lc.score->ntracks(); ++track) {
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                Element* e = segment->element(track);
                if (!e) {
                    continue;
                }
                if (e->isChordRest()) {
                    if (m->tick() < lc.startTick || m->tick() > lc.endTick) {
                        continue;
                    }
                    if (!lc.score->staff(track2staff(track))->show()) {
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
                            de = de->tuplet();
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
    lc.page->setPos(0, 0);
    system->setPos(lc.page->lm(), lc.page->tm() + lc.score->styleP(Sid::staffUpperBorder));
    lc.page->setWidth(system->width() + system->pos().x());
    // Set buffer space after the last system to avoid problems with mouse input.
    // Mouse input divides space between systems equally (see Score::searchSystem),
    // hence the choice of the value.
    const qreal buffer = 0.5 * lc.score->styleS(Sid::maxSystemDistance).val() * lc.score->spatium();
    lc.page->setHeight(system->height() + system->pos().y() + buffer);
    lc.page->invalidateBspTree();
}
