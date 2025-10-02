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

#include "../dom/tremolotwochord.h"

namespace mu::engraving {
class MoveTremolo : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, MoveTremolo)

    Score* score = nullptr;
    Fraction chord1Tick;
    Fraction chord2Tick;
    TremoloTwoChord* trem = nullptr;
    track_idx_t track = 0;

    Chord* oldC1 = nullptr;
    Chord* oldC2 = nullptr;

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    MoveTremolo(Score* s, Fraction c1, Fraction c2, TremoloTwoChord* tr, track_idx_t t)
        : score(s), chord1Tick(c1), chord2Tick(c2), trem(tr), track(t) {}

    UNDO_TYPE(CommandType::MoveTremolo)
    UNDO_NAME("MoveTremolo")
    UNDO_CHANGED_OBJECTS({ trem })
};
}
