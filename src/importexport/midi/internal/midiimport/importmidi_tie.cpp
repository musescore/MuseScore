/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "importmidi_tie.h"

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"

#ifdef QT_DEBUG
#include "engraving/dom/staff.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#endif

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::midi {
namespace MidiTie {
static bool isTied(const Segment* seg, track_idx_t strack, voice_idx_t voice,
                   mu::engraving::Tie* (Note::* tieFunc)() const)
{
    ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));
    if (cr && cr->isChord()) {
        Chord* chord = toChord(cr);
        const auto& notes = chord->notes();
        for (const Note* note: notes) {
            if ((note->*tieFunc)()) {
                return true;
            }
        }
    }
    return false;
}

bool isTiedFor(const Segment* seg, track_idx_t strack, voice_idx_t voice)
{
    return isTied(seg, strack, voice, &Note::tieFor);
}

bool isTiedBack(const Segment* seg, track_idx_t strack, voice_idx_t voice)
{
    return isTied(seg, strack, voice, &Note::tieBack);
}

void TieStateMachine::addSeg(const Segment* seg, track_idx_t strack)
{
    bool isChord = false;
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));
        if (!cr || !cr->isChord()) {
            continue;
        }
        if (!isChord) {
            isChord = true;
        }

        bool tiedFor = isTiedFor(seg, strack, voice);
        bool tiedBack = isTiedBack(seg, strack, voice);

        if (tiedFor && !tiedBack) {
            tiedVoices.insert(voice);
        } else if (!tiedFor && tiedBack) {
            tiedVoices.erase(voice);
        }
    }
    if (!isChord) {
        return;
    }

    if (tiedVoices.empty() && (state_ == State::TIED_FOR
                               || state_ == State::TIED_BOTH)) {
        state_ = State::TIED_BACK;
    } else if (tiedVoices.empty() && state_ == State::TIED_BACK) {
        state_ = State::UNTIED;
    } else if (!tiedVoices.empty() && (state_ == State::TIED_BACK
                                       || state_ == State::UNTIED)) {
        state_ = State::TIED_FOR;
    } else if (!tiedVoices.empty() && state_ == State::TIED_FOR) {
        state_ = State::TIED_BOTH;
    }
}

#ifdef QT_DEBUG

static void printInconsistentTieLocation(int measureIndex, staff_idx_t staffIndex)
{
    LOGD() << "Ties are inconsistent; measure number (from 1):"
           << measureIndex + 1
           << ", staff index (from 0):" << staffIndex;
}

bool areTiesConsistent(const Staff* staff)
{
    const track_idx_t strack = staff->idx() * VOICES;

    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        bool isTie = false;
        for (Segment* seg = staff->score()->firstSegment(SegmentType::All); seg; seg = seg->next1()) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));

                if (cr && cr->isRest() && isTie) {
                    printInconsistentTieLocation(seg->measure()->no(), staff->idx());
                    return false;
                }
                if (isTiedBack(seg, strack, voice)) {
                    if (!isTie) {
                        printInconsistentTieLocation(seg->measure()->no(), staff->idx());
                        return false;
                    }
                    isTie = false;
                }
                if (isTiedFor(seg, strack, voice)) {
                    if (isTie) {
                        printInconsistentTieLocation(seg->measure()->no(), staff->idx());
                        return false;
                    }
                    isTie = true;
                }
            }
        }
        if (isTie) {
            return false;
        }
    }
    return true;
}

#endif
} // namespace MidiTie
} // namespace mu::iex::midi
