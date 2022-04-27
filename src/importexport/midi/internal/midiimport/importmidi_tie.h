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
#ifndef IMPORTMIDI_TIE_H
#define IMPORTMIDI_TIE_H

#include <set>

namespace Ms {
class Segment;
class ChordRest;
class Staff;

namespace MidiTie {
bool isTiedFor(const Segment* seg, Ms::track_idx_t strack, Ms::voice_idx_t voice);
bool isTiedBack(const Segment* seg, Ms::track_idx_t strack, Ms::voice_idx_t voice);

class TieStateMachine
{
public:
    enum class State : char
    {
        UNTIED, TIED_FOR, TIED_BOTH, TIED_BACK
    };

    void addSeg(const Segment* seg, int strack);
    State state() const { return state_; }

private:
    std::set<int> tiedVoices;
    State state_ = State::UNTIED;
};

#ifdef QT_DEBUG
bool areTiesConsistent(const Staff* staff);
#endif
} // namespace MidiTie
} // namespace Ms

#endif // IMPORTMIDI_TIE_H
