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

#pragma once

#include "dom/segment.h"
#include "undo.h"

namespace mu::engraving {
class Score;

class CloneVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, CloneVoice)

    Segment* m_sourceSeg = nullptr;
    Fraction m_lTick;
    Segment* m_destSeg = nullptr;
    track_idx_t m_strack = muse::nidx;
    track_idx_t m_dtrack = muse::nidx;
    track_idx_t m_otrack = muse::nidx;
    bool m_link = false;
    bool m_first = true; // first redo

public:
    CloneVoice(Segment* sf, const Fraction& lTick, Segment* d, track_idx_t strack, track_idx_t dtrack, track_idx_t otrack,
               bool linked = true);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::CloneVoice)
    UNDO_NAME("CloneVoice")
    UNDO_CHANGED_OBJECTS({ m_sourceSeg, m_destSeg })
};
}
