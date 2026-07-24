/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "editclef.h"

#include "../dom/chordrest.h"
#include "../dom/clef.h"
#include "../dom/factory.h"
#include "../dom/instrument.h"
#include "../dom/linkedobjects.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"

#include "log.h"

using namespace mu::engraving;

// compute line position of noteheads after clef change
static void updateNoteLines(Segment* segment, track_idx_t track)
{
    Staff* staff = segment->score()->staff(track / VOICES);
    if (staff->isDrumStaff(segment->tick()) || staff->isTabStaff(segment->tick())) {
        return;
    }
    for (Segment* s = segment->next1(); s; s = s->next1()) {
        if ((s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef)) && s->element(track) && !s->element(track)->generated()) {
            break;
        }
        if (!s->isChordRestType()) {
            continue;
        }
        for (track_idx_t t = track; t < track + VOICES; ++t) {
            EngravingItem* e = s->element(t);
            if (e && e->isChord()) {
                Chord* chord = toChord(e);
                for (Note* n : chord->notes()) {
                    n->updateLine();
                }
                chord->sortNotes();
                for (Chord* gc : chord->graceNotes()) {
                    for (Note* gn : gc->notes()) {
                        gn->updateLine();
                    }
                    gc->sortNotes();
                }
            }
        }
    }
}

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

ChangeClefType::ChangeClefType(Clef* c, ClefType cl, ClefType tc)
{
    clef            = c;
    concertClef     = cl;
    transposingClef = tc;
}

void ChangeClefType::flip()
{
    ClefType ocl = clef->concertClef();
    ClefType otc = clef->transposingClef();

    clef->setConcertClef(concertClef);
    clef->setTransposingClef(transposingClef);

    clef->staff()->setClef(clef);
    Segment* segment = clef->segment();
    updateNoteLines(segment, clef->track());
    clef->triggerLayoutAll(); // TODO: reduce layout to clef range

    concertClef     = ocl;
    transposingClef = otc;
}

//---------------------------------------------------------
//   EditClef
//---------------------------------------------------------

bool EditClef::canInsertClef(const Score* score, ClefType type)
{
    if (type == ClefType::INVALID) {
        return false;
    }

    const Staff* staff = score->staff(score->inputState().track() / VOICES);
    const ChordRest* cr = score->inputState().cr();

    return staff && cr;
}

void EditClef::insertClef(Transaction& tx, Score* score, ClefType type)
{
    undoChangeClef(tx, score, score->staff(score->inputState().track() / VOICES), score->inputState().cr(), type);
}

//---------------------------------------------------------
//   insertClef
//    insert clef before cr
//---------------------------------------------------------

void EditClef::insertClef(Transaction& tx, Score* score, Clef* clef, ChordRest* cr)
{
    undoChangeClef(tx, score, cr->staff(), cr, clef->clefType());
    delete clef;
}

//---------------------------------------------------------
//   undoChangeClef
//    change clef if e is a clef
//    else
//    create a clef before element e
//---------------------------------------------------------

void EditClef::undoChangeClef(Transaction&, Score* score, Staff* ostaff, EngravingItem* e, ClefType ct, bool forInstrumentChange,
                              Clef* clefToRelink)
{
    IF_ASSERT_FAILED(ostaff && e) {
        return;
    }

    bool moveClef = false;
    SegmentType st = SegmentType::Clef;
    if (e->isMeasure()) {
        if (toMeasure(e)->prevMeasure()) {
            moveClef = true;
        } else {
            st = SegmentType::HeaderClef;
        }
    } else if (e->isClef()) {
        Clef* clef = toClef(e);
        if (clef->segment()->isHeaderClefType()) {
            if (clef->measure()->prevMeasure()) {
                moveClef = true;
            } else {
                st = SegmentType::HeaderClef;
            }
        } else if (clef->rtick() == clef->measure()->ticks()) {
            moveClef = true;
        }
    } else if (e->rtick() == Fraction(0, 1)) {
        Measure* curMeasure = e->findMeasure();
        Measure* prevMeasure = curMeasure ? curMeasure->prevMeasure() : nullptr;
        if (prevMeasure && !prevMeasure->sectionBreak()) {
            moveClef = true;
        }
    }

    bool concertPitch = score->style().styleB(Sid::concertPitch);
    Clef* gclef = 0;
    Fraction tick = e->tick();
    Fraction rtick = e->rtick();
    bool isSmall = (st == SegmentType::Clef);
    for (Staff* staff : ostaff->staffList()) {
        if (clefToRelink && ostaff == staff) {
            continue;
        }

        Score* staffScore = staff->score();
        Measure* measure = staffScore->tick2measure(tick);

        if (!measure) {
            LOGW("measure for tick %d not found!", tick.ticks());
            continue;
        }

        Segment* destSeg;
        Fraction rt;
        if (moveClef) {                // if at start of measure and there is a previous measure
            measure = measure->prevMeasure();
            rt      = measure->ticks();
        } else {
            rt = rtick;
        }
        destSeg = measure->undoGetSegmentR(st, rt);

        staff_idx_t staffIdx = staff->idx();
        track_idx_t track    = staffIdx * VOICES;
        Clef* clef   = toClef(destSeg->element(track));

        StaffType* staffType = staff->staffType(e->tick());
        StaffGroup staffGroup = staffType->group();
        if (ClefInfo::staffGroup(ct) != staffGroup && !forInstrumentChange) {
            continue;
        }

        if (clef) {
            //
            // for transposing instruments, differentiate
            // clef type for concertPitch
            //
            Instrument* i = staff->part()->instrument(tick);
            ClefType cp, tp;
            if (i->transpose().isZero()) {
                cp = ct;
                tp = ct;
            } else {
                if (concertPitch) {
                    cp = ct;
                    tp = clef->transposingClef();
                } else {
                    cp = clef->concertClef();
                    tp = ct;
                }
            }
            clef->setGenerated(false);
            staffScore->undo(new ChangeClefType(clef, cp, tp));
            Clef* oClef = clef->otherClef();
            if (oClef && !(oClef->generated())) {
                staffScore->undo(new ChangeClefType(oClef, cp, tp));
            }
            // change the clef in the mmRest if any
            if (measure->hasMMRest()) {
                Measure* mmMeasure = measure->mmRest();
                Segment* mmDestSeg = mmMeasure->findSegment(SegmentType::Clef, tick);
                if (mmDestSeg) {
                    Clef* mmClef = toClef(mmDestSeg->element(clef->track()));
                    if (mmClef) {
                        staffScore->undo(new ChangeClefType(mmClef, cp, tp));
                    }
                }
            }
        } else {
            if (gclef) {
                clef = toClef(gclef->linkedClone());
                clef->setScore(staffScore);
            } else {
                clef = Factory::createClef(staffScore->dummy()->segment());
                clef->setClefType(ct);
                gclef = clef;
            }
            clef->setTrack(track);
            clef->setParent(destSeg);
            clef->setIsHeader(st == SegmentType::HeaderClef);
            staffScore->doUndoAddElement(clef);
        }
        if (forInstrumentChange) {
            clef->setForInstrumentChange(true);
        }
        clef->setSmall(isSmall);

        if (clefToRelink) {
            LinkedObjects* links = clef->links();
            if (!links) {
                clef->linkTo(clefToRelink);
            } else if (!clef->isLinked(clefToRelink)) {
                clefToRelink->setLinks(links);
                links->push_back(clefToRelink);
            }
        }
    }
}
