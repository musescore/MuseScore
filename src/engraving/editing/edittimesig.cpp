/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "edittimesig.h"

#include <map>
#include <tuple>
#include <vector>

#include "global/containers.h"

#include "../dom/excerpt.h"
#include "../dom/factory.h"
#include "../dom/layoutbreak.h"
#include "../dom/masterscore.h"
#include "../dom/measure.h"
#include "../dom/measurerepeat.h"
#include "../dom/range.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/sig.h"
#include "../dom/slur.h"
#include "../dom/staff.h"
#include "../dom/system.h"
#include "../dom/timesig.h"
#include "../dom/tremolotwochord.h"

#include "addremoveelement.h"
#include "editmeasures.h"
#include "editspanner.h"
#include "edittremolo.h"
#include "transaction/transaction.h"
#include "transaction/undostack.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures from startMeasure to endMeasure (including)
//    If staffIdx is valid (>= 0), then rewrite a local
//    timesig change.
//---------------------------------------------------------

bool EditTimeSig::rewriteMeasures(Transaction& tx, Score* score, Measure* startMeasure, Measure* endMeasure, const Fraction& newTimeSig,
                                  staff_idx_t staffIdx)
{
    if (staffIdx != muse::nidx) {
        // local timesig
        // don't actually rewrite, just update measure rest durations
        // abort if there is anything other than measure rests in range
        track_idx_t strack = staffIdx * VOICES;
        track_idx_t etrack = strack + VOICES;
        for (Measure* m = startMeasure;; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (track_idx_t track = strack; track < etrack; ++track) {
                    ChordRest* cr = toChordRest(s->element(track));
                    if (!cr) {
                        continue;
                    }
                    if (cr->isRest() && cr->durationType() == DurationType::V_MEASURE) {
                        cr->undoChangeProperty(Pid::DURATION, newTimeSig);
                    } else {
                        return false;
                    }
                }
            }
            if (m == endMeasure) {
                break;
            }
        }
        return true;
    }

    bool fmr = true;

    // Format: chord 1 tick, chord 2 tick, tremolo, track
    std::vector<std::tuple<Fraction, Fraction, TremoloTwoChord*, track_idx_t> > tremoloChordTicks;

    track_idx_t strack, etrack;
    if (staffIdx == muse::nidx) {
        strack = 0;
        etrack = score->ntracks();
    } else {
        strack = staffIdx * VOICES;
        etrack = strack + VOICES;
    }

    std::vector<Segment*> endOfMeasureTimeSigsToRemove;

    for (Measure* m = startMeasure; m; m = m->nextMeasure()) {
        if (!m->isFullMeasureRest()) {
            fmr = false;
        }

        for (Segment* s = m->first(); s; s = s->next()) {
            if (!s->isChordRestType() && !(s->isTimeSigType() && s->endOfMeasureChange())) {
                continue;
            }

            if (s->isTimeSigType()) {
                endOfMeasureTimeSigsToRemove.push_back(s);
                continue;
            }

            for (track_idx_t track = strack; track < etrack; ++track) {
                ChordRest* cr = toChordRest(s->element(track));
                if (cr && cr->isChord()) {
                    Chord* chord = toChord(cr);
                    if (chord->tremoloTwoChord()) {
                        TremoloTwoChord* trem = chord->tremoloTwoChord();

                        // Don't add same chord twice
                        if (trem->chord2() == chord) {
                            continue;
                        }

                        std::tuple<Fraction, Fraction, TremoloTwoChord*, track_idx_t> newP
                            = { cr->tick(), trem->chord2()->segment()->tick(), trem, track };
                        tremoloChordTicks.push_back(newP);
                    }
                }
            }
        }

        if (m == endMeasure) {
            break;
        }
    }

    if (!fmr) {
        // check for local time signatures
        for (Measure* m = startMeasure; m; m = m->nextMeasure()) {
            for (size_t si = 0; si < score->nstaves(); ++si) {
                if (score->staff(si)->timeStretch(m->tick()) != Fraction(1, 1)) {
                    // we cannot change a staff with a local time signature
                    return false;
                }
                if (m == endMeasure) {
                    break;
                }
            }
        }
    }

    for (Segment* seg : endOfMeasureTimeSigsToRemove) {
        score->doUndoRemoveElement(seg);
    }

    ScoreRange range;
    Measure* nextMeasure = endMeasure->nextMeasure();
    Segment* finalSeg = endMeasure->last();
    if (nextMeasure) {
        finalSeg = nextMeasure->first();
    }
    range.read(startMeasure->first(), finalSeg);

    //
    // calculate number of required measures = newMeasures
    //
    Fraction ticks = range.ticks().isNotZero() ? range.ticks() : score->endTick() - startMeasure->first()->tick();
    Fraction k = ticks / newTimeSig;
    int newMeasures     = (k.numerator() + k.denominator() - 1) / k.denominator();

    Fraction newDuration = newTimeSig * Fraction(newMeasures, 1);

    // evtl. we have to fill the last measure
    Fraction fill = newDuration - ticks;
    range.fill(fill);

    for (Score* s : score->scoreList()) {
        Measure* m1 = s->tick2measure(startMeasure->tick());
        Measure* m2 = s->tick2measure(endMeasure->tick());

        Fraction tick1 = m1->tick();
        Fraction tick2 = m2->endTick();
        auto spanners = s->spannerMap().findOverlapping(tick1.ticks(), tick2.ticks());
        for (auto i : spanners) {
            if (i.value->tick() >= tick1 && i.value->tick() < tick2) {
                score->doUndoRemoveElement(i.value);
            }
        }
        s->undoRemoveMeasures(m1, m2, true);

        Measure* newFirstMeasure = nullptr;
        Measure* newLastMeasure = nullptr;
        Fraction tick     = startMeasure->tick();
        for (int i = 0; i < newMeasures; ++i) {
            Measure* m = Factory::createMeasure(s->dummy()->system());
            m->setPrev(newLastMeasure);
            if (newLastMeasure) {
                newLastMeasure->setNext(m);
            }
            m->setTimesig(newTimeSig);
            m->setTicks(newTimeSig);
            m->setTick(tick);
            tick += m->ticks();
            newLastMeasure = m;
            if (newFirstMeasure == 0) {
                newFirstMeasure = m;
            }
        }
        //
        // insert new calculated measures
        //
        newFirstMeasure->setPrev(m1->prev());
        newLastMeasure->setNext(m2->next());
        tx.push(new InsertMeasures(newFirstMeasure, newLastMeasure));
    }
    if (!fill.isZero()) {
        score->undoInsertTime(endMeasure->endTick(), fill);
    }

    if (!range.write(score->masterScore(), startMeasure->tick())) {
        return false;
    }

    for (Score* s : score->scoreList()) {
        s->connectTies(true);
    }

    // reset start and end elements for slurs that overlap the rewritten measures
    for (auto spanner : score->spannerMap().findOverlapping(startMeasure->tick().ticks(), endMeasure->tick().ticks())) {
        Slur* slur = (spanner.value->isSlur() ? toSlur(spanner.value) : nullptr);
        if (slur) {
            EngravingItem* startEl = slur->startElement();
            EngravingItem* endEl = slur->endElement();
            if (!startEl || !endEl) {
                continue;
            }
            tx.push(new ChangeStartEndSpanner(spanner.value, slur->findStartCR(), slur->findEndCR()));
        }
    }
    // Attempt to move tremolos to correct chords
    for (auto tremPair : tremoloChordTicks) {
        Fraction chord1Tick = std::get<0>(tremPair);
        Fraction chord2Tick = std::get<1>(tremPair);
        TremoloTwoChord* trem = std::get<2>(tremPair);
        track_idx_t track = std::get<3>(tremPair);

        tx.push(new MoveTremolo(trem->score(), chord1Tick, chord2Tick, trem, track));
    }

    if (score->noteEntryMode()) {
        // set input cursor to possibly re-written segment
        Fraction icTick = score->inputState().tick();
        Segment* icSegment = score->tick2segment(icTick, false, SegmentType::ChordRest);
        if (!icSegment) {
            // this can happen if cursor was on a rest
            // and in the rewriting it got subsumed into a full measure rest
            Measure* icMeasure = score->tick2measure(icTick);
            if (!icMeasure) {                         // shouldn't happen, but just in case
                icMeasure = score->firstMeasure();
            }
            icSegment = icMeasure->first(SegmentType::ChordRest);
        }
        score->inputState().setSegment(icSegment);
    }

    return true;
}

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature or section break
//---------------------------------------------------------

bool EditTimeSig::rewriteMeasures(Transaction& tx, Score* score, Measure* fm, const Fraction& ns, staff_idx_t staffIdx)
{
    Measure* lm  = fm;
    Measure* fm1 = fm;
    Measure* nm  = nullptr;
    LayoutBreak* sectionBreak = nullptr;
    Segment* endOfMeasureTimeSigSeg = nullptr;

    //
    // split into Measure segments fm-lm
    //
    auto foundLM = [fm, staffIdx, &endOfMeasureTimeSigSeg](MeasureBase* curMeas, Measure* curLM) {
        if (!curMeas || !curMeas->isMeasure() || curLM->sectionBreak()) {
            return true;
        }
        Segment* timeSigSeg = toMeasure(curMeas)->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        if (!timeSigSeg && curMeas->prevMeasure()) {
            // find time sig at end of previous measure
            Measure* prevMeasure = curMeas->prevMeasure();
            timeSigSeg = prevMeasure->findSegmentR(SegmentType::TimeSig, prevMeasure->ticks());
            if (timeSigSeg) {
                endOfMeasureTimeSigSeg = timeSigSeg;
            }
        }
        if (timeSigSeg && curMeas != fm) {
            return staffIdx == muse::nidx || timeSigSeg->element(staff2track(staffIdx));
        }
        return false;
    };
    for (MeasureBase* measure = fm;; measure = measure->next()) {
        if (foundLM(measure, lm)) {
            // save section break to reinstate after rewrite
            LayoutBreak* layoutBreak = lm->sectionBreakElement();

            if (lm && layoutBreak) {
                sectionBreak = Factory::copyLayoutBreak(*layoutBreak);
            }

            if (!rewriteMeasures(tx, score, fm1, lm, ns, staffIdx)) {
                if (staffIdx != muse::nidx) {
                    MScore::setError(MsError::CANNOT_CHANGE_LOCAL_TIMESIG_MEASURE_NOT_EMPTY);
                    // restore measure rests that were prematurely modified
                    Fraction fr(score->staff(staffIdx)->timeSig(fm->tick())->sig());
                    for (Measure* m = fm1; m; m = m->nextMeasure()) {
                        ChordRest* cr = m->findChordRest(m->tick(), staffIdx * VOICES);
                        if (cr && cr->isRest() && cr->durationType() == DurationType::V_MEASURE) {
                            cr->undoChangeProperty(Pid::DURATION, fr);
                        } else {
                            break;
                        }
                    }
                }
                for (Measure* m = fm1; m; m = m->nextMeasure()) {
                    if (m->first(SegmentType::TimeSig)) {
                        break;
                    }
                    Fraction fr(ns);
                    m->undoChangeProperty(Pid::TIMESIG_NOMINAL, fr);
                }
                return false;
            }

            // after rewrite, lm is not necessarily valid
            // m is first MeasureBase after rewritten range
            // m->prevMeasure () is new last measure of range
            // set nm to first true Measure after rewritten range
            // we may use this to reinstate time signatures
            if (measure && measure->prevMeasure()) {
                nm = measure->prevMeasure()->nextMeasure();
            } else {
                nm = nullptr;
            }

            if (sectionBreak) {
                // reinstate section break, then stop rewriting
                if (measure && measure->prevMeasure()) {
                    sectionBreak->setParent(measure->prevMeasure());
                    score->undoAddElement(sectionBreak);
                } else if (!measure) {
                    sectionBreak->setParent(score->lastMeasure());
                    score->undoAddElement(sectionBreak);
                } else {
                    LOGD("unable to restore section break");
                    nm = nullptr;
                    sectionBreak = nullptr;
                }
                break;
            }

            // stop rewriting at end of score
            // or at a measure (which means we found a time signature segment)
            if (!measure || measure->isMeasure()) {
                break;
            }

            // skip frames
            while (!measure->isMeasure()) {
                LayoutBreak* layoutBreak2 = measure->sectionBreakElement();

                if (layoutBreak2) {
                    // frame has a section break; we can stop skipping ahead
                    sectionBreak = layoutBreak2;
                    break;
                }
                measure = measure->next();
                if (!measure) {
                    break;
                }
            }
            // stop rewriting if we encountered a section break on a frame
            // or if there is a time signature on first measure after the frame
            if (sectionBreak || (measure && toMeasure(measure)->first(SegmentType::TimeSig))) {
                break;
            }

            // set up for next range to rewrite
            fm1 = toMeasure(measure);
            if (fm1 == 0) {
                break;
            }
        }

        // if we didn't break the loop already,
        // we must have an ordinary measure
        // add measure to range to rewrite
        lm = toMeasure(measure);
    }

    // if any staves don't have time signatures at the point where we stopped,
    // we need to reinstate their previous time signatures
    if (!nm) {
        return true;
    }
    Segment* timeSigSeg = nm->undoGetSegment(SegmentType::TimeSig, nm->tick());

    for (size_t i = 0; i < score->nstaves(); ++i) {
        if (staffIdx != muse::nidx && i != staffIdx) {
            continue;
        }
        if (!timeSigSeg->element(staff2track(i))) {
            TimeSig* ots = endOfMeasureTimeSigSeg
                           ? toTimeSig(endOfMeasureTimeSigSeg->element(staff2track(i))) : score->staff(i)->timeSig(nm->tick());
            if (ots) {
                TimeSig* nts = Factory::copyTimeSig(*ots);
                nts->setParent(timeSigSeg);
                if (sectionBreak) {
                    nts->setGenerated(false);
                    nts->setShowCourtesySig(false);
                }
                score->undoAddElement(nts);
            }
        }
    }

    return true;
}

//---------------------------------------------------------
//   addTimeSig
//
//    Add or change time signature at measure in response
//    to gui command (drop timesig on measure or timesig)
//---------------------------------------------------------

void EditTimeSig::addTimeSig(Transaction& tx, Score* score, Measure* fm, staff_idx_t staffIdx, TimeSig* ts, bool local)
{
    score->deselectAll();

    if (fm->isMMRest()) {
        fm = fm->mmRestFirst();
    }

    Fraction ns   = ts->sig();
    Fraction tick = fm->tick();
    if (local) {
        Fraction stretch = (ns / fm->timesig()).reduced();
        ts->setStretch(stretch);
    }

    track_idx_t track = staffIdx * VOICES;
    Segment* seg = fm->findSegment(SegmentType::TimeSig, tick);
    TimeSig* ots = seg ? toTimeSig(seg->element(track)) : nullptr;

    // Check same tick at end of previous measure
    if (!ots) {
        Measure* prevMeasure = fm->prevMeasure();
        seg = prevMeasure ? prevMeasure->findSegmentR(SegmentType::TimeSig, prevMeasure->ticks()) : seg;
        ots = seg ? toTimeSig(seg->element(track)) : nullptr;
    }

    if (ots && (*ots == *ts)) {
        //
        //  ignore if there is already a timesig
        //  with same values
        //
        delete ts;
        return;
    }

    seg = fm->undoGetSegment(SegmentType::TimeSig, tick);

    auto getStaffIdxRange = [score, local, staffIdx](const Score* lscore) -> std::pair<staff_idx_t /*start*/, staff_idx_t /*end*/> {
        staff_idx_t startStaffIdx, endStaffIdx;
        if (local) {
            if (lscore == score) {
                startStaffIdx = staffIdx;
                endStaffIdx = startStaffIdx + 1;
            } else {
                const Staff* thisStaff = score->staff(staffIdx);
                const Staff* linkedStaff = thisStaff->findLinkedInScore(lscore);
                if (linkedStaff) {
                    startStaffIdx = linkedStaff->idx();
                    endStaffIdx = startStaffIdx + 1;
                } else {
                    startStaffIdx = 0;
                    endStaffIdx = 0;
                }
            }
        } else {
            startStaffIdx = 0;
            endStaffIdx = lscore->nstaves();
        }
        return std::make_pair(startStaffIdx, endStaffIdx);
    };

    if (ots && ots->sig() == ns && ots->stretch() == ts->stretch()) {
        //
        // the measure duration does not change,
        // so its ok to just update the time signatures
        //
        TimeSig* nts = score->staff(staffIdx)->nextTimeSig(tick + Fraction::eps());
        const Fraction lmTick = nts ? nts->segment()->tick() : Fraction(-1, 1);
        for (Score* lscore : score->scoreList()) {
            Measure* mf = lscore->tick2measure(tick);
            Measure* lm = (lmTick != Fraction(-1, 1)) ? lscore->tick2measure(lmTick) : nullptr;
            for (Measure* m = mf; m != lm; m = m->nextMeasure()) {
                bool changeActual = m->ticks() == m->timesig();
                m->undoChangeProperty(Pid::TIMESIG_NOMINAL, ns);
                if (changeActual) {
                    m->undoChangeProperty(Pid::TIMESIG_ACTUAL, ns);
                }
            }
            std::pair<staff_idx_t, staff_idx_t> staffIdxRange = getStaffIdxRange(lscore);
            for (staff_idx_t si = staffIdxRange.first; si < staffIdxRange.second; ++si) {
                TimeSig* nsig = toTimeSig(seg->element(si * VOICES));
                if (!nsig) {
                    continue;
                }
                nsig->undoChangeProperty(Pid::SHOW_COURTESY, ts->showCourtesySig());
                nsig->undoChangeProperty(Pid::TIMESIG, ts->sig());
                nsig->undoChangeProperty(Pid::TIMESIG_TYPE, int(ts->timeSigType()));
                nsig->undoChangeProperty(Pid::NUMERATOR_STRING, ts->numeratorString());
                nsig->undoChangeProperty(Pid::DENOMINATOR_STRING, ts->denominatorString());
                nsig->undoChangeProperty(Pid::TIMESIG_STRETCH, ts->stretch());
                nsig->undoChangeProperty(Pid::GROUP_NODES, ts->groups().nodes());
                nsig->setSelected(false);
                nsig->setDropTarget(false);
            }
        }
    } else {
        Score* mScore = score->masterScore();
        Measure* mf  = mScore->tick2measure(tick);

        //
        // rewrite all measures up to the next time signature
        //
        if (mf == mScore->firstMeasure() && mf->nextMeasure() && (mf->ticks() != mf->timesig())) {
            // handle upbeat
            mf->undoChangeProperty(Pid::TIMESIG_NOMINAL, ns);
            Measure* m = mf->nextMeasure();
            Segment* s = m->findSegment(SegmentType::TimeSig, m->tick());
            mf = s ? 0 : mf->nextMeasure();
        } else {
            if (score->sigmap()->timesig(seg->tick().ticks()).nominal().identical(ns)) {
                // no change to global time signature,
                // but we need to rewrite any staves with local time signatures
                for (size_t i = 0; i < score->nstaves(); ++i) {
                    if (score->staff(i)->timeSig(tick) && score->staff(i)->timeSig(tick)->isLocal()) {
                        if (!rewriteMeasures(tx, mScore, mf, ns, i)) {
                            score->undoStack()->activeTransaction()->unwind();
                            return;
                        }
                    }
                }
                mf = 0;
            }
        }

        // try to rewrite the measures first
        // we will only add time signatures if this succeeds
        // this means, however, that the rewrite cannot depend on the time signatures being in place
        if (mf) {
            auto staffIdxRangeOnMaster = getStaffIdxRange(mScore);
            if (staffIdxRangeOnMaster.second != staffIdxRangeOnMaster.first
                && !rewriteMeasures(tx, mScore, mf, ns, local ? staffIdxRangeOnMaster.first : muse::nidx)) {
                score->undoStack()->activeTransaction()->unwind();
                return;
            }
        }
        // add the time signatures
        std::map<track_idx_t, TimeSig*> masterTimeSigs;
        for (Score* lscore : score->scoreList()) {
            Measure* nfm = lscore->tick2measure(tick);
            seg = nfm->undoGetSegment(SegmentType::TimeSig, nfm->tick());
            std::pair<staff_idx_t, staff_idx_t> staffIdxRange = getStaffIdxRange(lscore);
            for (staff_idx_t si = staffIdxRange.first; si < staffIdxRange.second; ++si) {
                if (fm->isMeasureRepeatGroup(si)) {
                    score->deleteItem(fm->measureRepeatElement(si));
                }
                TimeSig* nsig = toTimeSig(seg->element(si * VOICES));
                if (nsig == 0) {
                    nsig = Factory::copyTimeSig(*ts);
                    nsig->setScore(lscore);
                    nsig->setTrack(si * VOICES);
                    nsig->setParent(seg);
                    nsig->styleChanged();
                    score->undoAddElement(nsig);
                    if (lscore->excerpt()) {
                        const track_idx_t masterTrack = muse::key(lscore->excerpt()->tracksMapping(), nsig->track());
                        TimeSig* masterTimeSig = masterTimeSigs[masterTrack];
                        if (masterTimeSig) {
                            tx.push(new Link(masterTimeSig, nsig));
                        }
                    }
                } else {
                    nsig->undoChangeProperty(Pid::SHOW_COURTESY, ts->showCourtesySig());
                    nsig->undoChangeProperty(Pid::TIMESIG_TYPE, int(ts->timeSigType()));
                    nsig->undoChangeProperty(Pid::TIMESIG, ts->sig());
                    nsig->undoChangeProperty(Pid::NUMERATOR_STRING, ts->numeratorString());
                    nsig->undoChangeProperty(Pid::DENOMINATOR_STRING, ts->denominatorString());

                    // HACK do it twice to accommodate undo
                    nsig->undoChangeProperty(Pid::TIMESIG_TYPE, int(ts->timeSigType()));
                    nsig->undoChangeProperty(Pid::TIMESIG_STRETCH, ts->stretch());
                    nsig->undoChangeProperty(Pid::GROUP_NODES, ts->groups().nodes());
                    nsig->setSelected(false);
                    nsig->setDropTarget(false);                 // DEBUG
                }

                if (lscore->isMaster()) {
                    masterTimeSigs[nsig->track()] = nsig;
                }
            }
        }
    }
    delete ts;
}

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void EditTimeSig::removeTimeSig(Transaction& tx, Score* score, TimeSig* ts)
{
    Measure* m = ts->measure();
    Segment* s = ts->segment();

    //
    // we cannot remove a courtesy time signature
    //
    if (s->isCourtesySegment()) {
        return;
    }
    Fraction tick = m->tick();

    // if we remove all time sigs from segment, segment will be already removed by now
    // but this would leave us no means of detecting that we have measures in a local timesig
    // in cases where we try deleting the local time sig
    // known bug: this means we do not correctly detect non-empty measures when deleting global timesig change after a local one
    // see http://musescore.org/en/node/51596
    // Delete the time sig segment from the root score, we will rewriteMeasures from it
    // since it contains all the music while the part doesn't
    Score* rScore = score->masterScore();
    Measure* rm = rScore->tick2measure(m->tick());
    Segment* rs = rm->findSegment(SegmentType::TimeSig, s->tick());
    if (rs) {
        rScore->undoRemoveElement(rs);
    }
    // Measure can contain mmRest that can have its own timesig. We need to delete it too
    if (rm->mmRest()) {
        Segment* mmRestTimesig = rm->mmRest()->findSegment(SegmentType::TimeSig, s->tick());
        if (mmRestTimesig) {
            rScore->undoRemoveElement(mmRestTimesig);
        }
    }

    Measure* pm = m->prevMeasure();
    Fraction ns(pm ? pm->timesig() : Fraction(4, 4));

    if (!rewriteMeasures(tx, rScore, rm, ns, muse::nidx)) {
        score->undoStack()->activeTransaction()->unwind();
    } else {
        m = score->tick2measure(tick);           // old m may have been replaced
        // hack: fix measure rest durations for staves with local time signatures
        // if a time signature was deleted to reveal a previous local one,
        // then rewriteMeasures() got the measure rest durations wrong
        // (if we fixed it to work for delete, it would fail for add)
        // so we will fix measure rest durations here
        // TODO: fix rewriteMeasures() to get this right
        for (size_t i = 0; i < score->nstaves(); ++i) {
            TimeSig* tsig = score->staff(i)->timeSig(tick);
            if (tsig && tsig->isLocal()) {
                for (Measure* nm = m; nm; nm = nm->nextMeasure()) {
                    // stop when time signature changes
                    if (score->staff(i)->timeSig(nm->tick()) != tsig) {
                        break;
                    }
                    // fix measure rest duration
                    ChordRest* cr = nm->findChordRest(nm->tick(), i * VOICES);
                    if (cr && cr->isRest() && cr->durationType() == DurationType::V_MEASURE) {
                        cr->undoChangeProperty(Pid::DURATION, nm->stretchedLen(score->staff(i)));
                    }
                    //cr->setTicks(nm->stretchedLen(staff(i)));
                }
            }
        }
    }
}
