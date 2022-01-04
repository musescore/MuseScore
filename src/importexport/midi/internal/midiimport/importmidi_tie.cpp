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

#include "importmidi_tie.h"

#include <QDebug>

#include "libmscore/engravingitem.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"

#ifdef QT_DEBUG
#include "libmscore/staff.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#endif

namespace Ms {
namespace MidiTie {
bool isTied(const Segment* seg, int strack, int voice,
            Ms::Tie* (Note::* tieFunc)() const)
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

bool isTiedFor(const Segment* seg, int strack, int voice)
{
    return isTied(seg, strack, voice, &Note::tieFor);
}

bool isTiedBack(const Segment* seg, int strack, int voice)
{
    return isTied(seg, strack, voice, &Note::tieBack);
}

void TieStateMachine::addSeg(const Segment* seg, int strack)
{
    bool isChord = false;
    for (int voice = 0; voice < VOICES; ++voice) {
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

void printInconsistentTieLocation(int measureIndex, int staffIndex)
{
    qDebug() << "Ties are inconsistent; measure number (from 1):"
             << measureIndex + 1
             << ", staff index (from 0):" << staffIndex;
}

bool areTiesConsistent(const Staff* staff)
{
    const int strack = staff->idx() * VOICES;

    for (int voice = 0; voice < VOICES; ++voice) {
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
} // namespace Ms
