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
#include "beamlayout.h"

#include <unordered_map>

#include "containers.h"

#include "libmscore/beam.h"
#include "libmscore/spanner.h"
#include "libmscore/tremolo.h"
#include "libmscore/chord.h"
#include "libmscore/factory.h"
#include "libmscore/measure.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/utils.h"
#include "libmscore/note.h"

#include "layoutcontext.h"
#include "tlayout.h"
#include "chordlayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

void BeamLayout::layout(Beam* item, LayoutContext& ctx)
{
    // all of the beam layout code depends on _elements being in order by tick
    // this may not be the case if two cr's were recently swapped.
    std::sort(item->_elements.begin(), item->_elements.end(),
              [](const ChordRest* a, const ChordRest* b) -> bool {
        return a->tick() < b->tick();
    });
    System* system = item->_elements.front()->measure()->system();
    item->setParent(system);

    std::vector<ChordRest*> crl;

    size_t n = 0;
    for (ChordRest* cr : item->_elements) {
        auto newSystem = cr->measure()->system();
        if (newSystem && newSystem != system) {
            SpannerSegmentType st;
            if (n == 0) {
                st = SpannerSegmentType::BEGIN;
            } else {
                st = SpannerSegmentType::MIDDLE;
            }
            ++n;
            if (item->fragments.size() < n) {
                item->fragments.push_back(new BeamFragment);
            }
            layout2(item, ctx, crl, st, static_cast<int>(n) - 1);
            crl.clear();
            system = cr->measure()->system();
        }
        crl.push_back(cr);
    }
    item->setbbox(RectF());
    if (!crl.empty()) {
        SpannerSegmentType st;
        if (n == 0) {
            st = SpannerSegmentType::SINGLE;
        } else {
            st = SpannerSegmentType::END;
        }
        if (item->fragments.size() < (n + 1)) {
            item->fragments.push_back(new BeamFragment);
        }
        layout2(item, ctx, crl, st, static_cast<int>(n));

        double lw2 = item->_beamWidth / 2.0;

        for (const BeamSegment* bs : item->_beamSegments) {
            PolygonF a(4);
            a[0] = PointF(bs->line.x1(), bs->line.y1());
            a[1] = PointF(bs->line.x2(), bs->line.y2());
            a[2] = PointF(bs->line.x2(), bs->line.y2());
            a[3] = PointF(bs->line.x1(), bs->line.y1());
            RectF r(a.boundingRect().adjusted(0.0, -lw2, 0.0, lw2));
            item->addbbox(r);
        }
    }
}

//---------------------------------------------------------
//    - remove beam segments
//    - detach from system
//    - calculate stem direction and set chord
//---------------------------------------------------------
void BeamLayout::layout1(Beam* item, LayoutContext& ctx)
{
    item->resetExplicitParent();  // parent is System

    const StaffType* staffType = item->staffType();
    item->_tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    item->_isBesideTabStaff = item->_tab && !item->_tab->stemless() && !item->_tab->stemThrough();

    // TAB's with stem beside staves have special layout
    if (item->_isBesideTabStaff) {
        item->_up = !item->_tab->stemsDown();
        item->_slope = 0.0;
        item->_cross = false;
        item->_minMove = 0;
        item->_maxMove = 0;
        for (ChordRest* cr : item->_elements) {
            if (cr->isChord()) {
                item->_up = cr->up();
                break;
            }
        }
        return;
    }

    if (item->staff()->isDrumStaff(Fraction(0, 1))) {
        if (item->_direction != DirectionV::AUTO) {
            item->_up = item->_direction == DirectionV::UP;
        } else if (item->_isGrace) {
            item->_up = true;
        } else {
            // search through the beam for the first chord with explicit stem direction and use that.
            // if there is no explicit stem direction, default to the direction of the first stem.
            bool firstUp = false;
            bool firstChord = true;
            for (ChordRest* cr : item->_elements) {
                if (cr->isChord()) {
                    DirectionV crDirection = toChord(cr)->stemDirection();
                    if (crDirection != DirectionV::AUTO) {
                        item->_up = crDirection == DirectionV::UP;
                        break;
                    } else if (firstChord) {
                        firstUp = cr->up();
                        firstChord = false;
                    }
                }
                item->_up = firstUp;
            }
        }
        for (ChordRest* cr : item->_elements) {
            cr->computeUp();
            if (cr->isChord()) {
                ChordLayout::layoutStem(toChord(cr), ctx);
            }
        }
        return;
    }

    item->_minMove = std::numeric_limits<int>::max();
    item->_maxMove = std::numeric_limits<int>::min();
    double mag = 0.0;

    item->_notes.clear();
    staff_idx_t staffIdx = mu::nidx;
    for (ChordRest* cr : item->_elements) {
        double m = cr->isSmall() ? item->score()->styleD(Sid::smallNoteMag) : 1.0;
        mag = std::max(mag, m);
        if (cr->isChord()) {
            Chord* chord = toChord(cr);
            staffIdx = chord->vStaffIdx();
            int i = chord->staffMove();
            item->_minMove = std::min(item->_minMove, i);
            item->_maxMove = std::max(item->_maxMove, i);

            for (int distance : chord->noteDistances()) {
                item->_notes.push_back(distance);
            }
        }
    }

    std::sort(item->_notes.begin(), item->_notes.end());
    item->setMag(mag);

    //
    // determine beam stem direction
    //
    if (item->_elements.empty()) {
        return;
    }
    ChordRest* firstNote = item->_elements.front();
    Measure* measure = firstNote->measure();
    bool hasMultipleVoices = measure->hasVoices(firstNote->staffIdx(), item->tick(), item->ticks());
    if (item->_direction != DirectionV::AUTO) {
        item->_up = item->_direction == DirectionV::UP;
    } else if (item->_maxMove > 0) {
        item->_up = false;
    } else if (item->_minMove < 0) {
        item->_up = true;
    } else if (item->_isGrace) {
        if (hasMultipleVoices) {
            item->_up = firstNote->track() % 2 == 0;
        } else {
            item->_up = true;
        }
    } else if (item->_notes.size()) {
        if (hasMultipleVoices) {
            item->_up = firstNote->track() % 2 == 0;
        } else {
            if (const Chord* chord = item->findChordWithCustomStemDirection()) {
                item->_up = chord->stemDirection() == DirectionV::UP;
            } else {
                std::set<int> noteSet(item->_notes.begin(), item->_notes.end());
                std::vector<int> notes(noteSet.begin(), noteSet.end());
                item->_up = Chord::computeAutoStemDirection(notes) > 0;
            }
        }
    } else {
        item->_up = true;
    }

    int middleStaffLine = firstNote->staffType()->middleLine();
    for (size_t i = 0; i < item->_notes.size(); i++) {
        item->_notes[i] += middleStaffLine;
    }

    item->_cross = item->_minMove != item->_maxMove;
    bool isEntirelyMoved = false;
    if (item->_minMove == item->_maxMove && item->_minMove != 0) {
        isEntirelyMoved = true;
        item->setStaffIdx(staffIdx);
        if (item->_direction == DirectionV::AUTO) {
            item->_up = item->_maxMove > 0;
        }
    } else if (item->_elements.size()) {
        item->setStaffIdx(item->_elements.at(0)->staffIdx());
    }

    item->_slope = 0.0;

    for (ChordRest* cr : item->_elements) {
        const bool staffMove = cr->isChord() ? toChord(cr)->staffMove() : false;
        if (!item->_cross || !staffMove) {
            if (cr->up() != item->_up) {
                cr->setUp(isEntirelyMoved ? item->_up : (item->_up != staffMove));
                if (cr->isChord()) {
                    ChordLayout::layoutStem(toChord(cr), ctx);
                }
            }
        }
    }
}

void BeamLayout::layout2(Beam* item, LayoutContext& ctx, const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag)
{
    item->_layoutInfo = BeamTremoloLayout(item);
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

    item->_beamSpacing = item->score()->styleB(Sid::useWideBeams) ? 4 : 3;
    item->_beamDist = (item->_beamSpacing / 4.0) * item->spatium() * item->mag();
    item->_beamWidth = item->point(item->score()->styleS(Sid::beamWidth)) * item->mag();

    item->_startAnchor = item->_layoutInfo.chordBeamAnchor(startChord, BeamTremoloLayout::ChordBeamAnchorType::Start);
    item->_endAnchor = item->_layoutInfo.chordBeamAnchor(endChord, BeamTremoloLayout::ChordBeamAnchorType::End);

    if (item->_isGrace) {
        item->_beamDist *= item->score()->styleD(Sid::graceNoteMag);
        item->_beamWidth *= item->score()->styleD(Sid::graceNoteMag);
    }

    int fragmentIndex = (item->_direction == DirectionV::AUTO || item->_direction == DirectionV::DOWN) ? 0 : 1;
    if (item->_userModified[fragmentIndex]) {
        item->_layoutInfo = BeamTremoloLayout(item);
        double startY = item->fragments[frag]->py1[fragmentIndex];
        double endY = item->fragments[frag]->py2[fragmentIndex];
        if (item->score()->styleB(Sid::snapCustomBeamsToGrid)) {
            const double quarterSpace = item->spatium() / 4;
            startY = round(startY / quarterSpace) * quarterSpace;
            endY = round(endY / quarterSpace) * quarterSpace;
        }
        startY += item->pagePos().y();
        endY += item->pagePos().y();
        item->_startAnchor.setY(startY);
        item->_endAnchor.setY(endY);
        item->_layoutInfo.setAnchors(item->_startAnchor, item->_endAnchor);
        item->_slope = (item->_endAnchor.y() - item->_startAnchor.y()) / (item->_endAnchor.x() - item->_startAnchor.x());
        createBeamSegments(item, chordRests);
        item->setTremAnchors();
        return;
    }

    // anchor represents the middle of the beam, not the tip of the stem
    // location depends on _isBesideTabStaff

    if (!item->_isBesideTabStaff) {
        item->_layoutInfo = BeamTremoloLayout(item);
        item->_layoutInfo.calculateAnchors(chordRests, item->_notes);
        item->_startAnchor = item->_layoutInfo.startAnchor();
        item->_endAnchor = item->_layoutInfo.endAnchor();
        item->_slope = (item->_endAnchor.y() - item->_startAnchor.y()) / (item->_endAnchor.x() - item->_startAnchor.x());
        item->_beamDist = item->_layoutInfo.beamDist();
    } else {
        item->_slope = 0;
        Chord* startChord = nullptr;
        for (ChordRest* cr : chordRests) {
            if (cr->isChord()) {
                startChord = toChord(cr);
                break;
            }
        }
        item->_layoutInfo = BeamTremoloLayout(item);
        double x1 = item->_layoutInfo.chordBeamAnchorX(chordRests.front(), BeamTremoloLayout::ChordBeamAnchorType::Start);
        double x2 = item->_layoutInfo.chordBeamAnchorX(chordRests.back(), BeamTremoloLayout::ChordBeamAnchorType::End);
        double y = item->_layoutInfo.chordBeamAnchorY(startChord);
        item->_startAnchor = PointF(x1, y);
        item->_endAnchor = PointF(x2, y);
        item->_layoutInfo.setAnchors(item->_startAnchor, item->_endAnchor);
        item->_beamWidth = item->_layoutInfo.beamWidth();
    }

    item->fragments[frag]->py1[fragmentIndex] = item->_startAnchor.y() - item->pagePos().y();
    item->fragments[frag]->py2[fragmentIndex] = item->_endAnchor.y() - item->pagePos().y();

    createBeamSegments(item, chordRests);
    item->setTremAnchors();
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
        if (b->cross()) {
            return false;
        }

        // for beams not already considered cross,
        // consider them so here if any elements were moved up
        for (ChordRest* cr1 : b->elements()) {
            // some element moved up?
            if (cr1->staffMove() < 0) {
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
        if (b->cross()) {
            return true;
        }

        // for beams not already considered cross,
        // consider them so here if any elements were moved up
        for (ChordRest* cr1 : b->elements()) {
            // some element moved up?
            if (cr1->staffMove() < 0) {
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

void BeamLayout::restoreBeams(Measure* m)
{
    LayoutContext ctx(m->score());
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->elist()) {
            if (e && e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                Beam* b = cr->beam();
                if (b && !b->elements().empty() && b->elements().front() == cr) {
                    TLayout::layout(b, ctx);
                    b->addSkyline(m->system()->staff(b->staffIdx())->skyline());
                }
            }
        }
    }
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
    const size_t ntracks = ctx.score()->ntracks();
    Segment* fstSeg = next->first(SegmentType::ChordRest);
    if (!fstSeg) {
        return;
    }

    for (size_t track = 0; track < ntracks; ++track) {
        Staff* stf = ctx.score()->staff(track2staff(track));

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
            if (beamCR->measure() == measure) {
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
            newBeam = Factory::createBeam(ctx.score()->dummy()->system());
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

#define beamModeMid(a) (a == BeamMode::MID || a == BeamMode::BEGIN32 || a == BeamMode::BEGIN64)

//---------------------------------------------------------
//   beamGraceNotes
//---------------------------------------------------------

void BeamLayout::beamGraceNotes(Score* score, Chord* mainNote, bool after)
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
                LayoutContext ctx(score);
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
                LayoutContext ctx(score);
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
                    beam = Factory::createBeam(score->dummy()->system());
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
        LayoutContext ctx(score);
        layout1(beam, ctx);
    } else if (a1) {
        a1->removeDeleteBeam(false);
    }
}

void BeamLayout::createBeams(Score* score, LayoutContext& lc, Measure* measure)
{
    bool crossMeasure = score->styleB(Sid::crossMeasureValues);

    for (track_idx_t track = 0; track < score->ntracks(); ++track) {
        Staff* stf = score->staff(track2staff(track));

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
                int beat = (mcr->rtick() * stretch).ticks() / Constants::division;
                if (mu::contains(beatSubdivision, beat)) {
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

            if (firstCR) {
                firstCR = false;
                // Handle cross-measure beams
                BeamMode mode = cr->beamMode();
                if (mode == BeamMode::MID || mode == BeamMode::END || mode == BeamMode::BEGIN32 || mode == BeamMode::BEGIN64) {
                    ChordRest* prevCR = score->findCR(measure->tick() - Fraction::fromTicks(1), track);
                    if (prevCR) {
                        Beam* prevBeam = prevCR->beam();
                        const Measure* pm = prevCR->measure();
                        if (!beamNoContinue(prevCR->beamMode())
                            && !pm->lineBreak() && !pm->pageBreak() && !pm->sectionBreak()
                            && lc.prevMeasure
                            && !(prevCR->isChord() && prevCR->durationType().type() <= DurationType::V_QUARTER)) {
                            beam = prevBeam;
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

            // handle grace notes and cross-measure beaming
            // (tied chords?)
            if (cr->isChord()) {
                Chord* chord = toChord(cr);
                beamGraceNotes(score, chord, false);         // grace before
                beamGraceNotes(score, chord, true);          // grace after
                // set up for cross-measure values as soon as possible
                // to have all computations (stems, hooks, ...) consistent with it
                if (!chord->isGrace()) {
                    chord->crossMeasureSetup(crossMeasure);
                }
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
                    if ((tick.ticks() % Constants::division) == 0) {
                        int beat = tick.ticks() / Constants::division;
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

            if (cr->durationType().hooks() > 0 && cr->crossMeasure() == CrossMeasure::SECOND) {
                bm = BeamMode::NONE;
            }

            if ((cr->durationType().type() <= DurationType::V_QUARTER) || (bm == BeamMode::NONE)) {
                bool removeBeam = true;
                if (beam) {
                    LayoutContext ctx(score);
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
                    LayoutContext ctx(score);
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
                        beam = Factory::createBeam(score->dummy()->system());
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
            LayoutContext ctx(score);
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
        if (!e || !e->isChordRest() || !e->score()->staff(e->staffIdx())->show()) {
            // the beam and its system may still be referenced when selecting all,
            // even if the staff is invisible. The old system is invalid and does cause problems in #284012
            if (e && e->isChordRest() && !e->score()->staff(e->staffIdx())->show() && toChordRest(e)->beam()) {
                toChordRest(e)->beam()->resetExplicitParent();
            }
            continue;
        }
        ChordRest* cr = toChordRest(e);
        // layout beam
        if (BeamLayout::isTopBeam(cr)) {
            TLayout::layout(cr->beam(), ctx);
            if (!cr->beam()->tremAnchors().empty()) {
                // there are inset tremolos in here
                for (ChordRest* beamCr : cr->beam()->elements()) {
                    if (!beamCr->isChord()) {
                        continue;
                    }
                    Chord* c = toChord(beamCr);
                    if (c->tremolo() && c->tremolo()->twoNotes()) {
                        TLayout::layout(c->tremolo(), ctx);
                    }
                }
            }
        }
        if (!cr->isChord()) {
            continue;
        }
        for (Chord* grace : toChord(cr)->graceNotes()) {
            if (BeamLayout::isTopBeam(grace)) {
                TLayout::layout(grace->beam(), ctx);
            }
        }
    }
}

void BeamLayout::verticalAdjustBeamedRests(Rest* rest, Beam* beam, LayoutContext& ctx)
{
    const double spatium = rest->spatium();
    static constexpr Fraction rest32nd(1, 32);
    const bool up = beam->up();

    double restToBeamPadding;
    if (rest->ticks() <= rest32nd) {
        restToBeamPadding = 0.2 * spatium;
    } else {
        restToBeamPadding = 0.35 * spatium;
    }

    Shape beamShape = beam->shape().translated(beam->pagePos());
    mu::remove_if(beamShape, [&](ShapeElement& el) {
        return el.toItem && el.toItem->isBeamSegment() && toBeamSegment(el.toItem)->isBeamlet;
    });

    Shape restShape = rest->shape().translated(rest->pagePos() - rest->offset());

    double restToBeamClearance = up ? beamShape.verticalClearance(restShape) : restShape.verticalClearance(beamShape);
    if (restToBeamClearance > restToBeamPadding) {
        return;
    }

    if (up) {
        rest->verticalClearance().setAbove(restToBeamClearance);
    } else {
        rest->verticalClearance().setBelow(restToBeamClearance);
    }

    bool restIsLocked = rest->verticalClearance().locked();
    if (!restIsLocked) {
        double overlap = (restToBeamPadding - restToBeamClearance);
        double lineDistance = rest->staff()->lineDistance(rest->tick()) * spatium;
        int lineMoves = ceil(overlap / lineDistance);
        lineMoves *= up ? 1 : -1;
        double yMove = lineMoves * lineDistance;
        rest->movePosY(yMove);
        for (Rest* mergedRest : rest->mergedRests()) {
            mergedRest->movePosY(yMove);
        }

        Segment* segment = rest->segment();
        staff_idx_t staffIdx = rest->vStaffIdx();
        Score* score = rest->score();
        std::vector<Chord*> chords;
        std::vector<Rest*> rests;
        collectChordsAndRest(segment, staffIdx, chords, rests);
        ChordLayout::resolveRestVSChord(rests, chords, score, segment, staffIdx);
        ChordLayout::resolveRestVSRest(rests, score, segment, staffIdx, ctx, /*considerBeams*/ true);
    }

    TLayout::layout(beam, ctx);
}

void BeamLayout::createBeamSegments(Beam* item, const std::vector<ChordRest*>& chordRests)
{
    DeleteAll(item->_beamSegments);
    item->_beamSegments.clear();

    bool levelHasBeam = false;
    int level = 0;
    do {
        levelHasBeam = false;
        ChordRest* startCr = nullptr;
        ChordRest* endCr = nullptr;
        bool breakBeam = false;
        bool previousBreak32 = false;
        bool previousBreak64 = false;

        for (size_t i = 0; i < chordRests.size(); i++) {
            ChordRest* chordRest = chordRests[i];
            ChordRest* prevChordRest = i < 1 ? nullptr : chordRests[i - 1];

            if (level < chordRest->beams()) {
                levelHasBeam = true;
            }
            bool isBroken32 = false;
            bool isBroken64 = false;
            // updates isBroken32 and isBroken64
            item->calcBeamBreaks(chordRest, prevChordRest, level, isBroken32, isBroken64);
            breakBeam = isBroken32 || isBroken64;

            if (level < chordRest->beams() && !breakBeam) {
                endCr = chordRest;
                if (!startCr) {
                    startCr = chordRest;
                }
            } else {
                if (startCr && endCr) {
                    if (startCr == endCr && startCr->isChord()) {
                        bool isBeamletBefore = calcIsBeamletBefore(item,
                                                                   toChord(startCr),
                                                                   static_cast<int>(i) - 1,
                                                                   level,
                                                                   previousBreak32,
                                                                   previousBreak64);
                        createBeamletSegment(item, toChord(startCr), isBeamletBefore, level);
                    } else {
                        createBeamSegment(item, startCr, endCr, level);
                    }
                }
                bool setCr = chordRest && chordRest->isChord() && breakBeam && level < chordRest->beams();
                startCr = setCr ? chordRest : nullptr;
                endCr = setCr ? chordRest : nullptr;
            }
            previousBreak32 = isBroken32;
            previousBreak64 = isBroken64;
        }

        // if the beam ends on the last chord
        if (startCr && (endCr || breakBeam)) {
            if ((startCr == endCr || !endCr) && startCr->isChord()) {
                // this chord is either the last chord, or the first (followed by a rest)
                bool isBefore = !(startCr == chordRests.front());
                createBeamletSegment(item, toChord(startCr), isBefore, level);
            } else {
                createBeamSegment(item, startCr, endCr, level);
            }
        }
        level++;
    } while (levelHasBeam);
}

bool BeamLayout::calcIsBeamletBefore(const Beam* item, Chord* chord, int i, int level, bool isAfter32Break, bool isAfter64Break)
{
    // if first or last chord in beam group
    if (i == 0) {
        return false;
    } else if (i == static_cast<int>(item->_elements.size()) - 1) {
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
    ChordRest* nextChordRest = item->_elements[i + 1];
    ChordRest* currChordRest = item->_elements[i];
    ChordRest* prevChordRest = item->_elements[i - 1];
    if (nextChordRest->isChord()) {
        bool nextBreak32 = false;
        bool nextBreak64 = false;
        bool currBreak32 = false;
        bool currBreak64 = false;
        item->calcBeamBreaks(currChordRest, prevChordRest, prevChordRest->beams(), currBreak32, currBreak64);
        item->calcBeamBreaks(nextChordRest, currChordRest, level, nextBreak32, nextBreak64);
        if ((nextBreak32 && level >= 1) || (!currBreak32 && nextBreak64 && level >= 2)) {
            return true;
        }
    }

    // if previous or next chord has more beams, point in that direction
    int previousChordLevel = -1;
    int nextChordLevel = -1;
    int previousOffset = 1;
    while (i - previousOffset >= 0) {
        ChordRest* previous = item->_elements[i - previousOffset];
        if (previous->isChord()) {
            previousChordLevel = toChord(previous)->beams();
            if (isAfter32Break) {
                previousChordLevel = std::min(previousChordLevel, 1);
            } else if (isAfter64Break) {
                previousChordLevel = std::min(previousChordLevel, 2);
            }
            break;
        }
        ++previousOffset;
    }

    int nextOffset = 1;
    while (i + nextOffset < static_cast<int>(item->_elements.size())) {
        ChordRest* next = item->_elements[i + nextOffset];
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

void BeamLayout::createBeamSegment(Beam* item, ChordRest* startCr, ChordRest* endCr, int level)
{
    const bool isFirstSubgroup = startCr == item->_elements.front();
    const bool isLastSubgroup = endCr == item->_elements.back();
    const bool firstUp = startCr->up();
    const bool lastUp = endCr->up();
    bool overallUp = item->_up;
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
            for (ChordRest* cr : item->_elements) {
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
                overallUp = item->_up;
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

    const double startX = item->_layoutInfo.chordBeamAnchorX(startCr, BeamTremoloLayout::ChordBeamAnchorType::Start);
    const double endX = item->_layoutInfo.chordBeamAnchorX(endCr, BeamTremoloLayout::ChordBeamAnchorType::End);

    double startY = item->_slope * (startX - item->_startAnchor.x()) + item->_startAnchor.y() - item->pagePos().y();
    double endY = item->_slope * (endX - item->_startAnchor.x()) + item->_startAnchor.y() - item->pagePos().y();

    int beamsBelow = 0; // how many beams below level 0?
    int beamsAbove = 0; // how many beams above level 0?

    // avoid adjusting for beams on opposite side of level 0
    if (level != 0) {
        for (const BeamSegment* beam : item->_beamSegments) {
            if (beam->level == 0 || beam->endTick < startCr->tick() || beam->startTick > endCr->tick()) {
                continue;
            }
            ++(beam->above ? beamsAbove : beamsBelow);
        }

        const int upValue = overallUp ? -1 : 1;
        const int extraBeamAdjust = overallUp ? beamsAbove : beamsBelow;
        const double verticalOffset = item->_beamDist * (level - extraBeamAdjust) * upValue;

        if (RealIsEqual(item->_grow1, item->_grow2)) {
            startY -= verticalOffset * item->_grow1;
            endY -= verticalOffset * item->_grow1;
        } else {
            // Feathered beams
            double startProportionAlongX = (startX - item->_startAnchor.x()) / (item->_endAnchor.x() - item->_startAnchor.x());
            double endProportionAlongX = (endX - item->_startAnchor.x()) / (item->_endAnchor.x() - item->_startAnchor.x());

            double grow1 = startProportionAlongX * (item->_grow2 - item->_grow1) + item->_grow1;
            double grow2 = endProportionAlongX * (item->_grow2 - item->_grow1) + item->_grow1;

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
    item->_beamSegments.push_back(b);

    if (level > 0) {
        ++(b->above ? beamsAbove : beamsBelow);
    }

    // extend stems properly
    for (ChordRest* cr : item->_elements) {
        if (!cr->isChord() || cr->tick() < startCr->tick()) {
            continue;
        }
        if (cr->tick() > endCr->tick()) {
            break;
        }

        Chord* chord = toChord(cr);
        double addition = 0.0;

        if (level > 0) {
            double grow = item->_grow1;
            if (!RealIsEqual(item->_grow1, item->_grow2)) {
                double anchorX = item->_layoutInfo.chordBeamAnchorX(chord, BeamTremoloLayout::ChordBeamAnchorType::Middle);
                double proportionAlongX = (anchorX - item->_startAnchor.x()) / (item->_endAnchor.x() - item->_startAnchor.x());
                grow = proportionAlongX * (item->_grow2 - item->_grow1) + item->_grow1;
            }

            int extraBeamAdjust = cr->up() ? beamsBelow : beamsAbove;
            addition = grow * (level - extraBeamAdjust) * item->_beamDist;
        }

        if (level == 0 || !RealIsEqual(addition, 0.0)) {
            item->_layoutInfo.extendStem(chord, addition);
        }

        if (chord == endCr) {
            break;
        }
    }
}

void BeamLayout::createBeamletSegment(Beam* item, ChordRest* cr, bool isBefore, int level)
{
    const double startX = item->_layoutInfo.chordBeamAnchorX(cr,
                                                             isBefore ? BeamTremoloLayout::ChordBeamAnchorType::End : BeamTremoloLayout::ChordBeamAnchorType::Start);

    const double beamletLength = item->score()->styleMM(Sid::beamMinLen).val() * cr->mag();

    const double endX = startX + (isBefore ? -beamletLength : beamletLength);

    double startY = item->_slope * (startX - item->_startAnchor.x()) + item->_startAnchor.y() - item->pagePos().y();
    double endY = item->_slope * (endX - startX) + startY;

    // how many beams past level 0 (i.e. beams on the other side of level 0 for this subgroup)
    int extraBeamAdjust = 0;

    // avoid adjusting for beams on opposite side of level 0
    for (const BeamSegment* beam : item->_beamSegments) {
        if (beam->level == 0 || beam->line.x2() < startX || beam->line.x1() > endX) {
            continue;
        }

        if (cr->up() == beam->above) {
            extraBeamAdjust++;
        }
    }

    const int upValue = cr->up() ? -1 : 1;
    const double verticalOffset = item->_beamDist * (level - extraBeamAdjust) * upValue;

    if (RealIsEqual(item->_grow1, item->_grow2)) {
        startY -= verticalOffset * item->_grow1;
        endY -= verticalOffset * item->_grow1;
    } else {
        // Feathered beams
        double startProportionAlongX = (startX - item->_startAnchor.x()) / (item->_endAnchor.x() - item->_startAnchor.x());
        double endProportionAlongX = (endX - item->_startAnchor.x()) / (item->_endAnchor.x() - item->_startAnchor.x());

        double grow1 = startProportionAlongX * (item->_grow2 - item->_grow1) + item->_grow1;
        double grow2 = endProportionAlongX * (item->_grow2 - item->_grow1) + item->_grow1;

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
    item->_beamSegments.push_back(b);
}

bool BeamLayout::layout2Cross(Beam* item, const std::vector<ChordRest*>& chordRests, int frag)
{
    int fragmentIndex = (item->_direction == DirectionV::AUTO || item->_direction == DirectionV::DOWN) ? 0 : 1;
    ChordRest* startCr = item->_elements.front();
    ChordRest* endCr = item->_elements.back();

    const double quarterSpace = item->spatium() / 4;
    // imagine a line of beamed notes all in a row on the same staff. the first and last of those
    // are the 'outside' notes, and the slant of the beam is going to be affected by the 'middle' notes
    // between them.
    // we have to keep track of this for both staves.
    Chord* topFirst = nullptr;
    Chord* topLast = nullptr;
    Chord* bottomFirst = nullptr;
    Chord* bottomLast = nullptr;
    int maxMiddleTopLine = std::numeric_limits<int>::min(); // lowest note in the top staff
    int minMiddleBottomLine = std::numeric_limits<int>::max(); // highest note in the bottom staff
    int prevTopLine = maxMiddleTopLine; // previous note's line position (top)
    int prevBottomLine = minMiddleBottomLine; // previous note's line position (bottom)
    // if the immediate neighbor of one of the two 'outside' notes on either the top or bottom
    // are the same as that outside note, we need to record it so that we can add a 1/4 space slant.
    bool secondTopIsSame = false;
    bool secondBottomIsSame = false;
    bool penultimateTopIsSame = false;
    bool penultimateBottomIsSame = false;
    double maxY = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::min();
    int otherStaff = 0;
    // recompute _minMove and _maxMove as they may have shifted since last layout
    item->_minMove = std::numeric_limits<int>::max();
    item->_maxMove = std::numeric_limits<int>::min();
    for (ChordRest* c : chordRests) {
        IF_ASSERT_FAILED(c) {
            continue;
        }
        int staffMove = c->staffMove();
        item->_minMove = std::min(item->_minMove, staffMove);
        item->_maxMove = std::max(item->_maxMove, staffMove);

        if (staffMove != 0) {
            otherStaff = staffMove;
        }
    }
    if (otherStaff == 0 || item->_minMove == item->_maxMove) {
        return false;
    }
    // Find the notes on the top and bottom of staves
    //
    bool checkNextTop = false;
    bool checkNextBottom = false;
    for (ChordRest* cr : chordRests) {
        if (!cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);
        if ((c->staffMove() == otherStaff && otherStaff > 0) || (c->staffMove() != otherStaff && otherStaff < 0)) {
            // this chord is on the bottom staff
            if (penultimateBottomIsSame) {
                // the chord we took as the penultimate bottom note wasn't.
                // so treat it properly as a middle note
                minMiddleBottomLine = std::min(minMiddleBottomLine, prevBottomLine);
                penultimateBottomIsSame = false;
            }
            checkNextTop = false; // we are no longer looking for the second note in the top
                                  // staff being the same as the first--this note is on the bottom.
            if (!bottomFirst) {
                bottomFirst = c;
                checkNextBottom = true; // this was the first bottom note, so check for second next time
            } else {
                penultimateBottomIsSame = prevBottomLine == c->line();
                if (!penultimateBottomIsSame) {
                    minMiddleBottomLine = std::min(minMiddleBottomLine, prevBottomLine);
                }
                if (checkNextBottom) {
                    // this is the second bottom note, so we should see if this one is same line as first
                    secondBottomIsSame = c->line() == bottomFirst->line();
                    checkNextBottom = false;
                } else {
                    prevBottomLine = c->line();
                }
                bottomLast = c;
            }
            maxY = std::min(maxY, item->_layoutInfo.chordBeamAnchorY(toChord(c)));
        } else {
            // this chord is on the top staff
            if (penultimateTopIsSame) {
                // the chord we took as the penultimate top note wasn't.
                // so treat it properly as a middle note
                maxMiddleTopLine = std::max(maxMiddleTopLine, prevTopLine);
                penultimateTopIsSame = false;
            }
            checkNextBottom = false; // no longer looking for a bottom second note since this is on top
            if (!topFirst) {
                topFirst = c;
                checkNextTop = true;
            } else {
                penultimateTopIsSame = prevTopLine == c->line();
                if (!penultimateTopIsSame) {
                    maxMiddleTopLine = std::max(maxMiddleTopLine, prevTopLine);
                }
                if (checkNextTop) {
                    secondTopIsSame = c->line() == topFirst->line();
                    checkNextTop = false;
                } else {
                    prevTopLine = c->line();
                }
                topLast = c;
            }
            minY = std::max(minY, item->_layoutInfo.chordBeamAnchorY(toChord(c)));
        }
    }
    item->_startAnchor.ry() = (maxY + minY) / 2;
    item->_endAnchor.ry() = (maxY + minY) / 2;
    item->_slope = 0;

    if (!item->noSlope()) {
        int topFirstLine = topFirst ? topFirst->downNote()->line() : 0;
        int topLastLine = topLast ? topLast->downNote()->line() : 0;
        int bottomFirstLine = bottomFirst ? bottomFirst->upNote()->line() : 0;
        int bottomLastLine = bottomLast ? bottomLast->upNote()->line() : 0;
        bool constrainTopToQuarter = false;
        bool constrainBottomToQuarter = false;
        if ((topFirstLine > topLastLine && secondTopIsSame)
            || (topFirstLine < topLastLine && penultimateTopIsSame)) {
            constrainTopToQuarter = true;
        }
        if ((bottomFirstLine < bottomLastLine && secondBottomIsSame)
            || (bottomFirstLine > bottomLastLine && penultimateBottomIsSame)) {
            constrainBottomToQuarter = true;
        }
        if (!topLast && !bottomLast && topFirst && bottomFirst) {
            // if there are only two notes, one on each staff, special case
            // take max slope into account
            double yFirst, yLast;
            if (topFirst->tick() < bottomFirst->tick()) {
                yFirst = topFirst->stemPos().y();
                yLast = bottomFirst->stemPos().y();
            } else {
                yFirst = bottomFirst->stemPos().y();
                yLast = topFirst->stemPos().y();
            }
            int desiredSlant = round((yFirst - yLast) / item->spatium());
            int slant = std::min(std::abs(desiredSlant), item->_layoutInfo.getMaxSlope());
            slant *= (desiredSlant < 0) ? -quarterSpace : quarterSpace;
            item->_startAnchor.ry() += (slant / 2);
            item->_endAnchor.ry() -= (slant / 2);
        } else if (!topLast || !bottomLast) {
            // otherwise, if there is only one note on one of the staves, use slope from other staff
            int startNote = 0;
            int endNote = 0;
            bool forceHoriz = false;
            if (!topLast) {
                startNote = bottomFirstLine;
                endNote = bottomLastLine;
                if (minMiddleBottomLine <= std::min(startNote, endNote)) {
                    // there is a note closer to the beam than the start and end notes
                    // we force horizontal beam here.
                    forceHoriz = true;
                }
            } else if (!bottomLast) {
                startNote = topFirstLine;
                endNote = topLastLine;
                if (maxMiddleTopLine >= std::max(startNote, endNote)) {
                    // same as above, for the top staff
                    // force horizontal.
                    forceHoriz = true;
                }
            }

            if (!forceHoriz) {
                int slant = startNote - endNote;
                slant = std::min(std::abs(slant), item->_layoutInfo.getMaxSlope());
                if ((!bottomLast && constrainTopToQuarter) || (!topLast && constrainBottomToQuarter)) {
                    slant = 1;
                }
                double slope = slant * (startNote > endNote ? quarterSpace : -quarterSpace);
                item->_startAnchor.ry() += (slope / 2);
                item->_endAnchor.ry() -= (slope / 2);
            } // otherwise, do nothing, beam is already horizontal.
        } else {
            // otherwise, there are at least two notes on each staff
            // (that is, topLast and bottomLast are both set)
            bool forceHoriz = false;
            if (topFirstLine == topLastLine || bottomFirstLine == bottomLastLine) {
                // if outside notes on top or bottom staff are on the same staff line, slope = 0
                // no further adjustment needed, the beam is already well-placed and horizontal
                forceHoriz = true;
            }
            // otherwise, we have to compare the slopes from the top staff and bottom staff.
            int topSlant = topFirstLine - topLastLine;
            if (constrainTopToQuarter && topSlant != 0) {
                topSlant = topFirstLine < topLastLine ? -1 : 1;
            }
            int bottomSlant = bottomFirstLine - bottomLastLine;
            if (constrainBottomToQuarter && bottomSlant != 0) {
                bottomSlant = bottomFirstLine < bottomLastLine ? -1 : 1;
            }
            if ((maxMiddleTopLine >= std::max(topFirstLine, topLastLine)
                 || (minMiddleBottomLine <= std::min(bottomFirstLine, bottomLastLine)))) {
                forceHoriz = true;
            }
            if (topSlant == 0 || bottomSlant == 0 || forceHoriz) {
                // if one of the slants is 0, the whole slant is zero
            } else if ((topSlant < 0 && bottomSlant < 0) || (topSlant > 0 && bottomSlant > 0)) {
                int slant = (abs(topSlant) < abs(bottomSlant)) ? topSlant : bottomSlant;
                slant = std::min(std::abs(slant), item->_layoutInfo.getMaxSlope());
                double slope = slant * ((topSlant < 0) ? -quarterSpace : quarterSpace);
                item->_startAnchor.ry() += (slope / 2);
                item->_endAnchor.ry() -= (slope / 2);
            } else {
                // if the two slopes are in opposite directions, flat!
                // nothing needs to be done, the beam is already horizontal and placed nicely
            }
        }
        item->_startAnchor.setX(item->_layoutInfo.chordBeamAnchorX(startCr, BeamTremoloLayout::ChordBeamAnchorType::Start));
        item->_endAnchor.setX(item->_layoutInfo.chordBeamAnchorX(endCr, BeamTremoloLayout::ChordBeamAnchorType::End));
        item->_slope = (item->_endAnchor.y() - item->_startAnchor.y()) / (item->_endAnchor.x() - item->_startAnchor.x());
    }
    item->fragments[frag]->py1[fragmentIndex] = item->_startAnchor.y() - item->pagePos().y();
    item->fragments[frag]->py2[fragmentIndex] = item->_endAnchor.y() - item->pagePos().y();
    createBeamSegments(item, chordRests);
    return true;
}
