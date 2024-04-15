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

#ifndef MU_ENGRAVING_NOTEEVENT_H
#define MU_ENGRAVING_NOTEEVENT_H

#include <vector>
#include <algorithm>

#include "global/allocator.h"

namespace mu::engraving {
//---------------------------------------------------------
//    NoteEvent
//---------------------------------------------------------

class NoteEvent
{
public:
    constexpr static int NOTE_LENGTH = 1000;
    constexpr static double GHOST_VELOCITY_MULTIPLIER = 0.6;
    constexpr static double DEFAULT_VELOCITY_MULTIPLIER = 1.0;
    constexpr static int SLIDE_AMOUNT = 3;

    NoteEvent() {}
    NoteEvent(int a, int b, int c, double d = 1.0, double play = true, int offset = 0)
        : m_pitch(a), m_ontime(b), m_len(c), m_velocityMultiplier(d), m_play(play), m_offset(offset) {}

    int pitch() const { return m_pitch; }
    int ontime() const { return m_ontime; }
    int offtime() const { return m_ontime + m_len; }
    int len() const { return m_len; }
    double velocityMultiplier() const { return m_velocityMultiplier; }
    bool play() const { return m_play; }
    int offset() const { return m_offset; }
    bool slide() const { return m_slide; }
    bool hammerPull() const { return m_hammerPull; }
    void setPitch(int v) { m_pitch = v; }
    void setOntime(int v) { m_ontime = v; }
    void setLen(int v) { m_len = v; }
    void setVelocityMultiplier(double velocityMultiplier) { m_velocityMultiplier = velocityMultiplier; }
    void setOffset(int v) { m_offset = v; }
    void setSlide(bool slide) { m_slide = slide; }
    void setHammerPull(bool hammerPull) { m_hammerPull = hammerPull; }

    bool operator==(const NoteEvent&) const;

private:
    int m_pitch = 0;     // relative pitch to note pitch
    int m_ontime = 0;    // one unit is 1/1000 of nominal note len
    int m_len = NOTE_LENGTH;       // one unit is 1/1000 of nominal note len
    double m_velocityMultiplier = DEFAULT_VELOCITY_MULTIPLIER; // can be used to lower sound (0-1)
    bool m_play = true; // when 'false', note event is used only for length calculation
    int m_offset = 0;
    bool m_slide = false; // event is slide or glissando
    bool m_hammerPull = false;
};

//---------------------------------------------------------
//   NoteEventList
//---------------------------------------------------------

class NoteEventList : public std::vector<NoteEvent>
{
    OBJECT_ALLOCATOR(engraving, NoteEventList)
public:
    NoteEventList();

    int offtime()
    {
        return empty() ? 0 : std::max_element(cbegin(), cend(), [](const NoteEvent& n1, const NoteEvent& n2) {
            return n1.offtime() < n2.offtime();
        })->offtime();
    }
};
} // namespace mu::engraving
#endif
