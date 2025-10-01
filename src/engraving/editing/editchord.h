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

namespace mu::engraving {
class ChangeChordStaffMove : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeChordStaffMove)

    ChordRest* chordRest = nullptr;
    int staffMove = 0;

    void flip(EditData*) override;

public:
    ChangeChordStaffMove(ChordRest* cr, int);

    UNDO_TYPE(CommandType::ChangeChordStaffMove)
    UNDO_NAME("ChangeChordStaffMove")
    UNDO_CHANGED_OBJECTS({ chordRest })
};

class SwapCR : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapCR)

    ChordRest* cr1 = nullptr;
    ChordRest* cr2 = nullptr;

    void flip(EditData*) override;

public:
    SwapCR(ChordRest* a, ChordRest* b)
        : cr1(a), cr2(b) {}

    UNDO_TYPE(CommandType::SwapCR)
    UNDO_NAME("SwapCR")
    UNDO_CHANGED_OBJECTS({ cr1, cr2 })
};

class ChangeSpanArpeggio : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeSpanArpeggio)

    Chord* m_chord = nullptr;
    Arpeggio* m_spanArpeggio = nullptr;

    void flip(EditData*) override;
public:
    ChangeSpanArpeggio(Chord* chord, Arpeggio* spanArp)
        : m_chord(chord), m_spanArpeggio(spanArp) {}

    UNDO_NAME("ChangeSpanArpeggio")
    UNDO_CHANGED_OBJECTS({ m_chord })
};
}
