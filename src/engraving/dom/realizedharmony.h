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

#ifndef MU_ENGRAVING_REALIZEDHARMONY_H
#define MU_ENGRAVING_REALIZEDHARMONY_H

#include <map>

#include "containers.h"
#include "../types/fraction.h"

namespace mu::engraving {
class Harmony;

//voicing modes to use
enum class Voicing : signed char {
    INVALID = -1,
    AUTO = 0,
    ROOT_ONLY,
    CLOSE,
    DROP_2,
    SIX_NOTE,
    FOUR_NOTE,
    THREE_NOTE
};

//duration to realize notes for
enum class HDuration : signed char {
    INVALID = -1,
    UNTIL_NEXT_CHORD_SYMBOL = 0,    //lasts until the next chord symbol or end of the schore
    STOP_AT_MEASURE_END,            //lasts until next chord symbol or measure end
    SEGMENT_DURATION                //lasts for the duration of the segment
};

//-----------------------------------------
//    Realized Harmony
///     holds information and functions
///     to assist in the realization of chord
///     symbols. This is what is used to
///     allow for chord symbol playback
//-----------------------------------------
class RealizedHarmony
{
public:
    using PitchMap = std::multimap<int, int>;   //map from pitch to tpc

    RealizedHarmony()
        : m_harmony(0), m_notes(PitchMap()), m_dirty(1) {}
    RealizedHarmony(Harmony* h)
        : m_harmony(h), m_notes(PitchMap()), m_dirty(1) {}

    void setVoicing(Voicing);
    void setDuration(HDuration);
    void setLiteral(bool);
    void setDirty(bool dirty) { cascadeDirty(dirty); }   //set dirty flag and cascade
    void setHarmony(Harmony* h) { m_harmony = h; }

    Voicing voicing() const { return m_voicing; }
    HDuration duration() const { return m_duration; }
    bool literal() const { return m_literal; }
    Harmony* harmony() { return m_harmony; }

    bool valid() const { return !m_dirty && m_harmony; }

    const std::vector<int> pitches() const { return muse::keys(notes()); }
    const std::vector<int> tpcs() const { return muse::values(notes()); }

    const PitchMap& notes() const;
    const PitchMap generateNotes(int rootTpc, int bassTpc, bool literal, Voicing voicing, int transposeOffset) const;

    void update(int rootTpc, int bassTpc, int transposeOffset = 0);   //updates the notes map

    Fraction getActualDuration(int utick, HDuration durationType = HDuration::INVALID) const;

private:
    PitchMap getIntervals(int rootTpc, bool literal = true) const;
    PitchMap normalizeNoteMap(const PitchMap& intervals, int rootTpc, int rootPitch, size_t max = 128, bool enforceMaxAsGoal = false) const;
    void cascadeDirty(bool dirty);

    Harmony* m_harmony = nullptr;

    PitchMap m_notes;

    Voicing m_voicing = Voicing::AUTO;
    HDuration m_duration = HDuration::INVALID;

    //whether or not the current notes QMap is up to date
    bool m_dirty = false;
    bool m_literal = false;   //use all notes when possible and do not add any notes
};
}

#endif // __REALIZEDHARMONY_H__
