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
#include "layoutcontext.h"

#include "../spacer.h"
#include "../systemdivider.h"
#include "../measure.h"
#include "../system.h"

#include "layout.h"
#include "layoutlyrics.h"
#include "layoutmeasure.h"
#include "layoutbeams.h"
#include "verticalgapdata.h"

using namespace mu::engraving;
using namespace Ms;

LayoutContext::LayoutContext(Score* s)
    : score(s)
{
    firstSystemIndent = score && score->styleB(Sid::enableIndentationOnFirstSystem);
}

LayoutContext::~LayoutContext()
{
    for (Spanner* s : processedSpanners) {
        s->layoutSystemsDone();
    }

    for (MuseScoreView* v : score->getViewer()) {
        v->layoutChanged();
    }
}

void LayoutContext::layout()
{
    MeasureBase* lmb;
    do {
        getNextPage();
        collectPage();

        if (page && !page->systems().isEmpty()) {
            lmb = page->systems().back()->measures().back();
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
    } while (curSystem && !(rangeDone && lmb == pageOldMeasure));
    // && page->system(0)->measures().back()->tick() > endTick // FIXME: perhaps the first measure was meant? Or last system?

    if (!curSystem) {
        // The end of the score. The remaining systems are not needed...
        qDeleteAll(systemList);
        systemList.clear();
        // ...and the remaining pages too
        while (score->npages() > curPage) {
            delete score->pages().takeLast();
        }
    } else {
        Page* p = curSystem->page();
        if (p && (p != page)) {
            p->rebuildBspTree();
        }
    }
    score->systems().append(systemList);       // TODO
}

//---------------------------------------------------------
//   layoutSystemElements
//---------------------------------------------------------

void LayoutContext::layoutSystemElements(System* system)
{
    LayoutContext& lc = *this;
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
        if (score->lineMode() && (m->tick() < lc.startTick || m->tick() > lc.endTick)) {
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
        for (Element* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->score()->staff(e->staffIdx())->show()) {
                // the beam and its system may still be referenced when selecting all,
                // even if the staff is invisible. The old system is invalid and does cause problems in #284012
                if (e && e->isChordRest() && !score->score()->staff(e->staffIdx())->show() && toChordRest(e)->beam()) {
                    toChordRest(e)->beam()->setParent(nullptr);
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
            if (score->lineMode() && (m->tick() < lc.startTick || m->tick() > lc.endTick)) {
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
                    for (Element* e : s.elist()) {
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
                                for (Element* en : note->el()) {
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
        for (Element* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->score()->staff(e->staffIdx())->show()) {
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
                    for (Element* el : note->el()) {
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
        for (Element* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->score()->staff(e->staffIdx())->show()) {
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

    for (Segment* s : sl) {
        for (Element* e : s->elist()) {
            if (!e || !e->isChordRest() || !score->score()->staff(e->staffIdx())->show()) {
                continue;
            }
            ChordRest* cr = toChordRest(e);
            if (!isTopTuplet(cr)) {
                continue;
            }
            DurationElement* de = cr;
            while (de->tuplet() && de->tuplet()->elements().front() == de) {
                Tuplet* t = de->tuplet();
                t->layout();
                de = t;
            }
        }
    }

    //-------------------------------------------------------------
    // Drumline sticking
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
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
    auto spanners = score->score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks());

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
        for (Element* e : s->elist()) {
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
        for (Element* e : s->annotations()) {
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
        for (Element* e : s->annotations()) {
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

    LayoutLyrics::layoutLyrics(score, system);

    // here are lyrics dashes and melisma
    for (Spanner* sp : score->_unmanagedSpanner) {
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
        for (Element* e : s->annotations()) {
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
        layoutHarmonies(sl);
        alignHarmonies(system, sl, true, score->styleP(Sid::maxChordShiftAbove), score->styleP(Sid::maxChordShiftBelow));
    }

    //-------------------------------------------------------------
    // StaffText, InstrumentChange
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if (e->isStaffText() || e->isSystemText() || e->isInstrumentChange()) {
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
        for (Element* e : m->el()) {
            if (e->isJump() || e->isMarker()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // TempoText
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
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
            for (Element* e : s->annotations()) {
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

        layoutHarmonies(sl);
        alignHarmonies(system, sl, false, score->styleP(Sid::maxFretShiftAbove), score->styleP(Sid::maxFretShiftBelow));
    }

    //-------------------------------------------------------------
    // RehearsalMark
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if (e->isRehearsalMark()) {
                e->layout();
            }
        }
    }

    //-------------------------------------------------------------
    // Image
    //-------------------------------------------------------------

    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if (e->isImage()) {
                e->layout();
            }
        }
    }
}

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void LayoutContext::layoutLinear()
{
    System* system = score->systems().front();

    layoutSystemElements(system);

    system->layout2();     // compute staff distances

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);

        for (int track = 0; track < score->ntracks(); ++track) {
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                Element* e = segment->element(track);
                if (!e) {
                    continue;
                }
                if (e->isChordRest()) {
                    if (m->tick() < startTick || m->tick() > endTick) {
                        continue;
                    }
                    if (!score->staff(track2staff(track))->show()) {
                        continue;
                    }
                    ChordRest* cr = toChordRest(e);
                    if (notTopBeam(cr)) {                           // layout cross staff beams
                        cr->beam()->layout();
                    }
                    if (notTopTuplet(cr)) {
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
    page->setPos(0, 0);
    system->setPos(page->lm(), page->tm() + score->styleP(Sid::staffUpperBorder));
    page->setWidth(system->width() + system->pos().x());
    // Set buffer space after the last system to avoid problems with mouse input.
    // Mouse input divides space between systems equally (see Score::searchSystem),
    // hence the choice of the value.
    const qreal buffer = 0.5 * score->styleS(Sid::maxSystemDistance).val() * score->spatium();
    page->setHeight(system->height() + system->pos().y() + buffer);
    page->rebuildBspTree();
}

//---------------------------------------------------------
//   getNextPage
//---------------------------------------------------------

void LayoutContext::getNextPage()
{
    if (!page || curPage >= score->npages()) {
        page = new Page(score);
        score->pages().push_back(page);
        prevSystem = nullptr;
        pageOldMeasure = nullptr;
    } else {
        page = score->pages()[curPage];
        QList<System*>& systems = page->systems();
        pageOldMeasure = systems.isEmpty() ? nullptr : systems.back()->measures().back();
        const int i = systems.indexOf(curSystem);
        if (i > 0 && systems[i - 1]->page() == page) {
            // Current and previous systems are on the current page.
            // Erase only the current and the following systems
            // as the previous one will not participate in layout.
            systems.erase(systems.begin() + i, systems.end());
        } else { // system is not on the current page (or will be the first one)
            systems.clear();
        }
        prevSystem = systems.empty() ? nullptr : systems.back();
    }
    page->bbox().setRect(0.0, 0.0, score->loWidth(), score->loHeight());
    page->setNo(curPage);
    qreal x = 0.0;
    qreal y = 0.0;
    if (curPage) {
        Page* prevPage = score->pages()[curPage - 1];
        if (MScore::verticalOrientation()) {
            y = prevPage->pos().y() + page->height() + MScore::verticalPageGap;
        } else {
            qreal gap = (curPage + score->pageNumberOffset()) & 1 ? MScore::horizontalPageGapOdd : MScore::horizontalPageGapEven;
            x = prevPage->pos().x() + page->width() + gap;
        }
    }
    ++curPage;
    page->setPos(x, y);
}

//---------------------------------------------------------
//   distributeStaves
//---------------------------------------------------------

static void distributeStaves(Page* page)
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
    while (!almostZero(spaceLeft) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        qreal smallest     { vgdl.smallest() };
        qreal nextSmallest { vgdl.smallest(smallest) };
        if (almostZero(smallest) || almostZero(nextSmallest)) {
            break;
        }

        if ((nextSmallest - smallest) * vgdl.sumStretchFactor() > spaceLeft) {
            nextSmallest = smallest + spaceLeft / vgdl.sumStretchFactor();
        }

        qreal addedSpace { 0.0 };
        VerticalGapDataList modified;
        for (VerticalGapData* vgd : vgdl) {
            if (!almostZero(vgd->spacing() - smallest)) {
                continue;
            }
            qreal step { nextSmallest - vgd->spacing() };
            if (step < 0.0) {
                continue;
            }
            step = vgd->addSpacing(step);
            if (!almostZero(step)) {
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
    while (!almostZero(spaceLeft) && !almostZero(maxPageFill) && (ngaps > 0) && (++pass < maxPasses)) {
        ngaps = 0;
        qreal addedSpace { 0.0 };
        qreal step { spaceLeft / vgdl.sumStretchFactor() };
        for (VerticalGapData* vgd : vgdl) {
            qreal res { vgd->addFillSpacing(step, maxPageFill) };
            if (!almostZero(res)) {
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

//---------------------------------------------------------
//   checkDivider
//---------------------------------------------------------

static void checkDivider(bool left, System* s, qreal yOffset, bool remove = false)
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

//---------------------------------------------------------
//   layoutPage
//    restHeight - vertical space which has to be distributed
//                 between systems
//    The algorithm tries to produce most equally spaced
//    systems.
//---------------------------------------------------------

static void layoutPage(Page* page, qreal restHeight)
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

//---------------------------------------------------------
//   collectPage
//---------------------------------------------------------

void LayoutContext::collectPage()
{
    const qreal slb = score->styleP(Sid::staffLowerBorder);
    bool breakPages = score->layoutMode() != LayoutMode::SYSTEM;
    qreal ey        = page->height() - page->bm();
    qreal y         = 0.0;

    System* nextSystem = 0;
    int systemIdx = -1;

    // re-calculate positions for systems before current
    // (they may have been filled on previous layout)
    int pSystems = page->systems().size();
    if (pSystems > 0) {
        page->system(0)->restoreLayout2();
        y = page->system(0)->y() + page->system(0)->height();
    } else {
        y = page->tm();
    }
    for (int i = 1; i < pSystems; ++i) {
        System* cs = page->system(i);
        System* ps = page->system(i - 1);
        qreal distance = ps->minDistance(cs);
        y += distance;
        cs->setPos(page->lm(), y);
        cs->restoreLayout2();
        y += cs->height();
    }

    for (int k = 0;; ++k) {
        //
        // calculate distance to previous system
        //
        qreal distance;
        if (prevSystem) {
            distance = prevSystem->minDistance(curSystem);
        } else {
            // this is the first system on page
            if (curSystem->vbox()) {
                distance = 0.0;
            } else {
                distance = score->styleP(Sid::staffUpperBorder);
                bool fixedDistance = false;
                // TODO: curSystem->spacerDistance(true)
                for (MeasureBase* mb : curSystem->measures()) {
                    if (mb->isMeasure()) {
                        Measure* m = toMeasure(mb);
                        Spacer* sp = m->vspacerUp(0);                   // TODO: first visible?
                        if (sp) {
                            if (sp->spacerType() == SpacerType::FIXED) {
                                distance = sp->gap();
                                fixedDistance = true;
                                break;
                            } else {
                                distance = qMax(distance, sp->gap());
                            }
                        }
//TODO::ws                                    distance = qMax(distance, -m->staffShape(0).top());
                    }
                }
                if (!fixedDistance) {
                    distance = qMax(distance, curSystem->minTop());
                }
            }
        }
//TODO-ws ??
//          distance += score->staves().front()->userDist();

        y += distance;
        curSystem->setPos(page->lm(), y);
        curSystem->restoreLayout2();
        page->appendSystem(curSystem);
        y += curSystem->height();

        //
        //  check for page break or if next system will fit on page
        //
        bool collected = false;
        if (rangeDone) {
            // take next system unchanged
            if (systemIdx > 0) {
                nextSystem = score->systems().value(systemIdx++);
                if (!nextSystem) {
                    // TODO: handle next movement
                }
            } else {
                nextSystem = systemList.empty() ? 0 : systemList.takeFirst();
                if (nextSystem) {
                    score->systems().append(nextSystem);
                }
            }
        } else {
            nextSystem = collectSystem();
            if (nextSystem) {
                collected = true;
            }
        }
        prevSystem = curSystem;
        Q_ASSERT(curSystem != nextSystem);
        curSystem  = nextSystem;

        bool breakPage = !curSystem || (breakPages && prevSystem->pageBreak());

        if (!breakPage) {
            qreal dist = prevSystem->minDistance(curSystem) + curSystem->height();
            Box* vbox = curSystem->vbox();
            if (vbox) {
                dist += vbox->bottomGap();
            } else if (!prevSystem->hasFixedDownDistance()) {
                qreal margin = qMax(curSystem->minBottom(), curSystem->spacerDistance(false));
                dist += qMax(margin, slb);
            }
            breakPage = (y + dist) >= ey && breakPages;
        }
        if (breakPage) {
            qreal dist = qMax(prevSystem->minBottom(), prevSystem->spacerDistance(false));
            dist = qMax(dist, slb);
            layoutPage(page, ey - (y + dist));
            // if we collected a system we cannot fit onto this page,
            // we need to collect next page in order to correctly set system positions
            if (collected) {
                pageOldMeasure = nullptr;
            }
            break;
        }
    }

    Fraction stick = Fraction(-1, 1);
    for (System* s : page->systems()) {
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
                        if (notTopBeam(cr)) {                           // layout cross staff beams
                            cr->beam()->layout();
                        }
                        if (notTopTuplet(cr)) {
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

    if (score->systemMode()) {
        System* s = page->systems().last();
        qreal height = s ? s->pos().y() + s->height() + s->minBottom() : page->tm();
        page->bbox().setRect(0.0, 0.0, score->loWidth(), height + page->bm());
    }

    page->rebuildBspTree();
}

//---------------------------------------------------------
//   adjustMeasureNo
//---------------------------------------------------------

int LayoutContext::adjustMeasureNo(MeasureBase* m)
{
    measureNo += m->noOffset();
    m->setNo(measureNo);
    if (!m->irregular()) {          // don’t count measure
        ++measureNo;
    }
    if (m->sectionBreakElement() && m->sectionBreakElement()->startWithMeasureOne()) {
        measureNo = 0;
    }
    return measureNo;
}

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* LayoutContext::getNextSystem()
{
    LayoutContext& lc = *this;
    bool isVBox = lc.curMeasure->isVBox();
    System* system;
    if (lc.systemList.empty()) {
        system = new System(score);
        lc.systemOldMeasure = 0;
    } else {
        system = lc.systemList.takeFirst();
        lc.systemOldMeasure = system->measures().empty() ? 0 : system->measures().back();
        system->clear();       // remove measures from system
    }
    score->_systems.append(system);
    if (!isVBox) {
        int nstaves = score->Score::nstaves();
        system->adjustStavesNumber(nstaves);
        for (int i = 0; i < nstaves; ++i) {
            system->staff(i)->setShow(score->score()->staff(i)->show());
        }
    }
    return system;
}

//---------------------------------------------------------
//   hideEmptyStaves
//---------------------------------------------------------

void LayoutContext::hideEmptyStaves(System* system, bool isFirstSystem)
{
    int staves   = score->_staves.size();
    int staffIdx = 0;
    bool systemIsEmpty = true;

    for (Staff* staff : qAsConst(score->_staves)) {
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
        for (Staff* staff : qAsConst(score->_staves)) {
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
    if (systemIsEmpty && !score->_staves.isEmpty()) {
        Staff* staff = firstVisible ? firstVisible : score->_staves.front();
        SysStaff* ss = system->staff(staff->idx());
        ss->setShow(true);
    }
}

//---------------------------------------------------------
//   collectSystem
//---------------------------------------------------------

System* LayoutContext::collectSystem()
{
    LayoutContext& lc = *this;
    if (!lc.curMeasure) {
        return 0;
    }
    const MeasureBase* measure  = score->_systems.empty() ? 0 : score->_systems.back()->measures().back();
    if (measure) {
        measure = measure->findPotentialSectionBreak();
    }
    if (measure) {
        lc.firstSystem        = measure->sectionBreak() && score->_layoutMode != LayoutMode::FLOAT;
        lc.firstSystemIndent  = lc.firstSystem && measure->sectionBreakElement()->firstSystemIdentation() && score->styleB(
            Sid::enableIndentationOnFirstSystem);
        lc.startWithLongNames = lc.firstSystem && measure->sectionBreakElement()->startWithLongNames();
    }
    System* system = getNextSystem();
    Fraction lcmTick = lc.curMeasure->tick();
    system->setInstrumentNames(lc.startWithLongNames, lcmTick);

    qreal minWidth    = 0;
    qreal layoutSystemMinWidth = 0;
    bool firstMeasure = true;
    bool createHeader = false;
    qreal systemWidth = score->styleD(Sid::pagePrintableWidth) * DPI;
    system->setWidth(systemWidth);

    // save state of measure
    bool curHeader = lc.curMeasure->header();
    bool curTrailer = lc.curMeasure->trailer();
    MeasureBase* breakMeasure = nullptr;

    while (lc.curMeasure) {      // collect measure for system
        System* oldSystem = lc.curMeasure->system();
        system->appendMeasure(lc.curMeasure);

        qreal ww  = 0;          // width of current measure

        if (lc.curMeasure->isMeasure()) {
            Measure* m = toMeasure(lc.curMeasure);
            if (firstMeasure) {
                layoutSystemMinWidth = minWidth;
                system->layoutSystem(minWidth, lc.firstSystem, lc.firstSystemIndent);
                minWidth += system->leftMargin();
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                m->addSystemHeader(lc.firstSystem);
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
            m->computeMinWidth();
            ww = m->width();
        } else if (lc.curMeasure->isHBox()) {
            lc.curMeasure->computeMinWidth();
            ww = lc.curMeasure->width();
            createHeader = toHBox(lc.curMeasure)->createSystemHeader();
        } else {
            // vbox:
            LayoutMeasure::getNextMeasure(score, lc);
            system->layout2();         // compute staff distances
            return system;
        }
        // check if lc.curMeasure fits, remove if not
        // collect at least one measure and the break

        bool doBreak = (system->measures().size() > 1) && ((minWidth + ww) > systemWidth);
        if (doBreak) {
            breakMeasure = lc.curMeasure;
            system->removeLastMeasure();
            lc.curMeasure->setSystem(oldSystem);
            while (lc.prevMeasure && lc.prevMeasure->noBreak() && system->measures().size() > 1) {
                // remove however many measures are grouped with nobreak, working backwards
                // but if too many are grouped, stop before we get 0 measures left on system
                // TODO: intelligently break group into smaller groups instead
                lc.tick -= lc.curMeasure->ticks();
                lc.measureNo = lc.prevMeasure->no();

                lc.nextMeasure = lc.curMeasure;
                lc.curMeasure  = lc.prevMeasure;
                lc.prevMeasure = lc.curMeasure->prevMeasure();

                minWidth -= system->lastMeasure()->width();
                system->removeLastMeasure();
                lc.curMeasure->setSystem(oldSystem);
            }
            break;
        }

        if (lc.prevMeasure && lc.prevMeasure->isMeasure() && lc.prevMeasure->system() == system) {
            //
            // now we know that the previous measure is not the last
            // measure in the system and we finally can create the end barline for it

            Measure* m = toMeasure(lc.prevMeasure);
            // TODO: if lc.curMeasure is a frame, removing the trailer may be premature
            // but merely skipping this code isn't good enough,
            // we need to find the right time to re-enable the trailer,
            // since it seems to be disabled somewhere else
            if (m->trailer()) {
                qreal ow = m->width();
                m->removeSystemTrailer();
                minWidth += m->width() - ow;
            }
            // if the prev measure is an end repeat and the cur measure
            // is an repeat, the createEndBarLines() created an start-end repeat barline
            // and we can remove the start repeat barline of the current barline

            if (lc.curMeasure->isMeasure()) {
                Measure* m1 = toMeasure(lc.curMeasure);
                if (m1->repeatStart()) {
                    Segment* s = m1->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                        m1->computeMinWidth();
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
            minWidth += m->createEndBarLines(false);          // create final barLine
        }

        MeasureBase* mb = lc.curMeasure;
        bool lineBreak  = false;
        switch (score->_layoutMode) {
        case LayoutMode::PAGE:
        case LayoutMode::SYSTEM:
            lineBreak = mb->pageBreak() || mb->lineBreak() || mb->sectionBreak();
            break;
        case LayoutMode::FLOAT:
        case LayoutMode::LINE:
            lineBreak = false;
            break;
        }

        // preserve state of next measure (which is about to become current measure)
        if (lc.nextMeasure) {
            MeasureBase* nmb = lc.nextMeasure;
            if (nmb->isMeasure() && score->styleB(Sid::createMultiMeasureRests)) {
                Measure* nm = toMeasure(nmb);
                if (nm->hasMMRest()) {
                    nmb = nm->mmRest();
                }
            }
            nmb->setOldWidth(nmb->width());
            if (!lc.curMeasure->noBreak()) {
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

        LayoutMeasure::getNextMeasure(score, lc);

        minWidth += ww;

        // ElementType nt = lc.curMeasure ? lc.curMeasure->type() : ElementType::INVALID;
        mb = lc.curMeasure;
        bool tooWide = false;     // minWidth + minMeasureWidth > systemWidth;  // TODO: noBreak
        if (lineBreak || !mb || mb->isVBox() || mb->isTBox() || mb->isFBox() || tooWide) {
            break;
        }
    }

    if (lc.endTick < lc.prevMeasure->tick()) {
        // we've processed the entire range
        // but we need to continue layout until we reach a system whose last measure is the same as previous layout
        if (lc.prevMeasure == lc.systemOldMeasure) {
            // this system ends in the same place as the previous layout
            // ok to stop
            if (lc.curMeasure && lc.curMeasure->isMeasure()) {
                // we may have previously processed first measure(s) of next system
                // so now we must restore to original state
                Measure* m = toMeasure(lc.curMeasure);
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                const MeasureBase* pbmb = lc.prevMeasure->findPotentialSectionBreak();
                bool localFirstSystem = pbmb->sectionBreak() && score->_layoutMode != LayoutMode::FLOAT;
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
                    m->computeMinWidth();
                    m->stretchMeasure(m->oldWidth());
                    LayoutBeams::restoreBeams(m);
                    if (m == nm || !m->noBreak()) {
                        break;
                    }
                    m = m->nextMeasure();
                }
            }
            lc.rangeDone = true;
        }
    }

    //
    // now we have a complete set of measures for this system
    //
    // prevMeasure is the last measure in the system
    if (lc.prevMeasure && lc.prevMeasure->isMeasure()) {
        LayoutBeams::breakCrossMeasureBeams(toMeasure(lc.prevMeasure));
        qreal w = toMeasure(lc.prevMeasure)->createEndBarLines(true);
        minWidth += w;
    }

    hideEmptyStaves(system, lc.firstSystem);
    // Relayout system decorations to reuse space properly for
    // hidden staves' instrument names or other hidden elements.
    minWidth -= system->leftMargin();
    system->layoutSystem(layoutSystemMinWidth, lc.firstSystem, lc.firstSystemIndent);
    minWidth += system->leftMargin();

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
                lm->computeMinWidth();
            }
            minWidth += lm->width() - w;
        }
    }

    //
    // stretch incomplete row
    //
    qreal rest;
    if (MScore::noHorizontalStretch) {
        rest = 0;
    } else {
        qreal mw          = system->leftMargin();          // DEBUG
        qreal totalWeight = 0.0;

        for (MeasureBase* mb : system->measures()) {
            if (mb->isHBox()) {
                mw += mb->width();
            } else if (mb->isMeasure()) {
                Measure* m  = toMeasure(mb);
                mw          += m->width();                       // measures are stretched already with basicStretch()
                int weight   = m->layoutWeight();
                totalWeight += weight * m->basicStretch();
            }
        }

#ifndef NDEBUG
        if (!qFuzzyCompare(mw, minWidth)) {
            qDebug("==layoutSystem %6d old %.1f new %.1f", system->measures().front()->tick().ticks(), minWidth, mw);
        }
#endif
        rest = systemWidth - minWidth;
        //
        // don’t stretch last system row, if accumulated minWidth is <= lastSystemFillLimit
        //
        if (lc.curMeasure == 0 && ((minWidth / systemWidth) <= score->styleD(Sid::lastSystemFillLimit))) {
            if (minWidth > rest) {
                rest = rest * .5;
            } else {
                rest = minWidth;
            }
        }
        rest /= totalWeight;
    }

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
            qreal stretch = m->basicStretch();
            int weight = m->layoutWeight();
            ww  += rest * weight * stretch;
            m->stretchMeasure(ww);
            m->layoutStaffLines();
            if (createBrackets) {
                system->addBrackets(toMeasure(mb));
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

    layoutSystemElements(system);
    system->layout2();     // compute staff distances
    // TODO: now that the code at the top of this function does this same backwards search,
    // we might be able to eliminate this block
    // but, lc might be used elsewhere so we need to be careful
#if 1
    measure = system->measures().back();
    if (measure) {
        measure = measure->findPotentialSectionBreak();
    }
    if (measure) {
        lc.firstSystem        = measure->sectionBreak() && score->_layoutMode != LayoutMode::FLOAT;
        lc.firstSystemIndent  = lc.firstSystem && measure->sectionBreakElement()->firstSystemIdentation() && score->styleB(
            Sid::enableIndentationOnFirstSystem);
        lc.startWithLongNames = lc.firstSystem && measure->sectionBreakElement()->startWithLongNames();
    }
#endif
    return system;
}
