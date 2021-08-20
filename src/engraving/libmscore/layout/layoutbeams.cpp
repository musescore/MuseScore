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
#include "layoutbeams.h"

#include "../score.h"
#include "../staff.h"
#include "../chord.h"

#include "layoutcontext.h"

using namespace mu::engraving;
using namespace Ms;

//---------------------------------------------------------
//   isTopBeam
//    returns true for the first CR of a beam that is not cross-staff
//---------------------------------------------------------

bool LayoutBeams::isTopBeam(ChordRest* cr)
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
//   restoreBeams
//---------------------------------------------------------

void LayoutBeams::restoreBeams(Measure* m)
{
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (Element* e : s->elist()) {
            if (e && e->isChordRest()) {
                ChordRest* cr = toChordRest(e);
                if (isTopBeam(cr)) {
                    Beam* b = cr->beam();
                    b->layout();
                    b->addSkyline(m->system()->staff(b->staffIdx())->skyline());
                }
            }
        }
    }
}

//---------------------------------------------------------
//   breakCrossMeasureBeams
//---------------------------------------------------------

void LayoutBeams::breakCrossMeasureBeams(Measure* measure)
{
    MeasureBase* mbNext = measure->next();
    if (!mbNext || !mbNext->isMeasure()) {
        return;
    }

    Measure* next = toMeasure(mbNext);
    Score* score = measure->score();
    const int ntracks = score->ntracks();
    Segment* fstSeg = next->first(SegmentType::ChordRest);
    if (!fstSeg) {
        return;
    }

    for (int track = 0; track < ntracks; ++track) {
        Staff* stf = score->staff(track2staff(track));

        // don’t compute beams for invisible staves and tablature without stems
        if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->stemless())) {
            continue;
        }

        Element* e = fstSeg->element(track);
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
            newBeam = new Beam(score);
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
            newBeam->layout1();
        }
    }
}

static bool beamNoContinue(Beam::Mode mode)
{
    return mode == Beam::Mode::END || mode == Beam::Mode::NONE || mode == Beam::Mode::INVALID;
}

#define beamModeMid(a) (a == Beam::Mode::MID || a == Beam::Mode::BEGIN32 || a == Beam::Mode::BEGIN64)

//---------------------------------------------------------
//   beamGraceNotes
//---------------------------------------------------------

void LayoutBeams::beamGraceNotes(Score* score, Chord* mainNote, bool after)
{
    ChordRest* a1    = 0;        // start of (potential) beam
    Beam* beam       = 0;        // current beam
    Beam::Mode bm = Beam::Mode::AUTO;
    QVector<Chord*> graceNotes = after ? mainNote->graceNotesAfter() : mainNote->graceNotesBefore();

    for (ChordRest* cr : qAsConst(graceNotes)) {
        bm = Groups::endBeam(cr);
        if ((cr->durationType().type() <= TDuration::DurationType::V_QUARTER) || (bm == Beam::Mode::NONE)) {
            if (beam) {
                beam->layoutGraceNotes();
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
            bool beamEnd = bm == Beam::Mode::BEGIN;
            if (!beamEnd) {
                cr->replaceBeam(beam);
                cr = 0;
                beamEnd = (bm == Beam::Mode::END);
            }
            if (beamEnd) {
                beam->layoutGraceNotes();
                beam = 0;
            }
        }
        if (!cr) {
            continue;
        }
        if (a1 == 0) {
            a1 = cr;
        } else {
            if (!beamModeMid(bm) && (bm == Beam::Mode::BEGIN)) {
                a1->removeDeleteBeam(false);
                a1 = cr;
            } else {
                beam = a1->beam();
                if (beam == 0 || beam->elements().front() != a1) {
                    beam = new Beam(score);
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
        beam->layoutGraceNotes();
    } else if (a1) {
        a1->removeDeleteBeam(false);
    }
}

void LayoutBeams::createBeams(Score* score, LayoutContext& lc, Measure* measure)
{
    bool crossMeasure = score->styleB(Sid::crossMeasureValues);

    for (int track = 0; track < score->ntracks(); ++track) {
        Staff* stf = score->staff(track2staff(track));

        // don’t compute beams for invisible staves and tablature without stems
        if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->stemless())) {
            continue;
        }

        ChordRest* a1    = 0;          // start of (potential) beam
        bool firstCR     = true;
        Beam* beam       = 0;          // current beam
        Beam::Mode bm    = Beam::Mode::AUTO;
        ChordRest* prev  = 0;
        bool checkBeats  = false;
        Fraction stretch = Fraction(1, 1);
        QHash<int, TDuration> beatSubdivision;

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
                int beat = (mcr->rtick() * stretch).ticks() / MScore::division;
                if (beatSubdivision.contains(beat)) {
                    beatSubdivision[beat] = qMin(beatSubdivision[beat], mcr->durationType());
                } else {
                    beatSubdivision[beat] = mcr->durationType();
                }
            }
        }

        for (Segment* segment = measure->first(st); segment; segment = segment->next(st)) {
            ChordRest* cr = segment->cr(track);
            if (cr == 0) {
                continue;
            }

            if (firstCR) {
                firstCR = false;
                // Handle cross-measure beams
                Beam::Mode mode = cr->beamMode();
                if (mode == Beam::Mode::MID || mode == Beam::Mode::END) {
                    ChordRest* prevCR = score->findCR(measure->tick() - Fraction::fromTicks(1), track);
                    if (prevCR) {
                        const Measure* pm = prevCR->measure();
                        if (!beamNoContinue(prevCR->beamMode())
                            && !pm->lineBreak() && !pm->pageBreak() && !pm->sectionBreak()
                            && lc.prevMeasure
                            && !(prevCR->isChord() && prevCR->durationType().type() <= TDuration::DurationType::V_QUARTER)) {
                            beam = prevCR->beam();
                            //a1 = beam ? beam->elements().front() : prevCR;
                            a1 = beam ? nullptr : prevCR;               // when beam is found, a1 is no longer required.
                        }
                    }
                }
            }
#if 0
            for (Lyrics* l : cr->lyrics()) {
                if (l) {
                    l->layout();
                }
            }
#endif
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

            if (cr->isRest() && cr->beamMode() == Beam::Mode::AUTO) {
                bm = Beam::Mode::NONE;                   // do not beam rests set to Beam::Mode::AUTO
            } else {
                bm = Groups::endBeam(cr, prev);          // get defaults from time signature properties
            }
            // perform additional context-dependent checks
            if (bm == Beam::Mode::AUTO) {
                // check if we need to break beams according to minimum duration in current / previous beat
                if (checkBeats && cr->rtick().isNotZero()) {
                    Fraction tick = cr->rtick() * stretch;
                    // check if on the beat
                    if ((tick.ticks() % MScore::division) == 0) {
                        int beat = tick.ticks() / MScore::division;
                        // get minimum duration for this & previous beat
                        TDuration minDuration = qMin(beatSubdivision[beat], beatSubdivision[beat - 1]);
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
                bm = Beam::Mode::NONE;
            }

            if ((cr->isChord() && cr->durationType().type() <= TDuration::DurationType::V_QUARTER) || (bm == Beam::Mode::NONE)) {
                bool removeBeam = true;
                if (beam) {
                    beam->layout1();
                    removeBeam = (beam->elements().size() <= 1);
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
                bool beamEnd = (bm == Beam::Mode::BEGIN);
                if (!beamEnd) {
                    cr->replaceBeam(beam);
                    cr = 0;
                    beamEnd = (bm == Beam::Mode::END);
                }
                if (beamEnd) {
                    beam->layout1();
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
                    (bm == Beam::Mode::BEGIN
                     || (a1->segment()->segmentType() != cr->segment()->segmentType())
                     || (a1->tick() + a1->actualTicks() < cr->tick())
                    )
                    ) {
                    a1->removeDeleteBeam(false);
                    a1 = cr;
                } else {
                    beam = a1->beam();
                    if (beam == 0 || beam->elements().front() != a1) {
                        beam = new Beam(score);
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
            beam->layout1();
        } else if (a1) {
            Fraction nextTick = a1->tick() + a1->actualTicks();
            Measure* m = (nextTick >= measure->endTick() ? measure->nextMeasure() : measure);
            ChordRest* nextCR = (m ? m->findChordRest(nextTick, track) : nullptr);
            Beam* b = a1->beam();
            if (!(b && b->elements().startsWith(a1) && nextCR && beamModeMid(nextCR->beamMode()))) {
                a1->removeDeleteBeam(false);
            }
        }
    }
}
