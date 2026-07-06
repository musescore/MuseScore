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

#include "editslashnotation.h"

#include "noteinput.h"

#include "containers.h"

#include "../dom/chord.h"
#include "../dom/durationtype.h"
#include "../dom/input.h"
#include "../dom/linkedobjects.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   slashFill
///   fills selected region with slashes
//---------------------------------------------------------

void EditSlashNotation::slashFill(Transaction&, Score* score)
{
    staff_idx_t startStaff = score->selection().staffStart();
    staff_idx_t endStaff = score->selection().staffEnd();
    Segment* startSegment = score->selection().startSegment();
    if (!startSegment) { // empty score?
        return;
    }

    Segment* endSegment = score->selection().endSegment();

    // operate on measures underlying mmrests
    if (startSegment && startSegment->measure() && startSegment->measure()->isMMRest()) {
        startSegment = startSegment->measure()->mmRestFirst()->first();
    }
    if (endSegment && endSegment->measure() && endSegment->measure()->isMMRest()) {
        endSegment = endSegment->measure()->mmRestLast()->last();
    }

    Fraction endTick = endSegment ? endSegment->tick() : score->lastSegment()->tick() + Fraction::eps();
    Chord* firstSlash = 0;
    Chord* lastSlash = 0;

    // loop through staves in selection
    for (staff_idx_t staffIdx = startStaff; staffIdx < endStaff; ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        voice_idx_t voice = muse::nidx;
        // loop through segments adding slashes on each beat
        for (Segment* s = startSegment; s && s->tick() < endTick; s = s->next1()) {
            if (s->segmentType() != SegmentType::ChordRest) {
                continue;
            }
            // determine beat type based on time signature
            int d = s->measure()->timesig().denominator();
            int n = (d > 4 && s->measure()->timesig().numerator() % 3 == 0) ? 3 : 1;
            Fraction f(n, d);
            // skip over any leading segments before next (first) beat
            if (s->rtick().ticks() % f.ticks()) {
                continue;
            }
            // determine voice to use - first available voice for this measure / staff
            if (voice == muse::nidx || s->rtick().isZero()) {
                bool needGap[VOICES];
                for (voice = 0; voice < VOICES; ++voice) {
                    needGap[voice] = false;
                    ChordRest* cr = toChordRest(s->element(track + voice));
                    // no chordrest == treat as ordinary rest for purpose of determining availability of voice
                    // but also, we will need to make a gap for this voice if we do end up choosing it
                    if (!cr) {
                        needGap[voice] = true;
                    }
                    // chord == keep looking for an available voice
                    else if (cr->isChord()) {
                        continue;
                    }
                    // full measure rest == OK to use voice
                    else if (cr->durationType() == DurationType::V_MEASURE) {
                        break;
                    }
                    // no chordrest or ordinary rest == OK to use voice
                    // if there are nothing but rests for duration of measure / selection
                    bool ok = true;
                    for (Segment* ns = s->next(SegmentType::ChordRest); ns && ns != endSegment; ns = ns->next(SegmentType::ChordRest)) {
                        ChordRest* ncr = toChordRest(ns->element(track + voice));
                        if (ncr && ncr->isChord()) {
                            ok = false;
                            break;
                        }
                    }
                    if (ok) {
                        break;
                    }
                }
                // no available voices, just use voice 0
                if (voice == VOICES) {
                    voice = 0;
                }
                // no cr was found in segment for this voice, so make gap
                if (needGap[voice]) {
                    score->makeGapVoice(s, track + voice, f, s->tick());
                }
            }
            // construct note
            int line = 0;
            bool error = false;
            NoteVal nv;
            if (score->staff(staffIdx)->staffType(s->tick())->group() == StaffGroup::TAB) {
                line = score->staff(staffIdx)->lines(s->tick()) / 2;
            } else {
                line = score->staff(staffIdx)->middleLine(s->tick());             // staff(staffIdx)->lines() - 1;
            }
            if (score->staff(staffIdx)->staffType(s->tick())->group() == StaffGroup::PERCUSSION) {
                nv.pitch = 0;
                nv.headGroup = NoteHeadGroup::HEAD_SLASH;
            } else {
                Position p;
                p.segment = s;
                p.staffIdx = staffIdx;
                p.line = line;
                p.fret = INVALID_FRET_INDEX;
                score->inputState().setRest(false);             // needed for tab
                nv = NoteInput::noteValForPosition(score, p, AccidentalType::NONE, error);
            }
            if (error) {
                continue;
            }
            // insert & turn into slash
            s = score->setNoteRest(s, track + voice, nv, f);
            Chord* c = toChord(s->element(track + voice));
            if (c) {
                if (c->links()) {
                    for (EngravingObject* e : *c->links()) {
                        Chord* lc = toChord(e);
                        lc->setSlash(true, true);
                    }
                } else {
                    c->setSlash(true, true);
                }
            }
            lastSlash = c;
            if (!firstSlash) {
                firstSlash = c;
            }
        }
    }

    // re-select the slashes
    score->deselectAll();
    if (firstSlash && lastSlash) {
        score->select(firstSlash, SelectType::RANGE);
        score->select(lastSlash, SelectType::RANGE);
    }
}

//---------------------------------------------------------
//   slashRhythm
///   converts rhythms in selected region to slashes
//---------------------------------------------------------

void EditSlashNotation::slashRhythm(Transaction&, Score* score)
{
    std::set<Chord*> chords;
    // loop through all notes in selection
    for (EngravingItem* e : score->selection().elements()) {
        if (e->voice() >= 2 && e->isRest()) {
            Rest* r = toRest(e);
            if (r->links()) {
                for (EngravingObject* se : *r->links()) {
                    Rest* lr = toRest(se);
                    lr->setAccent(!lr->accent());
                }
            } else {
                r->setAccent(!r->accent());
            }
            continue;
        } else if (e->isNote()) {
            Note* n = toNote(e);
            if (n->noteType() != NoteType::NORMAL) {
                continue;
            }
            Chord* c = n->chord();
            // check for duplicates (chords with multiple notes)
            if (muse::contains(chords, c)) {
                continue;
            }
            chords.insert(c);
            // toggle slash setting
            c->setSlash(!c->slash(), false);
        }
    }
}
