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

#include "realizechordsymbols.h"

#include <vector>

#include "style/style.h"

#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/harmony.h"
#include "../dom/instrument.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/part.h"
#include "../dom/realizedharmony.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/staff.h"
#include "../dom/tie.h"
#include "../dom/tuplet.h"
#include "../dom/utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   setChord
//    'dur' is in local (stretched) time
//    return segment of last created chord
//---------------------------------------------------------
static Segment* setChord(Score* score, Segment* segment, track_idx_t track, const Chord* chordTemplate, Fraction dur)
{
    assert(segment->segmentType() == SegmentType::ChordRest);

    Fraction tick = segment->tick();
    Chord* nr     = nullptr;   //current added chord used so we can select the last added chord and so we can apply ties
    std::vector<Tie*> tie(chordTemplate->notes().size());   //keep pointer to a tie for each note in the chord in case we need to tie notes
    ChordRest* cr = toChordRest(segment->element(track));   //chord rest under the segment for the specified track

    bool addTie = false;

    Measure* measure = nullptr;
    //keep creating chords and tieing them until we created the full duration asked for (dur)
    for (;;) {
        if (track % VOICES) {
            score->expandVoice(segment, track);
        }

        Tuplet* t = cr ? cr->tuplet() : 0;
        Fraction tDur = segment->ticks();
        Segment* seg = segment->next();

        //we need to get a correct subduration so that makeGap can function properly
        //since makeGap() takes "normal" duration rather than actual length
        while (seg) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                //design choice made to keep multiple notes across a tuplet as tied single notes rather than combining them
                //since it's arguably more readable, but the other code is still here (commented)
                ChordRest* testCr = toChordRest(seg->element(track));

                //code here allows us to combine tuplet realization together which I have opted not to do for readability (of the music)
                //if (!!t ^ (testCr && testCr->tuplet())) //stop if we started with a tuplet and reach something that's not a tuplet,
                //      break;                          //or start with not a tuplet and reach a tuplet

                if (testCr && testCr->tuplet()) {       //stop on tuplet
                    break;
                }
                tDur += seg->ticks();
            }
            if (tDur >= dur) {       //do not go further than the duration asked for
                tDur = dur;
                break;
            }
            seg = seg->next();       //iterate only across measure (hence usage of next() rather than next1())
        }
        if (t) {
            tDur *= t->ratio();       //scale by tuplet ratio to get "normal" length rather than actual length when dealing with tuplets
        }
        // the returned gap ends at the measure boundary or at tuplet end
        Fraction dd = score->makeGap(segment, track, tDur, t);

        if (dd.isZero()) {
            LOGD("cannot get gap at %d type: %d/%d", tick.ticks(), dur.numerator(),
                 dur.denominator());
            break;
        }

        measure = segment->measure();
        std::vector<TDuration> dl = toDurationList(dd, true);
        size_t n = dl.size();
        //add chord, tieing when necessary within measure
        for (size_t i = 0; i < n; ++i) {
            const TDuration& d = dl[i];

            //create new chord from template and add it
            Chord* chord = Factory::copyChord(*chordTemplate);
            nr = chord;

            chord->setTrack(track);
            chord->setDurationType(d);
            chord->setTicks(d.fraction());
            chord->setTuplet(t);
            score->undoAddCR(chord, measure, tick);
            //if there is something to tie, complete tie backwards
            //and add the tie to score
            const std::vector<Note*> notes = chord->notes();
            if (addTie) {
                for (size_t j = 0; j < notes.size(); ++j) {
                    tie[j]->setEndNote(notes[j]);
                    notes[j]->setTieBack(tie[j]);
                    score->undoAddElement(tie[j]);
                }
                addTie = false;
            }
            //if we're not the last element in the duration list,
            //set tie forward
            if (i + 1 < n) {
                for (size_t j = 0; j < notes.size(); ++j) {
                    tie[j] = Factory::createTie(score->dummy());
                    tie[j]->setStartNote(notes[j]);
                    tie[j]->setTick(tie[j]->startNote()->tick());
                    tie[j]->setTrack(track);
                    notes[j]->setTieFor(tie[j]);
                    addTie = true;
                }
            }
            score->setPlayChord(true);
            segment = chord->segment();
            tick += chord->actualTicks();
        }

        //subtract the duration already realized and move on
        if (t) {
            dur -= dd / t->ratio();
        } else {
            dur -= dd;
        }
        //we are done when there is no duration left to realize
        if (dur.isZero()) {
            break;
        }

        //go to next segment unless we are at the score (which means we will just be done there)
        Segment* nseg = score->tick2segment(tick, false, SegmentType::ChordRest);
        if (!nseg) {
            LOGD("reached end of score");
            break;
        }

        //it is possible that the next measure's ticks have not been computed yet. compute them now
        if (nseg->ticks().isZero()) {
            nseg->measure()->computeTicks();
        }

        segment = nseg;

        cr = toChordRest(segment->element(track));

        if (!cr) {
            if (track % VOICES) {
                cr = score->addRest(segment, track, TDuration(DurationType::V_MEASURE), 0);
            } else {
                LOGD("no rest in voice 0");
                break;
            }
        }
        //
        //  Note does not fit on current measure, create Tie to
        //  next part of note
        std::vector<Note*> notes = nr->notes();
        for (size_t i = 0; i < notes.size(); ++i) {
            tie[i] = Factory::createTie(score->dummy());
            tie[i]->setStartNote(notes[i]);
            tie[i]->setTick(tie[i]->startNote()->tick());
            tie[i]->setTrack(notes[i]->track());
            notes[i]->setTieFor(tie[i]);
        }
    }
    if (!tie.empty()) {
        score->connectTies();
    }
    if (nr) {
        score->select(nr, SelectType::SINGLE, 0);
    }
    return segment;
}

//---------------------------------------------------------
//   realizeChordSymbols
///   Realize selected chord symbols into notes on the staff.
///
///   If a voicing and duration type are specified, the
///   harmony voicing settings will be overridden by the
///   passed parameters. Otherwise, the settings set on the
///   harmony object will be used.
//---------------------------------------------------------

void RealizeChordSymbols::realizeChordSymbols(Transaction&, Score* score, bool literal, Voicing voicing, HDuration durationType)
{
    // Create copy, because setChord selects newly created chord and thus
    // modifies selection().elements() while we're iterating over it
    const std::vector<EngravingItem*> elist = score->selection().elements();

    for (EngravingItem* e : elist) {
        if (!e->isHarmony()) {
            continue;
        }
        Harmony* h = toHarmony(e);
        if (!h->isRealizable()) {
            continue;
        }
        const RealizedHarmony& r = h->getRealizedHarmony();
        Segment* seg = h->explicitParent()->isSegment() ? toSegment(h->explicitParent()) : toSegment(h->explicitParent()->explicitParent());
        Fraction tick = seg->tick();
        Fraction duration = r.getActualDuration(tick.ticks(), durationType) * h->staff()->timeStretch(tick);
        bool concertPitch = score->style().styleB(Sid::concertPitch);

        Chord* chord = Factory::createChord(score->dummy()->segment());     //chord template
        chord->setTrack(h->track());     //set track so notes have a track to sit on

        //create chord from notes
        RealizedHarmony::PitchMap notes;
        if (voicing == Voicing::INVALID || durationType == HDuration::INVALID) {
            notes = r.notes();       //no override, just use notes from realize harmony
        } else {
            //generate notes list based on overridden settings
            int offset = 0;
            Interval interval = h->staff()->part()->instrument(h->tick())->transpose();
            if (!concertPitch) {
                offset = interval.chromatic;
            }
            notes = r.generateNotes(h->rootTpc(), h->bassTpc(),
                                    literal, voicing, offset);
        }

        for (const auto& p : notes) {
            Note* note = Factory::createNote(chord);
            NoteVal nval;
            nval.pitch = p.first;
            if (concertPitch) {
                nval.tpc1 = p.second;
            } else {
                nval.tpc2 = p.second;
            }
            chord->add(note);       //add note first to set track and such
            note->setNval(nval, tick);
        }

        if (!seg->isChordRestType()) {
            Segment* newCrSeg = seg->measure()->undoGetSegment(SegmentType::ChordRest, seg->tick());
            newCrSeg->setTicks(seg->ticks());
            seg = newCrSeg;
        }

        setChord(score, seg, h->track(), chord, duration);     //add chord using template
        delete chord;
    }
}
