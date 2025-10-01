/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editvoicing.h"

#include "dom/engravingitem.h"
#include "dom/measure.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

ExchangeVoice::ExchangeVoice(Measure* m, track_idx_t _val1, track_idx_t _val2, staff_idx_t _staff)
{
    measure = m;
    val1    = _val1;
    val2    = _val2;
    staff   = _staff;
}

void ExchangeVoice::undo(EditData*)
{
    measure->exchangeVoice(val2, val1, staff);
    measure->checkMultiVoices(staff);
}

void ExchangeVoice::redo(EditData*)
{
    measure->exchangeVoice(val1, val2, staff);
}

//---------------------------------------------------------
//   CloneVoice
//---------------------------------------------------------

CloneVoice::CloneVoice(Segment* _sf, const Fraction& _lTick, Segment* _d, track_idx_t _strack, track_idx_t _dtrack, track_idx_t _otrack,
                       bool _linked)
{
    sourceSeg = _sf;            // first source segment
    lTick     = _lTick;         // last tick to clone
    destSeg   = _d;             // first destination segment
    strack    = _strack;
    dtrack    = _dtrack;
    otrack    = _otrack;        // old source track if -1 delete voice in strack after copy
    linked    = _linked;        // if true  add elements in destination segment only
                                // if false add elements in every linked staff
}

void CloneVoice::undo(EditData*)
{
    Score* s = destSeg->score();
    Fraction ticks = destSeg->tick() + lTick - sourceSeg->tick();
    track_idx_t sTrack = otrack == muse::nidx ? dtrack : otrack;   // use the correct source / destination if deleting the source
    track_idx_t dTrack = otrack == muse::nidx ? strack : dtrack;

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    if (otrack != muse::nidx && linked) {
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(dTrack);
            if (el && el->isChordRest()) {
                el->unlink();
                seg->setElement(dTrack, nullptr);
            }
        }
    }

    if (otrack == muse::nidx && !linked) {
        // On the first run get going the undo redo action for adding/deleting elements and slurs
        if (first) {
            s->cloneVoice(sTrack, dTrack, sourceSeg, ticks, linked);
            auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                Spanner* sp = i->value;
                if (sp->isSlur() && (sp->track() == sTrack || sp->track2() == sTrack)) {
                    s->undoRemoveElement(sp);
                }
                if (sp->isHairpin() && (sp->track() == dTrack || sp->track2() == dTrack)) {
                    s->undoRemoveElement(sp);
                }
            }
            for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
                EngravingItem* el = seg->element(sTrack);
                if (el && el->isChordRest()) {
                    s->undoRemoveElement(el);
                }

                const std::vector<EngravingItem*> annotations = seg->annotations();
                for (EngravingItem* annotation : annotations) {
                    if (annotation && annotation->hasVoiceAssignmentProperties() && annotation->track() == dTrack) {
                        // Remove extra all voice annotations which have been created
                        VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                        if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                            s->undoRemoveElement(annotation);
                        }
                    }
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(sTrack % VOICES)) {
            s->setRest(destSeg->tick(), sTrack, ticks, false, nullptr);
        }
    } else {
        s->cloneVoice(sTrack, dTrack, sourceSeg, ticks, linked);
        if (!linked && !(dTrack % VOICES)) {
            s->setRest(destSeg->tick(), dTrack, ticks, false, nullptr);
        }

        // Remove annotations
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            const std::vector<EngravingItem*> annotations = seg->annotations();
            for (EngravingItem* annotation : annotations) {
                if (annotation && annotation->hasVoiceAssignmentProperties() && annotation->track() == dTrack) {
                    // Remove extra all voice annotations which have been created
                    VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                    if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                        s->undoRemoveElement(annotation);
                    }
                }
            }
        }

        auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
        for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp = i->value;
            if (sp->hasVoiceAssignmentProperties() && (sp->track() == dTrack || sp->track2() == dTrack)) {
                // Remove extra all voice annotations which have been created
                VoiceAssignment voiceAssignment = sp->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                    s->undoRemoveElement(sp);
                }
            }
        }
    }

    first = false;
}

void CloneVoice::redo(EditData*)
{
    Score* s = destSeg->score();
    Fraction ticks = destSeg->tick() + lTick - sourceSeg->tick();

    // Clear destination voice (in case of not linked and otrack = -1 we would delete our source
    if (otrack != muse::nidx && linked) {
        for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
            EngravingItem* el = seg->element(dtrack);
            if (el && el->isChordRest()) {
                el->unlink();
                seg->setElement(dtrack, nullptr);
            }
        }
    }

    if (otrack == muse::nidx && !linked) {
        // On the first run get going the undo redo action for adding/deleting elements and slurs
        if (first) {
            s->cloneVoice(strack, dtrack, sourceSeg, ticks, linked);
            auto spanners = s->spannerMap().findOverlapping(sourceSeg->tick().ticks(), lTick.ticks());
            for (auto i = spanners.begin(); i < spanners.end(); i++) {
                Spanner* sp = i->value;
                if (sp->isSlur() && (sp->track() == strack || sp->track2() == strack)) {
                    s->undoRemoveElement(sp);
                }
            }
            for (Segment* seg = destSeg; seg && seg->tick() < ticks; seg = seg->next1()) {
                EngravingItem* el = seg->element(strack);
                if (el && el->isChordRest()) {
                    s->undoRemoveElement(el);
                }

                const std::vector<EngravingItem*> annotations = seg->annotations();
                for (EngravingItem* annotation : annotations) {
                    if (annotation && annotation->track() == strack) {
                        if (annotation->hasVoiceAssignmentProperties()) {
                            VoiceAssignment voiceAssignment = annotation->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
                            if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                                continue;
                            }
                        }
                        s->undoRemoveElement(annotation);
                    }
                }
            }
        }
        // Set rests if first voice in a staff
        if (!(strack % VOICES)) {
            s->setRest(destSeg->tick(), strack, ticks, false, nullptr);
        }
    } else {
        s->cloneVoice(strack, dtrack, sourceSeg, ticks, linked, first);
    }
}
