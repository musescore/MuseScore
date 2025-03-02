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
#include "beamlayout.h"

#include <unordered_map>
#include <set>

#include "containers.h"

#include "dom/beam.h"
#include "dom/drumset.h"
#include "dom/part.h"
#include "dom/tremolotwochord.h"
#include "dom/tremolosinglechord.h"
#include "dom/chord.h"
#include "dom/factory.h"
#include "dom/measure.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/timesig.h"
#include "dom/tuplet.h"
#include "dom/utils.h"
#include "dom/note.h"

#include "layoutcontext.h"
#include "tlayout.h"
#include "chordlayout.h"
#include "beamtremololayout.h"
#include "tremololayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void BeamLayout::layout(Beam* item, const LayoutContext& ctx)
{
    Beam::LayoutData* ldata = item->mutldata();
    // all of the beam layout code depends on _elements being in order by tick
    // this may not be the case if two cr's were recently swapped.
    std::sort(item->elements().begin(), item->elements().end(),
              [](const ChordRest* a, const ChordRest* b) -> bool {
        return a->tick() < b->tick();
    });
    System* system = item->elements().front()->measure()->system();
    item->setParent(system);

    std::vector<ChordRest*> crl;

    size_t n = 0;
    for (ChordRest* cr : item->elements()) {
        auto newSystem = cr->measure()->system();
        if (newSystem && newSystem != system) {
            SpannerSegmentType st;
            if (n == 0) {
                st = SpannerSegmentType::BEGIN;
            } else {
                st = SpannerSegmentType::MIDDLE;
            }
            ++n;
            if (item->beamFragments().size() < n) {
                item->beamFragments().push_back(new BeamFragment);
            }
            layout2(item, ctx, crl, st, static_cast<int>(n) - 1);
            crl.clear();
            system = cr->measure()->system();
        }
        crl.push_back(cr);
    }

    Shape beamShape(Shape::Type::Composite);

    if (!crl.empty()) {
        SpannerSegmentType st;
        if (n == 0) {
            st = SpannerSegmentType::SINGLE;
        } else {
            st = SpannerSegmentType::END;
        }
        if (item->beamFragments().size() < (n + 1)) {
            item->beamFragments().push_back(new BeamFragment);
        }
        layout2(item, ctx, crl, st, static_cast<int>(n));

        for (const BeamSegment* bs : item->beamSegments()) {
            beamShape.add(bs->shape());
        }
    }

    ldata->setShape(beamShape);

    // The beam may have changed shape. one-note trems within this beam need to be layed out here
    for (ChordRest* cr : item->elements()) {
        if (cr->isChord() && toChord(cr)->tremoloSingleChord()) {
            TremoloLayout::layout(toChord(cr)->tremoloSingleChord(), ctx);
        }
    }
}

void BeamLayout::layoutIfNeed(Beam* item, const LayoutContext& ctx)
{
    if (!item->ldata()->isValid()) {
        BeamLayout::layout(item, ctx);
    }
}

//---------------------------------------------------------
//    - remove beam segments
//    - detach from system
//    - calculate stem direction and set chord
//---------------------------------------------------------
void BeamLayout::layout1(Beam* item, LayoutContext& ctx)
{
    Beam::LayoutData* ldata = item->mutldata();
    item->resetExplicitParent();  // parent is System

    const StaffType* staffType = item->staffType();
    item->setTab((staffType && staffType->isTabStaff()) ? staffType : nullptr);
    item->setIsBesideTabStaff(item->tab() && !item->tab()->stemless() && !item->tab()->stemThrough());

    // TAB's with stem beside staves have special layout
    if (item->isBesideTabStaff()) {
        item->setUp(!item->tab()->stemsDown());
        item->setSlope(0.0);
        item->setCross(false);
        item->setMinMove(0);
        item->setMaxMove(0);
        for (ChordRest* cr : item->elements()) {
            if (cr->isChord()) {
                item->setUp(cr->up());
                break;
            }
        }
        return;
    }

    if (item->staff()->isDrumStaff(item->tick())) {
        if (item->direction() != DirectionV::AUTO) {
            item->setUp(item->direction() == DirectionV::UP);
        } else if (item->isGrace()) {
            item->setUp(true);
        } else {
            // search through the beam for the first chord with explicit stem direction and use that.
            // if there is no explicit stem direction, default to the direction of the first stem.
            bool firstUp = false;
            bool firstChord = true;
            for (ChordRest* cr : item->elements()) {
                if (!cr->isChord()) {
                    item->setUp(firstUp);
                    continue;
                }
                Chord* chord = toChord(cr);
                DirectionV crDirection = toChord(cr)->stemDirection();
                if (crDirection != DirectionV::AUTO) {
                    item->setUp(crDirection == DirectionV::UP);
                    break;
                }
                if (firstChord) {
                    const Staff* staff = item->staff();
                    const Part* part = staff ? staff->part() : nullptr;
                    const Drumset* ds = part ? part->instrument(item->tick())->drumset() : nullptr;
                    firstUp = ds ? ds->stemDirection(chord->upNote()->pitch()) == DirectionV::UP : chord->up();
                    firstChord = false;
                }
                item->setUp(firstUp);
            }
        }
        for (ChordRest* cr : item->elements()) {
            ChordLayout::computeUp(cr, ctx);
            if (cr->isChord()) {
                ChordLayout::layoutStem(toChord(cr), ctx);
            }
        }
        return;
    }

    item->setMinMove(std::numeric_limits<int>::max());
    item->setMaxMove(std::numeric_limits<int>::min());
    double mag = 0.0;

    item->notePositions().clear();
    staff_idx_t staffIdx = muse::nidx;
    for (ChordRest* cr : item->elements()) {
        double m = cr->isSmall() ? ctx.conf().styleD(Sid::smallNoteMag) : 1.0;
        mag = std::max(mag, m);
        if (cr->isChord()) {
            Chord* chord = toChord(cr);
            staffIdx = chord->vStaffIdx();
            int i = chord->staffMove();
            item->setMinMove(std::min(item->minCRMove(), i));
            item->setMaxMove(std::max(item->maxCRMove(), i));

            for (int distance : chord->noteDistances()) {
                item->notePositions().push_back(BeamBase::NotePosition(distance, cr->vStaffIdx()));
            }
        }
    }

    std::sort(item->notePositions().begin(), item->notePositions().end());
    ldata->setMag(mag);

    item->setCross(item->minCRMove() != item->maxCRMove());

    //
    // determine beam stem direction
    //
    if (item->elements().empty()) {
        return;
    }
    ChordRest* firstNote = item->elements().front();
    Measure* measure = firstNote->measure();
    bool hasMultipleVoices = measure->hasVoices(firstNote->staffIdx(), item->tick(), item->ticks());
    if (computeUpForMovedCross(item)) {
    } else if (item->direction() != DirectionV::AUTO) {
        item->setUp(item->direction() == DirectionV::UP);
    } else if (item->maxCRMove() > 0) {
        item->setUp(false);
    } else if (item->minCRMove() < 0) {
        item->setUp(true);
    } else if (item->isGrace()) {
        if (hasMultipleVoices) {
            item->setUp(firstNote->track() % 2 == 0);
        } else {
            item->setUp(true);
        }
    } else if (item->notePositions().size()) {
        if (hasMultipleVoices) {
            item->setUp(firstNote->track() % 2 == 0);
        } else {
            if (const Chord* chord = item->findChordWithCustomStemDirection()) {
                item->setUp(chord->stemDirection() == DirectionV::UP);
            } else {
                std::set<BeamBase::NotePosition> notePosSet(item->notePositions().begin(), item->notePositions().end());
                std::vector<int> noteLines;
                noteLines.reserve(notePosSet.size());
                for (BeamBase::NotePosition pos : notePosSet) {
                    noteLines.push_back(pos.line);
                }
                item->setUp(ChordLayout::computeAutoStemDirection(noteLines) > 0);
            }
        }
    } else {
        item->setUp(true);
    }

    int middleStaffLine = firstNote->staffType()->middleLine();
    for (size_t i = 0; i < item->notePositions().size(); i++) {
        item->notePositions()[i].line += middleStaffLine;
    }

    bool isEntirelyMoved = false;
    if (item->minCRMove() == item->maxCRMove() && item->minCRMove() != 0) {
        isEntirelyMoved = true;
        item->setStaffIdx(staffIdx);
        if (item->direction() == DirectionV::AUTO) {
            item->setUp(item->maxCRMove() > 0);
        }
    } else if (item->elements().size()) {
        item->setStaffIdx(item->elements().at(0)->staffIdx());
    }
    item->setFullCross(isEntirelyMoved);

    item->setSlope(0.0);

    for (ChordRest* cr : item->elements()) {
        bool prevUp = cr->up();
        ChordLayout::computeUp(cr, ctx);
        if (cr->isChord() && cr->up() != prevUp) {
            ChordLayout::layoutChords1(ctx, cr->segment(), cr->staffIdx());
            cr->segment()->createShape(cr->staffIdx());
            if (toChord(cr)->isGrace()) {
                ChordLayout::layoutStem(toChord(cr), ctx);
            }
        }
    }
}

void BeamLayout::layout2(Beam* item, const LayoutContext& ctx, const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag)
{
    BeamTremoloLayout::setupLData(item, item->mutldata(), ctx);
    Chord* startChord = nullptr;
    Chord* endChord = nullptr;
    if (chordRests.empty()) {
        return;
    }
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord()) {
            if (!startChord) {
                startChord = toChord(chordRest);
                endChord = startChord;
            } else {
                endChord = toChord(chordRest);
            }
            ChordLayout::layoutStem(toChord(chordRest), ctx);
        }
    }
    if (!startChord) {
        // we were passed a vector of only rests. we don't support beams across only rests
        // this beam will be deleted in LayoutBeams
        return;
    }

    item->setBeamSpacing(ctx.conf().styleB(Sid::useWideBeams) ? 4 : 3);
    item->setBeamDist((item->beamSpacing() / 4.0) * item->spatium() * item->mag());
    item->setBeamWidth(item->absoluteFromSpatium(ctx.conf().styleS(Sid::beamWidth)) * item->mag());

    item->setStartAnchor(BeamTremoloLayout::chordBeamAnchor(item->ldata(), startChord, ChordBeamAnchorType::Start));
    item->setEndAnchor(BeamTremoloLayout::chordBeamAnchor(item->ldata(), endChord, ChordBeamAnchorType::End));

    if (item->isGrace()) {
        item->setBeamDist(item->beamDist() * ctx.conf().styleD(Sid::graceNoteMag));
        item->setBeamWidth(item->beamWidth() * ctx.conf().styleD(Sid::graceNoteMag));
    }

    int fragmentIndex = item->directionIdx();
    if (item->userModified()) {
        BeamTremoloLayout::setupLData(item, item->mutldata(), ctx);
        double startY = item->beamFragments()[frag]->py1[fragmentIndex];
        double endY = item->beamFragments()[frag]->py2[fragmentIndex];
        if (ctx.conf().styleB(Sid::snapCustomBeamsToGrid)) {
            const double quarterSpace = item->spatium() / 4;
            startY = round(startY / quarterSpace) * quarterSpace;
            endY = round(endY / quarterSpace) * quarterSpace;
        }
        startY += item->pagePos().y();
        endY += item->pagePos().y();
        item->startAnchor().setY(startY);
        item->endAnchor().setY(endY);
        item->mutldata()->setAnchors(item->startAnchor(), item->endAnchor());
        item->computeAndSetSlope();
        createBeamSegments(item, ctx, chordRests);
        BeamLayout::setTremAnchors(item, ctx);
        return;
    }

    // anchor represents the middle of the beam, not the tip of the stem
    // location depends on _isBesideTabStaff

    if (!item->isBesideTabStaff()) {
        BeamTremoloLayout::setupLData(item, item->mutldata(), ctx);
        BeamTremoloLayout::calculateAnchors(item, item->mutldata(), ctx, chordRests, item->notePositions());
        item->setStartAnchor(item->ldata()->startAnchor);
        item->setEndAnchor(item->ldata()->endAnchor);
        item->computeAndSetSlope();
        item->setBeamDist(item->ldata()->beamDist);
    } else {
        item->setSlope(0.0);
        Chord* startChord2 = nullptr;
        for (ChordRest* cr : chordRests) {
            if (cr->isChord()) {
                startChord2 = toChord(cr);
                break;
            }
        }
        BeamTremoloLayout::setupLData(item, item->mutldata(), ctx);
        double x1 = BeamTremoloLayout::chordBeamAnchorX(item->ldata(), chordRests.front(), ChordBeamAnchorType::Start);
        double x2 = BeamTremoloLayout::chordBeamAnchorX(item->ldata(), chordRests.back(), ChordBeamAnchorType::End);
        double y = BeamTremoloLayout::chordBeamAnchorY(item->ldata(), startChord2);
        item->startAnchor() = PointF(x1, y);
        item->endAnchor() = PointF(x2, y);
        item->mutldata()->setAnchors(item->startAnchor(), item->endAnchor());
    }

    item->beamFragments()[frag]->py1[fragmentIndex] = item->startAnchor().y() - item->pagePos().y();
    item->beamFragments()[frag]->py2[fragmentIndex] = item->endAnchor().y() - item->pagePos().y();

    createBeamSegments(item, ctx, chordRests);
    setTremAnchors(item, ctx);
}

//---------------------------------------------------------
//   isTopBeam
//    returns true for the first CR of a beam that is not cross-staff
//---------------------------------------------------------

bool BeamLayout::isTopBeam(ChordRest* cr)
{
    Beam* b = cr->beam();
    if (b && b->elements().front() == cr) {
        // beam already considered cross?
        if (b->cross() || b->fullCross()) {
            return false;
        }

        // for beams not already considered cross,
        // consider them so here if any elements were moved up
        for (ChordRest* cr1 : b->elements()) {
            // some element moved up?
            if (cr1->staffMove() != 0) {
                return false;
            }
        }

        // not cross
        return true;
    }

    // no beam or not first element
    return false;
}

//---------------------------------------------------------
//   notTopBeam
//    returns true for the first CR of a beam that is cross-staff
//---------------------------------------------------------

bool BeamLayout::notTopBeam(ChordRest* cr)
{
    Beam* b = cr->beam();
    if (b && b->elements().front() == cr) {
        // beam already considered cross?
        if (b->cross() || b->fullCross()) {
            return true;
        }

        // for beams not already considered cross,
        // consider them so here if any elements were moved up
        for (ChordRest* cr1 : b->elements()) {
            // some element moved up?
            if (cr1->staffMove() != 0) {
                return true;
            }
        }

        // not cross
        return false;
    }

    // no beam or not first element
    return false;
}

//---------------------------------------------------------
//   restoreBeams
//---------------------------------------------------------

void BeamLayout::restoreBeams(Measure* m, LayoutContext& ctx)
{
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->elist()) {
            if (e && e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                Beam* b = cr->beam();
                if (b && !b->elements().empty() && b->elements().front() == cr) {
                    TLayout::layoutBeam(b, ctx);
                    b->addSkyline(m->system()->staff(b->staffIdx())->skyline());
                }
            }
        }
    }
}

bool BeamLayout::measureMayHaveBeamsJoinedIntoNext(const Measure* measure)
{
    const MeasureBase* next = measure->next();
    if (!(next && next->isMeasure())) {
        return false;
    }

    const Measure* nextMeasure = toMeasure(next);
    const Segment* firstCrSeg = nextMeasure->findFirstR(SegmentType::ChordRest, Fraction(0, 1));
    if (!firstCrSeg) {
        return false;
    }

    for (const EngravingItem* item : firstCrSeg->elist()) {
        if (!item) {
            continue;
        }
        BeamMode beamMode = toChordRest(item)->beamMode();
        if (beamMode == BeamMode::MID || beamMode == BeamMode::BEGIN16 || beamMode == BeamMode::BEGIN32) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   breakCrossMeasureBeams
//---------------------------------------------------------

void BeamLayout::breakCrossMeasureBeams(Measure* measure, LayoutContext& ctx)
{
    MeasureBase* mbNext = measure->next();
    if (!mbNext || !mbNext->isMeasure()) {
        return;
    }

    Measure* next = toMeasure(mbNext);
    const size_t ntracks = ctx.dom().ntracks();
    Segment* fstSeg = next->first(SegmentType::ChordRest);
    if (!fstSeg) {
        return;
    }

    for (size_t track = 0; track < ntracks; ++track) {
        const Staff* stf = ctx.dom().staff(track2staff(track));

        // don’t compute beams for invisible staves and tablature without stems
        if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->stemless())) {
            continue;
        }

        EngravingItem* e = fstSeg->element(static_cast<int>(track));
        if (!e || !e->isChordRest()) {
            continue;
        }

        ChordRest* cr = toChordRest(e);
        Beam* beam = cr->beam();
        if (!beam || beam->elements().front()->measure() == next) {   // no beam or not cross-measure beam
            continue;
        }

        std::vector<ChordRest*> mElements;
        std::vector<ChordRest*> nextElements;

        for (ChordRest* beamCR : beam->elements()) {
            if (beamCR->tick() < next->tick()) {
                mElements.push_back(beamCR);
            } else {
                nextElements.push_back(beamCR);
            }
        }

        if (mElements.size() == 1) {
            mElements[0]->removeDeleteBeam(false);
        }

        Beam* newBeam = nullptr;
        if (nextElements.size() > 1) {
            newBeam = Factory::createBeam(ctx.mutDom().dummyParent()->system());
            newBeam->setGenerated(true);
            newBeam->setTrack(track);
        }

        const bool nextBeamed = bool(newBeam);
        for (ChordRest* nextCR : nextElements) {
            nextCR->removeDeleteBeam(nextBeamed);
            if (newBeam) {
                newBeam->add(nextCR);
            }
        }

        if (newBeam) {
            layout1(newBeam, ctx);
        }
    }
}

static bool beamNoContinue(BeamMode mode)
{
    return mode == BeamMode::END || mode == BeamMode::NONE || mode == BeamMode::INVALID;
}

#define beamModeMid(a) (a == BeamMode::MID || a == BeamMode::BEGIN16 || a == BeamMode::BEGIN32)

//---------------------------------------------------------
//   beamGraceNotes
//---------------------------------------------------------

void BeamLayout::beamGraceNotes(LayoutContext& ctx, Chord* mainNote, bool after)
{
    ChordRest* a1    = 0;        // start of (potential) beam
    Beam* beam       = 0;        // current beam
    BeamMode bm = BeamMode::AUTO;
    std::vector<Chord*> graceNotes = after ? mainNote->graceNotesAfter() : mainNote->graceNotesBefore();

    if (beam) {
        beam->setIsGrace(true);
    }

    for (ChordRest* cr : graceNotes) {
        bm = Groups::endBeam(cr);
        if ((cr->durationType().type() <= DurationType::V_QUARTER) || (bm == BeamMode::NONE)) {
            if (beam) {
                beam->setIsGrace(true);
                layout1(beam, ctx);
                beam = 0;
            }
            if (a1) {
                a1->removeDeleteBeam(false);
                a1 = 0;
            }
            cr->removeDeleteBeam(false);
            continue;
        }
        if (beam) {
            bool beamEnd = bm == BeamMode::BEGIN;
            if (!beamEnd) {
                cr->replaceBeam(beam);
                cr = 0;
                beamEnd = (bm == BeamMode::END);
            }
            if (beamEnd) {
                beam->setIsGrace(true);
                layout1(beam, ctx);
                beam = 0;
            }
        }
        if (!cr) {
            continue;
        }
        if (a1 == 0) {
            a1 = cr;
        } else {
            if (!beamModeMid(bm) && (bm == BeamMode::BEGIN)) {
                a1->removeDeleteBeam(false);
                a1 = cr;
            } else {
                beam = a1->beam();
                if (beam == 0 || beam->elements().front() != a1) {
                    beam = Factory::createBeam(ctx.mutDom().dummyParent()->system());
                    beam->setGenerated(true);
                    beam->setTrack(mainNote->track());
                    a1->replaceBeam(beam);
                }
                cr->replaceBeam(beam);
                a1 = 0;
            }
        }
    }
    if (beam) {
        beam->setIsGrace(true);
        layout1(beam, ctx);
    } else if (a1) {
        a1->removeDeleteBeam(false);
    }
}

void BeamLayout::createBeams(LayoutContext& ctx, Measure* measure)
{
    for (track_idx_t track = 0; track < ctx.dom().ntracks(); ++track) {
        const Staff* stf = ctx.dom().staff(track2staff(track));

        // don’t compute beams for invisible staves and tablature without stems
        if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->stemless())) {
            continue;
        }

        ChordRest* a1    = nullptr;          // start of (potential) beam
        bool firstCR     = true;
        Beam* beam       = nullptr;          // current beam
        BeamMode bm    = BeamMode::AUTO;
        ChordRest* prev  = nullptr;
        bool checkBeats  = false;
        Fraction stretch = Fraction(1, 1);
        std::unordered_map<int, TDuration> beatSubdivision;

        // if this measure is simple meter (actually X/4),
        // then perform a prepass to determine the subdivision of each beat

        beatSubdivision.clear();
        TimeSig* ts = stf->timeSig(measure->tick());
        checkBeats  = false;
        stretch     = ts ? ts->stretch() : Fraction(1, 1);

        const SegmentType st = SegmentType::ChordRest;
        if (ts && ts->denominator() == 4) {
            checkBeats = true;
            for (Segment* s = measure->first(st); s; s = s->next(st)) {
                ChordRest* mcr = toChordRest(s->element(track));
                if (mcr == 0) {
                    continue;
                }
                int beat = (mcr->rtick() * stretch).ticks() / Constants::DIVISION;
                if (muse::contains(beatSubdivision, beat)) {
                    beatSubdivision[beat] = std::min(beatSubdivision[beat], mcr->durationType());
                } else {
                    beatSubdivision[beat] = mcr->durationType();
                }
            }
        }

        for (Segment* segment = measure->first(st); segment; segment = segment->next(st)) {
            ChordRest* cr = segment->cr(track);
            if (!cr) {
                continue;
            }

            if (cr->isChord()) {
                Chord* chord = toChord(cr);
                for (Chord* c : chord->graceNotes()) {
                    c->setBeamlet(nullptr); // Will be defined during beam layout
                }
            }
            cr->setBeamlet(nullptr); // Will be defined during beam layout

            if (firstCR) {
                firstCR = false;
                // Handle cross-measure beams
                BeamMode mode = cr->beamMode();
                if (mode == BeamMode::MID || mode == BeamMode::END || mode == BeamMode::BEGIN16 || mode == BeamMode::BEGIN32) {
                    ChordRest* prevCR = ctx.mutDom().findCR(measure->tick() - Fraction::fromTicks(1), track);
                    if (prevCR) {
                        Beam* prevBeam = prevCR->beam();
                        const Measure* pm = prevCR->measure();
                        if (!beamNoContinue(prevCR->beamMode())
                            && !pm->lineBreak() && !pm->pageBreak() && !pm->sectionBreak()
                            && ctx.state().prevMeasure()
                            && !(prevCR->isChord() && prevCR->durationType().type() <= DurationType::V_QUARTER)) {
                            beam = prevBeam;
                            if (prevCR->isChord()) {
                                if (Hook* hook = toChord(prevCR)->hook()) {
                                    ctx.mutDom().doUndoRemoveElement(hook);
                                }
                            }
                            //a1 = beam ? beam->elements().front() : prevCR;
                            a1 = beam ? nullptr : prevCR;               // when beam is found, a1 is no longer required.
                        } else if (prevBeam && prevBeam == cr->beam() && prevBeam->elements().front() == prevCR) {
                            // remove the beam from the previous chordrest because we do not currently
                            // support cross-system beams
                            prevCR->removeDeleteBeam(false);
                        }
                    }
                }
            }

            bool hasAllRest = cr->beam() && cr->beam()->hasAllRests();
            bool isARestBeamStart = cr->isRest() && cr->beamMode() == BeamMode::BEGIN;
            if (hasAllRest && !isARestBeamStart) {
                cr->removeDeleteBeam(false);
            }

            // handle grace notes
            // (tied chords?)
            if (cr->isChord()) {
                Chord* chord = toChord(cr);
                beamGraceNotes(ctx, chord, false);         // grace before
                beamGraceNotes(ctx, chord, true);          // grace after
            }

            if (cr->isRest() && cr->beamMode() == BeamMode::AUTO) {
                bm = BeamMode::NONE;                   // do not beam rests set to BeamMode::AUTO or with only other rests
            } else {
                bm = Groups::endBeam(cr, prev);          // get defaults from time signature properties
            }
            // perform additional context-dependent checks
            if (bm == BeamMode::AUTO) {
                // check if we need to break beams according to minimum duration in current / previous beat
                if (checkBeats && cr->rtick().isNotZero()) {
                    Fraction tick = cr->rtick() * stretch;
                    // check if on the beat
                    if ((tick.ticks() % Constants::DIVISION) == 0) {
                        int beat = tick.ticks() / Constants::DIVISION;
                        // get minimum duration for this & previous beat
                        TDuration minDuration = std::min(beatSubdivision[beat], beatSubdivision[beat - 1]);
                        // re-calculate beam as if this were the duration of current chordrest
                        TDuration saveDuration        = cr->actualDurationType();
                        TDuration saveCMDuration      = cr->crossMeasureDurationType();
                        CrossMeasure saveCrossMeasVal = cr->crossMeasure();
                        cr->setDurationType(minDuration);
                        bm = Groups::endBeam(cr, prev);
                        cr->setDurationType(saveDuration);
                        cr->setCrossMeasure(saveCrossMeasVal);
                        cr->setCrossMeasureDurationType(saveCMDuration);
                    }
                }
            }

            prev = cr;

            // if chord has hooks and is 2nd element of a cross-measure value
            // set beam mode to NONE (do not combine with following chord beam/hook, if any)
            TDuration durationType = cr->durationType();
            if (durationType.hooks() > 0 && cr->crossMeasure() == CrossMeasure::SECOND) {
                bm = BeamMode::NONE;
            }

            // Rests of any duration can be beamed over, if required
            bool canBeBeamed = durationType.type() > DurationType::V_QUARTER || cr->isRest();
            if (!canBeBeamed || (bm == BeamMode::NONE)) {
                bool removeBeam = true;
                if (beam) {
                    layout1(beam, ctx);
                    removeBeam = (beam->elements().size() <= 1 || beam->hasAllRests());
                    beam = 0;
                }
                if (a1) {
                    if (removeBeam) {
                        a1->removeDeleteBeam(false);
                    }
                    a1 = 0;
                }
                cr->removeDeleteBeam(false);
                continue;
            }

            if (beam) {
                bool beamEnd = (bm == BeamMode::BEGIN);
                if (!beamEnd) {
                    cr->replaceBeam(beam);
                    cr = 0;
                    beamEnd = (bm == BeamMode::END);
                }
                if (beamEnd) {
                    layout1(beam, ctx);
                    beam = 0;
                }
            }
            if (!cr) {
                continue;
            }

            if (a1 == 0) {
                a1 = cr;
            } else {
                if (!beamModeMid(bm)
                    &&
                    (bm == BeamMode::BEGIN
                     || (a1->segment()->segmentType() != cr->segment()->segmentType())
                     || (a1->tick() + a1->actualTicks() < cr->tick())
                    )
                    ) {
                    a1->removeDeleteBeam(false);
                    a1 = cr;
                } else {
                    beam = a1->beam();
                    if (beam == 0 || beam->elements().front() != a1) {
                        beam = Factory::createBeam(ctx.mutDom().dummyParent()->system());
                        beam->setGenerated(true);
                        beam->setTrack(track);
                        a1->replaceBeam(beam);
                    }
                    cr->replaceBeam(beam);
                    a1 = 0;
                }
            }
        }
        if (beam) {
            layout1(beam, ctx);
        } else if (a1) {
            Fraction nextTick = a1->tick() + a1->actualTicks();
            Measure* m = (nextTick >= measure->endTick() ? measure->nextMeasure() : measure);
            ChordRest* nextCR = (m ? m->findChordRest(nextTick, track) : nullptr);
            Beam* b = a1->beam();
            if (!(b && !b->elements().empty() && !b->hasAllRests() && b->elements().front() == a1 && nextCR
                  && beamModeMid(nextCR->beamMode()))) {
                a1->removeDeleteBeam(false);
            }
        }
    }
}

/************************************************************
 * layoutNonCrossBeams()
 * layout all non-cross-staff beams starting on this segment
 * **********************************************************/

void BeamLayout::layoutNonCrossBeams(Segment* s, LayoutContext& ctx)
{
    for (EngravingItem* e : s->elist()) {
        if (!e || !e->isChordRest() || !ctx.dom().staff(e->staffIdx())->show()) {
            // the beam and its system may still be referenced when selecting all,
            // even if the staff is invisible. The old system is invalid and does cause problems in #284012
            if (e && e->isChordRest() && !ctx.dom().staff(e->staffIdx())->show() && toChordRest(e)->beam()) {
                toChordRest(e)->beam()->resetExplicitParent();
            }
            continue;
        }
        ChordRest* cr = toChordRest(e);
        // layout beam
        if (BeamLayout::isTopBeam(cr)) {
            TLayout::layoutBeam(cr->beam(), ctx);
            if (!cr->beam()->tremAnchors().empty()) {
                // there are inset tremolos in here
                for (ChordRest* beamCr : cr->beam()->elements()) {
                    if (!beamCr->isChord()) {
                        continue;
                    }
                    Chord* c = toChord(beamCr);
                    if (c->tremoloTwoChord()) {
                        TremoloLayout::layout(c->tremoloTwoChord(), ctx);
                    }
                }
            }
        }
        if (!cr->isChord()) {
            continue;
        }
        for (Chord* grace : toChord(cr)->graceNotes()) {
            if (BeamLayout::isTopBeam(grace)) {
                TLayout::layoutBeam(grace->beam(), ctx);
            }
        }
    }
}

void BeamLayout::verticalAdjustBeamedRests(Rest* rest, Beam* beam, LayoutContext& ctx)
{
    const double spatium = rest->spatium();
    static constexpr Fraction rest32nd(1, 32);
    const bool up = beam->up();
    const BeamMode restBeamMode = rest->beamMode();
    const bool firstRest = beam->elements().front() == rest
                           && (restBeamMode == BeamMode::BEGIN || restBeamMode == BeamMode::BEGIN16 || restBeamMode == BeamMode::BEGIN32
                               || restBeamMode == BeamMode::MID);

    double restToBeamPadding;
    if (rest->ticks() <= rest32nd) {
        restToBeamPadding = 0.2 * spatium;
    } else {
        restToBeamPadding = 0.35 * spatium;
    }

    const Shape beamShape = beam->shape().translate(beam->pagePos());
    const Shape restShape = rest->shape().translate(rest->pagePos() - rest->offset());
    const double minBeamToRestXDist = up && firstRest ? 0.1 * spatium : 0.0;

    const double restToBeamClearance = up
                                       ? beamShape.verticalClearance(restShape, minBeamToRestXDist)
                                       : restShape.verticalClearance(beamShape);

    if (restToBeamClearance > restToBeamPadding) {
        return;
    }

    if (up) {
        rest->verticalClearance().setAbove(restToBeamClearance);
    } else {
        rest->verticalClearance().setBelow(restToBeamClearance);
    }

    const bool restIsLocked = rest->verticalClearance().locked();
    if (!restIsLocked) {
        double overlap = (restToBeamPadding - restToBeamClearance);
        double lineDistance = rest->staff()->lineDistance(rest->tick()) * spatium;
        int lineMoves = ceil(overlap / lineDistance);
        lineMoves *= up ? 1 : -1;
        double yMove = lineMoves * lineDistance;
        rest->mutldata()->moveY(yMove);
        for (Rest* mergedRest : rest->ldata()->mergedRests) {
            mergedRest->mutldata()->moveY(yMove);
        }

        Segment* segment = rest->segment();
        staff_idx_t staffIdx = rest->vStaffIdx();
        const Staff* staff = ctx.dom().staff(staffIdx);
        std::vector<Chord*> chords;
        std::vector<Rest*> rests;
        collectChordsAndRest(segment, staffIdx, chords, rests);
        ChordLayout::resolveRestVSChord(rests, chords, staff, segment);
        ChordLayout::resolveRestVSRest(rests, staff, segment, ctx, /*considerBeams*/ true);
    }

    TLayout::layoutBeam(beam, ctx);
}

void BeamLayout::checkCrossPosAndStemConsistency(Beam* beam, LayoutContext& ctx)
{
    if (!beam->cross() || !beam->userModified()) {
        return;
    }

    int correctCrossStaffIdx = 0;
    bool inconsistencyFound = false;

    for (ChordRest* cr : beam->elements()) {
        if (!cr->isChord()) {
            continue;
        }
        Chord* chord = toChord(cr);
        bool currentUp = chord->up();
        bool actualUp = ChordLayout::isChordPosBelowBeam(chord, beam);
        if (actualUp != currentUp) {
            inconsistencyFound = true;
            chord->setUp(actualUp);
            ChordLayout::layoutChords1(ctx, chord->segment(), chord->staffIdx());
            chord->segment()->createShape(chord->staffIdx());
        }
        if (actualUp) {
            correctCrossStaffIdx = std::min(correctCrossStaffIdx, chord->staffMove());
        } else {
            correctCrossStaffIdx = std::max(correctCrossStaffIdx, chord->staffMove() + 1);
        }
    }

    int beamCrossStaffIdx = beam->crossStaffIdx();
    if (beamCrossStaffIdx != correctCrossStaffIdx) {
        inconsistencyFound = true;
        int curCrossStaffMove = beam->crossStaffMove();
        int diff = correctCrossStaffIdx - beamCrossStaffIdx;
        beam->setProperty(Pid::BEAM_CROSS_STAFF_MOVE, curCrossStaffMove + diff);
    }

    if (inconsistencyFound) {
        BeamLayout::layout(beam, ctx);
    }
}

void BeamLayout::createBeamSegments(Beam* item, const LayoutContext& ctx, const std::vector<ChordRest*>& chordRests)
{
    item->clearBeamSegments();

    bool levelHasBeam = false;
    int level = 0;
    constexpr size_t noLastChord = std::numeric_limits<size_t>::max();
    size_t numCr = chordRests.size();
    bool frenchStyleBeams = ctx.conf().styleB(Sid::frenchStyleBeams);
    do {
        levelHasBeam = false;
        ChordRest* startCr = nullptr;
        ChordRest* endCr = nullptr;
        bool breakBeam = false;
        bool previousBreak16 = false;
        bool previousBreak32 = false;
        size_t lastChordIndex = noLastChord;

        for (size_t i = 0; i < numCr; i++) {
            ChordRest* chordRest = chordRests[i];
            ChordRest* prevChordRest = i < 1 ? nullptr : chordRests[i - 1];

            if (level < chordRest->beams() && !chordRest->isRest()) {
                levelHasBeam = true;
            }
            bool isBroken16 = false;
            bool isBroken32 = false;

            // updates isBroken16 and isBroken32
            item->calcBeamBreaks(chordRest, prevChordRest, level, isBroken16, isBroken32);
            breakBeam = isBroken16 || isBroken32;
            bool skipRest = false;
            BeamMode beamMode = chordRest->beamMode();
            if (chordRest->isRest() && level >= chordRest->beams()
                && (beamMode == BeamMode::MID || beamMode == BeamMode::BEGIN16 || beamMode == BeamMode::BEGIN32)) {
                // if the level is lower than both the previous and next real chords, don't break here
                size_t nextChordIndex = i + 1;
                while (nextChordIndex < numCr && chordRests[nextChordIndex]->isRest()) {
                    ++nextChordIndex;
                }
                if (lastChordIndex < numCr && nextChordIndex < numCr) {
                    Chord* lastChord = toChord(chordRests[lastChordIndex]);
                    Chord* nextChord = toChord(chordRests[nextChordIndex]);
                    // we have both chords (not rests) on either side of this rest, so if the level is lower than both we can
                    // just continue the beam through here
                    // this creates situations where we can have the second beam of a pair of 16th notes continuing through
                    // an 8th rest, for example, even though an 8th note would very obviously break the beam
                    // this is desired for BeamType::MID on rests.
                    if (level < lastChord->beams() && level < nextChord->beams()) {
                        skipRest = (isBroken16 && level < 1) || (isBroken32 && level < 2) || (!isBroken16 && !isBroken32);
                    }
                }
            }
            if ((level < chordRest->beams() && !breakBeam) || skipRest) {
                endCr = chordRest;
                if (!startCr || (startCr->isRest() && startCr != item->elements().front())) {
                    startCr = chordRest;
                }
            } else if (level >= chordRest->beams() && chordRest->isRest() && !breakBeam) {
                // This rest has duration longer than the beam value, but it may still go
                // under the beam if the next chords or rests are beamed
                continue;
            } else {
                size_t beamletIndex = static_cast<size_t>(i) - 1;
                if (lastChordIndex < item->elements().size() && (chordRest->isRest() || (endCr && endCr->isRest()))) {
                    // we broke the beam on this chordrest, but the last cr of the beam segment can't end on a rest
                    // so it ends on lastChord
                    ChordRest* lastCr = toChordRest(item->elements()[lastChordIndex]);
                    if (lastCr && startCr && lastCr->tick() >= startCr->tick()) {
                        endCr = lastCr;
                        beamletIndex = lastChordIndex;
                        lastChordIndex = noLastChord;
                    }
                }
                if (startCr && endCr && levelHasBeam) {
                    if (startCr == endCr && startCr->isChord()) {
                        bool isBeamletBefore = calcIsBeamletBefore(item,
                                                                   toChord(startCr),
                                                                   static_cast<int>(beamletIndex),
                                                                   level,
                                                                   previousBreak16,
                                                                   previousBreak32);
                        createBeamletSegment(item, ctx, toChord(startCr), isBeamletBefore, level);
                    } else if (startCr != endCr) {
                        createBeamSegment(item, startCr, endCr, level, frenchStyleBeams);
                    }
                }
                bool setCr = chordRest && chordRest->isChord() && breakBeam && level < chordRest->beams();
                startCr = setCr ? chordRest : nullptr;
                endCr = setCr ? chordRest : nullptr;
            }
            previousBreak16 = isBroken16;
            previousBreak32 = isBroken32;
            if (chordRest && chordRest->isChord()) {
                lastChordIndex = i;
            }
        }

        // if the beam ends on the last chord
        if (startCr && (endCr || breakBeam)) {
            if ((startCr == endCr || !endCr) && startCr->isChord()) {
                // this chord is either the last chord, or the first (followed by a rest)
                bool isBefore = !(startCr == chordRests.front());
                createBeamletSegment(item, ctx, toChord(startCr), isBefore, level);
            } else {
                createBeamSegment(item, startCr, endCr, level, frenchStyleBeams);
            }
        }
        level++;
    } while (levelHasBeam);
}

bool BeamLayout::calcIsBeamletBefore(const Beam* item, Chord* chord, int i, int level, bool isAfter16Break, bool isAfter32Break)
{
    // if first or last chord in beam group
    if (i == 0) {
        return false;
    } else if (i == static_cast<int>(item->elements().size()) - 1) {
        return true;
    }
    // if first or last chord in tuplet
    Tuplet* tuplet = chord->tuplet();
    if (tuplet && chord == tuplet->elements().front()) {
        return false;
    } else if (tuplet && chord == tuplet->elements().back()) {
        return true;
    }

    // next note has a beam break
    ChordRest* nextChordRest = item->elements()[i + 1];
    ChordRest* currChordRest = item->elements()[i];
    ChordRest* prevChordRest = item->elements()[i - 1];
    if (nextChordRest->isChord()) {
        bool nextBreak16 = false;
        bool nextBreak32 = false;
        bool currBreak16 = false;
        bool currBreak32 = false;
        item->calcBeamBreaks(currChordRest, prevChordRest, prevChordRest->beams(), currBreak16, currBreak32);
        item->calcBeamBreaks(nextChordRest, currChordRest, level, nextBreak16, nextBreak32);
        if ((nextBreak16 && level >= 1) || (!currBreak16 && nextBreak32 && level >= 2)) {
            return true;
        }
    }

    // if previous or next chord has more beams, point in that direction
    int previousChordLevel = -1;
    int nextChordLevel = -1;
    int previousOffset = 1;
    while (i - previousOffset >= 0) {
        ChordRest* previous = item->elements()[i - previousOffset];
        if (previous->isChord()) {
            previousChordLevel = toChord(previous)->beams();
            if (isAfter16Break) {
                previousChordLevel = std::min(previousChordLevel, 1);
            } else if (isAfter32Break) {
                previousChordLevel = std::min(previousChordLevel, 2);
            }
            break;
        }
        ++previousOffset;
    }

    int nextOffset = 1;
    while (i + nextOffset < static_cast<int>(item->elements().size())) {
        ChordRest* next = item->elements()[i + nextOffset];
        if (next->isChord()) {
            nextChordLevel = toChord(next)->beams();
            break;
        }
        ++nextOffset;
    }
    int chordLevelDifference = nextChordLevel - previousChordLevel;
    if (chordLevelDifference != 0) {
        return chordLevelDifference < 0;
    }

    // if the chord ends a subdivision of the beat
    Fraction baseTick = tuplet ? tuplet->tick() : chord->measure()->tick();
    Fraction tickNext = nextChordRest->tick() - baseTick;
    if (tuplet) {
        // for tuplets with odd ratios, apply ratio
        // for tuplets with even ratios, use actual beat
        Fraction ratio = tuplet->ratio();
        if (ratio.numerator() & 1) {
            tickNext *= ratio;
        }
    }

    static const int BEAM_TUPLET_TOLERANCE = 6;
    int tickLargeSize  = chord->ticks().ticks() * 2;
    int remainder = tickNext.ticks() % tickLargeSize;
    if (remainder <= BEAM_TUPLET_TOLERANCE || (tickLargeSize - remainder) <= BEAM_TUPLET_TOLERANCE) {
        return true;
    }

    // default case
    return false;
}

static inline int computeStemLengthAdjustmentSteps(const ChordRest* cr, bool isStartOrEnd, bool isAbove, bool frenchStyleBeams,
                                                   int level, int beamsAbove, int beamsBelow)
{
    if (isAbove == cr->up()) { // beam segment on opposite side
        if (!frenchStyleBeams || isStartOrEnd) {
            // lengthen
            return level - (isAbove ? beamsBelow : beamsAbove);
        }
    } else { // beamlet on the note side
        if (frenchStyleBeams && !isStartOrEnd) {
            // shorten
            return (isAbove ? beamsBelow : beamsAbove) - level;
        }
    }

    return 0;
}

void BeamLayout::createBeamSegment(Beam* item, ChordRest* startCr, ChordRest* endCr, int level, bool frenchStyleBeams)
{
    const bool isFirstSubgroup = startCr == item->elements().front();
    const bool isLastSubgroup = endCr == item->elements().back();
    const bool firstUp = startCr->up();
    const bool lastUp = endCr->up();
    bool overallUp = item->up();
    if (isFirstSubgroup == isLastSubgroup) {
        // this subgroup is either the only one in the beam, or in the middle
        if (firstUp == lastUp) {
            // the "outside notes" of this subgroup go the same direction so use them
            // to determine the side of the beams
            overallUp = firstUp;
        } else {
            // no perfect way to solve this problem, for now we'll base it on the number of
            // up and down stemmed notes in this subgroup
            int upStems, downStems;
            upStems = downStems = 0;
            for (ChordRest* cr : item->elements()) {
                if (!cr->isChord() || cr->tick() < startCr->tick()) {
                    continue;
                }
                if (cr->tick() > endCr->tick()) {
                    break;
                }

                ++(toChord(cr)->up() ? upStems : downStems);

                if (cr == endCr) {
                    break;
                }
            }
            if (upStems == downStems) {
                // we are officially bamboozled. for now we can just use the default
                // direction based on the staff we're on I guess
                overallUp = item->up();
            } else {
                // use the direction with the most stems
                overallUp = upStems > downStems;
            }
        }
    } else if (isFirstSubgroup) {
        overallUp = lastUp;
    } else if (isLastSubgroup) {
        overallUp = firstUp;
    }

    const double startX = BeamTremoloLayout::chordBeamAnchorX(item->ldata(), startCr, ChordBeamAnchorType::Start);
    const double endX = BeamTremoloLayout::chordBeamAnchorX(item->ldata(), endCr, ChordBeamAnchorType::End);

    double startY = item->slope() * (startX - item->startAnchor().x()) + item->startAnchor().y() - item->pagePos().y();
    double endY = item->slope() * (endX - item->startAnchor().x()) + item->startAnchor().y() - item->pagePos().y();

    int beamsBelow = 0; // how many beams below level 0?
    int beamsAbove = 0; // how many beams above level 0?

    // avoid adjusting for beams on opposite side of level 0
    if (level != 0) {
        for (const BeamSegment* beam : item->beamSegments()) {
            if (beam->level == 0 || beam->endTick < startCr->tick() || beam->startTick > endCr->tick()) {
                continue;
            }
            ++(beam->above ? beamsAbove : beamsBelow);
        }

        const int upValue = overallUp ? -1 : 1;
        const int extraBeamAdjust = overallUp ? beamsAbove : beamsBelow;
        const double verticalOffset = item->beamDist() * (level - extraBeamAdjust) * upValue;

        if (muse::RealIsEqual(item->growLeft(), item->growRight())) {
            startY -= verticalOffset * item->growLeft();
            endY -= verticalOffset * item->growLeft();
        } else {
            // Feathered beams
            double startProportionAlongX = (startX - item->startAnchor().x()) / (item->endAnchor().x() - item->startAnchor().x());
            double endProportionAlongX = (endX - item->startAnchor().x()) / (item->endAnchor().x() - item->startAnchor().x());

            double grow1 = startProportionAlongX * (item->growRight() - item->growLeft()) + item->growLeft();
            double grow2 = endProportionAlongX * (item->growRight() - item->growLeft()) + item->growLeft();

            startY -= verticalOffset * grow1;
            endY -= verticalOffset * grow2;
        }
    }

    BeamSegment* b = new BeamSegment(item);
    b->above = !overallUp;
    b->level = level;
    b->line = LineF(startX, startY, endX, endY);
    b->startTick = startCr->tick();
    b->endTick = endCr->tick();
    item->beamSegments().push_back(b);

    if (level > 0) {
        ++(b->above ? beamsAbove : beamsBelow);
    }

    // extend stems properly
    for (ChordRest* cr : item->elements()) {
        if (!cr->isChord() || cr->tick() < startCr->tick()) {
            continue;
        }
        if (cr->tick() > endCr->tick()) {
            break;
        }

        Chord* chord = toChord(cr);
        if (!chord->stem()) {
            continue;
        }

        double _up = chord->up() ? -1.0 : 1.0;

        if (level == 0) {
            BeamTremoloLayout::extendStem(item->ldata(), chord, 0.0);
            // HACK: extend the bounding box of the stem (NOT the stem itself) by half beam width.
            // This way the bbox of the stem covers also the beam position. Hugely helps with all the collision checks.
            chord->stem()->mutldata()->beamCorrection = _up * item->beamWidth() / 2;
        } else {
            bool isStartOrEnd = cr == startCr || cr == endCr;
            int visualStemLengthAdjustmentSteps = computeStemLengthAdjustmentSteps(cr, isStartOrEnd, b->above, frenchStyleBeams,
                                                                                   level, beamsAbove, beamsBelow);
            if (visualStemLengthAdjustmentSteps != 0) {
                double grow = item->growLeft();
                if (!muse::RealIsEqual(item->growLeft(), item->growRight())) {
                    double anchorX = BeamTremoloLayout::chordBeamAnchorX(item->ldata(), chord, ChordBeamAnchorType::Middle);
                    double proportionAlongX = (anchorX - item->startAnchor().x()) / (item->endAnchor().x() - item->startAnchor().x());
                    grow = proportionAlongX * (item->growRight() - item->growLeft()) + item->growLeft();
                }

                double visualStemLengthAdjustment = grow * visualStemLengthAdjustmentSteps * item->beamDist();

                if (frenchStyleBeams) {
                    // Even if we visually shorten the stem, we need the 'original' length for collisions.
                    int bboxStemLengthAdjustmentSteps = computeStemLengthAdjustmentSteps(cr, isStartOrEnd, b->above,
                                                                                         /*frenchStyleBeams*/ false,
                                                                                         level, beamsAbove, beamsBelow);
                    chord->stem()->mutldata()->beamCorrection = _up * item->beamWidth() / 2 + _up * grow
                                                                * (bboxStemLengthAdjustmentSteps - visualStemLengthAdjustmentSteps)
                                                                * item->beamDist();
                }
                BeamTremoloLayout::extendStem(item->ldata(), chord, visualStemLengthAdjustment);
            }
        }

        if (cr == endCr) {
            break;
        }
    }
}

void BeamLayout::createBeamletSegment(Beam* item, const LayoutContext& ctx, ChordRest* cr, bool isBefore, int level)
{
    const double startX = BeamTremoloLayout::chordBeamAnchorX(item->ldata(), cr,
                                                              isBefore
                                                              ? ChordBeamAnchorType::End
                                                              : ChordBeamAnchorType::Start);

    const double beamletLength = ctx.conf().styleMM(Sid::beamMinLen).val() * cr->mag();

    const double endX = startX + (isBefore ? -beamletLength : beamletLength);

    double startY = item->slope() * (startX - item->startAnchor().x()) + item->startAnchor().y() - item->pagePos().y();
    double endY = item->slope() * (endX - startX) + startY;

    // how many beams past level 0 (i.e. beams on the other side of level 0 for this subgroup)
    int extraBeamAdjust = 0;

    // avoid adjusting for beams on opposite side of level 0
    for (const BeamSegment* beam : item->beamSegments()) {
        if (beam->level == 0 || beam->line.x2() < startX || beam->line.x1() > endX) {
            continue;
        }

        if (cr->up() == beam->above) {
            extraBeamAdjust++;
        }
    }

    const int upValue = cr->up() ? -1 : 1;
    const double verticalOffset = item->beamDist() * (level - extraBeamAdjust) * upValue;

    if (muse::RealIsEqual(item->growLeft(), item->growRight())) {
        startY -= verticalOffset * item->growLeft();
        endY -= verticalOffset * item->growLeft();
    } else {
        // Feathered beams
        double startProportionAlongX = (startX - item->startAnchor().x()) / (item->endAnchor().x() - item->startAnchor().x());
        double endProportionAlongX = (endX - item->startAnchor().x()) / (item->endAnchor().x() - item->startAnchor().x());

        double grow1 = startProportionAlongX * (item->growRight() - item->growLeft()) + item->growLeft();
        double grow2 = endProportionAlongX * (item->growRight() - item->growLeft()) + item->growLeft();

        startY -= verticalOffset * grow1;
        endY -= verticalOffset * grow2;
    }

    BeamSegment* b = new BeamSegment(item);
    b->above = !cr->up();
    b->level = level;
    b->line = LineF(startX, startY, endX, endY);
    b->isBeamlet = true;
    b->isBefore = isBefore;
    cr->setBeamlet(b);

    ChordLayout::fillShape(cr, cr->mutldata(), ctx.conf());
    //! NOTE Moved from ChordRest::setBeamlet
    cr->segment()->createShape(cr->vStaffIdx());
    item->beamSegments().push_back(b);
}

bool BeamLayout::computeUpForMovedCross(Beam* item)
{
    if (!item->cross()) {
        return false;
    }
    bool isAboveAllChords = true;
    bool isBelowAllChords = true;
    for (ChordRest* cr : item->elements()) {
        if (cr->isBelowCrossBeam(item)) {
            isBelowAllChords = false;
        } else {
            isAboveAllChords = false;
        }
    }

    if (isAboveAllChords) {
        item->setUp(true);
        return true;
    }

    if (isBelowAllChords) {
        item->setUp(false);
        return true;
    }

    return false;
}

PointF BeamLayout::chordBeamAnchor(const Beam* item, const ChordRest* chord, ChordBeamAnchorType anchorType)
{
    return BeamTremoloLayout::chordBeamAnchor(item->ldata(), chord, anchorType);
}

double BeamLayout::chordBeamAnchorY(const Beam* item, const ChordRest* chord)
{
    return BeamTremoloLayout::chordBeamAnchorY(item->ldata(), chord);
}

void BeamLayout::setTremAnchors(Beam* item, const LayoutContext& ctx)
{
    item->tremAnchors().clear();
    for (ChordRest* cr : item->elements()) {
        if (!cr || !cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);
        TremoloTwoChord* t = c ? c->tremoloTwoChord() : nullptr;
        if (t && t->chord1() == c && t->chord2()->beam() == item) {
            // there is an inset tremolo here!
            // figure out up / down
            bool tremUp = t->up();
            if (item->userModified()) {
                tremUp = c->up();
            } else if (item->cross() && t->chord1()->staffMove() == t->chord2()->staffMove()) {
                tremUp = t->chord1()->staffMove() == item->maxCRMove();
            }
            TremAnchor tremAnchor;
            tremAnchor.chord1 = c;
            int regularBeams = c->beams(); // non-tremolo strokes
            double scale = t->spatium() * t->mag();

            // find the left-side anchor
            double width = item->endAnchor().x() - item->startAnchor().x();
            double height = item->endAnchor().y() - item->startAnchor().y();
            double x = chordBeamAnchor(item, c, ChordBeamAnchorType::Middle).x();
            double proportionAlongX = (x - item->startAnchor().x()) / width;
            double y = item->startAnchor().y() + (proportionAlongX * height);
            y += regularBeams * (ctx.conf().styleB(Sid::useWideBeams) ? 1.0 : 0.75) * scale * (tremUp ? 1. : -1.);
            tremAnchor.y1 = y;
            // find the right-side anchor
            x = chordBeamAnchor(item, t->chord2(), ChordBeamAnchorType::Middle).x();
            proportionAlongX = (x - item->startAnchor().x()) / width;
            y = item->startAnchor().y() + (proportionAlongX * height);
            y += regularBeams * (ctx.conf().styleB(Sid::useWideBeams) ? 1.0 : 0.75) * scale * (tremUp ? 1. : -1.);
            tremAnchor.y2 = y;
            item->tremAnchors().push_back(tremAnchor);
        }
    }
}
