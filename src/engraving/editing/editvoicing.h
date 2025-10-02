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

#pragma once

#include "undo.h"

#include "../dom/measure.h"

namespace mu::engraving {
class ExchangeVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ExchangeVoice)

    Measure* measure = nullptr;
    track_idx_t val1 = muse::nidx;
    track_idx_t val2 = muse::nidx;
    staff_idx_t staff = muse::nidx;

public:
    ExchangeVoice(Measure*, track_idx_t val1, track_idx_t val2, staff_idx_t staff);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::ExchangeVoice)
    UNDO_NAME("ExchangeVoice")
    UNDO_CHANGED_OBJECTS({ measure })
};

class CloneVoice : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, CloneVoice)

    Segment* sourceSeg = nullptr;
    Fraction lTick;
    Segment* destSeg = nullptr;               //Destination
    track_idx_t strack = muse::nidx;
    track_idx_t dtrack = muse::nidx;
    track_idx_t otrack;
    bool linked = false;
    bool first = true;        //first redo

public:
    CloneVoice(Segment* sf, const Fraction& lTick, Segment* d, track_idx_t strack, track_idx_t dtrack, track_idx_t otrack,
               bool linked = true);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::CloneVoice)
    UNDO_NAME("CloneVoice")
    UNDO_CHANGED_OBJECTS({ sourceSeg, destSeg })
};
}
