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

#include "implodeexplode.h"

#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/measure.h"
#include "../dom/mscore.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../dom/utils.h"
#include "../rw/xmlreader.h"

#include "clonevoice.h"

using namespace mu::engraving;

bool ImplodeExplode::explode(Score* score)
{
    size_t srcStaff  = score->selection().staffStart();
    size_t lastStaff = score->selection().staffEnd();
    size_t srcTrack  = srcStaff * VOICES;

    // reset selection to top staff only
    // force complete measures
    Segment* startSegment = score->selection().startSegment();
    Segment* endSegment   = score->selection().endSegment();
    Measure* startMeasure = startSegment->measure();
    Measure* endMeasure   = nullptr;
    if (!endSegment) {
        endMeasure = score->lastMeasure();
    } else if (endSegment->tick() == endSegment->measure()->tick()) {
        endMeasure = endSegment->measure()->prevMeasure() ? endSegment->measure()->prevMeasure() : score->firstMeasure();
    } else {
        endMeasure = endSegment->measure();
    }

    Fraction lTick = endMeasure->endTick();
    bool voice = false;

    for (Measure* m = startMeasure; m && m->tick() != lTick; m = m->nextMeasure()) {
        if (m->hasVoices(srcStaff)) {
            voice = true;
            break;
        }
    }
    if (!voice) {
        // force complete measures
        score->deselectAll();
        score->select(startMeasure, SelectType::RANGE, srcStaff);
        score->select(endMeasure, SelectType::RANGE, srcStaff);
        startSegment = score->selection().startSegment();
        endSegment = score->selection().endSegment();
        if (srcStaff == lastStaff - 1) {
            // only one staff was selected up front - determine number of staves
            // loop through all chords looking for maximum number of notes
            int n = 0;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                EngravingItem* e = s->element(srcTrack);
                if (e && e->isChord()) {
                    Chord* c = toChord(e);
                    n = std::max(n, int(c->notes().size()));
                    for (Chord* graceChord : c->graceNotes()) {
                        n = std::max(n, int(graceChord->notes().size()));
                    }
                }
            }
            lastStaff = std::min(score->nstaves(), srcStaff + n);
        }

        // Check that all source and dest measures have the same time stretch - allows explode within a local time signature,
        // but don't yet support it between differing local time signatures.
        Fraction timeStretch(1, 0);
        for (Measure* m = startMeasure; m && m->tick() <= endMeasure->tick(); m = m->nextMeasure()) {
            for (size_t staffIdx = srcStaff; staffIdx < lastStaff; ++staffIdx) {
                Fraction mTimeStretch = score->staff(staffIdx)->timeStretch(m->tick());
                if (!timeStretch.isValid()) {
                    timeStretch = mTimeStretch;
                } else if (timeStretch != mTimeStretch) {
                    MScore::setError(MsError::CANNOT_EXPLODE_IMPLODE_LOCAL_TIMESIG);
                    return false;
                }
            }
        }

        const muse::ByteArray mimeData(score->selection().mimeData());
        // copy to all destination staves
        Segment* firstCRSegment = startMeasure->tick2segment(startMeasure->tick());
        for (size_t i = 1; srcStaff + i < lastStaff; ++i) {
            track_idx_t track = (srcStaff + i) * VOICES;
            ChordRest* cr = toChordRest(firstCRSegment->element(track));
            if (cr) {
                XmlReader e(mimeData);
                score->pasteStaff(e, cr->segment(), cr->staffIdx());
            }
        }

        auto doExplode = [score](Chord* c, size_t lastStaff, size_t srcStaff, size_t i) -> void
        {
            std::vector<Note*> notes = c->notes();
            size_t nnotes = notes.size();
            // keep note "i" from top, which is backwards from nnotes - 1
            // reuse notes if there are more instruments than notes
            size_t stavesPerNote = std::max((lastStaff - srcStaff) / nnotes, static_cast<size_t>(1));
            size_t keepIndex = static_cast<size_t>(std::max(static_cast<int>(nnotes) - 1 - static_cast<int>(i / stavesPerNote), 0));
            Note* keepNote = c->notes()[keepIndex];
            for (Note* n : notes) {
                if (n != keepNote) {
                    score->undoRemoveElement(n);
                }
            }
        };

        // loop through each staff removing all but one note from each chord
        for (size_t i = 0; srcStaff + i < lastStaff; ++i) {
            track_idx_t track = (srcStaff + i) * VOICES;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                EngravingItem* e = s->element(track);
                if (e && e->isChord()) {
                    Chord* c = toChord(e); //chord, laststaff, srcstaff
                    doExplode(c, lastStaff, srcStaff, i);
                    for (Chord* graceChord : c->graceNotes()) {
                        doExplode(graceChord, lastStaff, srcStaff, i);
                    }
                }
            }
        }
    } else {
        track_idx_t sTracks[VOICES];
        track_idx_t dTracks[VOICES];
        if (srcStaff == lastStaff - 1) {
            lastStaff = std::min(score->nstaves(), srcStaff + VOICES);
        }

        for (voice_idx_t i = 0; i < VOICES; i++) {
            sTracks[i] = muse::nidx;
            dTracks[i] = muse::nidx;
        }
        int full = 0;

        for (Segment* seg = startSegment; seg && seg->tick() < lTick; seg = seg->next1()) {
            for (track_idx_t i = srcTrack; i < srcTrack + VOICES && full != VOICES; i++) {
                bool t = true;
                for (voice_idx_t j = 0; j < VOICES; j++) {
                    if (i == sTracks[j]) {
                        t = false;
                        break;
                    }
                }

                if (!seg->measure()->hasVoice(i) || seg->measure()->isOnlyRests(i) || !t) {
                    continue;
                }
                sTracks[full] = i;

                for (size_t j = srcTrack + full * VOICES; j < lastStaff * VOICES; j++) {
                    if (i == j) {
                        dTracks[full] = j;
                        break;
                    }
                    for (Measure* m = seg->measure(); m && m->tick() < lTick; m = m->nextMeasure()) {
                        if (!m->hasVoice(j) || (m->hasVoice(j) && m->isOnlyRests(j))) {
                            dTracks[full] = j;
                        } else {
                            dTracks[full] = muse::nidx;
                            break;
                        }
                    }
                    if (dTracks[full] != muse::nidx) {
                        break;
                    }
                }
                full++;
            }
        }

        IF_ASSERT_FAILED(full > 0) {
            return false;
        }
        lastStaff = track2staff(dTracks[full - 1]) + 1;

        // Check that all source and dest measures have the same time stretch - allows explode within a local time signature,
        // but don't yet support it between differing local time signatures.
        Fraction timeStretch(1, 0);
        for (Measure* m = startMeasure; m && m->tick() <= endMeasure->tick(); m = m->nextMeasure()) {
            for (size_t staffIdx = srcStaff; staffIdx < lastStaff; ++staffIdx) {
                Fraction mTimeStretch = score->staff(staffIdx)->timeStretch(m->tick());
                if (!timeStretch.isValid()) {
                    timeStretch = mTimeStretch;
                } else if (timeStretch != mTimeStretch) {
                    MScore::setError(MsError::CANNOT_EXPLODE_IMPLODE_LOCAL_TIMESIG);
                    return false;
                }
            }
        }

        for (track_idx_t i = srcTrack, j = 0; i < lastStaff * VOICES && j < VOICES; i += VOICES, j++) {
            track_idx_t strack = sTracks[j % VOICES];
            track_idx_t dtrack = dTracks[j % VOICES];
            if (strack != muse::nidx && strack != dtrack && dtrack != muse::nidx) {
                CloneVoice::cloneVoice(startSegment, lTick, startSegment, strack, dtrack, true, false);
            }
        }
    }

    // select exploded region
    score->deselectAll();
    score->select(startMeasure, SelectType::RANGE, srcStaff);
    score->select(endMeasure, SelectType::RANGE, lastStaff - 1);

    return true;
}

bool ImplodeExplode::implode(Score* score)
{
    staff_idx_t dstStaff   = score->selection().staffStart();
    staff_idx_t endStaff   = score->selection().staffEnd();
    track_idx_t dstTrack   = dstStaff * VOICES;
    track_idx_t startTrack = dstStaff * VOICES;
    track_idx_t endTrack   = endStaff * VOICES;
    Segment* startSegment = score->selection().startSegment();
    Segment* endSegment = score->selection().endSegment();
    Measure* startMeasure = startSegment->measure();
    Measure* endMeasure = endSegment ? endSegment->measure() : score->lastMeasure();
    Fraction startTick       = startSegment->tick();
    Fraction endTick         = endSegment ? endSegment->tick() : score->lastMeasure()->endTick();
    assert(startMeasure && endMeasure);

    // Check that all source and dest measures have the same time stretch - allows implode within a local time signature,
    // but don't yet support it between differing local time signatures.
    Fraction timeStretch(1, 0);
    for (Measure* m = startMeasure; m && m->tick() <= endMeasure->tick(); m = m->nextMeasure()) {
        for (size_t staffIdx = dstStaff; staffIdx < endStaff; ++staffIdx) {
            Fraction mTimeStretch = score->staff(staffIdx)->timeStretch(m->tick());
            if (!timeStretch.isValid()) {
                timeStretch = mTimeStretch;
            } else if (timeStretch != mTimeStretch) {
                MScore::setError(MsError::CANNOT_EXPLODE_IMPLODE_LOCAL_TIMESIG);
                return false;
            }
        }
    }

    // if single staff selected, combine voices
    // otherwise combine staves
    if (dstStaff == endStaff - 1) {
        // loop through segments adding notes to chord on top staff
        for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
            if (!s->isChordRestType()) {
                continue;
            }
            EngravingItem* dst = s->element(dstTrack);
            if (dst && dst->isChord()) {
                Chord* dstChord = toChord(dst);
                // see if we are tying in to this chord
                Chord* tied = 0;
                for (Note* n : dstChord->notes()) {
                    if (n->tieBackNonPartial()) {
                        tied = n->tieBack()->startNote()->chord();
                        break;
                    }
                }
                // loop through each subsequent staff (or track within staff)
                // looking for notes to add
                for (track_idx_t srcTrack = startTrack + 1; srcTrack < endTrack; srcTrack++) {
                    EngravingItem* src = s->element(srcTrack);
                    if (src && src->isChord()) {
                        Chord* srcChord = toChord(src);
                        // when combining voices, skip if not same duration
                        if (srcChord->ticks() != dstChord->ticks()) {
                            continue;
                        }
                        // add notes
                        for (Note* n : srcChord->notes()) {
                            NoteVal nv(n->pitch());
                            nv.tpc1 = n->tpc1();
                            // skip duplicates
                            if (dstChord->findNote(nv.pitch)) {
                                continue;
                            }
                            bool forceAccidental = n->accidental() && n->accidental()->role() == AccidentalRole::USER;
                            Note* nn = score->addNote(dstChord, nv, forceAccidental);
                            // add tie to this note if original chord was tied
                            if (tied) {
                                // find note to tie to
                                for (Note* tn : tied->notes()) {
                                    if (nn->pitch() == tn->pitch() && nn->tpc() == tn->tpc() && !tn->tieFor()) {
                                        // found note to tie
                                        Tie* tie = Factory::createTie(score->dummy());
                                        tie->setStartNote(tn);
                                        tie->setEndNote(nn);
                                        tie->setTick(tie->startNote()->tick());
                                        tie->setTick2(tie->endNote()->tick());
                                        tie->setTrack(tn->track());
                                        score->undoAddElement(tie);
                                    }
                                }
                            }
                        }
                    }
                    // delete chordrest from source track if possible
                    if (src && src->voice()) {
                        score->undoRemoveElement(src);
                    }
                }
            }
            // TODO - use first voice that actually has a note and implode remaining voices on it?
            // see https://musescore.org/en/node/174111
            else if (dst) {
                // destination track has something, but it isn't a chord
                // remove rests from other voices if in "voice mode"
                for (voice_idx_t i = 1; i < VOICES; ++i) {
                    EngravingItem* e = s->element(dstTrack + i);
                    if (e && e->isRest()) {
                        score->undoRemoveElement(e);
                    }
                }
            }
        }
        // delete orphaned spanners (TODO: figure out solution to reconnect orphaned spanners to their cloned notes)
        score->checkSpanner(startTick, endTick);
    } else {
        track_idx_t tracks[VOICES];
        for (voice_idx_t i = 0; i < VOICES; i++) {
            tracks[i] = muse::nidx;
        }
        voice_idx_t full = 0;

        // identify tracks to combine, storing the source track numbers in tracks[]
        // first four non-empty tracks to win
        for (track_idx_t track = startTrack; track < endTrack && full < VOICES; ++track) {
            Measure* m = startMeasure;
            do {
                if (m->hasVoice(track) && !m->isOnlyRests(track)) {
                    tracks[full++] = track;
                    break;
                }
            } while ((m != endMeasure) && (m = m->nextMeasure()));
        }

        // clone source tracks into destination
        for (track_idx_t i = dstTrack; i < dstTrack + VOICES; i++) {
            track_idx_t strack = tracks[i % VOICES];
            if (strack != muse::nidx && strack != i) {
                CloneVoice::cloneVoice(startSegment, endTick, startSegment, strack, i, false, false);
            }
        }
    }

    // select destination staff only
    score->deselectAll();
    score->select(startMeasure, SelectType::RANGE, dstStaff);
    score->select(endMeasure, SelectType::RANGE, dstStaff);

    return true;
}
