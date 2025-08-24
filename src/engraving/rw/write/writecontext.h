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
#pragma once

#include "containers.h"

#include "dom/select.h"
#include "dom/score.h"

namespace mu::engraving::write {
class WriteContext
{
public:

    WriteContext(const Score* s)
        : m_score(s) {}

    std::shared_ptr<IEngravingConfiguration> configuration() const
    {
        return m_score->configuration();
    }

    Fraction curTick() const { return _curTick; }
    void setCurTick(const Fraction& v) { _curTick   = v; }
    void incCurTick(const Fraction& v) { _curTick += v; }

    Fraction tickDiff() const { return _tickDiff; }
    void setTickDiff(const Fraction& v) { _tickDiff  = v; }

    track_idx_t curTrack() const { return _curTrack; }
    void setCurTrack(track_idx_t v) { _curTrack  = v; }
    int trackDiff() const { return _trackDiff; }
    void setTrackDiff(int v) { _trackDiff = v; }

    bool clipboardmode() const { return _clipboardmode; }

    void setClipboardmode(bool v) { _clipboardmode = v; }

    void setFilter(SelectionFilter f) { _filter = f; }
    bool canWrite(const EngravingItem*) const;
    bool canWriteNoteIdx(size_t noteIdx, size_t totalNotesInChord) const;
    bool canWriteVoice(track_idx_t track) const;

    inline bool operator==(const WriteContext& c) const
    {
        return _curTick == c._curTick
               && _tickDiff == c._tickDiff
               && _curTrack == c._curTrack
               && _trackDiff == c._trackDiff
               && _clipboardmode == c._clipboardmode
               && _filter == c._filter;
    }

    inline bool operator!=(const WriteContext& c) const { return !this->operator==(c); }

private:

    const Score* m_score = nullptr;

    Fraction _curTick    { 0, 1 };           // used to optimize output
    Fraction _tickDiff   { 0, 1 };
    track_idx_t _curTrack = muse::nidx;
    int _trackDiff       { 0 };             // saved track is curTrack-trackDiff

    bool _clipboardmode  { false };     // used to modify write() behaviour

    SelectionFilter _filter;
};
}
